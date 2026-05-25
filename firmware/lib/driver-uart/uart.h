/**
 * @file uart.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// file descriptor
typedef int uart_fd_t;
#define UART_FD_INVALID (-1)

// error codes ( -errno )
#define UART_ERR_INVAL   (-1)   // invalid argument
#define UART_ERR_NODEV   (-2)   // file descriptor does not exist
#define UART_ERR_BUSY    (-3)   // device is busy
#define UART_ERR_IO      (-4)   // hardware failure
#define UART_ERR_TIMEOUT (-5)   // timeout on read

// config to open device
typedef struct
{
    int port;               // UART0, UART1 or UART2 ...
    int io_tx;
    int io_rx;
    uint32_t baud;
    size_t tx_buf_size;     // ring buffer tx (blocking if 0)
    size_t rx_buf_size;     // ring buffer rx (min: 256)

} uart_open_cfg_t;

// ioctl requests
#define UART_IOCTL_FLUSH_RX    1 // flush the rx buffer
#define UART_IOCTL_SET_BAUD    2 // uint32_t*
#define UART_IOCTL_GET_BAUD    3 // uint32_t*
#define UART_IOCTL_SET_TIMEOUT 4 // uint32_t* [ms]
#define UART_IOCTL_GET_EVQUEUE 5 // void**

// api
uart_fd_t uart_open   (const uart_open_cfg_t * cfg);
int       uart_close  (uart_fd_t fd);
int       uart_read   (uart_fd_t fd, void * buf, size_t count);
int       uart_write  (uart_fd_t fd, const void * buf, size_t count);
int       uart_ioctl  (uart_fd_t fd, int request, void * arg);

#ifdef __cplusplus
}
#endif

#endif // UART_H
