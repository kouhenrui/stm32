#include "wifi_config.h"
#include <string.h>

/*
 * 在此添加多个 WiFi，按数组顺序依次尝试连接。
 * 以 "your-ssid" 开头的条目视为未配置，启动时自动跳过。
 */
const wifi_profile_t g_wifi_profiles[] = {
    { "your-ssid",      "your-password" },   /* 热点 1：家里 */
    { "your-ssid-2",    "your-password-2" },  /* 热点 2：办公室 */
    { "your-ssid-3",    "your-password-3" },  /* 热点 3：备用 */
};

const uint8_t g_wifi_profile_count = (uint8_t)(sizeof(g_wifi_profiles) / sizeof(g_wifi_profiles[0]));

bool wifi_profile_is_configured(const wifi_profile_t *profile)
{
    if (profile == NULL || profile->ssid == NULL || profile->password == NULL) {
        return false;
    }
    if (profile->ssid[0] == '\0') {
        return false;
    }
    /* 跳过占位符（your-ssid、your-ssid-2 等） */
    if (strncmp(profile->ssid, "your-ssid", 9) == 0) {
        return false;
    }
    return true;
}
