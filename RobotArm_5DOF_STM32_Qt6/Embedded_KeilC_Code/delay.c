#include"delay.h"
int myTicks;

void delayMs(int delay)
{
	// Start to count 
	TIM4->CR1 |= TIM_CR1_CEN;
	myTicks=0;
	while(myTicks<delay*1000) ;
	
	TIM4->CR1&= ~TIM_CR1_CEN;
}

void TIM4_interrupt_config()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	
	TIM4->CR1 &= (~TIM_CR1_CEN);
	TIM4->PSC= 0;
	TIM4->ARR = 72-1;
	
	TIM4->CR1 |= TIM_CR1_URS;
	TIM4->DIER |= TIM_DIER_UIE;
	TIM4->EGR |= TIM_EGR_UG;
	
	NVIC_EnableIRQ(TIM4_IRQn);
	
	
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//  
//	/* 
//	 * TIM2 initialization for overflow every 1ms
//   * Update Event (Hz) = timer_clock / (TIM_Prescaler * Time_unit)
//	 * Update Event (Hz) = 72MHz / (3600 * 1000) = 20Hz (1ms)
//   */
//	TIM_TimeBaseInitStruct.TIM_Prescaler = 3600;
//	TIM_TimeBaseInitStruct.TIM_Period = 19;
//	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
//	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
//	
//	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
//	TIM_Cmd(TIM2, ENABLE);
//	
//	
//	NVIC_InitTypeDef NVIC_InitStruct;
//	
//	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
//	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
//	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStruct);
}

	
void TIM4_IRQHandler() 
{
	// Clear the UIF
	TIM4->SR &= (~TIM_SR_UIF);
	myTicks++;
}
