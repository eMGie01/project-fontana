#include "cli_handles.hpp"

#include "esp_log.h"

#include <climits>
#include <cstdlib>
#include <cstring>
#include <errno.h>


static const char * TAG = "HX711_H";


static cli_err_t
send_hx711_value_(my_uart_t& uart, const char * field, int32_t value)
{
    char payload[128];
    int len = snprintf(payload, sizeof(payload), "hx711 %s: %ld\r\n", field, (long)value);
    if (len <= 0)
    {
        return CLI_SEND_RES_ERR;
    }

    if (uart_write_bytes(uart.config.port, payload, (size_t)len) < 0)
    {
        return CLI_SEND_RES_ERR;
    }

    return CLI_OK;
}


static cli_err_t
parse_int32_(const char * text, int32_t * out_value)
{
    if (!text || !out_value)
    {
        return CLI_INVALID_ARG_ERR;
    }

    char * end = nullptr;
    errno = 0;
    long value = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || value < INT32_MIN || value > INT32_MAX)
    {
        return CLI_INVALID_ARG_ERR;
    }

    *out_value = (int32_t)value;
    return CLI_OK;
}


static cli_err_t
hx711_handle_get_(char ** tokens, size_t count, Context& ctx, my_uart_t& uart)
{
    if (count < 3)
    {
        return CLI_TOO_LITTLE_TOKENS;
    }

    int32_t value = 0;
    hx711_status_t res = HX711_UNEXPECTED_ERR;
    const char * field = tokens[2];

    if (strcmp(field, "offset") == 0)
    {
        if ( pdTRUE == xSemaphoreTake(ctx.hx711_mtx, portMAX_DELAY) )
        {
            res = hx711_get_offset_raw(ctx.hx711, &value);
            xSemaphoreGive(ctx.hx711_mtx);
        }
        else
        {
            ESP_LOGE(TAG, "couldnt take mutex over hx711 %s in cli_task_", field);
        }
    }
    else if (strcmp(field, "scale") == 0)
    {
        if ( pdTRUE == xSemaphoreTake(ctx.hx711_mtx, portMAX_DELAY) )
        {
            res = hx711_get_scale_raw(ctx.hx711, &value);
            xSemaphoreGive(ctx.hx711_mtx);
        }
        else
        {
            ESP_LOGE(TAG, "couldnt take mutex over hx711 %s in cli_task_", field);
        }
    }
    else
    {
        return CLI_MODULE_ERR;
    }

    if (res != HX711_OK)
    {
        ESP_LOGE(TAG, "`get` failed with error (%d)", res);
        return CLI_UNEXPECTED_ERR;
    }

    return send_hx711_value_(uart, field, value);
}


static cli_err_t
hx711_handle_set_(char ** tokens, size_t count, Context& ctx, my_uart_t& uart)
{
    if (count < 4)
    {
        return CLI_TOO_LITTLE_TOKENS;
    }

    int32_t value = 0;
    cli_err_t parse_res = parse_int32_(tokens[3], &value);
    if (parse_res != CLI_OK)
    {
        return parse_res;
    }

    hx711_status_t res = HX711_UNEXPECTED_ERR;
    const char * field = tokens[2];

    if (strcmp(field, "offset") == 0)
    {
        if ( pdTRUE == xSemaphoreTake(ctx.hx711_mtx, portMAX_DELAY) )
        {
            res = hx711_set_offset_raw(ctx.hx711, value);
            xSemaphoreGive(ctx.hx711_mtx);
        }
        else
        {
            ESP_LOGE(TAG, "couldnt take mutex over hx711 %s in cli_task_", field);
        }
    }
    else if (strcmp(field, "scale") == 0)
    {
        if ( pdTRUE == xSemaphoreTake(ctx.hx711_mtx, portMAX_DELAY) )
        {
            res = hx711_set_scale_raw(ctx.hx711, value);
            xSemaphoreGive(ctx.hx711_mtx);
        }
        else
        {
            ESP_LOGE(TAG, "couldnt take mutex over hx711 %s in cli_task_", field);
        }
    }
    else
    {
        return CLI_MODULE_ERR;
    }

    if (res != HX711_OK)
    {
        ESP_LOGE(TAG, "`set` failed with error (%d)", res);
        return CLI_UNEXPECTED_ERR;
    }

    return send_hx711_value_(uart, field, value);
}


cli_err_t
hx711_handle(char ** tokens, size_t count, Context& ctx, my_uart_t& uart)
{
    if (!tokens || !ctx.hx711 || !ctx.hx711_mtx)
    {
        return CLI_INVALID_ARG_ERR;
    }

    if (count < 2)
    {
        return CLI_TOO_LITTLE_TOKENS;
    }

    if (strcmp(tokens[1], "get") == 0)
    {
        return hx711_handle_get_(tokens, count, ctx, uart);
    }

    if (strcmp(tokens[1], "set") == 0)
    {
        return hx711_handle_set_(tokens, count, ctx, uart);
    }

    return CLI_MODULE_ERR;
}
