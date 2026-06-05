#include "lcd.h"

#define LCD_BL_LEDC_MODE      LEDC_LOW_SPEED_MODE
#define LCD_BL_LEDC_TIMER     LEDC_TIMER_0
#define LCD_BL_LEDC_CHANNEL   LEDC_CHANNEL_0
#define LCD_BL_PWM_FREQ_HZ    1000
#define LCD_BL_DEFAULT_DUTY   30

static esp_err_t
lcd_configBacklight_(lcd_handle_t lcd, gpio_num_t pin_bl)
{
    lcd->pin_bl = pin_bl;

    if (pin_bl == GPIO_NUM_NC)
    {
        return ESP_OK;
    }

    ledc_timer_config_t timer = {
        .speed_mode = LCD_BL_LEDC_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LCD_BL_LEDC_TIMER,
        .freq_hz = LCD_BL_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&timer);
    if (err != ESP_OK)
    {
        return err;
    }

    ledc_channel_config_t channel = {
        .gpio_num = pin_bl,
        .speed_mode = LCD_BL_LEDC_MODE,
        .channel = LCD_BL_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LCD_BL_LEDC_TIMER,
        .duty = LCD_BL_DEFAULT_DUTY,
        .hpoint = 0,
    };

    err = ledc_channel_config(&channel);
    if (err != ESP_OK)
    {
        return err;
    }

    return ESP_OK;
}

esp_err_t
lcd_open(lcd_handle_t lcd, const lcd_cfg_t* cfg)
{
    if (lcd == NULL || cfg == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    lcd->initialized = false;
    lcd->io = NULL;
    lcd->panel = NULL;
    lcd->pin_bl = GPIO_NUM_NC;
    lcd->spi_host = cfg->spi_host;

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = cfg->pin_dc,
        .cs_gpio_num = cfg->pin_cs,
        .pclk_hz = cfg->pixel_clock_hz,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    esp_err_t err = esp_lcd_new_panel_io_spi(cfg->spi_host, &io_config, &lcd->io);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = cfg->pin_rst,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };

    err = esp_lcd_new_panel_st7789(lcd->io, &panel_config, &lcd->panel);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    err = esp_lcd_panel_reset(lcd->panel);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    err = esp_lcd_panel_init(lcd->panel);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    err = esp_lcd_panel_invert_color(lcd->panel, cfg->invert_color);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    err = esp_lcd_panel_set_gap(lcd->panel, cfg->x_gap, cfg->y_gap);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    err = esp_lcd_panel_disp_on_off(lcd->panel, true);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    lcd->width = cfg->width;
    lcd->height = cfg->height;

    err = lcd_configBacklight_(lcd, cfg->pin_bl);
    if (err != ESP_OK)
    {
        lcd_close(lcd);
        return err;
    }

    lcd->initialized = true;

    return ESP_OK;
}

esp_err_t
lcd_close(lcd_handle_t lcd)
{
    if (lcd == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;

    if (lcd->pin_bl != GPIO_NUM_NC)
    {
        esp_err_t err = ledc_set_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL, 0);
        if (ret == ESP_OK && err != ESP_OK)
        {
            ret = err;
        }

        err = ledc_update_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL);
        if (ret == ESP_OK && err != ESP_OK)
        {
            ret = err;
        }
    }

    if (lcd->panel != NULL)
    {
        esp_err_t err = esp_lcd_panel_disp_on_off(lcd->panel, false);
        if (ret == ESP_OK && err != ESP_OK)
        {
            ret = err;
        }

        err = esp_lcd_panel_del(lcd->panel);
        if (ret == ESP_OK && err != ESP_OK)
        {
            ret = err;
        }
        lcd->panel = NULL;
    }

    if (lcd->io != NULL)
    {
        esp_err_t err = esp_lcd_panel_io_del(lcd->io);
        if (ret == ESP_OK && err != ESP_OK)
        {
            ret = err;
        }
        lcd->io = NULL;
    }

    lcd->initialized = false;
    lcd->width = 0;
    lcd->height = 0;
    lcd->pin_bl = GPIO_NUM_NC;

    return ret;
}

esp_err_t
lcd_setBacklight(lcd_handle_t lcd, uint8_t brightness)
{
    if (lcd == NULL || !lcd->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (lcd->pin_bl == GPIO_NUM_NC)
    {
        return ESP_OK;
    }

    esp_err_t err = ledc_set_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL, brightness);
    if (err != ESP_OK)
    {
        return err;
    }

    return ledc_update_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL);
}

