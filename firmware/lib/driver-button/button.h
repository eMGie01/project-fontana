/**
 * @file button.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief Simple active-low button GPIO wrapper
 * @version 0.1
 * @date 2026-06-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include "esp_err.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize button GPIO as input with pull-up
 * 
 * @param gpio_num GPIO pin number
 * @return ESP_OK on success, error otherwise
 */
esp_err_t button_init(gpio_num_t gpio_num);

/**
 * @brief Read button state (active low)
 * 
 * @param gpio_num GPIO pin number
 * @return true if pressed (low), false if released (high)
 */
bool button_read(gpio_num_t gpio_num);

#ifdef __cplusplus
}
#endif

#endif // BUTTON_H
