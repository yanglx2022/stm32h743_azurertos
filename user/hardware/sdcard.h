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

// 插拔检测 PB0
#define SD_DETECT_GPIO                  GPIOB
#define SD_DETECT_GPIO_EN               LL_AHB4_GRP1_PERIPH_GPIOB
#define SD_DETECT_PIN                   LL_GPIO_PIN_0


#ifdef __cplusplus
}
#endif
#endif /* SDCARD_H */


