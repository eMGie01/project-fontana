/**
 * @file sd_logger_task.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief SD card data logging task - records measurements to CSV file
 * @version 0.1
 * @date 2026-06-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SD_LOGGER_TASK_HPP
#define SD_LOGGER_TASK_HPP

#include "err_status.hpp"
#include "snapshot.hpp"
#include "sd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <cstddef>
#include <cstdint>

class SdLoggerTask
{
public:

    struct Config
    {
        uint32_t    stackSize;
        UBaseType_t priority;
    };

    // Base functions
    ErrStatus   init(Config cfg, sd_handle_t sd_handle);
    ErrStatus   start();
    ErrStatus   stop();

    // Control API
    ErrStatus   enableRecording();
    ErrStatus   disableRecording();
    bool        isRecording() const;
    
    /**
     * @brief Send measurement data to SD logger task
     * @param snap Measurement snapshot to log
     * @return ErrStatus::OK on success
     */
    ErrStatus   sendMeasurement(const Snapshot::Snap& snap);

private:

    enum class EventType
    {
        MEASUREMENT_READY,
        CONTROL_CMD,
    };

    struct ControlCommand
    {
        enum class Type {
            ENABLE_RECORDING,
            DISABLE_RECORDING,
        } type;
    };

    struct Event
    {
        EventType type;
        union
        {
            Snapshot::Snap snap;
            ControlCommand ctrl;
        } data;
    };

    static void taskEntry_(void* pvParameters);
    void        run_();
    ErrStatus   sendEvent_(const Event& evt);
    void        handleMeasurementReady_(const Snapshot::Snap& snap);
    ErrStatus   openFileIfNeeded_();
    ErrStatus   flushBuffer_();

    Config              config_ = {};
    TaskHandle_t        taskHandle_ = nullptr;
    bool                initialized_ = false;
    bool                recording_enabled_ = false;

    sd_handle_t         sd_handle_ = nullptr;
    sd_file_handle_t    current_file_ = {};
    bool                file_open_ = false;
    char                sample_buffer_[100 * 256] = {};
    size_t              sample_buffer_len_ = 0;
    uint32_t            buffered_samples_ = 0;

    QueueHandle_t       eventQueue_ = nullptr;

    static constexpr char TAG[] = "SD_LOGGER";
    static constexpr uint32_t BUFFERED_SAMPLE_LIMIT = 100;
    static constexpr size_t CSV_LINE_MAX_LEN = 256;
};

#endif // SD_LOGGER_TASK_HPP
