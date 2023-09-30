/**
 * @brief   : FileX的SD卡驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-21
 */

#include "fx_sdcard_driver.h"
#include "sdcard.h"

UINT _fx_partition_offset_calculate(void* partition_sector, UINT partition, ULONG *partition_start, ULONG *partition_size);

/**
 * @brief : I/O驱动程序入口
 * @param  *media_ptr
 * @return 
 */
VOID sdcard_driver(FX_MEDIA *media_ptr)
{
    switch (media_ptr->fx_media_driver_request)
    {
        case FX_DRIVER_INIT:            // SD卡初始化在FileX外管理
        case FX_DRIVER_UNINIT:
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        case FX_DRIVER_READ:            // 读扇区
            if (SDCard_Read(media_ptr->fx_media_driver_buffer, 
                            media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors, 
                            media_ptr->fx_media_driver_sectors) == HR_OK)
            {
                media_ptr->fx_media_driver_status = FX_SUCCESS;
            }
            else
            {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;
            }
            break;
        case FX_DRIVER_WRITE:           // 写扇区
            if (SDCard_Write(media_ptr->fx_media_driver_buffer, 
                             media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors, 
                             media_ptr->fx_media_driver_sectors) == HR_OK)
            {
                media_ptr->fx_media_driver_status = FX_SUCCESS;
            }
            else
            {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;
            }
            break;
        case FX_DRIVER_BOOT_READ:       // 读启动扇区
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            if (SDCard_Read(media_ptr->fx_media_driver_buffer, 0, 1) == HR_OK)
            {
                ULONG partition_start = 0;
                if (_fx_partition_offset_calculate(media_ptr->fx_media_driver_buffer, 0, &partition_start, FX_NULL) == FX_SUCCESS)
                {
                    media_ptr->fx_media_driver_status = FX_SUCCESS;
                    if (partition_start > 0)
                    {
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        if (SDCard_Read(media_ptr->fx_media_driver_buffer, partition_start, 1) == HR_OK)
                        {
                            media_ptr->fx_media_driver_status = FX_SUCCESS;
                        }
                    }
                }
            }
            break;
        case FX_DRIVER_BOOT_WRITE:      // 写启动扇区
            if (SDCard_Write(media_ptr->fx_media_driver_buffer, 0, 1) == HR_OK)
            {
                media_ptr->fx_media_driver_status = FX_SUCCESS;
            }
            else
            {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;
            }
            break;
        case FX_DRIVER_FLUSH:           // 不处理FLUSH与ABORT
        case FX_DRIVER_ABORT:
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        default:
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
    }
}


