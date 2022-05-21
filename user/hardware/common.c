/**
 * @brief   : MPU 时钟 延时 printf串口打印等
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-02
 */

#include "common.h"
#include "sdram.h"
#include "tx_thread.h"
#include "tx_initialize.h"
#include "sdcard.h"

static void MPU_Cache_Config(void);
static void SystemClock_Config(void);
static void Delay_Init(void);
static void USART1_Init(void);
static void LED_Init(void);
static void memory_init(void);


/**
 * @brief : 初始化(被__libc_init_array调用, 启动文件中先调用__libc_init_array再调用main)
 * @param  
 * @return 
 */
__attribute__((constructor)) void sys_init(void)
{
    // 中断分组
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); // 全抢占
    // MPU与Cache
    MPU_Cache_Config();
    // 时钟
    SystemClock_Config();
    SystemCoreClockUpdate();
    // 延时
    Delay_Init();
    // 串口
    USART1_Init();
    // LED
    LED_Init();
    // SDRAM初始化
    SDRAM_Init();
    // 自定义段变量初始化
    memory_init();
    // SD卡初始化
    SDCard_Init();
}

/**
 * @brief : 自定义段变量初始化
 * @param  
 * @return 
 */
static void memory_init(void)
{
    // AXI_RAM数据初始化
    // 从FLASH拷贝
    extern uint32_t _init_axi_ram_start;
    extern uint32_t _init_axi_ram_end;
    extern uint32_t _init_axi_ram_lma;
    uint32_t *ram   = &_init_axi_ram_start; // 注意_init_axi_ram_start只是ld文件中的符号, 不是普通变量, 要用&访问其值, 下同
    uint32_t *end   = &_init_axi_ram_end;
    uint32_t *flash = &_init_axi_ram_lma;
    while(ram < end)
    {
        *ram++ = *flash++;
    }
    // 赋0值
    extern uint32_t _noinit_axi_ram_start;
    extern uint32_t _noinit_axi_ram_end;
    ram = &_noinit_axi_ram_start;
    end = &_noinit_axi_ram_end;
    while(ram < end)
    {
        *ram++ = 0;
    }

    // SDRAM数据初始化
    // 从FLASH拷贝
    extern uint32_t _init_sdram_start;
    extern uint32_t _init_sdram_end;
    extern uint32_t _init_sdram_lma;
    ram   = &_init_sdram_start;
    end   = &_init_sdram_end;
    flash = &_init_sdram_lma;
    while(ram < end)
    {
        *ram++ = *flash++;
    }
    // 赋0值
    extern uint32_t _noinit_sdram_start;
    extern uint32_t _noinit_sdram_end;
    ram = &_noinit_sdram_start;
    end = &_noinit_sdram_end;
    while(ram < end)
    {
        *ram++ = 0;
    }
}

/**
 * @brief : 配置MPU与Cache
 * @param  
 * @return 
 */
