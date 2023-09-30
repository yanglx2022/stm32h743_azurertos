/**
 * @brief   : USB STORAGE读写驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-05
 */

#include "ux_storage_driver.h"


// 读
UINT usb_storage_media_read(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status)
{
    switch(lun)
    {
        case LUN_NAND_FLASH:
            for(ULONG i = 0; i < number_blocks; i++)
            {
                if (lx_nand_flash_sector_read(&nand_flash, lba + i, data_pointer + nand_flash.lx_nand_flash_bytes_per_page * i) != LX_SUCCESS)
                {
                    return LX_ERROR;
                }
            }
            break;
        case LUN_SDCARD:
            break;
        default:
            break;
    }
    return LX_SUCCESS;
}

// 写
UINT usb_storage_media_write(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status)
{
    switch(lun)
    {
        case LUN_NAND_FLASH:
        {
            for(ULONG i = 0; i < number_blocks; i++)
            {
                if (lx_nand_flash_sector_write(&nand_flash, lba + i, data_pointer + nand_flash.lx_nand_flash_bytes_per_page * i) != LX_SUCCESS)
                {
                    return LX_ERROR;
                }
            }
            break;
        }
        case LUN_SDCARD:
            break;
        default:
            break;
    }
    return LX_SUCCESS;
}

// 状态
UINT usb_storage_media_status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status)
{
  /* The ATA drive never fails. This is just for app_usb_device only !!!! */
  return (0);
}


