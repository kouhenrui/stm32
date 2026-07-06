#include "weather_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool json_get_string(const char *json, const char *key, char *out, size_t out_sz)
{
    char pattern[32];
    const char *p;
    const char *start;
    const char *end;
    size_t len;

    if (json == NULL || key == NULL || out == NULL || out_sz == 0U) {
        return false;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) {
        return false;
    }

    p = strchr(p + strlen(pattern), ':');
    if (p == NULL) {
        return false;
    }
    p++;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p != '"') {
        return false;
    }
    p++;
    start = p;
    end = strchr(start, '"');
    if (end == NULL) {
        return false;
    }

    len = (size_t)(end - start);
    if (len >= out_sz) {
        len = out_sz - 1U;
    }
    memcpy(out, start, len);
    out[len] = '\0';
    return true;
}

bool json_get_float(const char *json, const char *key, float *out)
{
    char pattern[32];
    const char *p;

    if (json == NULL || key == NULL || out == NULL) {
        return false;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) {
        return false;
    }

    p = strchr(p + strlen(pattern), ':');
    if (p == NULL) {
        return false;
    }
    p++;
    while (*p == ' ' || *p == '\t') {
        p++;
    }

    *out = strtof(p, NULL);
    return true;
}
