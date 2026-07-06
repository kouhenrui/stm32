## MODIFIED Requirements

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

## ADDED Requirements

### Requirement: 天气应用启动

系统上电完成 WiFi 连接后 SHALL 立即尝试获取首帧天气并显示在 OLED 上。

#### Scenario: 启动后首屏天气

- **GIVEN** ESP8266 AT 正常且 WiFi 连接成功
- **WHEN** 启动流程完成
- **THEN** OLED 显示当前天气界面（非仅 "WiFi OK"）
- **AND** 若首帧获取失败，显示错误并进入重试
