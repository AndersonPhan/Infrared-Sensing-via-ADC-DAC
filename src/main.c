#include "stm32l476xx.h"
#include "DAC.h"
#include "TIM.h"
#include "LED.h"
#include "SysTimer.h"
#include "SysClock.h"
#include "ADC.h"
#include "UART.h"

#include <string.h>
#include "stdio.h"

volatile uint32_t result;
volatile uint32_t global_counter;
volatile uint8_t flag = 0; 
const uint32_t THRESHOLD = 3723;
unsigned char str[6];			

void Timer_Init(void);

int main(void){
	
	int i;
	UART2_Init();
	//SysTick_Init();
	
	Timer_Init(); //use a timer to periodically trigger ADC
	
	// Analog Input: 
	//  PA1 (ADC12_IN 6)
	//  0V <=> 0, 3.3V <=> 4095
	ADC_Init();
	
	ADC1->CR |= ADC_CR_ADSTART;
	//start ADC. The exact staring time determined by the EXTEN[1:0] of ADC1->CFGR
	
	// Analog Outputs: PA5 (DAC2)
	DAC_Init();
	
	global_counter = 0;
	
	System_Clock_Init(); // Switch System Clock = 80 MHz
	SysTick_Init();

	configure_LED_pin();
	turn_off_LED();
		
	// TIM4_TRGO triggers DAC channel 2, i.e., PA.5
	// In addition, GPIO PA.15 (also connected to TIM4_CH1) is used for debugging
	TIM4_Init();
	
	while(1){	
		if(flag == 1) {
			turn_on_LED();
		} else {
			turn_off_LED();
		}
		
				float result_V = (float)result/1241;
        printf("measured volts by ADC: %f \r\n\n", result_V );
		
		delay(500);
	}
}


// Timer 4's frequency is 80MHz/(79+1)/(999+1)=440hz, OCREF's frequency.
void TIM4_IRQHandler(void){

	if((TIM4->SR & TIM_SR_CC1IF) != 0){
		TIM4->SR &= ~TIM_SR_CC1IF;

		global_counter = global_counter % 4096;
		DAC->DHR12R2 = global_counter; //12 bits, max number 4095.
		global_counter++;
		//every interrupt, increase the counter by 1.		
		
		// PA.5, DAC channel 2
		//since PA.5 is connected to green LED, we will 
		//see its brightness changes over time, according to the output
		//analog voltage level. 
		
		if (flag == 1) {
			DAC->DHR12R2 = result;
		} else {
			DAC->DHR12R2 = 0;
		}
	}

	if((TIM4->SR & TIM_SR_TIF) != 0) //trigger interrupt flag
		TIM4->SR &= ~TIM_SR_TIF;
	
	//if((TIM4->SR & TIM_SR_UIF) != 0) // timer counter overflow, over ARR
	//	TIM4->SR &= ~TIM_SR_UIF;
		
	return;
}

//******************************************************************************************
// 	ADC 1/2 Interrupt Handler
//******************************************************************************************
void ADC1_2_IRQHandler(void){
	NVIC_ClearPendingIRQ(ADC1_2_IRQn);
	
	//for a regular channel, check EOC flag
	// ADC End of Conversion (EOC)
	if ((ADC1->ISR & ADC_ISR_EOC) == ADC_ISR_EOC) {
		// It is cleared by software writing 1 to it 
		//or by reading the corresponding ADCx_DR register
		//ADC1->ISR |= ADC_ISR_EOC;
		result = ADC1->DR; //reading DR clears EOC flag
		if(result < THRESHOLD) {
			flag = 1;
		} else {
			flag = 0;
		}
	}
	
	// ADC End of Regular Sequence of Conversions  (EOS). If enabled then process it.
	//if ((ADC1->ISR & ADC_ISR_EOS) == ADC_ISR_EOS) {
		// It is cleared by software writing 1 to it.
		//ADC1->ISR |= ADC_ISR_EOS;		
	//}
}

// use timer 4's channel 1 to trigger ADC1's channel 6, that is, PA1 (ADC12_IN 6)
void Timer_Init(void) {
	
	RCC->APB1ENR1 		|= RCC_APB1ENR1_TIM4EN;   // Enable TIMER clock

	// Counting direction: 0 = up-counting, 1 = down-counting
	TIM4->CR1 &= ~TIM_CR1_DIR; 	
	TIM4->CR1 &= ~TIM_CR1_CMS; //clear edge-aligned mode bits

	//Master Mode Selection (MMS) bits
	TIM4->CR2 &=~TIM_CR2_MMS;   //clear those bits
	TIM4->CR2 |= TIM_CR2_MMS_2; //select 100 as TRGO, i.e., OC1REF selected as TRGO
                              // see reference manual.

	TIM4->CCMR1 &= ~TIM_CCMR1_OC1M; // Clear ouput compare mode bits for channel 1
  TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
  // OC1M = 110 for PWM Mode 1 output on ch1
    
  //TIM4->CCMR1  		|= TIM_CCMR1_OC1PE;  // Output 1 preload enable

  TIM4->PSC 	= 79; // Prescaler = 79
	TIM4->ARR	 = 1000-1; // Auto-reload: Upcouting (0..ARR), Downcouting (ARR..0)
	TIM4->CCR1  = 500;  // Output Compare Register for channel 1 

	//TIM4->CCER |= TIM_CCER_CC1E; // OC1 signal is output
	TIM4->CR1  |= TIM_CR1_CEN;  // Enable timer counter
	
	// Select output polarity: 0 = active high, 1 = active low
	//TIM1->CCMR1 &= ~TIM_CCER_CC1NP; // select active high
		
  // Enable output for ch1N
	//TIM1->CCER 	 		|= TIM_CCER_CC1NE;                       
    
  // Main output enable (MOE): 0 = Disable, 1 = Enable
	//TIM1->BDTR |= TIM_BDTR_MOE;
}