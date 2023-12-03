/**
 * @brief   : 红外遥控
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-12-02
 */

#include "remote.h"

// PD13
#define REMOTE_GPIO                     GPIOD
#define REMOTE_PIN                      LL_GPIO_PIN_13

// 定时器周期
#define REMOTE_TIM_PERIOD_MS            (60)

// 遥控器发送周期
#define REMOTE_SEND_PERIOD_MS           (108)

// 判断脉宽
#define IS_PULSE_MATCH(a, b)            (((a) >= (b).lower) && ((a) <= (b).upper))

// 脉宽参数(us)(NEC UPD6121G格式)            参考值 下限值 上限值
static const pulse_width_t PULSE_LEADER_N = {9000, 8000, 10000};    // 引导码负脉宽
static const pulse_width_t PULSE_LEADER_P = {4500, 4000, 6000 };    // 引导码正脉宽
static const pulse_width_t PULSE_REPEAT_P = {2250, 2000, 2500 };    // 重复码正脉宽
static const pulse_width_t PULSE_DATA_N   = {560,  500,  620  };    // 数据负脉宽
static const pulse_width_t PULSE_DATA0_P  = {565,  505,  625  };    // 数据bit0正脉宽
static const pulse_width_t PULSE_DATA1_P  = {1690, 1500, 1880 };    // 数据bit1正脉宽

// 遥控键值
static remote_key_t remote_key;


/**
 * @brief : 红外遥控初始化
 * @return 
 */
