#ifndef CLI_API_HPP
#define CLI_API_HPP

#include "err_status.hpp"
#include <stddef.h>

class CliControlApi
{
public:
    using CommandHandler = ErrStatus (*)(void* context, int argc, char**argv, char* response, size_t responseSize);
    struct Command
    {
        const char*     name;
        void*           context;
        CommandHandler  handler;
        const char*     help;
    };
    virtual ~CliControlApi() = default;
    virtual ErrStatus registerCommand(const Command& entry) = 0;
};

class CliCommandEntry
{
public:
    explicit CliCommandEntry(CliControlApi* cliApi)
    : cliApi_(cliApi)
    {}
    virtual ~CliCommandEntry() = default;
    virtual ErrStatus cmdRegister() = 0;

protected:
    CliControlApi* cliApi()
    {
        return cliApi_;
    }

private:
    CliControlApi* cliApi_;
};

#endif // CLI_API_HPP
