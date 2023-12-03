/**
 * @brief   : 蜂鸣器
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-11-25
 */

#ifndef BUZZER_H
#define BUZZER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"


void Buzzer_Init(void);
void Buzzer_Beep(uint16_t t_ms);


#ifdef __cplusplus
}
#endif
#endif /* BUZZER_H */


