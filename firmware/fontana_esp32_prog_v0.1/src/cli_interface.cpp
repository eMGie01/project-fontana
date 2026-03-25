#include "cli_interface.hpp"

#include <cstring>
#include <ctype.h>
#include "esp_log.h"


#define MAX_WORD_COUNT    8
#define MAX_CHARS_IN_WORD 16


static constexpr char * TAG = "CLI";


CLI::
CLI(my_uart_t& uart, Context& ctx) :
    uart_(uart),
    ctx_(ctx),
    rx_len_(0),
    cmd_ready_(false),
    overflow_(false),
    overflow_error_(false),
    echo_(false)
{
    memset(rx_line_, '\0', RX_LINE_MAX);
}


void CLI::
push (const char * data, size_t len)
{
    if ( data && len > 0 && !cmd_ready_ )
    {
        for (size_t i = 0; i < len; ++i)
        {

            if ( overflow_ )
            {
                if ( data[i] != '\n' )
                    continue;
                else
                {
                    overflow_ = false;
                    rx_len_ = 0;
                    continue;
                }
            }

            if ( rx_len_ >= RX_LINE_MAX - 1 )
            {
                overflow_ = true;
                overflow_error_ = true;
                continue;
            }

            if ( data[i] == '\r' )
                continue;

            if ( data[i] == '\n' )
            {
                rx_line_[rx_len_] = '\0';
                cmd_ready_ = true;
                return;
            }

            rx_line_[rx_len_++] = data[i];
        }
    }
}


void CLI::
process()
{
    if ( overflow_error_ )
    {
        printErr_("buffer overflow occurred");
        overflow_error_ = false;
        return;
    }

    if ( !cmd_ready_ )
        return;

    char * tokens[MAX_WORD_COUNT];
    size_t token_count = tokenizeLine_(tokens, rx_line_, MAX_WORD_COUNT);

    if ( !token_count )
    {
        resetLine_();
        return;
    }

    dispatchModule_(tokens, token_count);
    resetLine_();
}


char * CLI::
stripLine_()
{
    if ( !rx_line_ ) 
    {
        return NULL;
    }

    char * start = rx_line_;
    char * end = rx_line_ + rx_len_;

    while ( (end > start) && isspace( (unsigned char)*(end-1) ) )
    {
        --end;
    }
    *end = '\0';

    while ( (*start != '\0') && isspace( (unsigned char)*(end-1) ) )
    {
        ++start;
    }

    return start;
}


size_t CLI::
tokenizeLine_(char **tokens, char *line, size_t max_count)
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
            break;
            
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
        printErr_()
        return;
    }

}
