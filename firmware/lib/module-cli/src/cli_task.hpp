/**
 * @file cli_task.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "cli.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct cli_TaskContextTypeDef 
{
    Cli* cli;
    QueueHandle_t* eventQueue;
} cli_TaskContextTypeDef;

void cli_Task(void* arg);

// end of CLI_TASK_HPP
