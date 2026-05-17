/**
 * @file cli.cpp
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

static const char * TAG = "Cli";
static constexpr size_t kMaxTokens = 8;

void Cli::
Read(size_t count)
{
    if (count == 0)
    {
        return;
    }

    char buff[CLI_MAX_LINE];
    if (count > sizeof(buff))
    {
        count = sizeof(buff);
    }

    int n = stream_.read(buff, count);
    if (n <= 0)
    {
        return;
    }

    for (size_t i = 0; i < n; ++i)
    {
        if ( buff[i] == '\n' || buff[i] == '\r' )
        {
            if ( pos_ > 0 )
            {
                line_[pos_] = '\0';
                processLine_();
                pos_ = 0;
            }
            continue;
        }

        if ( pos_ < sizeof(line_) - 1 )
        {
            line_[pos_++] = buff[i];
        }
        else
        {
            pos_ = 0;
        }
    }
}

int Cli::
RegisterCommand(const cli_CommandTypeDef* cmd)
{
    if (cmd == nullptr || cmd->name == nullptr || cmd->handler == nullptr)
    {
        ESP_LOGE(TAG, "invalid command registration");
        return -1;
    }

    if (commandCount_ >= CLI_MAX_COMMANDS)
    {
        ESP_LOGE(TAG, "command table full");
        return -1;
    }

    for (size_t i = 0; i < commandCount_; ++i)
    {
        if (strcmp(commands_[i]->name, cmd->name) == 0)
        {
            ESP_LOGE(TAG, "command already registered: `%s`", cmd->name);
            return -1;
        }
    }

    commands_[commandCount_++] = cmd;
    return 0;
}


void Cli::
processLine_(void)
{   
    ESP_LOGD(TAG, "processing line: `%s`", line_);

    char * tokens[kMaxTokens];
    size_t token_count = tokenizeLine_(tokens);

    if ( !token_count || token_count > kMaxTokens )
    {
        ESP_LOGE(TAG, "invalid token count: `%zu`", token_count);
        return;
    }        

    dispatchModule_(tokens, token_count);
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
        
        if ( count < kMaxTokens )
        {
            tokens[count++] = ptr;
        }
        else
        {
            return (kMaxTokens + 1);
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
dispatchModule_(char ** tokens, size_t count)
{   
    ESP_LOGD(TAG, "dispatching module for token: `%s`", tokens[0]);

    if ( !tokens || count == 0 )
    {
        ESP_LOGE(TAG, "dispatchModule_ invalid args, count: `%zu`", count);
        return;
    }

    for (size_t i = 0; i < commandCount_; ++i)
    {
        if (strcmp(tokens[0], commands_[i]->name) == 0)
        {
            commands_[i]->handler(stream_, count, tokens);
            return;
        }
    }

    const char* msg = "unknown command\n";
    stream_.write(msg, strlen(msg));
}
