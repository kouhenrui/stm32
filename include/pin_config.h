#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "stm32f1xx_hal.h"

/* OLED — I2C1 */
#define OLED_I2C_INSTANCE      I2C1
#define OLED_I2C_SCL_PIN       GPIO_PIN_6
#define OLED_I2C_SCL_PORT      GPIOB
#define OLED_I2C_SDA_PIN       GPIO_PIN_7
#define OLED_I2C_SDA_PORT      GPIOB
#define OLED_I2C_ADDRESS       0x3C

/*
 * 面板类型：此前自研驱动在 SSD1306 下显示正常，保持 SSD1306。
 * 若换用 SH1106 模块再改为 1。
 */
#define OLED_USE_SH1106        0

/* I2C 时钟（Hz）：接线较长或花屏时可改为 100000 */
#define OLED_I2C_CLOCK_HZ      100000U

/* ESP8266 — USART2 */
#define ESP_UART_INSTANCE      USART2
#define ESP_UART_TX_PIN        GPIO_PIN_2
#define ESP_UART_TX_PORT       GPIOA
#define ESP_UART_RX_PIN        GPIO_PIN_3
#define ESP_UART_RX_PORT       GPIOA
#define ESP_UART_BAUDRATE      115200

/* 调试日志 — USART1 */
#define LOG_UART_INSTANCE      USART1
#define LOG_UART_TX_PIN        GPIO_PIN_9
#define LOG_UART_TX_PORT       GPIOA
#define LOG_UART_RX_PIN        GPIO_PIN_10
#define LOG_UART_RX_PORT       GPIOA
#define LOG_UART_BAUDRATE      115200

/* 蜂鸣器 — 有源，GPIO 高电平响 */
#define BUZZER_PIN             GPIO_PIN_0
#define BUZZER_PORT            GPIOB
#define BUZZER_ACTIVE_LEVEL    GPIO_PIN_SET

#endif /* PIN_CONFIG_H */
