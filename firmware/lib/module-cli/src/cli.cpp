#include "cli.hpp"
#include "esp_log.h"

#include <cstring>
#include <ctype.h>


static const char * TAG = "CLI";
static constexpr size_t kMaxTokens = 8;


void CLI::
Read(const char c)
{
    if ( c == '\n' || c == '\r' )
    {
        if ( pos_ > 0 )
        {
            line_[pos_] = '\0';
            processLine_();
            pos_ = 0;
        }
        return;
    }

    if ( pos_ < sizeof(line_) - 1 )
    {
        line_[pos_++] = c;
    }
}


void CLI::
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


size_t CLI::
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
            break;
        
        if ( count < kMaxTokens )
            tokens[count++] = ptr;
        else
            return (kMaxTokens + 1);
            
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
    ESP_LOGD(TAG, "dispatching module for token: `%s`", tokens[0]);

    if ( !tokens || count == 0 )
    {
        ESP_LOGE(TAG, "dispatchModule_ invalid args, count: `%zu`", count);
        return;
    }

    if ( strcmp(tokens[0], "hx711") == 0 )
    {
        // hx711 cmnd helper + Log
        const char * res = "execute hx711 command ... tbd";
        stream_.write(res, strlen(res));
    }
    else if ( strcmp(tokens[0], "meas") == 0 )
    {
        // meas cmnd helper + Log
        const char * res = "execute meas command ... tbd";
        stream_.write(res, strlen(res));
    }
    else if ( strcmp(tokens[0], "help") == 0 )
    {
        // prints out all help cmnds + Log
        const char * res = "execute help command ... tbd";
        stream_.write(res, strlen(res));
    }
    else
        return;
}
