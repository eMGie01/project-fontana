#ifndef _TASK_STRUCTURES_HPP
#define _TASK_STRUCUTRES_HPP

#include "hx711.h"
#include "measurement.hpp"


typedef enum
{
    TASK_OK = 0,
    TASK_INVALID_ARG,

} tasks_err_t;


typedef struct
{
    hx711_t * adc;
    Measurement * meas;
    Snapshot * snapshot;
    SemaphoreHandle_t meas_mtx;
} task_meas_ctx_t;



#endif // _TASK_STRUCTURES_HPP