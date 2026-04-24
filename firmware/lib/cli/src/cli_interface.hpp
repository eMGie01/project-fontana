/**
 * @file cli_interface.hpp
 * @author Marek Gałeczka
 * @brief Text-based CLI interface used to read and modify runtime parameters.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef CLI_INTERFACE_HPP
#define CLI_INTERFACE_HPP

#include "measurement.hpp"
#include "hx711.h"
#include "my_uart.h"
#include "snapshot.h"
#include <cstddef>


static constexpr size_t CLI_RX_LINE_MAX = 256;


/**
 * @brief Set of pointers and handles shared with CLI modules.
 */
struct cli_ctx_t
{
    hx711_t * hx711;
    Measurement * meas;
    snapshot_t * snap;
    SemaphoreHandle_t hx711_mtx;
    SemaphoreHandle_t meas_mtx;
};


/**
 * @brief Error codes returned by the CLI layer.
 */
enum cli_err_t 
{
    CLI_UNEXPECTED_ERR = -1,
    CLI_OK = 0,
    CLI_OVERFLOW_ERR,
    CLI_EMPTY_LINE_ERR,
    CLI_TOO_MANY_ARGS_ERR,
    CLI_INVALID_ARG_ERR,
    CLI_QUEUE_NOT_INIT_ERR,
    CLI_SEND_RES_ERR,
    CLI_MODULE_ERR,
    CLI_TOO_LITTLE_TOKENS,
    CLI_QUEUE_OVERFLOW,
    CLI_MUTEX_ERR
};

/**
 * @brief CLI command parser and dispatcher operating on shared application modules.
 */
class CLI
{

public:

    /**
     * @brief Construct a new CLI object
     * 
     * @param uart Reference to the configured UART interface used by the CLI.
     * @param ctx Reference to the shared context used by CLI handlers.
     */
    CLI(my_uart_t& uart, cli_ctx_t& ctx);

    /**
     * @brief Replaces the receive queue used to forward complete lines to the CLI task.
     * 
     * @param queue Queue handle that receives assembled command lines.
     */
    void updateQueue(QueueHandle_t queue);

    /**
     * @brief Updates the shared context content used by command handlers.
     * 
     * @param ctx New CLI context content.
     */
    void updateContext(cli_ctx_t& ctx);

    /**
     * @brief Receives raw UART data and assembles it into complete command lines.
     * 
     * @param data Pointer to the received bytes.
     * @param len Number of bytes in the input buffer.
     */
    void push(const char * data, size_t len);

    /**
     * @brief Parses and executes a single CLI command line.
     * 
     * @param buff Buffer containing a null-terminated command line.
     */
    void process(char * buff);

    /**
     * @brief Sends text data through the UART associated with the CLI.
     * 
     * @param data Pointer to the data to send.
     * @param len Number of bytes to send.
     */
    void printOut(const char * data, size_t len);

private:

    /**
     * @brief Clears the state of the currently assembled input line.
     */
    void   resetLine_();

    /**
     * @brief Splits an input line into whitespace-separated tokens.
     * 
     * @param tokens Output array for pointers to parsed tokens.
     * @param line Input buffer modified in place during tokenization.
     * @param max_count Maximum number of tokens that can be stored.
     * @return size_t Number of parsed tokens or a value greater than the limit.
     */
    size_t tokenizeLine_(char ** tokens, char * line, size_t max_count);

    /**
     * @brief Selects the module responsible for handling the parsed command.
     * 
     * @param token Token array describing the command.
     * @param count Number of available tokens.
     */
    void   dispatchModule_(char ** token, size_t count);

    /**
     * @brief Sends the list of available commands over UART.
     */
    void   printHelp_();

    /**
     * @brief Formats and sends a CLI error message.
     * 
     * @param mod Name of the module reporting the error.
     * @param err Error code.
     * @param msg Short text description of the error.
     */
    void   printErr_(const char * mod, const int err, const char * msg);

private:

    my_uart_t& uart_;
    cli_ctx_t& ctx_;
    QueueHandle_t queue_;

    size_t rx_len_;
    bool overflow_;
    bool echo_;

    char rx_line_[CLI_RX_LINE_MAX];
};

#endif /*CLI_INTERFACE_HPP*/
