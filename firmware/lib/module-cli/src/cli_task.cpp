/**
 * @file cli_task.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-05-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "cli_task.h"
#include "cli.hpp"
#include "cli_api.h"
#include "uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include <cstring>

static constexpr uart_port_t UART_PORT      = UART_NUM_0;
static constexpr gpio_num_t  UART_TX        = GPIO_NUM_16;
static constexpr gpio_num_t  UART_RX        = GPIO_NUM_17;
static constexpr size_t      TX_BUFF_SIZE   = 0;
static constexpr size_t      RX_BUFF_SZE    = 256;

static cli_Config_t     s_Config = {};
static TaskHandle_t     s_TaskHandle = nullptr;
static bool             s_TaskInitialized = false;

static uart_fd_t        s_UartFd = UART_FD_INVALID;
static Cli              s_Cli;
static QueueHandle_t    s_DataEventQueue = nullptr;

esp_err_t 
cli_RegisterCommand(const cli_Command_t* entry)
{
    if (s_TaskHandle != nullptr)
    {
        return ESP_FAIL;
    }
    return s_Cli.RegisterCommand(entry);
}

static void
cli_DataHandle(size_t bytesAvailable)
{
    char rxBuf[64];
    char response[128];

    while (bytesAvailable > 0)
    {
        size_t bytesToRead = (bytesAvailable < sizeof(rxBuf)) ? bytesAvailable : sizeof(rxBuf);
        int bytesRead = uart_read(s_UartFd, rxBuf, bytesToRead);
        if (bytesRead <= 0)
        {
            return;
        }

        bytesAvailable -= bytesRead;

        for (int i = 0; i < bytesRead; ++i)
        {
            s_Cli.Push(rxBuf[i]);

            if (!s_Cli.HasLine())
            {
                continue;
            }

            response[0] = '\0';
            if (s_Cli.Execute(response, sizeof(response)) == ESP_OK && response[0] != '\0')
            {
                uart_write(s_UartFd, response, std::strlen(response));
            }
        }
    }
}

static void
cli_TaskRun(void* pvParameters)
{
    // adding queues to one Set
    uart_event_t uartEvent;
    for (;;)
    {
        if (xQueueReceive(s_DataEventQueue, &uartEvent, portMAX_DELAY) == pdTRUE)
        {
            if (uartEvent.type != UART_DATA)
            {
                uart_ioctl(s_UartFd, UART_IOCTL_FLUSH_RX, nullptr);
                xQueueReset(s_DataEventQueue);
                continue;
            }
            cli_DataHandle(uartEvent.size);
        }
    }
}

esp_err_t
cli_TaskInit(cli_Config_t* cfg)
{
    if (cfg == nullptr)
    {
        // log
        return ESP_ERR_INVALID_ARG;
    }

    if (s_TaskInitialized != false)
    {
        // log task already initialized
        return ESP_FAIL;
    }

    s_Config = *cfg;

    const uart_open_cfg_t uartCfg = {
        .port = UART_PORT,
        .io_tx = UART_TX,
        .io_rx = UART_RX,
        .baud = 115200,
        .tx_buf_size = TX_BUFF_SIZE,
        .rx_buf_size = RX_BUFF_SZE,
    };

    // Initializing connection with UART
    s_UartFd = uart_open(&uartCfg);
    if (s_UartFd == UART_FD_INVALID)
    {
        // log
        return ESP_FAIL;
    }

    // get access to UARTs EventQueue
    if (uart_ioctl(s_UartFd, UART_IOCTL_GET_EVQUEUE, &s_DataEventQueue) != 0)
    {
        // log
        return ESP_FAIL;
    }

    s_TaskInitialized = true;
    return ESP_OK;
}

esp_err_t
cli_TaskStart()
{
    if (s_TaskHandle != nullptr)
    {
        // log
        return ESP_FAIL;
    }

    BaseType_t taskRes = xTaskCreate(cli_TaskRun, "CLI_TASK", s_Config.stackSize, nullptr, s_Config.priority, &s_TaskHandle);
    if (taskRes != pdTRUE)
    {
        // log
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t
cli_TaskStop()
{
    if (s_TaskHandle == nullptr)
    {
        return ESP_OK;
    }

    vTaskDelete(s_TaskHandle);
    s_TaskHandle = nullptr;
    xQueueReset(s_DataEventQueue);
    s_TaskInitialized = false;
    return ESP_OK;
}
