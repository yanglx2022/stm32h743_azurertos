/**
 * @brief   : 蜂鸣器
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-11-25
 */

#include "buzzer.h"

// PD2
#define BUZZER_GPIO                     GPIOD
#define BUZZER_PIN                      LL_GPIO_PIN_2

// 蜂鸣器参数(中心频率2.7kHz)
#define BUZZER_FREQUENCY_HZ             2700    // 频率2.7kHz

// 蜂鸣周期数
static uint32_t buzzer_cnt = 0;

/**
 * @brief : 蜂鸣器初始化
 * @return 
 */
void Buzzer_Init(void)
{
    // GPIO配置
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FAST;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = BUZZER_PIN;
    LL_GPIO_Init(BUZZER_GPIO, &GPIO_InitStruct);
    LL_GPIO_ResetOutputPin(BUZZER_GPIO, BUZZER_PIN);

    // 定时器TIM3
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    uint16_t prescaler  = 10;           // 预分频
    uint16_t autoreload = (uint32_t)    // 计数值
        ((float)Get_TIM_Clock_Freq(BUZZER_TIM3) / prescaler / BUZZER_FREQUENCY_HZ / 2 + 0.5f); // 定时周期
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler          = prescaler - 1;
    TIM_InitStruct.CounterMode        = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload         = autoreload - 1;
    TIM_InitStruct.ClockDivision      = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(BUZZER_TIM3, &TIM_InitStruct);
    LL_TIM_SetClockSource(BUZZER_TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableMasterSlaveMode(BUZZER_TIM3);

    // 中断
    LL_TIM_ClearFlag_UPDATE(BUZZER_TIM3);
    LL_TIM_EnableIT_UPDATE(BUZZER_TIM3);
    NVIC_SetPriority(TIM3_IRQn, BUZZER_PRI);
    NVIC_EnableIRQ(TIM3_IRQn);
}

/**
 * @brief : 蜂鸣一声
 * @param  t_ms 持续时间ms
 * @return 
 */
void Buzzer_Beep(uint16_t t_ms)
{
    buzzer_cnt = (int)BUZZER_FREQUENCY_HZ * t_ms / 1000;
    LL_TIM_EnableCounter(BUZZER_TIM3);
}

/**
 * @brief : TIM3中断 蜂鸣控制
 * @param  
 * @return 
 */
void TIM3_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(BUZZER_TIM3) != RESET)
    {
        if (buzzer_cnt == 0)
        {
            LL_TIM_DisableCounter(BUZZER_TIM3);
            LL_GPIO_ResetOutputPin(BUZZER_GPIO, BUZZER_PIN);
        }
        else
        {
            if (LL_GPIO_IsInputPinSet(BUZZER_GPIO, BUZZER_PIN))
            {
                LL_GPIO_ResetOutputPin(BUZZER_GPIO, BUZZER_PIN);
            }
            else
            {
                LL_GPIO_SetOutputPin(BUZZER_GPIO, BUZZER_PIN);
                buzzer_cnt--;
            }
        }

        LL_TIM_ClearFlag_UPDATE(BUZZER_TIM3);
    }
}


