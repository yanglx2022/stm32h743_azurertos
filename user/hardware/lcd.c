/**
 * @brief   : LCD驱动
 * @author  : yanglx2022
 * @version : V0.1.0
 * @date    : 2024-01-08
 * @note    : 
 */

#include "lcd.h"
#include "spi.h"
#include "font_ascii.h"
#include "stdarg.h"

// PB5 背光控制
#define LCD_BL_GPIO                     GPIOB
#define LCD_BL_PIN                      LL_GPIO_PIN_5
// PC11 数据命令选择
#define LCD_DCS_GPIO                    GPIOC
#define LCD_DCS_PIN                     LL_GPIO_PIN_11

// 文字颜色
#define COLOR_TEXT                      COLOR_DODGER_BLUE
// 背景颜色
#define COLOR_BACKGROUND                COLOR_BLACK
// 滚动条颜色
#define COLOR_SCROLLBAR                 COLOR_DIM_GRAY

// 显示方向
static lcd_direction_t lcd_direction = LCD_HORIZON;

// 坐标偏移(ST7789显存为240*320, 屏幕分辨率为135*240)
const static uint16_t OFFSET_X[] = {40, 52, 40, 53};
const static uint16_t OFFSET_Y[] = {53, 40, 52, 40};

uint16_t LCD_WIDTH  = 0;    // LCD像素宽
uint16_t LCD_HEIGHT = 0;    // LCD像素高

const static int FONT = 16;         // 文本字号
const static int SCROLL_WIDTH = 5;  // 滚动条宽度
static char text_buffer[4096] = {0};// 文本显示缓冲区
static char text_frame[512] = {0};  // 当前帧字符
static int text_line_cnt = 0;       // 文本总行数
static int text_line_cur = 0;       // 当前显示首行号

static void st7789_reg_write(uint8_t reg, uint8_t *data, uint32_t length);
static void st7789_data_write(uint16_t data);
static void st7789_area_set(uint16_t x, uint16_t y, uint16_t width, uint16_t height);


/**
 * @brief  LCD初始化
 * @return 
 */
