#ifndef MEASUREMENT_HPP
#define MEASUREMENT_HPP

#include <cstdint>

enum meas_err_t 
{
    MEAS_UNEXPECTED_ERR = -1,
    MEAS_OK = 0,
    MEAS_INVALID_ARG_ERR,
    MEAS_FILT_NOT_RDY,
    MEAS_AVG_NOT_RDY,
    MEAS_ZERO_DIV_ERR
};


class Measurement 
{

public:
    Measurement();
    void reset();

    void       setOffsetRaw(int32_t offset);
    meas_err_t setCountsPerMmHgX1000(int32_t scale);
    meas_err_t setIirShift(uint8_t shift);
    meas_err_t setAvgWindowSize(uint16_t window_size);

    void pushRaw(int32_t raw);

    meas_err_t getFilteredValueX1000(int64_t& value); 
    meas_err_t getAvgValueX1000(int64_t& value);

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
