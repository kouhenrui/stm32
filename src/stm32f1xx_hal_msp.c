#include "main.h"
#include "pin_config.h"

/**
 * @brief HAL I2C 底层初始化：配置 I2C1 引脚（PB6/PB7）
 * @param hi2c I2C 句柄
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef gpio = {0};

    if (hi2c->Instance == I2C1) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();

        gpio.Pin = OLED_I2C_SCL_PIN | OLED_I2C_SDA_PIN;
        gpio.Mode = GPIO_MODE_AF_OD;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &gpio);
    }
}

/**
 * @brief HAL I2C 底层反初始化
 * @param hi2c I2C 句柄
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        __HAL_RCC_I2C1_CLK_DISABLE();
        HAL_GPIO_DeInit(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN | OLED_I2C_SDA_PIN);
    }
}

/**
 * @brief HAL UART 底层初始化：USART1（调试）/ USART2（ESP8266）引脚
 * @param huart UART 句柄
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio = {0};

    if (huart->Instance == USART1) {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        gpio.Pin = LOG_UART_TX_PIN;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(LOG_UART_TX_PORT, &gpio);

        gpio.Pin = LOG_UART_RX_PIN;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(LOG_UART_RX_PORT, &gpio);
    } else if (huart->Instance == USART2) {
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        gpio.Pin = ESP_UART_TX_PIN;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(ESP_UART_TX_PORT, &gpio);

        gpio.Pin = ESP_UART_RX_PIN;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ESP_UART_RX_PORT, &gpio);
    }
}

/**
 * @brief HAL UART 底层反初始化
 * @param huart UART 句柄
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(LOG_UART_TX_PORT, LOG_UART_TX_PIN | LOG_UART_RX_PIN);
    } else if (huart->Instance == USART2) {
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(ESP_UART_TX_PORT, ESP_UART_TX_PIN | ESP_UART_RX_PIN);
    }
}
