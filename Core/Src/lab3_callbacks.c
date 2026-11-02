#include "main.h"
#include "lab3_config.h"
#include "input_reading.h"
#include "output_display.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        buttons_timer_tick((uint16_t)LAB3_TIM2_TICK_MS);
		} else if (htim->Instance == TIM3) {
			output_display_scan_1ms();
    }
}
