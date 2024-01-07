/**
 * @brief   : FUSB302驱动平台相关接口
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-12-18
 */

#include "platform.h"
#include "i2c.h"
#include "fusb302.h"

// PG13 BQ25606 OTG使能
#define BQ25606_OTG_GPIO                GPIOG
#define BQ25606_OTG_PIN                 LL_GPIO_PIN_13


/**
 * @brief  I2C写寄存器
 * @return 
 */
FSC_BOOL platform_i2c_write(FSC_U8 SlaveAddress,
                            FSC_U8 RegAddrLength,
                            FSC_U8 DataLength,
                            FSC_U8 PacketSize,
                            FSC_U8 IncSize,
                            FSC_U32 RegisterAddress,
                            FSC_U8* Data)
{
    i2c_register_write(SlaveAddress >> 1, RegisterAddress, Data, DataLength);

    return TRUE;
}

/**
 * @brief  I2C读寄存器
 * @return 
 */
FSC_BOOL platform_i2c_read(FSC_U8 SlaveAddress,
                           FSC_U8 RegAddrLength,
                           FSC_U8 DataLength,
                           FSC_U8 PacketSize,
                           FSC_U8 IncSize,
                           FSC_U32 RegisterAddress,
                           FSC_U8* Data)
{
    i2c_register_read(SlaveAddress >> 1, RegisterAddress, Data, DataLength);

    return TRUE;
}

/**
 * @brief  设置VBUS输出
 * @return 
 */
void platform_set_vbus_lvl_enable(FSC_U8 port, 
                                  VBUS_LVL level, 
                                  FSC_BOOL enable,
                                  FSC_BOOL disableOthers)
{
    static uint8_t otg_initialized = 0;
    if (otg_initialized == 0)
    {
        otg_initialized = 1;

        // GPIO使能时钟
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);

        // GPIO配置
        LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
        GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FAST;
        GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
        GPIO_InitStruct.Pin         = BQ25606_OTG_PIN;
        LL_GPIO_Init(BQ25606_OTG_GPIO, &GPIO_InitStruct);
        LL_GPIO_ResetOutputPin(BQ25606_OTG_GPIO, BQ25606_OTG_PIN);
    }

    // 由于只支持bq25606的OTG5V输出, 此处不再区分level
    if (enable)
    {
        LL_GPIO_SetOutputPin(BQ25606_OTG_GPIO, BQ25606_OTG_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(BQ25606_OTG_GPIO, BQ25606_OTG_PIN);
    }
}

/**
 * @brief  获取中断状态
 * @param  port
 * @return 
 */
FSC_BOOL platform_get_device_irq_state(FSC_U8 port)
{
#ifdef FSC_POLLING_MODE
    return TRUE;
#else
    return LL_GPIO_IsInputPinSet(FUSB302_INT_GPIO, FUSB302_INT_PIN) ? 0 : 1;
#endif /* FSC_POLLING MODE */
}

/**
 * @brief  获取定时器TICK
 * @return 
 */
FSC_U32 platform_get_system_time(void)
{
    return LL_TIM_GetCounter(FUSB302_TIM2);
}

/**
 * @brief  延时
 * @param  microseconds 延时时间(单位us)
 * @return 
 */
void platform_delay(FSC_U32 microseconds)
{
    FSC_U32 currentTime = LL_TIM_GetCounter(FUSB302_TIM2);

    /* Use the free running clock */
    while ((FSC_U32)((FSC_U32)LL_TIM_GetCounter(FUSB302_TIM2) - currentTime) < microseconds);
}

/**
 * @brief  延时
 * @param  tenMicroSecondCounter 延时时间(单位10us)
 * @return 
 */
void platform_delay_10us(FSC_U32 tenMicroSecondCounter)
{
    platform_delay(tenMicroSecondCounter * 10);
}

void platform_set_vbus_discharge(FSC_U8 port, FSC_BOOL enable)
{
}

void platform_set_pps_voltage(FSC_U8 port, FSC_U32 mv)
{
}

FSC_U16 platform_get_pps_voltage(FSC_U8 port)
{
    return 0;
}

void platform_set_pps_current(FSC_U8 port, FSC_U32 ma)
{
}

FSC_U16 platform_get_pps_current(FSC_U8 port)
{
    return 0;
}


