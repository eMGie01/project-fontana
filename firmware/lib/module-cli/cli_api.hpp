#pragma once

#include "err_status.hpp"
#include <cstddef>

class CliCommandEntry
{
public:
    virtual ~CliCommandEntry() = default;
    virtual ErrStatus execute(const char* subcommand, int argc, char** argv, char* response, size_t resSize) = 0;
    virtual ErrStatus cmdRegister() = 0;
};

class CliControlApi
{
public:
    struct Command
    {
        const char*         name;
        CliCommandEntry*    entry;
        const char*         help;
    };
    virtual ~CliControlApi() = default;
    virtual ErrStatus registerCommand(const Command& entry) = 0;
    static constexpr size_t MAX_COMMANDS = 32;
};
