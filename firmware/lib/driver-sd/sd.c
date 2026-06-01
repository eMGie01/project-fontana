#include "sd.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"

#include "hw_config.h"

static const char* TAG = "SD";

esp_err_t sd_mount(sd_handle_t sd, char* mp, spi_host_device_t host_id)
{
    if (sd == nullptr || mp == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t err;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = host_id;
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GPIO_NUM_4;
    slot_config.host_id = host_id;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 32,
        .allocation_unit_size = 16 * 1024,
    };
    sd->mount_point = mp;
    err = esp_vfs_fat_sdspi_mount(sd->mount_point, &host, &slot_config, &mount_config, sd->card);
    if (err != ESP_OK)
    {
        sd->card = nullptr;
        sd->mount_point == nullptr;
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t sd_file_create(sd_handle_t sd, const char* file_name)
{
    if (sd == nullptr || sd->card == nullptr || sd->mount_point == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }
    const char* fname = file_name;
    if (fname == nullptr)
    {
        // looking for next file
    }
}
