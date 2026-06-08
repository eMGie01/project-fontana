#include "ui_task.hpp"
#include "sd_logger_task.hpp"
#include "hw_config.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#include <cstdio>
#include <cstring>

namespace
{
constexpr char TAG[] = "UI_TASK";
constexpr char FW_VER[] = "ver 01";
constexpr uint64_t BUTTON_SHORT_PRESS_MS = 30;
constexpr uint64_t BUTTON_LONG_PRESS_MS = 3000;
constexpr uint32_t BUTTON_POLL_PERIOD_MS = 20;
}

ErrStatus UiTask::
init(Config cfg, Snapshot* snapshot, SdLoggerTask* sd_logger_task, const lcd_cfg_t* lcdCfg)
{
    if (initialized_)
    {
        ESP_LOGE(TAG, "cannot initialize already running module");
        return ErrStatus::FAIL;
    }
    if (snapshot == nullptr || lcdCfg == nullptr || sd_logger_task == nullptr)
    {
        ESP_LOGE(TAG, "snapshot, sd_logger_task or lcdCfg is nullptr");
        return ErrStatus::INVAL;
    }

    config_ = cfg;
    if (config_.updatePeriodMs == 0)
    {
        config_.updatePeriodMs = 1000;
    }
    snapshot_ = snapshot;
    sd_logger_task_ = sd_logger_task;

    // Initialize button
    esp_err_t err = button_init(config_.buttonPin);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "button_init failed: %s", esp_err_to_name(err));
        return ErrStatus::FAIL;
    }

    err = lcd_open(&lcd_, lcdCfg);
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
    const TickType_t buttonPeriod = pdMS_TO_TICKS(BUTTON_POLL_PERIOD_MS);
    const TickType_t drawPeriod = pdMS_TO_TICKS(config_.updatePeriodMs);
    TickType_t lastDraw = 0;
    bool forceDraw = true;

    for (;;)
    {
        TickType_t now = xTaskGetTickCount();
        if (forceDraw || (now - lastDraw) >= drawPeriod)
        {
            Snapshot::Snap snap = {};
            ErrStatus st = snapshot_->read(snap);
            if (st == ErrStatus::OK)
            {
                (void)drawSnapshot_(snap);
                if (snap.avgLcd)
                {
                    (void)snapshot_->cleanAvgLcdFlag();
                }
            }
            else
            {
                ESP_LOGW(TAG, "snapshot read failed: %d", static_cast<int>(st));
            }

            lastDraw = now;
            forceDraw = false;
        }

        handleButtonPoll_();

        vTaskDelayUntil(&lastWake, buttonPeriod);
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

    bool recording = sd_logger_task_ != nullptr && sd_logger_task_->isRecording();
    ErrStatus st = drawTextFieldIfChanged_(225,
                                           8,
                                           90,
                                           recording ? "SD:REC" : "SD:OFF",
                                           recording ? LCD_COLOR_RED : LCD_COLOR_DARKGREY,
                                           2,
                                           previousSdStatus_,
                                           sizeof(previousSdStatus_));
    if (st != ErrStatus::OK)
    {
        return st;
    }

    std::snprintf(
        buf,
        sizeof(buf),
        "O:%ld K:%ld",
        static_cast<long>(snap.codeOffset),
        static_cast<long>(snap.codeCounts));
    st = drawTextFieldIfChanged_(15,
                                 130,
                                 300,
                                 buf,
                                 LCD_COLOR_DARKGREY,
                                 2,
                                 previousConfigLine_,
                                 sizeof(previousConfigLine_));
    if (st != ErrStatus::OK)
    {
        return st;
    }

    std::snprintf(buf, sizeof(buf), "AVG:%u IIR:%u", snap.avgWin, snap.iirShift);
    st = drawTextFieldIfChanged_(15,
                                 150,
                                 300,
                                 buf,
                                 LCD_COLOR_DARKGREY,
                                 2,
                                 previousFilterLine_,
                                 sizeof(previousFilterLine_));
    if (st != ErrStatus::OK)
    {
        return st;
    }

    return ErrStatus::OK;
}

ErrStatus UiTask::
drawTextFieldIfChanged_(uint16_t x,
                        uint16_t y,
                        uint16_t width,
                        const char* text,
                        uint16_t fg,
                        uint8_t scale,
                        char* previous,
                        size_t previousSize)
{
    if (text == nullptr || previous == nullptr || previousSize == 0)
    {
        return ErrStatus::INVAL;
    }

    if (std::strncmp(previous, text, previousSize) == 0)
    {
        return ErrStatus::OK;
    }

    esp_err_t err = lcd_fillRect(&lcd_, x, y, width, 8U * scale, LCD_COLOR_BLACK);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    err = lcd_drawString(&lcd_, x, y, text, fg, LCD_COLOR_BLACK, scale);
    if (err != ESP_OK)
    {
        return ErrStatus::FAIL;
    }

    std::strncpy(previous, text, previousSize - 1);
    previous[previousSize - 1] = '\0';
    return ErrStatus::OK;
}

void UiTask::
handleButtonPoll_()
{
    if (sd_logger_task_ == nullptr)
    {
        return;
    }

    bool button_pressed = button_read(config_.buttonPin);
    uint64_t current_time_ms = esp_timer_get_time() / 1000;  // Convert microseconds to milliseconds

    switch (button_state_)
    {
        case ButtonState::IDLE:
            // Waiting for button press
            if (button_pressed)
            {
                button_state_ = ButtonState::PRESSED;
                button_press_start_ms_ = current_time_ms;
                ESP_LOGD(TAG, "Button pressed at %llu ms", button_press_start_ms_);
            }
            break;

        case ButtonState::PRESSED:
            // Button is held, check for short or long press
            if (!button_pressed)
            {
                // Button released - check duration
                uint64_t press_duration_ms = current_time_ms - button_press_start_ms_;

                if (press_duration_ms >= BUTTON_SHORT_PRESS_MS && 
                    press_duration_ms < BUTTON_LONG_PRESS_MS)
                {
                    // Short press detected
                    ESP_LOGI(TAG, "Short press detected (%llu ms) - toggle recording", press_duration_ms);
                    
                    // Toggle recording state
                    if (sd_logger_task_->isRecording())
                    {
                        sd_logger_task_->disableRecording();
                        ESP_LOGI(TAG, "Recording disabled");
                    }
                    else
                    {
                        sd_logger_task_->enableRecording();
                        ESP_LOGI(TAG, "Recording enabled");
                    }

                    button_state_ = ButtonState::IDLE;
                }
                else if (press_duration_ms < BUTTON_SHORT_PRESS_MS)
                {
                    // Too short - debounce noise
                    ESP_LOGD(TAG, "Debounce detected (%llu ms)", press_duration_ms);
                    button_state_ = ButtonState::IDLE;
                }
                else
                {
                    // Long press was already reported
                    button_state_ = ButtonState::IDLE;
                }
            }
            break;
    }
}
