/**
 * @file app_init.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief Application initialization and task setup
 * @version 0.3
 * @date 2026-06-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "app.hpp"
#include "cli_task.hpp"
#include "cli_meas_handler.hpp"
#include "meas_task.hpp"
#include "snapshot.hpp"
#include "ui_task.hpp"
#include "sd_logger_task.hpp"
#include "sd.h"
#include "hw_config.h"

#include "lcd.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

extern lcd_cfg_t lcd_cfg;

app_InitStatus
app_Init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        esp_err_t erase_err = nvs_flash_erase();
        if (erase_err != ESP_OK)
        {
            ESP_LOGE("APP", "NVS erase failed: %s", esp_err_to_name(erase_err));
            return app_InitStatus::RESTART;
        }

        err = nvs_flash_init();
    }
    if (err != ESP_OK)
    {
        ESP_LOGE("APP", "NVS init failed: %s", esp_err_to_name(err));
        return app_InitStatus::RESTART;
    }

    // Board initialization
    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE("APP", "GPIO ISR service install failed: %s", esp_err_to_name(err));
        return app_InitStatus::RESTART;
    }

    // Initialize SPI2 HOST
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = SPI2_SCLK;
    buscfg.mosi_io_num = SPI2_MOSI;
    buscfg.miso_io_num = SPI2_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 172 * 40 * sizeof(uint16_t);

    err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
    {
        ESP_LOGE("APP", "initialization of SPI2 failed");
        return app_InitStatus::RESTART;
    }

    // Initialize SD card
    static sd_type_t sd_handle = {};
    const sd_config_t sd_cfg = SD_CONFIG_DEFAULT(SD_CS);
    sd_err_t sd_err = sd_init(&sd_cfg, &sd_handle);
    if (sd_err != SD_OK)
    {
        ESP_LOGE("APP", "SD init failed: %d", sd_err);
        return app_InitStatus::RESTART;
    }

    sd_err = sd_mount(&sd_handle);
    if (sd_err != SD_OK)
    {
        ESP_LOGE("APP", "SD mount failed: %d", sd_err);
        sd_deinit(&sd_handle);
        return app_InitStatus::RESTART;
    }

    // Initialize application tasks
    static CliTask cliTask;
    static MeasTask measTask;
    static CliMeasCmdEntry cliMeasCmd(measTask, cliTask);
    static UiTask uiTask;
    static SdLoggerTask sdLoggerTask;
    static Snapshot snapshot;

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
    uiCfg.buttonPin = BUTTON_SD_PIN;

    SdLoggerTask::Config sdLoggerCfg = {};
    sdLoggerCfg.stackSize = 4096;
    sdLoggerCfg.priority = 3;
    
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

    st = uiTask.init(uiCfg, &snapshot, &sdLoggerTask, &lcd_cfg);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to init uiTask");
        return app_InitStatus::RESTART;
    }

    st = sdLoggerTask.init(sdLoggerCfg, &sd_handle);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to init sdLoggerTask");
        return app_InitStatus::RESTART;
    }

    // Connect MeasTask to SdLoggerTask for zero-loss data path
    st = measTask.setSdLoggerTask(&sdLoggerTask);
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to set SdLoggerTask in measTask");
        return app_InitStatus::RESTART;
    }

    st = cliMeasCmd.cmdRegister();
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

    st = sdLoggerTask.start();
    if (st != ErrStatus::OK)
    {
        ESP_LOGE("APP", "failed to start SD Logger task");
        return app_InitStatus::RESTART;
    }

    return app_InitStatus::DONE;
}
