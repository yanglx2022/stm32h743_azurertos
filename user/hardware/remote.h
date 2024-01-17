/**
 * @brief   : 红外遥控
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-12-02
 */

#ifndef REMOTE_H
#define REMOTE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// 键码
#define REMOTE_CODE_POWER               (0x45)  // "POWER"
#define REMOTE_CODE_MENU                (0x47)  // "MENU"
#define REMOTE_CODE_TEST                (0x44)  // "TEST"
#define REMOTE_CODE_P                   (0x40)  // "+"
#define REMOTE_CODE_RET                 (0x43)  // "RETURN"
#define REMOTE_CODE_LEFT                (0x07)  // "|<<"
#define REMOTE_CODE_PLAY                (0x15)  // "▶"
#define REMOTE_CODE_RIGHT               (0x09)  // ">>|"
#define REMOTE_CODE_0                   (0x16)  // "0"
#define REMOTE_CODE_N                   (0x19)  // "-"
#define REMOTE_CODE_C                   (0x0D)  // "C"
#define REMOTE_CODE_1                   (0x0C)  // "1"
#define REMOTE_CODE_2                   (0x18)  // "2"
#define REMOTE_CODE_3                   (0x5E)  // "3"
#define REMOTE_CODE_4                   (0x08)  // "4"
#define REMOTE_CODE_5                   (0x1C)  // "5"
#define REMOTE_CODE_6                   (0x5A)  // "6"
#define REMOTE_CODE_7                   (0x42)  // "7"
#define REMOTE_CODE_8                   (0x52)  // "8"
#define REMOTE_CODE_9                   (0x4A)  // "9"

// 解码状态
typedef enum
{
    LEADER, // 判引导码
    DATA,   // 解析数据
    STOP    // 停止位
} remote_state_t;

// 脉冲宽度(us)
typedef struct
{
    uint32_t reference; // 参考值
    uint32_t lower;     // 下限值
    uint32_t upper;     // 上限值
} pulse_width_t;

// 按键状态
typedef struct
{
    uint8_t  code;      // 按键键值
    uint8_t  press;     // 按键按下状态
    uint8_t  down;      // 按键按下动作
    uint8_t  up;        // 按键抬起动作
    uint32_t hold_ms;   // 按下保持时间
} remote_key_t;

void Remote_Init(void);
remote_key_t Remote_Key_State(void);


#ifdef __cplusplus
}
#endif
#endif /* REMOTE_H */


