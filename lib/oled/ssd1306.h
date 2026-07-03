#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define SSD1306_WIDTH   128U
#define SSD1306_HEIGHT  64U

/** 默认字体：u8g2_font_8x13_tf（在 ssd1306_init 中设置） */

void ssd1306_init(I2C_HandleTypeDef *hi2c);
void ssd1306_clear(void);
void ssd1306_update(void);
void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str);

/**
 * @brief 获取 u8g2 句柄（需 #include "u8g2.h" 后使用）
 * @return u8g2_t 指针
 */
struct u8g2_struct;
struct u8g2_struct *ssd1306_get_u8g2(void);

#endif /* SSD1306_H */
