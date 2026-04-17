#include "measurement.hpp"
#include "hx711.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#define HX711_DOUT GPIO_NUM_2
#define HX711_SCK  GPIO_NUM_3


// static const char * TAG = "TASK_MEAS";


void 
taskMeas(void * pvParameters)
{

    for (;;)
    {
        hx711_read_raw_isr(&hx711);
    }
}