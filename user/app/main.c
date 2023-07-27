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

#include "nandflash.h"
int main(void)
{
    // printf("GUID: 0x%08X%08X%08X\n", CPU_ID2, CPU_ID1, CPU_ID0);
    printf("hello world\n");

    uint8_t data[2048 + 64] = {0};
    uint32_t page = 0;
    HRESULT hret = NAND_Read_Page(page, 0, data, 2112);
    printf("page %d hret = %d\n", page, (int)hret);
    for(int i = 0; i < 2112; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");

    // tx_kernel_enter();
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


