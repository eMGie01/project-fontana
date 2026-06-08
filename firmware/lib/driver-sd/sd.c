#include "sd.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"
#include "hw_config.h"

static const char* TAG = "SD";

#define SLASH 47 // ASCII value for slash
#define SD_DATA_DIR_NAME "data"
#define SD_FILE_BASENAME_PREFIX "dat_"

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

static sd_err_t
s_configure_pullups(const sd_config_t* config)
{
    if (!config->use_pullups)
    {
        return SD_OK;
    }

    esp_err_t err = gpio_set_pull_mode(config->pin_cs, GPIO_PULLUP_ONLY);
    if (err == ESP_OK && config->spi_host == SPI2_HOST)
    {
        err = gpio_set_pull_mode(SPI2_MISO, GPIO_PULLUP_ONLY);
    }
    if (err == ESP_OK && config->spi_host == SPI2_HOST)
    {
        err = gpio_set_pull_mode(SPI2_MOSI, GPIO_PULLUP_ONLY);
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "pull-up configuration failed: %s", esp_err_to_name(err));
        return SD_ERR_INIT_FAIL;
    }

    return SD_OK;
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

    sd->host = (sdmmc_host_t)SDSPI_HOST_DEFAULT();
    sd->host.slot = config->spi_host;
    sd->host.max_freq_khz = config->freq_khz;

    sd->card = NULL;
    sd->mounted = false;

    sd_err_t cfg_err = s_configure_pullups(config);
    if (cfg_err != SD_OK)
    {
        return cfg_err;
    }

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

    sdspi_device_config_t dev_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    dev_cfg.host_id = sd->config.spi_host;
    dev_cfg.gpio_cs = sd->config.pin_cs;

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = sd->config.format_if_mount_failed,
        .max_files = sd->config.max_files,
        .allocation_unit_size = sd->config.allocation_unit_size,
    };

    esp_err_t err = esp_vfs_fat_sdspi_mount(
        sd->config.mount_point,
        &sd->host,
        &dev_cfg,
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

static sd_err_t
s_ensure_directory(const char* path)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            return SD_OK;
        }

        ESP_LOGE(TAG, "%s exists but is not a directory", path);
        return SD_ERR_ALREADY_EXISTS;
    }

    if (mkdir(path, 0777) == 0)
    {
        return SD_OK;
    }

    if (errno == EEXIST && stat(path, &st) == 0 && S_ISDIR(st.st_mode))
    {
        return SD_OK;
    }

    ESP_LOGE(TAG, "mkdir failed for %s: errno=%d (%s)", path, errno, strerror(errno));
    return SD_ERR_WRITE_FAIL;
}

sd_err_t
sd_file_create(sd_handle_t sd, sd_file_handle_t* handle)
{
    ESP_LOGD(TAG, "sd_file_create");
    
    if (sd == NULL || handle == NULL)
    {
        ESP_LOGE(TAG, "sd_file_create: invalid arguments");
        return SD_ERR_INVAL_ARG;
    }
    
    if (sd->state != SD_STATE_MOUNTED || !sd->mounted)
    {
        ESP_LOGE(TAG, "sd_file_create: SD not mounted");
        return SD_ERR_MOUNT_FAIL;
    }
    
    // Thread-safe access
    if (xSemaphoreTake(sd->mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "sd_file_create: mutex take failed");
        return SD_ERR_INVAL_STATE;
    }
    
    char data_dir[256];
    int path_len = snprintf(data_dir, sizeof(data_dir), "%s/%s", sd->config.mount_point, SD_DATA_DIR_NAME);
    if (path_len < 0 || path_len >= (int)sizeof(data_dir))
    {
        ESP_LOGE(TAG, "sd_file_create: data directory path too long");
        xSemaphoreGive(sd->mutex);
        return SD_ERR_BUFF_TOO_SMALL;
    }

    sd_err_t dir_err = s_ensure_directory(data_dir);
    if (dir_err != SD_OK)
    {
        xSemaphoreGive(sd->mutex);
        return dir_err;
    }
    
    char filepath[256];
    int next_num = -1;
    for (int num = 0; num <= 9999; ++num)
    {
        path_len = snprintf(filepath, sizeof(filepath), "%s/%s%04d.csv", data_dir, SD_FILE_BASENAME_PREFIX, num);
        if (path_len < 0 || path_len >= (int)sizeof(filepath))
        {
            ESP_LOGE(TAG, "sd_file_create: file path too long");
            xSemaphoreGive(sd->mutex);
            return SD_ERR_BUFF_TOO_SMALL;
        }

        struct stat st;
        if (stat(filepath, &st) != 0)
        {
            if (errno == ENOENT)
            {
                next_num = num;
                break;
            }

            ESP_LOGE(TAG, "stat failed for %s: errno=%d (%s)", filepath, errno, strerror(errno));
            xSemaphoreGive(sd->mutex);
            return SD_ERR_WRITE_FAIL;
        }
    }

    if (next_num < 0)
    {
        ESP_LOGE(TAG, "sd_file_create: reached max files limit (9999)");
        xSemaphoreGive(sd->mutex);
        return SD_ERR_NO_SPACE;
    }

    ESP_LOGI(TAG, "Creating file: %s", filepath);
    
    // Open file for writing
    FILE* fp = fopen(filepath, "w");
    if (fp == NULL)
    {
        ESP_LOGE(TAG, "sd_file_create: fopen failed for %s: errno=%d (%s)", filepath, errno, strerror(errno));
        xSemaphoreGive(sd->mutex);
        return SD_ERR_WRITE_FAIL;
    }
    
    // Write CSV header
    // Format: time_ms, code, filtered, averaged, offset, counts, iirShift, avgWin
    int header_ret = fprintf(fp, "time_ms,code,filtered,averaged,offset,counts,iirShift,avgWin\n");
    if (header_ret < 0)
    {
        ESP_LOGE(TAG, "sd_file_create: fprintf header failed");
        fclose(fp);
        xSemaphoreGive(sd->mutex);
        return SD_ERR_WRITE_FAIL;
    }
    
    // Populate output handle
    handle->fp = fp;
    handle->write_count = 0;
    strncpy(handle->filepath, filepath, sizeof(handle->filepath) - 1);
    handle->filepath[sizeof(handle->filepath) - 1] = '\0';
    
    xSemaphoreGive(sd->mutex);
    ESP_LOGI(TAG, "File created successfully: %s", filepath);
    return SD_OK;
}

