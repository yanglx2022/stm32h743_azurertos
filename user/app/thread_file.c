/**
 * @brief   : 文件任务
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-21
 */

#include "thread_file.h"
#include "fx_sdcard_driver.h"
#include "sdcard.h"

// 任务堆栈
static UCHAR thread_stack[STACK_SIZE_DEFAULT];
// 任务句柄
static TX_THREAD file_thread;

// 文件系统
static FX_MEDIA media;
static uint8_t media_memory[64 * 1024] __attribute__((section(".noinit.AXI_RAM"), aligned(4)));
__IO static uint8_t fs_mounted = 0;

// UTF16转UTF8
static void utf16_to_utf8(const uint16_t *utf16, uint32_t utf16_len, char *utf8)
{
    int index = 0;
    for (int i = 0; i < utf16_len; i++)
    {
        if (utf16[i] < 0x0080)
        {
            utf8[index] = utf16[i] & 0x7F;
            index++;
        }
        else if (utf16[i] < 0x0800)
        {
            *(uint16_t *)(utf8 + index) = 0x80C0 | ((utf16[i] & 0x3F) << 8) | ((utf16[i] >> 6) & 0x1F);
            index += 2;
        }
        else
        {
            *(uint32_t *)(utf8 + index) = 0x8080E0 | ((utf16[i] & 0x3F) << 16) | ((utf16[i] & 0x0FC0) << 2) | ((utf16[i] >> 12) & 0x0F);
            index += 3;
        }
    }
    utf8[index] = 0;
}
/**
 * @brief : 文件任务
 * @param  thread_input
 * @return 
 */
void file_thread_entry(ULONG thread_input)
{
    while (1)
    {
        // SD卡检测
        HRESULT hr = SDCard_Detect();
        // 文件系统
        if (hr == HR_SD_INSERT)
        {
            printf("SD卡已插入\n");
            // 文件系统挂载
            UINT status = fx_media_open(&media, "SD DISK", sdcard_driver, FX_NULL, media_memory, sizeof(media_memory));
            if (status == FX_SUCCESS)
            {
                fs_mounted = 1;

                // 测试
                printf("挂载文件系统成功\n");
                CHAR filename[128];
                CHAR filename_utf8[128];
                UINT attributes;
                ULONG size;
                UINT year;
                UINT month;
                UINT day;
                UINT hour;
                UINT minute;
                UINT second;
                while (fx_directory_next_full_entry_find(&media, filename, &attributes, &size,
                                                         &year, &month, &day, &hour, &minute, &second) == FX_SUCCESS)
                {
                    // 路径或文件名中有中文时得到的为短文件名,使用fx_unicode_name_get获取长文件名再转为utf8以便显示
                    ULONG len;
                    UINT ret = fx_unicode_name_get(&media, filename, (UCHAR *)filename, &len);
                    if (ret == FX_SUCCESS)
                    {
                        utf16_to_utf8((uint16_t*)filename, len, filename_utf8);
                        printf("%08X %s\n", attributes, filename_utf8);
                        for (int i = 0; i < strlen(filename_utf8); i++)
                        {
                            printf("%02X ", (int)filename_utf8[i]);
                        }
                        printf("\n");
                    }
                    else
                    {
                        printf("%08X %s (0x%02x)\n", attributes, filename, ret);
                    }
                }
                printf("===========\n");
            }
            else
            {
                printf("挂载文件系统失败(%d)", status);
            }
        }
        else if (hr == HR_SD_REMOVAL)
        {
            fs_mounted = 0;
            printf("SD卡已拔出\n");
            fx_media_close(&media);
        }
        if (fs_mounted)
        {
            fx_media_flush(&media);
        }
        tx_thread_sleep(MS_TO_TICKS(1000));
    }
}

/**
 * @brief : 创建文件任务
 * @param  
 * @return 
 */
void file_thread_create(void)
{
    // 文件系统初始化
    fx_system_initialize();
    // DATE_TIME datetime;
    // RTC_Get_DateTime(&datetime);
    // fx_system_date_set(datetime.year, datetime.month, datetime.day);
    // fx_system_time_set(datetime.hour, datetime.minute, datetime.second);
    // 创建任务
    tx_thread_create(&file_thread, "file_thread", file_thread_entry, 0,
                     thread_stack, STACK_SIZE_DEFAULT, FILE_THREAD_PRI, FILE_THREAD_PRI, TX_NO_TIME_SLICE, TX_AUTO_START);
}


