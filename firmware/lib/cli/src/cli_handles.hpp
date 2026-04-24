#ifndef CLI_HANDLES_HPP
#define CLI_HANDLES_HPP

#include "cli_interface.hpp"
#include "my_uart.h"
#include "hx711.h"

cli_err_t hx711_handle(char ** tokens, size_t count, cli_ctx_t& ctx, my_uart_t& uart);
cli_err_t meas_handle(char ** tokens, size_t count, cli_ctx_t& ctx, my_uart_t& uart);

#endif // CLI_HANDLES_HPP