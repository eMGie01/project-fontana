/**
 * @file button.c
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief Simple button driver implementation
 * @version 0.1
 * @date 2026-06-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "button.h"

esp_err_t button_init(gpio_num_t gpio_num)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << gpio_num),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };

    return gpio_config(&io_conf);
}

bool button_read(gpio_num_t gpio_num)
{
    return gpio_get_level(gpio_num) == 0;  // Active low
}
