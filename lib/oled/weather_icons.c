#include "weather_icons.h"

/** 太阳图标：圆盘 + 8 条射线 */
static void draw_sunny(u8g2_t *u8g2, uint8_t x, uint8_t y)
{
    static const int8_t rays[8][2] = {
        {0, -12}, {9, -9}, {12, 0}, {9, 9}, {0, 12}, {-9, 9}, {-12, 0}, {-9, -9}
    };
    uint8_t cx = (uint8_t)(x + 16U);
    uint8_t cy = (uint8_t)(y + 16U);
    uint8_t i;

    u8g2_DrawDisc(u8g2, cx, cy, 8, U8G2_DRAW_ALL);
    for (i = 0; i < 8U; i++) {
        u8g2_DrawLine(u8g2, cx, cy,
                      (uint8_t)(cx + rays[i][0]), (uint8_t)(cy + rays[i][1]));
    }
}

/** 多云图标 */
static void draw_cloudy(u8g2_t *u8g2, uint8_t x, uint8_t y)
{
    u8g2_DrawDisc(u8g2, (uint8_t)(x + 12U), (uint8_t)(y + 20U), 7, U8G2_DRAW_ALL);
    u8g2_DrawDisc(u8g2, (uint8_t)(x + 22U), (uint8_t)(y + 18U), 9, U8G2_DRAW_ALL);
    u8g2_DrawDisc(u8g2, (uint8_t)(x + 18U), (uint8_t)(y + 14U), 6, U8G2_DRAW_ALL);
    u8g2_DrawBox(u8g2, (uint8_t)(x + 6U), (uint8_t)(y + 20U), 24, 6);
}

/** 下雨图标：云 + 雨滴 */
static void draw_rainy(u8g2_t *u8g2, uint8_t x, uint8_t y)
{
    draw_cloudy(u8g2, x, (uint8_t)(y - 4U));
    u8g2_DrawLine(u8g2, (uint8_t)(x + 10U), (uint8_t)(y + 26U), (uint8_t)(x + 8U), (uint8_t)(y + 30U));
    u8g2_DrawLine(u8g2, (uint8_t)(x + 18U), (uint8_t)(y + 26U), (uint8_t)(x + 16U), (uint8_t)(y + 30U));
    u8g2_DrawLine(u8g2, (uint8_t)(x + 26U), (uint8_t)(y + 26U), (uint8_t)(x + 24U), (uint8_t)(y + 30U));
}

void weather_icon_draw(u8g2_t *u8g2, uint8_t x, uint8_t y, weather_type_t type)
{
    switch (type) {
    case WEATHER_SUNNY:
        draw_sunny(u8g2, x, y);
        break;
    case WEATHER_RAINY:
        draw_rainy(u8g2, x, y);
        break;
    case WEATHER_CLOUDY:
    default:
        draw_cloudy(u8g2, x, y);
        break;
    }
}
