#include "weather.h"
#include "weather_config.h"
#include "weather_json.h"
#include "esp8266.h"
#include <stdio.h>
#include <string.h>

static weather_info_t g_last_info;

static bool weather_api_configured(void)
{
    return (WEATHER_API_KEY[0] != '\0' &&
            strncmp(WEATHER_API_KEY, "your-api-key", 12) != 0);
}

weather_type_t weather_map_type(const char *main_str)
{
    if (main_str == NULL || main_str[0] == '\0') {
        return WEATHER_CLOUDY;
    }
    if (strcmp(main_str, "Clear") == 0) {
        return WEATHER_SUNNY;
    }
    if (strcmp(main_str, "Rain") == 0 ||
        strcmp(main_str, "Drizzle") == 0 ||
        strcmp(main_str, "Thunderstorm") == 0) {
        return WEATHER_RAINY;
    }
    return WEATHER_CLOUDY;
}

const char *weather_status_string(weather_status_t status)
{
    switch (status) {
    case WEATHER_OK:           return "OK";
    case WEATHER_ERR_CONFIG:   return "NO API KEY";
    case WEATHER_ERR_NETWORK:  return "NET FAIL";
    case WEATHER_ERR_PARSE:    return "PARSE ERR";
    default:                   return "UNKNOWN";
    }
}

bool weather_has_last(void)
{
    return g_last_info.valid;
}

const weather_info_t *weather_get_last(void)
{
    return g_last_info.valid ? &g_last_info : NULL;
}

weather_status_t weather_fetch(weather_info_t *info)
{
    char path[192];
    char body[768];
    char main_str[16];
    char desc[24];
    char city[24];
    float temp = 0.0f;
    float hum = 0.0f;
    size_t body_len = 0;
    const char *weather_section;

    if (info == NULL) {
        return WEATHER_ERR_PARSE;
    }

    if (!weather_api_configured()) {
        return WEATHER_ERR_CONFIG;
    }

    snprintf(path, sizeof(path),
             "/data/2.5/weather?q=%s&appid=%s&units=metric&lang=en",
             WEATHER_CITY, WEATHER_API_KEY);

    if (esp8266_http_get(WEATHER_API_HOST, WEATHER_API_PORT, path,
                         body, sizeof(body), &body_len,
                         WEATHER_HTTP_TIMEOUT_MS) != ESP8266_OK) {
        return WEATHER_ERR_NETWORK;
    }

    if (body[0] == '\0' || strstr(body, "\"cod\"") == NULL) {
        return WEATHER_ERR_PARSE;
    }

    if (!json_get_float(body, "temp", &temp)) {
        return WEATHER_ERR_PARSE;
    }

    if (!json_get_string(body, "name", city, sizeof(city))) {
        city[0] = '\0';
    }

    weather_section = strstr(body, "\"weather\"");
    if (weather_section == NULL) {
        return WEATHER_ERR_PARSE;
    }

    if (!json_get_string(weather_section, "main", main_str, sizeof(main_str))) {
        return WEATHER_ERR_PARSE;
    }

    if (!json_get_string(weather_section, "description", desc, sizeof(desc))) {
        desc[0] = '\0';
    }

    json_get_float(body, "humidity", &hum);

    info->temp_c = temp;
    info->humidity = (int8_t)hum;
    info->type = weather_map_type(main_str);
    strncpy(info->city, city, sizeof(info->city) - 1U);
    info->city[sizeof(info->city) - 1U] = '\0';
    strncpy(info->desc, desc, sizeof(info->desc) - 1U);
    info->desc[sizeof(info->desc) - 1U] = '\0';
    info->valid = true;

    g_last_info = *info;
    return WEATHER_OK;
}
