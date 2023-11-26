#include"stm32f10x.h"
#include"stm32f10x_gpio.h"
#include"stm32f10x_tim.h"
#include"stdbool.h"

#include"../ADC/delay.h"
#include"../ADC/debug.h"

#define PWM_FREQ 50   // PWM frequency in Hz
#define TIM_PRESCALER 72 // Timer prescaler
#define TIM_PERIOD (SystemCoreClock/(TIM_PRESCALER+1)/PWM_FREQ)-1 // Timer period 1/50= 0.02s

void TIM_Config();
void PWM_Config();

// Create a structure to store state of servo
static const float fDutyMin= 0.03f;
static const float fDutyMax= 0.11f;
static const float fDutyRange= fDutyMax- fDutyMin;
static const float fDutyAvr = (fDutyMin+fDutyMax)/2.0f;
typedef struct 
{
	float fTarget;
	float fPostision ;
	float fServoMax;
	float fServoMin;
	} Servo;



// Create a buffer to control 6 servos
Servo servos[6] ;
// Define default states for them
void setDefaultState(Servo* servo);
// Set target angle for servo
void setTargetAngle(Servo* servo, float fAngle);
// Update position for servo 
void updatePositionAngle(Servo* servo);
// Calculate duty cycle based on Angle Position
float calculateDutyCycle(Servo* servo);

	
/* USART Receive - Transmit data =========================================*/
	// Initialize USART1
void USART1_Init(void);
// UART receive buffer
bool bIsUsartReadable= false;
// TX buffer
unsigned char txBuffer[]= "Hello, World!";
uint8_t txBufferSize = 10;
uint8_t txBufferIndex = 0;
// RX buffer
unsigned char rxBuffer[]= "Hello, World!";
uint8_t rxBufferSize = 10;
uint8_t rxBufferIndex = 0;

// Send array using UART
void USART1_SendArray(uint8_t *array, uint8_t size);
// Receive array using UART
void USART1_ReceiveArray();

// USART1_IRQ
void USART1_IRQHandler(void);


int main()	
{
		TIM4_interrupt_config();
		USART1_Init();
		TIM_Config();
		PWM_Config();
	
	
		Servo servos[5];
		setDefaultState(servos);
		
		setTargetAngle(&servos[0], 0.0f);
		setTargetAngle(&servos[1], 50.0f);
		setTargetAngle(&servos[2], 90.0f);
		setTargetAngle(&servos[3], -25.0f);
		setTargetAngle(&servos[4], -20.0f);
		//setTargetAngle(&servos[5], -15.0f);
		
	// Create a buffer to store UART RX data
		//unsigned char buff[12];	
		uint8_t nCurrentServo=0;
	//	printMsg("Hello, program is starting right now..."); 
			delayMs(1000);
		while(1)
		{	
				// Read buffer received from UART_RX to set target angle for servos
			 USART1_ReceiveArray();
				//printMsg("Hello, program is starting right now..."); 
				delayMs(1000);
			
				if(bIsUsartReadable) 
				{
					printMsg("Reading data..."); 
					uint8_t i=0;
					while(i<10)
					{
							unsigned char c= rxBuffer[i];
							if(c<10) nCurrentServo = c;
							else servos[nCurrentServo].fTarget = ((float)c-128);
							i++;
					}
					// Reset state
					USART1_SendArray(rxBuffer, 10);
					bIsUsartReadable= false;
				}
				
				// Now, update all position angle to calculate duty cycle.1
				for(uint8_t i=0; i<5; i++)
				{
					updatePositionAngle(&servos[i]);
				}
				
				// Update duty cycle CCCR using TIM_SetComparex(TIMy, duty_cycle); // x- channel, y- timer
				TIM_SetCompare3(TIM2, calculateDutyCycle(&servos[0]) * (TIM_PERIOD+1) -1);
				TIM_SetCompare4(TIM2, calculateDutyCycle(&servos[1]) * (TIM_PERIOD+1) -1);
				TIM_SetCompare1(TIM3, calculateDutyCycle(&servos[2]) * (TIM_PERIOD+1) -1);
				TIM_SetCompare2(TIM3, calculateDutyCycle(&servos[3]) * (TIM_PERIOD+1) -1);
				TIM_SetCompare3(TIM3, calculateDutyCycle(&servos[4]) * (TIM_PERIOD+1) -1);
			//	TIM_SetCompare4(TIM3, calculateDutyCycle(&servos[5]) * (TIM_PERIOD+1) -1);
				
		}
}


void TIM_Config()
{
		// Config TIM3 and TIM2==========================================================
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // Structure for timer settings
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM2, ENABLE); // Enable clock for TIM3
    
    TIM_TimeBaseStructure.TIM_Prescaler = TIM_PRESCALER; // Set prescaler value
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // Set counter mode as up-counting
    TIM_TimeBaseStructure.TIM_Period = TIM_PERIOD; // Set period value
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // Set clock division 0
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; // Set repetition counter value
    
		// Initialize TIM3 with the structure, the UG bit of event generation register
		// UG bit int EGR will be set to 1 (0x01) to update event cature/compare generation all channel.
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
    // Enable TIM3 and TIM2 to start counting
    TIM_Cmd(TIM3, ENABLE); 
		TIM_Cmd(TIM2, ENABLE);
		
}

