#include "main.h"
#include "app_config.h"
#include "buzzer.h"
#include "esp8266.h"
#include "pin_config.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>

#if FEATURE_WEATHER
#include "weather.h"
#include "weather_config.h"
#include "weather_ui.h"
#endif

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void oled_show_status(const char *line1, const char *line2);

#if FEATURE_WEATHER
static void log_print(const char *msg);
static weather_status_t weather_fetch_and_show(bool show_updating);
static void weather_main_loop(void);
#endif

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
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
    if (at_status != ESP8266_OK) {
        oled_show_status("AT TIMEOUT", "Check A2 A3 EN");
        while (1) {
            HAL_Delay(1000);
        }
    }

    oled_show_status("ESP8266", "AT OK");

    uint8_t wifi_index = 0xFFU;
    esp8266_status_t wifi_status = ESP8266_ERR_WIFI;
    while (wifi_status != ESP8266_OK) {
        oled_show_status("Scan WiFi", "2.4GHz only");
        HAL_Delay(500);
        oled_show_status("WiFi...", "Connecting");
        wifi_status = esp8266_wifi_connect_configured(&wifi_index);
        if (wifi_status != ESP8266_OK) {
            oled_show_status(esp8266_wifi_last_error(), "Retry 15s");
            HAL_Delay(15000);
        }
    }

#if FEATURE_WEATHER
    oled_show_status("WiFi OK", esp8266_wifi_get_connected_ssid());
    HAL_Delay(800);

    {
        weather_status_t boot_status = weather_fetch_and_show(false);
        if (boot_status != WEATHER_OK) {
            if (weather_has_last()) {
                weather_ui_draw(weather_get_last(), "Retry...");
            } else {
                weather_ui_draw_error("Weather", weather_status_string(boot_status));
            }
        }
    }

    weather_main_loop();
#else
    oled_show_status("WiFi OK", esp8266_wifi_get_connected_ssid());
    while (1) {
        HAL_Delay(1000);
    }
#endif

#else
    oled_show_status("OLED Test", "OK");
    while (1) {
        HAL_Delay(1000);
    }
#endif

#if FEATURE_BUZZER
    buzzer_beep(100);
#endif
}

#if FEATURE_WEATHER
static void log_print(const char *msg)
{
    if (msg != NULL) {
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, (uint16_t)strlen(msg), 1000);
    }
}

static weather_status_t weather_fetch_and_show(bool show_updating)
{
    weather_info_t info;
    weather_status_t status;

    if (show_updating && weather_has_last()) {
        weather_ui_draw(weather_get_last(), "Updating...");
    }

    status = weather_fetch(&info);
    log_print("weather fetch: ");
    log_print(weather_status_string(status));
    log_print("\r\n");

    if (status == WEATHER_OK) {
        weather_ui_draw(&info, NULL);
    }

    return status;
}

static void weather_main_loop(void)
{
    uint32_t last_attempt = HAL_GetTick();
    bool last_ok = weather_has_last();

    while (1) {
        uint32_t now = HAL_GetTick();
        uint32_t interval = last_ok ? WEATHER_REFRESH_MS : WEATHER_RETRY_MS;

        if ((now - last_attempt) >= interval) {
            weather_status_t status = weather_fetch_and_show(last_ok);

            last_attempt = now;
            if (status == WEATHER_OK) {
                last_ok = true;
            } else if (weather_has_last()) {
                weather_ui_draw(weather_get_last(), "Updating...");
            } else {
                weather_ui_draw_error("Weather", weather_status_string(status));
            }
        }

        HAL_Delay(500);
    }
}
#endif

static void oled_show_status(const char *line1, const char *line2)
{
    ssd1306_clear();
    ssd1306_draw_string(0, 0, line1);
    ssd1306_draw_string(0, 2, line2);
    ssd1306_update();
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

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

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

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
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

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
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

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
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler();
}
#endif
