#include "sd.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

esp_err_t 
sd_init(const sd_mount_config_t cfg, char* mount_point)
{
    esp_err_t err;

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 32,
        .allocation_unit_size = (16 * 1024),
    };

    sdmmc_card_t* card;
    const char mount_point[] = "/sdcard";
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.unaligned_multi_block_rw_max_chunk_size = 8;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = cfg.io_mosi,
        .miso_io_num = cfg.io_miso,
        .sclk_io_num = cfg.io_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    err = spi_bus_initialize(host.slot, &bus_cfg, &card);
}