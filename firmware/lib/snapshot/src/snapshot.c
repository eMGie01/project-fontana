/**
 * @file snapshot.c
 * @author Marek Gałeczka (eMGie01)
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "snapshot.h"

snapshot_err_t
snapshot_init(snapshot_t * snap)
{
    if ( snap == NULL )
    {
        return SNAP_INVALID_ARG;
    }

    if ( snap->initialized )
    {
        return SNAP_INITIALIZED;
    }

    snap->ts = 0;
    snap->val_code = 0;
    snap->val_filt = 0;
    snap->val_avg = 0;
    snap->meas_scale = 0;
    snap->meas_offset = 0;
    snap->_mutex = xSemaphoreCreateMutex();
    if ( !snap->_mutex )
    {
        return SNAP_MUTEX_ERR;
    }
    snap->initialized = true;

    return SNAP_OK;
}


snapshot_err_t
snapshot_deinit(snapshot_t * snap)
{
    if ( snap == NULL )
    {
        return SNAP_INVALID_ARG;
    }

    if ( !snap->initialized )
    {
        return SNAP_OK;
    }

    if ( snap->_mutex )
    {
        vSemaphoreDelete(snap->_mutex);
    }

    snap->ts = 0;
    snap->val_code = 0;
    snap->val_filt = 0;
    snap->val_avg = 0;
    snap->meas_scale = 0;
    snap->meas_offset = 0;
    snap->_mutex = NULL;
    snap->initialized = false;

    return SNAP_OK;
}


snapshot_err_t 
snapshot_set_meas_values(snapshot_t * snap, uint64_t timestamp, int32_t code, int64_t val_filt, bool avg_ready, int64_t val_avg)
{
    if ( !snap || !snap->_mutex )
    {
        return SNAP_INVALID_ARG;
    }

    if ( pdTRUE != xSemaphoreTake(snap->_mutex, portMAX_DELAY) )
    {
        return SNAP_MUTEX_ERR;
    }

    snap->ts = timestamp;
    snap->val_code = code;
    snap->val_filt = val_filt;
    if (avg_ready)
    {
    snap->val_avg = val_avg;
    }

    xSemaphoreGive(snap->_mutex);

    return SNAP_OK;
}


snapshot_err_t 
snapshot_set_scale(snapshot_t * snap, int32_t scale)
{
    if ( !snap || !snap->_mutex )
    {
        return SNAP_INVALID_ARG;
    }

    if ( pdTRUE != xSemaphoreTake(snap->_mutex, portMAX_DELAY) )
    {
        return SNAP_MUTEX_ERR;
    }

    snap->meas_scale = scale;
    xSemaphoreGive(snap->_mutex);

    return SNAP_OK;
}


snapshot_err_t 
snapshot_set_offset(snapshot_t * snap, int32_t offset)
{
    if ( !snap || !snap->_mutex )
    {
        return SNAP_INVALID_ARG;
    }

    if ( pdTRUE != xSemaphoreTake(snap->_mutex, portMAX_DELAY) )
    {
        return SNAP_MUTEX_ERR;
    }

    snap->meas_offset = offset;
    xSemaphoreGive(snap->_mutex);
    
    return SNAP_OK;
}
