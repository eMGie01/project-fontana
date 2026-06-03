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

#include "driver/sdspi_host.h"
#include "freertos/FreeRTOS.h"

typedef enum
{
    SD_OK                   =  (0),
    SD_ERR_INVAL_ARG        = (-1),
    SD_ERR_INVAL_STATE      = (-2),
    SD_ERR_INIT_FAIL        = (-3),
    SD_ERR_MOUNT_FAIL       = (-4),
    SD_ERR_NOT_FOUND        = (-5),
    SD_ERR_ALREADY_EXISTS   = (-6),
    SD_ERR_WRITE_FAIL       = (-7),
    SD_ERR_NO_SPACE         = (-8),
    SD_ERR_BUFF_TOO_SMALL   = (-9),
    SD_ERR_NO_CARD          = (-10),
} sd_err_t;

typedef enum
{
    SD_STATE_INITED     = (0),
    SD_STATE_MOUNTED    = (1),
    SD_STATE_UNINIT     = (2),
} sd_state_t;

typedef struct
{
    sd_mode_t mode;
    spi_host_device_t spi_host;
    gpio_num_t pin_cs;
    const char* mount_point;
    int max_files;
    bool format_if_mount_failed;
    size_t allocation_unit_size;
    int freq_khz;
    bool use_pullups;
} sd_config_t;

typedef struct
{
    sdmmc_card_t* card;
    sdmmc_host_t host;
    sd_state_t state;
    sd_config_t config;
    bool mounted;
    SemaphoreHandle_t mutex;
} sd_type_t;

typedef sd_type_t* sd_handle_t;

sd_err_t sd_init(const sd_config_t* config, sd_handle_t sd);
sd_err_t sd_deinit(sd_handle_t sd);

sd_err_t sd_mount(sd_handle_t sd);
sd_err_t sd_unmount(sd_handle_t sd);


#ifdef __cplusplus
}
#endif

#endif // SD_H