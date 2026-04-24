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
typedef enum
{
    SNAP_OK = 0,
    SNAP_INVALID_ARG,
    SNAP_MUTEX_ERR,

    SNAP_INITIALIZED
} snapshot_err_t;


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
snapshot_err_t snapshot_init(snapshot_t * snap);
snapshot_err_t snapshot_deinit(snapshot_t * snap);

snapshot_err_t snapshot_set_meas_values(snapshot_t * snap, uint64_t timestamp, int32_t code, int64_t val_filt, bool avg_ready, int64_t val_avg);

snapshot_err_t snapshot_set_scale(snapshot_t * snap, int32_t scale);
snapshot_err_t snapshot_set_offset(snapshot_t * snap, int32_t offset);


#ifdef __cplusplus
}
#endif

#endif // _SNAPSHOT_H