esp_err_t 
lcd_landscape(lcd_handle_t lcd, bool variant)
{
    if (lcd == NULL || !lcd->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err;
    err = esp_lcd_panel_swap_xy(lcd->panel, true);
    if (err != ESP_OK)
    {
        return err;
    }

    if (variant)
    {
        err = esp_lcd_panel_mirror(lcd->panel, true, false);
        if (err != ESP_OK)
        {
            return err;
        }
    }
    else
    {
        err = esp_lcd_panel_mirror(lcd->panel, false, true);
        if (err != ESP_OK)
        {
            return err;
        }
    }

    err = esp_lcd_panel_set_gap(lcd->panel, 0, 34);
    if (err != ESP_OK)
    {
        return err;
    }

    lcd->width = 320;
    lcd->height = 172;  
    return ESP_OK;
}

esp_err_t
lcd_fillScreen(lcd_handle_t lcd, uint16_t color)
{
    if (lcd == NULL || !lcd->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return lcd_fillRect(lcd, 0, 0, lcd->width, lcd->height, color);
}

esp_err_t
lcd_fillRect(lcd_handle_t lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (lcd == NULL || !lcd->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (w == 0 || h == 0 || w > 320)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (x >= lcd->width || y >= lcd->height)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if ((uint32_t)x + w > lcd->width || (uint32_t)y + h > lcd->height)
    {
        return ESP_ERR_INVALID_ARG;
    }

    static uint16_t line[320];
    for (uint16_t i = 0; i < w; ++i)
    {
        line[i] = color;
    }

    for (uint16_t row = 0; row < h; ++row)
    {
        esp_err_t err = esp_lcd_panel_draw_bitmap(lcd->panel, x, (y+row), (x+w), (y+row+1), line);
        if (err != ESP_OK)
        {
            return err;
        }
    }
    return ESP_OK;
}

esp_err_t
lcd_drawString(lcd_handle_t lcd, uint16_t x, uint16_t y, const char* str, uint16_t fg, uint16_t bg, uint8_t scale)
{
    if (lcd == NULL || !lcd->initialized || str == NULL || scale == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t cursor_x = x;
    uint16_t cursor_y = y;
    const uint16_t char_step = 6U * scale;
    const uint16_t line_step = 8U * scale;

    for (const char* p = str; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            cursor_x = x;
            cursor_y += line_step;
            continue;
        }

        esp_err_t err = lcd_drawChar(lcd, cursor_x, cursor_y, *p, fg, bg, scale);
        if (err != ESP_OK)
        {
            return err;
        }
        cursor_x += char_step;
    }

    return ESP_OK;
}

esp_err_t
lcd_drawPixel(lcd_handle_t lcd, uint16_t x, uint16_t y, uint16_t color)
{
    if (lcd == NULL || !lcd->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (x >= lcd->width || y >= lcd->height)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return lcd_fillRect(lcd, x, y, 1, 1, color);
}

esp_err_t
lcd_drawChar(lcd_handle_t lcd, uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale)
{
    if (lcd == NULL || !lcd->initialized || scale == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (ch < ' ' || ch > '~')
    {
        ch = '?';
    }

    if ((uint32_t)x + 6U * scale > lcd->width || (uint32_t)y + 8U * scale > lcd->height)
    {
        return ESP_ERR_INVALID_ARG;
    }

    const uint8_t *glyph = font5x7[ch - ' '];

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t bits = glyph[col];

        for (uint8_t row = 0; row < 7; row++)
        {
            uint16_t color = (bits & (1 << row)) ? fg : bg;

            esp_err_t err;

            if (scale == 1)
            {
                err = lcd_drawPixel(lcd, x + col, y + row, color);
            }
            else
            {
                err = lcd_fillRect(
                    lcd,
                    x + col * scale,
                    y + row * scale,
                    scale,
                    scale,
                    color
                );
            }

            if (err != ESP_OK)
            {
                return err;
            }
        }
    }

    esp_err_t err = lcd_fillRect(lcd, x + 5U * scale, y, scale, 8U * scale, bg);
    if (err != ESP_OK)
    {
        return err;
    }

    return ESP_OK;
}
