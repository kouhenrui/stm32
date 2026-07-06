#include "esp8266.h"
#include "app_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define ESP_RX_BUF_SIZE   512U
#define ESP_WIFI_CMD_SIZE 128U

#ifndef ESP_WIFI_JAP_TIMEOUT_MS
#define ESP_WIFI_JAP_TIMEOUT_MS 35000U
#endif

#define ESP_HTTP_BUF_SIZE 1024U

static UART_HandleTypeDef *esp_uart;
static char rx_buf[ESP_RX_BUF_SIZE];
static char connected_ssid[33];
static char last_wifi_err[20] = "WIFI FAIL";

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

static void esp8266_set_wifi_err(const char *msg)
{
    if (msg != NULL) {
        strncpy(last_wifi_err, msg, sizeof(last_wifi_err) - 1U);
        last_wifi_err[sizeof(last_wifi_err) - 1U] = '\0';
    }
}

static void esp8266_parse_cwjap_error(const char *resp)
{
    if (strstr(resp, "+CWJAP:1") != NULL) {
        esp8266_set_wifi_err("JAP TIMEOUT");
    } else if (strstr(resp, "+CWJAP:2") != NULL) {
        esp8266_set_wifi_err("WRONG PASSWD");
    } else if (strstr(resp, "+CWJAP:3") != NULL) {
        esp8266_set_wifi_err("NO AP/2.4G?");
    } else if (strstr(resp, "+CWJAP:4") != NULL) {
        esp8266_set_wifi_err("CONN FAIL");
    } else {
        esp8266_set_wifi_err("WIFI FAIL");
    }
}

static esp8266_status_t esp8266_send_cwjap(const char *cmd, uint32_t timeout_ms)
{
    char tx[ESP_WIFI_CMD_SIZE];
    int len = snprintf(tx, sizeof(tx), "%s\r\n", cmd);
    size_t rx_len = 0;

    if (len <= 0 || (size_t)len >= sizeof(tx)) {
        return ESP8266_ERR_RESPONSE;
    }

    esp8266_flush_rx();
    memset(rx_buf, 0, sizeof(rx_buf));

    if (HAL_UART_Transmit(esp_uart, (uint8_t *)tx, (uint16_t)len, ESP_AT_TIMEOUT_MS) != HAL_OK) {
        esp8266_set_wifi_err("UART TX");
        return ESP8266_ERR_TIMEOUT;
    }

    {
        uint32_t start = HAL_GetTick();
        while ((HAL_GetTick() - start) < timeout_ms) {
            uint8_t ch;
            if (HAL_UART_Receive(esp_uart, &ch, 1, 50) != HAL_OK) {
                continue;
            }
            if (rx_len < sizeof(rx_buf) - 1U) {
                rx_buf[rx_len++] = (char)ch;
                rx_buf[rx_len] = '\0';
            }

            if (strstr(rx_buf, "WIFI GOT IP") != NULL) {
                return ESP8266_OK;
            }
            if (strstr(rx_buf, "OK") != NULL && strstr(rx_buf, "WIFI CONNECTED") != NULL) {
                return ESP8266_OK;
            }
            if (strstr(rx_buf, "FAIL") != NULL || strstr(rx_buf, "+CWJAP:") != NULL) {
                esp8266_parse_cwjap_error(rx_buf);
                return ESP8266_ERR_WIFI;
            }
            if (strstr(rx_buf, "ERROR") != NULL) {
                esp8266_set_wifi_err("AT ERROR");
                return ESP8266_ERR_RESPONSE;
            }
        }
    }

    esp8266_set_wifi_err("JAP TIMEOUT");
    return ESP8266_ERR_TIMEOUT;
}

/** @brief 发送 AT 并收集完整响应到 rx_buf */
static void esp8266_send_collect(const char *cmd, uint32_t timeout_ms)
{
    char tx[ESP_WIFI_CMD_SIZE];
    int len = snprintf(tx, sizeof(tx), "%s\r\n", cmd);
    size_t rx_len = 0;

    if (len <= 0 || (size_t)len >= sizeof(tx)) {
        return;
    }

    esp8266_flush_rx();
    memset(rx_buf, 0, sizeof(rx_buf));
    HAL_UART_Transmit(esp_uart, (uint8_t *)tx, (uint16_t)len, ESP_AT_TIMEOUT_MS);

    {
        uint32_t start = HAL_GetTick();
        while ((HAL_GetTick() - start) < timeout_ms) {
            uint8_t ch;
            if (HAL_UART_Receive(esp_uart, &ch, 1, 50) != HAL_OK) {
                continue;
            }
            if (rx_len < sizeof(rx_buf) - 1U) {
                rx_buf[rx_len++] = (char)ch;
                rx_buf[rx_len] = '\0';
            }
            if (strstr(rx_buf, "OK") != NULL || strstr(rx_buf, "FAIL") != NULL ||
                strstr(rx_buf, "ERROR") != NULL) {
                break;
            }
        }
    }
}

