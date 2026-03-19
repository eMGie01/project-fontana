#include "measurements.hpp"
#include "hx711.h"
#include "uart.h"

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

    /*init*/
    hx711_t hx711;
    if (HX711_ERR_ARG == hx711_init(&hx711, HX711_DOUT, HX711_SCK, HX711_MODE_A_128)) {
        ESP_LOGE(TAG, "initialization of hx711 failed with error: %d", HX711_ERR_ARG);
        for (;;) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    

    Measurement Meas;
    Meas.setCountsPerMmHg(10735);
    Meas.setAvgWindowSize(10);

    int32_t raw_value = 0;
    int64_t real_filtered_value = 0;
    int64_t real_avg_value = 0;

    /*main loop*/
    for (;;) 
    {
        vTaskDelay(pdMS_TO_TICKS(300));

        if (hx711_is_ready(&hx711)) {
            ESP_LOGD(TAG, "hx711 data is ready");

            hx711_status_t read_raw_response = hx711_read_raw(&hx711, &raw_value);
            if (HX711_OK != read_raw_response) {
                ESP_LOGE(TAG, "reading value from HX711 failed with error: %d", read_raw_response);
                continue;
            }

            ESP_LOGD(TAG, "raw_value got from hx711 driver: %d", raw_value);

            Meas.pushRaw(raw_value);

            bool filtered_value_response = Meas.getFilteredValueX1000(real_filtered_value);
            if (filtered_value_response) {
                ESP_LOGD(TAG, "filtered_value: %lld", (long long)real_filtered_value);
            }

            bool avg_value_response = Meas.getAvgValueX1000(real_avg_value);
            if (avg_value_response) {
                ESP_LOGD(TAG, "avg_value: %lld", (long long)real_avg_value);
            }

        } else {
            ESP_LOGD(TAG, "hx711 is not ready");
        }
    }
}
