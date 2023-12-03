/**
 * @brief   : 开关机
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-11-26
 */

#include "switch.h"
#include "key.h"

// 电源使能 PH2
#define POWER_ENABLE_GPIO               GPIOH
#define POWER_ENABLE_PIN                LL_GPIO_PIN_2

// 长按开机时间
#define SWITCH_ON_PRESS_MS              (1000)
// 长按关机时间
#define SWITCH_OFF_PRESS_MS             (1000)


/**
 * @brief : 开机(上电调用)
 * @return 
 */
void Switch_On(void)
{
    // GPIO配置
    // PH2 电源使能
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = POWER_ENABLE_PIN;
    LL_GPIO_Init(POWER_ENABLE_GPIO, &GPIO_InitStruct);

    while(1)
    {
        if (Key_Get_State(KEY_POWER).hold_ms >= SWITCH_ON_PRESS_MS)
        {
            LL_GPIO_SetOutputPin(POWER_ENABLE_GPIO, POWER_ENABLE_PIN);  // 电源使能
            break;
        }
        delay_ms(10);
    }
}

/**
 * @brief : 关机(循环调用)
 * @return 
 */
void Switch_Off(void)
{
    key_state_t key_state_t = Key_Get_State(KEY_POWER);
    if (key_state_t.press && key_state_t.count && (key_state_t.hold_ms >= SWITCH_OFF_PRESS_MS))
    {
        LL_GPIO_ResetOutputPin(POWER_ENABLE_GPIO, POWER_ENABLE_PIN);    // 电源关闭
        while(1);
    }
}


