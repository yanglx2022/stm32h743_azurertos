/**
 * @brief   : 空闲任务
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-02
 */

#include "thread_idle.h"
#include "tx_execution_profile.h"
#include "switch.h"
#include "key.h"
#include "remote.h"
#include "fusb302.h"
#include "lcd.h"

// 任务堆栈
static UCHAR thread_stack[STACK_SIZE_DEFAULT];
// 任务句柄
static TX_THREAD idle_thread;


/**
 * @brief : CPU利用率
 * @param  
 * @return 
 */
static int cpu_usage(void)
{
    int usage = -1;
    EXECUTION_TIME idle_time, thread_time, isr_time;
    if (_tx_execution_idle_time_get(&idle_time) == TX_SUCCESS &&
        _tx_execution_thread_total_time_get(&thread_time) == TX_SUCCESS &&
        _tx_execution_isr_time_get(&isr_time) == TX_SUCCESS)
    {
        if ((idle_time + thread_time + isr_time) > 0)
        {
            usage = 100 * (thread_time + isr_time) / (idle_time + thread_time + isr_time);
        }
    }
    _tx_execution_idle_time_reset();
    _tx_execution_thread_total_time_reset();
    _tx_execution_isr_time_reset();
    return usage;
}

/**
 * @brief : 空闲任务
 * @param  thread_input
 * @return 
 */
void idle_thread_entry(ULONG thread_input)
{
    LCD_Print("idle_thread_entry\n");
    Key_Get_State(KEY_TOUCH);   // 首次触摸判断不准
    uint32_t cnt = 0;
    while(1)
    {
        cnt++;
        int usage = cpu_usage();
        if (usage >= 0)
        {
            // printf("CPU usage: %02d%%\n", usage);
        }
        // LED_TOGGLE();

        // 触摸按键测试
        key_state_t key = Key_Get_State(KEY_TOUCH);
        if (key.up)
        {
            static int touch_cnt = 0;
            LCD_Print("touch key %d\n", touch_cnt++);
        }
        if (key.press)
        {
            LED_ON();
        }
        else
        {
            LED_OFF();
        }

        // 滚动显示测试(音量键控制滚动)
        key = Key_Get_State(KEY_VOL_N);
        if (key.up)
        {
            LCD_Scroll(1);
        }
        else if (key.press && key.hold_ms > 300 && (cnt % 10) == 0)
        {
            LCD_Scroll(4);
        }
        else
        {
            key = Key_Get_State(KEY_VOL_P);
            if (key.up)
            {
                LCD_Scroll(-1);
            }
            else if (key.press && key.hold_ms > 300 && (cnt % 10) == 0)
            {
                LCD_Scroll(-4);
            }
        }

        // 关机检测
        Switch_Off();

        // 红外遥控("0"选择PD5V "1"选择PD9V "PLAY"切换显示方向 "-"下滚一行 "+"上滚一行)
        remote_key_t remote_key = Remote_Key_State();
        if (remote_key.up)
        {
            printf("0x%02X up\n", remote_key.code);
            if (remote_key.code == REMOTE_CODE_0)
            {
                FUSB302_SrcCaps_Select(0);
            }
            else if (remote_key.code == REMOTE_CODE_1)
            {
                FUSB302_SrcCaps_Select(1);
            }
            else if (remote_key.code == REMOTE_CODE_PLAY)
            {
                static uint32_t direction = 0;
                direction = (direction + 1) % 4;
                LCD_Direction_Set(direction);
            }
            else if (remote_key.code == REMOTE_CODE_N)
            {
                LCD_Scroll(1);
            }
            else if (remote_key.code == REMOTE_CODE_P)
            {
                LCD_Scroll(-1);
            }
            else
            {
                LCD_Print("ir key code 0x%02X\n", remote_key.code);
            }
        }
        else if (remote_key.press && remote_key.hold_ms > 300 && (cnt % 10) == 0)
        {
            if (remote_key.code == REMOTE_CODE_N)
            {
                LCD_Scroll(4);
            }
            else if (remote_key.code == REMOTE_CODE_P)
            {
                LCD_Scroll(-4);
            }
        }
        tx_thread_sleep(MS_TO_TICKS(10));
    }
}

/**
 * @brief : 创建空闲任务
 * @param  
 * @return 
 */
void idle_thread_create(void)
{
    tx_thread_create(&idle_thread, "idle_thread", idle_thread_entry, 0,
                     thread_stack, STACK_SIZE_DEFAULT, IDLE_THREAD_PRI, IDLE_THREAD_PRI, TX_NO_TIME_SLICE, TX_AUTO_START);
}


