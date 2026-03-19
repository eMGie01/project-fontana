#ifndef UART_H
#define UART_H

#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 
 * 
 */
typedef void (*uart_rx_callback_t)(const char *data, size_t size);


/**
 * @brief Uart error types
 * 
 */
typedef enum {
  UART_OK = 0,
  UART_TIMEOUT,
  UART_ERR_INVALID_PARAM,
  UART_NOT_INITIALIZED,
  UART_RX_ERROR,
  UART_TX_SEND_ERR,
  UART_UNIDENTIFIED_ERROR
} uart_err_t;

/**
 * @brief Uart structure
 * 
 */
typedef struct {
  uart_port_t port;
  uart_config_t uart_cfg;
  int gpio_tx;
  int gpio_rx;
  int tx_buffer_size;
  int rx_buffer_size;
  int queue_size;
  uart_rx_callback_t callback;
} uart_t;

/**
 * @brief 
 * 
 * @param port 
 * @param baud_rate 
 * @param tx_pin 
 * @param rx_pin 
 * @return uart_err_t 
 */
uart_err_t uart_init(uart_t * cfg);

/**
 * @brief 
 * 
 */
void uart_start_task(void);

/**
 * @brief 
 * 
 * @param str 
 * @return uart_err_t 
 */
uart_err_t uart_send_string(const char * str, size_t size);



#ifdef __cplusplus
}
#endif

#endif /*UART_H*/