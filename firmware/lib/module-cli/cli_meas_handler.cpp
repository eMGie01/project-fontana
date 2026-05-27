#include "cli_meas_handler.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>

ErrStatus CliMeasCmdEntry::
cmdRegister()
{
    return cliApi_->registerCommand({"meas", this, "Measurement commands"});
}

ErrStatus CliMeasCmdEntry::
execute(const char* subcommand, int argc, char** argv, char* response, size_t resSize)
{
    if (subcommand == nullptr || std::strcmp(subcommand, "help") == 0)
    {
        snprintf(response, resSize,
            "meas subcommands:\r\n"
            "   reset\r\n"
            "   set_offset\r\n"
            "   set_iir\r\n"
            "   set_avgwin\r\n"
            "   set_counts\r\n"
        );
        return ErrStatus::OK;
    }

    if (std::strcmp(subcommand, "reset") == 0)
    {
        return cmdReset_(argc, argv, response, resSize);
    }

    if (std::strcmp(subcommand, "set_offset") == 0)
    {
        return cmdSetOffset_(argc, argv, response, resSize);
    }

    if (std::strcmp(subcommand, "set_iir") == 0)
    {
        return cmdSetIirShift_(argc, argv, response, resSize);
    }

    if (std::strcmp(subcommand, "set_avgwin") == 0)
    {
        return cmdSetAvgWindow_(argc, argv, response, resSize);
    }

    if (std::strcmp(subcommand, "set_counts") == 0)
    {
        return cmdSetCountsPerUmHg_(argc, argv, response, resSize);
    }

    // log smth
    return ErrStatus::INVAL;
}

ErrStatus CliMeasCmdEntry::
cmdReset_(int argc, char** argv, char* resp, size_t size)
{
    (void)argc;
    (void)argv;

    ErrStatus st = measApi_->reset();
    if (st == ErrStatus::OK)
    {
        snprintf(resp, size, "Meas reset success\r\n");
    }
    else
    {
        snprintf(resp, size, "Meas reset error: %d\r\n", static_cast<int>(st));
    }
    
    return st;
}

ErrStatus CliMeasCmdEntry::
cmdSetOffset_(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2)
    {
        snprintf(resp, size, "Usage: set_offset <counts>\r\n");
        return ErrStatus::INVAL;
    }

    int32_t value = std::atoi(argv[1]);
    ErrStatus st = measApi_->setOffset(value);
    if (st == ErrStatus::OK)
    {
        snprintf(resp, size, "Offset set to: %ld\r\n", static_cast<long>(value));
    }
    else
    {
        snprintf(resp, size, "Offset error: %d\r\n", static_cast<int>(st));
    }
    return st;
}

ErrStatus CliMeasCmdEntry::
cmdSetIirShift_(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2)
    {
        snprintf(resp, size, "Usage: set_iir <shift>\r\n");
        return ErrStatus::INVAL;
    }

    uint8_t value = std::atoi(argv[1]);
    ErrStatus st = measApi_->setIirShift(value);
    if (st == ErrStatus::OK)
    {
        snprintf(resp, size, "IIR shift set to: %d\r\n", value);
    }
    {
        snprintf(resp, size, "IIR shift error: %d\r\n", static_cast<int>(st));
    }
    
    return st;
}

ErrStatus CliMeasCmdEntry::
cmdSetCountsPerUmHg_(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2)
    {
        snprintf(resp, size, "Usage: set_scale <counts>\r\n");
        return ErrStatus::INVAL;
    }

    int32_t value = std::atoi(argv[1]);
    ErrStatus st = measApi_->setCountsPerUmHg(value);
    if (st == ErrStatus::OK)
    {
        snprintf(resp, size, "Counts per umHg set to: %ld\r\n", static_cast<long>(value));
    }
    {
        snprintf(resp, size, "Counts per umHg error: %d\r\n", static_cast<int>(st));
    }
   
    return st;
}

ErrStatus CliMeasCmdEntry::
cmdSetAvgWindow_(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2)
    {
        snprintf(resp, size, "Usage: set_avgwin <size>\r\n");
        return ErrStatus::INVAL;
    }

    uint8_t value = std::atoi(argv[1]);
    ErrStatus st = measApi_->setAvgWindowSize(value);
    if (st == ErrStatus::OK)
    {
        snprintf(resp, size, "Avg window size set to: %d\r\n", value);
    }
    {
        snprintf(resp, size, "Avg window size error: %d\r\n", static_cast<int>(st));
    }
    
    return st;
}
