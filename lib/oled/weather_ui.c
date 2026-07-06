#include "weather_ui.h"
#include "weather_icons.h"
#include "ssd1306.h"
#include "u8g2.h"
#include <stdio.h>
#include <string.h>

static void weather_ui_render(const weather_info_t *info, const char *footer)
{
    u8g2_t *u8g2 = (u8g2_t *)ssd1306_get_u8g2();
    char line[24];

    ssd1306_clear();

    weather_icon_draw(u8g2, 0, 16, info->type);

    u8g2_SetFont(u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(u8g2, 38, 14, info->city);

    u8g2_SetFont(u8g2, u8g2_font_helvB14_tr);
    snprintf(line, sizeof(line), "%d C", (int)(info->temp_c + (info->temp_c >= 0 ? 0.5f : -0.5f)));
    u8g2_DrawStr(u8g2, 38, 32, line);

    u8g2_SetFont(u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(u8g2, 38, 48, info->desc);

    if (footer != NULL && footer[0] != '\0') {
        u8g2_DrawStr(u8g2, 0, 62, footer);
    } else if (info->humidity > 0) {
        snprintf(line, sizeof(line), "Hum:%d%%", (int)info->humidity);
        u8g2_DrawStr(u8g2, 38, 62, line);
    }

    ssd1306_update();
}

void weather_ui_draw(const weather_info_t *info, const char *footer)
{
    const weather_info_t *src = info;

    if (src == NULL || !src->valid) {
        src = weather_get_last();
    }
    if (src == NULL || !src->valid) {
        weather_ui_draw_error("Weather", "No data");
        return;
    }

    weather_ui_render(src, footer);
}

void weather_ui_draw_error(const char *title, const char *msg)
{
    u8g2_t *u8g2 = (u8g2_t *)ssd1306_get_u8g2();

    ssd1306_clear();
    u8g2_SetFont(u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(u8g2, 0, 14, title != NULL ? title : "Weather");
    u8g2_DrawStr(u8g2, 0, 32, msg != NULL ? msg : "Error");
    ssd1306_update();
}
