/**
 * @file app_init.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "app.hpp"
#include "taskMeas.hpp"
#include "measurement.hpp"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define HX711_SCK  GPIO_NUM_3
#define HX711_DOUT GPIO_NUM_2

// Variables
static const char * tag = "APP_INIT";

static hx711_TypeDef s_hx711 = {};
static QueueHandle_t s_MeasEventQueue = NULL;
static Meas s_meas;

static MEAS_TaskContext s_measTaskContext = {
    .hx711 = &s_hx711,
    .meas = &s_meas,
    .eventQueue = &s_MeasEventQueue,
};

init_status_t
app_init()
{
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(tag, "GPIO ISR service install failed: %s", esp_err_to_name(err));
        return INIT_RESTART;
    }

    hx711_StatusTypeDef hx711State = hx711_Open(
        &s_hx711, HX711_SCK, HX711_DOUT, HX711_MODE_A128, 0, MEAS_Hx711DataReadyCallback, NULL);
    if (hx711State != HX711_ERR_OK)
    {
        ESP_LOGE(tag, "hx711 open failed: %d", hx711State);
        return INIT_RESTART;
    }

    err = gpio_intr_enable(s_hx711.ioDout);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "HX711 interrupt enable failed: %s", esp_err_to_name(err));
        return INIT_RESTART;
    }

    s_MeasEventQueue = xQueueCreate(8, 64);

    xTaskCreate(measTask, "MEAS_TASK", 3072, (void *)&s_measTaskContext, 7, nullptr);
    



    //
    return INIT_DONE;
}
