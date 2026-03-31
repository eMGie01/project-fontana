#include "my_uart.h"

#include "soc/gpio_num.h"

my_uart_t 
uart_default_dev(uart_rx_cb_t cb)
{
    return (my_uart_t) {
        .config = (my_uart_config_t) {
            .port = UART_NUM_0,
            .cfg = (uart_config_t) {
                .baud_rate = 115200,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            },
            .ios = (uart_hw_t) {
                .rx = GPIO_NUM_17,
                .tx = GPIO_NUM_16
            },
            .settings = (uart_set_t) {
                .name = "uart_def",
                .queue_size = 16,
                .rx_buffer_size = 256,
                .tx_buffer_size = 256
            },
            .callback = cb,
            .ctx = NULL
        },
        .runtime = (my_uart_runtime_t) {
            .handles = (uart_handles_t) {
                .queue = NULL,
                .task = NULL
            },
            .initialized = false
        },
    };
}
