/**
 * @brief   : NandFlash驱动(H27U4G8)(HAL)
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-07-15
 */

#include "nandflash.h"

// NAND Handler
static NAND_HandleTypeDef hNAND;

static uint32_t nand_wait_ready(uint32_t timeout_ns);
static HRESULT nand_reset(void);
static HRESULT nand_256byte_ecc_check(uint8_t* data, uint32_t ecc_value_calc, uint32_t ecc_value_read);


/**
 * @brief : NAND初始化
 * @param  
 * @return 
 */
HRESULT NAND_Init(void)
{
    // H27U4G8,8位数据宽度,2*2048*64*(2048+64)=512+16MB fmc_ker_ck=200MHz(5ns)
    // 设置NAND参数
    hNAND.Instance                  = FMC_NAND_DEVICE;
    hNAND.Init.NandBank             = FMC_NAND_BANK3;
    hNAND.Init.Waitfeature          = FMC_NAND_PCC_WAIT_FEATURE_DISABLE;    // 关闭等待特性
    hNAND.Init.MemoryDataWidth      = FMC_NAND_PCC_MEM_BUS_WIDTH_8;         // 8位数据宽度
    hNAND.Init.EccComputation       = FMC_NAND_ECC_DISABLE;                 // 初始ECC禁止
    hNAND.Init.ECCPageSize          = FMC_NAND_ECC_PAGE_SIZE_256BYTE;       // ECC为256字节
    hNAND.Init.TCLRSetupTime        = 1;                                    // tCLR=10ns(Min), tCLR=(TCLR+MEMSET+2)*T, 则TCLR=1 MEMSET=0时tCLR=15ns
    hNAND.Init.TARSetupTime         = 1;                                    // tAR=10ns(Min), tAR=(TAR+MEMSET+2)*T, 则TAR=1 MEMSET=0时tAR=15ns
    // 存储组织参数
    hNAND.Config.BlockNbr           = NAND_BLOCK_NUM;   // block总数
    hNAND.Config.BlockSize          = NAND_BLOCK_SIZE;  // 每个block的page数
    hNAND.Config.PageSize           = NAND_PAGE_SIZE;   // 每个page的数据区域大小
    hNAND.Config.SpareAreaSize      = NAND_SPARE_SIZE;  // 每个page的备用区域大小
    hNAND.Config.PlaneNbr           = NAND_PLANE_NUM;   // plane数
    hNAND.Config.PlaneSize          = NAND_PLANE_SIZE;  // 每个plane的block数
    hNAND.Config.ExtraCommandEnable = DISABLE;
    // 时序时间参数
    FMC_NAND_PCC_TimingTypeDef ComSpace_Timing;
    FMC_NAND_PCC_TimingTypeDef AttSpace_Timing;
    ComSpace_Timing.SetupTime       = 0;    // 建立时间=tCLS或tALS-tWP或tRP(Min均为12ns), 实测取20ns=4个fmc_ker_ck(设置值3+1)(未开启LTDC的话可取5ns)
    ComSpace_Timing.WaitSetupTime   = 4;    // 等待时间=tWP或tRP=12ns(Min), 理论可取3个fmc_ker_ck, 但实测需要至少5个CLK(设置值4+1)
    ComSpace_Timing.HoldSetupTime   = 2;    // 保持时间=tCLH或tALH=5ns(Min), 取10ns=2个(写)、15ns=3个(读)fmc_ker_ck(设置值2)
    ComSpace_Timing.HiZSetupTime    = 0;    // 高阻时间=tCLS或tALS-tDS(Min均为12ns), 实测取10ns=2个fmc_ker_ck(设置值2)(未开启LTDC的话可取5ns)
    AttSpace_Timing.SetupTime       = 0;    // 特性存储器空间未使用
    AttSpace_Timing.WaitSetupTime   = 0;
    AttSpace_Timing.HoldSetupTime   = 0;
    AttSpace_Timing.HiZSetupTime    = 0;
    // 初始化
    if (HAL_NAND_Init(&hNAND, &ComSpace_Timing, &AttSpace_Timing) != HAL_OK)
    {
        return HR_ERROR;
    }

    // 复位NAND
    if (nand_reset() != HR_OK)
    {
        return HR_ERROR;
    }

    return HR_OK;
}

