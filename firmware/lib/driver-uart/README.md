# UART Driver

Small file-descriptor style UART wrapper for ESP-IDF. The driver maps opened
UART peripherals to integer descriptors and exposes a compact C API:
`open`, `close`, `read`, `write` and `ioctl`.

It owns the ESP-IDF UART driver installation for each opened port and provides a
thin application-facing interface with consistent negative error codes.

## Features

- Descriptor-based API using `uart_fd_t`.
- Supports UART0..UART2.
- Fixed UART frame format: 8 data bits, no parity, 1 stop bit.
- No hardware flow control.
- Configurable TX and RX driver buffers.
- Blocking reads with configurable timeout.
- Basic runtime control through `uart_ioctl()`.
- Access to the ESP-IDF UART event queue created by `uart_driver_install()`.

## Files

- `uart.h` - public API, configuration structure, error codes and ioctl requests.
- `uart.c` - descriptor table, ESP-IDF UART setup and API implementation.

## Requirements

- ESP-IDF UART driver.
- ESP-IDF GPIO driver.
- FreeRTOS queue types through the ESP-IDF UART driver.
- A project-level build system entry that compiles `uart.c` and exposes this
  directory as an include path.

## Public API

```c
uart_fd_t uart_open(const uart_open_cfg_t* cfg);
int uart_close(uart_fd_t fd);
int uart_read(uart_fd_t fd, void* buf, size_t count);
int uart_write(uart_fd_t fd, const void* buf, size_t count);
int uart_ioctl(uart_fd_t fd, int request, void* arg);
```

The API follows a small POSIX-like shape:

- `uart_open()` returns a descriptor or `UART_FD_INVALID`.
- `uart_close()`, `uart_read()`, `uart_write()` and `uart_ioctl()` use that
  descriptor.
- Runtime errors are returned as negative `UART_ERR_*` values.
- Successful reads and writes return the number of bytes transferred.

## Opening a Port

Use `uart_open_cfg_t` to describe the hardware port, GPIO pins, baud rate and
driver buffer sizes.

```c
uart_open_cfg_t cfg = {
    .port = 1,
    .io_tx = 17,
    .io_rx = 16,
    .baud = 115200,
    .tx_buf_size = 0,
    .rx_buf_size = 256,
};

uart_fd_t fd = uart_open(&cfg);
if (fd == UART_FD_INVALID)
{
    /* handle open failure */
}
```

Only one descriptor may own a given UART hardware port at a time. The driver has
three internal descriptor slots, matching UART0..UART2.

On success, the read timeout is initialized to `1000 ms`.

## Reading and Writing

```c
uint8_t rx[64];
int n = uart_read(fd, rx, sizeof(rx));
if (n > 0)
{
    /* n bytes were received */
}

const uint8_t tx[] = "OK\r\n";
int written = uart_write(fd, tx, sizeof(tx) - 1);
```

`uart_read()` waits up to the configured timeout. If no byte is received before
the timeout expires, it returns `UART_ERR_TIMEOUT`.

`uart_write()` returns the number of bytes accepted by the ESP-IDF UART driver.

## Runtime Control

`uart_ioctl()` supports the following requests:

| Request | Argument type | Description |
| --- | --- | --- |
| `UART_IOCTL_FLUSH_RX` | ignored | Flush the RX input buffer. |
| `UART_IOCTL_SET_BAUD` | `uint32_t*` | Set UART baud rate. |
| `UART_IOCTL_GET_BAUD` | `uint32_t*` | Read current UART baud rate. |
| `UART_IOCTL_SET_TIMEOUT` | `uint32_t*` | Set read timeout in milliseconds. |
| `UART_IOCTL_GET_EVQUEUE` | `void**` | Get the ESP-IDF UART event queue handle. |

Example:

```c
uint32_t timeoutMs = 50;
uart_ioctl(fd, UART_IOCTL_SET_TIMEOUT, &timeoutMs);

uint32_t baud = 230400;
uart_ioctl(fd, UART_IOCTL_SET_BAUD, &baud);
```

## Error Codes

| Code | Meaning |
| --- | --- |
| `UART_FD_INVALID` | `uart_open()` failed. |
| `UART_ERR_INVAL` | Invalid argument or unsupported ioctl request. |
| `UART_ERR_NODEV` | Descriptor is invalid or not open. |
| `UART_ERR_BUSY` | Reserved for busy-device errors. |
| `UART_ERR_IO` | ESP-IDF UART operation failed. |
| `UART_ERR_TIMEOUT` | Read timeout expired before any byte was received. |

## Design Boundaries

This module intentionally does not:

- parse protocols or packets,
- own application tasks,
- allocate application message buffers,
- implement framing, retries or checksums,
- expose advanced UART features such as parity selection, stop-bit selection or
  hardware flow control.

Those responsibilities belong to the application or to a higher-level protocol
library built on top of this driver.
