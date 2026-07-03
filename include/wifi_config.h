#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

/** 单条 WiFi 配置 */
typedef struct {
    const char *ssid;
    const char *password;
} wifi_profile_t;

/** 已配置的 WiFi 列表（在 src/wifi_config.c 中编辑） */
extern const wifi_profile_t g_wifi_profiles[];
extern const uint8_t g_wifi_profile_count;

/**
 * @brief 判断该配置是否有效（非空且非占位符）
 * @param profile WiFi 配置项
 */
bool wifi_profile_is_configured(const wifi_profile_t *profile);

#endif /* WIFI_CONFIG_H */
