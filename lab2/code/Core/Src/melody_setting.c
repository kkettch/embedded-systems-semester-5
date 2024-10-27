#include <melody_setting.h>
#include "tim.h"


void Melody_Init (void) {
	HAL_TIM_OC_Start(&htim2,TIM_CHANNEL_1);
	HAL_TIM_PWM_Init(&htim2);
}

void Melody_Set_Freq (uint16_t freq) {
	TIM2->PSC = ((2 * HAL_RCC_GetPCLK1Freq()) / (2 * MELODY_VOLUME_MAX * freq)) - 1;
}

void Melody_Set_Volume (uint16_t volume) {
	if (volume > MELODY_VOLUME_MAX)
		volume = MELODY_VOLUME_MAX;
	TIM2->CCR1 = volume;
}

void Melody_Play (uint32_t* melody, uint32_t* delays, uint16_t len) {
	for(int i = 0; i < len; i++) {
	  	if (melody[i] != 0) {
	  		Melody_Set_Freq(melody[i]);
	  		Melody_Set_Volume(MELODY_VOLUME_MAX);
	  		HAL_Delay(1920/delays[i]);
	  		Melody_Set_Volume(MELODY_VOLUME_MUTE);
	  	} else HAL_Delay(1920/delays[i]);
	  	HAL_Delay(10);
	}
}
