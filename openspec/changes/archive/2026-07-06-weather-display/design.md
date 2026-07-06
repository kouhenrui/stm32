## Context

- **现状**：ESP8266 驱动仅支持 AT 测试与 WiFi 连接；OLED 使用 u8g2 绘图 + HAL I2C 刷屏；主循环空转。
- **约束**：STM32F103C8 仅 64KB Flash / 20KB RAM；ESP8266 经 USART2（115200）阻塞收发；无 RTOS。
- **目标**：WiFi 连接后周期性获取天气，OLED 显示图标 + 文字。

## Goals / Non-Goals

**Goals:**

- 经 ESP8266 AT 固件完成 HTTP GET，解析当前天气
- OLED 显示 3 类图标：晴（太阳）、阴（多云）、雨（云+雨）
- 显示城市名、温度（°C）、简短描述
- 失败可重试，保留上次成功数据

**Non-Goals:**

- HTTPS、OTA、中文 UI、多日预报
- 硬件变更

## Decisions

### 1. 天气 API：OpenWeatherMap Current Weather

| 选项 | 优点 | 缺点 | 结论 |
|------|------|------|------|
| OpenWeatherMap | 文档全、JSON 字段固定 | 需注册 API Key | **选用** |
| 和风天气 | 国内快 | 签名/Key 配置复杂 | 备选 |
| wttr.in 纯文本 | 解析简单 | 格式不稳定 | 不选 |

请求示例（明文 HTTP，80 端口）：

```
GET /data/2.5/weather?q=Beijing,cn&appid=KEY&units=metric&lang=en HTTP/1.1
Host: api.openweathermap.org
Connection: close

```

解析字段：`main.temp`（浮点）、`weather[0].main`（Clear/Clouds/Rain 等）、`name`（城市）。

### 2. JSON 解析：轻量字符串扫描

不引入 cJSON（Flash 占用大）。手写 `json_get_string()` / `json_get_float()` 按 key 子串定位，足够解析 OpenWeatherMap 响应。

### 3. 天气类型映射 → 3 种图标

```
OpenWeatherMap weather.main  →  weather_type_t
─────────────────────────────────────────────────
Clear                        →  WEATHER_SUNNY
Clouds, Mist, Fog, Haze      →  WEATHER_CLOUDY
Rain, Drizzle, Thunderstorm  →  WEATHER_RAINY
Snow, 其他                     →  WEATHER_CLOUDY（默认）
```

### 4. ESP8266 HTTP 流程（阻塞 AT）

```
AT+CIPMUX=0
AT+CIPSTART="TCP","api.openweathermap.org",80
AT+CIPSEND=<len>
<HTTP request>
等待 +IPD,<len>:<body>
AT+CIPCLOSE
```

- RX 缓冲区：`ESP_RX_BUF_SIZE` 256 → **1024**（响应体约 400~600B）
- 单次 HTTP 超时：**15s**；CIPSTART 超时：**10s**
- 通信方式：**阻塞**，不使用 DMA

### 5. OLED 布局（128×64）

```
┌──────────────────────────────────┐
│ [32×32]   Beijing                │  y=0~15  城市
│  icon     23°C                   │  y=20~35  温度（大字体）
│           Cloudy                 │  y=42~55  描述
│ ──────────────────────────────── │  y=60     更新时间 / WiFi 状态
└──────────────────────────────────┘
```

- 图标：3 组 **32×32 XBM** 位图，`u8g2_DrawXBMP()`
- 字体：城市/描述 `u8g2_font_8x13_tf`；温度 `u8g2_font_logisoso16_tf` 或 `u8g2_font_helvB14_tf`
- 刷屏：沿用 `ssd1306_hw_flush()` 混合方案

### 6. 主循环状态机

```
INIT → WIFI_CONNECT → FETCH_WEATHER → DISPLAY → WAIT(10min) → FETCH_WEATHER ...
                              ↓ fail
                         DISPLAY(last + "Retry")
                              ↓
                         WAIT(60s) → retry
```

- `WEATHER_REFRESH_MS` = 600000（10 分钟）
- `WEATHER_RETRY_MS` = 60000（失败后 1 分钟重试）

### 7. 模块划分

```
lib/
├── esp8266/
│   ├── esp8266.c          (+ http_get)
│   └── esp8266_http.c     (可选拆分)
├── weather/
│   ├── weather.c          (fetch + parse + map)
│   ├── weather.h
│   ├── weather_json.c     (轻量解析)
│   └── weather_icons.c    (XBM 数据)
└── oled/
    └── weather_ui.c       (布局绘制)
include/
└── weather_config.h       (API_KEY, CITY, REFRESH_MS)
```

## HAL 外设使用

| 外设 | 用途 | 模式 |
|------|------|------|
| I2C1 | OLED 刷屏 | 阻塞 Master Transmit |
| USART2 | ESP8266 AT | 阻塞 TX/RX |
| USART1 | 调试日志（可选） | 阻塞 |
| GPIO | 无新增 | — |
| TIM/DMA | 不使用 | — |

## Risks / Trade-offs

| 风险 | 缓解 |
|------|------|
| HTTP 响应超 1KB 截断 | 1024B 缓冲 + 只解析 JSON body 段 |
| OpenWeatherMap 免费额度限流 | 10 分钟刷新；失败退避 |
| AT 固件版本差异（+IPD 格式） | 兼容 `+IPD,n:` 与 `+IPD,n,data` 两种 |
| Flash 超限 | `-Os` 已开；图标仅 3 组；不用 cJSON |
| 无 HTTPS | 仅传输公开天气，API Key 写在固件（接受风险） |

## Migration Plan

1. 实现并单测 ESP8266 HTTP（串口打印响应）
2. 实现 JSON 解析 + 类型映射（PC 侧可用样例 JSON 单元测试思路）
3. 实现 weather_ui 静态 mock 数据上屏
4. 集成 main 循环；`FEATURE_ESP8266=1`
5. 用户配置 `weather_config.h` 中的 API Key 与城市

## Open Questions

- [ ] 用户是否已有 OpenWeatherMap API Key？（需自行注册 free tier）
- [ ] 默认城市名？（建议配置为 `Beijing,cn`，用户可改）
- [ ] 是否需要显示湿度？（128×64 空间允许加一行 `Hum: 65%`）
