/**
 * @file meas_task.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-05-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MEAS_TASK_HPP
#define MEAS_TASK_HPP

#include "hx711.h"
#include "measurement.hpp"
#include "meas_task_api.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

/**
 * @brief Type of event that occurs in Meas (for asynch)
 */
enum class meas_TaskEventType
{
    HX711_READY,
    CMD,
};

/**
 * @brief event structure, used for asynch
 */
struct meas_TaskEvent
{
    meas_TaskEventType type;
    union 
    {
        meas_TaskCmd cmd;
    } data;
};

/**
 * @brief Context structure of Meas Task
 */
struct meas_TaskContext
{
    hx711_HandleTypeDef hx711;
    Meas* meas;
    QueueHandle_t eventQueue; 
};

/**
 * @brief Callback for sensor (in our case Hx711), signal of data readiness
 * @param arg parameter for callback (in our case, NULL)
 */
void meas_DataReadyCallback(void* arg);

/**
 * @brief Main code of Meas Task (fully asynch)
 * @param pvParameters argument for Task Context
 */
void meas_Task(void* pvParameters);

#endif // MEAS_TASK_HPP
