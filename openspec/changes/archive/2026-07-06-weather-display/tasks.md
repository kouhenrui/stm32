## 1. 配置与工程结构

- [x] 1.1 新增 `include/weather_config.h`（`WEATHER_API_KEY`、`WEATHER_CITY`、`WEATHER_REFRESH_MS`、`WEATHER_RETRY_MS`）
- [x] 1.2 新增 `lib/weather/` 目录与 `platformio.ini` / `lib_extra_dirs` 确认可编译
- [x] 1.3 `app_config.h` 增加 `FEATURE_WEATHER=1`，启用时自动要求 `FEATURE_ESP8266=1`

## 2. ESP8266 HTTP 驱动层

- [x] 2.1 将 `ESP_RX_BUF_SIZE` 扩大至 1024，新增 `esp8266_http_get(host, port, path, body_out, body_max, timeout_ms)`
- [x] 2.2 实现 AT 序列：`CIPMUX=0` → `CIPSTART` → `CIPSEND` → 发送 HTTP 请求 → 解析 `+IPD` 响应体 → `CIPCLOSE`
- [x] 2.3 串口调试：WiFi 连接后 GET `api.openweathermap.org` 测试页，USART1 打印响应前 200 字符验证

## 3. 天气数据模块（weather-fetch）

- [x] 3.1 定义 `weather_info_t` 与 `weather_type_t`（`weather.h`）
- [x] 3.2 实现轻量 JSON 解析：`json_get_string()`、`json_get_float()`（`weather_json.c`）
- [x] 3.3 实现 `weather_fetch()`：拼装 OpenWeatherMap URL、调用 `esp8266_http_get`、解析 `main.temp` / `weather[0].main` / `name`
- [x] 3.4 实现 `weather_map_type()`：`Clear`→SUNNY，`Clouds`→CLOUDY，`Rain/Drizzle/Thunderstorm`→RAINY
- [x] 3.5 实现 `weather_get_last()` 缓存上次成功数据

## 4. 天气图标与 UI（weather-ui）

- [x] 4.1 绘制 3 组 32×32 XBM 位图：太阳、多云、下雨（`weather_icons.c`）
- [x] 4.2 实现 `weather_ui_draw(const weather_info_t *info)`：左侧图标 + 右侧城市/温度/描述
- [x] 4.3 实现 `weather_ui_draw_error(const char *msg)` 与重试中状态（底部 "Updating..."）
- [x] 4.4 用 mock 数据（不联网）验证三种图标在 OLED 上显示正确

## 5. 应用层集成

- [x] 5.1 修改 `main.c` 启动流程：WiFi OK 后立即 `weather_fetch()` + `weather_ui_draw()`
- [x] 5.2 主循环：计时刷新（10min）/ 失败退避（60s）状态机
- [x] 5.3 更新 `README.md`：OpenWeatherMap 注册说明、`weather_config.h` 配置示例

## 6. 验收

- [x] 6.1 晴天 API 响应 → OLED 显示太阳图标 + 温度
- [x] 6.2 断 WiFi → 显示错误/重试，恢复后自动更新
- [x] 6.3 `python -m platformio run` 编译通过，Flash 占用 < 55%
