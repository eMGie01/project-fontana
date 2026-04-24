/**
 * @file measurement.hpp
 * @author Marek Gałeczka (eMGie01)
 * @brief Measurement processing interface for filtering and averaging raw sensor values.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef MEASUREMENT_HPP
#define MEASUREMENT_HPP

#include <cstdint>
/**
 * @brief Error codes returned by the Measurement module.
 */
enum meas_err_t 
{
    MEAS_UNEXPECTED_ERR = -1,
    MEAS_OK = 0,
    MEAS_INVALID_ARG_ERR,
    MEAS_FILT_NOT_RDY,
    MEAS_AVG_NOT_RDY,
    MEAS_ZERO_DIV_ERR
};


/**
 * @brief Processes raw sensor samples into filtered and averaged measurement values.
 */
class Measurement 
{

public:

    /**
     * @brief Construct a new Measurement object
     */
    Measurement();

    /**
     * @brief Resets the runtime filter and averaging state.
     */
    void reset();

    /**
     * @brief Sets the raw offset subtracted from each incoming sample.
     * 
     * @param offset Raw offset value.
     */
    void       setOffsetRaw(int32_t offset);

    /**
     * @brief Sets the scale factor used to convert counts to pressure units.
     * 
     * @param scale Scale factor in counts-per-mmHg multiplied by 1000.
     * @return meas_err_t Result of the operation.
     */
    meas_err_t setCountsPerMmHgX1000(int32_t scale);

    /**
     * @brief Sets the IIR filter strength.
     * 
     * @param shift IIR shift value controlling smoothing intensity.
     * @return meas_err_t Result of the operation.
     */
    meas_err_t setIirShift(uint8_t shift);

    /**
     * @brief Sets the averaging window size and clears the current average state.
     * 
     * @param window_size Number of samples in the averaging window.
     * @return meas_err_t Result of the operation.
     */
    meas_err_t setAvgWindowSize(uint16_t window_size);

    /**
     * @brief Pushes a new raw sample into the filter and averaging pipeline.
     * 
     * @param raw Raw sensor sample.
     */
    void pushRaw(int32_t raw);

    /**
     * @brief Returns the latest filtered value scaled by 1000.
     * 
     * @param value Output reference for the filtered value.
     * @return meas_err_t Result of the operation.
     */
    meas_err_t getFilteredValueX1000(int64_t& value);

    /**
     * @brief Returns the latest averaged value scaled by 1000.
     * 
     * @param value Output reference for the averaged value.
     * @return meas_err_t Result of the operation.
     */
    meas_err_t getAvgValueX1000(int64_t& value);

    /**
     * @brief Returns the configured raw offset.
     * 
     * @return int32_t Current raw offset.
     */
    int32_t getOffset();

    /**
     * @brief Returns the configured scale factor.
     * 
     * @return int32_t Current scale factor.
     */
    int32_t getScale();

private:
    int32_t offset_raw_;
    int32_t scale_x1000_;

    int64_t filtered_raw_;
    uint8_t iir_shift_;
    bool first_sample_;
    bool filt_value_ready_;

    int64_t avg_sum_raw_;
    uint16_t avg_count_;
    uint16_t avg_window_size_;
    int64_t avg_raw_;
    bool avg_value_ready_;

};

#endif /* MEASUREMENT_HPP */