static void MPU_Cache_Config(void)
{
    // MPU配置说明(只要启用cache即开启read-allocate)
    // TEX=000 C=0 B=0 ----- 内存类型Strongly ordered
    // TEX=000 C=0 B=1 ----- 内存类型Device
    // TEX=000 C=1 B=0 ----- 内存类型Normal, write-through, no write-allocate
    // TEX=000 C=1 B=1 ----- 内存类型Normal, write-back, no write-allocate
    // TEX=001 C=0 B=0 ----- 内存类型Normal, 禁用cache
    // TEX=001 C=1 B=1 ----- 内存类型Normal, write-back, write-allocate

    // MPU配置前禁能
    LL_MPU_Disable();
    // AXI SRAM最高性能(Normal, Write back, Write allocate)
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER0, 
                        0x00,                               // 子区域全使能
                        AXI_SRAM_BASE,                      // AXI SRAM地址
                        LL_MPU_INSTRUCTION_ACCESS_ENABLE |  // 允许执行代码
                        LL_MPU_REGION_FULL_ACCESS        |  // 完全访问权限(特权与非特权模式均读写)
                        LL_MPU_TEX_LEVEL1                |  // TEX=001
                        LL_MPU_ACCESS_NOT_SHAREABLE      |  // S=0关闭共享
                        LL_MPU_ACCESS_CACHEABLE          |  // C=1使能Cache
                        LL_MPU_ACCESS_BUFFERABLE         |  // B=1禁止缓冲
                        LL_MPU_REGION_SIZE_512KB);          // 512KB
    // SDRAM(Normal, Write through)
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER1, 
                        0x00,                               // 子区域全使能
                        SDRAM_BASE,                         // SDRAM地址
                        LL_MPU_INSTRUCTION_ACCESS_ENABLE |  // 允许执行代码
                        LL_MPU_REGION_FULL_ACCESS        |  // 完全访问权限(特权与非特权模式均读写)
                        LL_MPU_TEX_LEVEL0                |  // TEX=000
                        LL_MPU_ACCESS_NOT_SHAREABLE      |  // S=0关闭共享
                        LL_MPU_ACCESS_CACHEABLE          |  // C=1使能Cache
                        LL_MPU_ACCESS_NOT_BUFFERABLE     |  // B=0禁止缓冲
                        LL_MPU_REGION_SIZE_32MB);           // 32MB
    // NAND Flash内存类型Device
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER2, 
                        0x00,                               // 子区域全使能
                        NAND_DEVICE,                        // SDRAM地址
                        LL_MPU_INSTRUCTION_ACCESS_ENABLE |  // 允许执行代码
                        LL_MPU_REGION_FULL_ACCESS        |  // 完全访问权限(特权与非特权模式均读写)
                        LL_MPU_TEX_LEVEL0                |  // TEX=000
                        LL_MPU_ACCESS_NOT_SHAREABLE      |  // S=0关闭共享
                        LL_MPU_ACCESS_NOT_CACHEABLE      |  // C=0
                        LL_MPU_ACCESS_BUFFERABLE         |  // B=1
                        LL_MPU_REGION_SIZE_512MB);          // 512MB
    // 使能MPU
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);	// 使能背景区, NMI与HardFault中断中关闭MPU
    // 开启Cache
    SCB_EnableICache();
    SCB_EnableDCache();
}

/**
 * @brief : 配置时钟
 * @param  
 * @return 
 */
