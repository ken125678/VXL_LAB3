#include "main.h"
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

void NMI_Handler(void) { while (1) { } }
void HardFault_Handler(void) { while (1) { } }
void MemManage_Handler(void) { while (1) { } }
void BusFault_Handler(void) { while (1) { } }
void UsageFault_Handler(void) { while (1) { } }
void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }
void SysTick_Handler(void) { HAL_IncTick(); }
void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }
void TIM3_IRQHandler(void) { HAL_TIM_IRQHandler(&htim3); }
