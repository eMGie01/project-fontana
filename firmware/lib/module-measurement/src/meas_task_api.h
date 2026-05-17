/**
 * @file measurement_api.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MEASUREMENT_API_H
#define MEASUREMENT_API_H

#include "measurement.hpp"

#include <stdint.h>
#include <stdbool.h>

typedef enum meas_TaskCmdTypeTypeDef
{
    MEAS_TASK_CMD_RESET = 0,
    MEAS_TASK_CMD_SET_OFFSET = 1,
    MEAS_TASK_CMD_SET_COUNTS_PER_UMHG = 2,
    MEAS_TASK_CMD_SET_IIR_SHIFT = 3,
    MEAS_TASK_CMD_SET_AVG_WINDOW_SIZE = 4,

} meas_TaskCmdTypeTypeDef;

typedef struct meas_TaskCmdTypeDef
{
    meas_TaskCmdTypeTypeDef type;
    union
    {
        int32_t codeOffset;
        int32_t codeCountsPerUmHg;
        uint8_t iirShift;
        uint8_t avgWindowSize;
    } arg;

} meas_TaskCmdTypeDef;

bool meas_TaskSendCmd(const meas_TaskCmdTypeDef* cmd);

#endif // MEASUREMENT_API_H