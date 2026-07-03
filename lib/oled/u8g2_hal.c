#include "u8g2_hal.h"

uint8_t u8x8_stm32_i2c_dummy_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    (void)msg;
    (void)arg_int;
    (void)arg_ptr;
    return 1;
}

uint8_t u8x8_stm32_gpio_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    (void)arg_ptr;

    switch (msg) {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        break;
    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(arg_int);
        break;
    default:
        break;
    }

    return 1;
}
