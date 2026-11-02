#include "main.h"
#include "lab3_config.h"
#include "input_reading.h"
#include "input_processing.h"
#include "output_display.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    MX_TIM2_Init();
    MX_TIM3_Init();
    output_display_init();
    buttons_init();
    lab3_app_init();
    if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK) Error_Handler();
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) Error_Handler();
    while (1) {
        fsm_for_input_processing();
    }
}

/* Use HSI 8MHz so PSC=7999 => 1kHz TIM2 count, ARR=tick_ms-1. */
static void SystemClock_Config(void) {
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_OFF;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef io = {0};
    const uint16_t seg_pins = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                              GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    const uint16_t digit_pins = GPIO_PIN_9 | GPIO_PIN_10 |
                                GPIO_PIN_11 | GPIO_PIN_12;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, (uint16_t)0x0FFFu, GPIO_PIN_RESET);
    /* Common Anode: segments and PNP digit drivers are active LOW. */
    HAL_GPIO_WritePin(GPIOB, seg_pins | digit_pins, GPIO_PIN_SET);
    io.Pin = (uint16_t)0x0FFFu;
    io.Mode = GPIO_MODE_OUTPUT_PP;
    io.Pull = GPIO_NOPULL;
    io.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &io);
    io.Pin = seg_pins | digit_pins;
    HAL_GPIO_Init(GPIOB, &io);
    io.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    io.Mode = GPIO_MODE_INPUT;
    io.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &io);
}

static void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7999u;         /* HSI 8MHz -> 1kHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = LAB3_TIM2_TICK_MS - 1u;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(TIM2_IRQn, 2u, 0u);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

static void MX_TIM3_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 7u;            /* HSI 8MHz -> 1MHz */
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999u;            /* 1ms digit-scan tick */
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(TIM3_IRQn, 3u, 0u);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void Error_Handler(void) {
    __disable_irq();
    while (1) { }
}
