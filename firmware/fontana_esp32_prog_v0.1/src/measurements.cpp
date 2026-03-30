#include "measurements.hpp"


static constexpr uint16_t DEFAULT_AVG_WINDOW_SIZE = 64;
static constexpr uint8_t DEFAULT_IIR_SHIFT = 1;


Measurement::Measurement() : 
    offset_raw_(0),
    scale_x1000_(1),
    filtered_raw_(0),
    iir_shift_(DEFAULT_IIR_SHIFT),
    first_sample_(true),
    filt_value_ready_(false),
    avg_sum_raw_(0),
    avg_count_(0),
    avg_window_size_(DEFAULT_AVG_WINDOW_SIZE),
    avg_raw_(0),
    avg_value_ready_(false)
{
}


void Measurement::reset()
{
    filtered_raw_ = 0;
    first_sample_ = true;
    filt_value_ready_ = false;
    avg_sum_raw_ = 0;
    avg_count_ = 0;
    avg_raw_ = 0;
    avg_value_ready_ = false;
}


void Measurement::
setOffsetRaw(int32_t offset)
{
    offset_raw_ = offset;
}


meas_err_t Measurement::
setCountsPerMmHgX1000(int32_t scale) 
{
    if ( scale <= 0 )
    {
        return MEAS_INVALID_ARG_ERR;
    }
    scale_x1000_ = scale;
    return MEAS_OK;
}


meas_err_t Measurement::
setIirShift(uint8_t shift)
{
    if (shift < 1 || shift > 4)
    {
        return MEAS_INVALID_ARG_ERR;
    }

    iir_shift_ = shift;
    return MEAS_OK;
}


meas_err_t Measurement::
setAvgWindowSize(uint16_t window_size)
{
    if (window_size == 0)
    {
        return MEAS_INVALID_ARG_ERR;
    }

    avg_window_size_ = window_size;
    avg_sum_raw_ = 0;
    avg_count_ = 0;
    avg_raw_ = 0;
    avg_value_ready_ = false;

    return MEAS_OK;
}


void Measurement::
pushRaw(int32_t raw) 
{
    int32_t raw_cor = raw - offset_raw_;

    if (first_sample_) 
    {
        first_sample_ = false;
        filtered_raw_ = raw_cor;
    } 
    else
    {
        filtered_raw_ += (raw_cor - filtered_raw_) / (1 << iir_shift_);
    }
    filt_value_ready_ = true;

    avg_sum_raw_ += raw_cor;
    if (++avg_count_ < avg_window_size_)
    {
        return;
    }

    avg_raw_ = avg_sum_raw_ / avg_count_;
    avg_sum_raw_ = 0;
    avg_count_ = 0;
    avg_value_ready_ = true;
}


meas_err_t Measurement::
getFilteredValueX1000(int32_t& value) const 
{
    if ( scale_x1000_ > 0 && filt_value_ready_ )
    {
        filt_value_ready_ = false;
        value = ( 1000000LL * filtered_raw_ / scale_x1000_ );
        return MEAS_OK;
    }

    return MEAS_FILT_NOT_RDY;
}


meas_err_t Measurement::
getAvgValueX1000(int32_t& value) const 
{
    if (scale_x1000_ == 0)
    {
        return MEAS_ZERO_DIV_ERR;
    }
    
    if ( avg_value_ready_ )
    {
        avg_value_ready_ = false;
        value = 1000000LL * avg_raw_ / scale_x1000_;
        return MEAS_OK;
    }

    return MEAS_AVG_NOT_RDY;
}
