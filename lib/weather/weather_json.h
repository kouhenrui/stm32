#ifndef WEATHER_JSON_H
#define WEATHER_JSON_H

#include <stdbool.h>
#include <stddef.h>

bool json_get_string(const char *json, const char *key, char *out, size_t out_sz);
bool json_get_float(const char *json, const char *key, float *out);

#endif /* WEATHER_JSON_H */
