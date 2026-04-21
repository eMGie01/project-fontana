#include "task_structures.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"


// Defines
#define HX711_SCK  GPIO_NUM_3
#define HX711_DOUT GPIO_NUM_2


// Tasks
extern void taskMeas(void * pvParameters);
extern void taskCli(void * pvParameters);


// Variables
static const char * TAG = "APP";

static hx711_t hx711;
static snapshot_t snapshot;
static Measurement measurement;


static my_uart_t huart;
static CLI * cli_ptr = nullptr;
static cli_ctx_t cli_ctx;
static QueueHandle_t cli_queue;


static task_meas_ctx_t task_meas_ctx;
static task_cli_ctx_t task_cli_ctx;


// Static Funciton
static void
uart_cli_bridge_(void * ctx, const char * data, size_t len)
{
    if ( ctx != nullptr )
    {
        static_cast<CLI *>(ctx)->push(data, len);
    }
}


// App Init
void
app_init()
{
    int res = 0;

    // Init HX711
    hx711_hw_t hx711_hw = { .io_sck = HX711_SCK, .io_dout = HX711_DOUT };
    hx711_set_t hx711_cfg = { .mode = HX711_MODE_A_128, .timeout_ms = 30 };
    res = (int)hx711_init_with_isr( &hx711, &hx711_hw, &hx711_cfg);
    if ( HX711_OK != res )
    {
        ESP_LOGE(TAG, "failed to init hx711 with error (%d)", res);
        esp_restart();
    }

    SemaphoreHandle_t hx711_mutex = xSemaphoreCreateMutex();
    if ( hx711_mutex == NULL )
    {
        ESP_LOGE(TAG, "failed to create hx711_mutex");
        esp_restart();
    }

    // Init Snapshot
    res = (int)snapshot_init(&snapshot);
    if ( SNAP_OK != res )
    {
        ESP_LOGE(TAG, "failed to init snapshot with error (%d)", res);
        esp_restart();
    }

    // Init Measurement instance with mutex
    SemaphoreHandle_t meas_mutex = xSemaphoreCreateMutex();
    if ( meas_mutex == NULL )
    {
        ESP_LOGE(TAG, "failed to create meas_mutex");
        esp_restart();
    }

    // Init Measurement context and task
    task_meas_ctx = { .hx711 = &hx711, .meas = &measurement, .snap = &snapshot, .hx711_mtx = hx711_mutex, .meas_mtx = meas_mutex};
    if ( pdTRUE != xTaskCreate(taskMeas, "MEAS", 2048, (void *)&task_meas_ctx, 8, NULL) )
    {
        ESP_LOGE(TAG, "failed to create task_meas");
        esp_restart();  
    }

    // Init UART with CLI
    cli_queue = xQueueCreate(8, CLI_RX_LINE_MAX);
    if ( !cli_queue )
    {
        ESP_LOGE(TAG, "cli queue init failed");
        esp_restart();
    }

    cli_ctx = {
        .hx711 = &hx711,
        .meas = &measurement,
        .snap = &snapshot,
        .hx711_mtx = hx711_mutex,
        .meas_mtx = meas_mutex
    };

    huart = uart_default_dev(nullptr);
    res = (int)uart_init(&huart);
    if ( UART_OK != res )
    {
        ESP_LOGE(TAG, "uart init failed with error (%d)", res);
        esp_restart();
    }

    static CLI cli(huart, cli_ctx, cli_queue);
    cli_ptr = &cli;

    res = (int)uart_set_callback(&huart, uart_cli_bridge_, cli_ptr);
    if ( UART_OK != res )
    {
        ESP_LOGE(TAG, "uart callback set failed with error (%d)", res);
        esp_restart();
    }

    res = uart_start_task(&huart);
    if ( UART_OK != res )
    {
        ESP_LOGE(TAG, "uart start task failed with error (%d)", res);
        esp_restart();
    }

    // Init CLI context and task
    task_cli_ctx = { .cli = cli_ptr, .queue = cli_queue };
    if ( pdTRUE != xTaskCreate(taskCli, "CLI", 2048, (void *)&task_cli_ctx, 8, NULL) )
    {
        ESP_LOGE(TAG, "failed to create task_cli");
        esp_restart();  
    }


}