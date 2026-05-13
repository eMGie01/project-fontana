/**
 * @file taskMeas.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "taskMeas.hpp"

static QueueHandle_t s_MeasEventQueue = nullptr;

static void IRAM_ATTR
MEAS_Hx711DataReadyCallback(void* arg)
{
    BaseType_t hpTaskWoken = pdFALSE;
    MEAS_TaskEvent event = {};
    event.type = MEAS_TASK_EVENT_HX711_READY;

    xQueueSendFromISR(s_MeasEventQueue, &event, &hpTaskWoken);
    
    if (hpTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

void 
taskMeas(void* pvParameters)
{
    // 
}