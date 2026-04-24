#include "app.hpp"
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
static SemaphoreHandle_t hx711_mutex;
static SemaphoreHandle_t meas_mutex;
static TaskHandle_t task_meas_handle;
static TaskHandle_t task_cli_handle;
static bool isr_service_installed;


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


void
app_deinit()
{
    if ( task_cli_handle != NULL )
    {
        vTaskDelete(task_cli_handle);
        task_cli_handle = NULL;
    }

    if ( task_meas_handle != NULL )
    {
        vTaskDelete(task_meas_handle);
        task_meas_handle = NULL;
    }

    if ( huart.runtime.initialized )
    {
        if ( UART_OK != uart_deinit(&huart) )
        {
            ESP_LOGW(TAG, "uart deinit failed");
        }
    }

    cli_ptr = nullptr;

    if ( cli_queue != NULL )
    {
        vQueueDelete(cli_queue);
        cli_queue = NULL;
    }

    if ( meas_mutex != NULL )
    {
        vSemaphoreDelete(meas_mutex);
        meas_mutex = NULL;
    }

    if ( snapshot.initialized )
    {
        if ( SNAP_OK != snapshot_deinit(&snapshot) )
        {
            ESP_LOGW(TAG, "snapshot deinit failed");
        }
    }

    if ( hx711_mutex != NULL )
    {
        vSemaphoreDelete(hx711_mutex);
        hx711_mutex = NULL;
    }

    if ( hx711.initialized )
    {
        if ( HX711_OK != hx711_deinit(&hx711) )
        {
            ESP_LOGW(TAG, "hx711 deinit failed");
        }
    }

    if ( isr_service_installed )
    {
        gpio_uninstall_isr_service();
        isr_service_installed = false;
    }
}


// App Init
init_status_t
app_init()
{
    int res = 0;

    if (ESP_OK != gpio_install_isr_service(0) )
    {
        ESP_LOGE(TAG, "failed to install isr service");
        return INIT_RESTART;
    }
    isr_service_installed = true;

    // Init HX711
    hx711_hw_t hx711_hw = { .io_sck = HX711_SCK, .io_dout = HX711_DOUT };
    hx711_set_t hx711_cfg = { .mode = HX711_MODE_A_128, .timeout_ms = 100 };
    res = (int)hx711_init_with_isr( &hx711, &hx711_hw, &hx711_cfg);
    if ( HX711_OK != res )
    {
        ESP_LOGE(TAG, "failed to init hx711 with error (%d)", res);
        goto fail;
    }

    hx711_mutex = xSemaphoreCreateMutex();
    if ( hx711_mutex == NULL )
    {
        ESP_LOGE(TAG, "failed to create hx711_mutex");
        goto fail;
    }

    // Init Snapshot
    res = (int)snapshot_init(&snapshot);
    if ( SNAP_OK != res )
    {
        ESP_LOGE(TAG, "failed to init snapshot with error (%d)", res);
        goto fail;
    }

    // Init Measurement instance with mutex
    meas_mutex = xSemaphoreCreateMutex();
    if ( meas_mutex == NULL )
    {
        ESP_LOGE(TAG, "failed to create meas_mutex");
        goto fail;
    }

    // Init Measurement context and task
    task_meas_ctx = { .hx711 = &hx711, .meas = &measurement, .snap = &snapshot, .hx711_mtx = hx711_mutex, .meas_mtx = meas_mutex};
    if ( pdTRUE != xTaskCreate(taskMeas, "MEAS", 2048, (void *)&task_meas_ctx, 8, &task_meas_handle) )
    {
        ESP_LOGE(TAG, "failed to create task_meas");
        goto fail;
    }

    // Init UART with CLI
    cli_queue = xQueueCreate(8, CLI_RX_LINE_MAX);
    if ( !cli_queue )
    {
        ESP_LOGE(TAG, "cli queue init failed");
        goto fail;
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
        goto fail;
    }

    static CLI cli(huart, cli_ctx);
    cli.updateQueue(cli_queue);
    cli_ptr = &cli;

    res = (int)uart_set_callback(&huart, uart_cli_bridge_, cli_ptr);
    if ( UART_OK != res )
    {
        ESP_LOGE(TAG, "uart callback set failed with error (%d)", res);
        goto fail;
    }

    res = uart_start_task(&huart);
    if ( UART_OK != res )
    {
        ESP_LOGE(TAG, "uart start task failed with error (%d)", res);
        goto fail;
    }

    // Init CLI context and task
    task_cli_ctx = { .cli = cli_ptr, .queue = cli_queue };
    if ( pdTRUE != xTaskCreate(taskCli, "CLI", 2048, (void *)&task_cli_ctx, 8, &task_cli_handle) )
    {
        ESP_LOGE(TAG, "failed to create task_cli");
        goto fail;
    }

    return INIT_DONE;

fail:
    app_deinit();
    return INIT_RESTART;
}
