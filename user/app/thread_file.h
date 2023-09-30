/**
 * @brief   : 文件任务
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2022-05-21
 */

#ifndef THREAD_FILE_H
#define THREAD_FILE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define FS_MEDIA_NUM                    (2)
#define FS_NANDFLASH                    (0)
#define FS_SDCARD                       (1)


void file_thread_create(void);


#ifdef __cplusplus
}
#endif
#endif /* THREAD_FILE_H */


