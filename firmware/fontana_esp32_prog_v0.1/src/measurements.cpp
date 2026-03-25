#include "measurements.hpp"


static constexpr uint16_t DEFAULT_AVG_WINDOW_SIZE = 64;
static constexpr uint8_t DEFAULT_IIR_SHIFT = 1;


Measurement::Measurement() : 
    offset_raw_(0),
    scale_(0),
    filtered_raw_(0),
    iir_shift_(DEFAULT_IIR_SHIFT),
    first_sample_(true),
    avg_sum_raw_(0),
    avg_count_(0),
    avg_window_size_(DEFAULT_AVG_WINDOW_SIZE),
    avg_raw_(0),
    avg_value_ready_(false)
{}


void Measurement::reset() {
    filtered_raw_ = 0;
    first_sample_ = true;
    avg_sum_raw_ = 0;
    avg_count_ = 0;
    avg_raw_ = 0;
    avg_value_ready_ = false;
}


void Measurement::setOffsetRaw(int32_t offset) {
    offset_raw_ = offset;
}


void Measurement::setCountsPerMmHg(int32_t scale) {
    scale_ = scale;
}


void Measurement::setIirShift(uint8_t shift) {
    if (shift < 1 || shift > 4) return;
    iir_shift_ = shift;
}


void Measurement::setAvgWindowSize(uint16_t window_size) {
    if (window_size == 0) return;
    avg_window_size_ = window_size;
    avg_sum_raw_ = 0;
    avg_count_ = 0;
    avg_raw_ = 0;
    avg_value_ready_ = false;
}


void Measurement::pushRaw(int32_t raw) {
    int32_t raw_cor = raw - offset_raw_;

    if (first_sample_) {
        first_sample_ = false;
        filtered_raw_ = raw_cor;
    } else {
        filtered_raw_ += (raw_cor - filtered_raw_) / (1 << iir_shift_);
    }
    
    avg_value_ready_ = false;
    avg_sum_raw_ += raw_cor;
    if (++avg_count_ < avg_window_size_) {
        return;
    }
    avg_raw_ = avg_sum_raw_ / avg_count_;
    avg_sum_raw_ = 0;
    avg_count_ = 0;
    avg_value_ready_ = true;
}


bool Measurement::getFilteredValueX1000(int64_t& value) const {
    if (scale_ == 0) return false;
    value = 1000LL * filtered_raw_ / scale_;
    return true;
}


bool Measurement::getAvgValueX1000(int64_t& value) const {
    if (!avg_value_ready_) return false;
    if (scale_ == 0) return false;
    value = 1000LL * avg_raw_ / scale_;
    return true;
}
