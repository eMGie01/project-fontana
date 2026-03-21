#ifndef MEASUREMENTS_HPP
#define MEASUREMENTS_HPP

#include <cstdint>


class Measurement 
{

public:
    Measurement();
    void reset();

    void setOffsetRaw(int32_t offset);
    void setCountsPerMmHg(int32_t scale);
    void setIirShift(uint8_t shift);
    void setAvgWindowSize(uint16_t window_size);

    void pushRaw(int32_t raw);

    bool getFilteredValueX1000(int64_t& value) const;
    bool getAvgValueX1000(int64_t& value) const;

private:
    int32_t offset_raw_;
    int32_t scale_;

    int64_t filtered_raw_;
    uint8_t iir_shift_;
    bool first_sample_;

    int64_t avg_sum_raw_;
    uint16_t avg_count_;
    uint16_t avg_window_size_;
    int64_t avg_raw_;
    bool avg_value_ready_;

};


#endif /* MEASUREMENTS_HPP */
