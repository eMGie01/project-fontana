/**
 * @file cli_interface.cpp
 * @author Marek Gałeczka
 * @brief Implementation of CLI command parsing, queuing, and dispatching.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "cli_handles.hpp"

#include <cstring>
#include <ctype.h>
#include "esp_log.h"
#include "freertos/idf_additions.h"


#define MAX_WORD_COUNT    8
#define MAX_CHARS_IN_WORD 16


static const char * TAG = "CLI";


CLI::
CLI(my_uart_t& uart, cli_ctx_t& ctx) :
    uart_(uart),
    ctx_(ctx),
    queue_(nullptr),
    rx_len_(0),
    overflow_(false),
    echo_(false)
{
    memset(rx_line_, '\0', CLI_RX_LINE_MAX);
}


void CLI::
updateQueue(QueueHandle_t queue)
{
    queue_ = queue;
    overflow_ = false;
    resetLine_();
}

void CLI::
updateContext(cli_ctx_t& ctx)
{
    ctx_ = ctx;
}


void CLI::
push (const char * data, size_t len)
{
    if ( !queue_ )
    {
        ESP_LOGE(TAG, "push failed: %d", CLI_QUEUE_NOT_INIT_ERR);
        printErr_(TAG, CLI_QUEUE_NOT_INIT_ERR, "push failed");
        return;
    }

    if ( data && len > 0 )
    {
        for (size_t i = 0; i < len; ++i)
        {

            if ( overflow_ )
            {
                if ( data[i] != '\n' )
                    continue;
                else
                {
                    ESP_LOGE(TAG, "push failed with error (%d)", CLI_OVERFLOW_ERR);
                    printErr_(TAG, CLI_OVERFLOW_ERR, "push failed");
                    overflow_ = false;
                    resetLine_();
                    continue;
                }
            }

            if ( rx_len_ >= CLI_RX_LINE_MAX - 1 )
            {
                overflow_ = true;
                continue;
            }

            if ( data[i] == '\r' )
                continue;

            if ( data[i] == '\n' )
            {
                rx_line_[rx_len_] = '\0';
                if ( pdPASS != xQueueSend(queue_, rx_line_, 0) )
                {
                    ESP_LOGE(TAG, "process failed with error (%d)", CLI_QUEUE_OVERFLOW);
                    printErr_(TAG, CLI_QUEUE_OVERFLOW, "process failed");
                }
                resetLine_();
                continue;
            }

            rx_line_[rx_len_++] = data[i];
        }
    }
}


void CLI::
process(char * line)
{
    char * tokens[MAX_WORD_COUNT];
    size_t token_count = tokenizeLine_(tokens, line, MAX_WORD_COUNT);

    if ( !token_count )
    {
        ESP_LOGE(TAG, "process failed with error (%d)", CLI_EMPTY_LINE_ERR);
        printErr_(TAG, CLI_EMPTY_LINE_ERR, "process failed");
        return;
    }
    else if ( token_count > MAX_WORD_COUNT )
    {
        ESP_LOGE(TAG, "process failed with error (%d)", CLI_TOO_MANY_ARGS_ERR);
        printErr_(TAG, CLI_TOO_MANY_ARGS_ERR, "process failed");
        return;
    }

    dispatchModule_(tokens, token_count);

    // char payload[128];

}


void CLI::
printOut(const char * data, size_t len)
{
    if ( !data || len == 0)
    {
        ESP_LOGE(TAG, "printOut failed with error (%d)", CLI_INVALID_ARG_ERR);
        return;
    }

    if ( uart_write_bytes(uart_.config.port, data, len) < 0 )
    {
        ESP_LOGE(TAG, "printOut failed with error (%d)", CLI_SEND_RES_ERR);
    }
}


size_t CLI::
tokenizeLine_(char ** tokens, char * line, size_t max_count)
{
    size_t count = 0;
    char * ptr = line;
    
    while ( *ptr != '\0' )
    {
        while (*ptr != '\0' && isspace((unsigned char)*ptr))
            ++ptr;
            
        if ( *ptr == '\0' )
            break;
        
        if ( count < max_count )
            tokens[count++] = ptr;
        else
            return (MAX_WORD_COUNT + 1);
            
        while ( (*ptr != '\0') && !isspace((unsigned char)*ptr) )
            ++ptr;
            
        if ( *ptr != '\0' )
        {
            *ptr = '\0';
            ++ptr;
        }
    }

    return count;
}

void CLI::
dispatchModule_(char ** tokens, size_t count)
{
    if ( !tokens )
    {
        ESP_LOGE(TAG, "dispatch failed with error (%d)", CLI_INVALID_ARG_ERR);
        printErr_(TAG, CLI_INVALID_ARG_ERR, "dispatch failed");
        return;
    }
	
    cli_err_t res = CLI_MODULE_ERR;
    if ( strcmp(tokens[0], "hx711") == 0 )
    {
        res = hx711_handle(tokens, count, ctx_, uart_);
    }
    else if ( strcmp(tokens[0], "meas") == 0 )
    {
        res = meas_handle(tokens, count, ctx_, uart_);
    }
    else if (strcmp(tokens[0], "help") == 0 )
    {
        printHelp_();
        res = CLI_OK;
    }   
    else
    {
        ESP_LOGW(TAG, "no matching module: %s", tokens[0]);
    }

    if ( res != CLI_OK )
    {
        ESP_LOGE(TAG, "dispatch failed with error (%d)", res);
        printErr_(TAG, res, "dispatch failed");
    }
}

void CLI::
resetLine_()
{
    rx_len_ = 0;
    rx_line_[0] = '\0';
}


void CLI::
printHelp_()
{
    static const char help_text[] =
        "Available commands:\r\n"
        "  help\r\n"
        "  hx711 get mode\r\n"
        "  hx711 get timeout\r\n"
        "  hx711 set mode <value>\r\n"
        "  hx711 set timeout <value>\r\n"
        "  meas get filt\r\n"
        "  meas get avg\r\n"
        "  meas set offset <value>\r\n"
        "  meas set scale <value>\r\n"
        "  meas set iir <1..4>\r\n"
        "  meas set avgwin <value>\r\n";

    if ( uart_write_bytes(uart_.config.port, help_text, sizeof(help_text) - 1) < 0 )
    {
        ESP_LOGE(TAG, "printHelp_ failed with error (%d)", CLI_SEND_RES_ERR);
    }
}


void CLI::
printErr_(const char * module, const int err, const char * msg)
{
    char payload[128];
    int length = snprintf(
        payload, 
        sizeof(payload),
        "ERR %s %d %s\r\n",
        module ? module : "cli",
        err,
        msg ? msg : ""
    );

    if ( length <= 0 )
        return;
    
    if ( length >= sizeof(payload) )
    {
        length = sizeof(payload) - 1;
    }

    if ( uart_write_bytes(uart_.config.port, payload, (size_t)length) < 0 )
    {
        ESP_LOGE(TAG, "printErr_ failed with error (%d)", CLI_SEND_RES_ERR);
    }
}
