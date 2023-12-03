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