bool esp8266_wifi_ssid_visible(const char *ssid)
{
    char cmd[80];

    if (ssid == NULL || ssid[0] == '\0') {
        return false;
    }

    esp8266_send_cmd("AT+CWMODE=1", "OK", ESP_AT_TIMEOUT_MS);
    snprintf(cmd, sizeof(cmd), "AT+CWLAP=\"%s\"", ssid);
    esp8266_send_collect(cmd, 12000U);

    return (strstr(rx_buf, "+CWLAP:") != NULL && strstr(rx_buf, ssid) != NULL);
}

void esp8266_init(UART_HandleTypeDef *huart)
{
    esp_uart = huart;
    connected_ssid[0] = '\0';

    /* ESP8266 上电需 1~2s 才就绪，先等再清空启动日志 */
    HAL_Delay(2000);

    esp8266_flush_rx();
    {
        uint32_t drain_start = HAL_GetTick();
        while ((HAL_GetTick() - drain_start) < 1500U) {
            uint8_t ch;
            if (HAL_UART_Receive(esp_uart, &ch, 1, 20) != HAL_OK) {
                continue;
            }
        }
    }
    esp8266_flush_rx();
}

esp8266_status_t esp8266_test_at(void)
{
    for (uint8_t i = 0; i < 5U; i++) {
        esp8266_flush_rx();
        if (esp8266_send_cmd("AT", "OK", ESP_AT_TIMEOUT_MS) == ESP8266_OK) {
            esp8266_send_cmd("ATE0", "OK", ESP_AT_TIMEOUT_MS);
            esp8266_send_cmd("AT+CWMODE=1", "OK", ESP_AT_TIMEOUT_MS);
            return ESP8266_OK;
        }
        HAL_Delay(1000);
    }
    return ESP8266_ERR_TIMEOUT;
}

