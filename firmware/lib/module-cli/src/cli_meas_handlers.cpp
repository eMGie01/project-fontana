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

#include "cli_meas_handlers.hpp"
#include "meas_task_api.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void cli_WriteString(IoStream& stream, const char* msg)
{
    stream.write(msg, strlen(msg));
}

static void cli_MeasPrintUsage(IoStream& stream)
{
    cli_WriteString(stream,
        "usage:\n"
        "  meas reset\n"
        "  meas offset <code>\n"
        "  meas scale <counts_per_umhg>\n"
        "  meas iir <shift>\n"
        "  meas avg <window_size>\n");
}

static bool cli_ParseInt32(const char* text, int32_t* value)
{
    if (text == nullptr || value == nullptr || *text == '\0')
    {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    long parsed = strtol(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || parsed < INT32_MIN || parsed > INT32_MAX)
    {
        return false;
    }

    *value = (int32_t)parsed;
    return true;
}

static bool cli_ParseUint8(const char* text, uint8_t* value)
{
    int32_t parsed = 0;
    if (!cli_ParseInt32(text, &parsed) || parsed < 0 || parsed > UINT8_MAX)
    {
        return false;
    }

    *value = (uint8_t)parsed;
    return true;
}

static int cli_SendMeasCmd(IoStream& stream, const meas_TaskCmdTypeDef* cmd)
{
    if (!meas_TaskSendCmd(cmd))
    {
        cli_WriteString(stream, "meas: send failed\n");
        return -1;
    }

    cli_WriteString(stream, "meas: ok\n");
    return 0;
}

static int cli_MeasTaskHandler(IoStream& stream, int argc, char** argv)
{
    meas_TaskCmdTypeDef cmd = {};

    if (argc < 2)
    {
        cli_MeasPrintUsage(stream);
        return -1;
    }

    if (strcmp(argv[1], "reset") == 0)
    {
        if (argc != 2)
        {
            cli_MeasPrintUsage(stream);
            return -1;
        }

        cmd.type = MEAS_TASK_CMD_RESET;
        return cli_SendMeasCmd(stream, &cmd);
    }

    if (strcmp(argv[1], "offset") == 0)
    {
        if (argc != 3 || !cli_ParseInt32(argv[2], &cmd.arg.codeOffset))
        {
            cli_WriteString(stream, "usage: meas offset <code>\n");
            return -1;
        }

        cmd.type = MEAS_TASK_CMD_SET_OFFSET;
        return cli_SendMeasCmd(stream, &cmd);
    }

    if (strcmp(argv[1], "scale") == 0 || strcmp(argv[1], "counts") == 0)
    {
        if (argc != 3 || !cli_ParseInt32(argv[2], &cmd.arg.codeCountsPerUmHg) || cmd.arg.codeCountsPerUmHg == 0)
        {
            cli_WriteString(stream, "usage: meas scale <nonzero_counts_per_umhg>\n");
            return -1;
        }

        cmd.type = MEAS_TASK_CMD_SET_COUNTS_PER_UMHG;
        return cli_SendMeasCmd(stream, &cmd);
    }

    if (strcmp(argv[1], "iir") == 0)
    {
        if (argc != 3 || !cli_ParseUint8(argv[2], &cmd.arg.iirShift) || cmd.arg.iirShift > 30)
        {
            cli_WriteString(stream, "usage: meas iir <shift_0_30>\n");
            return -1;
        }

        cmd.type = MEAS_TASK_CMD_SET_IIR_SHIFT;
        return cli_SendMeasCmd(stream, &cmd);
    }

    if (strcmp(argv[1], "avg") == 0)
    {
        if (argc != 3 || !cli_ParseUint8(argv[2], &cmd.arg.avgWindowSize) || cmd.arg.avgWindowSize == 0)
        {
            cli_WriteString(stream, "usage: meas avg <window_size_1_255>\n");
            return -1;
        }

        cmd.type = MEAS_TASK_CMD_SET_AVG_WINDOW_SIZE;
        return cli_SendMeasCmd(stream, &cmd);
    }

    cli_MeasPrintUsage(stream);
    return -1;
}

const cli_CommandTypeDef CLI_MEAS_COMMAND = {
    .name = "meas",
    .help = "measurement control",
    .handler = cli_MeasTaskHandler,
};
