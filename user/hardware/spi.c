/**
 * @brief   : SPI接口
 * @author  : yanglx2022
 * @version : V0.1.0
 * @date    : 2024-01-08
 * @note    : 
 */

#include "spi.h"

// SPI3引脚
#define SPI3_GPIO                       GPIOC
#define SPI3_SCK_PIN                    LL_GPIO_PIN_10
#define SPI3_MISO_PIN                   LL_GPIO_PIN_11
#define SPI3_MOSI_PIN                   LL_GPIO_PIN_12

// 片选信号
#define SPI_CSN_LCD_GPIO                GPIOA
#define SPI_CSN_LCD_PIN                 LL_GPIO_PIN_5


/**
 * @brief  SPI初始化
 * @return 
 */
void spi_init(void)
{
    // GPIO使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FAST;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_6;
    GPIO_InitStruct.Pin         = SPI3_SCK_PIN | SPI3_MISO_PIN | SPI3_MOSI_PIN;
    LL_GPIO_Init(SPI3_GPIO, &GPIO_InitStruct);
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FAST;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = SPI_CSN_LCD_PIN;
    LL_GPIO_Init(SPI_CSN_LCD_GPIO, &GPIO_InitStruct);

    // SPI3配置
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI3);
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_SPI3);
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode              = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity     = LL_SPI_POLARITY_LOW;
    SPI_InitStruct.ClockPhase        = LL_SPI_PHASE_1EDGE;
    SPI_InitStruct.NSS               = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV32;
    SPI_InitStruct.BitOrder          = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
    LL_SPI_Init(SPI3, &SPI_InitStruct);
    LL_SPI_EnableGPIOControl(SPI3);
}

/**
 * @brief  收发一个字节
 * @param  data 发送字节数据
 * @return 接收字节数据
 */
uint8_t spi_byte_write_read(uint8_t data)
{
    LL_GPIO_ResetOutputPin(SPI_CSN_LCD_GPIO, SPI_CSN_LCD_PIN);
    LL_SPI_Enable(SPI3);                    // 使能SPI3
    LL_SPI_StartMasterTransfer(SPI3);       // 启动传输
    while(!LL_SPI_IsActiveFlag_TXP(SPI3));  // 等待发送区空
    LL_SPI_TransmitData8(SPI3, data);       // 发送一个字节
    while(!LL_SPI_IsActiveFlag_RXP(SPI3));  // 等待接收完成
    data = LL_SPI_ReceiveData8(SPI3);       // 接收一个字节
    LL_SPI_ClearFlag_EOT(SPI3);             // 清EOTC
    LL_SPI_ClearFlag_TXTF(SPI3);            // 清TXTFC
    LL_SPI_Disable(SPI3);                   // 关闭SPI3,会执行状态机复位/FIFO重置等操作
    LL_GPIO_SetOutputPin(SPI_CSN_LCD_GPIO, SPI_CSN_LCD_PIN);
    return data;
}


