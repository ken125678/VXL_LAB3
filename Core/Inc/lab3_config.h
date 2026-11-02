#ifndef LAB3_CONFIG_H
#define LAB3_CONFIG_H
/* Clock source is HSI = 8 MHz. Timer 2 is a 10 ms input-sampling timer.
 * Change LAB3_TIM2_TICK_MS to 1 or 100 and rebuild to study Exercise 4.
 * Display scanning (TIM3) and LED blinking (HAL_GetTick) are independent. */
#ifndef LAB3_TIM2_TICK_MS
#define LAB3_TIM2_TICK_MS      10u
#endif
#define LAB3_DEBOUNCE_MS       20u
#define LAB3_RED_DEFAULT    3u
#define LAB3_AMBER_DEFAULT  1u
#define LAB3_GREEN_DEFAULT  2u
#endif
