#include "main.h"
#include "stm32f1xx_it.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/** @brief SysTick 中断：驱动 HAL 时基（HAL_Delay / HAL_GetTick） */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/** @brief USART1 中断服务函数 */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/** @brief USART2 中断服务函数 */
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

/** @brief I2C1 事件中断服务函数 */
void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&hi2c1);
}

/** @brief I2C1 错误中断服务函数 */
void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&hi2c1);
}
