/**
 * @file lcd.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief Small ESP-IDF ST7789 LCD driver wrapper.
 * @version 0.2
 * @date 2026-05-26
 *
 * This module wraps ESP-IDF's esp_lcd ST7789 panel driver for the 1.47 inch
 * 172x320 SPI display used by the original MonitorCisnieniaHX711 Arduino
 * sketch. It intentionally stays low level: it initializes the panel, controls
 * the backlight and provides color/rectangle/bitmap drawing primitives.
 *
 * For text and full screens, put a small UI layer above this driver or connect
 * LVGL/u8g2 to lcd_DrawBitmap().
 */

#ifndef LCD_H
#define LCD_H

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_COLOR_BLACK      0x0000
#define LCD_COLOR_BLUE       0x001F
#define LCD_COLOR_RED        0xF800
#define LCD_COLOR_GREEN      0x07E0
#define LCD_COLOR_CYAN       0x07FF
#define LCD_COLOR_MAGENTA    0xF81F
#define LCD_COLOR_YELLOW     0xFFE0
#define LCD_COLOR_WHITE      0xFFFF
#define LCD_COLOR_LIGHTGREY  0xC618
#define LCD_COLOR_DARKGREY   0x7BEF

extern const uint8_t font5x7[95][5];

typedef enum
{
    LEFT = 0,
    RIGHT = 1,
} lcd_rot_t;

typedef struct
{
    spi_host_device_t   spi_host;
    gpio_num_t          pin_sclk;
    gpio_num_t          pin_mosi;
    gpio_num_t          pin_miso;
    gpio_num_t          pin_cs;
    gpio_num_t          pin_dc;
    gpio_num_t          pin_rst;
    gpio_num_t          pin_bl;
    uint16_t            width;
    uint16_t            height;
    uint16_t            x_gap;
    uint16_t            y_gap;
    uint32_t            pixel_clock_hz;
    bool                invert_color;
} lcd_cfg_t;

typedef struct 
{
    esp_lcd_panel_io_handle_t   io;
    esp_lcd_panel_handle_t      panel;
    spi_host_device_t           spi_host;
    gpio_num_t                  pin_bl;
    uint16_t                    width;
    uint16_t                    height;
    bool                        initialized;
    bool                        spi_bus_owned;
} lcd_t;

typedef lcd_t* lcd_handle_t;

esp_err_t lcd_open(lcd_handle_t lcd, const lcd_cfg_t* cfg);
esp_err_t lcd_close(lcd_handle_t lcd);

esp_err_t lcd_setBacklight(lcd_handle_t lcd, uint8_t brightness);
esp_err_t lcd_landscape(lcd_handle_t lcd, bool variant);
esp_err_t lcd_fillScreen(lcd_handle_t lcd, uint16_t color);
esp_err_t lcd_fillRect(lcd_handle_t lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
esp_err_t lcd_drawPixel(lcd_handle_t lcd, uint16_t x, uint16_t y, uint16_t color);
esp_err_t lcd_drawChar(lcd_handle_t lcd, uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale);
esp_err_t lcd_drawString(lcd_handle_t lcd, uint16_t x, uint16_t y, const char* str, uint16_t fg, uint16_t bg, uint8_t scale);


#ifdef __cplusplus
}
#endif

#endif // LCD_H
