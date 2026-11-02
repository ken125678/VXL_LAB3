#include "output_display.h"
/* Seven-segment COMMON ANODE, active-low segment lines and PNP
 * high-side digit drivers: PB9, PB10, PB11, PB12 active LOW.
 * PNP emitter = +3.3V, collector = digit common anode,
 * base = corresponding PBx through 1kohm. No decimal points. */
#define SEGMENTS_ALL (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_5 | \
                      GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8)
#define DIGITS_ALL   (GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12)

static const uint16_t segment_pins[7] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_5,
    GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8
};
static const uint16_t digit_pins[4] = {
    GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12
};
/* bit0=a, bit1=b, bit2=c, bit3=d, bit4=e, bit5=f, bit6=g */
static const uint8_t seven_seg_font[10] = {
    0x3Fu, 0x06u, 0x5Bu, 0x4Fu, 0x66u,
    0x6Du, 0x7Du, 0x07u, 0x7Fu, 0x6Fu
};
static volatile uint8_t digits[4];
static uint8_t digit_index = 0u;

void output_display_init(void) {
    for (uint8_t i = 0; i < 4u; i++) digits[i] = 0u;
    digit_index = 0u;
    GPIOB->BSRR = DIGITS_ALL | SEGMENTS_ALL; /* all off (logic HIGH) */
}

void output_display_pair(uint8_t left, uint8_t right) {
    if (left > 99u) left = 99u;
    if (right > 99u) right = 99u;
    digits[0] = (uint8_t)(left / 10u);
    digits[1] = (uint8_t)(left % 10u);
    digits[2] = (uint8_t)(right / 10u);
    digits[3] = (uint8_t)(right % 10u);
}

/* A different digit is scanned every 1 ms: 250 Hz refresh per digit. */
void output_display_scan_1ms(void) {
    uint8_t idx = digit_index;
    uint8_t value = digits[idx];
    uint8_t font = seven_seg_font[value <= 9u ? value : 0u];
    uint16_t seg_on = 0u;
    for (uint8_t s = 0u; s < 7u; ++s) {
        if ((font & (1u << s)) != 0u) seg_on |= segment_pins[s];
    }
    /* Multiplexing for Common Anode: LOW switches ON, HIGH switches OFF. */
    GPIOB->BSRR = DIGITS_ALL;   /* disable all PNP high-side drivers */
    GPIOB->BSRR = SEGMENTS_ALL; /* switch all segment cathodes OFF */
    GPIOB->BRR = seg_on;        /* pull requested segment cathodes LOW */
    GPIOB->BRR = digit_pins[idx]; /* PNP base LOW: enable this digit */
    digit_index = (uint8_t)((idx + 1u) & 3u);
}
