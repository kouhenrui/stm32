# 硬件规格

## Purpose

记录 STM32F103 开发板与外设（OLED、ESP8266、蜂鸣器）的硬件参数与引脚分配，供驱动开发与接线核对使用。

## Requirements

### Requirement: 主控规格

主控 SHALL 为 STM32F103 系列（Cortex-M3，72 MHz），具体 Flash/RAM 依型号而定。

#### Scenario: 型号确认

- **GIVEN** 开发板已选定具体 MCU 型号（如 C8T6）
- **WHEN** 查阅数据手册或丝印
- **THEN** Flash、RAM 容量与型号一致
- **AND** 工程链接脚本与启动文件匹配该型号

### Requirement: OLED 硬件

0.9 英寸 OLED SHALL 采用 SSD1306 驱动，分辨率 128×64，工作电压 3.3V，接口为 I2C（优先）或 SPI。

#### Scenario: I2C 接线

- **GIVEN** OLED 模块为 I2C 接口
- **WHEN** 按规格连接 VCC、GND、SDA、SCL
- **THEN** 模块上电后地址可被扫描（通常 0x3C 或 0x3D）
- **AND** 通信电平为 3.3V

### Requirement: ESP8266 硬件

ESP8266 模块 SHALL 通过 UART 与 MCU 通信，工作电压 3.3V，默认波特率 115200。

#### Scenario: 正常运行模式

- **GIVEN** ESP8266 模块已供电
- **WHEN** EN/CH_PD 接 3.3V 且 GPIO0 接 3.3V
- **THEN** 模块进入 AT 固件运行模式
- **AND** UART TX/RX 与 MCU 交叉连接

### Requirement: 蜂鸣器硬件

蜂鸣器 SHALL 由 GPIO 或 TIM PWM 驱动；有源蜂鸣器可直接 GPIO，无源蜂鸣器需 PWM 方波。

#### Scenario: 有源蜂鸣器驱动

- **GIVEN** 使用有源蜂鸣器且经三极管驱动
- **WHEN** GPIO 输出高电平
- **THEN** 蜂鸣器发声
- **AND** GPIO 低电平时停止

### Requirement: 引脚分配草案

固件 SHALL 在 `pin_config.h`（或等价头文件）中集中定义引脚，并与本规格保持一致。

#### Scenario: 默认引脚映射

- **GIVEN** 使用推荐接线草案
- **WHEN** 查阅本规格引脚表
- **THEN** OLED SDA/SCL 为 B7/B6（代码 PB7/PB6，I2C1）
- **AND** ESP8266 UART 为 A2/A3（代码 PA2/PA3，USART2）
- **AND** 蜂鸣器为 B0（代码 PB0）

开发板排针丝印写作 **Ax / Bx**（无 `P` 前缀），与代码 **PAx / PBx** 一一对应。

| 功能 | 开发板丝印 | 代码名 | 外设 |
|------|------------|--------|------|
| OLED SDA | B7 | PB7 | I2C1 |
| OLED SCL | B6 | PB6 | I2C1 |
| ESP8266 TX → MCU RX | A3 | PA3 | USART2_RX |
| ESP8266 RX ← MCU TX | A2 | PA2 | USART2_TX |
| 蜂鸣器 | B0 | PB0 | GPIO / TIM |
| 调试 TX（可选） | A9 | PA9 | USART1_TX |
| 调试 RX（可选） | A10 | PA10 | USART1_RX |

> 若开发板已占用部分引脚，须在变更提案中更新本规格并同步代码。

### Requirement: 调试接口

工程 SHALL 支持 SWD 烧录调试，并可选 USART1（A9/A10，代码 PA9/PA10）输出日志。

#### Scenario: SWD 连接

- **GIVEN** ST-Link 或兼容调试器
- **WHEN** 连接 SWDIO、SWCLK、GND、3.3V
- **THEN** 可烧录 .hex/.bin 并单步调试
