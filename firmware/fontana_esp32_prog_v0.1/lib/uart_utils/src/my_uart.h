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
#define MAX_EVENT_BUFF_SIZE 128

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
uart_err_t uart_init(my_uart_t * dev);
uart_err_t uart_deinit(my_uart_t * dev);
uart_err_t uart_start_task(my_uart_t * dev);
uart_err_t uart_end_task(my_uart_t * dev);
uart_err_t uart_set_callback(my_uart_t * dev, uart_rx_cb_t cb, void * ctx);
my_uart_t  uart_default_dev(uart_rx_cb_t cb);


#ifdef __cplusplus
}
#endif

#endif /*UART_H*/
