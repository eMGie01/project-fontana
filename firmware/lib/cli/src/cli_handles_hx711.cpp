/**
 * @file cli_handles_hx711.cpp
 * @author Marek Gałeczka (eMGie01)
 * @brief CLI command handlers responsible for configuring the HX711 module.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "cli_handles.hpp"

#include "esp_log.h"

#include <climits>
#include <cstdlib>
#include <cstring>
#include <errno.h>


static const char * TAG = "HX711_H";


static cli_err_t
take_mutex_(SemaphoreHandle_t mtx, const char * what)
{
    if (!mtx)
    {
        return CLI_INVALID_ARG_ERR;
    }

    if (xSemaphoreTake(mtx, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "couldn't take mutex over %s", what ? what : "unknown");
        return CLI_MUTEX_ERR;
    }

    return CLI_OK;
}


static cli_err_t
map_hx711_err_(hx711_status_t res)
{
    switch (res)
    {
        case HX711_OK:
            return CLI_OK;
        case HX711_INVALID_ARG:
            return CLI_INVALID_ARG_ERR;
        case HX711_NOT_READY:
        case HX711_TIMEOUT:
            return CLI_MODULE_ERR;
        case HX711_NOT_INITIALIZED:
        case HX711_HW_ERR:
        case HX711_UNEXPECTED_ERR:
        default:
            return CLI_UNEXPECTED_ERR;
    }
}


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
hx711_handle_get_(char ** tokens, size_t count, cli_ctx_t& ctx, my_uart_t& uart)
{
    if (count < 3)
    {
        return CLI_TOO_LITTLE_TOKENS;
    }

    int32_t value = 0;
    hx711_status_t res = HX711_UNEXPECTED_ERR;
    const char * field = tokens[2];
    cli_err_t lock_res = take_mutex_(ctx.hx711_mtx, field);
    if (lock_res != CLI_OK)
    {
        return lock_res;
    }

    if (strcmp(field, "mode") == 0)
    {
        ESP_LOGW(TAG, "no handle for this command yet");
        res = HX711_UNEXPECTED_ERR;
    }
    else if (strcmp(field, "timeout") == 0)
    {
        ESP_LOGW(TAG, "no handle for this command yet");
        res = HX711_UNEXPECTED_ERR;
    }
    else
    {
        xSemaphoreGive(ctx.hx711_mtx);
        return CLI_MODULE_ERR;
    }

    xSemaphoreGive(ctx.hx711_mtx);

    cli_err_t mapped = map_hx711_err_(res);
    if (mapped != CLI_OK)
    {
        ESP_LOGE(TAG, "`get` failed with error (%d)", res);
        return mapped;
    }

    return send_hx711_value_(uart, field, value);
}


static cli_err_t
hx711_handle_set_(char ** tokens, size_t count, cli_ctx_t& ctx, my_uart_t& uart)
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
    cli_err_t lock_res = take_mutex_(ctx.hx711_mtx, field);
    if (lock_res != CLI_OK)
    {
        return lock_res;
    }

    if (strcmp(field, "mode") == 0)
    {
        ESP_LOGW(TAG, "no handle for this command yet");
        res = HX711_UNEXPECTED_ERR;
    }
    else if (strcmp(field, "timeout") == 0)
    {
        ESP_LOGW(TAG, "no handle for this command yet");
        res = HX711_UNEXPECTED_ERR;
    }
    else
    {
        xSemaphoreGive(ctx.hx711_mtx);
        return CLI_MODULE_ERR;
    }

    xSemaphoreGive(ctx.hx711_mtx);

    cli_err_t mapped = map_hx711_err_(res);
    if (mapped != CLI_OK)
    {
        ESP_LOGE(TAG, "`set` failed with error (%d)", res);
        return mapped;
    }

    return send_hx711_value_(uart, field, value);
}


cli_err_t
hx711_handle(char ** tokens, size_t count, cli_ctx_t& ctx, my_uart_t& uart)
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
