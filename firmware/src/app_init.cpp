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
#include "cli_task.h"
#include "meas_task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

app_InitStatus
app_Init()
{
    // Init global isr service
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE("APP", "GPIO ISR service install failed: %s", esp_err_to_name(err));
        return app_InitStatus::RESTART;
    }

    static cli_Config_t cliConfig = { .stackSize = 3072, .priority = 5 };
    err = cli_TaskInit(&cliConfig);
    if (err != ESP_OK) {
        ESP_LOGE("APP", "CLI init failed: %d", err);
        return app_InitStatus::RESTART;
    }

    static meas_TaskConfig_t measConfig = { .stackSize = 3072, .priority = 6 };
    err = meas_TaskInit(&measConfig);
    if (err != ESP_OK) {
        ESP_LOGE("APP", "MEAS init failed: %d", err);
        return app_InitStatus::RESTART;
    }

    cli_MeasRegister();

    cli_TaskStart();
    meas_TaskStart();

    return app_InitStatus::DONE;
}
