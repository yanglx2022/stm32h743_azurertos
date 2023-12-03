/**
 * @brief   : 按键
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-11-26
 */

#ifndef KEY_H
#define KEY_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define KEY_POWER                       (0x01)  // 电源键
#define KEY_VOL_P                       (0x02)  // 音量+键
#define KEY_VOL_N                       (0x04)  // 音量-键
#define KEY_WAKE_UP                     (0x08)  // 唤醒键
#define KEY_TOUCH                       (0x10)  // 触摸键
#define KEY_NUM                         (5)     // 按键个数

// 按键状态
typedef struct
{
    uint8_t  press;     // 按键按下状态
    uint8_t  down;      // 按键按下动作
    uint8_t  up;        // 按键抬起动作
    uint8_t  count;     // 循环计数
    uint32_t hold_ms;   // 按下保持时间
} key_state_t;

void Key_Init(void);
key_state_t Key_Get_State(uint32_t key);


#ifdef __cplusplus
}
#endif
#endif /* KEY_H */


