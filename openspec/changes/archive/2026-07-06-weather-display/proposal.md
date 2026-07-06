## Why

当前固件已完成 OLED 显示与 ESP8266 WiFi 连接，但主循环无业务逻辑。用户需要在本地屏幕上实时查看天气（温度、描述、图标），形成完整的 IoT 终端体验。

## What Changes

- 扩展 ESP8266 驱动：支持 TCP 连接与 HTTP GET 请求（AT 指令）
- 新增天气模块：调用开放天气 API，解析 JSON，映射为晴/阴/雨三种状态
- 新增 OLED 天气界面：左侧天气图标（太阳、多云、下雨），右侧文字（城市、温度、描述）
- 主循环定时刷新（默认 10 分钟），失败时在 OLED 显示错误并退避重试
- 新增 `weather_config.h`：API Key、城市名、刷新间隔等配置
- 启用 `FEATURE_ESP8266=1`，启动流程改为连接 WiFi 后拉取首帧天气

## Non-goals

- 不改硬件接线与引脚
- 不做多日预报、空气质量、定位自动切换城市
- 不使用 RTOS / DMA；首版采用阻塞式 AT 通信
- 不支持 HTTPS（ESP8266 AT 固件 TCP 明文 HTTP 即可）
- 不增加中文字体（描述用英文或拼音缩写，如 Sunny / Cloudy / Rain）

## Capabilities

### New Capabilities

- `weather-fetch`: 经 ESP8266 发起 HTTP 请求、解析天气 JSON、维护 `weather_info_t` 数据结构
- `weather-ui`: 在 128×64 OLED 上绘制天气图标与文字布局

### Modified Capabilities

- `project`: 主循环从空转改为天气刷新状态机；启动流程增加首帧天气拉取

## Impact

| 区域 | 影响 |
|------|------|
| `lib/esp8266/` | 新增 `esp8266_http_get()` 等 TCP/HTTP 封装；RX 缓冲区扩大 |
| `lib/weather/` | 新模块：API 调用、轻量 JSON 解析、天气类型枚举 |
| `lib/oled/` | 新增 `weather_ui.c` 或扩展 `ssd1306` 绘制接口 |
| `include/weather_config.h` | API Key、城市、刷新间隔 |
| `src/main.c` | 主循环天气刷新逻辑 |
| `include/app_config.h` | `FEATURE_ESP8266=1`，可选 `FEATURE_WEATHER=1` |
| Flash/RAM | 预计 +4~8KB Flash（JSON 解析 + 3 组 XBM 图标）；RAM +512B（HTTP 响应缓冲） |
