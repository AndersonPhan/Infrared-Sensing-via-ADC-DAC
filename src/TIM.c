#include "TIM.h"
#include "LED.h"
#include "stm32l476xx.h"
#include <stdint.h>

//******************************************************************************************
// GPIO PA15 as TIM4_CH1 for DAC trigger
//******************************************************************************************
void TIM4_Init(){
	
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN; // Enable Clock of Timer 4
	
	TIM4->CR1  &= ~TIM_CR1_DIR;  // Counting direction: Up Counting
	TIM4->CR1  &= ~TIM_CR1_CMS;  // Edge-aligned mode, default 00
	
	// Master mode selection
	// 000: UG bit from the TIMx_EGR register is used as trigger output (TRGO). 
	// 001: Enable - the Counter Enable signal CNT_EN is used as trigger output (TRGO). 
	// 010: Update - The update event is selected as trigger output (TRGO). 
	// 011: Compare Pulse - The trigger output send a positive pulse when the CC1IF flag is to be set (even if it was already high).
	// 100: Compare - OC1REF signal is used as trigger output (TRGO)
	// 101: Compare - OC2REF signal is used as trigger output (TRGO)
	// 110: Compare - OC3REF signal is used as trigger output (TRGO)
	// 111: Compare - OC4REF signal is used as trigger output (TRGO)	
	TIM4->CR2  &= ~TIM_CR2_MMS;  // Master mode selection
	TIM4->CR2  |= TIM_CR2_MMS_2; // 100 = OC1REF as TRGO

	TIM4->DIER |= TIM_DIER_TIE;  // Trigger interrupt enable
	//TIM4->DIER |= TIM_DIER_UIE;  // Update interrupt enable
	TIM4->DIER |= TIM_DIER_CC1IE;//
		
	// OC1M: Output Compare 1 mode
	// 0110: PWM mode 1 - In upcounting, 
	//		channel 1 is active as long as TIMx_CNT < TIMx_CCR1
  // else inactive. 
	//In downcounting, channel 1 is active (OC1REF = 1) 
	// as long as TIMx_CNT <= TIMx_CCR1 
	TIM4->CCMR1 &= ~TIM_CCMR1_OC1M; 
	TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;  // 0110 = PWM mode 1
	
	// The counter clock frequency (CK_CNT) = fCK_PSC / (PSC[15:0] + 1)
	// Timer driving frequency = 80 MHz/(1 + PSC) = 80 MHz/(1+79) = 1MHz
	TIM4->PSC  =   79;    // max 65535
	
  // Trigger frequency = 1MHz / (1 + ARR) = 1MHz/1000 = 1KHz
	TIM4->ARR  = 2271;   // max 440 Hz
	
	TIM4->CCR1 = 1136;   // Duty ratio 50%
	 
	TIM4->CCER |= TIM_CCER_CC1E; //  OC1 signal is output on the corresponding output pin
	TIM4->CR1  |= TIM_CR1_CEN;   // Enable timer counter
	
	// Select output polarity: 0 = active high, 1 = active low
	TIM4->CCMR1 &= ~TIM_CCER_CC1P; // select active high
		
  // Enable output for ch1
	TIM4->CCER 	 		|= TIM_CCER_CC1E;                       
    
  // Main output enable (MOE): 0 = Disable, 1 = Enable
	TIM4->BDTR |= TIM_BDTR_MOE;
	
	// Set up GPIO pin PA.15 as TIM2_CH1 for debugging
	//observe the timer's output PWM pulse signal with freq 1KHZ. 
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;
	GPIOA->MODER  &= ~(3U<<(2*15));
	GPIOA->MODER  |=   2U<<(2*15);    // Input(00, reset), Output(01), AlterFunc(10), Analog(11, reset)
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL15; 
	GPIOA->AFR[1] |=  GPIO_AFRH_AFSEL15_0;    // AF1 = TIM2_CH1 for PA15	
	
	
	NVIC_SetPriority(TIM4_IRQn, 1);		// Set Priority to 1
	NVIC_EnableIRQ(TIM4_IRQn);
}

