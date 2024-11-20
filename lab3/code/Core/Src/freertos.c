#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "oled.h"
#include "kb.h"
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

typedef struct snake {
	struct snake *prev;
	struct snake *next;
	uint32_t x;
	uint32_t y;
} snake;

enum direction {
	LEFT,
	UP,
	RIGHT,
	DOWN
};

uint32_t random_seed();

uint32_t seed = 326443256;
uint32_t A = 2345345, C = 54217867, M = 4294967296;
uint8_t direction = LEFT;

osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osMessageQId myQueue01Handle;

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);

void MX_FREERTOS_Init(void);

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void MX_FREERTOS_Init(void) {

  osMessageQDef(myQueue01, 16, uint16_t);
  myQueue01Handle = osMessageCreate(osMessageQ(myQueue01), NULL);

  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  osThreadDef(myTask02, StartTask02, osPriorityNormal, 0, 128);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);



}

void StartDefaultTask(void const * argument)
{

	oled_Init();


	uint32_t width_screen = 128;
	uint32_t height_screen = 64;
	uint32_t width_snake = 8;
	uint32_t height_snake = 8;
	uint32_t x_cell = width_screen / width_snake;
	uint32_t y_cell = height_screen / height_snake;
	osEvent event;
	int global_flag = 0;
	oled_SetCursor(10, 15);
	oled_WriteString("SNAKE GAME!", Font_11x18, White);
	oled_SetCursor(15, 35);
	oled_WriteString("Press to start", Font_7x10, White);
	oled_UpdateScreen();
	osDelay(100);

	int flag = 0;
	uint8_t Row[4] = {ROW4, ROW3, ROW2, ROW1};
	while (flag == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			if (Check_Row( Row[i] ) != 0x00)
				flag = 1;
		}
	}

	for (;;)
	{

		// Получение нажатий с клавиатуры
		event = osMessageGet(myQueue01Handle, 100);
		while (event.status == osEventMessage)
		{
			event = osMessageGet(myQueue01Handle, 100);
		}

		// Обновление экрана при запуске
		oled_Reset();

		// Инициализация головы змейки
		snake *head = (snake*) pvPortMalloc(sizeof(snake));
		head->x = x_cell/2;
		head->y = y_cell/2;
		snake *tail = head;

		// Заполнение структуры данных змейки ее частями тела
		for (int i = 1; i < 3; i++)
		{
			snake *node = (snake*) pvPortMalloc(sizeof(snake));
			node->next = tail;
			node->x = x_cell/2 + i;
			node->y = y_cell/2;
			tail->prev = node;
			tail = node;
		}

		// Отрисовка всех частей змейки
		snake *current_node = head;
		while (current_node != NULL)
		{
			uint16_t x1 = current_node->x * width_snake;
			uint16_t y1 = current_node->y * height_snake;
			uint16_t x2 = x1 + width_snake;
			uint16_t y2 = y1 + height_snake;

			oled_DrawFilledSquare(x1, x2, y1, y2, White);

			current_node = current_node->prev;
		}
		oled_UpdateScreen();
		osDelay(100);


		int16_t x_Apple = -1;
		int16_t y_Apple = -1;
		uint8_t A = 1664525;

		for (;;)
		{
			// Получение нажатие с клавиатуры
			event = osMessageGet(myQueue01Handle, 100);

			if (event.status == osEventMessage)
			{
				direction = event.value.v;
			}

			//
			snake * node = (snake*) pvPortMalloc(sizeof(snake));
			node->x = head->x;
			node->y = head->y;

			// TODO: if instead of switch

			switch (direction)
			{
				case DOWN:
					node->y += 1;
					break;
				case UP:
					node->y -= 1;
					break;
				case RIGHT:
					node->x += 1;
					break;
				case LEFT:
					node->x -= 1;
					break;
			}

			//
			head->next = node;
			node->prev = head;
			head = node;
			uint16_t x1 = head->x * width_snake;
			uint16_t y1 = head->y * height_snake;
			uint16_t x2 = x1 + width_snake;
			uint16_t y2 = y1 + height_snake;
			oled_DrawFilledSquare(x1, x2, y1, y2, White);

			// Если съели яблоко, то убираем его с экрана, иначе просто двигаем змейку
			if (head->x == x_Apple && head->y == y_Apple) {
				x_Apple = -1;
				y_Apple = -1;
			} else {
				x1 = tail->x * width_snake;
				y1 = tail->y * height_snake;
				x2 = x1 + width_snake;
				y2 = y1 + height_snake;
				oled_DrawFilledSquare(x1, x2, y1, y2, Black);

				tail = tail->next;
				vPortFree(tail->prev);
				tail->prev = NULL;
			}

			// Если врезалась в стену то GameOver
			if (head->x >= x_cell || head->y >= y_cell || head->x < 0 || head->y < 0) {
				oled_Reset();
				oled_SetCursor(15, 20);
				oled_WriteString("Game Over!", Font_11x18, White);
				oled_SetCursor(75, 50);
				oled_WriteString("Reset", Font_7x10, White);
				oled_UpdateScreen();
				osDelay(100);
				while (Check_Row( ROW4 ) != 0x01)
					continue;
				break;
			}

			snake *current_node = head->prev;
			uint8_t flag = 0x00;

			// Смейка врезалась в саму себя GameOver
			while (current_node != NULL) {
				if (head->x == current_node->x && head->y == current_node->y) {
					flag = 0x01;
					break;
				}
				current_node = current_node->prev;
			}
			if (flag == 0x01) {
				oled_Reset();
				oled_SetCursor(15, 20);
				oled_WriteString("Game Over!", Font_11x18, White);
				oled_SetCursor(75, 50);
				oled_WriteString("Reset", Font_7x10, White);
				oled_UpdateScreen();
				osDelay(100);
				while (Check_Row( ROW4 ) != 0x01)
					continue;
				break;
			}


			if (x_Apple == -1 && y_Apple == -1) {
				for (;;) {
					x_Apple = (int16_t) random_seed() % (x_cell - 1);
					y_Apple = (int16_t) random_seed() % (y_cell - 1);
					current_node = head;
					flag = 0x00;
					while (current_node != NULL) {
						if (x_Apple == current_node->x && y_Apple == current_node->y) {
							flag = 0x01;
							break;
						}
						current_node = current_node->prev;
					}
					if (flag == 0x00) {
						break;
					}
				}

			}
			oled_DrawSquare(x_Apple * width_snake, (x_Apple + 1) * width_snake,
					y_Apple * height_snake, (y_Apple + 1) * height_snake, White);

			oled_UpdateScreen();
			osDelay(100);
		}

		current_node = head;
		while (current_node != NULL) {
			uint16_t x1 = current_node->x * width_snake, y1 = current_node->y * height_snake;
			uint16_t x2 = x1 + width_snake, y2 = y1 + height_snake;
			oled_DrawFilledSquare(x1, x2, y1, y2, Black);
			oled_UpdateScreen();
			osDelay(100);
			current_node = current_node->prev;

		}
		if (global_flag)
			break;
		direction = LEFT;
	}
}

void StartTask02(void const * argument)
{
	for (;;) {
		uint8_t Row[4] = {ROW4, ROW3, ROW2, ROW1};
		for ( int i = 1; i < 4; i++ ) {
			uint8_t Key = Check_Row( Row[i] );
			if (i == 1 && Key == 0x02 && direction != UP && direction != DOWN) {
				direction = DOWN;
				osMessagePut(myQueue01Handle, direction, 100);
			} else if ( i == 2 && Key == 0x04 && direction != RIGHT && direction != LEFT) {
				direction = LEFT;
				osMessagePut(myQueue01Handle, direction, 100);
			} else if ( i == 2 && Key == 0x01 && direction != LEFT && direction != RIGHT) {
				direction = RIGHT;
				osMessagePut(myQueue01Handle, direction, 100);
			} else if ( i == 3 && Key == 0x02 && direction != DOWN && direction != UP) {
				direction = UP;
				osMessagePut(myQueue01Handle, direction, 100);
			}

		}
		osDelay(10);
	}
}

uint32_t random_seed() {
	seed = (A * seed + C) % M;
	return seed;
}
