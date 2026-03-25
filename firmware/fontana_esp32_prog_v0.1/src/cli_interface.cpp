#include "cli_interface.hpp"

#include <cstring>
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
        printErr("buffer overflow occurred");
        overflow_error_ = false;
        return;
    }

    if ( !cmd_ready_ )
        return;

    char parsed_data[MAX_WORD_COUNT][MAX_CHARS_IN_WORD] = {};
    size_t word_count = 0;

    // parsowanie danych

    if ( word_count == 0 )
    {
        resetLine();
        return;
    }

    // dispatch komend

    resetLine();
}


