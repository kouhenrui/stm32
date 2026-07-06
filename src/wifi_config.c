#include "wifi_config.h"
#include <string.h>

#ifndef WIFI_SSID_1
#define WIFI_SSID_1            "your-ssid"
#endif
#ifndef WIFI_PASSWORD_1
#define WIFI_PASSWORD_1        "your-password"
#endif
#ifndef WIFI_SSID_2
#define WIFI_SSID_2            "your-ssid-2"
#endif
#ifndef WIFI_PASSWORD_2
#define WIFI_PASSWORD_2        "your-password-2"
#endif
#ifndef WIFI_SSID_3
#define WIFI_SSID_3            "your-ssid-3"
#endif
#ifndef WIFI_PASSWORD_3
#define WIFI_PASSWORD_3        "your-password-3"
#endif

/*
 * WiFi 列表由 .env 注入（WIFI_SSID_1 / WIFI_PASSWORD_1 …）
 * 见 extra_scripts/env_config.py
 */
const wifi_profile_t g_wifi_profiles[] = {
    { WIFI_SSID_1, WIFI_PASSWORD_1 },
    { WIFI_SSID_2, WIFI_PASSWORD_2 },
    { WIFI_SSID_3, WIFI_PASSWORD_3 },
};

const uint8_t g_wifi_profile_count = (uint8_t)(sizeof(g_wifi_profiles) / sizeof(g_wifi_profiles[0]));

bool wifi_profile_is_configured(const wifi_profile_t *profile)
{
    if (profile == NULL || profile->ssid == NULL || profile->password == NULL) {
        return false;
    }
    if (profile->ssid[0] == '\0' || profile->password[0] == '\0') {
        return false;
    }
    if (strncmp(profile->ssid, "your-ssid", 9) == 0) {
        return false;
    }
    if (strncmp(profile->password, "your-password", 13) == 0) {
        return false;
    }
    return true;
}
