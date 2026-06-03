#include "sd.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
// #include "driver/sdspi_host.h"
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

    sd->host = SDSPI_DEFAULT_HOST();
    sd->host.slot = config->spi_host;
    sd->host.max_freq_khz = config->freq_khz;
    sdspi_device_config_t dev_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    dev_cfg.host_id = config->spi_host;
    dev_cfg.gpio_cs = config->pin_cs;
    
    esp_err_t res = sdspi_host_init_device(&dev_cfg, &sd->host.slot);
    if (res != ESP_OK)
    {
        return SD_ERR_INIT_FAIL;
    }

    sd->card = NULL;
    sd->mounted = false;
    sd->mutex = xSemaphoreCreateMutex();
    if (sd->mutex == NULL)
    {
        sdspi_host_remove_device(&sd->host.slot);
        return SD_ERR_INIT_FAIL;
    }

    memcpy(&sd->config, config, sizeof(sd_config_t));

}

esp_err_t sd_mount(sd_handle_t sd, char* mp, spi_host_device_t host_id)
{
    if (sd == NULL || mp == NULL)
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
        sd->card = NULL;
        sd->mount_point == NULL;
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t sd_file_create(sd_handle_t sd, char* file_name, char* file_path)
{
    if (sd == NULL || sd->card == NULL || sd->mount_point == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (strlen(file_name) > 58)
    {
        return ESP_ERR_INVALID_ARG;
    }
    DIR* d = opendir(sd->mount_point);
    struct dirent* dir;
    int max_index = -1;
    char buff[64] = {0};
    snprintf(buff, sizeof(buff), "%s_%%d.csv", file_name);
    if (d)
    {
        while((dir = readdir(d)) != NULL)
        {
            int index;
            if (sscanf(dir->d_name, buff, &index) == 1)
            {
                max_index = index;
            }
        }
        closedir(d);
    }
    else
    {
        ESP_LOGD(TAG, "cannot open file for dir: %s", sd->mount_point);
    }
    int next_index = (max_index == -1) ? 1 : (max_index + 1);
    char path[128];
    snprintf(path, sizeof(path), "%s/%s_%02d.csv", sd->mount_point, file_name, next_index);
    FILE* f = fopen(path, "w");
    if (f != NULL)
    {
        ESP_LOGD(TAG, "new file created in dir: %s", path);
        fclose(f);
        file_path = path;
        return ESP_OK;
    }
    ESP_LOGD(TAG, "creating file with path: %s failed", path);
    return ESP_FAIL;
}

esp_err_t
sd_file_write(sd_handle_t sd, char* file_path)
{
    FILE* f = fopen(file_path, "w");
    if (f == NULL)
    {
        ESP_LOGD(TAG, "writing to file: %s failed", file_path);
    }

}


