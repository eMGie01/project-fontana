/**
 * @file measurement_api.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MEASUREMENT_API_HPP
#define MEASUREMENT_API_HPP

#include <stdint.h>

enum class meas_TaskCmdType
{
    RESET = 0,
    SET_OFFSET = 1,
    SET_COUNTS_PER_UMHG = 2,
    SET_IIR_SHIFT = 3,
    SET_AVG_WINDOW_SIZE = 4,
};

struct meas_TaskCmd
{
    meas_TaskCmdType type;
    union
    {
        int32_t codeOffset;
        int32_t codeCountsPerUmHg;
        uint8_t iirShift;
        uint8_t avgWindowSize;
    } arg;

};

bool meas_TaskSendCmd(const meas_TaskCmd& cmd);

#endif // MEASUREMENT_API_HPP