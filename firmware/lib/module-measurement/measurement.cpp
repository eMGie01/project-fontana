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
write(int32_t code)
{
    int32_t offCode = code - codeOffset_;

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
    if (++averageIndexCount_ < averageWindowSize_)
    {
        return;
    }

    codeAverage_ = codeAverageSum_ / static_cast<int64_t>(averageIndexCount_);
    codeAverageSum_ = 0;
    averageIndexCount_ = 0;
    averageValueReady_ = true;
}

ErrStatus Meas::
readFiltVal(int64_t& umHgFilt)
{
    if (codeCountPerUmHg_ == 0)
    {
        return ErrStatus::INVAL;
    }
    umHgFilt = codeFilt_ / codeCountPerUmHg_;
    return ErrStatus::OK;
}

ErrStatus Meas::
readAvgVal(int64_t& umHgAvg)
{
    if (codeCountPerUmHg_ == 0)
    {
        return ErrStatus::INVAL;
    }
    if (averageValueReady_)
    {
        averageValueReady_ = false;
        umHgAvg = codeAverage_ / codeCountPerUmHg_;
        return ErrStatus::OK;
    }
    return ErrStatus::TIMEOUT;
}

void Meas::
reset(void)
{
    codeFilt_ = 0;
    firstSample_ = true;
    codeAverage_ = 0;
    codeAverageSum_ = 0;
    averageIndexCount_ = 0;
    averageValueReady_ = false;
}

void Meas::
setCodeOffset(int32_t code)
{
    codeOffset_ = code;
}

ErrStatus Meas::
setCodeCountsPerUmHg(int32_t countsPerUmHg)
{
    if (countsPerUmHg == 0)
    {
        return ErrStatus::INVAL;
    }
    codeCountPerUmHg_ = countsPerUmHg;
    return ErrStatus::OK;
}

ErrStatus Meas::
setIirShift(uint8_t shift)
{
    if (shift < 1 || shift > 4)
    {
        return ErrStatus::INVAL;
    }
    iirShift_ = shift;
    return ErrStatus::OK;
}

ErrStatus Meas::
setAvgWindowSize(uint8_t size)
{
    averageWindowSize_ = size;
    return ErrStatus::OK;
}
