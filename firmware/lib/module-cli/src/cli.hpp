/**
 * @file cli.h
 * @author Marek Gałeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CLI_HPP
#define CLI_HPP

#include "uart_stream.hpp"

#define CLI_MAX_LINE 64

class CLI
{
public: 
    CLI(IoStream& stream/*, modules */)
        : stream_(stream)
        , pos_(0)
    {}

    // fcn
    void push(const char c);
    // void Log(const log_evt_t& evt);

private:
    // fcn
    void    processLine_();
    size_t  tokenizeLine_(char ** tokens);
    void    dispatchModule_(char ** tokens, size_t count);

    // var
    IoStream& stream_;
    char line_[CLI_MAX_LINE];
    size_t pos_;
};

#endif // CLI_HPP