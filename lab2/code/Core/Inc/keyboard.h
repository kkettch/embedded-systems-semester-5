#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7


uint8_t check_nrow( uint8_t  Nrow );
HAL_StatusTypeDef init_kb( void );

#endif
