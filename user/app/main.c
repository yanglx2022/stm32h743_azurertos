/**
 * @brief   : 基于STM32H743+AzureRTOS的综合示例项目
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-02
 */

#include "common.h"
#include "thread_idle.h"
#include "thread_file.h"

// 版本号使用后三个字节
const uint32_t VERSION = 0x000001;    // 版本号V0.0.1


int main(void)
{
    printf("GUID: 0x%08X%08X%08X\n", CPU_ID2, CPU_ID1, CPU_ID0);

    tx_kernel_enter();
    while(1);
}

void tx_application_define(void *first_unused_memory)
{
    TX_THREAD_NOT_USED(first_unused_memory);
    // 创建空闲任务
    idle_thread_create();
    // 创建文件任务
    file_thread_create();
}


