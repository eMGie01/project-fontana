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


static void
block_forever_()
{
    for (;;) { vTaskDelay(portMAX_DELAY); }
}


static void
mutex_create_check_(SemaphoreHandle_t * mtx, const char * mtx_nm)
{
    if ( mtx == nullptr || mtx_nm == nullptr )
    {
        ESP_LOGE(TAG, "mutex_create wrong arg");
        block_forever_();
    }

    *mtx = xSemaphoreCreateMutex();
    if ( *mtx == nullptr )
    {
        ESP_LOGE(TAG, "%s mutex is null", mtx_nm);
        block_forever_();
    }
}


static void
init_hx711_or_die_(hx711_t * dev, SemaphoreHandle_t * mtx)
{
    if ( dev == nullptr || mtx == nullptr )
    {
        ESP_LOGE(TAG, "hx711 args are null");
        block_forever_();
    }

    hx711_hw_t dev_hw = {
        .io_sck = HX711_SCK,
        .io_dout = HX711_DOUT
    };

    hx711_status_t dev_res = hx711_init_default(dev, &dev_hw);
    if ( HX711_OK != dev_res )
    {
        ESP_LOGE(TAG, "hx711 init failed with error (%d)", dev_res);
        block_forever_();
    }

    mutex_create_check_(mtx, "hx711");
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
uart_cli_bridge_(void * ctx, const char * data, size_t len)
{
    if ( ctx != nullptr )
    {
        static_cast<CLI *>(ctx)->push(data, len);
    }
}


static void
meas_task_( void *pvParameters )
{
    Context *ctx = (Context *)pvParameters;
    if ( !ctx || !ctx->hx711 || !ctx->meas || !ctx->hx711_mtx || !ctx->meas_mtx ) 
    {
        ESP_LOGE(TAG, "invalid context in meas_task_");
        vTaskDelete(NULL);
    }

    int32_t value = 0;
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if ( pdTRUE == xSemaphoreTake(ctx->hx711_mtx, portMAX_DELAY) )
        {
            gpio_intr_disable((gpio_num_t)ctx->hx711->ios.io_dout);
            hx711_status_t res = hx711_read_raw(ctx->hx711, &value);
            gpio_intr_enable((gpio_num_t)ctx->hx711->ios.io_dout);

            xSemaphoreGive(ctx->hx711_mtx);

            if ( res == HX711_OK )
            {
                if ( pdTRUE == xSemaphoreTake(ctx->meas_mtx, portMAX_DELAY) )
                {
                    ctx->meas->pushRaw(value);
                    xSemaphoreGive(ctx->meas_mtx);
                }
                else 
                {
                    ESP_LOGE(TAG, "couldnt take mutex over meas in meas_task_");
                }
            }
        }
        else 
        {
            ESP_LOGE(TAG, "couldnt take mutex over hx711 in meas_task_");
        }
    }
}


extern "C" void
app_main()
{
    ESP_LOGI(TAG, "program started");

    // Init HX711 module
    static hx711_t hx711;
    static SemaphoreHandle_t hx711_mtx;
    init_hx711_or_die_(&hx711, &hx711_mtx);

    // Init Measurement object
    static Measurement meas;
    static SemaphoreHandle_t meas_mtx;
    mutex_create_check_(&meas_mtx, "meas");

    static QueueHandle_t cli_queue = xQueueCreate(8, CLI_RX_LINE_MAX);
    if ( !cli_queue )
    {
        ESP_LOGE(TAG, "cli queue init failed");
        block_forever_();
    }

    static Context ctx = {
        .hx711 = &hx711,
        .meas = &meas,
        .hx711_mtx = hx711_mtx,
        .meas_mtx = meas_mtx
    };

    my_uart_t uart = uart_default_dev(NULL);
    CLI cli(uart, ctx, cli_queue);

    uart_err_t uart_res = uart_init(&uart);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart init failed with error: %d", uart_res);
        block_forever_();
    }

    uart_res = uart_set_callback(&uart, uart_cli_bridge_, &cli);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart set callback failed with error: %d", uart_res);
        block_forever_();
    }

    uart_res = uart_start_task(&uart);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart start task failed with error: %d", uart_res);
        block_forever_();
    }

    TaskHandle_t meas_handle = NULL;

    esp_err_t esp_res = gpio_set_intr_type((gpio_num_t)hx711.ios.io_dout, GPIO_INTR_NEGEDGE);
    if ( esp_res != ESP_OK )
    {
        ESP_LOGE(TAG, "gpio set intr type failed with error: %d", esp_res);
        block_forever_();
    }

    esp_res = gpio_install_isr_service(0);
    if ( esp_res != ESP_OK )
    {
        ESP_LOGE(TAG, "gpio install isr service failed with error: %d", esp_res);
        block_forever_();
    }

    BaseType_t task_res = xTaskCreate(meas_task_, "MEAS", 2048, &ctx, 7, &meas_handle);
    if ( task_res != pdPASS )
    {
        ESP_LOGE(TAG, "gpio task create failed with error: %d", task_res);
        block_forever_();
    }

    esp_res = gpio_isr_handler_add((gpio_num_t)hx711.ios.io_dout, hx711_dout_isr_handler, (void *)meas_handle);
    if ( esp_res != ESP_OK )
    {
        ESP_LOGE(TAG, "gpio add isr handler failed with error: %d", esp_res);
        block_forever_();
    }

    char line[CLI_RX_LINE_MAX] = {};
    for (;;)
    {
        if ( xQueueReceive(cli_queue, line, portMAX_DELAY) == pdTRUE )
        {
            cli.process(line);
        }
    }
}
