#ifndef INPUT_READING_H
#define INPUT_READING_H
#include "main.h"
#define BUTTON_MODE 0u
#define BUTTON_INC  1u
#define BUTTON_SET  2u
void buttons_init(void);
void buttons_timer_tick(uint16_t tick_ms);
uint8_t button_take_press(uint8_t index);
#endif
