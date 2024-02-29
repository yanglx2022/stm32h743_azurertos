/**
 * @brief   : LTC2941库仑计
 * @author  : yanglx2022
 * @version : V0.1.0
 * @date    : 2024-02-28
 * @note    : 
 */

#include "ltc2941.h"
#include "i2c.h"

// PA4 AL#/CC
#define LTC2941_CC_GPIO                 GPIOA
#define LTC2941_CC_PIN                  LL_GPIO_PIN_4

// I2C地址
#define LTC2941_I2C_ADDR                0x64

// 寄存器
#define STATUS_REGISTER_A               0x00    // 状态寄存器
#define CONTROL_REGISTER_B              0x01    // 控制寄存器
#define MSB_REGISTER_C                  0x02    // 电荷量高字节
#define LSB_REGISTER_D                  0x03    // 电荷量低字节


/**
 * @brief  LTC2941初始化
 * @return 
 */
void LTC2941_Init(void)
{
    // GPIO使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = LTC2941_CC_PIN;
    LL_GPIO_Init(LTC2941_CC_GPIO, &GPIO_InitStruct);    // 开漏输出至CC(充电完成)
    LL_GPIO_ResetOutputPin(LTC2941_CC_GPIO, LTC2941_CC_PIN);

    // I2C初始化
    i2c_init();

    // 配置参数
    uint8_t value = 0xFA;   // 11(3V欠压报警)_111(M=128)_01(CC)_0(非低功耗)
    i2c_register_write(LTC2941_I2C_ADDR, CONTROL_REGISTER_B, &value, 1);
}

/**
 * @brief  充电完成
 * @return 
 */
void LTC2941_Charge_Complete(void)
{
    LL_GPIO_SetOutputPin(LTC2941_CC_GPIO, LTC2941_CC_PIN);
}

/**
 * @brief  读取电荷量与欠压状态
 * @param  *capacity      电荷量
 * @param  *under_voltage 欠压状态(1:<3V 0:>=3V)
 * @return 
 */
void LTC2941_Read(uint16_t *capacity, uint8_t *under_voltage)
{
    i2c_register_read(LTC2941_I2C_ADDR, MSB_REGISTER_C, capacity, 2);
    i2c_register_read(LTC2941_I2C_ADDR, STATUS_REGISTER_A, under_voltage, 1);
    *under_voltage = (*under_voltage >> 1) & 0x01;
}


