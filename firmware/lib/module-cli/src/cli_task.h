/**
 * @file cli_task.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-05-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CLI_TASK_H
#define CLI_TASK_H

#include "esp_err.h"
#include "portmacro.h"

typedef struct
{
    uint32_t stackSize;
    UBaseType_t priority;
} cli_Config_t;

esp_err_t cli_TaskInit(cli_Config_t* cfg);
esp_err_t cli_TaskStart();
esp_err_t cli_TaskStop();

esp_err_t cli_MeasRegister();

#endif // CLI_TASK_H