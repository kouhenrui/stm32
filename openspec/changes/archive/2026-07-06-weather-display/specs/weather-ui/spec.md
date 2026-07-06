## ADDED Requirements

### Requirement: 天气图标显示

系统 SHALL 在 OLED 128×64 屏幕上，根据 `weather_type_t` 显示对应 32×32 像素图标：

| 类型 | 图标内容 |
|------|----------|
| `WEATHER_SUNNY` | 太阳（圆形 + 射线） |
| `WEATHER_CLOUDY` | 云朵 |
| `WEATHER_RAINY` | 云朵 + 雨滴 |

#### Scenario: 晴天图标

- **GIVEN** `weather_info_t.type` 为 `WEATHER_SUNNY`
- **WHEN** 调用 `weather_ui_draw(&info)`
- **THEN** OLED 左侧（x=0, y=16）显示太阳图标

#### Scenario: 阴天图标

- **GIVEN** `weather_info_t.type` 为 `WEATHER_CLOUDY`
- **WHEN** 调用 `weather_ui_draw(&info)`
- **THEN** OLED 左侧显示云朵图标

#### Scenario: 雨天图标

- **GIVEN** `weather_info_t.type` 为 `WEATHER_RAINY`
- **WHEN** 调用 `weather_ui_draw(&info)`
- **THEN** OLED 左侧显示云+雨图标

### Requirement: 天气文字信息

系统 SHALL 在图标右侧显示城市名、温度（含 °C 单位）和英文天气描述。

#### Scenario: 完整信息显示

- **GIVEN** `weather_info_t` 含 city="Beijing", temp=23.5, desc="Cloudy"
- **WHEN** 调用 `weather_ui_draw(&info)`
- **THEN** 屏幕显示 "Beijing"、"23°C"（或 "24°C" 四舍五入）、"Cloudy"
- **AND** 文字不与 32×32 图标区域重叠

#### Scenario: 刷新屏幕

- **GIVEN** 天气 UI 已绘制
- **WHEN** `weather_ui_draw()` 完成
- **THEN** 整屏刷新耗时 SHALL 小于 50ms（HAL I2C 直刷）

### Requirement: 错误状态显示

当无有效天气数据时，系统 SHALL 显示可读的错误提示。

#### Scenario: 首次获取失败

- **GIVEN** 从未成功获取过天气
- **WHEN** `weather_fetch()` 失败
- **THEN** OLED 显示 "Weather" / "Fetch fail" 或等价提示

#### Scenario: 重试中

- **GIVEN** 有历史成功数据且正在重试
- **WHEN** 绘制 UI
- **THEN** 显示上次数据
- **AND** 底部或第二行显示 "Updating..." 或 WiFi 断连提示
