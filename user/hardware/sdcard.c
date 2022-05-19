/**
 * @brief   : SD卡驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-19
 */

#include "sdcard.h"

void SDCard_Init(void)
{
    // 使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    LL_AHB4_GRP1_EnableClock(SD_DETECT_GPIO_EN);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_SDMMC2);

    /** SDMMC2 GPIO
    PB14 AF9  ------> SDMMC2_D0
    PB15 AF9  ------> SDMMC2_D1
    PB3  AF9  ------> SDMMC2_D2
    PB4  AF9  ------> SDMMC2_D3
    PC1  AF9  ------> SDMMC2_CK
    PD7  AF11 ------> SDMMC2_CMD */
    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FAST;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_9;
    // PB3 PB4 PB14 PB15
    GPIO_InitStruct.Pin = LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    // PC1
    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    // PD7
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_11;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    // 插拔检测 上拉输入
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Pin         = SD_DETECT_PIN;
    LL_GPIO_Init(SD_DETECT_GPIO, &GPIO_InitStruct);
}