static void SystemClock_Config(void)
{
    // 晶振为25MHz
    // 注意主频改变时要同步修改延时相关参数,SDRAM刷新率以及tx_initialize_low_level.S中SYSTEM_CLOCK

    // 设置FLASH读延时(2个等待状态)
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2);
    // 使能稳压器
    LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
    // 电压调节1(最佳FLASH访问性能)
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    while(LL_PWR_GetRegulVoltageScaling() != LL_PWR_REGU_VOLTAGE_SCALE1);

    // 使能HSE
    LL_RCC_HSE_Enable();
    while(LL_RCC_HSE_IsReady() == 0);

    // 使能HSI48(USB使用)
    LL_RCC_HSI48_Enable();
    while(LL_RCC_HSI48_IsReady() == 0);

    // 设置PLL时钟源为HSE
    LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);

    // 设置PLL1输入频率范围4MHz~8MHz
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
    // 设置PLL1输出频率范围WIDE(192~836MHz)
    LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    // 设置PLL1预分频系数M(取值1~63) 倍频系数N(取值4~512) 注意:N倍频后频率要在PLL1输出频率范围内
    LL_RCC_PLL1_SetM(5);
    LL_RCC_PLL1_SetN(160);      // 25MHz/5*160=800MHz
    // 设置PLL1P PLL1Q PLL1R分频系数(取值2~128)
    LL_RCC_PLL1_SetP(2);        // 800MHz/2=400MHz
    LL_RCC_PLL1_SetQ(4);        // 800MHz/4=200MHz
    LL_RCC_PLL1_SetR(2);        // 800MHz/2=400MHz
    // 使能PLL1
    LL_RCC_PLL1P_Enable();
    LL_RCC_PLL1_Enable();
    while(LL_RCC_PLL1_IsReady() == 0);

    // 设置PLL2输入频率范围1MHz~2MHz
    LL_RCC_PLL2_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_1_2);
    // 设置PLL2输出频率范围WIDE(192~836MHz)
    LL_RCC_PLL2_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    // 设置PLL2预分频系数M(取值1~63) 倍频系数N(取值4~512) 注意:N倍频后频率要在PLL2输出频率范围内
    LL_RCC_PLL2_SetM(25);
    LL_RCC_PLL2_SetN(504);      // 25MHz/25*504=504MHz
    // 设置PLL1P PLL1Q PLL1R分频系数(取值2~128)
    LL_RCC_PLL2_SetP(7);        // 504MHz/7=72MHz
    LL_RCC_PLL2_SetQ(2);        // 504MHz/2=252MHz
    LL_RCC_PLL2_SetR(21);       // 504MHz/21=24MHz
    LL_RCC_PLL2P_Enable();
    // 使能PLL2
    LL_RCC_PLL2P_Enable();
    LL_RCC_PLL2R_Enable();
    LL_RCC_PLL2_Enable();
    while(LL_RCC_PLL2_IsReady() == 0);

    // 设置PLL3输入频率范围8MHz~16MHz
    LL_RCC_PLL3_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    // 设置PLL3输出频率范围WIDE(192~836MHz)
    LL_RCC_PLL3_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    // 设置PLL3预分频系数M(取值1~63) 倍频系数N(取值4~512) 注意:N倍频后频率要在PLL3输出频率范围内
    LL_RCC_PLL3_SetM(32);
    LL_RCC_PLL3_SetN(128);      // 25MHz/32*128=100MHz
    // 设置PLL1P PLL1Q PLL1R分频系数(取值2~128)
    LL_RCC_PLL3_SetP(2);        // 100MHz/2=50MHz
    LL_RCC_PLL3_SetQ(2);        // 100MHz/2=50MHz
    LL_RCC_PLL3_SetR(3);        // LTDC 2--50MHz时屏幕闪烁严重(刷新时闪现一些像素粒)  3--33.3MHz无像素粒
    // 使能PLL3
    LL_RCC_PLL3R_Enable();      // LTDC时钟固定为PLL3R
    LL_RCC_PLL3_Enable();
    while(LL_RCC_PLL3_IsReady() == 0);

    // AHB预分频设为2
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
    // 选择PLL1作为系统时钟源 则SYSCLK=PLL1P=400MHz
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    // 系统预分频为1 则CPU主频=SYSCLK=400MHz
    LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_SetSystemCoreClock(400000000);

    // 设置AHB APB预分频
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);       // AXI AHB HCLK = CPU主频/2=200MHz
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);     // APB = AHB/2=100MHz APB_TIM = AHB/2*2=200MHz
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
    LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);

    // 设置外设时钟源
    LL_RCC_SetUSARTClockSource(LL_RCC_USART16_CLKSOURCE_PCLK2);
    LL_RCC_SetFMCClockSource(LL_RCC_FMC_CLKSOURCE_HCLK);
    LL_RCC_SetSDMMCClockSource(LL_RCC_SDMMC_CLKSOURCE_PLL1Q);
    // LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_HSI48);
    // LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    // LL_RCC_SetQSPIClockSource(LL_RCC_QSPI_CLKSOURCE_HCLK);
    // LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_PLL2P);
}

/**
 * @brief : 延时初始化
 * @param  
 * @return 
 */
static void Delay_Init(void)
{
    // DWT ns级延时
    CoreDebug->DEMCR |= 1 << 24;    // 使能DWT
    DWT->CYCCNT       = 0;          // CYCCNT计数清0
    DWT->CTRL        |= 1;          // 使能CYCCNT
}

/**
 * @brief : us延时(阻塞)
 * @param  num us数
 * @return 
 */
void delay_us(uint32_t num)
{
    uint32_t cnt = num / 4000000;
    uint32_t remain = num - cnt * 4000000;
    while(cnt--)
    {
        delay_ns(4000000000);
    }
    if (remain)
    {
        delay_ns(remain * 1000);
    }
}

/**
 * @brief : ms延时(RTOS尚未运行时基于DWT)
 * @param  num ms数
 * @return 
 */
void delay_ms(uint32_t num)
{
    if (_tx_thread_system_state == TX_INITIALIZE_IS_FINISHED)
    {
        tx_thread_sleep(MS_TO_TICKS(num));
    }
    else
    {
        while(num--)
        {
            delay_us(1000);
        }
    }
}

