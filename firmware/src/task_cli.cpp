#include "task_structures.hpp"

#include "freertos/task.h"
#include "esp_log.h"


// Variables
static const char * TAG = "TASK_CLI";


void
taskCli(void * pvParameters)
{
    if ( pvParameters == nullptr )
    {
        ESP_LOGE(TAG, "task_cli failed with error (%d)", TASK_INVALID_ARG);
        esp_restart();
    }

    task_cli_ctx_t * ctx = (task_cli_ctx_t *)pvParameters;
    if ( !ctx->cli || ctx->queue == nullptr )
    {
        ESP_LOGE(TAG, "task_cli failed with error (%d)", TASK_INVALID_ARG);
        esp_restart();
    }

    char line[CLI_RX_LINE_MAX] = {};

    for(;;)
    {
        if ( pdTRUE == xQueueReceive(ctx->queue, line, portMAX_DELAY) )
        {
            ctx->cli->process(line);
        }
    }
}