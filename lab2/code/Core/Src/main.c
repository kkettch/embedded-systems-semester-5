#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "melody_setting.h"
#include "oled.h"
#include "keyboard.h"
#include "sdk_uart.h"

void SystemClock_Config(void);

void play_symphony( void )
{
	int length_of_melody;

	uint32_t symphony_notes[] = {N_DS5, N_F5, N_G5, N_AS5, N_G5, N_AS5, N_C6, N_DS6, N_F6, N_F6, N_DS6};
	uint32_t symphony_duration[] = {8, 8, 8, 8, 8, 8, 8, 8, 5, 5, 5};
	length_of_melody = sizeof(symphony_notes) / sizeof(uint32_t);

	Melody_Play(symphony_notes, symphony_duration, length_of_melody);
}

void oled_reset(void)
{
	fillOledScreenWithColor(Black);
	set_cursor(0, 0);
	screen_update();
}

void get_timer_array(uint8_t timer_array[2], uint16_t timer) {
	size_t i;

	i = 0;
	while (i < 2)
	{
		if (timer != 0)
			timer_array[i] = timer % 10 + 0x30;
		timer /= 10;
		i++;
	}
}


void oled_print_timer(uint16_t timer)
{
	uint8_t i = 0;
    uint8_t timer_array[2] = {0, 0};

    if ((timer + 1) % 10 == 0)
        oled_reset();
    set_cursor(60, 28);
    if (timer == 0)
    	write_char('0', Font_7x10, White);
    else
    {
    	get_timer_array(timer_array, timer);
    	while (i < 2) {
			if (timer_array[1 - i] != 0) {
				write_char(timer_array[1 - i], Font_7x10, White);
			}
			i++;
		}
    }
    screen_update();
}

void start_timer( void )
{
	int is_button_pusshed;
	uint8_t i;
	uint16_t timer;
	uint8_t button;
	uint8_t Row[4] = {ROW4, ROW3, ROW2, ROW1};

	UART_Transmit( (uint8_t*)"Start keyboard\n" );
	oled_reset();
	string_write("Enter value:", Font_7x10, White);
	screen_update();

	is_button_pusshed = 0;
	timer = 0;
	while (!is_button_pusshed)
	{
		i = 0;
		while (i < 4)
		{
			button = check_nrow(Row[i]);
			if (button == 0xC)
			{
				timer /= 10;
				oled_reset();
				string_write("Enter value:", Font_7x10, White);
				oled_print_timer(timer);
				HAL_Delay(100);
			}
			else if (button == 0xB && timer < 10)
			{
				timer = timer * 10;
				oled_print_timer(timer);
				HAL_Delay(100);
			}
			else if (button == 0xA)
			{
				is_button_pusshed = 1;
				oled_print_timer(timer);
				break;
			}
			else if (button != 0 && timer < 10)
			{
				timer = timer * 10 + button;
				oled_print_timer(timer);
				HAL_Delay(100);
			}
			i++;
		}
	}
	oled_reset();
	while (timer > 0)
	{
		oled_print_timer(timer);
		HAL_Delay(1000);
		timer -= 1;
	}
	oled_print_timer(0);
	HAL_Delay(1000);

	oled_reset();
	set_cursor(0, 15);
	string_write("I just wanna be", Font_7x10, White);
	set_cursor(0, 25);
	string_write("a part of your", Font_7x10, White);
	set_cursor(0, 35);
	string_write("symphony", Font_7x10, White);
	screen_update();
	play_symphony();
	oled_reset();
}

int main(void)
{
	HAL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_I2C1_Init();
	MX_USART6_UART_Init();

	oled_Start();
	Melody_Init();

	while (1)
	{
		start_timer();
		HAL_Delay(500);
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}



/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
