#ifndef __OLED_H
#define __OLED_H

#include "stm32f4xx_hal.h"
#include "fonts.h"
#include "i2c.h"

#define OLED_I2C_ADDR 0x78
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

typedef enum {
    White = 0x00,
    Black = 0x01
} OLED_COLOR;

typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} OLED_t;

uint8_t oled_Start(void);
void fillOledScreenWithColor(OLED_COLOR color);
void screen_update(void);
void draw_pixel(uint8_t x, uint8_t y, OLED_COLOR color);
char write_char(char ch, FontDef Font, OLED_COLOR color);
char string_write(char* str, FontDef Font, OLED_COLOR color);
void set_cursor(uint8_t x, uint8_t y);

#endif
