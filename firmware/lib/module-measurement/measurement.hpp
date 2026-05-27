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

#include "err_status.hpp"

#include <cstddef>
#include <cstdint>

/**
 * @brief Main Meas Class body
 */
class Meas
{
public:
    Meas()
    : codeOffset_(0)
    , codeCountPerUmHg_(1)
    , codeFilt_(0)
    , iirShift_(1/*i have to change it*/)
    , firstSample_(true)
    , codeAverage_(0)
    , codeAverageSum_(0)
    , averageWindowSize_(64/*change it to default win size*/)
    , averageIndexCount_(0)
    , averageValueReady_(false)
    {}

    ~Meas() = default;

    /**
     * @brief write() function used for pushing and computing new data received by sensor
     *        *2026.05.18* - it pushes data through IIR filter (for filtering) and performs averaging in specified,
     *                       by @param averageWindowSize window size
     * @param code raw output from sensor
     */
    void write(int32_t code);

    /**
     * @brief 
     * @param umHgFilt 
     * @return ErrStatus:
     *                      - ZERO_DIV if scale is set to 0, which is incorrect (clear error),
     *                      - OK
     */
    ErrStatus readFiltVal(int64_t& umHgFilt) const;

    /**
     * @brief 
     * @param umHgAvg 
     * @return ErrStatus:
     *                      - ZERO_DIV if scale is set to 0, which is incorrect (clear error),
     *                      - AVG_NRDY
     *                      - OK
     */
    ErrStatus readAvgVal(int64_t& umHgAvg);

    /**
     * @brief Set the codeOffset_ object
     * @param code raw value from sensor at point 0
     * @return ErrStatus
     */
    void setCodeOffset(int32_t code);

    int32_t getCodeOffset() const;

    /**
     * @brief Set the codeCountsPerUmHg_ object
     * @param code calculated value of scale stated by selected equation
     * @return ErrStatus
     */
    ErrStatus setCodeCountsPerUmHg(int32_t countsPerUmHg);

    int32_t getCodeCountsPerUmHg() const;

    /**
     * @brief Set the iirShift_ object
     * @param shift value of IIR filter shift
     * @return ErrStatus
     */
    ErrStatus setIirShift(uint8_t shift);

    uint8_t getIirShift() const;

    /**
     * @brief Set the avgWindowSize_ object
     * @param size size of averaging window
     * @return ErrStatus
     */
    ErrStatus setAvgWindowSize(uint8_t size);

    uint8_t getAvgWindowSize() const;

    /**
     * @brief reset() function used for reseting Meas Class settings
     */
    void reset();

private:
    int32_t codeOffset_;
    int32_t codeCountPerUmHg_;

    int64_t codeFilt_;
    uint8_t iirShift_;
    bool    firstSample_;

    int64_t codeAverage_;
    int64_t codeAverageSum_;
    uint8_t averageWindowSize_; 
    uint8_t averageIndexCount_;   
    bool    averageValueReady_;
};

#endif // MEASUREMENT_HPP