void LCD_Init(void)
{
    spi_init();

    // GPIO使能时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);

    // GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FAST;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = LCD_BL_PIN;
    LL_GPIO_Init(LCD_BL_GPIO, &GPIO_InitStruct);
    GPIO_InitStruct.Pin         = LCD_DCS_PIN;
    LL_GPIO_Init(LCD_DCS_GPIO, &GPIO_InitStruct);

    // 配置LCD
    st7789_reg_write(0x36, (uint8_t []){0xC0}, 1);
    st7789_reg_write(0x3A, (uint8_t []){0x05}, 1);
    st7789_reg_write(0xB2, (uint8_t []){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5);
    st7789_reg_write(0xB7, (uint8_t []){0x35}, 1);
    st7789_reg_write(0xBB, (uint8_t []){0x19}, 1);
    st7789_reg_write(0xC0, (uint8_t []){0x2C}, 1);
    st7789_reg_write(0xC2, (uint8_t []){0x01}, 1);
    st7789_reg_write(0xC3, (uint8_t []){0x12}, 1);
    st7789_reg_write(0xC4, (uint8_t []){0x20}, 1);
    st7789_reg_write(0xC6, (uint8_t []){0x0F}, 1);
    st7789_reg_write(0xD0, (uint8_t []){0xA4, 0xA1}, 2);
    st7789_reg_write(0xE0, (uint8_t []){0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}, 14);
    st7789_reg_write(0xE1, (uint8_t []){0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}, 14);
    st7789_reg_write(0x21, NULL, 0);
    st7789_reg_write(0x11, NULL, 0);
    st7789_reg_write(0x29, NULL, 0);

    // 默认水平方向
    LCD_Direction_Set(LCD_HORIZON);

    // 提示信息
    LCD_Print("#scroll by vol+/vol- button#\n");

    // 打开背光
    LCD_Enable(1);
}

/**
 * @brief  显示使能
 * @param  enable 使能/禁止
 * @return 
 */
void LCD_Enable(uint8_t enable)
{
    if (enable)
    {
        LL_GPIO_SetOutputPin(LCD_BL_GPIO, LCD_BL_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(LCD_BL_GPIO, LCD_BL_PIN);
    }
}

/**
 * @brief  设置显示方向
 * @param  direction 方向
 * @return 
 */
void LCD_Direction_Set(lcd_direction_t direction)
{
    lcd_direction = direction;
    switch (direction)
    {
    case LCD_HORIZON:
        st7789_reg_write(0x36, (uint8_t []){0x60}, 1);
        LCD_WIDTH  = 240;
        LCD_HEIGHT = 135;
        break;
    case LCD_VERTICAL:
        st7789_reg_write(0x36, (uint8_t []){0x00}, 1);
        LCD_WIDTH  = 135;
        LCD_HEIGHT = 240;
        break;
    case LCD_HORIZON_INVERT:
        st7789_reg_write(0x36, (uint8_t []){0xA0}, 1);
        LCD_WIDTH  = 240;
        LCD_HEIGHT = 135;
        break;
    case LCD_VERTICAL_INVERT:
        st7789_reg_write(0x36, (uint8_t []){0xC0}, 1);
        LCD_WIDTH  = 135;
        LCD_HEIGHT = 240;
        break;
    default:
        break;
    }

    // 刷新显示
    LCD_Fill(0, 0, LCD_WIDTH, LCD_HEIGHT, COLOR_BACKGROUND);
    LCD_Scroll(1000000);
    LCD_Print("");
}

/**
 * @brief  填充矩形区域
 * @param  x 左上坐标x
 * @param  y 左上坐标y
 * @param  width  像素宽
 * @param  height 像素高
 * @param  color  颜色
 * @return 
 */
void LCD_Fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    st7789_area_set(x, y, width, height);
    for(int i = 0; i < width * height; i++)
    {
        st7789_data_write(color);
    }
}

/**
 * @brief  绘制一个ASCII字符
 * @param  code  ASCII字符 
 * @param  x     左上坐标x
 * @param  y     左上坐标y
 * @param  font  字号
 * @param  color 颜色
 * @return 可显示的字符或0
 */
char LCD_Char(char code, uint16_t x, uint16_t y, uint8_t font, uint16_t color)
{
    if (code < ' ' || code > '~')
    {
        return '\0';
    }

    // 支持4种字号(12、16、24、32)
    const uint8_t *data = NULL;
    int index = code - ' ';
    if (font < 14)      // 字号12
    {
        font = 12;
        data = ascii_font_1206[index];
    }
    else if (font < 20) // 字号16
    {
        font = 16;
        data = ascii_font_1608[index];
    }
    else if (font < 28) // 字号24
    {
        font = 24;
        data = ascii_font_2412[index];
    }
    else                // 字号32
    {
        font = 32;
        data = ascii_font_3216[index];
    }

    // 绘制点阵
    st7789_area_set(x, y, font / 2, font);
    for(int i = 0; i < font; i++)
    {
        for(int j = 0; j < font / 2; j++)
        {
            st7789_data_write(data[(font + 7) / 8 * j + i / 8] & (0x80 >> (i % 8)) ? color : COLOR_BACKGROUND);
        }
    }

    return code;
}

/**
 * @brief  显示文本
 * @param  scroll 滚动行数(>0向下滚动 <0向上滚动)
 * @return 
 */
void LCD_Scroll(int scroll)
{
    // 更新当前显示行号
    const int SCREEN_ROWS = LCD_HEIGHT / FONT;  // 屏幕可显示行数
    scroll = text_line_cur + scroll;
    if (text_line_cnt <= SCREEN_ROWS || scroll <= 0)
    {
        text_line_cur = 0;
    }
    else if (scroll > (text_line_cnt - SCREEN_ROWS))
    {
        text_line_cur = text_line_cnt - SCREEN_ROWS;
    }
    else
    {
        text_line_cur = scroll;
    }

    // 刷新当前帧字符
    const int ROW_CHARS = (LCD_WIDTH - SCROLL_WIDTH - 1) / (FONT / 2);   // 每行字符数
    memset(text_frame, 0, SCREEN_ROWS * ROW_CHARS);
    int char_cnt = 0;
    int line_cnt = -text_line_cur;
    for(int i = 0; i < sizeof(text_buffer); i++)
    {
        if (text_buffer[i] == '\0')
        {
            break;
        }
        else if ((text_buffer[i] >= ' ' && text_buffer[i] <= '~') || text_buffer[i] == '\t')
        {
            if (line_cnt >= SCREEN_ROWS)
            {
                break;
            }
            if (text_buffer[i] == '\t')
            {
                int tab_width = 4 - char_cnt % 4;   // Tab 4个字符对齐
                if (line_cnt >= 0)
                {
                    for(int v = 0; v < tab_width; v++)
                    {
                        text_frame[ROW_CHARS * line_cnt + char_cnt + v] = ' ';
                    }
                }
                char_cnt += tab_width;
            }
            else
            {
                if (line_cnt >= 0)
                {
                    text_frame[ROW_CHARS * line_cnt + char_cnt] = text_buffer[i];
                }
                char_cnt++;
            }
            if (char_cnt >= ROW_CHARS)
            {
                char_cnt = 0;
                line_cnt++;
            }
        }
        else if (text_buffer[i] == '\n')
        {
            char_cnt = 0;
            line_cnt++;
        }
    }

    // 绘制字符
    for(int y = 0; y < SCREEN_ROWS; y++)
    {
        for(int x = 0; x < ROW_CHARS; x++)
        {
            if (text_frame[ROW_CHARS * y + x])
            {
                LCD_Char(text_frame[ROW_CHARS * y + x], FONT / 2 * x, FONT * y, FONT, COLOR_TEXT);
            }
            else
            {
                LCD_Fill(FONT / 2 * x, FONT * y, FONT / 2 * (ROW_CHARS - x), FONT, COLOR_BACKGROUND);
                break;
            }
        }
    }

    // 绘制滚动条
    if (text_line_cnt > SCREEN_ROWS)
    {
        static int size_last = 0;
        static int pos_last  = 0;
        int size = LCD_HEIGHT * SCREEN_ROWS / text_line_cnt;
        int pos  = (LCD_HEIGHT - size) * text_line_cur / (text_line_cnt - SCREEN_ROWS);
        LCD_Fill(LCD_WIDTH - SCROLL_WIDTH, pos_last, SCROLL_WIDTH, size_last, COLOR_BACKGROUND);
        LCD_Fill(LCD_WIDTH - SCROLL_WIDTH, pos, SCROLL_WIDTH, size, COLOR_SCROLLBAR);
        size_last = size;
        pos_last  = pos;
    }
    else
    {
        LCD_Fill(LCD_WIDTH - SCROLL_WIDTH, 0, SCROLL_WIDTH, LCD_HEIGHT, COLOR_BACKGROUND);
    }
}

/**
 * @brief  显示字符串
 * @param  *format 格式字符串
 * @return 
 */
int LCD_Print(const char *format, ...)
{
    // 添加文本至显示缓冲区
    static int text_length = 0;
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(text_buffer + text_length, sizeof(text_buffer) - text_length, format, ap);
    va_end(ap);
    if (len > (sizeof(text_buffer) - text_length - 1))
    {
        memset(text_buffer, 0, sizeof(text_buffer));
        text_length = 0;
        va_start(ap, format);
        len = vsprintf(text_buffer, format, ap);
        va_end(ap);
    }
    text_length += len;

    // 判断当前是否显示的最新文本, 否则不刷新显示
    const int SCREEN_ROWS = LCD_HEIGHT / FONT;  // 屏幕可显示行数
    int display_flag = text_line_cur >= (text_line_cnt - SCREEN_ROWS);

    // 计算行数
    const int ROW_CHARS = (LCD_WIDTH - SCROLL_WIDTH - 1) / (FONT / 2); // 每行字符数
    int char_cnt = 0;
    text_line_cnt = 0;
    for(int i = 0; i < text_length; i++)
    {
        if ((text_buffer[i] >= ' ' && text_buffer[i] <= '~') || text_buffer[i] == '\t')
        {
            if (text_buffer[i] == '\t')
            {
                char_cnt += 4 - char_cnt % 4;   // Tab宽度4个字符
            }
            else
            {
                char_cnt++;
            }
            if (char_cnt >= ROW_CHARS)
            {
                char_cnt = 0;
                text_line_cnt++;
            }
        }
        else if (text_buffer[i] == '\n')
        {
            char_cnt = 0;
            text_line_cnt++;
        }
    }

    // 绘制
    if (display_flag)
    {
        LCD_Scroll(text_line_cnt - SCREEN_ROWS - text_line_cur);
    }
    else
    {
        LCD_Scroll(0);  // 用于刷新滚动条
    }

    return len;
}

/**
 * @brief  ST7789写命令
 * @param  reg    寄存器
 * @param  *data  数据
 * @param  length 数据字节长度
 * @return 
 */
static void st7789_reg_write(uint8_t reg, uint8_t *data, uint32_t length)
{
    // 写命令
    LL_GPIO_ResetOutputPin(LCD_DCS_GPIO, LCD_DCS_PIN);
    (void)spi_byte_write_read(reg);
    // 写数据
    for(int i = 0; i< length; i++)
    {
        LL_GPIO_SetOutputPin(LCD_DCS_GPIO, LCD_DCS_PIN);
        (void)spi_byte_write_read(data[i]);
    }
}

/**
 * @brief  ST7789写数据
 * @param  data 数据
 * @return 
 */
static void st7789_data_write(uint16_t data)
{
    LL_GPIO_SetOutputPin(LCD_DCS_GPIO, LCD_DCS_PIN);
    (void)spi_byte_write_read(data >> 8);
    (void)spi_byte_write_read(data);
}

/**
 * @brief  ST7789设置显示区域
 * @param  x      左上坐标x
 * @param  y      左上坐标y
 * @param  width  区域宽度
 * @param  height 区域高度
 * @return 
 */
static void st7789_area_set(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    // 列地址
    st7789_reg_write(0x2A, NULL, 0);
    st7789_data_write(x + OFFSET_X[lcd_direction]);
    st7789_data_write(x + OFFSET_X[lcd_direction] + width - 1);
    // 行地址
    st7789_reg_write(0x2B, NULL, 0);
    st7789_data_write(y + OFFSET_Y[lcd_direction]);
    st7789_data_write(y + OFFSET_Y[lcd_direction] + height - 1);
    // 开始传输像素数据
    st7789_reg_write(0x2C, NULL, 0);
}


