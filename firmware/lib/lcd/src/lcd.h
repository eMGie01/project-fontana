/**
 * @file lcd.h
 * @author Marek Gałeczka
 * @brief Public interface placeholder for the LCD module.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef LCD_H
#define LCD_H

#include "driver/gpio.h"

// Defines

#define LCD_SCLK_PIN    7
#define LCD_MOSI_PIN    6
#define LCD_MISO_PIN    5
#define LCD_CS_PIN      14
#define LCD_DC_PIN      15
#define LCD_RST_PIN     21
#define LCD_BL_PIN      22
#define LCD_PWM_FREQ    1000
#define LCD_PWM_RES     8
#define LCD_WIDTH       172
#define LCD_HEIGHT      320

// Enums

typedef enum
{
    LCD_OK = 0,
    LCD_INVALID_ARG,
    LCD_HW_ERR,
    

} lcd_err_t;

// Structs

typedef struct 
{
    gpio_num_t io_sclk;
    gpio_num_t io_mosi;
    gpio_num_t io_miso;
    gpio_num_t io_cs;
    gpio_num_t io_dc;
    gpio_num_t io_rst;
    gpio_num_t io_bl;
    uint16_t width;
    uint16_t height;
} lcd_cfg_t;

// Functions

lcd_err_t lcd_init(const lcd_cfg_t * cfg);
lcd_err_t lcd_fill_screen(const uint16_t colour);
lcd_err_t lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour);


#endif // LCD_H
