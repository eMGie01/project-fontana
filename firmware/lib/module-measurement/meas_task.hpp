/**
 * @file meas_task.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.5
 * @date 2026-05-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MEAS_TASK_HPP
#define MEAS_TASK_HPP

#include "err_status.hpp"
#include "measurement.hpp"
#include "hx711.h"
#include "snapshot.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include <cstdint>

class MeasControlApi
{
public:
    virtual ~MeasControlApi() = default;
    virtual ErrStatus reset() = 0;
    virtual ErrStatus setOffset(int32_t offset) = 0;
    virtual ErrStatus setCountsPerUmHg(int32_t counts) = 0;
    virtual ErrStatus setIirShift(uint8_t shift) = 0;
    virtual ErrStatus setAvgWindowSize(uint8_t size) = 0;
};

class MeasTask : public MeasControlApi
{
public:

    struct Config
    {
        uint32_t    stackSize;
        UBaseType_t priority;
    };

    // Class's Base funcitons
    ErrStatus   init(Config cfg, Snapshot* snap);
    ErrStatus   start();
    ErrStatus   stop();

    // Public API
    ErrStatus   reset();
    ErrStatus   setOffset(int32_t offset);
    ErrStatus   setCountsPerUmHg(int32_t counts);
    ErrStatus   setIirShift(uint8_t shift);
    ErrStatus   setAvgWindowSize(uint8_t avgwin);

private:

    enum class CommandType
    {
        RESET,
        SET_OFFSET,
        SET_COUNTS_PER_UMHG,
        SET_IIR_SHIFT,
        SET_AVG_WINDOW_SIZE,
    };

    struct Command
    {
        CommandType type;
        union
        {
            int32_t codeOffset;
            int32_t codeCountsPerUmHg;
            uint8_t iirShift;
            uint8_t avgWindowSize;
        } arg;
    };

    enum class EventType
    {
        SENSOR_RDY,
        COMMAND,
    };

    struct Event
    {
        EventType type;
        Command   cmd;
    };

    static void             taskEntry_(void* pvParameters);
    static void IRAM_ATTR   cbEntry_(void* arg);

    void        notifySensorReady_ISR();
    void        run_();
    ErrStatus   sendCommand_(const Command& cmd);
    void        handleCommand_(const Command& cmd);
    void        handleSensorReady_();

    static constexpr gpio_num_t PIN_SCK      = GPIO_NUM_3;
    static constexpr gpio_num_t PIN_DOUT     = GPIO_NUM_2;
    static constexpr uint8_t    SENSOR_MODE  = HX711_MODE_A128;

    Config          config_ = {};
    TaskHandle_t    taskHandle_ = nullptr;
    bool            initialized_ = false;

    hx711_TypeDef   sensor_ = {};
    Meas            meas_;
    Snapshot*       snap_;
    QueueHandle_t   eventQueue_ = nullptr;

};

#endif // MEAS_TASK_HPP
