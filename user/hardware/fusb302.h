/**
 * @brief   : FUSB302驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-12-17
 */

#ifndef FUSB302_H
#define FUSB302_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// PA2 FUSB302中断脚
#define FUSB302_INT_GPIO                GPIOA
#define FUSB302_INT_PIN                 LL_GPIO_PIN_2

void FUSB302_Init(void);
void FUSB302_SrcCaps_Select(int index);


#ifdef __cplusplus
}
#endif
#endif /* FUSB302_H */


