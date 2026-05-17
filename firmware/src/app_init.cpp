/**
 * @file app_init.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "app.hpp"
#include "uart.h"
#include "uart_stream.hpp"
#include "cli_task.hpp"
#include "taskMeas.hpp"
#include "measurement.hpp"
#include "cli_meas_handlers.hpp"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define HX711_SCK   GPIO_NUM_3
#define HX711_DOUT  GPIO_NUM_2
#define UART_RX     GPIO_NUM_17
#define UART_TX     GPIO_NUM_16

extern void cli_Task(void* arg);

// Variables
static const char * tag = "APP_INIT";

static uart_fd_t s_UartFd = UART_FD_INVALID;
static UartStream s_UartStream(s_UartFd);
static QueueHandle_t s_UartEventQueue = nullptr;
static Cli s_cli(s_UartStream);

static hx711_TypeDef s_hx711 = {};
static QueueHandle_t s_MeasEventQueue = nullptr;
static Meas s_meas;

static cli_TaskContextTypeDef s_cliTaskContext = {
    .cli = &s_cli,
    .eventQueue = s_UartEventQueue,
};

static meas_TaskContextTypeDef s_measTaskContext = {
    .hx711 = &s_hx711,
    .meas = &s_meas,
    .eventQueue = &s_MeasEventQueue,
};

init_status_t
app_init()
{
    const uart_open_cfg_t uartConfig = {
        .port = UART_NUM_0,
        .io_tx = UART_TX,
        .io_rx = UART_RX,
        .baud = 115200,
        .tx_buf_size = 0,
        .rx_buf_size = 256,
    };

    s_UartFd = uart_open(&uartConfig);
    if (s_UartFd == UART_FD_INVALID)
    {
        ESP_LOGE(tag, "UART open failed");
        return INIT_RESTART;
    }

    if (s_cli.RegisterCommand(&CLI_MEAS_COMMAND) != 0)
    {
        ESP_LOGE(tag, "cli meas command registration failed");
        return INIT_RESTART;
    }

    if (uart_ioctl(s_UartFd, UART_IOCTL_GET_EVQUEUE, &s_UartEventQueue) != 0)
    {
        ESP_LOGE(tag, "failed to get uart event queue");
        return INIT_RESTART;
    }
    s_cliTaskContext.eventQueue = s_UartEventQueue;

    BaseType_t cliTaskErr = xTaskCreate(cli_Task, "CLI_TASK", 3072, (void*)&s_cliTaskContext, 5, nullptr);
    if (cliTaskErr != pdTRUE)
    {
        ESP_LOGE(tag, "CLI task creation failed");
        return INIT_RESTART;
    }

    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(tag, "GPIO ISR service install failed: %s", esp_err_to_name(err));
        return INIT_RESTART;
    }

    hx711_StatusTypeDef hx711State = hx711_Open(
        &s_hx711, HX711_SCK, HX711_DOUT, HX711_MODE_A128, 0, meas_DataReadyCallback, NULL);
    if (hx711State != HX711_ERR_OK)
    {
        ESP_LOGE(tag, "hx711 open failed: %d", hx711State);
        return INIT_RESTART;
    }

    s_MeasEventQueue = xQueueCreate(8, sizeof(meas_TaskEventTypeDef));
    if (s_MeasEventQueue == NULL)
    {
        ESP_LOGE(tag, "MeasEventQueue creation failed");
        return INIT_RESTART;
    }

    BaseType_t measTaskErr = xTaskCreate(meas_Task, "MEAS_TASK", 3072, (void *)&s_measTaskContext, 7, nullptr);
    if (measTaskErr != pdTRUE)
    {
        ESP_LOGE(tag, "measTask creation failed");
        return INIT_RESTART;
    }

    err = gpio_intr_enable(s_hx711.ioDout);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "HX711 interrupt enable failed: %s", esp_err_to_name(err));
        return INIT_RESTART;
    }
    



    //
    return INIT_DONE;
}
