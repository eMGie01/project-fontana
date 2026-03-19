#include "uart.h"

#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"


static const char *TAG = "UART";
static QueueHandle_t uart_event_queue = NULL;
static uart_t * config = NULL;


static void uart_event_task(void *pvParameters) {
  uart_event_t event;
  char buffer[256];
  for (;;) {
    if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY)) {
      switch (event.type) {
        case UART_DATA:
          size_t len = (size_t)uart_read_bytes(config->port, buffer, sizeof(buffer), portMAX_DELAY);
          if (len > 0) {
            config->callback(buffer, len);
          }
          break;
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
          ESP_LOGW(TAG, "buffer overflow, flushing RX ...");
          uart_flush_input(config->port);
          xQueueReset(uart_event_queue);
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

  if (ESP_OK != uart_driver_install(cfg->port, cfg->rx_buffer_size, cfg->tx_buffer_size, cfg->queue_size, &uart_event_queue, 0)) {
    ESP_LOGE(TAG, "installing uart driver for port %d failed", cfg->port);
    return UART_NOT_INITIALIZED;
  }

  config = cfg;

  ESP_LOGD(TAG, "UART (num: %d) inited successfully", (uint8_t)cfg->port);
  return UART_OK;
}


void uart_start_task(void) {
  xTaskCreate(uart_event_task, "uart_task", 2048, NULL, 3, NULL);
}


uart_err_t uart_send_string(const char *str, size_t size) {
  if (!config) {
    ESP_LOGE(TAG, "sending string failed with error: %d", UART_NOT_INITIALIZED);
    return UART_NOT_INITIALIZED;
  }
  if (0 > uart_write_bytes(config->port, str, size)) {
    ESP_LOGE(TAG, "sending string with port: %d failed with unexpected error", config->port);
    return UART_TX_SEND_ERR;
  }
  return UART_OK;
}
