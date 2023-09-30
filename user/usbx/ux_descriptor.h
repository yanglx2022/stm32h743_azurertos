/**
 * @brief   : USB描述符
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-09-04
 */

#ifndef UX_DESCRIPTOR_H
#define UX_DESCRIPTOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// 描述符类型定义
#define  USB_DESC_TYPE_DEVICE                           0x01U
#define  USB_DESC_TYPE_CONFIGURATION                    0x02U
#define  USB_DESC_TYPE_STRING                           0x03U
#define  USB_DESC_TYPE_INTERFACE                        0x04U
#define  USB_DESC_TYPE_ENDPOINT                         0x05U

// 电源配置
#define USB_CONFIG_BUS_POWERED                          0x80
#define USB_CONFIG_SELF_POWERED                         0xC0
#define USB_CONFIG_POWER_MA(mA)                         ((mA)/2)

#define WBVAL(x)                                        ((x) & 0xFF), (((x) >> 8) & 0xFF)


uint8_t* usb_device_config_descriptor_get(uint32_t* length);
uint8_t* usb_string_descriptor_get(uint32_t* length);
uint8_t* usb_language_id_get(uint32_t* length);


#ifdef __cplusplus
}
#endif
#endif /* UX_DESCRIPTOR_H */


