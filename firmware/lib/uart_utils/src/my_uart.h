/**
 * @file my_uart.h
 * @author Marek Gałeczka (eMGie01)
 * @brief Lightweight UART wrapper with callback-based receive handling and task support.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef MY_UART_H
#define MY_UART_H

#include <stdbool.h>
#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"

#ifdef __cplusplus
extern "C" {
#endif


// Defines
#define MAX_EVENT_BUFF_SIZE 256

// Enums
typedef enum
{
    UART_UNEXPECTED_ERR = -1,
    UART_OK = 0,
    UART_INVALID_ARG,
    UART_INVALID_CONFIG,
    UART_RUNTIME_ERR,
    UART_NOT_INITIALIZED,
    UART_TIMEOUT,
    UART_HW_ERROR,
    UART_NOT_READY,
    UART_INVALID_RX_BUFFER_SIZE,
    UART_INVALID_TX_BUFFER_SIZE,
    UART_TASK_RUNNING
} uart_err_t;

// Structs
typedef struct
{
    int tx;
    int rx;
} uart_hw_t;

typedef struct 
{
    const char * name;
    size_t queue_size;
    int    tx_buffer_size;
    int    rx_buffer_size;
} uart_set_t;

typedef struct
{
    QueueHandle_t queue;
    TaskHandle_t  task;
} uart_handles_t;

typedef void (*uart_rx_cb_t)(void * ctx, const char * data, size_t size);

typedef struct 
{
    uart_port_t port;
    uart_config_t cfg;
    uart_hw_t ios;
    uart_set_t settings;
    uart_rx_cb_t callback;
    void * ctx;
} my_uart_config_t;

typedef struct
{
    uart_handles_t handles;
    bool initialized;
} my_uart_runtime_t;

typedef struct
{
    my_uart_config_t config;
    my_uart_runtime_t runtime;
} my_uart_t;


// Functions

/**
 * @brief Initializes the UART peripheral and driver for the provided device descriptor.
 * 
 * @param dev UART device descriptor to initialize.
 * @return uart_err_t Result of the operation.
 */
uart_err_t uart_init(my_uart_t * dev);

/**
 * @brief Stops the UART runtime and removes the installed driver.
 * 
 * @param dev UART device descriptor to deinitialize.
 * @return uart_err_t Result of the operation.
 */
uart_err_t uart_deinit(my_uart_t * dev);

/**
 * @brief Starts the UART event task responsible for receive processing.
 * 
 * @param dev UART device descriptor.
 * @return uart_err_t Result of the operation.
 */
uart_err_t uart_start_task(my_uart_t * dev);

/**
 * @brief Stops the UART event task if it is running.
 * 
 * @param dev UART device descriptor.
 * @return uart_err_t Result of the operation.
 */
uart_err_t uart_end_task(my_uart_t * dev);

/**
 * @brief Registers the receive callback and user context for the UART device.
 * 
 * @param dev UART device descriptor.
 * @param cb Callback invoked for received data.
 * @param ctx User context passed to the callback.
 * @return uart_err_t Result of the operation.
 */
uart_err_t uart_set_callback(my_uart_t * dev, uart_rx_cb_t cb, void * ctx);

/**
 * @brief Creates a UART device descriptor filled with default configuration values.
 * 
 * @param cb Default receive callback stored in the descriptor.
 * @return my_uart_t Default-initialized UART device descriptor.
 */
my_uart_t  uart_default_dev(uart_rx_cb_t cb);


#ifdef __cplusplus
}
#endif

#endif /*UART_H*/
