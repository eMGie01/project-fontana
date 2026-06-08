/**
 * @file sd.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief SD card driver header - file operations interface
 * @version 0.2
 * @date 2026-06-08
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
#include "sdmmc_cmd.h"
#include "freertos/FreeRTOS.h"

#define SD_CONFIG_DEFAULT(cs_pin) {            \
    .spi_host = SPI2_HOST,                     \
    .pin_cs = (cs_pin),                        \
    .mount_point = "/sdcard",                  \
    .max_files = 5,                            \
    .format_if_mount_failed = true,           \
    .allocation_unit_size = 16 * 1024,         \
    .freq_khz = SDMMC_FREQ_DEFAULT,            \
    .use_pullups = false,                      \
}

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
    SD_STATE_UNINIT     = (0),
    SD_STATE_INITED     = (1),
    SD_STATE_MOUNTED    = (2),
} sd_state_t;

typedef struct
{
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

/**
 * @brief File handle structure for tracking open files
 * Contains FILE* and metadata for flushing every N writes
 */
typedef struct
{
    FILE* fp;                           // FILE descriptor for the file
    uint32_t write_count;               // Counter of write operations (for flush strategy)
    char filepath[256];                 // Full path to file (for logging)
} sd_file_handle_t;

sd_err_t sd_init(const sd_config_t* config, sd_handle_t sd);
sd_err_t sd_mount(sd_handle_t sd);
sd_err_t sd_unmount(sd_handle_t sd);
sd_err_t sd_deinit(sd_handle_t sd);

/**
 * @brief Creates a new CSV file in /sdcard/data/dat_XXXX.csv
 * 
 * @param sd SD handle
 * @param handle Output - pointer to sd_file_handle_t
 * 
 * @return sd_err_t
 *         - SD_OK on success
 *         - SD_ERR_MOUNT_FAIL if SD not mounted
 *         - SD_ERR_WRITE_FAIL if cannot open file
 *         - SD_ERR_NO_SPACE if insufficient space on SD
 */
sd_err_t sd_file_create(sd_handle_t sd, sd_file_handle_t* handle);

/**
 * @brief Writes a buffered CSV block to an already open file and flushes it to SD
 *
 * @param handle File handle from sd_file_create()
 * @param data Buffered data block
 * @param len Number of bytes to write
 * @param sample_count Number of samples contained in the block
 *
 * @return sd_err_t
 *         - SD_OK on success
 *         - SD_ERR_INVAL_ARG if handle/data NULL or len is 0
 *         - SD_ERR_WRITE_FAIL if fwrite() or fflush() fails
 */
sd_err_t sd_file_write_block(sd_file_handle_t* handle, const char* data, size_t len, uint32_t sample_count);

/**
 * @brief Closes file and flushes remaining data
 * 
 * @param handle File handle to close
 * 
 * @return sd_err_t
 *         - SD_OK on success
 *         - SD_ERR_INVAL_ARG if handle NULL
 *         - SD_ERR_WRITE_FAIL if fclose() fails
 */
sd_err_t sd_file_close(sd_file_handle_t* handle);


#ifdef __cplusplus
}
#endif

#endif // SD_H
