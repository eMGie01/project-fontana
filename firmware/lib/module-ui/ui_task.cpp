#include "ui_task.hpp"

#include "esp_err.h"
#include "esp_log.h"

#include <cstdio>

namespace
{
constexpr char TAG[] = "UI_TASK";
constexpr char FW_VER[] = "ver 01";
}

ErrStatus UiTask::
init(Config cfg, Snapshot* snapshot, const lcd_cfg_t* lcdCfg)
{
    if (initialized_)
    {
        ESP_LOGE(TAG, "cannot initialize already running module");
        return ErrStatus::FAIL;
    }
    if (snapshot == nullptr || lcdCfg == nullptr)
    {
        ESP_LOGE(TAG, "snapshot or lcdCfg is nullptr");
        return ErrStatus::INVAL;
    }

    config_ = cfg;
    if (config_.updatePeriodMs == 0)
    {
        config_.updatePeriodMs = 1000;
    }
    snapshot_ = snapshot;

    esp_err_t err = lcd_open(&lcd_, lcdCfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "lcd_open failed: %s", esp_err_to_name(err));
        return ErrStatus::FAIL;
    }

    err = lcd_landscape(&lcd_, false);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "lcd_landscape failed: %s", esp_err_to_name(err));
        return ErrStatus::FAIL;
    }

    ErrStatus st = drawBootScreen_();
    if (st != ErrStatus::OK)
    {
        return st;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "task initialized successfully");
    return ErrStatus::OK;
}

ErrStatus UiTask::
start()
{
    if (!initialized_)
    {
        return ErrStatus::NODEV;
    }
    if (taskHandle_ != nullptr)
    {
        return ErrStatus::OK;
    }

    BaseType_t res = xTaskCreate(taskEntry_, "UI_TASK", config_.stackSize, this, config_.priority, &taskHandle_);
    if (res != pdTRUE)
    {
        ESP_LOGE(TAG, "task creation failed");
        return ErrStatus::FAIL;
    }

    ESP_LOGI(TAG, "task started successfully");
    return ErrStatus::OK;
}

ErrStatus UiTask::
stop()
{
    if (taskHandle_ != nullptr)
    {
        vTaskDelete(taskHandle_);
        taskHandle_ = nullptr;
    }

    ESP_LOGI(TAG, "task stopped successfully");
    return ErrStatus::OK;
}

void UiTask::
taskEntry_(void* pvParameters)
{
    auto* self = static_cast<UiTask*>(pvParameters);
    self->run_();
}

void UiTask::
run_()
{
    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(config_.updatePeriodMs);

    for (;;)
    {
        Snapshot::Snap snap = {};
        ErrStatus st = snapshot_->read(snap);
        if (st == ErrStatus::OK)
        {
            (void)drawSnapshot_(snap);
            if (snap.avgLcd)
            {
                (void)snapshot_->toggleAvgLcdFlag();
            }
        }
        else
        {
            ESP_LOGW(TAG, "snapshot read failed: %d", static_cast<int>(st));
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

ErrStatus UiTask::
drawBootScreen_()
{
    esp_err_t err = lcd_fillScreen(&lcd_, LCD_COLOR_BLACK);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    err = lcd_drawString(&lcd_, 30, 20, "Data logger", LCD_COLOR_GREEN, LCD_COLOR_BLACK, 4);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    err = lcd_drawString(&lcd_, 100, 60, FW_VER, LCD_COLOR_LIGHTGREY, LCD_COLOR_BLACK, 3);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    err = lcd_drawString(&lcd_, 50, 130, "Init...", LCD_COLOR_WHITE, LCD_COLOR_BLACK, 4);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(700));
    err = lcd_fillScreen(&lcd_, LCD_COLOR_BLACK);
    return (err == ESP_OK) ? ErrStatus::OK : ErrStatus::FAIL;
}

ErrStatus UiTask::
drawSnapshot_(const Snapshot::Snap& snap)
{
    char buf[48];
    esp_err_t err;

    std::snprintf(buf, sizeof(buf), "%8llu s", static_cast<unsigned long long>(snap.timeMs / 1000ULL));
    err = lcd_drawString(&lcd_, 150, 30, buf, LCD_COLOR_LIGHTGREY, LCD_COLOR_BLACK, 2);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    int64_t value = snap.filtered;
    int64_t whole = value / 1000;
    int64_t frac = (value % 1000) / 10;
    frac = (frac < 0) ? -frac : frac; 
    std::snprintf(buf, sizeof(buf), "%5lld.%02lld", whole, frac);
    err = lcd_drawString(&lcd_, 10, 60, buf, LCD_COLOR_WHITE, LCD_COLOR_BLACK, 5);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    err = lcd_drawString(&lcd_, 260, 90, " mmHg", LCD_COLOR_GREEN, LCD_COLOR_BLACK, 2);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    if (snap.avgLcd)
    {
        value = snap.averaged;
        whole = value / 1000;
        frac = (value % 1000) / 10;
        frac = (frac < 0) ? -frac : frac; 
        std::snprintf(buf, sizeof(buf), "AVG: %lld.%02lld", whole, frac);
        err = lcd_drawString(&lcd_, 15, 112, buf, LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 2);
        if (err != ESP_OK)
        {
            return ErrStatus::FAIL;
        }
    }

    std::snprintf(
        buf,
        sizeof(buf),
        "O:%ld K:%ld",
        static_cast<long>(snap.codeOffset),
        static_cast<long>(snap.codeCounts));
    err = lcd_drawString(&lcd_, 15, 130, buf, LCD_COLOR_DARKGREY, LCD_COLOR_BLACK, 2);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    std::snprintf(buf, sizeof(buf), "AVG:%u IIR:%u", snap.avgWin, snap.iirShift);
    err = lcd_drawString(&lcd_, 15, 150, buf, LCD_COLOR_DARKGREY, LCD_COLOR_BLACK, 2);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    return ErrStatus::OK;
}
