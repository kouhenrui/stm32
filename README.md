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
│   ├── app_config.h      # WiFi 凭据等应用配置
│   └── main.h
├── src/
│   ├── main.c            # 启动流程与主循环
│   ├── stm32f1xx_hal_msp.c
│   └── stm32f1xx_it.c
├── lib/
│   ├── buzzer/           # 蜂鸣器驱动
│   ├── oled/             # SSD1306 + u8g2 驱动
│   └── esp8266/          # ESP8266 AT 指令封装
├── docs/
│   └── 硬件连接.md       # 全部模块接线说明
├── openspec/             # OpenSpec 规格管理
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

### 配置 WiFi（支持多个热点）

编辑 [`src/wifi_config.c`](src/wifi_config.c)，按优先级添加多个热点：

```c
const wifi_profile_t g_wifi_profiles[] = {
    { "家里WiFi",  "password1" },
    { "办公室WiFi", "password2" },
    { "手机热点",   "password3" },
};
```

启动时按数组顺序依次尝试，**连接成功即停止**。以 `your-ssid` 开头的条目视为未配置，会自动跳过。

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
3. 蜂鸣器短鸣提示
4. ESP8266 AT 连通测试，结果显示在 OLED
5. 若已配置 WiFi，尝试连接并显示结果

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
- [ ] TCP/UDP 数据上报
- [ ] 主循环状态机与业务逻辑

## 许可证

[MIT](LICENSE)
