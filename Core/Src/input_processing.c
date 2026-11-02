#include "main.h"
#include "input_reading.h"
#include "input_processing.h"
#include "output_display.h"
#include "lab3_config.h"

typedef enum { MODE_NORMAL = 1, MODE_RED = 2,
               MODE_AMBER = 3, MODE_GREEN = 4 } AppMode;
typedef enum { MAIN_GREEN, MAIN_AMBER, CROSS_GREEN, CROSS_AMBER } Phase;
typedef enum { LIGHT_RED, LIGHT_AMBER, LIGHT_GREEN } Light;

static AppMode mode = MODE_NORMAL;
static Phase phase = MAIN_GREEN;
static uint8_t red_seconds = LAB3_RED_DEFAULT;
static uint8_t amber_seconds = LAB3_AMBER_DEFAULT;
static uint8_t green_seconds = LAB3_GREEN_DEFAULT;
static uint8_t candidate_seconds = LAB3_RED_DEFAULT;
static uint32_t phase_start_ms = 0u;
static uint32_t last_draw_ms = 0u;

/* Red on main road is divided into cross green and cross amber periods.
 * If main red is only one second, cross amber is necessarily omitted. */
static uint8_t cross_amber_seconds(void) {
    if (red_seconds <= 1u) return 0u;
    uint8_t maximum = (uint8_t)(red_seconds - 1u);
    return amber_seconds < maximum ? amber_seconds : maximum;
}
static uint8_t cross_green_seconds(void) {
    return (uint8_t)(red_seconds - cross_amber_seconds());
}
static uint32_t phase_duration_ms(Phase p) {
    switch (p) {
        case MAIN_GREEN: return (uint32_t)green_seconds * 1000u;
        case MAIN_AMBER: return (uint32_t)amber_seconds * 1000u;
        case CROSS_GREEN: return (uint32_t)cross_green_seconds() * 1000u;
        default: return (uint32_t)cross_amber_seconds() * 1000u;
    }
}
static uint8_t seconds_remaining(uint32_t remaining_ms) {
    uint32_t seconds = (remaining_ms + 999u) / 1000u;
    return (seconds > 99u) ? 99u : (uint8_t)seconds;
}
static void change_mode(uint32_t now) {
    mode = (mode == MODE_GREEN) ? MODE_NORMAL : (AppMode)(mode + 1);
    if (mode == MODE_NORMAL) {
        phase = MAIN_GREEN;
        phase_start_ms = now;
    } else if (mode == MODE_RED) {
        candidate_seconds = red_seconds;
    } else if (mode == MODE_AMBER) {
        candidate_seconds = amber_seconds;
    } else {
        candidate_seconds = green_seconds;
    }
}
static void apply_candidate(void) {
    if (mode == MODE_RED) red_seconds = candidate_seconds;
    else if (mode == MODE_AMBER) amber_seconds = candidate_seconds;
    else if (mode == MODE_GREEN) green_seconds = candidate_seconds;
}
static void update_traffic_fsm(uint32_t now) {
    /* When another phase is due, advance. Also safely skip a zero-length
       CROSS_AMBER phase if red_seconds == 1. */
    for (uint8_t limit = 0u; limit < 8u; ++limit) {
        uint32_t duration = phase_duration_ms(phase);
        if (duration > 0u && (uint32_t)(now - phase_start_ms) < duration) break;
        phase_start_ms += duration;
        phase = (Phase)(((uint8_t)phase + 1u) % 4u);
    }
}
static uint16_t pins_for_light(Light l, uint8_t road) {
    /* Each road has two identical lamps, on both sides of intersection. */
    static const uint16_t on_main[3] = {
        GPIO_PIN_0 | GPIO_PIN_3, GPIO_PIN_1 | GPIO_PIN_4,
        GPIO_PIN_2 | GPIO_PIN_5
    };
    static const uint16_t on_cross[3] = {
        GPIO_PIN_6 | GPIO_PIN_9, GPIO_PIN_7 | GPIO_PIN_10,
        GPIO_PIN_8 | GPIO_PIN_11
    };
    return road == 0u ? on_main[l] : on_cross[l];
}
static void draw_traffic_lights(uint32_t now) {
    const uint16_t all_lights = (uint16_t)0x0FFFu;
    uint16_t active = 0u;
    if (mode == MODE_NORMAL) {
        if (phase == MAIN_GREEN) {
            active = pins_for_light(LIGHT_GREEN, 0u) | pins_for_light(LIGHT_RED, 1u);
        } else if (phase == MAIN_AMBER) {
            active = pins_for_light(LIGHT_AMBER, 0u) | pins_for_light(LIGHT_RED, 1u);
        } else if (phase == CROSS_GREEN) {
            active = pins_for_light(LIGHT_RED, 0u) | pins_for_light(LIGHT_GREEN, 1u);
        } else {
            active = pins_for_light(LIGHT_RED, 0u) | pins_for_light(LIGHT_AMBER, 1u);
        }
    } else if ((now % 500u) < 250u) { /* Exactly 2 Hz, SysTick independent of TIM2 */
        uint8_t lamp = (mode == MODE_RED) ? LIGHT_RED :
                       (mode == MODE_AMBER) ? LIGHT_AMBER : LIGHT_GREEN;
        active = pins_for_light((Light)lamp, 0u) | pins_for_light((Light)lamp, 1u);
    }
    GPIOA->BRR = all_lights;
    GPIOA->BSRR = active;
}
static void draw_seven_segment(uint32_t now) {
    if (mode != MODE_NORMAL) {
        output_display_pair((uint8_t)mode, candidate_seconds);
        return;
    }
    uint32_t elapsed = (uint32_t)(now - phase_start_ms);
    uint32_t remaining = phase_duration_ms(phase) - elapsed;
    uint32_t main_remaining = remaining;
    uint32_t cross_remaining = remaining;
    switch (phase) {
        case MAIN_GREEN:
            cross_remaining += (uint32_t)amber_seconds * 1000u;
            break;
        case MAIN_AMBER:
            break;
        case CROSS_GREEN:
            main_remaining += (uint32_t)cross_amber_seconds() * 1000u;
            break;
        case CROSS_AMBER:
            break;
    }
    output_display_pair(seconds_remaining(main_remaining),
                        seconds_remaining(cross_remaining));
}
void lab3_app_init(void) {
    mode = MODE_NORMAL;
    phase = MAIN_GREEN;
    red_seconds = LAB3_RED_DEFAULT;
    amber_seconds = LAB3_AMBER_DEFAULT;
    green_seconds = LAB3_GREEN_DEFAULT;
    candidate_seconds = red_seconds;
    phase_start_ms = HAL_GetTick();
    last_draw_ms = phase_start_ms - 10u;
}
void fsm_for_input_processing(void) {
    uint32_t now = HAL_GetTick();
    while (button_take_press(BUTTON_MODE)) change_mode(now);
    while (button_take_press(BUTTON_INC)) {
        if (mode != MODE_NORMAL) {
            candidate_seconds = (candidate_seconds >= 99u) ? 1u :
                                (uint8_t)(candidate_seconds + 1u);
        }
    }
    while (button_take_press(BUTTON_SET)) apply_candidate();
    if (mode == MODE_NORMAL) update_traffic_fsm(now);
    if ((uint32_t)(now - last_draw_ms) >= 10u) {
        last_draw_ms = now;
        draw_traffic_lights(now);
        draw_seven_segment(now);
    }
}
