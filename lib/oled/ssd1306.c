#include "ssd1306.h"
#include "ssd1306_hw.h"
#include "u8g2_hal.h"
#include "pin_config.h"
#include "u8g2.h"

static u8g2_t g_u8g2;

void ssd1306_init(I2C_HandleTypeDef *hi2c)
{
    ssd1306_hw_bind(hi2c);

    /* u8g2 仅负责显存与字体，I2C 初始化/刷屏用已验证的 hw 驱动 */
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        &g_u8g2, U8G2_R0, u8x8_stm32_i2c_dummy_cb, u8x8_stm32_gpio_delay_cb);

    u8x8_SetI2CAddress(&g_u8g2.u8x8, (uint8_t)(OLED_I2C_ADDRESS << 1));

    ssd1306_hw_init();

    u8g2_SetFont(&g_u8g2, u8g2_font_8x13_tf);
    u8g2_ClearBuffer(&g_u8g2);
    ssd1306_update();
}

void ssd1306_clear(void)
{
    u8g2_ClearBuffer(&g_u8g2);
}

void ssd1306_update(void)
{
    ssd1306_hw_flush(u8g2_GetBufferPtr(&g_u8g2));
}

void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str)
{
    u8g2_DrawStr(&g_u8g2, x, (uint16_t)(y * 8U + 13U), str);
}

struct u8g2_struct *ssd1306_get_u8g2(void)
{
    return (struct u8g2_struct *)&g_u8g2;
}
