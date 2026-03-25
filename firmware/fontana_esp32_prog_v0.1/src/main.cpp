#include "measurements.hpp"
#include "hx711.h"
#include "my_uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"


#define HX711_DOUT GPIO_NUM_2
#define HX711_SCK  GPIO_NUM_4


static const char * TAG = "MAIN";


extern "C" void app_main() 
{
    ESP_LOGI(TAG, "program started");

    // Init
    my_uart_t uart_0_ = uart_default_dev(NULL); /*change to proper callback*/

    uart_err_t uart_res_ = uart_init(&uart_0_);
    if ( UART_OK != uart_res_ )
    {
        ESP_LOGE(TAG, "uart init failed with error: %d", uart_res_);
        for (;;) {vTaskDelay(1000);}
    }
    ESP_LOGI(TAG, "uart initialized successfully");

    uart_res_ = uart_start_task(&uart_0_);
    if ( UART_OK != uart_res_ )
    {
        ESP_LOGE(TAG, "uart init failed with error: %d", uart_res_);
        for (;;) {vTaskDelay(1000);}
    }
    ESP_LOGI(TAG, "uart task started successfully");

    
    // Loop
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
