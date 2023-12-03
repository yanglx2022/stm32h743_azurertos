/**
 * @brief   : 基于STM32H743+AzureRTOS的综合示例项目
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-02
 */

#include "common.h"
#include "thread_idle.h"
#include "thread_file.h"
#include "lx_nand_driver.h"
#include "ux_device.h"
#include "buzzer.h"

// 版本号使用后三个字节
const uint32_t VERSION = 0x000001;    // 版本号V0.0.1


int main(void)
{
    printf("GUID: 0x%08X%08X%08X\n", CPU_ID2, CPU_ID1, CPU_ID0);
    // Buzzer_Beep(100);
    // delay_ms(101);  // threadx初始化过程中会关总中断，因此此处无延时则会等threadx启动完成后才响
    
    // 进入系统
    tx_kernel_enter();
    while(1);
}

// 启动任务
void tx_application_define(void *first_unused_memory)
{
    TX_THREAD_NOT_USED(first_unused_memory);

    // 创建空闲任务
    idle_thread_create();

    // LevelX初始化
    lx_nand_flash_initialize();
    lx_nand_flash_open(&nand_flash, "nand flash", nand_driver_initialize);

    // 创建文件任务
    // file_thread_create();

    // PA7 ------> USB/音频切换
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);     // 高电平选择USB

    // USB
    usb_device_init();
}


