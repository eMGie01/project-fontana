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
} sd_mount_config_t;

typedef struct
{
    sdmmc_card_t* card;
    char* mount_point;
} sd_t;
typedef sd_t* sd_handle_t;

esp_err_t sd_mount(sd_handle_t sd, char* mp, spi_host_device_t host);
esp_err_t sd_file_create(sd_handle_t sd, const char* file_name);
esp_err_t sd_file_write();

#ifdef __cplusplus
}
#endif

#endif // SD_H