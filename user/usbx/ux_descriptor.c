/**
 * @brief   : USB描述符
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-09-04
 */

#include "ux_descriptor.h"
#include "ux_stm32_config.h"

#define LANGUAGE_ID                                 0x0409              // English
#define MANUFACTURER_STRING                         "YANGLX2022"
#define PRODUCT_STRING                              "YANGLX2022"        // U盘弹出显示的内容

// USB设备描述符配置描述符集合
uint8_t USB_DeviceDesc_FS[] __attribute__ ((aligned (4))) =
{
    // 设备描述符
    0x12,                                   // bLength
    USB_DESC_TYPE_DEVICE,                   // bDescriptorType: DEVICE
    WBVAL(0x0200),                          // bcdUSB 2.0
    0x00,                                   // bDeviceClass
    0x00,                                   // bDeviceSubClass
    0x00,                                   // bDeviceProtocol
    USB_MAX_EP0_SIZE,                       // bMaxPacketSize
    WBVAL(0x0483),                          // idVendor
    WBVAL(0x0005),                          // idProduct
    WBVAL(0x0200),                          // bcdDevice rel. 2.00
    1,                                      // Index of manufacturer string descriptor
    2,                                      // Index of product string descriptor
    3,                                      // Index of serial number string descriptor
    1,                                      // bNumConfigurations

    // 配置描述符
    0x09,                                   // bLength
    USB_DESC_TYPE_CONFIGURATION,            // bDescriptorType: CONFIGURATION
    WBVAL(0x0020),                          // wTotalLength: 32
    0x01,                                   // bNumInterfaces
    0x01,                                   // bConfigurationValue
    0x00,                                   // iConfiguration(0 means no configuration string descriptor)
    USB_CONFIG_BUS_POWERED,                 // bmAttributes: bus powered
    USB_CONFIG_POWER_MA(300),               // MaxPower: 300mA

    // 接口描述符
    0x09,                                   // bLength
    USB_DESC_TYPE_INTERFACE,                // bDescriptorType: INTERFACE
    0x00,                                   // bInterfaceNumber
    0x00,                                   // bAlternateSetting
    0x02,                                   // bNumEndpoints
    0x08,                                   // bInterfaceClass: MSC Class
    0x06,                                   // bInterfaceSubClass: SCSI transparent
    0x50,                                   // nInterfaceProtocol
    0x00,                                   // iInterface(0 means no interface string descriptor)

    // 端点描述符(Bulk In)
    0x07,                                   // bLength
    USB_DESC_TYPE_ENDPOINT,                 // bDescriptorType: ENDPOINT
    0x81,                                   // bEndpointAddress: IN, Address 1
    0x02,                                   // bmAttributes: Bulk endpoint
    WBVAL(USB_FS_MAX_PACKET_SIZE),          // wMaxPacketSize
    0x00,                                   // bInterval(Polling interval in milliseconds)

    // 端点描述符(Bulk Out)
    0x07,                                   // bLength
    USB_DESC_TYPE_ENDPOINT,                 // bDescriptorType: ENDPOINT
    0x02,                                   // bEndpointAddress: OUT, Address 2
    0x02,                                   // bmAttributes: Bulk endpoint
    WBVAL(USB_FS_MAX_PACKET_SIZE),          // wMaxPacketSize
    0x00                                    // bInterval(Polling interval in milliseconds)
};

// 字符串描述符缓存
uint8_t USB_StringDesc[256] __attribute__ ((aligned (4))) = {0};

// 语言ID
uint8_t USB_LanguageID[]  __attribute__ ((aligned (4))) = 
{
    WBVAL(LANGUAGE_ID)
};


static uint8_t string_descriptor_make(uint8_t* descriptor, const char* text, uint8_t id);
static void serial_number_get(char* buffer);

/**
 * @brief : 获取设备描述符配置描述符集合
 * @param  length [OUT]描述符长度
 * @return 
 */
uint8_t* usb_device_config_descriptor_get(uint32_t* length)
{
    *length = sizeof(USB_DeviceDesc_FS);
    return USB_DeviceDesc_FS;
}

/**
 * @brief : 获取字符串描述符集合
 * @param  length [OUT]描述符长度
 * @return 
 */
uint8_t* usb_string_descriptor_get(uint32_t* length)
{
    uint8_t cnt;
    char serial_number[32];
    
    // 厂商字符串
    cnt = string_descriptor_make(USB_StringDesc, MANUFACTURER_STRING, 1);
    // 产品字符串
    cnt += string_descriptor_make(USB_StringDesc + cnt, PRODUCT_STRING, 2);
    // 序列号
    serial_number_get(serial_number);
    cnt += string_descriptor_make(USB_StringDesc + cnt, serial_number, 3);
    
    *length = cnt;
    return USB_StringDesc;
}

/**
 * @brief : 获取语言ID描述符
 * @param  length [OUT]描述符长度
 * @return 
 */
uint8_t* usb_language_id_get(uint32_t* length)
{
    *length = sizeof(USB_LanguageID);
    return USB_LanguageID;
}

// 生成单个符串描述符
static uint8_t string_descriptor_make(uint8_t* descriptor, const char* text, uint8_t id)
{
    uint8_t cnt = 0;
    uint8_t len = strlen(text);
    descriptor[cnt++] = LANGUAGE_ID & 0xFF;
    descriptor[cnt++] = LANGUAGE_ID >> 8;
    descriptor[cnt++] = id;
    descriptor[cnt++] = len;
    memcpy(descriptor + cnt, text, len);
    return (cnt + len);
}

// 数字转字符串
static void uint32_to_hex_string(uint32_t value, char* buffer)
{
    uint32_t val;
    for(int i = 0; i < 8; i++)
    {
        val = value >> 28;
        if (val < 0x0A)
        {
            buffer[i] = val + '0';
        }
        else
        {
            buffer[i] = val - 10 + 'A';
        }
        value = value << 4;
    }
}

// 获取STM32序列号
static void serial_number_get(char* buffer)
{
    uint32_to_hex_string(*(uint32_t*)UID_BASE, buffer);
    uint32_to_hex_string(*(uint32_t*)(UID_BASE + 4), buffer + 8);
    uint32_to_hex_string(*(uint32_t*)(UID_BASE + 8), buffer + 16);
    buffer[24] = 0;
}


