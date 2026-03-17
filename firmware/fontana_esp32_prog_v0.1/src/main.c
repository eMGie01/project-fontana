#include "measurements.h"
#include "hx711.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"


static const char * TAG = "MAIN";


int app_main() 
{
    ESP_LOGI(TAG, "program started");
    hx711_t hx711;
    hx711_status_t hx711_response = hx711_init(&hx711, GPIO_NUM_2, GPIO_NUM_3, HX711_MODE_A_128);
    if (hx711_response == HX711_ERR_ARG) {
        ESP_LOGE(TAG, "init of the hx711 driver failed with error: %s (%d)", esp_err_to_name(hx711_response), (int)hx711_response);
        return 1;
    }
    ESP_LOGI(TAG, "init of the hx711 driver success");
    return 0;
}
