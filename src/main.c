#include "main.h"
#include "app_config.h"
#include "buzzer.h"
#include "esp8266.h"
#include "pin_config.h"
#include "ssd1306.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void oled_show_status(const char *line1, const char *line2);

/**
 * @brief 程序入口：初始化外设，完成启动自检后进入主循环
 */
int main(void)
{
    /* HAL_Init — STM32 HAL 库，初始化 SysTick、NVIC 等
     * 定义: framework-stm32cubef1/.../stm32f1xx_hal.c */
    HAL_Init();

    /* SystemClock_Config — 配置 HSI+PLL 系统时钟
     * 定义: 本文件 static SystemClock_Config() */
    SystemClock_Config();

    /* MX_GPIO_Init — 使能 GPIOA/GPIOB 时钟
     * 定义: 本文件 static MX_GPIO_Init() */
    MX_GPIO_Init();

    /* MX_I2C1_Init — 初始化 I2C1（400kHz，OLED）
     * 定义: 本文件 static MX_I2C1_Init()
     * 引脚 MSP: src/stm32f1xx_hal_msp.c HAL_I2C_MspInit() */
    MX_I2C1_Init();

    /* MX_USART1_UART_Init — 初始化 USART1（调试串口 PA9/PA10）
     * 定义: 本文件 static MX_USART1_UART_Init()
     * 引脚 MSP: src/stm32f1xx_hal_msp.c HAL_UART_MspInit() */
    MX_USART1_UART_Init();

    /* MX_USART2_UART_Init — 初始化 USART2（ESP8266，PA2/PA3）
     * 定义: 本文件 static MX_USART2_UART_Init()
     * 引脚 MSP: src/stm32f1xx_hal_msp.c HAL_UART_MspInit() */
    MX_USART2_UART_Init();

#if FEATURE_BUZZER
    buzzer_init();
#endif

    ssd1306_init(&hi2c1);

#if FEATURE_ESP8266
    esp8266_init(&huart2);
#endif

#if FEATURE_BUZZER
    buzzer_beep(150);
#endif

    oled_show_status("STM32F103", "Booting...");

#if FEATURE_ESP8266
    esp8266_status_t at_status = esp8266_test_at();
    if (at_status == ESP8266_OK) {
        oled_show_status("ESP8266", "AT OK");
    } else {
        oled_show_status("ESP8266", esp8266_status_string(at_status));
    }

    uint8_t wifi_index = 0xFFU;
    esp8266_status_t wifi_status = esp8266_wifi_connect_configured(&wifi_index);
    if (wifi_status == ESP8266_OK) {
        oled_show_status("WiFi OK", esp8266_wifi_get_connected_ssid());
    } else {
        oled_show_status("WiFi", esp8266_status_string(wifi_status));
    }
#else
    oled_show_status("OLED Test", "OK");
#endif

#if FEATURE_BUZZER
    buzzer_beep(100);
#endif

    while (1) {
        /* HAL_Delay — 阻塞延时，依赖 SysTick 时基
         * 定义: framework-stm32cubef1/.../stm32f1xx_hal.c */
        HAL_Delay(1000);
    }
}

/**
 * @brief 在 OLED 上显示两行文本并刷新
 * @param line1 第一行（页 0）
 * @param line2 第二行（页 2）
 */
static void oled_show_status(const char *line1, const char *line2)
{
    /* ssd1306_clear / ssd1306_draw_string / ssd1306_update
     * 定义: lib/oled/ssd1306.c */
    ssd1306_clear();
    ssd1306_draw_string(0, 0, line1);
    ssd1306_draw_string(0, 2, line2);
    ssd1306_update();
}

/**
 * @brief 配置系统时钟：HSI + PLL，SYSCLK 64MHz
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    /* HAL_RCC_OscConfig / HAL_RCC_ClockConfig — RCC 时钟配置
     * 定义: framework-stm32cubef1/.../stm32f1xx_hal_rcc.c */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    osc.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/** @brief 使能 GPIOA / GPIOB 时钟 */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/** @brief 初始化 I2C1（400kHz，供 OLED 使用） */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = OLED_I2C_CLOCK_HZ;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    /* HAL_I2C_Init — 定义: framework-stm32cubef1/.../stm32f1xx_hal_i2c.c */
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

/** @brief 初始化 USART1（调试日志，PA9/PA10） */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = LOG_UART_BAUDRATE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    /* HAL_UART_Init — 定义: framework-stm32cubef1/.../stm32f1xx_hal_uart.c */
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

/** @brief 初始化 USART2（ESP8266，PA2/PA3） */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = ESP_UART_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief HAL 断言失败回调
 * @param file 源文件名
 * @param line 行号
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler();
}
#endif
