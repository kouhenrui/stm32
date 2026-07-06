#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*
 * 功能开关与 ESP 超时 — 由 .env 编译时注入（extra_scripts/env_config.py）
 * 以下为未配置 .env 时的默认值
 */

#ifndef ESP_AT_TIMEOUT_MS
#define ESP_AT_TIMEOUT_MS      3000U
#endif

#ifndef ESP_WIFI_RETRY_MAX
#define ESP_WIFI_RETRY_MAX     3U
#endif

#ifndef FEATURE_BUZZER
#define FEATURE_BUZZER         0
#endif

#ifndef FEATURE_ESP8266
#define FEATURE_ESP8266        1
#endif

#ifndef FEATURE_WEATHER
#define FEATURE_WEATHER        1
#endif

#if FEATURE_WEATHER && !FEATURE_ESP8266
#error FEATURE_WEATHER requires FEATURE_ESP8266
#endif

#endif /* APP_CONFIG_H */
