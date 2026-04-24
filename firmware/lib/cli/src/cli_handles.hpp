/**
 * @file cli_handles.hpp
 * @author Marek Gałeczka (eMGie01)
 * @brief Declarations of CLI command handlers for application modules.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef CLI_HANDLES_HPP
#define CLI_HANDLES_HPP

#include "cli_interface.hpp"
#include "my_uart.h"
#include "hx711.h"

/**
 * @brief Handles CLI commands addressed to the HX711 module.
 * 
 * @param tokens Token array describing the command.
 * @param count Number of tokens in the command.
 * @param ctx Shared resource context used by the handler.
 * @param uart UART interface used for text responses.
 * @return cli_err_t Command handling result code.
 */
cli_err_t hx711_handle(char ** tokens, size_t count, cli_ctx_t& ctx, my_uart_t& uart);

/**
 * @brief Handles CLI commands addressed to the measurement module.
 * 
 * @param tokens Token array describing the command.
 * @param count Number of tokens in the command.
 * @param ctx Shared resource context used by the handler.
 * @param uart UART interface used for text responses.
 * @return cli_err_t Command handling result code.
 */
cli_err_t meas_handle(char ** tokens, size_t count, cli_ctx_t& ctx, my_uart_t& uart);

#endif // CLI_HANDLES_HPP
