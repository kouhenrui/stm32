#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/**
 * @brief 初始化蜂鸣器 GPIO（PB0，推挽输出）
 */
void buzzer_init(void);

/**
 * @brief 打开蜂鸣器（输出有效电平）
 */
void buzzer_on(void);

/**
 * @brief 关闭蜂鸣器
 */
void buzzer_off(void);

/**
 * @brief 蜂鸣器响一声后自动关闭
 * @param duration_ms 持续时长（毫秒）
 */
void buzzer_beep(uint32_t duration_ms);

#endif /* BUZZER_H */
