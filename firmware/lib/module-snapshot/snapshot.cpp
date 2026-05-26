#include "snapshot.hpp"

Snapshot::
~Snapshot()
{
    (void)close();
}

ErrStatus Snapshot::
open()
{
    if (initialized_)
    {
        return ErrStatus::OK;
    }

    snapshot_ = {};
    mutex_ = xSemaphoreCreateMutex();
    if (mutex_ == nullptr)
    {
        return ErrStatus::FAIL;
    }

    initialized_ = true;
    return ErrStatus::OK;
}

ErrStatus Snapshot::
close()
{
    if (!initialized_)
    {
        return ErrStatus::OK;
    }

    if (mutex_ != nullptr)
    {
        vSemaphoreDelete(mutex_);
        mutex_ = nullptr;
    }

    snapshot_ = {};
    initialized_ = false;
    return ErrStatus::OK;
}

ErrStatus Snapshot::
read(Snap& snap)
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }

    snap = snapshot_;
    xSemaphoreGive(mutex_);

    return ErrStatus::OK;
}

ErrStatus Snapshot::
setMeas(uint64_t ms, int32_t code, int64_t filtered, int64_t averaged, bool avgFlag)
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }

    snapshot_.timeMs = ms;
    snapshot_.code = code;
    snapshot_.filtered = filtered;
    if (avgFlag)
    {
        snapshot_.averaged = averaged;
        snapshot_.avgLcd = true;
        snapshot_.avgSD = true;
    }

    xSemaphoreGive(mutex_);
    return ErrStatus::OK;
}

ErrStatus Snapshot::
setOffset(int32_t offset)
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }

    snapshot_.codeOffset = offset;

    xSemaphoreGive(mutex_);
    return ErrStatus::OK;
}

ErrStatus Snapshot::
setCountsPerUmHg(int32_t counts)
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }

    snapshot_.codeCounts = counts;

    xSemaphoreGive(mutex_);
    return ErrStatus::OK;
}

ErrStatus Snapshot::
setAvgWinSize(uint8_t avgwin)
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }

    snapshot_.avgWin = avgwin;

    xSemaphoreGive(mutex_);
    return ErrStatus::OK;
}

ErrStatus Snapshot::
setIirShift(uint8_t shift)
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }

    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }

    snapshot_.iirShift = shift;

    xSemaphoreGive(mutex_);
    return ErrStatus::OK;
}

ErrStatus Snapshot::
toggleAvgLcdFlag()
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }
    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }
    snapshot_.avgLcd = false;
    xSemaphoreGive(mutex_);
    return ErrStatus::OK;
}

ErrStatus Snapshot::
toggleAvgSdFlag()
{
    if (!initialized_ || mutex_ == nullptr)
    {
        return ErrStatus::NODEV;
    }
    if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ErrStatus::TIMEOUT;
    }
    snapshot_.avgSD = false;
    xSemaphoreGive(mutex_);
    return ErrStatus::OK;
}
