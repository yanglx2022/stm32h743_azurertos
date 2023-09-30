/**
 * @brief   : USB STORAGE读写驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-05
 */

#ifndef UX_STORAGE_DRIVER_H
#define UX_STORAGE_DRIVER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lx_nand_driver.h"

#define LUN_NAND_FLASH              0
#define LUN_SDCARD                  1

UINT usb_storage_media_read(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status);
UINT usb_storage_media_write(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status);
UINT usb_storage_media_status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status);


#ifdef __cplusplus
}
#endif
#endif /* UX_STORAGE_DRIVER_H */