/**
 * @brief : Msp初始化(GPIO)
 * @param  *hnand NAND Handler
 * @return 
 */
void HAL_NAND_MspInit(NAND_HandleTypeDef *hnand)
{
    // 使能时钟
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_FMC);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /** NAND FMC GPIO Configuration
    PD14  ------>  FMC_D0       PD11  ------>  FMC_A16_CLE
    PD15  ------>  FMC_D1       PD12  ------>  FMC_A17_ALE 
    PD0   ------>  FMC_D2       PD5   ------>  FMC_NWE
    PD1   ------>  FMC_D3       PD4   ------>  FMC_NOE  
    PE7   ------>  FMC_D4       PG9   ------>  FMC_NCE3
    PE8   ------>  FMC_D5       PD6   ------>  FMC_NWAIT
    PE9   ------>  FMC_D6
    PE10  ------>  FMC_D7  */
    // PD0 PD1 PD4 PD5 PD11 PD12 PD14 PD15
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_12;
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | 
                                  LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    // PE7 PE8 PE9 PE10
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_7 | LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    // PG9
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_9;
    LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    // PD6
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_6;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
 * @brief : 读Page(只有读整页时进行ECC校验)
 * @param  page   页索引
 * @param  column 页内字节索引
 * @param  buffer 数据缓冲区
 * @param  length 要读取数据的字节数
 * @return 
 */
HRESULT NAND_Read_Page(uint32_t page, uint32_t column, uint8_t* buffer, uint32_t length)
{
    NAND_HandleTypeDef *hnand = &hNAND;
    if (page >= ((hnand->Config.BlockSize) * (hnand->Config.BlockNbr)) || 
        (column + length) > (hnand->Config.PageSize + hnand->Config.SpareAreaSize))
    {
        return HR_INVALID_ADDRESS;
    }

    // 读命令
    *(__IO uint8_t *)(NAND_DEVICE | CMD_AREA)  = NAND_CMD_AREA_A;
    *(__IO uint8_t *)(NAND_DEVICE | ADDR_AREA) = COLUMN_1ST_CYCLE(column);
    *(__IO uint8_t *)(NAND_DEVICE | ADDR_AREA) = COLUMN_2ND_CYCLE(column);
    *(__IO uint8_t *)(NAND_DEVICE | ADDR_AREA) = ADDR_1ST_CYCLE(page);
    *(__IO uint8_t *)(NAND_DEVICE | ADDR_AREA) = ADDR_2ND_CYCLE(page);
    *(__IO uint8_t *)(NAND_DEVICE | ADDR_AREA) = ADDR_3RD_CYCLE(page);
    *(__IO uint8_t *)(NAND_DEVICE | CMD_AREA)  = NAND_CMD_AREA_TRUE1;
    // 延时tWB=100ns(Max)
    delay_ns(100);
    // 等待tR=25us(Max)(开启SDRAM+LTDC时实测最大等待>32us)
    if (nand_wait_ready(50000) != NAND_READY)
    {
        return HR_ERROR;
    }
    // 读状态之后需要再次设置数据输出
    *(__IO uint8_t *)(NAND_DEVICE | CMD_AREA) = NAND_CMD_AREA_A;

    if (column == 0 && length == hnand->Config.PageSize 
        #ifdef LX_INCLUDE_USER_DEFINE_FILE
        && (page % hnand->Config.BlockSize)     // 由于LevelX会对Page0多次读写, 因此Page0不校验
        #endif
    )
    {
        // ECC为256BYTE, 分8次读出数据并获取硬件ECC值
        uint32_t ecc_value_new[8];
        uint8_t* data = buffer;
        for(int i = 0; i < 8; i++)
        {
            // 启动ECC
            HAL_NAND_ECC_Enable(hnand);
            // 读数据
            for(int i = 0; i < 256; i++)
            {
                *buffer++ = *(__IO uint8_t *)NAND_DEVICE;
            }
            // 获取ECC值(阻塞等待FIFO空)
            HAL_NAND_GetECC(hnand, &ecc_value_new[i], HAL_MAX_DELAY);
            ecc_value_new[i] &= 0x3FFFFF;
            // 关闭ECC
            HAL_NAND_ECC_Disable(hnand);
        }
        // 读取备份区域中的ECC值(备用区域前24BYTE为ECC)
        uint8_t ecc_data_spare[24];
        for(int i = 0; i < 24; i++)
        {
            ecc_data_spare[i] = *(__IO uint8_t *)NAND_DEVICE;
        }
        // ECC校验
        uint32_t ecc_value_spare = 0;
        for(int i = 0; i < 8; i++)
        {
            ((uint8_t*)&ecc_value_spare)[0] = ecc_data_spare[i * 3];
            ((uint8_t*)&ecc_value_spare)[1] = ecc_data_spare[i * 3 + 1];
            ((uint8_t*)&ecc_value_spare)[2] = ecc_data_spare[i * 3 + 2];
            if (ecc_value_spare != ecc_value_new[i])
            {
                if (ecc_value_spare == 0x00FFFFFF && ecc_value_new[i] == 0)
                {
                    break;                  // 判断是未使用的页(全0xFF)则不再ECC校验
                }
                if (nand_256byte_ecc_check(data, ecc_value_new[i], ecc_value_spare) == HR_ECC_ERROR)
                {
                    printf("Page %d ECC Error\n", (int)page);
                    return HR_ECC_ERROR;    // 1bit以上ECC错误
                }
            }
            data += 256;
        }
    }
    else
    {
        for(int i = 0; i < length; i++)
        {
            *buffer++ = *(__IO uint8_t *)NAND_DEVICE;
        }
    }

    return HR_OK;
}

