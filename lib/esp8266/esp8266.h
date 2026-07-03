#ifndef ESP8266_H
#define ESP8266_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "wifi_config.h"

/** ESP8266 操作结果 */
typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERR_TIMEOUT,
    ESP8266_ERR_RESPONSE,
    ESP8266_ERR_WIFI
} esp8266_status_t;

/**
 * @brief 绑定 UART 并等待模块就绪
 * @param huart 已配置的 UART 句柄（USART2）
 */
void esp8266_init(UART_HandleTypeDef *huart);

/**
 * @brief 发送 AT 指令测试连通性，并关闭回显（ATE0）
 * @return ESP8266_OK 表示收到 OK 响应
 */
esp8266_status_t esp8266_test_at(void);

/**
 * @brief 连接指定 WiFi 热点（Station 模式）
 * @param ssid WiFi 名称
 * @param password WiFi 密码
 * @return 连接结果状态码
 */
esp8266_status_t esp8266_wifi_connect(const char *ssid, const char *password);

/**
 * @brief 按顺序尝试连接多个 WiFi，成功即停止
 * @param profiles 配置数组
 * @param count 数组长度
 * @param connected_index 成功时写入下标，可传 NULL
 * @return ESP8266_OK 表示至少连接上一个热点
 */
esp8266_status_t esp8266_wifi_connect_profiles(const wifi_profile_t *profiles,
                                               uint8_t count,
                                               uint8_t *connected_index);

/**
 * @brief 使用 g_wifi_profiles 连接（便捷封装）
 * @param connected_index 成功时写入下标，可传 NULL
 */
esp8266_status_t esp8266_wifi_connect_configured(uint8_t *connected_index);

/**
 * @brief 获取最近一次成功连接的 SSID
 * @return SSID 字符串，未连接过则返回空串
 */
const char *esp8266_wifi_get_connected_ssid(void);

/**
 * @brief 将状态码转为可读字符串（用于 OLED 显示）
 * @param status 状态码
 * @return 静态字符串，勿 free
 */
const char *esp8266_status_string(esp8266_status_t status);

#endif /* ESP8266_H */
