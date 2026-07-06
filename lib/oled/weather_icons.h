#ifndef WEATHER_ICONS_H
#define WEATHER_ICONS_H

#include "u8g2.h"
#include "weather.h"

/**
 * @brief 在指定位置绘制 32×32 天气图标
 * @param u8g2 u8g2 句柄
 * @param x 左上角 X
 * @param y 左上角 Y
 * @param type 天气类型
 */
void weather_icon_draw(u8g2_t *u8g2, uint8_t x, uint8_t y, weather_type_t type);

#endif /* WEATHER_ICONS_H */
