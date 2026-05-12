/**
 * @file snapshot.h
 * @author Marek Gałeczka (eMGie01)
 * @brief Thread-safe snapshot storage for the latest measurement and configuration values.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef _SNAPSHOT_H
#define _SNAPSHOT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Struct typedefs
/**
 * @brief Error codes returned by the snapshot module.
 */
typedef enum
{
    SNAP_OK = 0,
    SNAP_INVALID_ARG,
    SNAP_MUTEX_ERR,

    SNAP_INITIALIZED
} snapshot_err_t;

/**
 * @brief Shared snapshot of the latest measured and configured values.
 */
typedef struct
{
    uint64_t ts;
    int32_t val_code;
    int64_t val_filt;
    int64_t val_avg;
    int32_t meas_offset;
    int32_t meas_scale;
    SemaphoreHandle_t _mutex;
    bool initialized;
} snapshot_t;


// Functions

/**
 * @brief Initializes the snapshot structure and its synchronization primitive.
 * 
 * @param snap Snapshot instance to initialize.
 * @return snapshot_err_t Result of the operation.
 */
snapshot_err_t snapshot_init(snapshot_t * snap);

/**
 * @brief Releases resources owned by the snapshot structure.
 * 
 * @param snap Snapshot instance to deinitialize.
 * @return snapshot_err_t Result of the operation.
 */
snapshot_err_t snapshot_deinit(snapshot_t * snap);

/**
 * @brief Updates the measurement-related fields stored in the snapshot.
 * 
 * @param snap Snapshot instance to update.
 * @param timestamp Measurement timestamp in milliseconds.
 * @param code Latest raw measurement code.
 * @param val_filt Latest filtered value.
 * @param avg_ready Indicates whether a new averaged value is available.
 * @param val_avg Latest averaged value.
 * @return snapshot_err_t Result of the operation.
 */
snapshot_err_t snapshot_set_meas_values(snapshot_t * snap, uint64_t timestamp, int32_t code, int64_t val_filt, bool avg_ready, int64_t val_avg);

/**
 * @brief Updates the scale value stored in the snapshot.
 * 
 * @param snap Snapshot instance to update.
 * @param scale Scale value to store.
 * @return snapshot_err_t Result of the operation.
 */
snapshot_err_t snapshot_set_scale(snapshot_t * snap, int32_t scale);

/**
 * @brief Updates the offset value stored in the snapshot.
 * 
 * @param snap Snapshot instance to update.
 * @param offset Offset value to store.
 * @return snapshot_err_t Result of the operation.
 */
snapshot_err_t snapshot_set_offset(snapshot_t * snap, int32_t offset);


#ifdef __cplusplus
}
#endif

#endif // _SNAPSHOT_H