/**
 * @brief : 写Page(只有写整页时进行ECC校验)
 * @param  page   页索引
 * @param  column 页内字节索引
 * @param  buffer 要写入的数据
 * @param  length 要写入的数据字节数
 * @return 
 */
HRESULT NAND_Write_Page(uint32_t page, uint32_t column, uint8_t* buffer, uint32_t length)
{
    NAND_HandleTypeDef *hnand = &hNAND;
    if (page >= ((hnand->Config.BlockSize) * (hnand->Config.BlockNbr)) || 
        (column + length) > (hnand->Config.PageSize + hnand->Config.SpareAreaSize))
    {
        return HR_INVALID_ADDRESS;
    }

    // 写命令
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_WRITE0;
    // 地址
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = COLUMN_1ST_CYCLE(column);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = COLUMN_2ND_CYCLE(column);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = ADDR_1ST_CYCLE(page);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = ADDR_2ND_CYCLE(page);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = ADDR_3RD_CYCLE(page);
    // 延时tADL=70ns(Min)
    delay_ns(70);

    if (column == 0 && length == hnand->Config.PageSize 
        #ifdef LX_INCLUDE_USER_DEFINE_FILE
        && (page % hnand->Config.BlockSize)     // 由于LevelX会对Page0多次读写, 因此Page0不校验
        #endif
    )
    {
        // ECC为256BYTE, 分8次写数据并获取硬件ECC值
        uint32_t ecc_value_new[8];
        for(int i = 0; i < 8; i++)
        {
            // 启动ECC
            HAL_NAND_ECC_Enable(hnand);
            // 写数据
            for(int i = 0; i < 256; i++)
            {
                *(__IO uint8_t*)NAND_DEVICE = *buffer++;
            }
            // 获取ECC值(阻塞等待FIFO空)
            HAL_NAND_GetECC(hnand, &ecc_value_new[i], HAL_MAX_DELAY);
            ecc_value_new[i] &= 0x3FFFFF;
            // 关闭ECC
            HAL_NAND_ECC_Disable(hnand);
        }
        // 将ECC写入备份区域
        for(int i = 0; i < 8; i++)
        {
            *(__IO uint8_t *)NAND_DEVICE = ((uint8_t*)&ecc_value_new[i])[0];
            *(__IO uint8_t *)NAND_DEVICE = ((uint8_t*)&ecc_value_new[i])[1];
            *(__IO uint8_t *)NAND_DEVICE = ((uint8_t*)&ecc_value_new[i])[2];
        }
    }
    else
    {
        for(int i = 0; i < length; i++)
        {
            *(__IO uint8_t*)NAND_DEVICE = *buffer++;
        }
    }

    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_WRITE_TRUE1;
    // 延时tWB=100ns(Max)
    delay_ns(100);
    // 等待完成tPROG=700us(Max)
    if (nand_wait_ready(700000) != NAND_READY)
    {
        return HR_ERROR;
    }

    return HR_OK;
}

