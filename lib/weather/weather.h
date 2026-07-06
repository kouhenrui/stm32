#ifndef WEATHER_H
#define WEATHER_H

#include <stdbool.h>
#include <stdint.h>

/** 天气图标类型（3 类） */
typedef enum {
    WEATHER_SUNNY = 0,
    WEATHER_CLOUDY,
    WEATHER_RAINY
} weather_type_t;

/** 天气拉取结果 */
typedef enum {
    WEATHER_OK = 0,
    WEATHER_ERR_CONFIG,
    WEATHER_ERR_NETWORK,
    WEATHER_ERR_PARSE
} weather_status_t;

/** 当前天气信息 */
typedef struct {
    char city[24];
    char desc[20];
    float temp_c;
    int8_t humidity;
    weather_type_t type;
    bool valid;
} weather_info_t;

/**
 * @brief 从 OpenWeatherMap 拉取当前天气
 * @param info 输出结构体
 */
weather_status_t weather_fetch(weather_info_t *info);

/** @brief 是否有缓存的上次成功数据 */
bool weather_has_last(void);

/** @brief 获取上次成功数据（只读） */
const weather_info_t *weather_get_last(void);

/** @brief 将 API weather.main 映射为图标类型 */
weather_type_t weather_map_type(const char *main_str);

/** @brief 状态码转字符串 */
const char *weather_status_string(weather_status_t status);

#endif /* WEATHER_H */
