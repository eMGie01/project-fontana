/**
 * @file sd.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SD_H
#define SD_H

#include "driver/gpio.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * open,
 * close,
 * mount
 * unmount,
 * is_mounted
 */

typedef struct
{
    bool format_if_mount_failed;
    int max_nr_of_files;
    size_t allocation_unit_size;
    gpio_num_t io_miso;
    gpio_num_t io_mosi;
    gpio_num_t io_sclk;
    gpio_num_t io_cs;
    size_t max_transfer_sz;
    

} sd_mount_config_t;

esp_err_t sd_init();

#ifdef __cplusplus
}
#endif

#endif // SD_H