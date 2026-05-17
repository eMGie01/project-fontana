/**
 * @file taskMeas.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef TASKMEAS_HPP
#define TASKMEAS_HPP

#include "measurement.hpp"
#include "meas_task_api.h"
#include "hx711.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef enum meas_TaskEventTypeTypeDef
{
    MEAS_TASK_EVENT_HX711_READY = 1,
    MEAS_TASK_EVENT_CMD = 2,

} meas_TaskEventTypeTypeDef;

typedef struct meas_TaskEventTypeDef
{
    meas_TaskEventTypeTypeDef type;
    union 
    {
        meas_TaskCmdTypeDef cmd;
    } data;

} meas_TaskEventTypeDef;

typedef struct meas_TaskContextTypeDef
{
    hx711_HandleTypeDef hx711;
    Meas* meas;
    QueueHandle_t* eventQueue; 
} meas_TaskContextTypeDef;

void meas_DataReadyCallback(void* arg);
void meas_Task(void* pvParameters);

#endif // TASKMEAS_HPP
