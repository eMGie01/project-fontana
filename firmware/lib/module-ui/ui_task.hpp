#pragma once

#include "err_status.hpp"
#include "lcd.h"
#include "snapshot.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdint>

class UiTask
{
public:
    struct Config
    {
        uint32_t stackSize;
        UBaseType_t priority;
        uint32_t updatePeriodMs;
    };

    ErrStatus init(Config cfg, Snapshot* snapshot, const lcd_cfg_t* lcdCfg);
    ErrStatus start();
    ErrStatus stop();

private:
    static void taskEntry_(void* pvParameters);

    void run_();
    ErrStatus drawBootScreen_();
    ErrStatus drawSnapshot_(const Snapshot::Snap& snap);

    Config config_ = {};
    TaskHandle_t taskHandle_ = nullptr;
    bool initialized_ = false;

    Snapshot* snapshot_ = nullptr;
    lcd_t lcd_ = {};
};
