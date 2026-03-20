#ifndef MY_UART_H
#define MY_UART_H

#include <stdbool.h>
#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"

#ifdef __cplusplus
extern "C" {
#endif


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
    UART_NOT_READY
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

typedef void (*uart_rx_cb_t)(const char * data, size_t size);

typedef struct 
{
    uart_port_t port;
    uart_config_t cfg;
    uart_hw_t ios;
    uart_set_t settings;
    uart_handles_t handles;
    uart_rx_cb_t callback;
    bool initialized;
} my_uart_t;


// Functions
uart_err_t uart_init(my_uart_t * dev);


#ifdef __cplusplus
}
#endif

#endif /*UART_H*/
