/**
 * @file measurement.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MEASUREMENT_HPP
#define MEASUREMENT_HPP

#include <stddef.h>
#include <stdint.h>

typedef enum MEAS_StatusTypeDef
{
    MEAS_ERR_NODEV      = -1,
    MEAS_ERR_INVAL      = -2,
    MEAS_ERR_RUNTIME    = -3,
    MEAS_ERR_FILT_NRDY  = -4,
    MEAS_ERR_AVG_NRDY   = -5,
    MEAS_ERR_ZERO_DIV   = -6,
    MEAS_ERR_OK         =  0,
} MEAS_StatusTypeDef;

typedef enum MEAS_IoctlTypeDef
{
    MEAS_IOCTL_RESET                    = 1,
    MEAS_IOCTL_SET_CODE_OFFSET          = 2,
    MEAS_IOCTL_SET_CODE_COUNTS_PER_UMHG = 3,
    MEAS_IOCTL_SET_IIR_SHIFT            = 4,
    MEAS_IOCTL_SET_AVG_WINDOW_SIZE      = 5,
} MEAS_IoctlTypeDef;

typedef struct MEAS_ReadStructDef
{
    int64_t umHgFilt;
    int64_t umHgAvg;
} MEAS_ReadStructDef;

class Meas
{
public:
    Meas()
    : codeOffset_(0)
    , codeCountPerUmHg_(1)
    , codeFilt_(0)
    , iirShift_(1/*i have to change it*/)
    // , filtValueReady_(true)
    , firstSample_(true)
    , codeAverage_(0)
    , codeAverageSum_(0)
    , averageWindowSize_(64/*change it to default win size*/)
    , averageValueReady_(false)
    {} 
    ~Meas() = default;

    // MEAS_StatusTypeDef Open();
    // MEAS_StatusTypeDef Close();
    void Write(int32_t code);
    MEAS_StatusTypeDef Read(MEAS_ReadStructDef& values);
    MEAS_StatusTypeDef Ioctl(MEAS_IoctlTypeDef request, void* arg);

private:
    void reset_();
    MEAS_StatusTypeDef setCodeOffset_(int32_t* code);
    MEAS_StatusTypeDef setCodeCountsPerUmHg_(int32_t* code);
    MEAS_StatusTypeDef setIirShift_(uint8_t* shift);
    MEAS_StatusTypeDef setAvgWindowSize_(uint8_t* size);

    // variables
    int32_t codeOffset_;
    int32_t codeCountPerUmHg_;

    int64_t codeFilt_;
    uint8_t iirShift_;
    // bool    filtValueReady_;
    bool    firstSample_;

    int64_t codeAverage_;
    int64_t codeAverageSum_;
    uint8_t averageWindowSize_; 
    uint8_t averageIndexCount_;   
    bool    averageValueReady_;
};

#endif // MEASUREMENT_HPP
