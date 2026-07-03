#include "esp8266.h"
#include "app_config.h"
#include <stdio.h>
#include <string.h>

#define ESP_RX_BUF_SIZE   256U
#define ESP_WIFI_CMD_SIZE 128U

static UART_HandleTypeDef *esp_uart;
static char rx_buf[ESP_RX_BUF_SIZE];
static char connected_ssid[33];

/** @brief 清空 UART 接收缓冲区中的残留数据 */
static void esp8266_flush_rx(void)
{
    uint8_t dummy;
    while (HAL_UART_Receive(esp_uart, &dummy, 1, 10) == HAL_OK) {
    }
}

/**
 * @brief 发送 AT 指令并等待期望响应
 * @param cmd AT 指令（不含 \\r\\n）
 * @param expect 期望出现的子串，NULL 表示不校验
 * @param timeout_ms 超时时间（毫秒）
 */
static esp8266_status_t esp8266_send_cmd(const char *cmd, const char *expect, uint32_t timeout_ms)
{
    char tx[ESP_WIFI_CMD_SIZE];
    int len = snprintf(tx, sizeof(tx), "%s\r\n", cmd);
    if (len <= 0 || (size_t)len >= sizeof(tx)) {
        return ESP8266_ERR_RESPONSE;
    }

    esp8266_flush_rx();
    if (HAL_UART_Transmit(esp_uart, (uint8_t *)tx, (uint16_t)len, timeout_ms) != HAL_OK) {
        return ESP8266_ERR_TIMEOUT;
    }

    uint32_t start = HAL_GetTick();
    size_t rx_len = 0;
    memset(rx_buf, 0, sizeof(rx_buf));

    while ((HAL_GetTick() - start) < timeout_ms) {
        uint8_t ch;
        if (HAL_UART_Receive(esp_uart, &ch, 1, 50) == HAL_OK) {
            if (rx_len < sizeof(rx_buf) - 1U) {
                rx_buf[rx_len++] = (char)ch;
                rx_buf[rx_len] = '\0';
            }
            if (expect != NULL && strstr(rx_buf, expect) != NULL) {
                return ESP8266_OK;
            }
            if (strstr(rx_buf, "ERROR") != NULL || strstr(rx_buf, "FAIL") != NULL) {
                return ESP8266_ERR_RESPONSE;
            }
        }
    }

    return (expect == NULL) ? ESP8266_OK : ESP8266_ERR_TIMEOUT;
}

void esp8266_init(UART_HandleTypeDef *huart)
{
    esp_uart = huart;
    connected_ssid[0] = '\0';
    HAL_Delay(500);
}

esp8266_status_t esp8266_test_at(void)
{
    for (uint8_t i = 0; i < 3U; i++) {
        if (esp8266_send_cmd("AT", "OK", ESP_AT_TIMEOUT_MS) == ESP8266_OK) {
            esp8266_send_cmd("ATE0", "OK", ESP_AT_TIMEOUT_MS);
            return ESP8266_OK;
        }
        HAL_Delay(500);
    }
    return ESP8266_ERR_TIMEOUT;
}

esp8266_status_t esp8266_wifi_connect(const char *ssid, const char *password)
{
    char cmd[ESP_WIFI_CMD_SIZE];

    if (ssid == NULL || password == NULL || ssid[0] == '\0') {
        return ESP8266_ERR_RESPONSE;
    }

    if (esp8266_send_cmd("AT+CWMODE=1", "OK", ESP_AT_TIMEOUT_MS) != ESP8266_OK) {
        return ESP8266_ERR_RESPONSE;
    }

    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    for (uint8_t retry = 0; retry < ESP_WIFI_RETRY_MAX; retry++) {
        if (esp8266_send_cmd(cmd, "OK", 15000U) == ESP8266_OK) {
            strncpy(connected_ssid, ssid, sizeof(connected_ssid) - 1U);
            connected_ssid[sizeof(connected_ssid) - 1U] = '\0';
            return ESP8266_OK;
        }
        HAL_Delay(1000);
    }
    return ESP8266_ERR_WIFI;
}

esp8266_status_t esp8266_wifi_connect_profiles(const wifi_profile_t *profiles,
                                               uint8_t count,
                                               uint8_t *connected_index)
{
    if (profiles == NULL || count == 0U) {
        return ESP8266_ERR_RESPONSE;
    }

    if (connected_index != NULL) {
        *connected_index = 0xFFU;
    }

    for (uint8_t i = 0; i < count; i++) {
        if (!wifi_profile_is_configured(&profiles[i])) {
            continue;
        }

        esp8266_status_t status = esp8266_wifi_connect(profiles[i].ssid, profiles[i].password);
        if (status == ESP8266_OK) {
            if (connected_index != NULL) {
                *connected_index = i;
            }
            return ESP8266_OK;
        }
    }

    return ESP8266_ERR_WIFI;
}

esp8266_status_t esp8266_wifi_connect_configured(uint8_t *connected_index)
{
    return esp8266_wifi_connect_profiles(g_wifi_profiles, g_wifi_profile_count, connected_index);
}

const char *esp8266_wifi_get_connected_ssid(void)
{
    return connected_ssid;
}

const char *esp8266_status_string(esp8266_status_t status)
{
    switch (status) {
    case ESP8266_OK:           return "OK";
    case ESP8266_ERR_TIMEOUT:  return "TIMEOUT";
    case ESP8266_ERR_RESPONSE: return "AT ERR";
    case ESP8266_ERR_WIFI:     return "WIFI FAIL";
    default:                   return "UNKNOWN";
    }
}
