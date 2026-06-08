/**
 * @file sd_logger_task.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief SD card data logging task implementation
 * @version 0.1
 * @date 2026-06-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "sd_logger_task.hpp"
#include "esp_log.h"

#include <cstdio>
#include <cstring>

ErrStatus SdLoggerTask::
init(Config cfg, sd_handle_t sd_handle)
{
    if (initialized_)
    {
        ESP_LOGE(TAG, "cannot initialize already running module");
        return ErrStatus::FAIL;
    }
    
    if (sd_handle == nullptr)
    {
        ESP_LOGE(TAG, "sd_handle is nullptr");
        return ErrStatus::INVAL;
    }

    config_ = cfg;
    sd_handle_ = sd_handle;
    
    eventQueue_ = xQueueCreate(8, sizeof(Event));
    if (eventQueue_ == nullptr)
    {
        ESP_LOGE(TAG, "eventQueue creation failed");
        return ErrStatus::FAIL;
    }

    recording_enabled_ = false;
    file_open_ = false;
    sample_buffer_len_ = 0;
    buffered_samples_ = 0;
    memset(&current_file_, 0, sizeof(current_file_));

    initialized_ = true;
    ESP_LOGI(TAG, "task initialized successfully");
    return ErrStatus::OK;
}

ErrStatus SdLoggerTask::
start()
{
    if (!initialized_)
    {
        ESP_LOGE(TAG, "task not initialized");
        return ErrStatus::NODEV;
    }

    if (taskHandle_ != nullptr)
    {
        ESP_LOGE(TAG, "task already running");
        return ErrStatus::FAIL;
    }

    BaseType_t res = xTaskCreate(taskEntry_, "SD_LOGGER", config_.stackSize, this, config_.priority, &taskHandle_);
    if (res != pdTRUE)
    {
        ESP_LOGE(TAG, "task creation failed");
        return ErrStatus::FAIL;
    }

    ESP_LOGI(TAG, "task started successfully");
    return ErrStatus::OK;
}

ErrStatus SdLoggerTask::
stop()
{
    if (taskHandle_ == nullptr)
    {
        return ErrStatus::OK;
    }

    // Close file if open
    (void)flushBuffer_();
    if (file_open_)
    {
        sd_file_close(&current_file_);
        file_open_ = false;
    }

    vTaskDelete(taskHandle_);
    taskHandle_ = nullptr;

    if (xQueueReset(eventQueue_) == pdFALSE)
    {
        ESP_LOGE(TAG, "failed to reset eventQueue");
        return ErrStatus::FAIL;
    }

    initialized_ = false;
    ESP_LOGI(TAG, "task stopped successfully");
    return ErrStatus::OK;
}

ErrStatus SdLoggerTask::
enableRecording()
{
    Event evt = {};
    evt.type = EventType::CONTROL_CMD;
    evt.data.ctrl.type = ControlCommand::Type::ENABLE_RECORDING;
    return sendEvent_(evt);
}

ErrStatus SdLoggerTask::
disableRecording()
{
    Event evt = {};
    evt.type = EventType::CONTROL_CMD;
    evt.data.ctrl.type = ControlCommand::Type::DISABLE_RECORDING;
    return sendEvent_(evt);
}

bool SdLoggerTask::
isRecording() const
{
    return recording_enabled_;
}

ErrStatus SdLoggerTask::
sendMeasurement(const Snapshot::Snap& snap)
{
    if (eventQueue_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    Event evt = {};
    evt.type = EventType::MEASUREMENT_READY;
    evt.data.snap = snap;

    BaseType_t res = xQueueSend(eventQueue_, &evt, pdMS_TO_TICKS(100));
    if (res != pdTRUE)
    {
        ESP_LOGW(TAG, "failed to send measurement to queue");
        return ErrStatus::FAIL;
    }

    return ErrStatus::OK;
}

void SdLoggerTask::
taskEntry_(void* pvParameters)
{
    auto* self = static_cast<SdLoggerTask*>(pvParameters);
    self->run_();
}

ErrStatus SdLoggerTask::
sendEvent_(const Event& evt)
{
    if (eventQueue_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    BaseType_t res = xQueueSend(eventQueue_, &evt, pdMS_TO_TICKS(100));
    if (res != pdTRUE)
    {
        ESP_LOGW(TAG, "failed to send event to queue");
        return ErrStatus::FAIL;
    }

    return ErrStatus::OK;
}

ErrStatus SdLoggerTask::
openFileIfNeeded_()
{
    if (file_open_)
    {
        return ErrStatus::OK;
    }

    sd_err_t err = sd_file_create(sd_handle_, &current_file_);
    if (err != SD_OK)
    {
        ESP_LOGE(TAG, "failed to create file: %d", err);
        return ErrStatus::FAIL;
    }

    file_open_ = true;
    ESP_LOGI(TAG, "file opened for logging");
    return ErrStatus::OK;
}

ErrStatus SdLoggerTask::
flushBuffer_()
{
    if (buffered_samples_ == 0)
    {
        return ErrStatus::OK;
    }

    ErrStatus st = openFileIfNeeded_();
    if (st != ErrStatus::OK)
    {
        return st;
    }

    sd_err_t err = sd_file_write_block(&current_file_,
                                       sample_buffer_,
                                       sample_buffer_len_,
                                       buffered_samples_);
    if (err != SD_OK)
    {
        ESP_LOGE(TAG, "failed to flush sample buffer: %d", err);
        return ErrStatus::FAIL;
    }

    ESP_LOGI(TAG, "flushed %u buffered samples", (unsigned)buffered_samples_);
    sample_buffer_len_ = 0;
    buffered_samples_ = 0;
    return ErrStatus::OK;
}

void SdLoggerTask::
handleMeasurementReady_(const Snapshot::Snap& snap)
{
    if (!recording_enabled_)
    {
        return;
    }

    // Format measurement data as CSV line
    char csv_line[CSV_LINE_MAX_LEN];
    int line_len = snprintf(csv_line, sizeof(csv_line),
                            "%llu,%ld,%lld,%lld,%ld,%ld,%u,%u\n",
                            snap.timeMs,
                            snap.code,
                            snap.filtered,
                            snap.averaged,
                            snap.codeOffset,
                            snap.codeCounts,
                            snap.iirShift,
                            snap.avgWin);
    if (line_len < 0 || line_len >= (int)sizeof(csv_line))
    {
        ESP_LOGE(TAG, "CSV line too long");
        return;
    }

    if ((sample_buffer_len_ + (size_t)line_len) > sizeof(sample_buffer_))
    {
        if (flushBuffer_() != ErrStatus::OK)
        {
            return;
        }
    }

    memcpy(&sample_buffer_[sample_buffer_len_], csv_line, (size_t)line_len);
    sample_buffer_len_ += (size_t)line_len;
    buffered_samples_++;

    if (buffered_samples_ >= BUFFERED_SAMPLE_LIMIT)
    {
        (void)flushBuffer_();
    }
}

void SdLoggerTask::
run_()
{
    Event event;

    for (;;)
    {
        // Wait indefinitely for events (measurements or control commands)
        if (xQueueReceive(eventQueue_, &event, portMAX_DELAY) == pdTRUE)
        {
            if (event.type == EventType::MEASUREMENT_READY)
            {
                // Received measurement from MeasTask
                handleMeasurementReady_(event.data.snap);
            }
            else if (event.type == EventType::CONTROL_CMD)
            {
                // Process control command
                if (event.data.ctrl.type == ControlCommand::Type::ENABLE_RECORDING)
                {
                    recording_enabled_ = true;
                    ESP_LOGI(TAG, "recording enabled");
                }
                else if (event.data.ctrl.type == ControlCommand::Type::DISABLE_RECORDING)
                {
                    recording_enabled_ = false;
                    (void)flushBuffer_();

                    // Close file if open
                    if (file_open_)
                    {
                        sd_err_t err = sd_file_close(&current_file_);
                        if (err != SD_OK)
                        {
                            ESP_LOGE(TAG, "failed to close file: %d", err);
                        }
                        file_open_ = false;
                        ESP_LOGI(TAG, "file closed");
                    }

                    ESP_LOGI(TAG, "recording disabled");
                }
            }
        }
    }
}
