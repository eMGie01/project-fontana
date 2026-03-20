#include "hx711_new.h"

#include "esp_rom_sys.h"
#include "esp_err.h"
#include "esp_log.h"


#define RETURN_HW_IF_ESPERR(cb) \
    do {                        \
        esp_err_t e_ = (cb);    \
        if ( (e_) != ESP_OK )   \
            return HX711_HW_ERR;\
    } while (0)

    
static hx711_status_t
hx711_cfg_ios(hx711_hw_t * gpios)
{
    RETURN_HW_IF_ESPERR(
        gpio_config(&(gpio_config_t) {
        .pin_bit_mask = (1ULL << gpios->io_dout),
        .mode = GPIO_MODE_INPUT
    }));

    RETURN_HW_IF_ESPERR(
        gpio_config(&(gpio_config_t){
        .pin_bit_mask = (1ULL << gpios->io_sck),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE
    }));

    RETURN_HW_IF_ESPERR(
        gpio_set_level(gpios->io_sck, 0));

    return HX711_OK;
}


hx711_status_t
hx711_init(hx711_t * dev, hx711_hw_t * gpios, hx711_set_t * settings)
{

    if ( !dev || !gpios || !settings )
    {
        return HX711_INVALID_ARG;
    }

    if ( !GPIO_IS_VALID_GPIO(gpios->io_dout) || !GPIO_IS_VALID_OUTPUT_GPIO(gpios->io_sck) )
    {
        return HX711_HW_ERR;
    }

    if ( HX711_MODE_MIN > settings->mode || HX711_MODE_MAX < settings->mode )
    {
        return HX711_INVALID_ARG;
    }

    if ( hx711_cfg_ios(gpios) != HX711_OK )
    {
        return HX711_HW_ERR;
    }
    
    /* default config */
    dev->ios = *gpios;
    dev->settings = *settings;
    dev->calib = (hx711_cal_t) {
        .offset = 0,
        .scaleX1k = 1000
    };
    dev->last_raw = 0;
    dev->is_ready = false;
    dev->initialized = true;

    return HX711_OK;
}


hx711_status_t 
hx711_init_default(hx711_t * dev, hx711_hw_t * gpios)
{

    if (!dev || !gpios)
    {
        return HX711_INVALID_ARG;
    }

    if ( !GPIO_IS_VALID_GPIO(gpios->io_dout) || !GPIO_IS_VALID_OUTPUT_GPIO(gpios->io_sck) )
    {
        return HX711_HW_ERR;
    }

    if ( hx711_cfg_ios(gpios) != HX711_OK )
    {
        return HX711_HW_ERR;
    }

    /* default config */
    dev->ios = *gpios;
    dev->settings = (hx711_set_t) {
        .mode = HX711_MODE_A_128,
        .timeout_ms = 50
    };
    dev->calib = (hx711_cal_t) {
        .offset = 0,
        .scaleX1k = 1000
    };
    dev->last_raw = 0;
    dev->is_ready = false;
    dev->initialized = true;

    return HX711_OK;
}

hx711_status_t
hx711_deinit(hx711_t * dev)
{

    if ( !dev )
    {
        return HX711_INVALID_ARG;
    }

    if ( !dev->initialized )
    {
        return HX711_NOT_INITIALIZED;
    }

    dev->settings = (hx711_set_t) {0, 0};
    dev->calib = (hx711_cal_t) {0, 0};
    dev->last_raw = 0;
    dev->is_ready = false;
    dev->initialized = false;

    if ( gpio_reset_pin(dev->ios.io_sck) != ESP_OK )
    {
        return HX711_HW_ERR;
    }
    
    if ( gpio_reset_pin(dev->ios.io_dout) != ESP_OK )
    {
        return HX711_HW_ERR;
    }

    dev->ios = (hx711_hw_t) {0, 0};

    return HX711_OK;
}

