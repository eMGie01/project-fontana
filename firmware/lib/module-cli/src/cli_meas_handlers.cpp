/**
 * @file cli_meas_handlers.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief
 * @version 0.1
 * @date 2026-05-15
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "cli_meas_handler.hpp"
#include "meas_task.hpp"

#include "esp_log.h"

#include <cstdlib>
#include <cstring>

namespace
{
constexpr char TAG[] = "CLI_MEAS";

CliMeasCmdEntry* contextToSelf(void* context)
{
    return static_cast<CliMeasCmdEntry*>(context);
}

struct MeasSubCommand
{
    const char* name;
    CliControlApi::CommandHandler handler;
    const char* help;
};
}

ErrStatus CliMeasCmdEntry::
cmdRegister()
{
    if (cliApi() == nullptr || measApi_ == nullptr)
    {
        return ErrStatus::INVAL;
    }

    const CliControlApi::Command entry = {"meas", this, cmdMeasThunk_, "Measurement commands"};
    return cliApi()->registerCommand(entry);
}

ErrStatus CliMeasCmdEntry::
cmdMeasThunk_(void* context, int argc, char** argv, char* resp, size_t size)
{
    if (context == nullptr)
    {
        return ErrStatus::INVAL;
    }
    return contextToSelf(context)->cmdMeas_(argc, argv, resp, size);
}

ErrStatus CliMeasCmdEntry::
cmdSetOffsetThunk_(void* context, int argc, char** argv, char* resp, size_t size)
{
    if (context == nullptr)
    {
        return ErrStatus::INVAL;
    }
    return contextToSelf(context)->cmdSetOffset_(argc, argv, resp, size);
}

ErrStatus CliMeasCmdEntry::
cmdSetCountsPerUmHgThunk_(void* context, int argc, char** argv, char* resp, size_t size)
{
    if (context == nullptr)
    {
        return ErrStatus::INVAL;
    }
    return contextToSelf(context)->cmdSetCountsPerUmHg_(argc, argv, resp, size);
}

ErrStatus CliMeasCmdEntry::
cmdSetIirShiftThunk_(void* context, int argc, char** argv, char* resp, size_t size)
{
    if (context == nullptr)
    {
        return ErrStatus::INVAL;
    }
    return contextToSelf(context)->cmdSetIirShift_(argc, argv, resp, size);
}

ErrStatus CliMeasCmdEntry::
cmdSetAvgWindowThunk_(void* context, int argc, char** argv, char* resp, size_t size)
{
    if (context == nullptr)
    {
        return ErrStatus::INVAL;
    }
    return contextToSelf(context)->cmdSetAvgWindow_(argc, argv, resp, size);
}

ErrStatus CliMeasCmdEntry::
cmdResetThunk_(void* context, int argc, char** argv, char* resp, size_t size)
{
    if (context == nullptr)
    {
        return ErrStatus::INVAL;
    }
    return contextToSelf(context)->cmdReset_(argc, argv, resp, size);
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
        return ErrStatus::OK;
    }

    snprintf(resp, size, "Meas reset error: %d\r\n", (int)st);
    return ErrStatus::FAIL;
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
        snprintf(resp, size, "Offset set to: %ld\r\n", (long)value);
        return ErrStatus::OK;
    }

    snprintf(resp, size, "Offset error: %d\r\n", (int)st);
    return ErrStatus::FAIL;
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
        return ErrStatus::OK;
    }

    snprintf(resp, size, "IIR shift error: %d\r\n", (int)st);
    return ErrStatus::FAIL;
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
        snprintf(resp, size, "Counts per umHg set to: %ld\r\n", (long)value);
        return ErrStatus::OK;
    }

    snprintf(resp, size, "Counts per umHg error: %d\r\n", (int)st);
    return ErrStatus::FAIL;
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
        return ErrStatus::OK;
    }

    snprintf(resp, size, "Avg window size error: %d\r\n", (int)st);
    return ErrStatus::FAIL;
}

ErrStatus CliMeasCmdEntry::
cmdMeas_(int argc, char** argv, char* resp, size_t size)
{
    ESP_LOGD(TAG, "cmd_Meas argc=%d argv0=%s", argc, argv[0]);

    static const MeasSubCommand measCmds[] = {
        {"set_offset", cmdSetOffsetThunk_,        "Set code offset"},
        {"set_scale",  cmdSetCountsPerUmHgThunk_, "Set scale"},
        {"set_iir",    cmdSetIirShiftThunk_,      "Set IIR shift"},
        {"set_avgwin", cmdSetAvgWindowThunk_,     "Set avg window size"},
        {"reset",      cmdResetThunk_,            "Reset Meas state"},
    };

    if (argc < 2)
    {
        snprintf(resp, size, "Usage: meas <command> [args]\r\n");
        return ErrStatus::INVAL;
    }

    for (size_t i = 0; i < sizeof(measCmds) / sizeof(measCmds[0]); i++)
    {
        if (std::strcmp(argv[1], measCmds[i].name) == 0)
        {
            return measCmds[i].handler(this, argc - 1, &argv[1], resp, size);
        }
    }

    snprintf(resp, size, "Unknown: meas %s\r\n", argv[1]);
    return ErrStatus::OK;
}
