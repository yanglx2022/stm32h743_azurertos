/**
 * @brief   : 基于STM32H743+AzureRTOS的综合示例项目
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-02
 */

#include "common.h"
#include "thread_idle.h"

// 版本号使用后三个字节
const uint32_t VERSION = 0x000001;    // 版本号V0.0.1

uint8_t axi_data[1024] __attribute__ ((section (".noinit.AXI_RAM")));
uint8_t sd_data[1024] __attribute__ ((section (".init.SDRAM"))) = {1};

int main(void)
{
    printf("hello world %d %d\n", axi_data[0], sd_data[0]);

    tx_kernel_enter();
    while(1);
}

void tx_application_define(void *first_unused_memory)
{
    TX_THREAD_NOT_USED(first_unused_memory);
    // 空闲任务
    idle_thread_create();
}


