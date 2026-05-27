#include "uart.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"

// open ports
#define UART_MAX_DEVS 3 // esp32 has UART0..2

typedef struct
{
    bool     open;          // indicator wheter port is busy or not
    int      port;          // uart_port_t
    uint32_t timeout_ms;    // timeout for read
    QueueHandle_t evqueue;

} uart_dev_t;

static uart_dev_t s_devs[UART_MAX_DEVS];

// helpers
static uart_dev_t *
fd_to_dev_(uart_fd_t fd)
{
    if ( fd < 0 || fd >= UART_MAX_DEVS )
        return NULL;
    if ( !s_devs[fd].open )
        return NULL;
    return &s_devs[fd];
}

// open
uart_fd_t
uart_open(const uart_open_cfg_t * cfg)
{
    if ( !cfg || cfg->port < 0 || cfg->port >= UART_MAX_DEVS )
        goto failfd;

    // check wheter port is already open or not
    for (int i = 0; i < UART_MAX_DEVS; ++i)
    {
        if ( s_devs[i].open && s_devs[i].port == cfg->port )
            goto failfd;
    }

    // find free slot and save device
    int fd = UART_FD_INVALID;
    for (int i = 0; i < UART_MAX_DEVS; ++i)
    {
        if ( !s_devs[i].open )
        {
            fd = i;
            break;
        }
    }

    if ( fd == UART_FD_INVALID )
        goto failfd;

    const uart_config_t ucfg = {
        .baud_rate  = cfg->baud,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    if ( ESP_OK != uart_param_config(cfg->port, &ucfg) )
        goto failfd;

    if ( ESP_OK != uart_set_pin(cfg->port, cfg->io_tx, cfg->io_rx, -1, -1) )
        goto failfd;

    if ( ESP_OK != uart_driver_install(
        cfg->port, 
        cfg->rx_buf_size, 
        cfg->tx_buf_size, 
        8, 
        &s_devs[fd].evqueue, 
        0)
    )
        goto failfd;

    s_devs[fd].open       = true;
    s_devs[fd].port       = cfg->port;
    s_devs[fd].timeout_ms = 1000UL;

    return fd;

failfd:
    return UART_FD_INVALID;
}

// close
int
uart_close(uart_fd_t fd)
{
    uart_dev_t * dev = fd_to_dev_(fd);
    if ( !dev )
        return UART_ERR_NODEV;

    uart_driver_delete(dev->port);
    dev->evqueue = NULL;
    dev->open = false;

    return 0;
}

// read
int
uart_read(uart_fd_t fd, void * buf, size_t count)
{
    uart_dev_t * dev = fd_to_dev_(fd);
    if ( !dev )
        return UART_ERR_NODEV;
    if ( !buf )
        return UART_ERR_INVAL;
    if ( count == 0 )
        return 0;

    TickType_t ticks = pdMS_TO_TICKS(dev->timeout_ms);
    int n = uart_read_bytes(dev->port, buf, count, ticks);
    if ( n < 0 )
        return UART_ERR_IO;
    if ( n == 0 )
        return UART_ERR_TIMEOUT;

    return n;
}

// write
int
uart_write(uart_fd_t fd, const void * buf, size_t count)
{
    uart_dev_t * dev = fd_to_dev_(fd);
    if ( !dev )
        return UART_ERR_NODEV;
    if ( !buf )
        return UART_ERR_INVAL;
    if ( count == 0 )
        return 0;
    
    int n = uart_write_bytes(dev->port, buf, count);
    if ( n < 0 )
        return UART_ERR_IO;
    
    return n;
}

// ioctl
int
uart_ioctl(uart_fd_t fd, int request, void * arg)
{
    uart_dev_t * dev = fd_to_dev_(fd);
    if ( !dev )
        return UART_ERR_NODEV;
    
    switch ( request )
    {
    
    case UART_IOCTL_FLUSH_RX:

        uart_flush_input(dev->port);
        return 0;

    case UART_IOCTL_SET_BAUD:

        if ( !arg )
            return UART_ERR_INVAL;
        return ( ESP_OK != uart_set_baudrate(dev->port, *(uint32_t *)arg) ) ? UART_ERR_IO : 0;

    case UART_IOCTL_GET_BAUD:

        if ( !arg )
            return UART_ERR_INVAL;
        uint32_t baud;
        if ( ESP_OK != uart_get_baudrate(dev->port, &baud) )
            return UART_ERR_IO;
        *(uint32_t *)arg = baud;
        return 0;

    case UART_IOCTL_SET_TIMEOUT:

        if ( !arg )
            return UART_ERR_INVAL;
        dev->timeout_ms = *(uint32_t *)arg;
        return 0;

    case UART_IOCTL_GET_EVQUEUE:

        if ( !arg )
            return UART_ERR_INVAL;
        *(void **)arg = dev->evqueue;
        return 0;

    default:
        return UART_ERR_INVAL;
    }
}
