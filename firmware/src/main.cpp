/**
 * @file main.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "app.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

static constexpr int MAX_INIT_COUNT = 3; 

extern "C" void
app_main()
{
    int retries = 0;
    auto st = app_InitStatus::ONGOING;
    while( app_InitStatus::DONE != st )
    {
        switch(st)
        {
            case app_InitStatus::ONGOING:
                st = app_Init();
                break;
            case app_InitStatus::RESTART:
                if ( ++retries >= MAX_INIT_COUNT )
                {
                    st = app_InitStatus::FATAL;
                    break;
                }
                else
                {
                    st = app_InitStatus::ONGOING;
                }
                break;
            case app_InitStatus::FATAL:
                esp_restart();
                break;
            default:
                st = app_InitStatus::FATAL;
                break;
        }
    }
    vTaskDelete(NULL);
}
