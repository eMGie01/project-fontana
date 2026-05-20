/**
 * @file command_line_interface.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "cli.hpp"
#include "esp_log.h"

#include <cstring>
#include <ctype.h>

void Cli::
Push(const char c)
{
    if (lineReady_)
    {
        return;
    }

    if ( c == '\n' || c == '\r' )
    {
        if (pos_ > 0)
        {
            line_[pos_] = '\0';
            lineReady_ = true;
        }
        return;
    }

    if (pos_ < sizeof(line_) - 1)
    {
        line_[pos_++] = c;
    }
}

bool Cli::
HasLine() const
{
    return lineReady_;
}

esp_err_t Cli::
Execute(char* response, size_t responseSize)
{
    if (!lineReady_)
    {
        return ESP_ERR_TIMEOUT;
    }

    char* tokens[K_MAX_TOKENS];
    size_t count = tokenizeLine_(tokens);

    if (!count || count > K_MAX_TOKENS)
    {
        pos_ = 0;
        lineReady_ = false;
        return ESP_FAIL;
    }

    dispatchCommand_(tokens, count, response, responseSize);

    pos_ = 0;
    lineReady_ = false;
    return ESP_OK;
}

esp_err_t Cli::
RegisterCommand(const cli_Command_t* entry)
{
    if (entry == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (commandCount_ >= MAX_COMMANDS)
    {
        return ESP_ERR_NO_MEM;
    }

    commands_[commandCount_++] = *entry;
    return ESP_OK;
}

size_t Cli::
tokenizeLine_(char ** tokens)
{   
    ESP_LOGD(tag, "tokenizing line...");
    
    if ( !tokens )
    {
        ESP_LOGE(tag, "invalid argument");
        return 0;
    }

    size_t count = 0;
    char * ptr = line_;
    
    while ( *ptr != '\0' )
    {
        while (*ptr != '\0' && isspace((unsigned char)*ptr))
            ++ptr;
            
        if ( *ptr == '\0' )
        {
            break;
        }
        
        if ( count < K_MAX_TOKENS )
        {
            tokens[count++] = ptr;
        }
        else
        {
            return (K_MAX_TOKENS + 1);
        }
            
        while ( (*ptr != '\0') && !isspace((unsigned char)*ptr) )
        {
            ++ptr;
        }
            
        if ( *ptr != '\0' )
        {
            *ptr = '\0';
            ++ptr;
        }
    }

    return count;
}


void Cli::
dispatchCommand_(char** tokens, size_t count, char* response, size_t responseSize)
{   
    ESP_LOGD(tag, "dispatching command: `%s`", tokens[0]);

    if ( !tokens || count == 0 )
    {
        ESP_LOGE(tag, "invalid args, count: `%zu`", count);
        return;
    }

    for (size_t i = 0; i < commandCount_; ++i)
    {
        if (strcmp(tokens[0], commands_[i].name) == 0)
        {
            commands_[i].handler(count, tokens, response, responseSize);
            return;
        }
    }

    snprintf(response, responseSize, "Unknown: %s\r\n", tokens[0]);
}
