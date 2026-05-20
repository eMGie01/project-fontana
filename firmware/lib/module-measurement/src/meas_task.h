/**
 * @file meas_task.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-05-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MEAS_TASK_H
#define MEAS_TASK_H

#include "esp_err.h"
#include "portmacro.h"

typedef struct
{
    uint32_t    stackSize;
    UBaseType_t priority;
} meas_TaskConfig_t;

/**
 * @brief Init function for measurement module
 * @param cfg Configuration for Measurement Task
 * @return esp_err_t 
 */
esp_err_t meas_TaskInit(meas_TaskConfig_t* cfg);

/**
 * @brief Start function for measurement module, task starts after successful init
 * @return esp_err_t 
 */
esp_err_t meas_TaskStart();

/**
 * @brief Tasks deletes itself and resets all queues and hadnles, except Meas Class
 * @return esp_err_t 
 */
esp_err_t meas_TaskStop();

#endif // MEAS_TASK_H
