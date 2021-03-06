/**
 * @brief   : 空闲任务
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-02
 */

#include "thread_idle.h"
#include "tx_execution_profile.h"

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
 * @brief : 
 * @param  thread_input
 * @return 
 */
void idle_thread_entry(ULONG thread_input)
{
    while(1)
    {
        int usage = cpu_usage();
        if (usage >= 0)
        {
            printf("CPU usage: %02d%%\n", usage);
        }
        LED_TOGGLE();
        tx_thread_sleep(MS_TO_TICKS(1000));
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
                     thread_stack, STACK_SIZE_DEFAULT, IDLE_TASK_PRI, IDLE_TASK_PRI, TX_NO_TIME_SLICE, TX_AUTO_START);
}

