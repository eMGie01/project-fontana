#include "cli_interface.hpp"
#include "measurements.hpp"
#include "my_uart.h"
#include "hx711.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"


#define HX711_DOUT GPIO_NUM_2
#define HX711_SCK  GPIO_NUM_4


static const char * TAG = "MAIN";


static void
uart_cli_bridge_(void * ctx, const char * data, size_t len)
{
    if ( ctx != nullptr )
    {
        static_cast<CLI *>(ctx)->push(data, len);
    }
}


extern "C" void
app_main()
{
    ESP_LOGI(TAG, "program started");

    hx711_t hx711;
    hx711_status_t hx_res = hx711_init_default(
        &hx711,
        &(hx711_hw_t){
            .io_sck = HX711_SCK,
            .io_dout = HX711_DOUT
        }
    );
    if ( HX711_OK != hx_res )
    {
        ESP_LOGE(TAG, "hx711 init failed with error: %d", hx_res);
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }

    Measurement meas;

    QueueHandle_t cli_queue = xQueueCreate(8, MAX_EVENT_BUFF_SIZE);
    if ( cli_queue == NULL )
    {
        ESP_LOGE(TAG, "cli queue init failed");
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }

    my_uart_t uart = uart_default_dev(NULL);
    Context ctx = {
        .hx711 = &hx711,
        .meas = &meas
    };
    CLI cli(uart, ctx, cli_queue);

    uart_err_t uart_res = uart_init(&uart);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart init failed with error: %d", uart_res);
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }

    uart_res = uart_set_callback(&uart, uart_cli_bridge_, &cli);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart set callback failed with error: %d", uart_res);
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }

    uart_res = uart_start_task(&uart);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart start task failed with error: %d", uart_res);
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }

    for (;;)
    {
        char line[MAX_EVENT_BUFF_SIZE] = {};
        if ( xQueueReceive(cli_queue, line, portMAX_DELAY) == pdTRUE )
        {
            cli.process(line);
        }
    }
}
