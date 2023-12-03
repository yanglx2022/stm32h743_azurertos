/**
 * @brief   : MPU 时钟 延时 printf串口打印等
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-02
 */

#ifndef COMMON_H
#define COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_fmc.h"
#include "stm32h7xx_ll_tim.h"
#include "stdio.h"
#include "string.h"
#include "memory.h"
#include "tx_api.h"
#include "fx_api.h"
#include "ux_api.h"


// 版本号
extern const uint32_t VERSION;

// UID
#define CPU_ID0                         (*(uint32_t*)0x1FF1E800)
#define CPU_ID1                         (*(uint32_t*)0x1FF1E804)
#define CPU_ID2                         (*(uint32_t*)0x1FF1E808)

// 定时器资源分配
#define BUZZER_TIM2                     TIM2
#define REMOTE_TIM4                     TIM4
#define KEY_TIM5                        TIM5

#define STACK_SIZE_DEFAULT              (2048)

// 中断优先级分配
// 注意: SysTick优先级4(tx_initialize_low_level.S中设置)
#define BUZZER_PRI                      (5)
#define KEY_PRI                         (6)
#define REMOTE_PRI                      (7)
#define SDCARD_PRI                      (9)
#define USB_PRI                         (12)

// 任务优先级分配
#define FILE_THREAD_PRI                 (11)
#define IDLE_THREAD_PRI                 (31)

// 硬件驱动返回值
typedef enum
{
    HR_OK = 0,              // 成功
    HR_ERROR,               // 未知错误
    HR_NO_DEVICE,           // 无设备
    HR_TIMEOUT,             // 超时
    HR_SD_INSERT,           // SD卡插入
    HR_SD_REMOVAL,          // SD卡拔出
    HR_INVALID_ADDRESS,     // 地址无效
    HR_ECC_ERROR            // ECC校验失败
} HRESULT;

// LED PB1
#define LED_ON()                        (LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_1))
#define LED_OFF()                       (LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_1))
#define LED_TOGGLE()                    (LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_1))

/**
 * @brief : 获取cpu tick值(400MHz时1tick=2.5ns 最大2^32*2.5ns=10.7s)
 * @param  
 * @return cpu tick
 */
__STATIC_INLINE uint32_t get_cpu_tick(void)
{
    return (DWT->CYCCNT);
}

/**
 * @brief : cpu tick转ns(主频400MHz,约2.15s溢出)
 * @param  cpu_tick cpu tick值
 * @return ns
 */
__STATIC_INLINE uint32_t cpu_tick_to_ns(uint32_t cpu_tick)
{
    return ((cpu_tick) * 5 / 2);
}

/**
 * @brief : ns转cpu tick(主频400MHz)
 * @param  ns ns值
 * @return cpu tick
 */
__STATIC_INLINE uint32_t ns_to_cpu_tick(uint32_t ns)
{
    return ((ns) / 5 * 2 + 1);
}

/**
 * @brief : ns延时(精度2.5ns)
 * @param  num ns数
 * @return 
 */
__STATIC_INLINE void delay_ns(uint32_t num)
{
    uint32_t start = DWT->CYCCNT;
    num = ns_to_cpu_tick(num);
    while((DWT->CYCCNT - start) < num);
}


void delay_us(uint32_t num);
void delay_ms(uint32_t num);
uint32_t get_tick_ms(void);
uint32_t Get_TIM_Clock_Freq(TIM_TypeDef *TIMx);

#ifdef __cplusplus
}
#endif
#endif /* COMMON_H */


