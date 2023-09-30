/**
 * @brief   : NandFlash驱动(H27U4G8)(HAL)
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-07-15
 */

#ifndef NANDFLASH_H
#define NANDFLASH_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define NAND_BLOCK_NUM                  (4096)  // 总块数
#define NAND_BLOCK_SIZE                 (64)    // 每块页数
#define NAND_PAGE_SIZE                  (2048)  // 每页数据字节数
#define NAND_SPARE_SIZE                 (64)    // 每页备用字节数
#define NAND_PLANE_NUM                  (2)     // plane数
#define NAND_PLANE_SIZE                 (2048)  // 每plane块数


HRESULT NAND_Init(void);
HRESULT NAND_Read_Page(uint32_t page, uint32_t column, uint8_t* buffer, uint32_t length);
HRESULT NAND_Write_Page(uint32_t page, uint32_t column, uint8_t* buffer, uint32_t length);
HRESULT NAND_Erase_Block(uint32_t block);
HRESULT NAND_Erase_Chip(void);


#ifdef __cplusplus
}
#endif
#endif /* NANDFLASH_H */


