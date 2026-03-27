#ifndef CLI_INTERFACE_HPP
#define CLI_INTERFACE_HPP

#include "measurements.hpp"
#include "hx711.h"
#include "my_uart.h"


struct Context
{
    hx711_t *hx711;
    Measurement *meas;
};


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
    CLI_MODULE_ERR
};


class CLI
{

public:

    CLI(my_uart_t& uart, Context& ctx, QueueHandle_t queue);

    void push(const char * data, size_t len);
    void process(char * buff);

private:

    void   resetLine_();
    size_t tokenizeLine_(char ** tokens, char * line, size_t max_count);
    void   dispatchModule_(char ** token, size_t count);
    void   printHelp_();
    void   printErr_(const char * mod, const int err, const char * msg);

private:

    my_uart_t& uart_;
    Context& ctx_;
    QueueHandle_t queue_;

    size_t rx_len_;
    bool overflow_;
    bool echo_;

    static constexpr size_t RX_LINE_MAX = 128;
    char rx_line_[RX_LINE_MAX];

};


#endif /*CLI_INTERFACE_HPP*/