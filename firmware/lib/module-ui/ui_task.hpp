#pragma once

#include "err_status.hpp"
#include "lcd.h"
#include "snapshot.hpp"
#include "button.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstddef>
#include <cstdint>

class UiTask
{
public:
    struct Config
    {
        uint32_t stackSize;
        UBaseType_t priority;
        uint32_t updatePeriodMs;
        gpio_num_t buttonPin;
    };

    ErrStatus init(Config cfg, Snapshot* snapshot, class SdLoggerTask* sd_logger_task, const lcd_cfg_t* lcdCfg);
    ErrStatus start();
    ErrStatus stop();

private:
    static void taskEntry_(void* pvParameters);

    void run_();
    ErrStatus drawBootScreen_();
    ErrStatus drawSnapshot_(const Snapshot::Snap& snap);
    ErrStatus drawTextFieldIfChanged_(uint16_t x,
                                      uint16_t y,
                                      uint16_t width,
                                      const char* text,
                                      uint16_t fg,
                                      uint8_t scale,
                                      char* previous,
                                      size_t previousSize);
    void handleButtonPoll_();

    Config config_ = {};
    TaskHandle_t taskHandle_ = nullptr;
    bool initialized_ = false;

    Snapshot* snapshot_ = nullptr;
    class SdLoggerTask* sd_logger_task_ = nullptr;
    lcd_t lcd_ = {};
    char previousSdStatus_[16] = {};
    char previousConfigLine_[48] = {};
    char previousFilterLine_[48] = {};
    
    // Button state machine
    enum class ButtonState
    {
        IDLE,
        PRESSED,
    };
    
    ButtonState button_state_ = ButtonState::IDLE;
    uint64_t button_press_start_ms_ = 0;
};
