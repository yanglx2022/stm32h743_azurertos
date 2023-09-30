/**
 * @brief   : SD卡驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-19
 */

#ifndef SDCARD_H
#define SDCARD_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define SD_BLOCKSIZE                    BLOCKSIZE
#define SD_TIMEOUT_MS                   1000

#define SD_CardInfo                     HAL_SD_CardInfoTypeDef

// 插拔检测 PB0
#define SD_DETECT_GPIO                  GPIOB
#define SD_DETECT_GPIO_EN               LL_AHB4_GRP1_PERIPH_GPIOB
#define SD_DETECT_PIN                   LL_GPIO_PIN_0


void SDCard_Init(void);
HRESULT SDCard_Detect(void);
HRESULT SDCard_GetCardInfo(SD_CardInfo* CardInfo);
HRESULT SDCard_Read(uint8_t* data, uint32_t block_idx, uint32_t block_num);
HRESULT SDCard_Write(uint8_t* data, uint32_t block_idx, uint32_t block_num);


#ifdef __cplusplus
}
#endif
#endif /* SDCARD_H */


