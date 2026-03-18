#ifndef HX711_H
#define HX711_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Mode configurations
 */
typedef enum {
    HX711_MODE_A_128 = 25,
    HX711_MODE_B_32  = 26,
    HX711_MODE_A_64  = 27
} hx711_mode_t;

/**
 * @brief Return values of driver functions
 */
typedef enum {
    HX711_OK = 0,
    HX711_ERR_ARG,
    HX711_ERR_TIMEOUT,
    HX711_ERR_NOT_READY
} hx711_status_t;

/**
 * @brief Configuration parameters of HX711
 */
typedef struct {
    gpio_num_t gpio_dout;
    gpio_num_t gpio_sck;
    hx711_mode_t next_mode;
    int32_t offset;
    int32_t scale;
} hx711_t;

/**
 * @brief   Initialization of HX711 driver
 *
 * @param   dev Pointer to driver's configuration
 * @param   gpio_dout gpio number of dout signal
 * @param   gpio_sck gpio number of sck signal
 * @param   mode setting of next aquisition mode
 *
 * @return
 *     - HX711_OK success
 *     - HX711_ERR_ARG Parameter error
 */
hx711_status_t hx711_init(hx711_t * dev, gpio_num_t gpio_dout, gpio_num_t gpio_sck, hx711_mode_t mode);

/**
 * @brief   Check wheter HX711 data is ready to pull
 *
 * @param   dev Pointer to driver's configuration
 *
 * @return
 *     - True
 *     - False
 */
bool hx711_is_ready(const hx711_t *dev);

/**
 * @brief   Read measured value from ADC
 *
 * @param   dev Pointer to driver's configuration
 * @param   value Pointer to variable for measurement
 *
 * @return
 *     - HX711_OK success
 *     - HX711_ERR_ARG Parameter error
 *     - HX711_ERR_NOT_READY Data not ready to pull
 */
hx711_status_t hx711_read_raw(hx711_t * dev, int32_t * value);


#ifdef __cplusplus
}
#endif

#endif /* HX711_H */
