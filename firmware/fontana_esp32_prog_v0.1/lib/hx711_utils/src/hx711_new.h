#ifndef HX711_NEW_H
#define HX711_NEW_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Enums
typedef enum {
    HX711_MODE_MIN   = 25,
    HX711_MODE_A_128 = 25,
    HX711_MODE_B_32  = 26,
    HX711_MODE_A_64  = 27,
    HX711_MODE_MAX   = 27
} hx711_mode_t;

typedef enum
{
    HX711_UNEXPECTED_ERR = -1,
    HX711_OK = 0,
    HX711_INVALID_ARG,
    HX711_NOT_INITIALIZED,
    HX711_TIMEOUT,
    HX711_HW_ERR,
    HX711_NOT_READY
} hx711_status_t;

// Structures
typedef struct 
{
    int io_sck;
    int io_dout;
} hx711_hw_t;

typedef struct
{
    hx711_mode_t mode;
    uint32_t     timeout_ms;
} hx711_set_t;

typedef struct
{
    int32_t scaleX1k;
    int32_t offset;
} hx711_cal_t;

typedef struct 
{
    hx711_hw_t ios;
    hx711_set_t settings;
    hx711_cal_t calib;
    int32_t last_raw;
    bool is_ready;
    bool initialized;
} hx711_t;


// Functions
hx711_status_t hx711_init(hx711_t * dev, const hx711_hw_t * gpios, const hx711_set_t * settings);
hx711_status_t hx711_init_default(hx711_t * dev, const hx711_hw_t * gpios);
hx711_status_t hx711_deinit(hx711_t * dev);
bool hx711_is_ready(const hx711_t * dev);

#ifdef __cplusplus
}
#endif

#endif /* HX711_H */
