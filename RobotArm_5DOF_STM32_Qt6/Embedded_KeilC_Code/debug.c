#include"debug.h"

void printMsg(char* msg, ...)
{
	char buff[30];
	#ifdef DEBUG_UART 
		va_list args;
		va_start(args, msg);
		vsprintf(buff, msg, args);
		for(int i=0; i<strlen(buff); i++)
		{
				USART1->DR= buff[i];
				while(!(USART1->SR & USART_SR_TXE));
		}
	
	#endif
}

void USART1_Config()
{
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;
	
	// Config GPIOA_port 9 as output maxspeed 50mhz, alternate function push_pull
	GPIOA->CRH |= GPIO_CRH_CNF9_1|GPIO_CRH_MODE9_0| GPIO_CRH_MODE9_1;
	GPIOA->CRH &= ~(GPIO_CRH_CNF9_0);
	
	// Config GPIOA port 10 as input float 
	GPIOA->CRH &= ~(GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1 | GPIO_CRH_CNF10_1);
	GPIOA->CRH |= GPIO_CRH_CNF10_0;
	
	
	// ClkPerSec /(baudx16) = 72mhz/(9600x16)
	// 468.75
	USART1->BRR = 0x1d4c; 
	USART1->CR1|= USART_CR1_TE; // transmit enable 
	USART1->CR1 |= USART_CR1_UE; // uart enable
}
