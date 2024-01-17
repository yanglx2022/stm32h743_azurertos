/**
 * @brief   : LCD驱动
 * @author  : yanglx2022
 * @version : V0.1.0
 * @date    : 2024-01-08
 * @note    : 
 */

#ifndef LCD_H
#define LCD_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// RGB565
#define RGB565(r, g, b)                 (((b) >> 3)|(((g) >> 2) << 5)|(((r) >> 3) << 11))

#define COLOR_BLACK                     RGB565(0, 0, 0)         // 黑色
#define COLOR_WHITE                     RGB565(255, 255, 255)   // 白色
#define COLOR_GRAY                      RGB565(128, 128, 128)   // 灰色
#define COLOR_DIM_GRAY                  RGB565(105, 105, 105)   // 暗灰色
#define COLOR_RED                       RGB565(255, 0, 0)       // 红色
#define COLOR_GREEN                     RGB565(0, 255, 0)       // 绿色
#define COLOR_BLUE                      RGB565(0, 0, 255)       // 蓝色
#define COLOR_CORN_FLOWER_BLUE          RGB565(100, 149, 237)   // 矢车菊蓝
#define COLOR_DODGER_BLUE               RGB565(30, 144, 255)    // 道奇蓝
#define COLOR_LIGHT_BLUE                RGB565(173, 216, 230)   // 浅蓝


// 显示方向
typedef enum
{
    LCD_HORIZON = 0,    // 横屏显示
    LCD_VERTICAL,       // 竖屏显示
    LCD_HORIZON_INVERT, // 横屏转180°
    LCD_VERTICAL_INVERT // 竖屏转180°
} lcd_direction_t;

extern uint16_t LCD_WIDTH;
extern uint16_t LCD_HEIGHT;

void LCD_Init(void);
void LCD_Enable(uint8_t enable);
void LCD_Direction_Set(lcd_direction_t direction);
void LCD_Fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
char LCD_Char(char code, uint16_t x, uint16_t y, uint8_t font, uint16_t color);
void LCD_Scroll(int scroll);
int  LCD_Print(const char *format, ...);


#ifdef __cplusplus
}
#endif
#endif /* LCD_H */


