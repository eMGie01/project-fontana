#pragma once

#include "err_status.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <cstdint>

class Snapshot
{
public:
    struct Snap
    {
        uint64_t timeMs;
        int32_t code;
        int64_t filtered;
        int64_t averaged;
        int32_t codeOffset;
        int32_t codeCounts;
        uint8_t avgWin;
        uint8_t iirShift;
        bool avgSD;
        bool avgLcd;
    };

    explicit Snapshot()
    : snapshot_({})
    , mutex_(nullptr)
    , initialized_(false)
    {}
    ~Snapshot();

    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

    ErrStatus open();
    ErrStatus close();

    ErrStatus read(Snap& snap);
    ErrStatus setMeas(uint64_t ms, int32_t code, int64_t filtered, int64_t averaged, bool avgFlag);
    ErrStatus setOffset(int32_t offset);
    ErrStatus setCountsPerUmHg(int32_t counts);
    ErrStatus setAvgWinSize(uint8_t avgwin);
    ErrStatus setIirShift(uint8_t shift);
    ErrStatus toggleAvgLcdFlag();
    ErrStatus toggleAvgSdFlag();

private:
    Snap snapshot_;
    SemaphoreHandle_t mutex_;
    bool initialized_;
};
