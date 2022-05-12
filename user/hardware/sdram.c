/**
 * @brief   : SDRAM驱动(W9825G6KH 16位宽 32M字节)
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-06
 */

#include "sdram.h"

static void SDRAM_GPIO_Init(void);
static void SDRAM_FMC_Init(void);
static void SDRAM_Initialization_Sequence(void);

/**
 * @brief : SDRAM初始化
 * @param  
 * @return 
 */
void SDRAM_Init(void)
{
    SDRAM_GPIO_Init();
    SDRAM_FMC_Init();
    SDRAM_Initialization_Sequence();
}

/**
 * @brief : GPIO配置
 * @param  
 * @return 
 */
static void SDRAM_GPIO_Init(void)
{
    // 使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);
    /** FMC GPIO
    PF0  AF12 ------> FMC_A0        PD14 AF12 ------> FMC_D0        PC0  AF12 ------> FMC_SDNWE
    PF1  AF12 ------> FMC_A1        PD15 AF12 ------> FMC_D1        PG15 AF12 ------> FMC_SDNCAS 
    PF2  AF12 ------> FMC_A2        PD0  AF12 ------> FMC_D2        PF11 AF12 ------> FMC_SDNRAS
    PF3  AF12 ------> FMC_A3        PD1  AF12 ------> FMC_D3        PC2  AF12 ------> FMC_SDNE0  
    PF4  AF12 ------> FMC_A4        PE7  AF12 ------> FMC_D4        PG4  AF12 ------> FMC_BA0
    PF5  AF12 ------> FMC_A5        PE8  AF12 ------> FMC_D5        PG5  AF12 ------> FMC_BA1
    PF12 AF12 ------> FMC_A6        PE9  AF12 ------> FMC_D6        PC3  AF12 ------> FMC_SDCKE0 
    PF13 AF12 ------> FMC_A7        PE10 AF12 ------> FMC_D7        PG8  AF12 ------> FMC_SDCLK
    PF14 AF12 ------> FMC_A8        PE11 AF12 ------> FMC_D8        PE1  AF12 ------> FMC_NBL1
    PF15 AF12 ------> FMC_A9        PE12 AF12 ------> FMC_D9        PE0  AF12 ------> FMC_NBL0
    PG0  AF12 ------> FMC_A10       PE13 AF12 ------> FMC_D10
    PG1  AF12 ------> FMC_A11       PE14 AF12 ------> FMC_D11
    PG2  AF12 ------> FMC_A12       PE15 AF12 ------> FMC_D12
                                    PD8  AF12 ------> FMC_D13
                                    PD9  AF12 ------> FMC_D14
                                    PD10 AF12 ------> FMC_D15  */
    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_12;
    // PC0 PC2 PC3
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    // PD0 PD1 PD8 PD9 PD10 PD14 PD15
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_8 | 
                          LL_GPIO_PIN_9 | LL_GPIO_PIN_10 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    // PE0 PE1 PE7 PE8 PE9 PE10 PE11 PE12 PE13 PE14 PE15
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_7 | LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10 | 
                          LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    // PF0 PF1 PF2 PF3 PF4 PF5 PF11 PF12 PF13 PF14 PF15
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | 
                          LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    // PG0 PG1 PG2 PG4 PG5 PG8 PG15
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_8 | 
                          LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

/**
 * @brief : FMC配置
 * @param  
 * @return 
 */
static void SDRAM_FMC_Init(void)
{
    // 开启时钟
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_FMC);

    // 配置
    FMC_SDRAM_InitTypeDef init = {0};
    init.SDBank              = FMC_SDRAM_BANK1;                     // 选择BANK区
    init.ColumnBitsNumber    = FMC_SDRAM_COLUMN_BITS_NUM_9;         // 9位列地址
    init.RowBitsNumber       = FMC_SDRAM_ROW_BITS_NUM_13;           // 13位行地址
    init.MemoryDataWidth     = FMC_SDRAM_MEM_BUS_WIDTH_16;          // 16位数据宽度
    init.InternalBankNumber  = FMC_SDRAM_INTERN_BANKS_NUM_4;        // 4个内部BANK
    init.CASLatency          = FMC_SDRAM_CAS_LATENCY_2;             // CAS
    init.WriteProtection     = FMC_SDRAM_WRITE_PROTECTION_DISABLE;  // 禁止写保护
    init.SDClockPeriod       = FMC_SDRAM_CLOCK_PERIOD_2;            // 2分频
    init.ReadBurst           = FMC_SDRAM_RBURST_ENABLE;             // 突发模式
    init.ReadPipeDelay       = FMC_SDRAM_RPIPE_DELAY_1;             // 读延迟
    FMC_SDRAM_Init(FMC_SDRAM_DEVICE, &init);

    // 时间参数配置
    FMC_SDRAM_TimingTypeDef timing;
    timing.LoadToActiveDelay    = 2;    // 加载模式寄存器到激活的延迟:2个时钟周期
    timing.ExitSelfRefreshDelay = 8;    // 退出自刷新延迟:8个时钟周期
    timing.SelfRefreshTime      = 5;    // 自刷新时间:5个时钟周期
    timing.RowCycleDelay        = 7;    // 行循环延迟:7个时钟周期
    timing.WriteRecoveryTime    = 2;    // 恢复延迟:2个时钟周期
    timing.RPDelay              = 2;    // 行预充电延迟:2个时钟周期
    timing.RCDDelay             = 2;    // 行到列延迟:2个时钟周期
    FMC_SDRAM_Timing_Init(FMC_SDRAM_DEVICE, &timing, init.SDBank);

    // 使能FMC外设
    __FMC_ENABLE();
}

