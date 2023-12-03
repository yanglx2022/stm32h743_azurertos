/**
 * @brief   : 按键
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-11-26
 */

#include "key.h"

// 电源键 PH3
#define POWER_KEY_GPIO                  GPIOH
#define POWER_KEY_PIN                   LL_GPIO_PIN_3
// 音量+键 PA8
#define VOL_P_KEY_GPIO                  GPIOA
#define VOL_P_KEY_PIN                   LL_GPIO_PIN_8
// 音量-键 PG12
#define VOL_N_KEY_GPIO                  GPIOG
#define VOL_N_KEY_PIN                   LL_GPIO_PIN_12
// WakeUp键 PA0
#define WAKE_UP_KEY_GPIO                GPIOA
#define WAKE_UP_KEY_PIN                 LL_GPIO_PIN_0
// 触摸按键 PA1
#define TOUCH_KEY_GPIO                  GPIOA
#define TOUCH_KEY_PIN                   LL_GPIO_PIN_1

// 按键扫描周期
#define KEY_SCAN_PERIOD_MS              (5)

// 触摸按键阈值(电容充电时间 单位100ns)
#define TOUCH_KEY_THRESHOLD             (40)

// 按键状态
static key_state_t key_list[KEY_NUM] = {0};

/**
 * @brief : 按键初始化
 * @return 
 */
