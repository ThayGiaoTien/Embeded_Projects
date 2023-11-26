#include"stm32f10x.h"

#include"stdint.h"
#include"string.h"
#include"stdlib.h"
#include"stdarg.h"


#define DEBUG_UART USART1

 void printMsg(char* msg, ...);

void USART1_Config();
