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
#define HX711_SCK  GPIO_NUM_3


static const char * TAG = "MAIN";


/**
 * TASKS
 */
static void IRAM_ATTR hx711_dout_isr_handler(void *arg);
static void meas_task_( void *pvParameters );


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
    hx711_hw_t hx711_hw = {
        .io_sck = HX711_SCK,
        .io_dout = HX711_DOUT
    };
    hx711_status_t hx_res = hx711_init_default(
        &hx711,
        &hx711_hw
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

    TaskHandle_t meas_handle = NULL;
    esp_err_t esp_res = gpio_set_intr_type((gpio_num_t)hx711.ios.io_dout, GPIO_INTR_NEGEDGE);
    if ( esp_res != ESP_OK )
    {
        ESP_LOGE(TAG, "gpio set intr type failed with error: %d", esp_res);
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }
    esp_res = gpio_install_isr_service(0);
    if ( esp_res != ESP_OK )
    {
        ESP_LOGE(TAG, "gpio install isr service failed with error: %d", esp_res);
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }
    BaseType_t task_res = xTaskCreate(meas_task_, "MEAS", 2048, &ctx, 7, &meas_handle);
    if ( task_res != pdPASS )
    {
        ESP_LOGE(TAG, "gpio task create failed with error: %d", task_res);
        for (;;) { vTaskDelay(portMAX_DELAY); }
    }
    esp_res = gpio_isr_handler_add((gpio_num_t)hx711.ios.io_dout, hx711_dout_isr_handler, (void *)meas_handle);
    if ( esp_res != ESP_OK )
    {
        ESP_LOGE(TAG, "gpio add isr handler failed with error: %d", esp_res);
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



static void 
hx711_dout_isr_handler(void *arg)
{
    TaskHandle_t task_handle = (TaskHandle_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (task_handle != NULL)
    {
        vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}


static void
meas_task_( void *pvParameters )
{
    Context *ctx = (Context *)pvParameters;
    int32_t value = 0;

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        gpio_intr_disable((gpio_num_t)ctx->hx711->ios.io_dout);

        hx711_status_t res = hx711_read_raw(ctx->hx711, &value);
        if ( res == HX711_OK )
        {
            ctx->meas->pushRaw(value);
        }

        gpio_intr_enable((gpio_num_t)ctx->hx711->ios.io_dout);
    }
}
