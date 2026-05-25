/**
 * @file uart.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief Small file-descriptor style UART driver wrapper for ESP-IDF.
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 * This module provides a compact C API around the ESP-IDF UART driver. It maps
 * opened UART peripherals to small integer descriptors and exposes basic
 * open/close/read/write/ioctl operations.
 *
 * The driver owns the ESP-IDF UART driver installation for each opened port.
 * It supports UART0..UART2, 8 data bits, no parity, 1 stop bit and no hardware
 * flow control.
 */

#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UART device descriptor type.
 *
 * A descriptor is returned by uart_open() and must be passed to uart_close(),
 * uart_read(), uart_write() and uart_ioctl().
 */
typedef int uart_fd_t;

/**
 * @brief Invalid UART descriptor value.
 *
 * Returned by uart_open() when the device cannot be opened.
 */
#define UART_FD_INVALID (-1)

/**
 * @brief Invalid argument.
 */
#define UART_ERR_INVAL   (-1)

/**
 * @brief Descriptor does not refer to an opened device.
 */
#define UART_ERR_NODEV   (-2)

/**
 * @brief Device is busy.
 *
 * Reserved for API consistency. uart_open() currently reports open failures by
 * returning UART_FD_INVALID.
 */
#define UART_ERR_BUSY    (-3)

/**
 * @brief Hardware or ESP-IDF UART driver failure.
 */
#define UART_ERR_IO      (-4)

/**
 * @brief Read timeout expired before any byte was received.
 */
#define UART_ERR_TIMEOUT (-5)

/**
 * @brief UART open configuration.
 */
typedef struct
{
    /** ESP-IDF UART port number. Supported range is UART0..UART2. */
    int port;

    /** TX GPIO number. */
    int io_tx;

    /** RX GPIO number. */
    int io_rx;

    /** Baud rate configured during open. */
    uint32_t baud;

    /** TX ring buffer size in bytes. Use 0 for blocking TX without a TX buffer. */
    size_t tx_buf_size;

    /** RX ring buffer size in bytes. ESP-IDF requires enough space for the UART driver. */
    size_t rx_buf_size;

} uart_open_cfg_t;

/**
 * @brief Flush the RX input buffer.
 *
 * Argument: ignored and may be NULL.
 */
#define UART_IOCTL_FLUSH_RX    1

/**
 * @brief Set UART baud rate.
 *
 * Argument type: uint32_t*.
 */
#define UART_IOCTL_SET_BAUD    2

/**
 * @brief Get UART baud rate.
 *
 * Argument type: uint32_t*.
 */
#define UART_IOCTL_GET_BAUD    3

/**
 * @brief Set read timeout in milliseconds.
 *
 * Argument type: uint32_t*.
 */
#define UART_IOCTL_SET_TIMEOUT 4

/**
 * @brief Get ESP-IDF UART event queue handle.
 *
 * Argument type: void**. The returned pointer is the queue handle created by
 * uart_driver_install().
 */
#define UART_IOCTL_GET_EVQUEUE 5

/**
 * @brief Open and configure a UART port.
 *
 * The function allocates one internal descriptor slot, configures the selected
 * UART port and installs the ESP-IDF UART driver with an event queue depth of 8.
 * Only one descriptor may own a given hardware UART port at a time.
 *
 * On success, the read timeout is initialized to 1000 ms.
 *
 * @param cfg Open configuration.
 *
 * @return UART descriptor on success.
 * @return UART_FD_INVALID if cfg is NULL, the port is invalid, the port is
 * already open, no descriptor slot is available or ESP-IDF UART setup fails.
 */
uart_fd_t uart_open   (const uart_open_cfg_t * cfg);

/**
 * @brief Close a previously opened UART descriptor.
 *
 * Deletes the ESP-IDF UART driver for the owned port and releases the internal
 * descriptor slot.
 *
 * @param fd UART descriptor returned by uart_open().
 *
 * @return 0 on success.
 * @return UART_ERR_NODEV if fd is invalid or not open.
 */
int       uart_close  (uart_fd_t fd);

/**
 * @brief Read bytes from a UART descriptor.
 *
 * The call waits up to the configured read timeout. The timeout can be changed
 * with UART_IOCTL_SET_TIMEOUT. If no byte is received before the timeout expires,
 * UART_ERR_TIMEOUT is returned.
 *
 * @param fd UART descriptor returned by uart_open().
 * @param buf Destination buffer.
 * @param count Maximum number of bytes to read.
 *
 * @return Positive number of bytes read.
 * @return 0 if count is 0.
 * @return UART_ERR_NODEV if fd is invalid or not open.
 * @return UART_ERR_INVAL if buf is NULL.
 * @return UART_ERR_TIMEOUT if no byte is received before timeout.
 * @return UART_ERR_IO if the ESP-IDF read operation fails.
 */
int       uart_read   (uart_fd_t fd, void * buf, size_t count);

/**
 * @brief Write bytes to a UART descriptor.
 *
 * @param fd UART descriptor returned by uart_open().
 * @param buf Source buffer.
 * @param count Number of bytes to write.
 *
 * @return Positive number of bytes written.
 * @return 0 if count is 0.
 * @return UART_ERR_NODEV if fd is invalid or not open.
 * @return UART_ERR_INVAL if buf is NULL.
 * @return UART_ERR_IO if the ESP-IDF write operation fails.
 */
int       uart_write  (uart_fd_t fd, const void * buf, size_t count);

/**
 * @brief Execute a runtime UART control request.
 *
 * Supported requests:
 * - UART_IOCTL_FLUSH_RX: flush RX input buffer, arg may be NULL.
 * - UART_IOCTL_SET_BAUD: arg points to uint32_t baud rate.
 * - UART_IOCTL_GET_BAUD: arg points to uint32_t output storage.
 * - UART_IOCTL_SET_TIMEOUT: arg points to uint32_t timeout in milliseconds.
 * - UART_IOCTL_GET_EVQUEUE: arg points to void* output storage.
 *
 * @param fd UART descriptor returned by uart_open().
 * @param request Ioctl request selector.
 * @param arg Request-specific input or output pointer.
 *
 * @return 0 on success.
 * @return UART_ERR_NODEV if fd is invalid or not open.
 * @return UART_ERR_INVAL if request is unknown or arg is required but NULL.
 * @return UART_ERR_IO if the ESP-IDF control operation fails.
 */
int       uart_ioctl  (uart_fd_t fd, int request, void * arg);

#ifdef __cplusplus
}
#endif

#endif // UART_H
