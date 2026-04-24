#ifndef _TASK_STRUCTURES_HPP
#define _TASK_STRUCUTRES_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "hx711.h"
#include "measurement.hpp"
#include "snapshot.h"
#include "cli_interface.hpp"


typedef enum
{
    TASK_OK = 0,
    TASK_INVALID_ARG,

} tasks_err_t;


typedef struct
{
    hx711_t * hx711;
    Measurement * meas;
    snapshot_t * snap;
    SemaphoreHandle_t hx711_mtx;
    SemaphoreHandle_t meas_mtx;
} task_meas_ctx_t;


typedef struct 
{
    CLI * cli;
    QueueHandle_t queue;
} task_cli_ctx_t;




#endif // _TASK_STRUCTURES_HPP