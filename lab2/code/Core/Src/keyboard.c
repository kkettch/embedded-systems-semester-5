#include "main.h"
#include "pca9538.h"
#include "keyboard.h"
#include "sdk_uart.h"
#include "usart.h"

#define KBRD_ADDR 0xE2
#define CELL_1 0x01
#define CELL_4 0x04
#define CELL_7 0x07
#define CELL_10 0xA

#define CELL_2 0x02
#define CELL_5 0x05
#define CELL_8 0x08
#define CELL_11 0xB

#define CELL_3 0x03
#define CELL_6 0x06
#define CELL_9 0x09
#define CELL_12 0xC


HAL_StatusTypeDef init_kb( void ) {
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t buf;

	buf = 0;
	ret = PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf);
	if( ret != HAL_OK ) {
		UART_Transmit((uint8_t*)"Error write polarity\n");
		return ret;
	}

	buf = 0;
	ret = PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf);
	if( ret != HAL_OK ) {
		UART_Transmit((uint8_t*)"Error write output\n");
	}

	return ret;

//exit:
//	return ret;
}

uint8_t check_nrow( uint8_t  Nrow ) {
	uint8_t Nkey = 0x00;
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t buf;
	uint8_t kbd_in;

	ret = init_kb();
	if( ret != HAL_OK ) {
		UART_Transmit((uint8_t*)"Error write init\n");
	}

	buf = Nrow;
	ret = PCA9538_Write_Register(KBRD_ADDR, CONFIG, &buf);
	if( ret != HAL_OK ) {
		UART_Transmit((uint8_t*)"Error write config\n");
	}

	ret = PCA9538_Read_Inputs(KBRD_ADDR, &buf);
	if( ret != HAL_OK ) {
		UART_Transmit((uint8_t*)"Read error\n");
	}

	kbd_in = buf & 0x70;
	Nkey = kbd_in;
	if( kbd_in != 0x70) {
		if( !(kbd_in & 0x10) ) {
			if (Nrow == ROW1){
				Nkey = CELL_1;
			} else if (Nrow == ROW2){
				Nkey = CELL_4;
			} else if (Nrow == ROW3){
				Nkey = CELL_7;
			} else if (Nrow == ROW4){
				Nkey = CELL_10;
			}
		}
		if( !(kbd_in & 0x20) ) {
			if (Nrow == ROW1){
				Nkey = CELL_2;
			} else if (Nrow == ROW2){
				Nkey = CELL_5;
			} else if (Nrow == ROW3){
				Nkey = CELL_8;
			} else if (Nrow == ROW4){
				Nkey = CELL_11;
			}
		}
		if( !(kbd_in & 0x40) ) {
			if (Nrow == ROW1){
				Nkey = CELL_3;
			} else if (Nrow == ROW2){
				Nkey = CELL_6;
			} else if (Nrow == ROW3){
				Nkey = CELL_9;
			} else if (Nrow == ROW4){
				Nkey = CELL_12;
			}
		}
	}
	else Nkey = 0x00;

	return Nkey;
}
