#include "buzzer.h"
#include "pin_config.h"
#include "stm32f1xx_hal.h"

void buzzer_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio.Pin = BUZZER_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_PORT, &gpio);

    buzzer_off();
}

void buzzer_on(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, BUZZER_ACTIVE_LEVEL);
}

void buzzer_off(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN,
                      (BUZZER_ACTIVE_LEVEL == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void buzzer_beep(uint32_t duration_ms)
{
    buzzer_on();
    HAL_Delay(duration_ms);
    buzzer_off();
}
