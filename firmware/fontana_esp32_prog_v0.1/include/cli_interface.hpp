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


class CLI
{

public:

    CLI(my_uart_t& uart, Context& ctx);
    void push(const char * data, size_t len);
    void process();

private:

    void resetLine();
    void executeLine();
    void printHelp();

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