/**
 * @brief   : 内存地址
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-04
 */

#ifndef MEMORY_H
#define MEMORY_H
#ifdef __cplusplus
extern "C" {
#endif


/* 片上内存定义
 * 共 (64 + 128 + 512 + 288 + 64 + 4)KB = 1060KB */
// ITCM 64KB 与内核同速 仅支持MDMA
#define ITCM_BASE                       ((uint32_t)0x00000000)
#define ITCM_SIZE                       (64 * 1024)

// DTCM 128KB 与内核同速 仅支持MDMA(不支持DMA1 DMA2 DMA2D BDMA)
#define DTCM_BASE                       ((uint32_t)0x20000000)
#define DTCM_SIZE                       (128 * 1024)

// D1域AXI_SRAM 512KB 不支持BDMA
#define D1_SRAM_BASE                    ((uint32_t)0x24000000)
#define D1_SRAM_SIZE                    (512 * 1024)
#define AXI_SRAM_BASE                   D1_SRAM_BASE
#define AXI_SRAM_SIZE                   D1_SRAM_SIZE

// D2域SRAM123 共288KB 不支持BDMA 注意使用这三块内存的话上电后需要分别使能
#define D2_SRAM_BASE                    ((uint32_t)0x30000000)
#define D2_SRAM_SIZE                    (288 * 1024)
// SRAM1 128KB
#define SRAM1_BASE                      D2_SRAM_BASE
#define SRAM1_SIZE                      (128 * 1024)
// SRAM2 128KB
#define SRAM2_BASE                      (SRAM1_BASE + SRAM1_SIZE)
#define SRAM2_SIZE                      (128 * 1024)
// SRAM3 32KB
#define SRAM3_BASE                      (SRAM2_BASE + SRAM2_SIZE)
#define SRAM3_SIZE                      (32 * 1024)

// D3_SRAM4 64KB 支持所有DMA
#define D3_SRAM4_BASE                   ((uint32_t)0x38000000)
#define D3_SRAM4_SIZE                   (64 * 1024)
#define SRAM4_BASE                      D3_SRAM4_BASE
#define SRAM4_SIZE                      D3_SRAM4_SIZE

// D3域备份RAM区 4KB
#define BACKUP_SRAM_BASE                ((uint32_t)0x38800000)
#define BACKUP_SRAM_SIZE                (4 * 1024)

/* 片外内存定义 */
// SDRAM 32MB
#define SDRAM_BASE                      ((uint32_t)0xC0000000)
#define SDRAM_SIZE                      (32 * 1024 * 1024)


#ifdef __cplusplus
}
#endif
#endif /* MEMORY_H */


