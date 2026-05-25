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
#include <cctype>

void Cli::
push(const char c)
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
hasLine() const
{
    return lineReady_;
}

ErrStatus Cli::
execute(char* response, size_t responseSize)
{
    if (!lineReady_)
    {
        return ErrStatus::TIMEOUT;
    }

    char* tokens[K_MAX_TOKENS];
    size_t count = tokenizeLine_(tokens);

    if (!count || count > K_MAX_TOKENS)
    {
        pos_ = 0;
        lineReady_ = false;
        return ErrStatus::FAIL;
    }

    dispatchCommand_(tokens, count, response, responseSize);

    pos_ = 0;
    lineReady_ = false;
    return ErrStatus::OK;
}

ErrStatus Cli::
registerCmd(const CliControlApi::Command& entry)
{
    if (entry.name == nullptr || entry.handler == nullptr)
    {
        return ErrStatus::INVAL;
    }

    if (commandCount_ >= MAX_COMMANDS)
    {
        return ErrStatus::RUNTIME;
    }

    commands_[commandCount_++] = entry;
    return ErrStatus::OK;
}

size_t Cli::
tokenizeLine_(char ** tokens)
{   
    ESP_LOGD(TAG, "tokenizing line...");
    
    if ( !tokens )
    {
        ESP_LOGE(TAG, "invalid argument");
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
    ESP_LOGD(TAG, "dispatching command: `%s`", tokens[0]);

    if ( !tokens || count == 0 )
    {
        ESP_LOGE(TAG, "invalid args, count: `%zu`", count);
        return;
    }

    for (size_t i = 0; i < commandCount_; ++i)
    {
        if (strcmp(tokens[0], commands_[i].name) == 0)
        {
            commands_[i].handler(commands_[i].context, count, tokens, response, responseSize);
            return;
        }
    }

    snprintf(response, responseSize, "Unknown: %s\r\n", tokens[0]);
}
