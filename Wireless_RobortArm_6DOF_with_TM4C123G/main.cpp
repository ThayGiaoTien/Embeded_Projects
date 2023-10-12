/* Generates 50HZ and variable duty cycle on PF2 pin of TM4C123 Tiva C Launchpad */
/* PWM1 module and PWM generator 3 of PWM1 module is used. Hence PWM channel*/
#include "TM4C123GH6PM.h"
#include"PWM.h"

#include<iostream>

void UART5_init();
void UART5_Transmitter(unsigned char data);
void UART5_Reciever(unsigned char data);
void SendStringViaUART5(char* str);
unsigned char BluetoothReadViaUART5();
int main(void)
{
void Delay_ms(int n);
int duty_cycle = 4850-1;
char* msg;
float duty ;
    
/* Clock setting for PWM and GPIO PORT */
SYSCTL->RCGCPWM |= (1<<1)|(1<<0);       /* Enable clock to PWM1 module */
SYSCTL->RCGCGPIO |= (1<<5)|(1<<1);  /* Enable system clock to PORTF */
SYSCTL->RCC |= (1<<20);    /* Enable System Clock Divisor function for PWM */ //PWMDIVENB
SYSCTL->RCC |= 0x000E0000; /* Use pre-divider value of 64 and after that feed clock to PWM1 module*/ 

//	
//GPIOF->AFSEL |= (1<<2);  /* PF2 sets a alternate function */
//GPIOF->PCTL &= ~0x00000F00; /*set PF2 as output pin */
//GPIOF->PCTL |= 0x00000500; /* make PF2 PWM output pin */
//GPIOF->DEN |= (1<<2);      /* set PF2 as a digital pin */
//    
//PWM1->_3_CTL &= ~(1<<0);   /* Disable Generator 3 counter */
//PWM1->_3_CTL &= ~(1<<1);   /* select down count mode of counter 3*/
//PWM1->_3_GENA = 0x0000008C;  /* Set PWM output when counter reloaded and clear when matches PWMCMPA */
//PWM1->_3_LOAD = 5000-1;     /* set load value for 50Hz 16MHz/65 = 250kHz and (250KHz/5000) */
//PWM1->_3_CMPA = duty_cycle;     /* set duty cyle to to minumum value*/
//PWM1->_3_CTL = 1;           /* Enable Generator 3 counter */
//PWM1->ENABLE = 0x40;      /* Enable PWM1 channel 6 output */
M1PWM6_Init(5000,4500);
M1PWM7_Init(5000,4500);
M0PWM0_Init(5000,4500);
M0PWM1_Init(5000,4500);
M0PWM2_Init(5000,4500);
M0PWM3_Init(5000,4500);

// Init uart Rx/PE4, Tx/PE5
UART5_init();
Delay_ms(100);


    while(1)
		{
//    	PWM1->_3_CMPA= duty_cycle;
//        /*duty_cycle = duty_cycle - 10;
//        if (duty_cycle 0;<= 4350) 
//				duty_cycle = 4870;
//        PWM1->_3_CMPA = duty_cycle;
//        Delay_ms(100);*/
//				Delay_ms(500);
//				duty=  (float)BluetoothReadViaUART5()*100;
//				if(duty<4850 && duty> 4380)
//				{
//					PWM1->_3_CMPA = (int)duty-1;
//			/*	PWM1->_3_CMPA = 4635	-1;
//        Delay_ms(1000);
//				PWM1->_3_CMPA =4380-1;
//        Delay_ms(1000);
//				PWM1->_3_CMPA = 4850-1;*/
//				Delay_ms(500);
//			sprintf(msg, "duty cycle is: %d\n", (int)duty);
//				SendStringViaUART5(msg);
//				}
//			  
//				Delay_ms(500);
       
    }
}
unsigned char BluetoothReadViaUART5()
{
	unsigned short data;
	while((UART5->FR & (1<<4)) != 0) ; 
	data= UART5->DR;
	return data;
}

/* This function generates delay in ms */
/* calculations are based on 16MHz system clock frequency */
void UART5_init()
{   
    // Enable clock
    SYSCTL->RCGCUART |= 0x20; // enable uart clock 5
    SYSCTL->RCGCGPIO |= 0x10;  // PE4/RX PE5/TX
    //Delay(1);

    // Initialization baudrate 9600 - Now system clock is 250MHZ
    UART5->CTL=0;
    UART5->IBRD = 104;		// IRBD = clock_fre/16 /9600
    UART5->FBRD=11;   //((float)(clock/baud*64)-(int)(clock/b(clock/baud*64))*64) =0.5
    UART5->CC= 0;
    UART5->LCRH = 0x60;  // 8bit, not parity bit, no FIFO
    UART5->CTL=0x301; 		// Rx , Tx, En

    // Configuration PE4/RX & PE5/TX
    GPIOE->DEN = 0x30;      // enable digital function  
    GPIOE->AFSEL = 0x30;    //  enable alternate function
    GPIOE->AMSEL = 0; // turn off analog function
    GPIOE->PCTL = 0x00110000; 

}
void UART5_Transmitter(unsigned char data)
{
    while((UART5->FR & (1<<5)) !=0) ; // Wait until Tx buffer is not full
    UART5->DR= data;
}
void SendStringViaUART5(char* str)
{
    while(*str)
    {
        UART5_Transmitter(*(str++));
    }

} 
void Delay_ms(int time_ms)
{
    int i, j;
    for(i = 0 ; i < time_ms; i++)
        for(j = 0; j < 3180; j++)
            {}  /* excute NOP for 1ms */
}
