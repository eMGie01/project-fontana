#include "hx711_new.h"

#include "esp_rom_sys.h"
#include "esp_err.h"
#include "esp_log.h"

/**
 * read raw value
 * is read ready
 */

bool
hx711_is_ready(const hx711_t * dev)
{
    if ( !dev )
    {
        return false;
    }
    return !gpio_get_level(dev->ios.io_dout);
}


hx711_status_t
hx711_read_raw(hx711_t * dev, int32_t * value)
{

    if ( !dev || !value )
    {
        return HX711_INVALID_ARG;
    }

    if ( 0 != gpio_get_level(dev->ios.io_sck) )
    {
        if ( ESP_OK != gpio_set_level(dev->ios.io_sck, 0) )
        {
            return HX711_HW_ERR;
        }
    }

    if ( !dev->is_ready )
    {
        return HX711_NOT_READY; // data not ready
    }

    uint32_t raw_adc_value = 0;

    for (uint8_t i = 0; i < 24; ++i)
    {

        if (ESP_OK != gpio_set_level(dev->ios.io_sck, 1) )
        {
            return HX711_HW_ERR;
        }
        esp_rom_delay_us(1);

        raw_adc_value = (raw_adc_value << 1) | gpio_get_level(dev->ios.io_dout);
        
        if (ESP_OK != gpio_set_level(dev->ios.io_sck, 0) )
        {
            return HX711_HW_ERR;
        }
        esp_rom_delay_us(1);

    }

    for (uint8_t i = 0; i < ((uint8_t)dev->settings.mode - HX711_MODE_MIN); ++i)
    {
        for (uint8_t j = 0; j < 2; ++j)
        {
            if (ESP_OK != gpio_set_level(dev->ios.io_sck, (int)!(j%2)) )
            {
                return HX711_HW_ERR;
            }
            esp_rom_delay_us(1);
        }
        
    }

    return HX711_OK;
}