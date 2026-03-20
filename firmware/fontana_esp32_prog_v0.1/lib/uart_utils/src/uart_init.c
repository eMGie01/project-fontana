#include "my_uart.h"

#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "driver/gpio.h"
#include "esp_err.h"


#define FLUSH_RESET(s)                  \
    do {                                \
        uart_flush_input(s->port);      \
        xQueueReset(s->handles.queue);  \
    } while (0)


static void
uart_event_task (void * pvParameters)
{
    if ( !pvParameters )
    {
        vTaskDelete(NULL);
    }

    my_uart_t * self = (my_uart_t *)pvParameters;
    uart_event_t event;
    char rx_buffer[self->settings.rx_buffer_size];

    for ( ;; )
    {

        if ( xQueueReceive(self->handles.queue, &event, portMAX_DELAY) )
        {
            switch ( event.type )
            {

                case UART_DATA:

                    size_t len = (size_t)uart_read_bytes(self->port, rx_buffer, event.size, portMAX_DELAY);
                    if ( 0 < len )
                    {
                        self->callback(rx_buffer, len);
                    }
                    break;

                case UART_FIFO_OVF:

                    FLUSH_RESET(self);
                    break;

                case UART_BUFFER_FULL:

                    FLUSH_RESET(self);
                    break;

                default:
                    break;
            }
        }
    }

    return;
}


uart_err_t 
uart_init(my_uart_t * dev)
{
    if ( !dev || !dev->callback )
    {
        return UART_INVALID_ARG;
    }

    if ( 0 == dev->cfg.baud_rate )
    {
        return UART_INVALID_CONFIG;
    }

    if ( !GPIO_IS_VALID_OUTPUT_GPIO(dev->ios.tx) || !GPIO_IS_VALID_GPIO(dev->ios.rx) )
    {
        return UART_HW_ERROR;
    }

    if ( ESP_OK != uart_param_config(dev->port, &dev->cfg) )
    {
        return UART_INVALID_CONFIG;
    }

    if ( ESP_OK != uart_set_pin(dev->port, dev->ios.tx, dev->ios.rx, -1, -1) )
    {
        return UART_HW_ERROR;
    }

    dev->handles.queue = NULL;
    dev->handles.task = NULL;
    dev->initialized = false;

    if ( ESP_OK != uart_driver_install(
        dev->port, dev->settings.rx_buffer_size, 
        dev->settings.tx_buffer_size, dev->settings.queue_size,
    &dev->handles.queue, 0) )
    {
        return UART_RUNTIME_ERR;
    }

    dev->initialized = true;
    return UART_OK;
}


uart_err_t
uart_start_task(my_uart_t * dev)
{
    if ( !dev || !dev->handles.task )
    {
        return UART_INVALID_ARG;
    }

    if ( !dev->initialized )
    {
        return UART_NOT_INITIALIZED;
    }

    BaseType_t res = xTaskCreate(
        uart_event_task, dev->settings.name,
        2048, (void *)dev, 3, &dev->handles.task
    );

    if ( pdPASS != res )
    {
        return UART_RUNTIME_ERR;
    }

    return UART_OK;
}