/**
 * @brief : 块擦除
 * @param  block 块索引
 * @return 
 */
HRESULT NAND_Erase_Block(uint32_t block)
{
    NAND_HandleTypeDef *hnand = &hNAND;
    if (block >= hnand->Config.BlockNbr)
    {
        return HR_INVALID_ADDRESS;
    }

    uint32_t nandaddress = block * hnand->Config.BlockSize;
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_ERASE0;
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = ADDR_3RD_CYCLE(nandaddress);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_ERASE1;
    // 延时tWB=100ns(Max)
    delay_ns(100);
    // 等待完成tBERS=10ms(Max)
    if (nand_wait_ready(10000000) != NAND_READY)
    {
        return HR_ERROR;
    }

    return HR_OK;
}

/**
 * @brief : 全片擦除(只擦除非全FF的块)
 * @param  
 * @return 
 */
HRESULT NAND_Erase_Chip(void)
{
    for(int i = 0; i < NAND_BLOCK_NUM; i++)
    {
        if (NAND_Erase_Block(i) != HR_OK)
        {
            printf("Erase Block %d Failed\n", i);
            return HR_ERROR;
        }
    }
    return HR_OK;
}

/**
 * @brief : 复位
 * @return 
 */
static HRESULT nand_reset(void)
{
    // 命令
    *(__IO uint8_t *)(NAND_DEVICE | CMD_AREA) = NAND_CMD_RESET;
    // 延时tWB=100ns(Max)
    delay_ns(100);
    // 等待完成tRST=1ms(Max)
    if (nand_wait_ready(1000000) != NAND_READY)
    {
        return HR_ERROR;
    }
    return HR_OK;
}

/**
 * @brief : 读状态
 * @return 
 */
static uint32_t nand_read_status(void)
{
    // 命令
    *(__IO uint8_t *)(NAND_DEVICE | CMD_AREA) = NAND_CMD_STATUS;
    // 延时tWHR=60ns(Min) FMC控制的时序此处有延时，但实测小于60ns，因此此处加一个延时
    delay_ns(60);
    // 数据
    uint32_t data = *(__IO uint8_t *)NAND_DEVICE;
    // 返回状态
    if ((data & NAND_ERROR) == NAND_ERROR)
    {
        return NAND_ERROR;
    }
    else if ((data & NAND_READY) == NAND_READY)
    {
        return NAND_READY;
    }
    else
    {
        return NAND_BUSY;
    }
}

/**
 * @brief : 等待RDY
 * @param  timeout_ns 超时时间ns
 * @return 
 */
static uint32_t nand_wait_ready(uint32_t timeout_ns)
{
    uint32_t start_tick = get_cpu_tick();
    uint32_t timeout    = ns_to_cpu_tick(timeout_ns);
    while((get_cpu_tick() - start_tick) < timeout)
    {
        uint32_t status = nand_read_status();
        if (status != NAND_BUSY)
        {
            // printf("%dns\n", cpu_tick_to_ns(get_cpu_tick() - start_tick));
            return status;
        }
    }
    return NAND_TIMEOUT_ERROR;
}


// ECC校验纠错
static HRESULT nand_256byte_ecc_check(uint8_t* data, uint32_t ecc_value_calc, uint32_t ecc_value_read)
{
    uint32_t bit_index = 0;
    uint32_t check_value = (ecc_value_calc ^ ecc_value_read) & 0x3FFFFF;
    if (check_value == 0)
    {
        return HR_OK;
    }
    for(int i = 0; i < 11; i++)
    {
        uint8_t val = check_value & 0x03;
        check_value = check_value >> 2;
        if (val == 0x01)
        {
            continue;
        }
        else if (val == 0x02)
        {
            bit_index |= 1 << i;
        }
        else
        {
            return HR_ECC_ERROR;
        }
    }
    data[bit_index / 8] ^= (uint8_t)(1 << (bit_index % 8));
    return HR_OK;
}










