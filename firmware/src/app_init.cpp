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


// Variables
static const char * TAG = "APP";

static hx711_t hx711 = {0};
static Measurement measurement;
static task_meas_ctx_t meas_ctx;


// App Init
void
app_init()
{
    int res = 0;

    // Init HX711
    hx711_hw_t hx711_hw = { .io_sck = HX711_SCK, .io_dout = HX711_DOUT };
    hx711_set_t hx711_cfg = { .mode = HX711_MODE_A_128, .timeout_ms = 100 };
    res = (int)hx711_init_with_isr( &hx711, &hx711_hw, &hx711_cfg);
    if ( HX711_OK != res )
    {
        ESP_LOGE(TAG, "failed to init hx711 with error (%d)", res);
        esp_restart();
    }

    // Init Measurement instance with mutex
    SemaphoreHandle_t meas_mutex = xSemaphoreCreateBinary();

    // Init Measurement context and task
    meas_ctx = { .adc = &hx711, .meas = &measurement, .meas_mtx = meas_mutex};
    xTaskCreate(taskMeas, "MEAS", 2048, (void *)&meas_ctx, 8, NULL);
}