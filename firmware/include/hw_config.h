/**
 * @file hw_config.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "driver/gpio.h"

/**
 * one file for all HW configuration pinout
 */

// HX711
#define HX711_GPIO_SCK  GPIO_NUM_3
#define HX711_GPIO_DOUT GPIO_NUM_2

// UART
#define CLI_UART_PORT   UART_NUM_0
#define CLI_UART_TX     GPIO_NUM_16
#define CLI_UART_RX     GPIO_NUM_17

// SD
#define SD_CS           GPIO_NUM_4

// LCD
#define LCD_CS          GPIO_NUM_14
#define LCD_DC          GPIO_NUM_15
#define LCD_BL          GPIO_NUM_22
#define LCD_RST         GPIO_NUM_21

// SPI2
#define SPI2_SCLK         GPIO_NUM_7
#define SPI2_MISO         GPIO_NUM_5
#define SPI2_MOSI         GPIO_NUM_6


#endif // HW_CONFIG_H