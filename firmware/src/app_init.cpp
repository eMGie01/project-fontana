/**
 * @file app_init.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "app.hpp"
#include "measurement.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


// Variables
static const char * tag = "APP_INIT";

init_status_t
app_init()
{
    //
    return INIT_DONE;
}