esp8266_status_t esp8266_wifi_connect(const char *ssid, const char *password)
{
    char cmd[ESP_WIFI_CMD_SIZE];

    if (ssid == NULL || password == NULL || ssid[0] == '\0') {
        return ESP8266_ERR_RESPONSE;
    }

    esp8266_send_cmd("AT+CWQAP", NULL, ESP_AT_TIMEOUT_MS);
    esp8266_send_cmd("AT+CWMODE=1", "OK", ESP_AT_TIMEOUT_MS);
    esp8266_send_cmd("AT+CWDHCP=1,1", "OK", ESP_AT_TIMEOUT_MS);

    if (!esp8266_wifi_ssid_visible(ssid)) {
        esp8266_set_wifi_err("NOT IN SCAN");
        return ESP8266_ERR_WIFI;
    }

    /* jap_timeout 最后一参：连接超时（秒），部分 AT 固件支持 */
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\",,,,,60", ssid, password);
    for (uint8_t retry = 0; retry < ESP_WIFI_RETRY_MAX; retry++) {
        esp8266_status_t st = esp8266_send_cwjap(cmd, ESP_WIFI_JAP_TIMEOUT_MS);
        if (st == ESP8266_OK) {
            strncpy(connected_ssid, ssid, sizeof(connected_ssid) - 1U);
            connected_ssid[sizeof(connected_ssid) - 1U] = '\0';
            esp8266_set_wifi_err("OK");
            return ESP8266_OK;
        }
        esp8266_send_cmd("AT+CWQAP", NULL, ESP_AT_TIMEOUT_MS);
        HAL_Delay(2000);
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

const char *esp8266_wifi_last_error(void)
{
    return last_wifi_err;
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

/** @brief 发送原始数据（无 \\r\\n） */
static esp8266_status_t esp8266_send_raw(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if (esp_uart == NULL || data == NULL || len == 0U) {
        return ESP8266_ERR_RESPONSE;
    }
    if (HAL_UART_Transmit(esp_uart, (uint8_t *)data, len, timeout_ms) != HAL_OK) {
        return ESP8266_ERR_TIMEOUT;
    }
    return ESP8266_OK;
}

/** @brief 从 HTTP 原始响应中提取 body（跳过 header） */
static void http_extract_body(char *buf)
{
    char *body = strstr(buf, "\r\n\r\n");
    if (body != NULL) {
        body += 4;
        memmove(buf, body, strlen(body) + 1U);
    }
}

/** @brief 去掉 +IPD 前缀与 HTTP 头，保留 JSON body */
static void http_normalize_response(char *buf)
{
    char *ipd = strstr(buf, "+IPD,");
    char *closed;

    if (ipd != NULL) {
        char *data = strchr(ipd, ':');
        if (data != NULL) {
            data++;
            memmove(buf, data, strlen(data) + 1U);
        }
    }

    closed = strstr(buf, "CLOSED");
    if (closed != NULL) {
        *closed = '\0';
    }

    http_extract_body(buf);
}

esp8266_status_t esp8266_http_get(const char *host, uint16_t port, const char *path,
                                  char *body_out, size_t body_max, size_t *body_len,
                                  uint32_t timeout_ms)
{
    char cmd[ESP_WIFI_CMD_SIZE];
    char req[256];
    char http_buf[ESP_HTTP_BUF_SIZE];
    int req_len;
    size_t collected = 0;
    bool got_ipd = false;

    if (host == NULL || path == NULL || body_out == NULL || body_max == 0U || esp_uart == NULL) {
        return ESP8266_ERR_RESPONSE;
    }

    req_len = snprintf(req, sizeof(req),
                       "GET %s HTTP/1.1\r\n"
                       "Host: %s\r\n"
                       "Connection: close\r\n"
                       "\r\n",
                       path, host);
    if (req_len <= 0 || (size_t)req_len >= sizeof(req)) {
        return ESP8266_ERR_RESPONSE;
    }

    if (esp8266_send_cmd("AT+CIPMUX=0", "OK", ESP_AT_TIMEOUT_MS) != ESP8266_OK) {
        return ESP8266_ERR_RESPONSE;
    }

    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u", host, (unsigned)port);
    if (esp8266_send_cmd(cmd, "OK", 10000U) != ESP8266_OK) {
        return ESP8266_ERR_RESPONSE;
    }

    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", req_len);
    esp8266_flush_rx();
    if (HAL_UART_Transmit(esp_uart, (uint8_t *)cmd, (uint16_t)strlen(cmd), ESP_AT_TIMEOUT_MS) != HAL_OK) {
        esp8266_send_cmd("AT+CIPCLOSE", NULL, ESP_AT_TIMEOUT_MS);
        return ESP8266_ERR_TIMEOUT;
    }
    HAL_UART_Transmit(esp_uart, (uint8_t *)"\r\n", 2, ESP_AT_TIMEOUT_MS);

    uint32_t start = HAL_GetTick();
    bool prompt = false;
    memset(rx_buf, 0, sizeof(rx_buf));
    size_t rx_len = 0;

    while ((HAL_GetTick() - start) < ESP_AT_TIMEOUT_MS) {
        uint8_t ch;
        if (HAL_UART_Receive(esp_uart, &ch, 1, 50) == HAL_OK) {
            if (rx_len < sizeof(rx_buf) - 1U) {
                rx_buf[rx_len++] = (char)ch;
                rx_buf[rx_len] = '\0';
            }
            if (strchr(rx_buf, '>') != NULL) {
                prompt = true;
                break;
            }
            if (strstr(rx_buf, "ERROR") != NULL) {
                esp8266_send_cmd("AT+CIPCLOSE", NULL, ESP_AT_TIMEOUT_MS);
                return ESP8266_ERR_RESPONSE;
            }
        }
    }

    if (!prompt) {
        esp8266_send_cmd("AT+CIPCLOSE", NULL, ESP_AT_TIMEOUT_MS);
        return ESP8266_ERR_TIMEOUT;
    }

    if (esp8266_send_raw((const uint8_t *)req, (uint16_t)req_len, ESP_AT_TIMEOUT_MS) != ESP8266_OK) {
        esp8266_send_cmd("AT+CIPCLOSE", NULL, ESP_AT_TIMEOUT_MS);
        return ESP8266_ERR_TIMEOUT;
    }

    memset(http_buf, 0, sizeof(http_buf));
    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms) {
        uint8_t ch;
        if (HAL_UART_Receive(esp_uart, &ch, 1, 50) != HAL_OK) {
            continue;
        }

        if (collected < sizeof(http_buf) - 1U) {
            http_buf[collected++] = (char)ch;
            http_buf[collected] = '\0';
        }

        if (!got_ipd && strstr(http_buf, "+IPD,") != NULL) {
            got_ipd = true;
        }

        if (got_ipd && strstr(http_buf, "CLOSED") != NULL) {
            break;
        }
    }

    esp8266_send_cmd("AT+CIPCLOSE", NULL, ESP_AT_TIMEOUT_MS);

    if (!got_ipd) {
        return ESP8266_ERR_TIMEOUT;
    }

    http_normalize_response(http_buf);

    if (body_len != NULL) {
        *body_len = strlen(http_buf);
    }
    strncpy(body_out, http_buf, body_max - 1U);
    body_out[body_max - 1U] = '\0';

    return ESP8266_OK;
}
