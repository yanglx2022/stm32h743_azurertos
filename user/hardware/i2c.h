/**
 * @brief   : I2C主机驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-12-17
 */

#ifndef I2C_H
#define I2C_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define I2C_BAUDRATE_US                 (2)     // I2C通信比特周期(us)
#define I2C_ACK_TIMEOUT_US              (20)    // I2C等待ACK超时时间(us)


void i2c_init(void);
void i2c_write(uint8_t address, const uint8_t *data, uint32_t length);
void i2c_register_write(uint8_t address, uint8_t reg, const uint8_t *data, uint32_t length);
void i2c_read(uint8_t address, uint8_t *buffer, uint32_t length);
void i2c_register_read(uint8_t address, uint8_t reg, uint8_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif
#endif /* I2C_H */


