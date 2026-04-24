/**
 * @file task_meas.cpp
 * @author Marek Gałeczka (eMGie01)
 * @brief Measurement task implementation responsible for reading HX711 data and updating processed values.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "task_structures.hpp"

#include "freertos/task.h"
#include "esp_log.h"


// Variables
static const char * TAG = "TASK_MEAS";


void 
taskMeas(void * pvParameters)
{
    // initial args check
    task_meas_ctx_t * ctx = (task_meas_ctx_t *)pvParameters;
    if ( ctx == NULL || ctx->hx711 == NULL || ctx->meas == NULL || ctx->snap == NULL ||
        !ctx->snap->initialized || ctx->meas_mtx == NULL || ctx->hx711_mtx == NULL )
    {
        ESP_LOGE(TAG, "task_meas failed with error (%d)", TASK_INVALID_ARG);
        esp_restart();
    }

    // task init
    if ( pdTRUE != xSemaphoreTake(ctx->meas_mtx, portMAX_DELAY) )
    {
        ESP_LOGE(TAG, "failed to take meas_mtx semaphore");
        esp_restart();
    }

    if ( SNAP_OK != snapshot_set_scale(ctx->snap, ctx->meas->getScale()) ||
        SNAP_OK != snapshot_set_offset(ctx->snap, ctx->meas->getOffset()) )
    {
        ESP_LOGE(TAG, "failed to set snapshot initial measurement values");
        esp_restart();
    }

    xSemaphoreGive(ctx->meas_mtx);


    // local variables
    int res = 0;
    TickType_t tick_count;
    int32_t code = 0;
    int64_t filt_val = 0, avg_val = 0;
    bool filt_ready = false, avg_ready = false;

    // task loop
    for (;;)
    {
        filt_ready = false;
        avg_ready = false;


        if ( pdTRUE != xSemaphoreTake(ctx->hx711_mtx, portMAX_DELAY) )
        {
            ESP_LOGE(TAG, "failed to take hx711_mtx semaphore");
            continue;
        }

        res = hx711_read_raw_isr_wait(ctx->hx711, &code);
        xSemaphoreGive(ctx->hx711_mtx);
        if ( res != HX711_OK )
        {
            if ( HX711_TIMEOUT == res )
            {
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            else
            {
                ESP_LOGW(TAG, "reading data from hx711 failed with warning (%d)", res);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            continue;
        }
        tick_count = xTaskGetTickCount();

        // semaphore -> if i change parameters of measurement, i don't want to
        // portMaAX_DELAY -> in future change it for timeout with warning message
        if ( pdTRUE != xSemaphoreTake(ctx->meas_mtx, portMAX_DELAY) )
        {
            ESP_LOGE(TAG, "failed to take meas_mtx semaphore");
            continue;
        }
        ctx->meas->pushRaw(code);
        filt_ready = ( MEAS_OK == ctx->meas->getFilteredValueX1000(filt_val) );
        avg_ready = ( MEAS_OK == ctx->meas->getAvgValueX1000(avg_val) );

        xSemaphoreGive(ctx->meas_mtx);
        
        // save to snapshot
        if ( !filt_ready )
        {
            continue;
        }

        res = (int)snapshot_set_meas_values(
            ctx->snap,
            (uint64_t)(tick_count * portTICK_PERIOD_MS),
            code,
            filt_val,
            avg_ready,
            avg_val
        );

        if ( SNAP_OK != res )
        {
            ESP_LOGW(TAG, "snapshot update failed (%d)", res);
        }

        ESP_LOGI(TAG, "[%lld, %ld, %lld, %lld]", ctx->snap->ts, ctx->snap->val_code, ctx->snap->val_filt, ctx->snap->val_avg);

    }
}
