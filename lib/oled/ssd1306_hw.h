#ifndef SSD1306_HW_H
#define SSD1306_HW_H

#include "stm32f1xx_hal.h"

void ssd1306_hw_bind(I2C_HandleTypeDef *hi2c);
void ssd1306_hw_init(void);
void ssd1306_hw_flush(const uint8_t *framebuffer);

#endif /* SSD1306_HW_H */
