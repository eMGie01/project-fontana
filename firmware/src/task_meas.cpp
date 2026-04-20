#include "task_structures.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


// Variables
static const char * TAG = "TASK_MEAS";

static task_meas_ctx_t * ctx;


void 
taskMeas(void * pvParameters)
{
    ctx = (task_meas_ctx_t *)pvParameters;
    if ( ctx == NULL || ctx->adc == NULL || ctx->meas == NULL || ctx->snapshot == NULL || ctx->meas_mtx == NULL )
    {
        ESP_LOGE(TAG, "task_meas failed with error (%d)", TASK_INVALID_ARG);
        esp_restart();
    }


    int res = 0;
    int32_t code = 0;
    int64_t filt_val = 0, avg_val = 0;
    for (;;)
    {
        res = hx711_read_raw_isr(ctx->adc, &code);
        if ( res == HX711_TIMEOUT )
        {
            ESP_LOGW(TAG, "adc timeout");
            continue;
        }

        // semaphore -> if i change parameters of measurement, i don't want to
        // portMaAX_DELAY -> in future change it for timeout with warning message
        if ( pdTRUE == xSemaphoreTake(ctx->meas_mtx, portMAX_DELAY) )
        {
            ctx->meas->pushRaw(code);
            ctx->meas->getFilteredValueX1000(filt_val);
            ctx->meas->getAvgValueX1000(avg_val);
            xSemaphoreGive(ctx->meas_mtx);
        }
        
        // save to snapshot
        // code for save to snapshot
    }
}