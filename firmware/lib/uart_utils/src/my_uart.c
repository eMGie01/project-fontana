#include "my_uart.h"

#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"


#define FLUSH_RESET(s)                          \
    do {                                        \
        uart_flush_input(s->config.port);       \
        xQueueReset(s->runtime.handles.queue);  \
    } while (0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))


static void
uart_event_task_ (void * pvParameters)
{
    if ( !pvParameters )
    {
        vTaskDelete(NULL);
    }

    my_uart_t * self = (my_uart_t *)pvParameters;
    uart_event_t event;

    /* buffer size max 128 */
    char rx_buffer[MIN(self->config.settings.rx_buffer_size, MAX_EVENT_BUFF_SIZE)];
    for ( ;; )
    {

        if ( xQueueReceive(self->runtime.handles.queue, &event, portMAX_DELAY) )
        {
            switch ( event.type )
            {

                case UART_DATA:

                    size_t to_read = MIN((size_t)event.size, sizeof(rx_buffer));
                    int len = uart_read_bytes(
                        self->config.port, 
                        rx_buffer, 
                        to_read, 
                        portMAX_DELAY
                    );

                    if ( len > 0 && self->config.callback != NULL)
                    {
                        self->config.callback(self->config.ctx, rx_buffer, (size_t)len);
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
}


uart_err_t 
uart_init(my_uart_t * dev)
{
    if ( !dev )
    {
        return UART_INVALID_ARG;
    }

    if ( 0 == dev->config.cfg.baud_rate )
    {
        return UART_INVALID_CONFIG;
    }

    if ( !GPIO_IS_VALID_OUTPUT_GPIO(dev->config.ios.tx) || !GPIO_IS_VALID_GPIO(dev->config.ios.rx) )
    {
        return UART_HW_ERROR;
    }

    if ( ESP_OK != uart_param_config(dev->config.port, &dev->config.cfg) )
    {
        return UART_INVALID_CONFIG;
    }

    if ( ESP_OK != uart_set_pin(dev->config.port, dev->config.ios.tx, dev->config.ios.rx, -1, -1) )
    {
        return UART_HW_ERROR;
    }

    if ( dev->config.settings.rx_buffer_size > MAX_EVENT_BUFF_SIZE || dev->config.settings.rx_buffer_size <= 0 ) 
    {
        return UART_INVALID_RX_BUFFER_SIZE;
    }

    if ( dev->config.settings.tx_buffer_size > MAX_EVENT_BUFF_SIZE || dev->config.settings.tx_buffer_size <= 0 ) 
    {
        return UART_INVALID_TX_BUFFER_SIZE;
    }

    dev->runtime.handles.queue = NULL;
    dev->runtime.handles.task = NULL;
    dev->runtime.initialized = false;


    if ( ESP_OK != uart_driver_install(
        dev->config.port, dev->config.settings.rx_buffer_size, 
        dev->config.settings.tx_buffer_size, dev->config.settings.queue_size,
    &dev->runtime.handles.queue, 0) )
    {
        return UART_RUNTIME_ERR;
    }

    dev->runtime.initialized = true;
    return UART_OK;
}

uart_err_t
uart_set_callback(my_uart_t * dev, uart_rx_cb_t cb, void * ctx)
{
    if ( !dev )
    {
        return UART_INVALID_ARG;
    }

    if ( !dev->runtime.initialized )
    {
        return UART_NOT_INITIALIZED;
    }

    if ( dev->runtime.handles.task != NULL )
    {
        return UART_TASK_RUNNING;
    }

    dev->config.callback = cb;
    dev->config.ctx = ctx;
    return UART_OK;
}


uart_err_t
uart_start_task(my_uart_t * dev)
{
    if ( !dev || dev->runtime.handles.task || !dev->config.settings.name )
    {
        return UART_INVALID_ARG;
    }

    if ( !dev->runtime.initialized )
    {
        return UART_NOT_INITIALIZED;
    }

    BaseType_t res = xTaskCreate(
        uart_event_task_, dev->config.settings.name,
        2048, (void *)dev, 3, &dev->runtime.handles.task
    );

    if ( pdPASS != res )
    {
        return UART_RUNTIME_ERR;
    }

    return UART_OK;
}


uart_err_t 
uart_end_task(my_uart_t * dev)
{
    if ( !dev || !dev->runtime.handles.task )
    {
        return UART_INVALID_ARG;
    }

    if ( !dev->runtime.initialized )
    {
        return UART_NOT_INITIALIZED;
    }

    vTaskDelete(dev->runtime.handles.task);
    dev->runtime.handles.task = NULL;

    return UART_OK;
}


uart_err_t
uart_deinit(my_uart_t * dev)
{
    if ( !dev )
    {
        return UART_INVALID_ARG;
    }

    if ( !dev->runtime.initialized )
    {
        return UART_NOT_INITIALIZED;
    }

    if ( dev->runtime.handles.task )
    {
        uart_err_t res = uart_end_task(dev);
        if ( res != UART_OK )
        {
            return res;
        }
    }

    if ( ESP_OK != uart_driver_delete(dev->config.port) )
    {
        return UART_RUNTIME_ERR;
    }

    dev->runtime.handles.queue = NULL;
    dev->runtime.handles.task = NULL;
    dev->runtime.initialized = false;

    return UART_OK;
}
