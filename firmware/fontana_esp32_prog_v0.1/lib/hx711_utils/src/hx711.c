#include "hx711.h"

#include "esp_rom_sys.h"
#include "esp_err.h"
#include "esp_log.h"


static const char * TAG = "HX711";


hx711_status_t hx711_init(hx711_t * dev, gpio_num_t gpio_dout, gpio_num_t gpio_sck, hx711_mode_t mode) {
    ESP_LOGD(TAG, "init process started");
    esp_err_t res;

    if (dev == NULL) {
        ESP_LOGE(TAG, "dev pointer to device configuration is NULL");
        return HX711_ERR_ARG;
    }

    if (mode != HX711_MODE_A_128 && mode != HX711_MODE_A_64 && mode != HX711_MODE_B_32) {
        ESP_LOGE(TAG, "mode (%d) input value is not correct", mode);
        return HX711_ERR_ARG;
    }

    if (!GPIO_IS_VALID_GPIO(gpio_dout)) {
        ESP_LOGE(TAG, "gpio_dout (%d) is not a valid pin", gpio_dout);
        return HX711_ERR_ARG;
    }

    if (!GPIO_IS_VALID_OUTPUT_GPIO(gpio_sck)) {
        ESP_LOGE(TAG, "gpio_sck (%d) is not a valid output pin", gpio_sck);
        return HX711_ERR_ARG;
    }

    dev->gpio_dout = gpio_dout;
    dev->gpio_sck = gpio_sck;
    dev->next_mode = mode;

    /* set dout as input */
    ESP_LOGD(TAG, "setting %d pin as input", (int)gpio_dout);
    gpio_config_t dout_conf = {
        .pin_bit_mask = (1ULL << gpio_dout),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    res = gpio_config(&dout_conf);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "setting %d pin as input failed with error: %s (%d)", (int)gpio_dout, esp_err_to_name(res), (int)res);
        return HX711_ERR_ARG;
    }

    /* seto sck as output */
    ESP_LOGD(TAG, "setting %d pin as output", (int)gpio_sck);
    gpio_config_t sck_conf = {
        .pin_bit_mask = (1ULL << gpio_sck),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    res = gpio_config(&sck_conf);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "setting %d pin as output failed with error: %s (%d)", (int)gpio_sck, esp_err_to_name(res), (int)res);
        return HX711_ERR_ARG;
    }

    res = gpio_set_level(gpio_sck, 0);
    if (res == ESP_OK) {
        ESP_LOGD(TAG, "init success");
        return HX711_OK;
    }

    ESP_LOGE(TAG, "gpio set level to LOW failed with error: %s (%d)", esp_err_to_name(res), (int)res);
    return HX711_ERR_ARG;
}


bool hx711_is_ready(const hx711_t * dev) {
    if (dev == NULL) {
        ESP_LOGE(TAG, "dev pointer to hx711 configuration is NULL");
        return false;
    }
    return !gpio_get_level(dev->gpio_dout);
}


hx711_status_t hx711_read_raw(hx711_t * dev, int32_t * value) {
    if (dev == NULL || value == NULL) {
        ESP_LOGE(TAG, "dev pointer to hx711 configuration or read ADC value pointer is NULL");
        return HX711_ERR_ARG;
    }

    if (gpio_get_level(dev->gpio_sck) != 0) {
        if (gpio_set_level(dev->gpio_sck, 0) != ESP_OK) {
            ESP_LOGE(TAG, "gpio_sck (%d) state does not change to LOW", dev->gpio_sck);
            return HX711_ERR_ARG;
        }
    }

    if (!hx711_is_ready(dev)) {
        return HX711_ERR_NOT_READY;
    }

    uint32_t raw = 0;
    for (uint8_t i = 0; i < 24; ++i) {
        
        if (gpio_set_level(dev->gpio_sck, 1) != ESP_OK) {
            ESP_LOGE(TAG, "couldn't set gpio_sck (%d) to HIGH", dev->gpio_sck);
            return HX711_ERR_ARG;
        }
        esp_rom_delay_us(1);

        raw = (raw << 1) | gpio_get_level(dev->gpio_dout);

        if (gpio_set_level(dev->gpio_sck, 0) != ESP_OK) {
            ESP_LOGE(TAG, "couldn't set gpio_sck (%d) to LOW", dev->gpio_sck);
            return HX711_ERR_ARG;
        }
        esp_rom_delay_us(1);

    }

    for (uint8_t i = 0; i < ((uint8_t)dev->next_mode - 24); ++i) {
        if (gpio_set_level(dev->gpio_sck, 1) != ESP_OK) {
            ESP_LOGE(TAG, "couldn't set gpio_sck (%d) to HIGH", dev->gpio_sck);
            return HX711_ERR_ARG;
        }
        esp_rom_delay_us(1);
        if (gpio_set_level(dev->gpio_sck, 0) != ESP_OK) {
            ESP_LOGE(TAG, "couldn't set gpio_sck (%d) to LOW", dev->gpio_sck);
            return HX711_ERR_ARG;
        }
        esp_rom_delay_us(1);
    }

    // *value = (raw & 0x800000) ? (int32_t)(raw | 0xFF000000) : (int32_t)raw;
    /* for test purposes */
        static uint8_t index = 0;
        index = index % 10;
        int32_t values[] = {50000, 50500, 49800, 51000, 50000, 49700, 51500, 50010, 49850, 50120};
        *value = values[index++];
    /* ================= */
    return HX711_OK;
}
