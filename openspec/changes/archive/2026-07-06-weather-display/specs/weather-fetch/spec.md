## ADDED Requirements

### Requirement: HTTP 天气数据获取

系统 SHALL 在 WiFi 已连接时，经 ESP8266 向 OpenWeatherMap 发起 HTTP GET 请求，解析 JSON 响应并填充 `weather_info_t` 结构体（城市名、温度、描述、天气类型）。

#### Scenario: 成功获取当前天气

- **GIVEN** WiFi 已连接且 API Key 与城市配置有效
- **WHEN** 调用 `weather_fetch()`
- **THEN** 返回 `WEATHER_OK`
- **AND** `weather_info_t` 包含非空城市名、有效温度值（-50~60°C）
- **AND** `type` 为 `WEATHER_SUNNY`、`WEATHER_CLOUDY` 或 `WEATHER_RAINY` 之一

#### Scenario: 网络或 API 失败

- **GIVEN** WiFi 未连接或 HTTP 超时
- **WHEN** 调用 `weather_fetch()`
- **THEN** 返回错误码（`WEATHER_ERR_NETWORK` / `WEATHER_ERR_PARSE`）
- **AND** 上次成功的 `weather_info_t` 不被清空

#### Scenario: 天气类型映射

- **GIVEN** API 返回 `weather[0].main` 为 `Clear`
- **WHEN** 解析完成
- **THEN** `type` SHALL 为 `WEATHER_SUNNY`

- **GIVEN** API 返回 `weather[0].main` 为 `Rain` 或 `Drizzle` 或 `Thunderstorm`
- **WHEN** 解析完成
- **THEN** `type` SHALL 为 `WEATHER_RAINY`

- **GIVEN** API 返回 `weather[0].main` 为 `Clouds`
- **WHEN** 解析完成
- **THEN** `type` SHALL 为 `WEATHER_CLOUDY`

### Requirement: 周期性刷新

系统 SHALL 按配置的间隔（默认 10 分钟）自动重新拉取天气；失败后 SHALL 在 60 秒内重试。

#### Scenario: 定时刷新

- **GIVEN** 首帧天气已成功显示
- **WHEN** 距上次成功刷新超过 `WEATHER_REFRESH_MS`
- **THEN** 系统自动发起新一轮 `weather_fetch()`

#### Scenario: 失败退避

- **GIVEN** 最近一次 `weather_fetch()` 失败
- **WHEN** 距失败时间超过 `WEATHER_RETRY_MS`
- **THEN** 系统重试拉取
- **AND** OLED 仍显示上次成功数据并标注重试状态
