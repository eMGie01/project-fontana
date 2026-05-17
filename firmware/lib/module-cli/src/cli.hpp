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

static constexpr size_t CLI_MAX_LINE = 64;
static constexpr size_t CLI_MAX_COMMANDS = 16;

typedef int (*cli_CommandHandler)(IoStream& stream, int argc, char** argv);

typedef struct cli_CommandTypeDef
{
    const char* name;
    const char* help;
    cli_CommandHandler handler;
} cli_CommandTypeDef;

class Cli
{
public: 
    Cli(IoStream& stream/*, modules */)
        : stream_(stream)
        , pos_(0)
        , commandCount_(0)
    {};
    ~Cli() = default;

    void Read(size_t count);
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
    const cli_CommandTypeDef* commands_[CLI_MAX_COMMANDS];
    size_t commandCount_;
};

#endif // CLI_HPP
