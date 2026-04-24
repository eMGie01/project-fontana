/**
 * @file main.cpp
 * @author Marek Gałeczka (eMGie01)
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "app.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"

#define MAX_INIT_COUNT 3

// static const char * TAG = "MAIN";

extern "C" void
app_main()
{
    // ESP_LOGI(TAG, "program started");


    init_status_t st = INIT_ONGOING;
    int retries = 0;
    while( INIT_DONE != st )
    {
        switch(st)
        {
            case INIT_ONGOING:
                st = app_init();
                break;
            case INIT_RESTART:
                if ( ++retries >= MAX_INIT_COUNT )
                {
                    st = INIT_FATAL;
                    break;
                }
                else
                {
                    st = INIT_ONGOING;
                }
                break;
            case INIT_FATAL:
                esp_restart();
                break;
            default:
                st = INIT_FATAL;
                break;
        }
    }
    vTaskDelete(NULL);
}
