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

#include "meas_task_api.h"
#include "cli_api.h"
#include "esp_err.h"
#include "string.h"

static esp_err_t
cmd_Reset(int argc, char** argv, char* resp, size_t size)
{
    meas_TaskCommand_t cmd = {};
    cmd.type = RESET;
    meas_TaskSendCmd(cmd);
    snprintf(resp, size, "Meas reset\r\n");
    return ESP_OK;
}

static esp_err_t
cmd_SetOffset(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2) {
        snprintf(resp, size, "Usage: set_offset <counts>\r\n");
        return ESP_FAIL;
    }

    int32_t value = atoi(argv[1]);
    meas_TaskCommand_t cmd = {};
    cmd.type = SET_OFFSET;
    cmd.arg.codeOffset = value;

    meas_TaskSendCmd(cmd);
    snprintf(resp, size, "Offset set to %ld\r\n", value);
    return ESP_OK;
}

static esp_err_t
cmd_SetIirShift(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2) {
        snprintf(resp, size, "Usage: set_iir <shift>\r\n");
        return ESP_FAIL;
    }

    uint8_t value = atoi(argv[1]);
    meas_TaskCommand_t cmd = {};
    cmd.type = SET_IIR_SHIFT;
    cmd.arg.iirShift = value;

    meas_TaskSendCmd(cmd);
    snprintf(resp, size, "IIR shift set to %u\r\n", value);
    return ESP_OK;
}

static esp_err_t
cmd_SetCountsPerUmHg(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2) {
        snprintf(resp, size, "Usage: set_scale <counts>\r\n");
        return ESP_FAIL;
    }

    int32_t value = atoi(argv[1]);
    meas_TaskCommand_t cmd = {};
    cmd.type = SET_COUNTS_PER_UMHG;
    cmd.arg.codeCountsPerUmHg = value;

    meas_TaskSendCmd(cmd);
    snprintf(resp, size, "Counts per umHg set to %lu\r\n", value);
    return ESP_OK;
}

static esp_err_t
cmd_SetAvgWindow(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2) {
        snprintf(resp, size, "Usage: set_avgwin <size>\r\n");
        return ESP_FAIL;
    }

    uint8_t value = atoi(argv[1]);
    meas_TaskCommand_t cmd = {};
    cmd.type = SET_AVG_WINDOW_SIZE;
    cmd.arg.avgWindowSize = value;

    meas_TaskSendCmd(cmd);
    snprintf(resp, size, "AVG window size to %u\r\n", value);
    return ESP_OK;
}

static const cli_Command_t s_MeasCommands[] = {
    {"set_offset",      cmd_SetOffset,        "Set code offset"},
    {"set_scale",       cmd_SetCountsPerUmHg, "Set scale"},
    {"set_iir",         cmd_SetIirShift,      "Set IIR shift"},
    {"set_avgwin",      cmd_SetAvgWindow, "Set avg window size"},
    {"reset",           cmd_Reset,            "Reset Meas state"},
};

static esp_err_t
cmd_Meas(int argc, char** argv, char* resp, size_t size)
{
    if (argc < 2) {
        snprintf(resp, size, "Usage: meas <command> [args]\r\n");
        return ESP_FAIL;
    }

    for (size_t i = 0; i < sizeof(s_MeasCommands) / sizeof(s_MeasCommands[0]); i++)
    {
        if (strcmp(argv[1], s_MeasCommands[i].name) == 0)
        {
            return s_MeasCommands[i].handler(argc - 1, &argv[1], resp, size);
        }
    }

    snprintf(resp, size, "Unknown: meas %s\r\n", argv[1]);
    return ESP_OK;
}

esp_err_t
cli_MeasRegister(void)
{
    static const cli_Command_t entry = {"meas", cmd_Meas, "Measurement commands"};
    return cli_RegisterCommand(&entry);
}
