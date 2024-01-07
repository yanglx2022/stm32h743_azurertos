/**
 * @brief   : I2C主机驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2023-12-17
 */

#include "i2c.h"

// PH4 SCL
#define I2C_SCL_GPIO                    GPIOH
#define I2C_SCL_PIN                     LL_GPIO_PIN_4
// PH5 SDA
#define I2C_SDA_GPIO                    GPIOH
#define I2C_SDA_PIN                     LL_GPIO_PIN_5

// IO操作
#define I2C_SCL(n)                      ((n) ? LL_GPIO_SetOutputPin(I2C_SCL_GPIO, I2C_SCL_PIN) : LL_GPIO_ResetOutputPin(I2C_SCL_GPIO, I2C_SCL_PIN)) // 写SCL
#define I2C_SDA(n)                      ((n) ? LL_GPIO_SetOutputPin(I2C_SDA_GPIO, I2C_SDA_PIN) : LL_GPIO_ResetOutputPin(I2C_SDA_GPIO, I2C_SDA_PIN)) // 写SDA
#define I2C_SDA_IN(n)                   ((LL_GPIO_ReadInputPort(I2C_SDA_GPIO) & I2C_SDA_PIN) ? 1 : 0)   // 读SDA

static void i2c_start(void);
static void i2c_stop(void);
static void i2c_write_byte(uint8_t data);
static uint8_t i2c_read_byte(uint8_t ack);


/**
 * @brief : I2C初始化
 * @return 
 */
void i2c_init(void)
{
    static uint8_t initialized = 0;
    if (initialized)
    {
        return;
    }
    initialized = 1;

    // GPIO使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = I2C_SCL_PIN;
    LL_GPIO_Init(I2C_SCL_GPIO, &GPIO_InitStruct);   // SCL开漏输出
    GPIO_InitStruct.Pin         = I2C_SDA_PIN;
    LL_GPIO_Init(I2C_SDA_GPIO, &GPIO_InitStruct);   // SDA开漏输出(开漏输出时也可读取GPIO状态)

    // 空闲状态
    I2C_SCL(1);
    I2C_SDA(1);
}

/**
 * @brief : I2C写数据
 * @param  address I2C地址(7bit)
 * @param  *data   数据
 * @param  length  数据长度
 * @return 
 */
void i2c_write(uint8_t address, const uint8_t *data, uint32_t length)
{
    i2c_start();
    i2c_write_byte(address << 1);
    for(int i = 0; i < length; i++)
    {
        i2c_write_byte(data[i]);    // 发bit
    }
    i2c_stop();
}

/**
 * @brief : I2C写寄存器数据
 * @param  address I2C地址(7bit)
 * @param  reg     寄存器地址
 * @param  *data   数据
 * @param  length  数据长度
 * @return 
 */
void i2c_register_write(uint8_t address, uint8_t reg, const uint8_t *data, uint32_t length)
{
    i2c_start();
    i2c_write_byte(address << 1);
    i2c_write_byte(reg);
    for(int i = 0; i < length; i++)
    {
        i2c_write_byte(data[i]);    // 发bit
    }
    i2c_stop();
}

/**
 * @brief : I2C读数据
 * @param  address I2C地址(7bit)
 * @param  *buffer 数据缓冲区
 * @param  length  数据长度
 * @return 
 */
void i2c_read(uint8_t address, uint8_t *buffer, uint32_t length)
{
    i2c_start();
    i2c_write_byte((address << 1) | 0x01);
    for(int i = 0; i < length - 1; i++)
    {
        buffer[i] = i2c_read_byte(1);       // 读bit并ACK
    }
    buffer[length - 1] = i2c_read_byte(0);  // 读bit并NACK
    i2c_stop();
}

/**
 * @brief : I2C读寄存器数据
 * @param  address I2C地址(7bit)
 * @param  reg     寄存器地址
 * @param  *buffer 数据缓冲区
 * @param  length  数据长度
 * @return 
 */
void i2c_register_read(uint8_t address, uint8_t reg, uint8_t *buffer, uint32_t length)
{
    i2c_start();
    i2c_write_byte(address << 1);
    i2c_write_byte(reg);
    i2c_start();
    i2c_write_byte((address << 1) | 0x01);
    for(int i = 0; i < length - 1; i++)
    {
        buffer[i] = i2c_read_byte(1);       // 读bit并ACK
    }
    buffer[length - 1] = i2c_read_byte(0);  // 读bit并NACK
    i2c_stop();
}

/**
 * @brief : I2C起始信号
 * @return 
 */
static void i2c_start(void)
{
    // SCL高时SDA下降沿为起始信号
    I2C_SCL(1);
    I2C_SDA(1);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);
    I2C_SDA(0);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);
}

/**
 * @brief : I2C停止信号
 * @return 
 */
static void i2c_stop(void)
{
    // SCL高时SDA上升沿为停止信号
    I2C_SCL(1);
    I2C_SDA(0);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);
    I2C_SDA(1);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);
}

/**
 * @brief : I2C写一个字节并等待ACK
 * @param  data 字节数据
 * @return 
 */  
static void i2c_write_byte(uint8_t data)
{
    // 逐位发送数据
    for(int i = 7; i >= 0; i--)
    {
        I2C_SCL(0);
        delay_ns(I2C_BAUDRATE_US * 1000 / 4);
        I2C_SDA((data >> i) & 0x01);
        delay_ns(I2C_BAUDRATE_US * 1000 / 4);
        I2C_SCL(1);
        delay_ns(I2C_BAUDRATE_US * 1000 / 2);
    }

    // 等待ACK
    uint8_t wait_us_cnt = 0;
    I2C_SDA(1); // 开漏输出1即释放SDA线
    I2C_SCL(0);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);
    I2C_SCL(1);
    do
    {
        wait_us_cnt++;
        if (wait_us_cnt > I2C_ACK_TIMEOUT_US)
        {
            break;
        }
        delay_us(1);
    } while(I2C_SDA_IN());
    I2C_SCL(0);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);
}

/**
 * @brief : I2C读一个字节并写ACK
 * @param  ack 1ACK 0NACK
 * @return 字节数据
 */
static uint8_t i2c_read_byte(uint8_t ack)
{
    // 逐位接收数据
    uint8_t data = 0;
    I2C_SDA(1); // 开漏输出1即释放SDA线
    for(int i = 7; i >= 0; i--)
    {
        I2C_SCL(0);
        delay_ns(I2C_BAUDRATE_US * 1000 / 2);
        I2C_SCL(1);
        delay_ns(I2C_BAUDRATE_US * 1000 / 4);
        data |= I2C_SDA_IN() << i;
        delay_ns(I2C_BAUDRATE_US * 1000 / 4);
    }

    // 发送ACK
    I2C_SCL(0);
    delay_ns(I2C_BAUDRATE_US * 1000 / 4);
    I2C_SDA(!ack);
    delay_ns(I2C_BAUDRATE_US * 1000 / 4);
    I2C_SCL(1);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);
    I2C_SCL(0);
    delay_ns(I2C_BAUDRATE_US * 1000 / 2);

    return data;
}


