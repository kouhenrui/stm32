# STM32F103 嵌入式开发项目

基于 **STM32F103C8** 的固件工程，外接蜂鸣器、0.9 英寸 OLED（SSD1306）与 ESP8266 WiFi 模块。

## 硬件清单

| 组件 | 型号/规格 | 说明 |
|------|-----------|------|
| 主控 | STM32F103C8T6 | Cortex-M3，64KB Flash，20KB RAM |
| 显示屏 | 0.9" OLED（128×64，SSD1306） | I2C，**u8g2** 字体（默认 8×13） |
| 无线模块 | ESP8266 | UART AT 指令，115200 |
| 蜂鸣器 | 有源蜂鸣器 | GPIO 驱动（PB0） |

引脚定义见 [`include/pin_config.h`](include/pin_config.h)。**完整接线说明（OLED、蜂鸣器、ESP8266、调试串口、ST-Link）见 [`docs/硬件连接.md`](docs/硬件连接.md)。**

## 项目结构

```
stm32/
├── include/
│   ├── pin_config.h      # 引脚集中定义
│   ├── app_config.h      # 功能开关默认值（实际由 .env 注入）
│   ├── weather_config.h  # 天气参数默认值（实际由 .env 注入）
│   └── main.h
├── src/
│   ├── main.c            # 启动流程与天气主循环
│   ├── wifi_config.c     # 多 WiFi 热点配置
│   ├── stm32f1xx_hal_msp.c
│   └── stm32f1xx_it.c
├── lib/
│   ├── buzzer/           # 蜂鸣器驱动
│   ├── oled/             # SSD1306 + u8g2 + 天气 UI
│   ├── esp8266/          # ESP8266 AT + HTTP GET
│   └── weather/          # 天气拉取与 JSON 解析
├── docs/
│   └── 硬件连接.md       # 全部模块接线说明
├── .env.example          # 环境变量模板（复制为 .env）
├── platformio.ini        # PlatformIO 工程配置
└── README.md
```

## 快速开始

### 环境要求

- [PlatformIO](https://platformio.org/)（推荐通过 VS Code 插件或 `pip install platformio`）
- ST-Link 调试器

### 编译

```bash
python -m platformio run
```

### 烧录

```bash
python -m platformio run --target upload
```

### 环境变量配置（`.env`）

敏感信息与运行参数统一放在 **`.env`**，编译/烧录时由 `extra_scripts/env_config.py` 注入固件，**不写入 Git**。

```bash
copy .env.example .env    # Windows
# cp .env.example .env    # Linux/macOS
```

| 变量 | 敏感 | 说明 |
|------|:----:|------|
| `WEATHER_API_KEY` | 是 | OpenWeatherMap API Key |
| `WIFI_SSID_1` ~ `_3` | 是 | WiFi 名称（按优先级尝试） |
| `WIFI_PASSWORD_1` ~ `_3` | 是 | WiFi 密码 |
| `WEATHER_CITY` | 否 | 城市，如 `Shanghai,cn` |
| `WEATHER_API_HOST` | 否 | API 主机 |
| `WEATHER_API_PORT` | 否 | API 端口，默认 80 |
| `WEATHER_REFRESH_MS` | 否 | 天气刷新间隔（毫秒） |
| `WEATHER_RETRY_MS` | 否 | 失败后重试间隔 |
| `WEATHER_HTTP_TIMEOUT_MS` | 否 | HTTP 超时 |
| `FEATURE_BUZZER` | 否 | 0/1 蜂鸣器开关 |
| `FEATURE_ESP8266` | 否 | 0/1 ESP8266 开关 |
| `FEATURE_WEATHER` | 否 | 0/1 天气功能（依赖 ESP8266） |
| `ESP_AT_TIMEOUT_MS` | 否 | AT 指令超时 |
| `ESP_WIFI_RETRY_MAX` | 否 | WiFi 连接重试次数 |

`.env` 完整示例见 [`.env.example`](.env.example)。也可使用系统环境变量（优先级：**系统 env > `.env` 文件**）。

**保留在源码中（非环境变量）**：[`include/pin_config.h`](include/pin_config.h) 引脚接线、I2C 地址、波特率等硬件常量——与 PCB 绑定，不属于密钥。

配置完成后编译烧录：

```bash
python -m platformio run --target upload
```

烧录后 WiFi 连接成功会自动拉取上海天气，OLED 显示图标、温度、湿度；每 **10 分钟**刷新，失败 **60 秒**重试。

### OLED 字体（u8g2）

默认字体 **`u8g2_font_8x13_tf`**，在 `lib/oled/ssd1306.c` 的 `ssd1306_init()` 中修改，例如：

```c
u8g2_SetFont(&g_u8g2, u8g2_font_6x10_tf);   /* 更紧凑 */
u8g2_SetFont(&g_u8g2, u8g2_font_ncenB10_tr); /* 无衬线 */
```

高级绘图可通过 `ssd1306_get_u8g2()` 获取 `u8g2_t *` 后调用 `u8g2_DrawUTF8()` 等 API（需 `#include "u8g2.h"`）。

## 启动流程

上电后固件依次执行：

1. HAL 与外设初始化（I2C1、USART1/2）
2. OLED 显示启动信息
3. ESP8266 AT 连通测试
4. 连接 WiFi（多热点依次尝试）
5. 拉取 OpenWeatherMap 当前天气并显示（图标 + 城市/温度/描述）
6. 主循环定时刷新（10 分钟 / 失败 60 秒重试）

## 开发环境

本工程使用 **PlatformIO + STM32Cube HAL** 构建。也可将源码迁移至 STM32CubeIDE，保留 `lib/` 驱动与 `include/pin_config.h` 即可。

| 方式 | 说明 |
|------|------|
| PlatformIO | `platformio.ini`，已配置 ST-Link 烧录 |
| STM32CubeIDE | 需自行创建工程并导入源文件 |
| Keil MDK | 需自行配置工程 |

## OpenSpec

本项目使用 [OpenSpec](https://github.com/Fission-AI/OpenSpec) 管理规格与变更。

```bash
openspec validate --specs    # 校验规格
```

在 Cursor 中：

| 命令 | 用途 |
|------|------|
| `/opsx:propose "..."` | 创建功能变更提案 |
| `/opsx:apply` | 按 tasks 实现代码 |
| `/opsx:archive` | 归档已完成变更 |

## 功能状态

- [x] 系统时钟与 GPIO 初始化
- [x] SSD1306 OLED 驱动与文本显示
- [x] 蜂鸣器短鸣
- [x] ESP8266 AT 指令与 WiFi 连接
- [x] ESP8266 HTTP GET（OpenWeatherMap）
- [x] OLED 天气显示（图标 + 文字 + 湿度）
- [x] 主循环定时刷新与失败重试
- [ ] TCP/UDP 自定义数据上报

## 许可证

[MIT](LICENSE)
