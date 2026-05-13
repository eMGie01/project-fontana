/**
 * @file cli.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CLI_HPP
#define CLI_HPP

#include <stddef.h>
#include "iostream.hpp"

#define CLI_MAX_LINE 64

class Cli
{
public: 
    Cli(IoStream& stream/*, modules */)
        : stream_(stream)
        , pos_(0)
    {}
    ~Cli() = default;

    void Read(const char c);
    int RegisterCommand(const cli_CommandTypeDef* cmd);

private:
    // fcn
    void    processLine_();
    size_t  tokenizeLine_(char **tokens);
    void    dispatchModule_(char **tokens, size_t count);

    // var
    IoStream& stream_;
    char line_[CLI_MAX_LINE];
    size_t pos_;
};

#endif // CLI_HPP