/**
 * @brief : 发送初始化序列
 * @param  
 * @return 
 */
static void SDRAM_Initialization_Sequence(void)
{
    FMC_SDRAM_CommandTypeDef command = {0};

    // 时钟配置命令
    command.CommandMode             = FMC_SDRAM_CMD_CLK_ENABLE;         // 开启SDRAM时钟
    command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK1;       // BANK1
    command.AutoRefreshNumber       = 1;
    command.ModeRegisterDefinition  = 0;
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &command, SDRAM_TIMEOUT);
    delay_ms(1);

    // 预充电命令
    command.CommandMode             = FMC_SDRAM_CMD_PALL;               // 预充电命令
    command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK1;       // BANK1
    command.AutoRefreshNumber       = 1;
    command.ModeRegisterDefinition  = 0;
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &command, SDRAM_TIMEOUT);
    
    // 自动刷新命令
    command.CommandMode             = FMC_SDRAM_CMD_AUTOREFRESH_MODE;   // 自动刷新命令
    command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK1;       // BANK1
    command.AutoRefreshNumber       = 8;                                // 自动刷新次数
    command.ModeRegisterDefinition  = 0;
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &command, SDRAM_TIMEOUT);

    // 模式寄存器命令
    command.CommandMode             = FMC_SDRAM_CMD_LOAD_MODE;          // 模式寄存器命令
    command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK1;       // BANK1
    command.AutoRefreshNumber       = 1;
    command.ModeRegisterDefinition  = (3 << 0) |        // 突发长度:8(bit0~2 BL=2^val)
                                      (0 << 3) |        // 地址模式:连续(bit3 0:连续 1:交错)
                                      (2 << 4) |        // 列地址选通延迟CAS:2个时钟周期(bit4~6 只支持2/3两种值)
                                      (1 << 9);         // 写模式:突发读单次写(bit9 0:突发读突发写 1:突发读单次写)
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &command, SDRAM_TIMEOUT);

    // 配置刷新率(SDRAM刷新周期对应的时钟周期数/行数-20)
    // SDRAM刷新周期64ms 行数2^13=8192 时钟AHB3_CLK/2=100MHz => 刷新率=64e-3*100e6/8192-20=761.25
    FMC_SDRAM_ProgramRefreshRate(FMC_SDRAM_DEVICE, 761);
}


