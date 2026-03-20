#include "my_uart_1.h"

#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "driver/gpio.h"
#include "esp_err.h"


static const char *TAG = "UART";


static void uart_event_task(void *pvParameters) {

  if (!pvParameters) {
    ESP_LOGE(TAG, "task parameters were NULL, destroying the task...");
    vTaskDelete(NULL);
  }
  
  uart_t * self = (uart_t *)pvParameters;
  uart_event_t event;
  char buffer[self->rx_buffer_size];
  for (;;) {
    if (xQueueReceive(self->event_queue, &event, portMAX_DELAY)) {
      switch (event.type) {
        case UART_DATA:
          size_t len = (size_t)uart_read_bytes(self->port, buffer, sizeof(buffer), portMAX_DELAY);
          if (len > 0) {
            self->callback(buffer, len);
          }
          break;
        case (UART_FIFO_OVF || UART_BUFFER_FULL):
          ESP_LOGW(TAG, "buffer event error, flushing RX ...");
          uart_flush_input(self->port);
          xQueueReset(self->event_queue);
          break;
        default:
          break;
      }
    }
  }
}


uart_err_t uart_init(uart_t * cfg) {
  if (!cfg || !cfg->callback) {
    ESP_LOGE(TAG, "init failed: configuration is NULL");
    return UART_ERR_INVALID_PARAM;
  }

  if (0 == cfg->uart_cfg.baud_rate) {
    ESP_LOGE(TAG, "baudrate (%ld) has wrong value", (long)cfg->uart_cfg.baud_rate);
    return UART_ERR_INVALID_PARAM;
  }

  if (!GPIO_IS_VALID_OUTPUT_GPIO(cfg->gpio_tx)) {
    ESP_LOGE(TAG, "gpio_tx (%d) is not a valid output pin", cfg->gpio_tx);
    return UART_ERR_INVALID_PARAM;
  }

  if (!GPIO_IS_VALID_GPIO(cfg->gpio_rx)) {
    ESP_LOGE(TAG, "gpio_rx (%d) is not a valid pin", cfg->gpio_rx);
    return UART_ERR_INVALID_PARAM;
  }

  if (ESP_OK != uart_param_config(cfg->port, &cfg->uart_cfg)) {
    ESP_LOGE(TAG, "setting uart configuration parameters for port: %d failed", (uint8_t)cfg->port);
    return UART_NOT_INITIALIZED;
  }
  if (ESP_OK != uart_set_pin(cfg->port, cfg->gpio_tx, cfg->gpio_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)) {
    ESP_LOGE(TAG, "setting pins (gpio_tx: %d and gpio_rx: %d) for port: %d failed", cfg->gpio_tx, cfg->gpio_rx, (uint8_t)cfg->port);
    return UART_NOT_INITIALIZED;
  }

  cfg->event_queue = NULL;
  cfg->event_task = NULL;
  cfg->initialized = false;
  if (ESP_OK != uart_driver_install(cfg->port, cfg->rx_buffer_size, cfg->tx_buffer_size, cfg->queue_size, &cfg->event_queue, 0)) {
    ESP_LOGE(TAG, "installing uart driver for port %d failed", cfg->port);
    return UART_NOT_INITIALIZED;
  }

  cfg->initialized = true;
  ESP_LOGD(TAG, "UART (num: %d) inited successfully", (uint8_t)cfg->port);
  return UART_OK;
}


uart_err_t uart_start_task(uart_t *cfg) {
  if (!cfg || !cfg->initialized) {
    ESP_LOGE(TAG, "creating uart task failed with error: %d", UART_NOT_INITIALIZED);
    return UART_NOT_INITIALIZED;
  }
  if (pdPASS != xTaskCreate(uart_event_task, "uart_task", 2048, (void *)cfg, 3, &cfg->event_task)) {
    ESP_LOGE(TAG, "creating uart task failed with error: %d", UART_UNIDENTIFIED_ERROR);
    return UART_UNIDENTIFIED_ERROR;
  }
  return UART_OK;
}


uart_err_t uart_send_string(uart_t * cfg, const char * str, size_t size) {
  if (!cfg || !cfg->initialized) {
    ESP_LOGE(TAG, "sending string failed with error: %d", UART_NOT_INITIALIZED);
    return UART_NOT_INITIALIZED;
  } else if (str == NULL) {
    ESP_LOGE(TAG, "sending string failed with error: %d, str: NULL", UART_ERR_INVALID_PARAM);
    return UART_ERR_INVALID_PARAM;
  } else if (size == 0) {
    ESP_LOGE(TAG, "sending string failed with error: %d, str: %s", UART_ERR_INVALID_PARAM, str);
    return UART_ERR_INVALID_PARAM;
  }
  if (0 > uart_write_bytes(cfg->port, str, size)) {
    ESP_LOGE(TAG, "sending string with port: %d failed with unexpected error", cfg->port);
    return UART_TX_SEND_ERR;
  }
  return UART_OK;
}
