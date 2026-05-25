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
// #include "cli_task.hpp"
#include "cli_task.hpp"
#include "cli_meas_handler.hpp"
#include "meas_task.hpp"

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

    static CliTask cliTask;
    static MeasTask measTask;
    static CliMeasCmdEntry measCliCmdEntry(&cliTask, &measTask);

    CliTask::Config cliCfg = {};
    cliCfg.stackSize = 3072;
    cliCfg.priority = 5;

    MeasTask::Config measCfg = {};
    measCfg.stackSize = 3072;
    measCfg.priority = 6;
    
    ErrStatus st;

    st = cliTask.init(cliCfg);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to init cliTask");
        return app_InitStatus::RESTART;
    }

    st = measTask.init(measCfg);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to init measTask");
        return app_InitStatus::RESTART;
    }

    st = measCliCmdEntry.cmdRegister();
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to register meas CLI commands");
        return app_InitStatus::RESTART;
    }

    cliTask.start();
    measTask.start();

    return app_InitStatus::DONE;
}
