/********************************************
*			STM32F439 Main (C Header File)  			*
*			Developed for the STM32								*
*			Author: Dr. Glenn Matthews						*
*			Header File														*
********************************************/


// Compiler pragmas



// Setup functions
void RCC_init();
void LED_GPIO_config();
void Input_GPIO_config();
void ADC_config();

void RCC_init()
{
	
	// Enable GPIOA, GPIOB and GPIOF
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
	
	// Timer setup
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	
	// UART setup
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
	
	// ADC3 setup
	RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
	
	// Reset peripheral interfaces
	RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOARST;
	RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOBRST;
	RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOFRST;
	RCC->APB1RSTR |= RCC_APB1RSTR_TIM6RST;
	RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
	RCC->APB2RSTR |= RCC_APB2RSTR_ADCRST;
	__asm("NOP");
	__asm("NOP");
	
	RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_GPIOARST);
	RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_GPIOBRST);
	RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_GPIOFRST);
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM6RST);
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_USART3RST);
	RCC->APB2RSTR &= ~(RCC_APB2RSTR_ADCRST);
	__asm("NOP");
	__asm("NOP");
	
}

void LED_GPIO_config()
{
	// Configure LED2, 5, 6 and 7 for output
	// Clear MODER bits
	GPIOA->MODER &= ~(GPIO_MODER_MODE9_Msk);	// Clear modder bits for GPIOA_9
	GPIOB->MODER &= ~(GPIO_MODER_MODE1_Msk);	// Clear modder bits for GPIOB_1
	GPIOB->MODER &= ~(GPIO_MODER_MODE8_Msk);	// Clear modder bits for GPIOB_8
	GPIOF->MODER &= ~(GPIO_MODER_MODE8_Msk);	// Clear modder bits for GPIOF_8
	
	// Enable as outputs
	GPIOA->MODER |= 0x01 << GPIO_MODER_MODE9_Pos;	 // Enable PA9 as output	
	GPIOB->MODER |= 0x01 << GPIO_MODER_MODE1_Pos;	 // Enable PB1 as output
	GPIOB->MODER |= 0x01 << GPIO_MODER_MODE8_Pos;	 // Enable PB8 as output
	GPIOF->MODER |= 0x01 << GPIO_MODER_MODE8_Pos;	 // Enable PF8 as output
	
	// Enable push-pull output
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT9);	//Clear bit 9
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT1);	//Clear bit 1
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT8);	//Clear bit 8
	GPIOF->OTYPER &= ~(GPIO_OTYPER_OT8);	//Clear bit 8
	
	// Set speed to medium
	GPIOA->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED9_Pos);	// Clear bits
	GPIOB->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED1_Pos);
	GPIOB->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED8_Pos);
	GPIOF->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED8_Pos);
	
	GPIOA->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED9_Pos);	// Set to medium speed (0b01)
	GPIOB->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED1_Pos);
	GPIOB->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED8_Pos);
	GPIOF->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED8_Pos);
	
	// Clear PUPDR reg
	GPIOA->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD9_Pos);	// Clear bits
	GPIOB->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD1_Pos);
	GPIOB->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD8_Pos);
	GPIOF->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD8_Pos);
	
	// Set ODR to clear LED
	GPIOA->ODR |= GPIO_ODR_OD9;	// Set bit to 1
	GPIOB->ODR |= GPIO_ODR_OD1;
	GPIOB->ODR |= GPIO_ODR_OD8;
	GPIOF->ODR |= GPIO_ODR_OD8;
}

void Input_GPIO_config()
{
	// Configure GPIO inputs
	// Clear MODER bits	/ Enable as input mode
	GPIOA->MODER &= ~(GPIO_MODER_MODE8_Msk);	// Clear modder bits for GPIOA_8
	GPIOA->MODER &= ~(GPIO_MODER_MODE10_Msk);	// Clear modder bits for GPIOA_10
	GPIOB->MODER &= ~(GPIO_MODER_MODE0_Msk);	// Clear modder bits for GPIOB_0
	GPIOF->MODER &= ~(GPIO_MODER_MODE10_Msk);	// Clear modder bits for GPIOF_10
	
	// Enable PF10 as analogue mode
	GPIOF->MODER |= 0x03 << GPIO_MODER_MODE10_Pos;	 // Enable PF8 as analogue (0b11)
	
	// Enable push-pull output - does not affect input mode
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT8);	//Clear bit 8
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT10);	//Clear bit 10
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT8);	//Clear bit 0
	GPIOF->OTYPER &= ~(GPIO_OTYPER_OT10);	//Clear bit 10
	
	// Set speed to medium
	GPIOA->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED8_Pos);	// Clear bits
	GPIOA->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED10_Pos);
	GPIOB->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED0_Pos);
	GPIOF->OSPEEDR &= ~(0x03 << GPIO_OSPEEDR_OSPEED10_Pos);
	
	GPIOA->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED8_Pos);	// Set to medium speed (0b01)
	GPIOA->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED10_Pos);
	GPIOB->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED0_Pos);
	GPIOF->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED10_Pos);
	
	// Clear PUPDR reg
	GPIOA->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD8_Pos);	// Clear bits
	GPIOA->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD10_Pos);
	GPIOB->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD0_Pos);
	GPIOF->PUPDR &= ~(0x03 << GPIO_PUPDR_PUPD10_Pos);
	
}

void ADC_config()
{
	// Configure ADC3 channel 8 -> PF10 
	// Disable battery sensing channel
	ADC123_COMMON->CCR &= ~(ADC_CCR_VBATE);
	
	// Enable temp sensing channel
	ADC123_COMMON->CCR |= (ADC_CCR_TSVREFE) | (0x03 << ADC_CCR_ADCPRE_Pos);
	
	// Disable scan mode and set resolution to 12 bits
	ADC3->CR1 &= ~((ADC_CR1_SCAN) | (0x00 << ADC_CR1_RES_Pos));
	
	// Alignment set to right and set single mode conversion
	ADC3->CR2 &= ~(ADC_CR2_CONT |  ADC_CR2_ALIGN | ADC_CR2_SWSTART);
	
	// Set to single channel - channel 8
	ADC3->SQR3 &= ~(ADC_SQR3_SQ1_Msk);
	ADC3->SQR3 |= 0x08;
	ADC3->SQR3 &= ~(ADC_SQR1_L_Msk);
	
	// Set sample time register - 56 cycles
	ADC3->SMPR2 &= ~(ADC_SMPR2_SMP0_Msk);
	ADC3->SMPR2 |= 0x03 << (ADC_SMPR2_SMP0_Pos);
	
	// Enable ADC
	ADC3->CR2 |= ADC_CR2_ADON;
}





