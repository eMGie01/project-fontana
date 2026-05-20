/**
 * @file meas_task.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "meas_task.h"
#include "meas_task_api.h"
#include "measurement.hpp"
#include "hx711.h"
#include "err_status.hpp"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"    // to be deleted

static constexpr gpio_num_t PIN_SCK      = GPIO_NUM_3;
static constexpr gpio_num_t PIN_DOUT     = GPIO_NUM_2;
static constexpr uint8_t    SENSOR_MODE  = HX711_MODE_A128;

static meas_TaskConfig_t    s_Config = {};
static TaskHandle_t         s_TaskHandle = nullptr;
static bool                 s_TaskInitialized = false;

static hx711_TypeDef        s_Sensor = {};
static hx711_HandleTypeDef  s_SensorHandle = &s_Sensor;
static Meas                 s_Meas;
static QueueHandle_t        s_EventQueue = nullptr;

void IRAM_ATTR
meas_DataReadyCB(void* arg)
{
    BaseType_t hpTaskWoken = pdFALSE;
    meas_TaskEvent_t event = {};
    event.type = SENSOR_RDY;
    xQueueSendFromISR(s_EventQueue, &event, &hpTaskWoken);
    if (hpTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

static void
meas_SensorRead(void)
{
    int32_t code;
    int64_t filtValue = 0;
    int64_t avgValue = 0;
    hx711_StatusTypeDef sensorResponse;
    ErrStatus response;

    sensorResponse = hx711_Read(s_SensorHandle, &code);
    if (sensorResponse != HX711_ERR_OK)
    {
        // log error
        return;
    }

    uint64_t time = (uint64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    s_Meas.Write(code);
    (void)s_Meas.ReadFiltValue(filtValue);
    // save filt val to snapshot

    response = s_Meas.ReadAvgValue(avgValue);
    if (response == ErrStatus::TIMEOUT)
    {
        /* for now i log with ESP_LOGX() just to check correctness of my code */
        ESP_LOGI("meas_task", "[%lld, %ld, %lld]", time, code, filtValue);
        /* everything up to next comment will be deleted */
        return;
    }
    // save avg val to snapshot
    /* for now i log with ESP_LOGX() just to check correctness of my code */
    ESP_LOGI("meas_task", "[%lld, %ld, %lld, %lld]", time, code, filtValue, avgValue);
    /* everything up to next comment will be deleted */

}

static void
meas_CommandHandle(const meas_TaskCommand_t cmd)
{
    ErrStatus response;

    switch(cmd.type)
    {
    case RESET:
    {
        s_Meas.reset();
        // log
        break;
    }
    case SET_OFFSET:
    {
        s_Meas.setCodeOffset(cmd.arg.codeOffset);
        // log changed code offset
        break;
    }
    case SET_COUNTS_PER_UMHG:
    {
        response = s_Meas.setCodeCountsPerUmHg(cmd.arg.codeCountsPerUmHg);
        if (response != ErrStatus::OK)
        {
            // log inval
            break;
        }
        // log normal
        break;
    }
    case SET_IIR_SHIFT:
    {
        response = s_Meas.setIirShift(cmd.arg.iirShift);
        if (response != ErrStatus::OK)
        {
            // log inval
            break;
        }
        // log normal
        break;
    }
    case SET_AVG_WINDOW_SIZE:
    {
        response = s_Meas.setAvgWindowSize(cmd.arg.avgWindowSize);
        if (response != ErrStatus::OK)
        {
            // log inval
            break;
        }
        // log normal
        break;
    }
    default:
    {
        // log smth
        break;
    }
    }
}

static void
meas_TaskRun(void* pvParameters)
{
    meas_TaskEvent_t event;
    for (;;)
    {
        if (xQueueReceive(s_EventQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            if (event.type == SENSOR_RDY)
            {
                meas_SensorRead();
            }
            else if (event.type == CMD)
            {
                meas_CommandHandle(event.cmd);
            }
            else
            {
                // log error
                continue;
            }
        }
    }
}

esp_err_t
meas_TaskInit(meas_TaskConfig_t* cfg)
{
    if (cfg == nullptr)
    {
        // log
        return ESP_ERR_INVALID_ARG;
    }

    if (s_TaskInitialized != false)
    {
        // log task already initialized
        return ESP_FAIL;
    }

    s_Config = *cfg;
    s_EventQueue = xQueueCreate(8, sizeof(meas_TaskEvent_t));

    hx711_StatusTypeDef sensorRes = hx711_Open(s_SensorHandle, PIN_SCK, PIN_DOUT, SENSOR_MODE, 0, meas_DataReadyCB, s_EventQueue);
    if (sensorRes != HX711_ERR_OK)
    {
        // log
        return ESP_FAIL;
    }

    s_TaskInitialized = true;
    return ESP_OK;
}

esp_err_t
meas_TaskStart(void)
{
    if (s_TaskHandle != nullptr)
    {
        // log
        return ESP_FAIL;
    }

    BaseType_t taskRes = xTaskCreate(meas_TaskRun, "MEAS_TASK", s_Config.stackSize, nullptr, s_Config.priority, &s_TaskHandle);
    if (taskRes != pdTRUE)
    {
        // log
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t
meas_TaskStop(void)
{
    if (s_TaskHandle == nullptr)
    {
        return ESP_OK;
    }

    vTaskDelete(s_TaskHandle);
    s_TaskHandle = nullptr;
    xQueueReset(s_EventQueue);
    s_TaskInitialized = false;
    return ESP_OK;
}

esp_err_t meas_TaskSendCmd(const meas_TaskCommand_t cmd)
{
	if (s_EventQueue == nullptr)
	{
		return ESP_ERR_INVALID_ARG;
	}
	meas_TaskEvent_t event = {
        .type = CMD,
        .cmd = cmd
    };
	return (xQueueSend(s_EventQueue, &event, portMAX_DELAY) == pdTRUE) ? ESP_OK : ESP_FAIL;
}
