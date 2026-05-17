/**
 * @file measurement.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "measurement.hpp"

void Meas::
Write(int32_t code)
{
    int32_t offCode = code - codeOffset_;

    // 1. filtering the input code
    if (firstSample_)
    {
        firstSample_ = false;
        codeFilt_ = offCode;
    }
    else
    {
        codeFilt_ += (offCode - codeFilt_) / (1 << iirShift_);
    }

    // 2. average value from window
    codeAverageSum_ += offCode;
    if (averageIndexCount_++ < averageWindowSize_)
    {
        return;
    }

    codeAverage_ = codeAverageSum_ / (int64_t)averageIndexCount_;
    codeAverageSum_ = 0;
    averageIndexCount_ = 0;
    averageValueReady_ = true;
}

meas_StatusTypeDef Meas::
Read(meas_ReadTypeDef& values)
{
    if (codeCountPerUmHg_ == 0)
    {
        return MEAS_ERR_ZERO_DIV;
    }

    values.umHgFilt = codeFilt_ / codeCountPerUmHg_;
    if (averageValueReady_)
    {
        averageValueReady_ = false;
        values.umHgAvg = codeAverage_ / codeCountPerUmHg_;
    }

    return MEAS_ERR_OK;
}

void Meas::
reset_(void)
{
    codeFilt_ = 0;
    // filtValueReady_ = false;
    firstSample_ = true;
    codeAverage_ = 0;
    codeAverageSum_ = 0;
    averageIndexCount_ = 0;
    averageValueReady_ = false;
}

meas_StatusTypeDef Meas::
setCodeOffset_(int32_t* code)
{
    if (code == NULL)
    {
        return MEAS_ERR_INVAL;
    }
    codeOffset_ = *code;
    return MEAS_ERR_OK;
}

meas_StatusTypeDef Meas::
setCodeCountsPerUmHg_(int32_t* countsPerUmHg)
{
    if (countsPerUmHg == NULL || *countsPerUmHg == 0)
    {
        return MEAS_ERR_INVAL;
    }
    codeCountPerUmHg_ = *countsPerUmHg;
    return MEAS_ERR_OK;
}

meas_StatusTypeDef Meas::
setIirShift_(uint8_t* shift)
{
    if (shift == NULL)
    {
        return MEAS_ERR_INVAL;
    }
    iirShift_ = *shift;
    return MEAS_ERR_OK;
}

meas_StatusTypeDef Meas::
setAvgWindowSize_(uint8_t* size)
{
    if (size == NULL)
    {
        return MEAS_ERR_INVAL;
    }
    averageWindowSize_ = *size;
    return MEAS_ERR_OK;
}

meas_StatusTypeDef Meas::
Ioctl(meas_IoctlTypeDef request, void* arg)
{
    meas_StatusTypeDef status;

    switch (request)
    {
    case MEAS_IOCTL_RESET:
    {
        reset_();
        status = MEAS_ERR_OK;
        break;
    }
    case MEAS_IOCTL_SET_CODE_OFFSET:
    {
        status = setCodeOffset_((int32_t *)arg);
        break;
    }
    case MEAS_IOCTL_SET_CODE_COUNTS_PER_UMHG:
    {
        status = setCodeCountsPerUmHg_((int32_t *)arg);
        break;
    }
    case MEAS_IOCTL_SET_IIR_SHIFT:
    {
        status = setIirShift_((uint8_t *)arg);
        break;
    }
    case MEAS_IOCTL_SET_AVG_WINDOW_SIZE:
    {
        status = setAvgWindowSize_((uint8_t *)arg);
        break;
    }
    default:
    {
        return MEAS_ERR_INVAL;
    }
    }

    return status;
}