sd_err_t
sd_file_write_block(sd_file_handle_t* handle, const char* data, size_t len, uint32_t sample_count)
{
    if (handle == NULL || data == NULL || len == 0 || handle->fp == NULL)
    {
        ESP_LOGE(TAG, "sd_file_write_block: invalid arguments");
        return SD_ERR_INVAL_ARG;
    }

    size_t written = fwrite(data, 1, len, handle->fp);
    if (written != len)
    {
        ESP_LOGE(TAG, "sd_file_write_block: fwrite failed for file %s (%u/%u bytes)",
                 handle->filepath,
                 (unsigned)written,
                 (unsigned)len);
        return SD_ERR_WRITE_FAIL;
    }

    handle->write_count += sample_count;

    if (fflush(handle->fp) != 0)
    {
        ESP_LOGE(TAG, "sd_file_write_block: fflush failed");
        return SD_ERR_WRITE_FAIL;
    }

    ESP_LOGD(TAG, "sd_file_write_block: flushed %u samples (%u bytes)",
             (unsigned)sample_count,
             (unsigned)len);
    return SD_OK;
}

sd_err_t
sd_file_close(sd_file_handle_t* handle)
{
    // Validate arguments
    if (handle == NULL || handle->fp == NULL)
    {
        ESP_LOGE(TAG, "sd_file_close: invalid arguments");
        return SD_ERR_INVAL_ARG;
    }
    
    ESP_LOGD(TAG, "sd_file_close: closing file %s (write_count=%lu)", 
             handle->filepath, handle->write_count);
    
    // Final flush
    if (fflush(handle->fp) != 0)
    {
        ESP_LOGE(TAG, "sd_file_close: final fflush failed");
        return SD_ERR_WRITE_FAIL;
    }
    
    // Close file
    if (fclose(handle->fp) != 0)
    {
        ESP_LOGE(TAG, "sd_file_close: fclose failed");
        return SD_ERR_WRITE_FAIL;
    }
    
    // Clear handle
    handle->fp = NULL;
    handle->write_count = 0;
    memset(handle->filepath, 0, sizeof(handle->filepath));
    
    ESP_LOGI(TAG, "File closed successfully");
    return SD_OK;
}

sd_err_t
sd_deinit(sd_handle_t sd)
{
    ESP_LOGD(TAG, "sd_deinit");
    
    if (sd == NULL)
    {
        return SD_ERR_INVAL_ARG;
    }
    
    // If SD is mounted -> unmount
    if (sd->state == SD_STATE_MOUNTED)
    {
        sd_unmount(sd);
    }
    
    // Take mutex one last time
    if (sd->mutex != NULL)
    {
        xSemaphoreTake(sd->mutex, portMAX_DELAY);
        // Not giving it back - we're deleting it
    }
    
    // Delete mutex
    if (sd->mutex != NULL)
    {
        vSemaphoreDelete(sd->mutex);
        sd->mutex = NULL;
    }
    
    // Reset state
    sd->state = SD_STATE_UNINIT;
    sd->card = NULL;
    
    ESP_LOGI(TAG, "sd_deinit completed");
    return SD_OK;
}