/**
 * @brief : 获取tick_ms值(RTOS尚未运行时tick基于DWT,注意此时循环只有10.7s(400MHz时))
 * @param  
 * @return tick_ms
 */
uint32_t get_tick_ms(void)
{
    if (_tx_thread_system_state == TX_INITIALIZE_IS_FINISHED)
    {
        return (1000 / TX_TIMER_TICKS_PER_SECOND * tx_time_get());
    }
    else
    {
        // RTOS尚未运行的话利用DWT的tick,注意此时一个循环只有10.7s(400MHz时)
        return (cpu_tick_to_ns(get_cpu_tick() / 1000000));
    }
}

/**
 * @brief : 重写ms延时(基于DWT)(阻塞)
 * @param  Delay 延时ms数
 * @return 
 */
void HAL_Delay(uint32_t Delay)
{
    while(Delay--)
    {
        delay_us(1000);
    }
}

/**
 * @brief : 重写获取ms tick值
 * @param  
 * @return tick_ms
 */
uint32_t HAL_GetTick(void)
{
  return (cpu_tick_to_ns(get_cpu_tick() / 1000000));
}

/**
 * @brief : 串口1初始化
 * @param  
 * @return 
 */
static void USART1_Init(void)
{
    // 开启时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    // PA9 ------> USART1_TX
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // USART1
    LL_USART_InitTypeDef USART_InitStruct = {0};
    USART_InitStruct.PrescalerValue         = LL_USART_PRESCALER_DIV1;
    USART_InitStruct.BaudRate               = 115200;
    USART_InitStruct.DataWidth              = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits               = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity                 = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection      = LL_USART_DIRECTION_TX;
    USART_InitStruct.HardwareFlowControl    = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling           = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1, &USART_InitStruct);
    LL_USART_DisableFIFO(USART1);
    LL_USART_ConfigAsyncMode(USART1);
    LL_USART_Enable(USART1);
    while(LL_USART_IsActiveFlag_TEACK(USART1) == 0);
}

/**
 * @brief : 重定义_write函数(支持printf)
 * @param  fd
 * @param  *ptr
 * @param  len
 * @return 
 */
int _write(int fd, char *ptr, int len)  
{
    for(int i = 0; i < len; i++)
    {
        LL_USART_TransmitData8(USART1, ptr[i]);
        while(LL_USART_IsActiveFlag_TC(USART1) == 0);
    }
    return len;
}

/**
 * @brief : LED初始化(注意VREF+为高电平时此LED无效)
 * @param  
 * @return 
 */
static void LED_Init(void)
{
    // 开启时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);

    // PB1 LED 低电平亮
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_1);     // 默认高电平LED不亮
}

/**
 * @brief : 错误指示(LED闪烁 串口打印错误信息)
 * @param  text
 * @return 
 */
static void Error_Handler(const char* text)
{
    // 打印错误信息
    const char* tip = "[Unhandled Error] ";
    for(int i = 0; i < strlen(tip); i++)
    {
        LL_USART_TransmitData8(USART1, tip[i]);
        while(LL_USART_IsActiveFlag_TC(USART1) == 0);
    }
    for(int i = 0; i < strlen(text); i++)
    {
        LL_USART_TransmitData8(USART1, text[i]);
        while(LL_USART_IsActiveFlag_TC(USART1) == 0);
    }
    // LED闪烁
    while(1)
    {
        LED_TOGGLE();
        delay_us(250000);
    }
}

/* Cortex Processor Interruption and Exception Handlers */
// NMI
void NMI_Handler(void)
{
    Error_Handler("NMI: Non maskable interrupt\n");
}

// HardFault
void HardFault_Handler(void)
{
    Error_Handler("HardFault: Hard fault interrupt\n");
}

// MemManage
void MemManage_Handler(void)
{
    Error_Handler("MemManage: Memory management fault\n");
}

// BusFault
void BusFault_Handler(void)
{
    Error_Handler("BusFault: Pre-fetch fault, memory access fault\n");
}

// UsageFault
void UsageFault_Handler(void)
{
    Error_Handler("UsageFault: Undefined instruction or illegal state\n");
}
/* */


