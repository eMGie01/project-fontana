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
    CLI_
};


class CLI
{

public:

    CLI(my_uart_t& uart, Context& ctx);

    void push(const char * data, size_t len);
    void process();

private:

    void   resetLine_();
    void   executeLine_();
    char * stripLine_();
    size_t tokenizeLine_(char ** tokens, char * line, size_t max_count);
    void   dispatchModule_(char ** token, size_t count);
    void   printHelp_();
    void   printErr_(const char * err);

private:

    my_uart_t& uart_;
    Context& ctx_;
    static constexpr size_t RX_LINE_MAX = 128;
    char rx_line_[RX_LINE_MAX];
    size_t rx_len_;
    bool overflow_;
    bool overflow_error_;
    bool echo_;
    bool cmd_ready_;

};


#endif /*CLI_INTERFACE_HPP*/