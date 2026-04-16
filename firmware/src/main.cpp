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

static constexpr uint32_t CLI_QUEUE_LENGTH   = 8;
static constexpr uint32_t SENSOR_TASK_STACK  = 2048;
static constexpr UBaseType_t SENSOR_TASK_PRIO = 8;

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


static QueueHandle_t
create_cli_queue_or_die_()
{
    QueueHandle_t queue = xQueueCreate(CLI_QUEUE_LENGTH, CLI_RX_LINE_MAX);
    if ( !queue )
    {
        ESP_LOGE(TAG, "cli queue init failed");
        block_forever_();
    }

    return queue;
}


static void
hx711_dout_isr_handler(void *arg)
{
    TaskHandle_t task_handle = (TaskHandle_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( task_handle != NULL )
    {
        vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
    }

    if ( xHigherPriorityTaskWoken == pdTRUE )
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
init_uart_cli_or_die_(my_uart_t * uart, CLI * cli)
{
    if ( uart == nullptr || cli == nullptr )
    {
        ESP_LOGE(TAG, "uart/cli args are null");
        block_forever_();
    }

    uart_err_t uart_res = uart_init(uart);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart init failed with error: %d", uart_res);
        block_forever_();
    }

    uart_res = uart_set_callback(uart, uart_cli_bridge_, cli);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart set callback failed with error: %d", uart_res);
        block_forever_();
    }

    uart_res = uart_start_task(uart);
    if ( UART_OK != uart_res )
    {
        ESP_LOGE(TAG, "uart start task failed with error: %d", uart_res);
        block_forever_();
    }
}


static void
init_hx711_irq_or_die_(const hx711_t * dev)
{
    if ( dev == nullptr )
    {
        ESP_LOGE(TAG, "hx711 irq init wrong arg");
        block_forever_();
    }

    esp_err_t esp_res = gpio_set_intr_type((gpio_num_t)dev->ios.io_dout, GPIO_INTR_NEGEDGE);
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
}


static TaskHandle_t
create_sensor_task_or_die_(Context * ctx)
{
    TaskHandle_t handle = NULL;
    BaseType_t task_res = xTaskCreate(sensor_task_, "SENSOR", SENSOR_TASK_STACK, ctx, SENSOR_TASK_PRIO, &handle);
    if ( task_res != pdPASS )
    {
        ESP_LOGE(TAG, "sensor task create failed with error: %d", task_res);
        block_forever_();
    }

    return handle;
}


static void
add_hx711_isr_or_die_(const hx711_t * dev, TaskHandle_t notify_task)
{
    if ( dev == nullptr || notify_task == NULL )
    {
        ESP_LOGE(TAG, "add isr wrong args");
        block_forever_();
    }

    esp_err_t esp_res = gpio_isr_handler_add((gpio_num_t)dev->ios.io_dout, hx711_dout_isr_handler, (void *)notify_task);
    if ( esp_res != ESP_OK || esp_res != ESP_ERR_INVALID_STATE )
    {
        ESP_LOGE(TAG, "gpio add isr handler failed with error: %d", esp_res);
        block_forever_();
    }
}


static void
sensor_task_( void *pvParameters )
{
    Context *ctx = (Context *)pvParameters;
    if ( !ctx || !ctx->hx711 || !ctx->meas || !ctx->hx711_mtx || !ctx->meas_mtx )
    {
        ESP_LOGE(TAG, "invalid context in sensor_task_");
        vTaskDelete(NULL);
        return;
    }

    int32_t adc_code = 0;
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if ( pdTRUE != xSemaphoreTake(ctx->hx711_mtx, portMAX_DELAY) )
        {
            ESP_LOGE(TAG, "couldnt take mutex over hx711 in sensor_task_");
            continue;
        }

        gpio_intr_disable((gpio_num_t)ctx->hx711->ios.io_dout);
        hx711_status_t res = hx711_read_raw(ctx->hx711, &adc_code);
        gpio_intr_enable((gpio_num_t)ctx->hx711->ios.io_dout);
        xSemaphoreGive(ctx->hx711_mtx);

        if ( res != HX711_OK )
        {
            ESP_LOGE(TAG, "hx711 read failed: %d", res);
            continue;
        }
        
        (void)adc_code;
    }
}


extern "C" void
app_main()
{
    ESP_LOGI(TAG, "program started");

    // Module init
    static hx711_t hx711;
    static SemaphoreHandle_t hx711_mtx;
    init_hx711_or_die_(&hx711, &hx711_mtx);

    static Measurement meas;
    static SemaphoreHandle_t meas_mtx;
    mutex_create_check_(&meas_mtx, "meas");

    // Shared context + CLI
    static QueueHandle_t cli_queue = create_cli_queue_or_die_();
    static Context ctx = {
        .hx711 = &hx711,
        .meas = &meas,
        .hx711_mtx = hx711_mtx,
        .meas_mtx = meas_mtx
    };

    my_uart_t uart = uart_default_dev(NULL);
    CLI cli(uart, ctx, cli_queue);
    init_uart_cli_or_die_(&uart, &cli);

    // Sensor task + IRQ binding
    init_hx711_irq_or_die_(&hx711);
    TaskHandle_t sensor_handle = create_sensor_task_or_die_(&ctx);
    add_hx711_isr_or_die_(&hx711, sensor_handle);

    // CLI processing loop
    char line[CLI_RX_LINE_MAX] = {};
    for (;;)
    {
        if ( xQueueReceive(cli_queue, line, portMAX_DELAY) == pdTRUE )
        {
            cli.process(line);
        }
    }
}
