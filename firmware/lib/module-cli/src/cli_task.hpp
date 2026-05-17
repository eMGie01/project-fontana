#pragma once

#include "cli.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct cli_TaskContextTypeDef 
{
    Cli* cli;
    QueueHandle_t eventQueue;
} cli_TaskContextTypeDef;

void cli_Task(void* arg);
