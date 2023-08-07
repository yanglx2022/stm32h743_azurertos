/**
 * @brief   : LevelX的NAND FLASH驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-05
 */

#include "lx_nand_driver.h"

// LX_NAND_FLASH实例(由于开启了直接映射缓存此结构会很大(>1MB),因此放在SDRAM)
LX_NAND_FLASH nand_flash __attribute__ ((section (".noinit.SDRAM"), aligned (4)));

// 缓存区
static uint8_t verify_buffer[NAND_PAGE_SIZE + NAND_SPARE_SIZE];
static uint8_t page_buffer[NAND_PAGE_SIZE];

// 读page
UINT nand_driver_read_page(ULONG block, ULONG page, ULONG *destination, ULONG words)
{
    if (NAND_Read_Page((block * NAND_BLOCK_SIZE + page), 0, (uint8_t*)destination, words * 4) == HR_OK)
    {
        return LX_SUCCESS;
    }
    return LX_ERROR;
}

// 写page
UINT nand_driver_write_page(ULONG block, ULONG page, ULONG *source, ULONG words)
{
    if (NAND_Write_Page((block * NAND_BLOCK_SIZE + page), 0, (uint8_t*)source, words * 4) == HR_OK)
    {
        return LX_SUCCESS;
    }
    return LX_ERROR;
}

// 擦除块
UINT nand_driver_block_erase(ULONG block, ULONG erase_count)
{
    LX_PARAMETER_NOT_USED(erase_count);
    if (NAND_Erase_Block(block) == HR_OK)
    {
        return LX_SUCCESS;
    }
    return LX_ERROR;
}

// 页擦除验证
UINT nand_driver_page_erased_verify(ULONG block, ULONG page)
{
    uint32_t length = NAND_PAGE_SIZE + NAND_SPARE_SIZE;
    if (NAND_Read_Page((block * NAND_BLOCK_SIZE + page), 0, verify_buffer, length) == HR_OK)
    {
        for(int i = 0; i < length; i += 4)
        {
            if (*(uint32_t*)(verify_buffer + i) != 0xFFFFFFFF)
            {
                return LX_ERROR;
            }
        }
        return LX_SUCCESS;
    }
    return LX_ERROR;
}

// 块擦除验证
UINT nand_driver_block_erased_verify(ULONG block)
{
    for(int i = 0; i < NAND_BLOCK_SIZE; i++)
    {
        if (nand_driver_page_erased_verify(block, i) != LX_SUCCESS)
        {
            return LX_ERROR;
        }
    }
    return LX_SUCCESS;
}

// 获取块状态
UINT nand_driver_block_status_get(ULONG block, UCHAR *bad_block_byte)
{
    if (NAND_Read_Page(block * NAND_BLOCK_SIZE, NAND_PAGE_SIZE + BAD_BLOCK_FLAG_POS, bad_block_byte, 1) == HR_OK)
    {
        return LX_SUCCESS;
    }
    return LX_ERROR;
}


// 设置块状态
UINT nand_driver_block_status_set(ULONG block, UCHAR bad_block_byte)
{
    if (NAND_Write_Page(block * NAND_BLOCK_SIZE, NAND_PAGE_SIZE + BAD_BLOCK_FLAG_POS, &bad_block_byte, 1) == HR_OK)
    {
        return LX_SUCCESS;
    }
    return LX_ERROR;
}

// 获取额外字节
UINT nand_driver_block_extra_bytes_get(ULONG block, ULONG page, UCHAR *destination, UINT size)
{
    if (NAND_Read_Page((block * NAND_BLOCK_SIZE + page), NAND_PAGE_SIZE + EXTRA_BYTES_POS, destination, size) == HR_OK)
    {
        return LX_SUCCESS;
    }
    return LX_ERROR;
}

// 设置额外字节
UINT nand_driver_block_extra_bytes_set(ULONG block, ULONG page, UCHAR *source, UINT size)
{
    if (NAND_Write_Page((block * NAND_BLOCK_SIZE + page), NAND_PAGE_SIZE + EXTRA_BYTES_POS, source, size) == HR_OK)
    {
        return LX_SUCCESS;
    }
    return LX_ERROR;
}

// 系统错误
UINT nand_driver_system_error(UINT error_code, ULONG block, ULONG page)
{
    printf("LevelX nand_driver_system_error %d(block %d page %d)\n", (int)error_code, (int)block, (int)page);
    return LX_ERROR;
}

/**
 * @brief : LevelX NAND驱动初始化
 * @param  *instance LX_NAND_FLASH实例
 * @return 
 */
UINT nand_driver_initialize(LX_NAND_FLASH *instance)
{
    memset(page_buffer, 0xFF, NAND_PAGE_SIZE);

    // 参数
    instance->lx_nand_flash_total_blocks                = NAND_BLOCK_NUM;
    instance->lx_nand_flash_pages_per_block             = NAND_BLOCK_SIZE;
    instance->lx_nand_flash_bytes_per_page              = NAND_PAGE_SIZE;
    instance->lx_nand_flash_page_buffer                 = (ULONG*)page_buffer;

    // 回调
    instance->lx_nand_flash_driver_read                 = nand_driver_read_page;
    instance->lx_nand_flash_driver_write                = nand_driver_write_page;
    instance->lx_nand_flash_driver_block_erase          = nand_driver_block_erase;
    instance->lx_nand_flash_driver_page_erased_verify   = nand_driver_page_erased_verify;
    instance->lx_nand_flash_driver_block_erased_verify  = nand_driver_block_erased_verify;
    instance->lx_nand_flash_driver_block_status_get     = nand_driver_block_status_get;
    instance->lx_nand_flash_driver_block_status_set     = nand_driver_block_status_set;
    instance->lx_nand_flash_driver_extra_bytes_get      = nand_driver_block_extra_bytes_get;
    instance->lx_nand_flash_driver_extra_bytes_set      = nand_driver_block_extra_bytes_set;
    instance->lx_nand_flash_driver_system_error         = nand_driver_system_error;

    return LX_SUCCESS;
}


