#ifndef U8G2_HAL_H
#define U8G2_HAL_H

#include "stm32f1xx_hal.h"
#include "u8g2.h"

uint8_t u8x8_stm32_i2c_dummy_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_stm32_gpio_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#endif /* U8G2_HAL_H */
