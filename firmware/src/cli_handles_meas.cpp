#include "cli_handles.hpp"

#include "esp_log.h"

#include <cstdint>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <errno.h>


static const char * TAG = "MEAS_H";


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
map_meas_err_(meas_err_t res)
{
    switch (res)
    {
        case MEAS_OK:
            return CLI_OK;
        case MEAS_INVALID_ARG_ERR:
            return CLI_INVALID_ARG_ERR;
        case MEAS_FILT_NOT_RDY:
        case MEAS_AVG_NOT_RDY:
        case MEAS_ZERO_DIV_ERR:
            return CLI_MODULE_ERR;
        case MEAS_UNEXPECTED_ERR:
        default:
            return CLI_UNEXPECTED_ERR;
    }
}


static cli_err_t
send_meas_value_(my_uart_t& uart, const char * field, int32_t value)
{
    char payload[128];
    int len = snprintf(payload, sizeof(payload), "meas %s: %ld\r\n", field, (long)value);
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
meas_handle_get_(char ** tokens, size_t count, Context& ctx, my_uart_t& uart)
{
    if (count < 3)
    {
        return CLI_TOO_LITTLE_TOKENS;
    }

    if (!ctx.meas)
    {
        return CLI_INVALID_ARG_ERR;
    }

    int64_t value = 0;
    const char * field = tokens[2];
    cli_err_t lock_res = take_mutex_(ctx.meas_mtx, field);
    if (lock_res != CLI_OK)
    {
        return lock_res;
    }
    meas_err_t res = MEAS_UNEXPECTED_ERR;

    if (strcmp(field, "filt") == 0)
    {
        res = ctx.meas->getFilteredValueX1000(value);
    }
    else if (strcmp(field, "avg") == 0)
    {
        res = ctx.meas->getAvgValueX1000(value);
    }
    else
    {
        xSemaphoreGive(ctx.meas_mtx);
        return CLI_MODULE_ERR;
    }

    xSemaphoreGive(ctx.meas_mtx);

    cli_err_t mapped = map_meas_err_(res);
    if (mapped != CLI_OK)
    {
        ESP_LOGE(TAG, "`get` failed with error (%d)", res);
        return mapped;
    }

    return send_meas_value_(uart, strcmp(field, "filt") == 0 ? "filt_x1000" : "avg_x1000", value);
}


static cli_err_t
meas_handle_set_(char ** tokens, size_t count, Context& ctx, my_uart_t& uart)
{
    (void)uart;
    if (count < 4)
    {
        return CLI_TOO_LITTLE_TOKENS;
    }

    if (!ctx.meas)
    {
        return CLI_INVALID_ARG_ERR;
    }

    int32_t value = 0;
    cli_err_t parse_res = parse_int32_(tokens[3], &value);
    if (parse_res != CLI_OK)
    {
        return parse_res;
    }

    const char * field = tokens[2];
    meas_err_t res = MEAS_UNEXPECTED_ERR;

    if (strcmp(field, "offset") == 0)
    {
        cli_err_t lock_res = take_mutex_(ctx.meas_mtx, field);
        if (lock_res != CLI_OK)
        {
            return lock_res;
        }
        ctx.meas->setOffsetRaw(value);
        xSemaphoreGive(ctx.meas_mtx);
        return CLI_OK;
    }
    else if (strcmp(field, "scale") == 0)
    {
        cli_err_t lock_res = take_mutex_(ctx.meas_mtx, field);
        if (lock_res != CLI_OK)
        {
            return lock_res;
        }
        res = ctx.meas->setCountsPerMmHgX1000(value);
        xSemaphoreGive(ctx.meas_mtx);
    }
    else if (strcmp(field, "iir") == 0)
    {
        if (value < 0 || value > UINT8_MAX)
        {
            return CLI_INVALID_ARG_ERR;
        }
        cli_err_t lock_res = take_mutex_(ctx.meas_mtx, field);
        if (lock_res != CLI_OK)
        {
            return lock_res;
        }
        res = ctx.meas->setIirShift((uint8_t)value);
        xSemaphoreGive(ctx.meas_mtx);
    }
    else if (strcmp(field, "avgwin") == 0)
    {
        if (value < 0 || value > UINT16_MAX)
        {
            return CLI_INVALID_ARG_ERR;
        }
        cli_err_t lock_res = take_mutex_(ctx.meas_mtx, field);
        if (lock_res != CLI_OK)
        {
            return lock_res;
        }
        res = ctx.meas->setAvgWindowSize((uint16_t)value);
        xSemaphoreGive(ctx.meas_mtx);
    }
    else
    {
        return CLI_MODULE_ERR;
    }

    cli_err_t mapped = map_meas_err_(res);
    if (mapped != CLI_OK)
    {
        ESP_LOGE(TAG, "`set` failed with error (%d)", res);
        return mapped;
    }

    return CLI_OK;
}


cli_err_t
meas_handle(char ** tokens, size_t count, Context& ctx, my_uart_t& uart)
{
    if (!tokens || !ctx.meas || !ctx.meas_mtx)
    {
        return CLI_INVALID_ARG_ERR;
    }

    if (count < 2)
    {
        return CLI_TOO_LITTLE_TOKENS;
    }

    if (strcmp(tokens[1], "get") == 0)
    {
        return meas_handle_get_(tokens, count, ctx, uart);
    }

    if (strcmp(tokens[1], "set") == 0)
    {
        return meas_handle_set_(tokens, count, ctx, uart);
    }

    return CLI_MODULE_ERR;
}
