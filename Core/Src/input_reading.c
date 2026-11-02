#include "input_reading.h"
#include "lab3_config.h"

typedef struct {
    GPIO_PinState stable;
    GPIO_PinState candidate;
    uint16_t candidate_time_ms;
    volatile uint8_t pending_press;
} Button;

static Button buttons[3];
static const uint16_t button_pins[3] = {GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15};

void buttons_init(void) {
    for (uint8_t i = 0; i < 3u; ++i) {
        GPIO_PinState state = HAL_GPIO_ReadPin(GPIOB, button_pins[i]);
        buttons[i].stable = state;
        buttons[i].candidate = state;
        buttons[i].candidate_time_ms = 0u;
        buttons[i].pending_press = 0u;
    }
}

/* Run from TIM2 ISR. Stable transition is accepted only after at least
 * two samples and a continuous delay of LAB3_DEBOUNCE_MS. Active low. */
void buttons_timer_tick(uint16_t tick_ms) {
    for (uint8_t i = 0; i < 3u; ++i) {
        Button *b = &buttons[i];
        GPIO_PinState raw = HAL_GPIO_ReadPin(GPIOB, button_pins[i]);
        if (raw != b->candidate) {
            b->candidate = raw;
            b->candidate_time_ms = 0u;
        } else if (b->stable != b->candidate) {
            uint32_t total = (uint32_t)b->candidate_time_ms + tick_ms;
            b->candidate_time_ms = (total >= 65535u) ? 65535u : (uint16_t)total;
            if (b->candidate_time_ms >= LAB3_DEBOUNCE_MS) {
                b->stable = b->candidate;
                if (b->stable == GPIO_PIN_RESET && b->pending_press < 255u) {
                    ++b->pending_press;
                }
            }
        }
    }
}

/* Consume the event atomically between foreground code and timer ISR. */
uint8_t button_take_press(uint8_t index) {
    if (index >= 3u) return 0u;
    uint8_t got = 0u;
    __disable_irq();
    if (buttons[index].pending_press != 0u) {
        --buttons[index].pending_press;
        got = 1u;
    }
    __enable_irq();
    return got;
}
