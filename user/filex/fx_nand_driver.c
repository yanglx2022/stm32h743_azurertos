/**
 * @brief   : FileX的NandFlash驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-08-05
 */

#include "fx_nand_driver.h"
#include "lx_api.h"

extern LX_NAND_FLASH nand_flash;

UINT _fx_partition_offset_calculate(void* partition_sector, UINT partition, ULONG *partition_start, ULONG *partition_size);


/**
 * @brief : I/O驱动程序入口
 * @param  *media_ptr
 * @return 
 */
void nand_driver(FX_MEDIA *media_ptr)
{
    switch (media_ptr->fx_media_driver_request)
    {
        case FX_DRIVER_INIT:			// 初始化
            media_ptr->fx_media_driver_free_sector_update = FX_TRUE;
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        case FX_DRIVER_UNINIT:			// 反初始化
            if (lx_nand_flash_close(&nand_flash) != LX_SUCCESS)
            {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                return;
            }
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        case FX_DRIVER_READ:			// 读扇区
        {
            ULONG sector = media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors;
            UCHAR* buffer = (UCHAR*)media_ptr->fx_media_driver_buffer;
            for(int i = 0; i < media_ptr->fx_media_driver_sectors; i++)
            {
                if (lx_nand_flash_sector_read(&nand_flash, sector, buffer) != LX_SUCCESS)
                {
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    return;
                }
                sector++;
                buffer += media_ptr->fx_media_bytes_per_sector;
            }
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }
        case FX_DRIVER_WRITE:			// 写扇区
        {
            ULONG sector = media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors;
            UCHAR* buffer = (UCHAR*)media_ptr->fx_media_driver_buffer;
            for(int i = 0; i < media_ptr->fx_media_driver_sectors; i++)
            {
                if (lx_nand_flash_sector_write(&nand_flash, sector, buffer) != LX_SUCCESS)
                {
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    return;
                }
                sector++;
                buffer += media_ptr->fx_media_bytes_per_sector;
            }
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }
        case FX_DRIVER_BOOT_READ:		// 读启动扇区
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            if (lx_nand_flash_sector_read(&nand_flash, 0, media_ptr->fx_media_driver_buffer) == LX_SUCCESS)
            {
                ULONG partition_start = 0;
                if (_fx_partition_offset_calculate(media_ptr->fx_media_driver_buffer, 0, &partition_start, FX_NULL) == FX_SUCCESS)
                {
                    media_ptr->fx_media_driver_status = FX_SUCCESS;
                    if (partition_start > 0)
                    {
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        if (lx_nand_flash_sector_read(&nand_flash, partition_start, media_ptr->fx_media_driver_buffer) == LX_SUCCESS)
                        {
                            media_ptr->fx_media_driver_status = FX_SUCCESS;
                        }
                    }
                }
            }
            break;
        case FX_DRIVER_BOOT_WRITE:		// 写启动扇区
            if (lx_nand_flash_sector_write(&nand_flash, 0, media_ptr->fx_media_driver_buffer) != LX_SUCCESS)
            {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                return;
            }
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        case FX_DRIVER_RELEASE_SECTORS:	// 释放扇区
        {
            ULONG sector = media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors;
            for(int i = 0; i < media_ptr->fx_media_driver_sectors; i++)
            {
                if (lx_nand_flash_sector_release(&nand_flash, sector + i) != LX_SUCCESS)
                {
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    return;
                }
            }
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }
        case FX_DRIVER_FLUSH:			// 不处理FLUSH与ABORT
        case FX_DRIVER_ABORT:
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        default:
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
    }
}


