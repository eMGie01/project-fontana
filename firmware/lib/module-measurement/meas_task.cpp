/**
 * @file meas_task.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.5
 * @date 2026-05-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "meas_task.hpp"
#include "esp_log.h"

//

static constexpr char TAG[] = "MEAS_TASK";

//

ErrStatus MeasTask::
init(Config cfg)
{
    if (initialized_ == true)
    {
        ESP_LOGE(TAG, "cannot initialize already running module %s", TAG);
        return ErrStatus::FAIL;
    }
    config_ = cfg;
    eventQueue_ = xQueueCreate(8, sizeof(Event));
    if (eventQueue_ == nullptr)
    {
        ESP_LOGE(TAG, "eventQueue_ creation failed");
        return ErrStatus::FAIL;
    }
    hx711_StatusTypeDef sensorRes = hx711_Open(&sensor_, PIN_SCK, PIN_DOUT, SENSOR_MODE, 0, cbEntry_, this);
    if (sensorRes != HX711_ERR_OK)
    {
        ESP_LOGE(TAG, "initialization of hx711 failed with error: %d", sensorRes);
        vQueueDelete(eventQueue_);
        eventQueue_ = nullptr;
        return ErrStatus::FAIL;
    }
    initialized_ = true;
    ESP_LOGI(TAG, "task initialized successfully");
    return ErrStatus::OK;
}

ErrStatus MeasTask::
start()
{
    if (taskHandle_ != nullptr)
    {
        ESP_LOGE(TAG, "starting failed, task already running");
        return ErrStatus::FAIL;
    }

    BaseType_t taskRes = xTaskCreate(taskEntry_, "MEAS_TASK", config_.stackSize, this, config_.priority, &taskHandle_);
    if (taskRes == pdTRUE)
    {
        ESP_LOGI(TAG, "task started successfully");
        return ErrStatus::OK;
    }
    ESP_LOGE(TAG, "initialization of task failed");
    return ErrStatus::FAIL;
}

ErrStatus MeasTask::
stop()
{
    if (taskHandle_ == nullptr)
    {
        return ErrStatus::OK;
    }

    vTaskDelete(taskHandle_);
    taskHandle_ = nullptr;
    if (xQueueReset(eventQueue_) == pdFALSE)
    {
        ESP_LOGE(TAG, "reset of eventQueue_ failed");
        return ErrStatus::FAIL;
    }
    initialized_ = false;
    ESP_LOGI(TAG, "task stopped successfully");
    return ErrStatus::OK;
}

//

ErrStatus MeasTask::
reset(void)
{
    Command cmd = {};
    cmd.type = CommandType::RESET;
    return sendCommand_(cmd);
}

ErrStatus MeasTask::
setOffset(int32_t offset)
{
    Command cmd = {};
    cmd.type = CommandType::SET_OFFSET;
    cmd.arg.codeOffset = offset;
    return sendCommand_(cmd);
}

ErrStatus MeasTask::
setCountsPerUmHg(int32_t counts)
{
    Command cmd = {};
    cmd.type = CommandType::SET_COUNTS_PER_UMHG;
    cmd.arg.codeCountsPerUmHg = counts;
    return sendCommand_(cmd);
}

ErrStatus MeasTask::
setIirShift(uint8_t shift)
{
    Command cmd = {};
    cmd.type = CommandType::SET_IIR_SHIFT;
    cmd.arg.iirShift = shift;
    return sendCommand_(cmd);
}

ErrStatus MeasTask::
setAvgWindowSize(uint8_t avgwin)
{
    Command cmd = {};
    cmd.type = CommandType::SET_AVG_WINDOW_SIZE;
    cmd.arg.avgWindowSize = avgwin;
    return sendCommand_(cmd);
}

//

void MeasTask::
taskEntry_(void* pvParameters)
{
    auto* self = static_cast<MeasTask*>(pvParameters);
    self->run_();
}

void IRAM_ATTR MeasTask::
cbEntry_(void* arg)
{
    auto* self = static_cast<MeasTask*>(arg);
    self->notifySensorReady_ISR();
}

void MeasTask::
notifySensorReady_ISR()
{
    if (eventQueue_ == nullptr)
    {
        return;
    }
    BaseType_t hpTaskWoken = pdFALSE;
    Event event = {};
    event.type = EventType::SENSOR_RDY;
    xQueueSendFromISR(eventQueue_, &event, &hpTaskWoken);
    if (hpTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

void MeasTask::
run_()
{
    Event event;
    for (;;)
    {
        if (xQueueReceive(eventQueue_, &event, portMAX_DELAY) == pdTRUE)
        {
            if (event.type == EventType::SENSOR_RDY)
            {
                handleSensorReady_();
            }
            else if (event.type == EventType::COMMAND)
            {
                handleCommand_(event.cmd);
            }
            else
            {
                ESP_LOGW(TAG, "incorrect queue type");
                continue;
            }
        }
    }
}

ErrStatus MeasTask::
sendCommand_(const Command& cmd)
{
    if (eventQueue_ == nullptr)
    {
        ESP_LOGE(TAG, "eventQueue_ is nullptr");
        return ErrStatus::NODEV;
    }
    Event event = {};
    event.type = EventType::COMMAND;
    event.cmd = cmd;
    return (xQueueSend(eventQueue_, &event, portMAX_DELAY) == pdTRUE) ? ErrStatus::OK : ErrStatus::FAIL;
}

void MeasTask::
handleCommand_(const Command& cmd)
{
    ErrStatus response;
    switch(cmd.type)
    {
    case CommandType::RESET:
    {
        meas_.reset();
        ESP_LOGI(TAG, "meas values reset");
        break;
    }
    case CommandType::SET_OFFSET:
    {
        meas_.setCodeOffset(cmd.arg.codeOffset);
        ESP_LOGI(TAG, "meas offset changed to: %ld", cmd.arg.codeOffset);
        break;
    }
    case CommandType::SET_COUNTS_PER_UMHG:
    {
        response = meas_.setCodeCountsPerUmHg(cmd.arg.codeCountsPerUmHg);
        if (response != ErrStatus::OK)
        {
            ESP_LOGW(TAG, "meas code_counts_per_umHg cannot be set to: %ld", cmd.arg.codeCountsPerUmHg);
            break;
        }
        ESP_LOGI(TAG, "meas code_counts_per_umHg set to: %ld", cmd.arg.codeCountsPerUmHg);
        break;
    }
    case CommandType::SET_IIR_SHIFT:
    {
        response = meas_.setIirShift(cmd.arg.iirShift);
        if (response != ErrStatus::OK)
        {
            ESP_LOGW(TAG, "meas iir_shift cannot be set to: %d", cmd.arg.iirShift);
            break;
        }
        ESP_LOGI(TAG, "meas iir_shift set to: %d", cmd.arg.iirShift);
        break;
    }
    case CommandType::SET_AVG_WINDOW_SIZE:
    {
        response = meas_.setAvgWindowSize(cmd.arg.avgWindowSize);
        if (response != ErrStatus::OK)
        {
            ESP_LOGW(TAG, "meas avgwin cannot be set to: %d", cmd.arg.avgWindowSize);
            break;
        }
        ESP_LOGI(TAG, "meas avgwin set to: %d", cmd.arg.avgWindowSize);
        break;
    }
    default:
    {
        ESP_LOGE(TAG, "incorrect command type");
        break;
    }
    }
}

void MeasTask::
handleSensorReady_()
{
    hx711_StatusTypeDef sensorResponse;
    ErrStatus response;

    int32_t code;
    int64_t filtVal = 0;
    int64_t avgVal = 0;
    
    sensorResponse = hx711_Read(&sensor_, &code);
    if (sensorResponse != HX711_ERR_OK)
    {
        ESP_LOGE(TAG, "Sensor read failed with error (%d)", sensorResponse);
        return;
    }

    uint64_t time = (uint64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    meas_.write(code);
    (void)meas_.readFiltVal(filtVal);
    // save filt val to snapshot

    response = meas_.readAvgVal(avgVal);
    if (response == ErrStatus::TIMEOUT)
    {
        /* for now i log with ESP_LOGX() just to check correctness of my code */
        ESP_LOGI(TAG, "[%lld, %ld, %lld]", time, code, filtVal);
        /* everything up to next comment will be deleted */
        return;
    }
    // save avg val to snapshot
    /* for now i log with ESP_LOGX() just to check correctness of my code */
    ESP_LOGI(TAG, "[%lld, %ld, %lld, %lld]", time, code, filtVal, avgVal);
    /* everything up to next comment will be deleted */
}

//
