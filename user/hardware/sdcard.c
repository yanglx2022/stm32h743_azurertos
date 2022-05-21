/**
 * @brief   : SD卡驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-19
 */

#include "sdcard.h"

// HAL SDCard实例
static SD_HandleTypeDef hSDCard;

// SD卡插拔状态
__IO static uint8_t sdcard_inserted = 0;

// DMA完成信号量
static TX_SEMAPHORE transfer_semaphore;

// 对齐缓冲区(IDMA需要字对齐)
static uint8_t scratch[SD_BLOCKSIZE] __attribute__ ((section (".noinit.AXI_RAM"), aligned (4)));


/**
 * @brief : SDCard初始化
 * @param  
 * @return 
 */
void SDCard_Init(void)
{
    // 使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    LL_AHB4_GRP1_EnableClock(SD_DETECT_GPIO_EN);

    /** SDMMC2 GPIO
    PB14 AF9  ------> SDMMC2_D0
    PB15 AF9  ------> SDMMC2_D1
    PB3  AF9  ------> SDMMC2_D2
    PB4  AF9  ------> SDMMC2_D3
    PC1  AF9  ------> SDMMC2_CK
    PD7  AF11 ------> SDMMC2_CMD */
    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FAST;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_9;
    // PB3 PB4 PB14 PB15
    GPIO_InitStruct.Pin = LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    // PC1
    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    // PD7
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_11;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    // 插拔检测 上拉输入
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Pin         = SD_DETECT_PIN;
    LL_GPIO_Init(SD_DETECT_GPIO, &GPIO_InitStruct);

    // 设置SDMMC参数
    hSDCard.Instance                 = SDMMC2;
    hSDCard.Init.ClockEdge           = SDMMC_CLOCK_EDGE_FALLING;
    hSDCard.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hSDCard.Init.BusWide             = SDMMC_BUS_WIDE_4B;
    hSDCard.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hSDCard.Init.ClockDiv            = 4;   // SDMMC Clock frequency = 200MHz / (2 * 4) = 25MHz

    // 设置中断
    NVIC_SetPriority(SDMMC2_IRQn, SDCARD_PRI);
    NVIC_EnableIRQ(SDMMC2_IRQn);
}

/**
 * @brief : Msp初始化
 * @param  *hsd
 * @return 
 */
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    UNUSED(hsd);
    // 开启时钟
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_SDMMC2);
    // 复位
    LL_AHB2_GRP1_ForceReset(LL_AHB2_GRP1_PERIPH_SDMMC2);
    LL_AHB2_GRP1_ReleaseReset(LL_AHB2_GRP1_PERIPH_SDMMC2);
}

/**
 * @brief : Msp反初始化
 * @param  *hsd
 * @return 
 */
void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
    UNUSED(hsd);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_SDMMC2);
}

/**
 * @brief : SD卡插拔检测(SD初始化)
 * @param  
 * @return HR_SD_INSERT:SD卡插入 HR_SD_REMOVAL:SD卡拔出 HR_OK:无动作(插入状态) HR_NO_DEVICE:无动作(未插入状态)
 */
HRESULT SDCard_Detect(void)
{
    uint8_t insert = (LL_GPIO_IsInputPinSet(SD_DETECT_GPIO, SD_DETECT_PIN) == 0UL);
    if (insert && sdcard_inserted == 0)         // 插入
    {
        // 反初始化
        HAL_SD_DeInit(&hSDCard);
        // SD初始化
        if (HAL_SD_Init(&hSDCard) == HAL_OK)
        {
            sdcard_inserted = 1;
            // 创建DMA传输完成信号量
            tx_semaphore_create(&transfer_semaphore, "sdcard transfer semaphore", 0);
            return HR_SD_INSERT;
        }
        return HR_NO_DEVICE;
    }
    else if (insert == 0 && sdcard_inserted)    // 拔出
    {
        sdcard_inserted = 0;
        // 删除信号量
        tx_semaphore_delete(&transfer_semaphore);
        // 反初始化
        HAL_SD_DeInit(&hSDCard);
        return HR_SD_REMOVAL;
    }
    if (sdcard_inserted)
    {
        return HR_OK;
    }
    return HR_NO_DEVICE;
}

/**
 * @brief : 查询SD卡信息
 * @param  CardInfo SD卡信息
 * @return HR_OK:查询成功 HR_ERROR:查询失败 HR_NO_DEVICE:未插入SD卡
 */
