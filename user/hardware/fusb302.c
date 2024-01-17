/**
 * @brief   : FUSB302硬件驱动
 * @author  : yanglx2022
 * @version : V0.1.0
 * @date    : 2023-12-17
 * @note    : 
 */

#include "fusb302.h"
#include "i2c.h"
#include "core.h"
#include "PD_Types.h"
#include "PDPolicy.h"
#include "lcd.h"

int source_capability_index = 0;

static DevicePolicyPtr_t dpm;
static Port_t port;

static void handle_core_event(FSC_U32 event, FSC_U8 portid, void *usr_ctx, void *app_ctx);


/**
 * @brief  FUSB302初始化
 * @return 
 */
void FUSB302_Init(void)
{
    // GPIO使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode    = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed   = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull    = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Pin     = FUSB302_INT_PIN;
    LL_GPIO_Init(FUSB302_INT_GPIO, &GPIO_InitStruct);
    // 外部中断
    LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE2);
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    EXTI_InitStruct.Line_0_31   = LL_EXTI_LINE_2;
    EXTI_InitStruct.Line_32_63  = LL_EXTI_LINE_NONE;
    EXTI_InitStruct.Line_64_95  = LL_EXTI_LINE_NONE;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode        = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
    NVIC_SetPriority(EXTI2_IRQn, FUSB302_PRI);
    NVIC_EnableIRQ(EXTI2_IRQn);

    // 定时器TIM2
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    uint16_t prescaler = Get_TIM_Clock_Freq(FUSB302_TIM2) / 1000000;    // 预分频1us
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler          = prescaler - 1;
    TIM_InitStruct.CounterMode        = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload         = 0xFFFFFFFF;
    TIM_InitStruct.ClockDivision      = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(FUSB302_TIM2, &TIM_InitStruct);
    LL_TIM_SetClockSource(FUSB302_TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableMasterSlaveMode(FUSB302_TIM2);
    // 输出比较(仅产生中断不控制IO输出)
    LL_TIM_OC_SetMode(FUSB302_TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_FROZEN);
    NVIC_SetPriority(TIM2_IRQn, FUSB302_PRI);
    NVIC_EnableIRQ(TIM2_IRQn);
    LL_TIM_EnableCounter(FUSB302_TIM2);

    // I2C初始化
    i2c_init();

    // FUSB302初始化
    DPM_Init(&dpm);
    port.dpm = dpm;
    port.PortID = 0;
    core_initialize(&port, FUSB300SlaveAddr);
    DPM_AddPort(dpm, &port);
    register_observer(EVENT_ALL, handle_core_event, NULL);
    PolicySendHardReset(&port); // 硬复位
}

/**
 * @brief  选择Source Capabilities
 * @param  index Source Capabilities索引
 * @return 
 */
void FUSB302_SrcCaps_Select(int index)
{
    source_capability_index = index;
    PolicySendHardReset(&port);
}

/**
 * @brief  开始定时
 * @param  microseconds 定时时间(us)
 * @return 
 */
static void timer_start(uint32_t microseconds)
{
    LL_TIM_OC_SetCompareCH1(FUSB302_TIM2, LL_TIM_GetCounter(FUSB302_TIM2) + microseconds);
    LL_TIM_ClearFlag_CC1(FUSB302_TIM2);
    LL_TIM_EnableIT_CC1(FUSB302_TIM2);
}

/**
 * @brief  停止定时
 * @return 
 */
static void timer_stop(void)
{
    LL_TIM_DisableIT_CC1(FUSB302_TIM2);
}

/**
 * @brief  事件处理回调函数
 * @param  event 事件
 * @param  portid
 * @param  *usr_ctx
 * @param  *app_ctx
 * @return 
 */
static void handle_core_event(FSC_U32 event, FSC_U8 portid, void *usr_ctx, void *app_ctx)
{
    if (event & CC1_ORIENT)
    {
        printf("CC1_ORIENT %d\n", (int)port.sourceOrSink);
    }
    if (event & CC2_ORIENT)
    {
        printf("CC2_ORIENT %d\n", (int)port.sourceOrSink);
    }
    if (event & CC_NO_ORIENT)
    {
        printf("CC_NO_ORIENT %d\n", (int)port.sourceOrSink);
    }
    if (event & PD_NEW_CONTRACT)
    {
        printf("PD_NEW_CONTRACT %d\n", (int)port.sourceOrSink);
        for(int i = 0; i < port.SrcCapsHeaderReceived.NumDataObjects; i++)
        {
            if ((port.SinkRequest.FVRDO.ObjectPosition - 1) == i)
            {
                printf("[*]");
                LCD_Print("[*]");
            }
            else
            {
                printf("[ ]");
                LCD_Print("[ ]");
            }
            printf("%dV\t%.2fA\n", port.SrcCapsReceived[i].FPDOSupply.Voltage * 50 / 1000, port.SrcCapsReceived[i].FPDOSupply.MaxCurrent * 10.0 / 1000);
            LCD_Print("%dV\t%.2fA\n", port.SrcCapsReceived[i].FPDOSupply.Voltage * 50 / 1000, port.SrcCapsReceived[i].FPDOSupply.MaxCurrent * 10.0 / 1000);
        }
    }
    if (event & PD_NO_CONTRACT)
    {
        printf("PD_NO_CONTRACT\n");
    }
    if (event & ACC_UNSUPPORTED)
    {
        printf("ACC_UNSUPPORTED\n");
    }
}

/**
 * @brief  运行Type-C及PD状态机
 * @return TRUE：需要继续运行  FALSE：等待下次中断
 */
static uint8_t state_machine_run(void)
{
    // 停止定时
    timer_stop();

    // 运行状态机
    core_state_machine(&port);

    // 中断状态仍有效则继续运行
    if(platform_get_device_irq_state(port.PortID))
    {
        return TRUE;
    }

    // 定时(超时)处理
    uint32_t timer_value = core_get_next_timeout(&port);
    if (timer_value > 0)
    {
        if (timer_value == 1)   // 1表示定时时间到
        {
            return TRUE;
        }
        else
        {
            // 启动定时
            timer_start(timer_value);
        }
    }

    return FALSE;
}

/**
 * @brief  GPIO外部中断
 * @return 
 */
void EXTI2_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET)
    {
        if (state_machine_run() == FALSE)
        {
            LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
        }
    }
}

/**
 * @brief  TIM2比较中断
 * @return 
 */
void TIM2_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_CC1(FUSB302_TIM2) != RESET)
    {
        if (state_machine_run() == FALSE)
        {
            LL_TIM_DisableIT_CC1(FUSB302_TIM2);
            LL_TIM_ClearFlag_CC1(FUSB302_TIM2);
        }
    }
}


