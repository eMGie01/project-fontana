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
#include "sd_logger_task.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"

//

static constexpr char TAG[] = "MEAS_TASK";
static constexpr char NVS_NAMESPACE[] = "meas";
static constexpr char NVS_KEY_OFFSET[] = "offset";
static constexpr char NVS_KEY_COUNTS[] = "counts";
static constexpr char NVS_KEY_IIR[] = "iir";
static constexpr char NVS_KEY_AVGWIN[] = "avgwin";

//

ErrStatus MeasTask::
init(Config cfg, Snapshot* snap)
{
    if (initialized_ == true)
    {
        ESP_LOGE(TAG, "cannot initialize already running module %s", TAG);
        return ErrStatus::FAIL;
    }
    
    if (snap == nullptr)
    {
        ESP_LOGE(TAG, "snap is nullptr, cannot init task Meas");
        return ErrStatus::INVAL;
    }

    config_ = cfg;
    snap_ = snap;
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

    loadConfig_();

    initialized_ = true;
    ESP_LOGI(TAG, "task initialized successfully");

    snap_->setOffset(meas_.getCodeOffset());
    snap_->setCountsPerUmHg(meas_.getCodeCountsPerUmHg());
    snap_->setIirShift(meas_.getIirShift());
    snap_->setAvgWinSize(meas_.getAvgWindowSize());

    return ErrStatus::OK;
}

ErrStatus MeasTask::
setSdLoggerTask(SdLoggerTask* sd_logger_task)
{
    if (sd_logger_task == nullptr)
    {
        ESP_LOGE(TAG, "sd_logger_task is nullptr");
        return ErrStatus::INVAL;
    }
    sd_logger_task_ = sd_logger_task;
    ESP_LOGI(TAG, "SdLoggerTask reference set");
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
    if (taskRes != pdTRUE)
    {
        ESP_LOGE(TAG, "initialization of task failed");
        return ErrStatus::FAIL;
    }

    hx711_StatusTypeDef sensorRes = hx711_Ioctl(&sensor_, HX711_IOCTL_INTR_EN, nullptr);
    if (sensorRes != HX711_ERR_OK)
    {
        ESP_LOGE(TAG, "enabling hx711 interrupt failed");
        return ErrStatus::FAIL;
    }

    ESP_LOGI(TAG, "task started successfully");
    return ErrStatus::OK;
}

ErrStatus MeasTask::
stop()
{
    if (taskHandle_ == nullptr)
    {
        return ErrStatus::OK;
    }
    hx711_StatusTypeDef sensorRes = hx711_Ioctl(&sensor_, HX711_IOCTL_INTR_DIS, nullptr);
    if (sensorRes != HX711_ERR_OK)
    {
        ESP_LOGE(TAG, "disabling hx711 interrupt failed");
        return ErrStatus::FAIL;
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
loadConfig_()
{
    nvs_handle_t nvs = 0;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "no stored measurement config, using defaults");
        return;
    }
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "NVS open failed: %s", esp_err_to_name(err));
        return;
    }

    int32_t offset = 0;
    err = nvs_get_i32(nvs, NVS_KEY_OFFSET, &offset);
    if (err == ESP_OK)
    {
        meas_.setCodeOffset(offset);
    }
    else if (err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "NVS read offset failed: %s", esp_err_to_name(err));
    }

    int32_t counts = 0;
    err = nvs_get_i32(nvs, NVS_KEY_COUNTS, &counts);
    if (err == ESP_OK)
    {
        if (meas_.setCodeCountsPerUmHg(counts) != ErrStatus::OK)
        {
            ESP_LOGW(TAG, "stored counts value is invalid: %ld", counts);
        }
    }
    else if (err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "NVS read counts failed: %s", esp_err_to_name(err));
    }

    uint8_t iir = 0;
    err = nvs_get_u8(nvs, NVS_KEY_IIR, &iir);
    if (err == ESP_OK)
    {
        if (meas_.setIirShift(iir) != ErrStatus::OK)
        {
            ESP_LOGW(TAG, "stored iir value is invalid: %u", iir);
        }
    }
    else if (err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "NVS read iir failed: %s", esp_err_to_name(err));
    }

    uint8_t avgwin = 0;
    err = nvs_get_u8(nvs, NVS_KEY_AVGWIN, &avgwin);
    if (err == ESP_OK)
    {
        if (meas_.setAvgWindowSize(avgwin) != ErrStatus::OK)
        {
            ESP_LOGW(TAG, "stored avgwin value is invalid: %u", avgwin);
        }
    }
    else if (err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "NVS read avgwin failed: %s", esp_err_to_name(err));
    }

    nvs_close(nvs);
    ESP_LOGI(TAG,
             "measurement config loaded: offset=%ld counts=%ld iir=%u avgwin=%u",
             meas_.getCodeOffset(),
             meas_.getCodeCountsPerUmHg(),
             meas_.getIirShift(),
             meas_.getAvgWindowSize());
}

ErrStatus MeasTask::
saveConfig_()
{
    nvs_handle_t nvs = 0;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS open for write failed: %s", esp_err_to_name(err));
        return ErrStatus::FAIL;
    }

    err = nvs_set_i32(nvs, NVS_KEY_OFFSET, meas_.getCodeOffset());
    if (err == ESP_OK)
    {
        err = nvs_set_i32(nvs, NVS_KEY_COUNTS, meas_.getCodeCountsPerUmHg());
    }
    if (err == ESP_OK)
    {
        err = nvs_set_u8(nvs, NVS_KEY_IIR, meas_.getIirShift());
    }
    if (err == ESP_OK)
    {
        err = nvs_set_u8(nvs, NVS_KEY_AVGWIN, meas_.getAvgWindowSize());
    }
    if (err == ESP_OK)
    {
        err = nvs_commit(nvs);
    }

    nvs_close(nvs);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS save failed: %s", esp_err_to_name(err));
        return ErrStatus::FAIL;
    }

    ESP_LOGI(TAG,
             "measurement config saved: offset=%ld counts=%ld iir=%u avgwin=%u",
             meas_.getCodeOffset(),
             meas_.getCodeCountsPerUmHg(),
             meas_.getIirShift(),
             meas_.getAvgWindowSize());
    return ErrStatus::OK;
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
        snap_->setOffset(cmd.arg.codeOffset);
        (void)saveConfig_();
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
        snap_->setCountsPerUmHg(cmd.arg.codeCountsPerUmHg);
        (void)saveConfig_();
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
        snap_->setIirShift(cmd.arg.iirShift);
        (void)saveConfig_();
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
        snap_->setAvgWinSize(cmd.arg.avgWindowSize);
        (void)saveConfig_();
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
    // test
    
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
        snap_->setMeas(time, code, filtVal, 0, false);

        if (sd_logger_task_ != nullptr)
        {
            Snapshot::Snap snap = {};
            if (snap_->read(snap) == ErrStatus::OK)
            {
                sd_logger_task_->sendMeasurement(snap);
            }
        }

        return;
    }

    snap_->setMeas(time, code, filtVal, avgVal, true);
    
    // Send measurement to SD logger task if available
    if (sd_logger_task_ != nullptr)
    {
        Snapshot::Snap snap = {};
        if (snap_->read(snap) == ErrStatus::OK)
        {
            sd_logger_task_->sendMeasurement(snap);
        }
    }
}

//