HRESULT SDCard_GetCardInfo(SD_CardInfo* CardInfo)
{
    if (sdcard_inserted == 0)
    {
        return HR_NO_DEVICE;
    }
    if (HAL_SD_GetCardInfo(&hSDCard, CardInfo) == HAL_OK)
    {
        return HR_OK;
    }
    return HR_ERROR;
}

/**
 * @brief : 读数据
 * @param  data      数据缓冲区
 * @param  block_idx 块索引
 * @param  block_num 块数量
 * @return HR_OK:读成功 HR_ERROR:读失败 HR_TIMEOUT:读超时 HR_NO_DEVICE:未插入SD卡
 */
HRESULT SDCard_Read(uint8_t* data, uint32_t block_idx, uint32_t block_num)
{
    if (sdcard_inserted == 0)
    {
        return HR_NO_DEVICE;
    }
    if ((uint32_t)data & 0x03UL ||      // IDMA需要字对齐
        (uint32_t)data < D1_SRAM_BASE)  // IDMA无法访问TCM区
    {
        for(int i = 0; i < block_num; i++)
        {
            if (HAL_SD_ReadBlocks_DMA(&hSDCard, scratch, block_idx++, 1) != HAL_OK)
            {
                return HR_ERROR;
            }
            if (tx_semaphore_get(&transfer_semaphore, MS_TO_TICKS(SD_TIMEOUT_MS)) != TX_SUCCESS)
            {
                return HR_TIMEOUT;
            }
            SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, SD_BLOCKSIZE);
            memcpy(data, scratch, SD_BLOCKSIZE);
            data += SD_BLOCKSIZE;
        }
    }
    else
    {
        if (HAL_SD_ReadBlocks_DMA(&hSDCard, data, block_idx, block_num) != HAL_OK)
        {
            return HR_ERROR;
        }
        if (tx_semaphore_get(&transfer_semaphore, MS_TO_TICKS(SD_TIMEOUT_MS)) != TX_SUCCESS)
        {
            return HR_TIMEOUT;
        }
        SCB_InvalidateDCache_by_Addr((uint32_t*)data, SD_BLOCKSIZE * block_num);
    }
    return HR_OK;
}

/**
 * @brief : 写数据
 * @param  data      数据缓冲区
 * @param  block_idx 块索引
 * @param  block_num 块数量
 * @return HR_OK:写成功 HR_ERROR:写失败 HR_TIMEOUT:写超时 HR_NO_DEVICE:未插入SD卡
 */
HRESULT SDCard_Write(uint8_t* data, uint32_t block_idx, uint32_t block_num)
{
    if (sdcard_inserted == 0)
    {
        return HR_NO_DEVICE;
    }
    if ((uint32_t)data & 0x03UL ||      // IDMA需要字对齐
        (uint32_t)data < D1_SRAM_BASE)  // IDMA无法访问TCM区
    {
        for(int i = 0; i < block_num; i++)
        {
            memcpy(scratch, data, SD_BLOCKSIZE);
            data += SD_BLOCKSIZE;
            SCB_CleanDCache_by_Addr((uint32_t*)scratch, SD_BLOCKSIZE);
            if (HAL_SD_WriteBlocks_DMA(&hSDCard, scratch, block_idx++, 1) != HAL_OK)
            {
                return HR_ERROR;
            }
            if (tx_semaphore_get(&transfer_semaphore, MS_TO_TICKS(SD_TIMEOUT_MS)) != TX_SUCCESS)
            {
                return HR_TIMEOUT;
            }
        }
    }
    else
    {
        SCB_CleanDCache_by_Addr((uint32_t*)data, SD_BLOCKSIZE * block_num);
        if (HAL_SD_WriteBlocks_DMA(&hSDCard, data, block_idx, block_num) != HAL_OK)
        {
            return HR_ERROR;
        }
        if (tx_semaphore_get(&transfer_semaphore, MS_TO_TICKS(SD_TIMEOUT_MS)) != TX_SUCCESS)
        {
            return HR_TIMEOUT;
        }
    }
    return HR_OK;
}

/**
 * @brief : 读完成回调
 * @param  *hsd
 * @return 
 */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    UNUSED(hsd);
    tx_semaphore_put(&transfer_semaphore);
}

/**
 * @brief : 写完成回调
 * @param  *hsd
 * @return 
 */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    HAL_SD_RxCpltCallback(hsd);
}

/**
 * @brief : 错误回调
 * @param  *hsd
 * @return 
 */
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    printf("HAL_SD_ErrorCallback %d\n", (int)hsd->ErrorCode);
}

/**
 * @brief : 中断响应
 * @param  
 * @return 
 */
void SDMMC2_IRQHandler(void)
{
    HAL_SD_IRQHandler(&hSDCard);
}


