#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define ESP_AT_TIMEOUT_MS      3000U
#define ESP_WIFI_RETRY_MAX     3U

/* 外设开关：不接某模块时设为 0，避免误触发或长时间等待 */
#define FEATURE_BUZZER         0   /* 1=启用蜂鸣器  0=仅测 OLED 时关闭 */
#define FEATURE_ESP8266        0   /* 1=启用 ESP8266  0=仅测 OLED 时关闭 */

#endif /* APP_CONFIG_H */
