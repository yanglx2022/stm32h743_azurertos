/**
 * @brief   : SPI接口
 * @author  : yanglx2022
 * @version : V0.1.0
 * @date    : 2024-01-08
 * @note    : 
 */

#ifndef SPI_H
#define SPI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"


void spi_init(void);
uint8_t spi_byte_write_read(uint8_t data);


#ifdef __cplusplus
}
#endif
#endif /* SPI_H */


