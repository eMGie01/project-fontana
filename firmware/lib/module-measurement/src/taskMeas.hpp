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

typedef enum MEAS_TaskEventType
{
    MEAS_TASK_EVENT_HX711_READY = 1,
    MEAS_TASK_EVENT_CMD = 2,

} MEAS_TaskEventType;

typedef struct MEAS_TaskEvent
{
    MEAS_TaskEventType type;
    union 
    {
        MEAS_TaskCmd cmd;
    } data;

} MEAS_TaskEvent;

typedef struct MEAS_TaskContext
{
    hx711_TypeDef* hx711;
    Meas* meas;
    QueueHandle_t* eventQueue; 
} MEAS_TaskContext;

void MEAS_Hx711DataReadyCallback(void* arg);
void measTask(void* pvParameters);

#endif // TASKMEAS_HPP
