#include "cli_interface.hpp"
#include "measurement.hpp"
#include "my_uart.h"
#include "hx711.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"

static const char * TAG = "MAIN";

extern void app_init();


extern "C" void
app_main()
{
    ESP_LOGI(TAG, "program started");

    if (ESP_OK != gpio_install_isr_service(0) )
    {
        ESP_LOGE(TAG, "failed to install isr service");
        esp_restart();
    }

    app_init();

    //
    vTaskDelete(NULL);
}
