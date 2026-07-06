#ifndef WEATHER_CONFIG_H
#define WEATHER_CONFIG_H

/*
 * 以下宏由 PlatformIO 编译时从 .env / 环境变量注入
 * （见 extra_scripts/weather_env.py）
 *
 * 本地配置：复制 .env.example 为 .env 并编辑
 */

#ifndef WEATHER_API_KEY
#define WEATHER_API_KEY        ""
#endif

#ifndef WEATHER_CITY
#define WEATHER_CITY           "Shanghai,cn"
#endif

#ifndef WEATHER_API_HOST
#define WEATHER_API_HOST       "api.openweathermap.org"
#endif

#ifndef WEATHER_API_PORT
#define WEATHER_API_PORT       80U
#endif

#ifndef WEATHER_REFRESH_MS
#define WEATHER_REFRESH_MS     600000U
#endif

#ifndef WEATHER_RETRY_MS
#define WEATHER_RETRY_MS       60000U
#endif

#ifndef WEATHER_HTTP_TIMEOUT_MS
#define WEATHER_HTTP_TIMEOUT_MS 15000U
#endif

#endif /* WEATHER_CONFIG_H */