void PWM_Config()
{
    // Config GPIOpins as output push pull alternate function=====================================
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // Enable clock for GPIOA
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // Enable clock for GPIOB
    
    GPIO_InitTypeDef GPIO_InitStructure; // Structure for GPIO settings
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 |GPIO_Pin_3 |GPIO_Pin_2; // Set GPIOA pins 6,7 and 2,3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Set mode as alternate function push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Set speed as 50 MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure); // Initialize GPIOA with the structure
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; // Set GPIOB pins 0 and 1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Set mode as alternate function push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Set speed as 50 MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure); // Initialize GPIOA with the structure
	
		

		// Config PWM mode==========================
		TIM_OCInitTypeDef TIM_OCInitStructure; // Structure for PWM settings
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // Set PWM mode as PWM1 (duty cycle is inversely proportional to pulse length)
    
    /* Configure tim3 channel 1 - PA6 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Enable output compare state
    TIM_OCInitStructure.TIM_Pulse = (TIM_PERIOD + 1) * fDutyAvr - 1; // Set pulse width to 25% duty cycle
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // Set output polarity as high
    TIM_OC1Init(TIM3, &TIM_OCInitStructure); // Initialize channel 1 with the structure
    
    /* Configure tim3 channel 2 - PA7*/
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Enable output compare state
    TIM_OCInitStructure.TIM_Pulse = (TIM_PERIOD + 1) * fDutyAvr - 1; // Set pulse width to 75% duty cycle
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // Set output polarity as high
    TIM_OC2Init(TIM3, &TIM_OCInitStructure); // Initialize channel 2 with the structure
		
		/* Configure tim3 channel 3 - PB0 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Enable output compare state
    TIM_OCInitStructure.TIM_Pulse = (TIM_PERIOD + 1) * fDutyAvr- 1; // Set pulse width to 25% duty cycle
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // Set output polarity as high
    TIM_OC3Init(TIM3, &TIM_OCInitStructure); // Initialize channel 1 with the structure
    
    /* Configure tim3 channel 4 - PB1*/
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Enable output compare state
    TIM_OCInitStructure.TIM_Pulse = (TIM_PERIOD + 1) * fDutyAvr - 1; // Set pulse width to 75% duty cycle
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // Set output polarity as high
    TIM_OC4Init(TIM3, &TIM_OCInitStructure); // Initialize channel 2 with the structure
		
    /* Configure tim2 channel 3 - PA2 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Enable output compare state
    TIM_OCInitStructure.TIM_Pulse = (TIM_PERIOD + 1) *fDutyAvr - 1; // Set pulse width to 25% duty cycle
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // Set output polarity as high
    TIM_OC3Init(TIM2, &TIM_OCInitStructure); // Initialize channel 1 with the structure
    
    /* Configure tim2 channel 4 - PA3*/
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Enable output compare state
    TIM_OCInitStructure.TIM_Pulse = (TIM_PERIOD + 1) *fDutyAvr - 1; // Set pulse width to 75% duty cycle
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // Set output polarity as high
    TIM_OC4Init(TIM2, &TIM_OCInitStructure); // Initialize channel 2 with the structure
		 
    // Enable output compare preload ===================   
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
}

void setDefaultState(Servo* servo)
{
	for(uint8_t i=0; i<6; i++)
	{
		servo[i].fServoMax= 90.0f;
		servo[i].fServoMin = -90.0f;
		servo[i].fPostision =0.0f;
		servo[i].fTarget = servo[i].fPostision;
	}
}

void setTargetAngle(Servo* servo, float fAngle)
{
	servo->fTarget  = fAngle;
	if(servo->fTarget <servo->fServoMin) servo->fTarget= servo->fServoMin;
	if(servo->fTarget>servo->fServoMax) servo->fTarget = servo->fServoMax;
}

void updatePositionAngle(Servo* servo)
{
	servo->fPostision = servo->fTarget;
}

float calculateDutyCycle(Servo* servo)
{
	float fDutyCycle= servo->fPostision/(servo->fServoMax - servo->fServoMin);
	fDutyCycle = fDutyMin + fDutyRange/2.0f + fDutyCycle*fDutyRange;
}

// Initialize USART1
void USART1_Init(void) {
    // ... (USART1 initialization code as before)
	// Enable clock for GPIOA and USART1
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
		
		// Configure PA9 as USART1 TX (alternate function output push-pull)
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		// Configure PA10 as USART1 RX (input floating)
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		// Set USART1 parameters (baud rate, word length, stop bit, etc.)
		USART_InitTypeDef USART_InitStructure;
		USART_StructInit(&USART_InitStructure); // Initialize with default values
		USART_InitStructure.USART_BaudRate = 9600; // Change baud rate to 9600 bps
		USART_Init(USART1, &USART_InitStructure); // Apply the configuration
		
		// Enable USART1
		USART_Cmd(USART1, ENABLE);
    
    // Enable USART1 interrupt in NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Enable USART1 TXE interrupt
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

// Send array using UART
void USART1_SendArray(uint8_t *array, uint8_t size) {
    txBufferIndex = 0; // Reset buffer index
    txBufferSize = size; // Set buffer size
    memcpy(txBuffer, array, size); // Copy data to buffer
    
    // Enable TXE interrupt to start transmission
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

// Receive array using UART
void USART1_ReceiveArray()
{
	rxBufferIndex =0;
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

// USART1 Interrupt Service Routine
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) { // Transmit buffer empty
        if (txBufferIndex < txBufferSize) {
            // Send next byte
            USART_SendData(USART1, txBuffer[txBufferIndex++]);
        } else {
            // Disable TXE interrupt after last byte has been transmitted
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
						
        }
    }
		
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
			
			if(rxBufferIndex <rxBufferSize)
				rxBuffer[rxBufferIndex++] = USART_ReceiveData(USART1);
			else
			{
				
				USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
				bIsUsartReadable= true;
			}
		}
	
}



