/**
 * @file meas_task_api.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MEAS_TASK_API_H
#define MEAS_TASK_API_H

#include <stdint.h>
#include "esp_err.h"

typedef enum
{
    RESET               = 0,
    SET_OFFSET          = 1,
    SET_COUNTS_PER_UMHG = 2,
    SET_IIR_SHIFT       = 3,
    SET_AVG_WINDOW_SIZE = 4,
} meas_TaskCommandType_t;

typedef struct
{
    meas_TaskCommandType_t type;
    union
    {
        int32_t codeOffset;
        int32_t codeCountsPerUmHg;
        uint8_t iirShift;
        uint8_t avgWindowSize;
    } arg;
} meas_TaskCommand_t;

typedef enum
{
    SENSOR_RDY = 0,
    CMD        = 1,
} meas_TaskEventType_t;

typedef struct
{
    meas_TaskEventType_t type;
    meas_TaskCommand_t   cmd;
} meas_TaskEvent_t;

/**
 * @brief function that allows other modules to send commands via Queue to Meas Task
 * @param cmd command to send
 * @return esp_err_t 
 */
esp_err_t meas_TaskSendCmd(const meas_TaskCommand_t cmd);

#endif // MEAS_TASK_API_H
