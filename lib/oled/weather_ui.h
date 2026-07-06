#ifndef WEATHER_UI_H
#define WEATHER_UI_H

#include "weather.h"

/**
 * @brief 绘制天气主界面（图标 + 城市/温度/描述/湿度）
 * @param info 天气数据，NULL 时使用上次缓存
 * @param footer 底部状态行，NULL 则不显示
 */
void weather_ui_draw(const weather_info_t *info, const char *footer);

/** @brief 显示错误界面 */
void weather_ui_draw_error(const char *title, const char *msg);

#endif /* WEATHER_UI_H */
