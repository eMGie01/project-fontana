#include "sd.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "hw_config.h"

static const char* TAG = "SD";

#define SLASH 47 // ascii value for slash

static bool
s_check_config(const sd_config_t* config)
{
    if (config->spi_host > SPI_HOST_MAX || config->spi_host == SPI1_HOST)
    {
        return false;
    }
    if (!GPIO_IS_VALID_GPIO(config->pin_cs))
    {
        return false;
    }
    if (config->mount_point == NULL || config->mount_point[0] != SLASH)
    {
        return false;
    }
    if (config->max_files > 256 || config->max_files <= 0)
    {
        return false;
    }
    if (config->freq_khz <= 0)
    {
        return false;
    }
    return true;
}

sd_err_t
sd_init(const sd_config_t* config, sd_handle_t sd)
{
    ESP_LOGD(TAG, "sd card init");
    if (sd == NULL || config == NULL)
    {
        ESP_LOGD(TAG, "invalid initial arguments");
        return SD_ERR_INVAL_ARG;
    }

    if (sd->state != SD_STATE_UNINIT)
    {
        return SD_ERR_INVAL_STATE;
    }

    if (!s_check_config(config))
    {
        return SD_ERR_INVAL_ARG;
    }

    sd->host = (sdmmc_host_t) SDSPI_HOST_DEFAULT();
    sd->host.slot = config->spi_host;
    // sd->host.max_freq_khz = config->freq_khz;
    sd->host.max_freq_khz = SDMMC_FREQ_PROBING;
    // sdspi_device_config_t dev_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    // dev_cfg.host_id = config->spi_host;
    // dev_cfg.gpio_cs = config->pin_cs;
    
    // esp_err_t res = sdspi_host_init_device(&dev_cfg, &sd->host.slot);
    // if (res != ESP_OK)
    // {
    //     return SD_ERR_INIT_FAIL;
    // }

    sd->card = NULL;
    sd->mounted = false;
    sd->mutex = xSemaphoreCreateMutex();
    if (sd->mutex == NULL)
    {
        sdspi_host_remove_device(sd->host.slot);
        return SD_ERR_INIT_FAIL;
    }

    memcpy(&sd->config, config, sizeof(sd_config_t));
    sd->state = SD_STATE_INITED;
    ESP_LOGI(TAG, "sd driver installed, cs=GPIO%d, host=%d", config->pin_cs, config->spi_host);
    return SD_OK;
}

sd_err_t
sd_mount(sd_handle_t sd)
{
    if (sd == NULL)
    {
        return SD_ERR_INVAL_ARG;
    }

    if (sd->state != SD_STATE_INITED)
    {
        return SD_ERR_INVAL_STATE;
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = sd->config.format_if_mount_failed,
        .max_files = sd->config.max_files,
        .allocation_unit_size = sd->config.allocation_unit_size,
    };

    esp_err_t err = esp_vfs_fat_sdspi_mount(
        sd->config.mount_point,
        &sd->host,
        &(sdspi_device_config_t){
            .host_id = sd->config.spi_host,
            .gpio_cs = sd->config.pin_cs,
        },
        &mount_cfg,
        &sd->card
    );

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "mount failed: %s", esp_err_to_name(err));
        return SD_ERR_MOUNT_FAIL;
    }

    sd->mounted = true;
    sd->state = SD_STATE_MOUNTED;
    ESP_LOGI(TAG, "mounted at %s", sd->config.mount_point);
    sdmmc_card_print_info(stdout, sd->card);
    return SD_OK;
}

sd_err_t
sd_unmount(sd_handle_t sd)
{
    if (sd == NULL)
    {
        return SD_ERR_INVAL_ARG;
    }

    if (sd->state != SD_STATE_MOUNTED)
    {
        return SD_ERR_INVAL_STATE;
    }

    esp_err_t err = esp_vfs_fat_sdcard_unmount(sd->config.mount_point, sd->card);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "unmount failed: %s", esp_err_to_name(err));
        return SD_ERR_MOUNT_FAIL;
    }

    sd->card = NULL;
    sd->mounted = false;
    sd->state = SD_STATE_INITED;
    ESP_LOGI(TAG, "unmounted");
    return SD_OK;
}

sd_err_t
sd_file_create(sd_handle_t sd)
{
    if (!sd->mounted || sd->state != SD_STATE_MOUNTED)
    {
        return SD_ERR_INVAL_STATE;
    }
    // DIR* dir = opendir(sd->config.mount_point);
    // if (dir == NULL)
    // {
    //     ESP_LOGE(TAG, "opendir failed");
    //     return SD_ERR_WRITE_FAIL;
    // }

    // struct dirent* entry;
    // while ((entry = readdir(dir)) != NULL)
    // {
    //     ESP_LOGI(TAG, "%s %s",
    //         entry->d_type == DT_DIR ? "[DIR]" : "[FILE]",
    //         entry->d_name);
    // }
    // closedir(dir);
    return SD_OK;
}
