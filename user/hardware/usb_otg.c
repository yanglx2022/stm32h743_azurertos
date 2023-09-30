/**
 * @brief   : USB_OTG硬件驱动(HAL)
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-08-30
 */

#include "usb_otg.h"
#include "ux_dcd_stm32.h"
#include "ux_stm32_config.h"

static PCD_HandleTypeDef hpcd_USB_OTG_FS;


/**
 * @brief : USB OTG硬件初始化
 * @param  
 * @return 
 */
HRESULT USB_OTG_Init(void)
{
    // 初始化USB
    memset(&hpcd_USB_OTG_FS, 0, sizeof(PCD_HandleTypeDef));
    hpcd_USB_OTG_FS.Instance            = USB_OTG_FS;
    hpcd_USB_OTG_FS.Init.dev_endpoints  = UX_DCD_STM32_MAX_ED;
    hpcd_USB_OTG_FS.Init.ep0_mps        = USB_MAX_EP0_SIZE;
    hpcd_USB_OTG_FS.Init.speed          = PCD_SPEED_FULL;
    hpcd_USB_OTG_FS.Init.phy_itface     = PCD_PHY_EMBEDDED;
    if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
    {
        return HR_ERROR;
    }

    // 设置RX_FIFO和TX_FIFO(注意单位为字, 加起来不能超过1024)
    HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 512);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 64);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 256);

    // 链接到USBX协议栈
    if (ux_dcd_stm32_initialize((ULONG)USB_OTG_FS, (ULONG)&hpcd_USB_OTG_FS) != UX_SUCCESS)
    {
        return HR_ERROR;
    }

    // USB启动
    if (HAL_PCD_Start(&hpcd_USB_OTG_FS) != HAL_OK)
    {
        return HR_ERROR;
    }

    return HR_OK;
}

/**
 * @brief : Msp初始化(GPIO 时钟 中断)
 * @param  pcdHandle
 * @return 
 */
void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{
    // 使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);

    // GPIO
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /** 
    PA11  ------>  USB_OTG_FS_DM
    PA12  ------>  USB_OTG_FS_DP  */
    GPIO_InitStruct.Mode		= LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed		= LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType	= LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull 		= LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate	= LL_GPIO_AF_10;
    GPIO_InitStruct.Pin			= LL_GPIO_PIN_11 | LL_GPIO_PIN_12;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 使能电压检测
    LL_PWR_EnableUSBVoltageDetector();

    // 使能USB时钟
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    
    // 配置中断
    NVIC_SetPriority(OTG_FS_IRQn, USB_PRI);
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

/**
 * @brief : Msp DeInit
 * @param  pcdHandle
 * @return 
 */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{
	__HAL_RCC_USB_OTG_FS_CLK_DISABLE();
	NVIC_DisableIRQ(OTG_FS_IRQn);
}

/**
 * @brief : USB中断处理
 * @return 
 */
void OTG_FS_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}


