#include "cli_interface.hpp"

#include <cstring>
#include <ctype.h>
#include "esp_log.h"


#define MAX_WORD_COUNT    8
#define MAX_CHARS_IN_WORD 16


static const char * TAG = "CLI";


CLI::
CLI(my_uart_t& uart, Context& ctx, QueueHandle_t queue) :
    uart_(uart),
    ctx_(ctx),
    queue_(queue),
    rx_len_(0),
    overflow_(false),
    echo_(false)
{
    memset(rx_line_, '\0', RX_LINE_MAX);
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

            if ( rx_len_ >= RX_LINE_MAX - 1 )
            {
                overflow_ = true;
                continue;
            }

            if ( data[i] == '\r' )
                continue;

            if ( data[i] == '\n' )
            {
                rx_line_[rx_len_] = '\0';
                xQueueSend(queue_, rx_line_, 0);
                resetLine_();
                return;
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
	
	if ( strcmp(tokens[0], "hx711") == 0 )
	{
	    ESP_LOGI(TAG, "module match: %s", tokens[0]);
	}
	else if ( strcmp(tokens[0], "meas") == 0 )
	{
	    ESP_LOGI(TAG, "module match: %s", tokens[0]);
	}
	else
	{
	    ESP_LOGW(TAG, "no matching module: %s", tokens[0]);
        printErr_(TAG, CLI_MODULE_ERR, "dispatch failed");
	}
}

void CLI::
resetLine_()
{
    rx_len_ = 0;
    rx_line_[0] = '\0';
}


void CLI::
printErr_(const char * module, const int err, const char * msg)
{
    char payload[128];
    size_t length = snprintf(
        payload, 
        sizeof(payload),
        "ERR %s %d %s\r\n",
        module ? module : "cli",
        err,
        msg ? msg : ""
    );

    if ( length < 0 )
        return;
    
    if ( length >= sizeof(payload) )
    {
        length = sizeof(payload) - 1;
    }

    if ( uart_write_bytes(uart_.config.port, payload, length) < 0 )
    {
        ESP_LOGE(TAG, "failed with error: %d", CLI_SEND_RES_ERR);
    }
}
