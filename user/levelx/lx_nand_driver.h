/**
 * @brief   : LevelX的NAND FLASH驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-05
 */

#ifndef LX_NAND_DRIVER_H
#define LX_NAND_DRIVER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "lx_api.h"

// 坏块标志位置(起始位置放了ECC, 因此坏块标志往后放了)
#define	BAD_BLOCK_FLAG_POS                  32
// 额外字节位置
#define	EXTRA_BYTES_POS                     36

extern LX_NAND_FLASH nand_flash;

UINT nand_driver_initialize(LX_NAND_FLASH *instance);


#ifdef __cplusplus
}
#endif
#endif /* LX_NAND_DRIVER_H */


