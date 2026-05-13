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
#include "measurementCli.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

#endif // TASKMEAS_HPP
