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
#include "cli_task.hpp"
#include "cli_meas_handler2.hpp"
#include "meas_task.hpp"
#include "snapshot.hpp"
#include "ui_task.hpp"

#include "lcd.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

extern lcd_cfg_t lcd_cfg;

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
<<<<<<< HEAD
    static CliMeasCmdEntry cliMeasCmd(&measTask, &cliTask);
=======
    static UiTask uiTask;
    static Snapshot snapshot;
    static CliMeasCmdEntry measCliCmdEntry(&cliTask, &measTask);
>>>>>>> bd872d901b0a1e4c34a96b0f01b1e3c0155bde31

    CliTask::Config cliCfg = {};
    cliCfg.stackSize = 4096;
    cliCfg.priority = 5;

    MeasTask::Config measCfg = {};
    measCfg.stackSize = 4096;
    measCfg.priority = 6;

    UiTask::Config uiCfg = {};
    uiCfg.stackSize = 4096;
    uiCfg.priority = 4;
    uiCfg.updatePeriodMs = 1000;
    
    ErrStatus st;

    st = snapshot.open();
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to open snapshot");
        return app_InitStatus::RESTART;
    }

    st = cliTask.init(cliCfg);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to init cliTask");
        return app_InitStatus::RESTART;
    }

    st = measTask.init(measCfg, &snapshot);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to init measTask");
        return app_InitStatus::RESTART;
    }

<<<<<<< HEAD
    st = cliMeasCmd.cmdRegister();
=======
    st = uiTask.init(uiCfg, &snapshot, &lcd_cfg);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to init uiTask");
        return app_InitStatus::RESTART;
    }

    st = measCliCmdEntry.cmdRegister();
>>>>>>> bd872d901b0a1e4c34a96b0f01b1e3c0155bde31
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to register meas CLI commands");
        return app_InitStatus::RESTART;
    }

    st = cliTask.start();
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to start CLI task");
        return app_InitStatus::RESTART;
    }

    st = measTask.start();
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to start MEAS task");
        return app_InitStatus::RESTART;
    }

    st = uiTask.start();
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to start UI task");
        return app_InitStatus::RESTART;
    }

    return app_InitStatus::DONE;
}
