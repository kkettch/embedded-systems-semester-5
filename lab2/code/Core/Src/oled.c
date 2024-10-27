#include "oled.h"
#include "fonts.h"
#include "i2c.h"

static uint8_t OLED_Buffer[OLED_WIDTH * OLED_HEIGHT / 8];
static OLED_t OLED;

static void configOled(uint8_t command) {
    HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, 0x00, 1, &command, 1, 10);
}


void fillOledScreenWithColor(OLED_COLOR color) {
	uint32_t i = 0;
	while (i < sizeof(OLED_Buffer)){
		OLED_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
		i++;
	}
}

uint8_t oled_Start(void) {
    HAL_Delay(100);


    configOled(0xAE);
    configOled(0x20);
    configOled(0x10);
    configOled(0xB0);
    configOled(0xC8);
    configOled(0x00);
    configOled(0x10);
    configOled(0x40);
    configOled(0x81);
    configOled(0xFF);
    configOled(0xA1);
    configOled(0xA6);
    configOled(0xA8);
    configOled(0x3F);
    configOled(0xA4);
    configOled(0xD3);
    configOled(0x00);
    configOled(0xD5);
    configOled(0xF0);
    configOled(0xD9);
    configOled(0x22);
    configOled(0xDA);
    configOled(0x12);
    configOled(0xDB);
    configOled(0x20);
    configOled(0x8D);
    configOled(0x14);
    configOled(0xAF);


    fillOledScreenWithColor(Black);
    screen_update();

    OLED.CurrentX = 0;
    OLED.CurrentY = 0;
    OLED.Initialized = 1;

    return 1;
}



void screen_update(void) {
	uint8_t i = 0;
	while (i < 8){
		configOled(0xB0 + i);
		configOled(0x00);
		configOled(0x10);
		HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, 0x40, 1, &OLED_Buffer[OLED_WIDTH * i], OLED_WIDTH, 100);
		i++;
	}
}

void draw_pixel(uint8_t x, uint8_t y, OLED_COLOR color) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) {
        return;
    }


    if (OLED.Inverted) {
        color = (OLED_COLOR)!color;
    }


    if (color == White) {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= 1 << (y % 8);
    } else {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

char write_char(char ch, FontDef Font, OLED_COLOR color) {
    uint32_t i, b, j;


    if (OLED_WIDTH <= (OLED.CurrentX + Font.FontWidth) ||
        OLED_HEIGHT <= (OLED.CurrentY + Font.FontHeight)) {
        return 0;
    }


    i = 0;
    while (i < Font.FontHeight) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        j = 0;
        while (j < Font.FontWidth) {
            if ((b << j) & 0x8000) {
                draw_pixel(OLED.CurrentX + j, (OLED.CurrentY + i), color);
            } else {
                draw_pixel(OLED.CurrentX + j, (OLED.CurrentY + i), (OLED_COLOR)!color);
            }
            j++;
        }
        i++;
    }




    OLED.CurrentX += Font.FontWidth;

    return ch;
}

char string_write(char* str, FontDef Font, OLED_COLOR color) {
    while (*str) {
        if (write_char(*str, Font, color) != *str) {
            return *str;
        }
        str++;
    }

    return *str;
}

void set_cursor(uint8_t x, uint8_t y) {
    OLED.CurrentX = x;
    OLED.CurrentY = y;
}
