/**
 * @file cli_task.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "cli_meas_handlers.hpp"
#include "uart_stream.hpp"

#include "taskMeas.hpp"
#include "measurement.hpp"

#include "uart.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char * tag = "TASK_CLI";

void cli_Task(void* arg)
{
    cli_TaskContextTypeDef* ctx = static_cast<cli_TaskContextTypeDef*>(arg);
    if (ctx == nullptr || ctx->cli == nullptr || ctx->eventQueue == nullptr)
    {
        ESP_LOGE(tag, "invalid arguments in task cli");
        vTaskDelete(nullptr);
        return;
    }

    Cli* cli = ctx->cli;
    QueueHandle_t uartEventQueue = ctx->eventQueue;

    uart_event_t event;

    for (;;)
    {
        if (xQueueReceive(uartEventQueue, &event, portMAX_DELAY))
        {
            if (event.type == UART_DATA)
            {
                cli->Read(event.size);
            }
        }
    }
}