void Remote_Init(void)
{
    // GPIO使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode      = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed     = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull      = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
    GPIO_InitStruct.Pin       = REMOTE_PIN;
    LL_GPIO_Init(REMOTE_GPIO, &GPIO_InitStruct);

    // 定时器TIM4
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
    uint16_t prescaler  = Get_TIM_Clock_Freq(REMOTE_TIM4) / 1000000;  // 预分频1us
    uint16_t autoreload = REMOTE_TIM_PERIOD_MS * 1000;                // 计数值
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler        = prescaler - 1;
    TIM_InitStruct.CounterMode      = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload       = autoreload - 1;
    TIM_InitStruct.ClockDivision    = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(REMOTE_TIM4, &TIM_InitStruct);
    LL_TIM_SetClockSource(REMOTE_TIM4, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableMasterSlaveMode(REMOTE_TIM4);
    // TIM4_CH2捕获用于测量脉宽
    LL_TIM_IC_SetActiveInput(REMOTE_TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(REMOTE_TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetFilter(REMOTE_TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1_N4);
    LL_TIM_IC_SetPolarity(REMOTE_TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_BOTHEDGE);
    LL_TIM_CC_EnableChannel(REMOTE_TIM4, LL_TIM_CHANNEL_CH2);

    // 中断
    LL_TIM_ClearFlag_UPDATE(REMOTE_TIM4);
    LL_TIM_EnableIT_UPDATE(REMOTE_TIM4);
    LL_TIM_ClearFlag_CC2(REMOTE_TIM4);
    LL_TIM_EnableIT_CC2(REMOTE_TIM4);
    NVIC_SetPriority(TIM4_IRQn, REMOTE_PRI);
    NVIC_EnableIRQ(TIM4_IRQn);

    // 启动定时器
    LL_TIM_EnableCounter(REMOTE_TIM4);
}

/**
 * @brief  红外遥控
 * @return 红外遥控键值
 */
remote_key_t Remote_Key_State(void)
{
    remote_key_t key = remote_key;
    if (key.down)
    {
        remote_key.down = 0;
    }
    if (key.up)
    {
        remote_key.up    = 0;
        remote_key.code  = 0;
    }

    return key;
}

/**
 * @brief : TIM4中断 红外遥控
 * @param  
 * @return 
 */
void TIM4_IRQHandler(void)
{
    static remote_state_t state = LEADER;   // 解码状态
    static uint32_t pulse_n_width_us = 0;   // 负脉宽
    static uint32_t pulse_p_width_us = 0;   // 正脉宽
    static uint8_t  bit_cnt = 0;            // 数据位计数
    static uint32_t data    = 0;            // 数据

    // 定时周期
    if (LL_TIM_IsActiveFlag_UPDATE(REMOTE_TIM4) != RESET)
    {
        if (LL_GPIO_IsInputPinSet(REMOTE_GPIO, REMOTE_PIN)) // 正脉冲
        {
            if (pulse_p_width_us < REMOTE_SEND_PERIOD_MS * 1000)
            {
                pulse_p_width_us += REMOTE_TIM_PERIOD_MS * 1000;
            }
            else    // 超过周期的正脉冲认为是按键抬起
            {
                if (remote_key.press)
                {
                    remote_key.press = 0;
                    remote_key.up    = 1;
                }
            }
        }
        else    // 负脉冲
        {
            if (pulse_p_width_us < REMOTE_SEND_PERIOD_MS * 1000)    // 防止溢出
            {
                pulse_n_width_us += REMOTE_TIM_PERIOD_MS * 1000;
            }
        }

        LL_TIM_ClearFlag_UPDATE(REMOTE_TIM4);
    }

    // 捕获脉冲
    if (LL_TIM_IsActiveFlag_CC2(REMOTE_TIM4) != RESET)
    {
        LL_TIM_SetCounter(REMOTE_TIM4, 0);  // 为方便计算脉宽计数清零
        if (LL_GPIO_IsInputPinSet(REMOTE_GPIO, REMOTE_PIN))     // 上升沿
        {
            pulse_n_width_us += LL_TIM_IC_GetCaptureCH2(REMOTE_TIM4);
            pulse_p_width_us = 0;
        }
        else    // 下降沿
        {
            pulse_p_width_us += LL_TIM_IC_GetCaptureCH2(REMOTE_TIM4);
            switch (state)
            {
                case LEADER:
                    if (IS_PULSE_MATCH(pulse_n_width_us, PULSE_LEADER_N))
                    {
                        if (IS_PULSE_MATCH(pulse_p_width_us, PULSE_LEADER_P))       // 引导码
                        {
                            bit_cnt = 0;
                            data    = 0;
                            state = DATA;
                        }
                        else if (IS_PULSE_MATCH(pulse_p_width_us, PULSE_REPEAT_P))  // 连发码
                        {
                            remote_key.hold_ms += REMOTE_SEND_PERIOD_MS;
                            state = STOP;
                        }
                    }
                    if (state != STOP)  // 两个按键间隔较短或者解码错误均输出上一个按键抬起
                    {
                        if (remote_key.press)
                        {
                            remote_key.press = 0;
                            remote_key.up    = 1;
                        }
                    }
                    break;
                case DATA:
                    state = LEADER;
                    if (IS_PULSE_MATCH(pulse_n_width_us, PULSE_DATA_N))
                    {
                        if (IS_PULSE_MATCH(pulse_p_width_us, PULSE_DATA0_P))        // 数据bit0
                        {
                            bit_cnt++;
                            state = DATA;
                        }
                        else if (IS_PULSE_MATCH(pulse_p_width_us, PULSE_DATA1_P))   // 数据bit1
                        {
                            data |= 1 << bit_cnt;
                            bit_cnt++;
                            state = DATA;
                        }
                        if (bit_cnt == 32)  // 数据解码完成
                        {
                            // 不判地址只判键值
                            if ((((uint8_t *)&data)[2] ^ ((uint8_t *)&data)[3]) == 0xFF)
                            {
                                remote_key.code    = ((uint8_t *)&data)[2]; // 输出键值
                                remote_key.press   = 1;
                                remote_key.down    = 1;
                                remote_key.hold_ms = REMOTE_SEND_PERIOD_MS;
                            }
                            else
                            {
                                remote_key.code    = 0;
                                remote_key.press   = 0;
                                remote_key.down    = 0;
                                remote_key.hold_ms = 0;
                            }
                            state = STOP;
                        }
                    }
                    break;
                case STOP:  // 跳过停止位判断(避免解析为错影响连发)
                    state = LEADER;
                    break;
                default:
                    break;
            }

            pulse_n_width_us = 0;
        }

        LL_TIM_ClearFlag_CC2(REMOTE_TIM4);
    }
}


