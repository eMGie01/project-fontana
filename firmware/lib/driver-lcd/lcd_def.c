#include "lcd.h"

lcd_cfg_t lcd_cfg = {
    .spi_host = SPI2_HOST,
    .pin_sclk = GPIO_NUM_7,
    .pin_mosi = GPIO_NUM_6,
    .pin_miso = GPIO_NUM_5,
    .pin_cs   = GPIO_NUM_14,
    .pin_dc   = GPIO_NUM_15,
    .pin_rst  = GPIO_NUM_21,
    .pin_bl   = GPIO_NUM_22,
    .width = 172,
    .height = 320,
    .x_gap = 34,
    .y_gap = 0,
    .pixel_clock_hz = 40 * 1000 * 1000,
    .invert_color = true,
};