void Key_Init(void)
{
    // GPIO使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode    = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed   = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.Pull    = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Pin     = POWER_KEY_PIN;
    LL_GPIO_Init(POWER_KEY_GPIO, &GPIO_InitStruct);
    GPIO_InitStruct.Pin     = VOL_P_KEY_PIN;
    LL_GPIO_Init(VOL_P_KEY_GPIO, &GPIO_InitStruct);
    GPIO_InitStruct.Pin     = VOL_N_KEY_PIN;
    LL_GPIO_Init(VOL_N_KEY_GPIO, &GPIO_InitStruct);
    GPIO_InitStruct.Pull    = LL_GPIO_PULL_DOWN;
    GPIO_InitStruct.Pin     = WAKE_UP_KEY_PIN;
    LL_GPIO_Init(WAKE_UP_KEY_GPIO, &GPIO_InitStruct);

    // 定时器TIM5
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
    uint16_t prescaler  = Get_TIM_Clock_Freq(KEY_TIM5) / 10000000;  // 预分频100ns
    uint16_t autoreload = KEY_SCAN_PERIOD_MS * 10000;               // 计数值
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler        = prescaler - 1;
    TIM_InitStruct.CounterMode      = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload       = autoreload - 1;
    TIM_InitStruct.ClockDivision    = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(KEY_TIM5, &TIM_InitStruct);
    LL_TIM_SetClockSource(KEY_TIM5, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableMasterSlaveMode(KEY_TIM5);
    // TIM5_CH2捕获用于测量触摸按键充电时间
    LL_TIM_IC_SetActiveInput(KEY_TIM5, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(KEY_TIM5, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetFilter(KEY_TIM5, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1_N8);
    LL_TIM_IC_SetPolarity(KEY_TIM5, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
    LL_TIM_CC_EnableChannel(KEY_TIM5, LL_TIM_CHANNEL_CH2);

    // 中断
    LL_TIM_ClearFlag_UPDATE(KEY_TIM5);
    LL_TIM_EnableIT_UPDATE(KEY_TIM5);
    LL_TIM_ClearFlag_CC2(KEY_TIM5);
    LL_TIM_EnableIT_CC2(KEY_TIM5);
    NVIC_SetPriority(TIM5_IRQn, KEY_PRI);
    NVIC_EnableIRQ(TIM5_IRQn);

    // 启动定时器
    LL_TIM_EnableCounter(KEY_TIM5);
}

/**
 * @brief : 获取按键状态
 * @param  key 按键
 * @return 
 */
key_state_t Key_Get_State(uint32_t key)
{
    int index = 0;
    switch (key)
    {
        case KEY_POWER:
            index = 0;
            break;
        case KEY_VOL_P:
            index = 1;
            break;
        case KEY_VOL_N:
            index = 2;
            break;
        case KEY_WAKE_UP:
            index = 3;
            break;
        case KEY_TOUCH:
            index = 4;
            break;
        default:
            break;
    }

    key_state_t key_state = key_list[index];
    key_list[index].down = 0;
    key_list[index].up   = 0;
    
    return key_state;
}

/**
 * @brief : 触摸按键电容放电
 * @return 
 */
static void capacitor_discharge(void)
{
    // 配置为推挽输出
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = TOUCH_KEY_PIN;
    LL_GPIO_Init(TOUCH_KEY_GPIO, &GPIO_InitStruct);
    // 输出低电平给电容放电
    LL_GPIO_ResetOutputPin(TOUCH_KEY_GPIO, TOUCH_KEY_PIN);
}

/**
 * @brief : 触摸按键电容充电
 * @return 
 */
static void capacitor_charge(void)
{
    // 配置为定时器捕获输入(浮空输入)电容开始充电
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_2;
    GPIO_InitStruct.Pin         = TOUCH_KEY_PIN;
    LL_GPIO_Init(TOUCH_KEY_GPIO, &GPIO_InitStruct);
}

/**
 * @brief : TIM5中断 按键扫描
 * @param  
 * @return 
 */
void TIM5_IRQHandler(void)
{
    // 定时周期
    if (LL_TIM_IsActiveFlag_UPDATE(KEY_TIM5) != RESET)
    {
        // 实体按键
        uint32_t key_press[4];
        key_press[0] = LL_GPIO_IsInputPinSet(POWER_KEY_GPIO, POWER_KEY_PIN) == 0;   // 电源键按下
        key_press[1] = LL_GPIO_IsInputPinSet(VOL_P_KEY_GPIO, VOL_P_KEY_PIN) == 0;   // 音量+键按下
        key_press[2] = LL_GPIO_IsInputPinSet(VOL_N_KEY_GPIO, VOL_N_KEY_PIN) == 0;   // 音量-键按下
        key_press[3] = LL_GPIO_IsInputPinSet(WAKE_UP_KEY_GPIO, WAKE_UP_KEY_PIN);    // 唤醒键按下

        for(int i = 0; i < KEY_NUM - 1; i++)
        {
            if (key_press[i])   // 按下
            {
                if (key_list[i].press == 0)
                {
                    key_list[i].press   = 1;
                    key_list[i].down    = 1;
                    key_list[i].hold_ms = 0;
                }
                key_list[i].hold_ms += KEY_SCAN_PERIOD_MS;
            }
            else
            {
                if (key_list[i].press == 1)
                {
                    key_list[i].press = 0;
                    key_list[i].up    = 1;
                    key_list[i].count++;
                    if (key_list[i].count == 0)
                    {
                        key_list[i].count = 1;
                    }
                }
            }
        }

        // 触摸按键
        static uint8_t charge = 0;
        if (charge == 0)
        {
            charge = 1;
            capacitor_discharge();
        }
        else
        {
            charge = 0;
            capacitor_charge();
        }

        LL_TIM_ClearFlag_UPDATE(KEY_TIM5);
    }

    // 捕获上升沿测量充电时间
    if (LL_TIM_IsActiveFlag_CC2(KEY_TIM5) != RESET)
    {
        if (LL_TIM_IC_GetCaptureCH2(KEY_TIM5) > TOUCH_KEY_THRESHOLD)    // 按下
        {
            if (key_list[KEY_NUM - 1].press == 0)
            {
                key_list[KEY_NUM - 1].press   = 1;
                key_list[KEY_NUM - 1].down    = 1;
                key_list[KEY_NUM - 1].hold_ms = 0;
            }
            key_list[KEY_NUM - 1].hold_ms += KEY_SCAN_PERIOD_MS * 2;
        }
        else
        {
            if (key_list[KEY_NUM - 1].press == 1)
            {
                key_list[KEY_NUM - 1].press = 0;
                key_list[KEY_NUM - 1].up    = 1;
                key_list[KEY_NUM - 1].count++;
            }
        }

        LL_TIM_ClearFlag_CC2(KEY_TIM5);
    }
}


