#include "lcd.h"
#include "hw_config.h"

lcd_cfg_t lcd_cfg = {
    .spi_host = SPI2_HOST,
    .pin_cs   = LCD_CS,
    .pin_dc   = LCD_DC,
    .pin_rst  = LCD_RST,
    .pin_bl   = LCD_BL,
    .width = 172,
    .height = 320,
    .x_gap = 34,
    .y_gap = 0,
    .pixel_clock_hz = 40 * 1000 * 1000,
    .invert_color = true,
};
