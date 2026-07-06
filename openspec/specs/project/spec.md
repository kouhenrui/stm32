# 项目规格：STM32F103 物联网终端

## Purpose

定义 STM32F103 固件在蜂鸣器、OLED 与 ESP8266 协同下的系统行为，作为实现与验收的依据。

## Requirements

### Requirement: 系统启动

系统上电后 SHALL 完成时钟、GPIO 及外设初始化，并在 OLED 上显示启动状态。

#### Scenario: 正常上电

- **GIVEN** 开发板供电正常且外设接线正确
- **WHEN** 系统复位或上电
- **THEN** MCU 完成初始化
- **AND** OLED 显示启动信息或 Logo
- **AND** 蜂鸣器可发出短促提示音（可选）

### Requirement: OLED 显示

系统 SHALL 通过 SSD1306 驱动 128×64 OLED，支持显示文本与简单图形。

#### Scenario: 显示文本

- **GIVEN** OLED 已初始化
- **WHEN** 应用请求显示字符串
- **THEN** 指定内容在屏幕上正确渲染
- **AND** 单帧刷新不导致主循环长时间阻塞（< 50ms）

### Requirement: 蜂鸣器提示

系统 SHALL 通过 GPIO 或 PWM 控制蜂鸣器，支持短鸣、长鸣及告警模式。

#### Scenario: 短鸣提示

- **GIVEN** 蜂鸣器驱动已初始化
- **WHEN** 调用短鸣接口
- **THEN** 蜂鸣器响一声后自动停止
- **AND** 持续时间可配置（默认 100–300ms）

### Requirement: ESP8266 串口通信

系统 SHALL 通过 UART 与 ESP8266 通信，支持 AT 指令收发与响应解析。

#### Scenario: AT 连通测试

- **GIVEN** ESP8266 已上电且 UART 波特率为 115200
- **WHEN** 发送 `AT\r\n`
- **THEN** 在超时时间内收到 `OK` 响应

#### Scenario: WiFi 连接

- **GIVEN** ESP8266 AT 固件正常
- **WHEN** 配置 SSID 与密码并发起连接
- **THEN** OLED 或日志反映连接成功状态
- **AND** 失败时具备重试或错误提示机制

### Requirement: 模块协同

各外设模块 SHALL 通过清晰接口协作，避免在中断中执行耗时操作。

#### Scenario: 状态上报

- **GIVEN** WiFi 已连接
- **WHEN** 应用触发数据上报
- **THEN** 数据经 ESP8266 发送至指定服务端
- **AND** OLED 同步显示发送状态（成功/失败）

#### Scenario: 天气显示主循环

- **GIVEN** WiFi 已连接且天气模块已初始化
- **WHEN** 系统进入主循环
- **THEN** 系统 SHALL 周期性拉取天气并在 OLED 显示图标与文字
- **AND** HTTP 请求与 JSON 解析 SHALL 在主循环中执行，不在中断内
- **AND** 单次拉取阻塞时间 SHALL 不超过 20 秒

### Requirement: 天气应用启动

系统上电完成 WiFi 连接后 SHALL 立即尝试获取首帧天气并显示在 OLED 上。

#### Scenario: 启动后首屏天气

- **GIVEN** ESP8266 AT 正常且 WiFi 连接成功
- **WHEN** 启动流程完成
- **THEN** OLED 显示当前天气界面（非仅 "WiFi OK"）
- **AND** 若首帧获取失败，显示错误并进入重试
