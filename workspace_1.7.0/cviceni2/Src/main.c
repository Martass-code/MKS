/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include <stdint.h>

#include "stm32f0xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

#define LED_TIME_BLINK 300 // blikaci perioda 300 ms

#define LED_TIME_SHORT 100 // doba rozsviceni LED2
#define LED_TIME_LONG 1000 // doba rozsviceni LED2
#define VZORKOVANI 5 //perioda vzorkovani tlacitka

void tlacitka(void); //rizeni LED podle stisknuti tlacitek

int main(void) {

	/*
	 o LED1 (vlevo) = PA4
	 o LED2 (vpravo) = PB0
	 o S1 (vpravo) = PC1
	 o S2 (vlevo) = PC0 (EXTI0)
	 */

	SysTick_Config(8000); // 1ms inicializace SysTick casovace (hodiny jsou 8 MHz)

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN; // povoleni hodin portu A, B, C

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //povoleni hodin SYSCFG

	GPIOA->MODER |= GPIO_MODER_MODER4_0; // LED1 = PA4, output
	GPIOB->MODER |= GPIO_MODER_MODER0_0; // LED2 = PB0, output

	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_0; // S2 = PC0, pullup
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR1_0; // S1 = PC1, pullup

	/*
	 SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PC; // select PC0 for EXTI0
	 EXTI->IMR |= EXTI_IMR_MR0; // mask
	 EXTI->FTSR |= EXTI_FTSR_TR0; // trigger on falling edge
	 NVIC_EnableIRQ(EXTI0_1_IRQn); // enable EXTI0_1
	 */
	//GPIOA->BSRR = (1 << 0); // set
	for (;;) {
		tlacitka();
	}

}

volatile uint32_t Tick;

void SysTick_Handler(void) {
	Tick++; //perioda 1ms
}

/////////////////////////
void tlacitka(void) {

	// obsluha tlacitka S2
	static uint32_t new_s2;
	static uint32_t old_s2;
	static uint32_t off_time_s2;

	static uint32_t delay_s2 = 0;

	if (Tick > (delay_s2 + VZORKOVANI)) {
		new_s2 = GPIOC->IDR & (1 << 0); //aktualni stav tlacitka

		if (old_s2 && !new_s2) { // detekce sestupne hrany - (old 1 new 0)
			off_time_s2 = Tick + LED_TIME_SHORT; //vypocet casu vypnuti LED
			GPIOB->BSRR = (1 << 0); //zapnuti LED - PB0 - vpravo
		}
		old_s2 = new_s2; //aktualizace minuleho stavu tlacitka

		delay_s2 = Tick;
	}

	//obsluha tlacitka1
	static uint32_t new_s1;
	//static uint32_t old_s1;
	static uint32_t off_time_s1;

	static uint16_t debounce = 0xFFFF;

	static uint32_t delay_s1 = 0;

	if (Tick > (delay_s1 + VZORKOVANI)) {
		new_s1 = GPIOC->IDR & (1 << 1); //aktualni stav tlacitka
		debounce <<= 1; //(kdyz bude seple tak tam zustane shiftnuta 0)
		//debounce = (new_s1 & 0x0001)
		if (new_s1) { //pokud tlacitko rozeple tak shiftnout 1 do debounce
			debounce |= 0x0001;
		}

		if (debounce == 0x7FFF) { // detekce sestupne hrany - (old 1 new 0)
			off_time_s1 = Tick + LED_TIME_LONG; //vypocet casu vypnuti LED2
			GPIOB->BSRR = (1 << 0); //zapnuti LED2 - PB0 - vpravo
		}
		//old_s1 = new_s1; //aktualizace minuleho stavu tlacitka

		delay_s1 = Tick; //dalsi vzorkovaci perioda bude v tomhle case
	}

	if ((Tick > off_time_s1) && (Tick > off_time_s2)) { //cas vypnuti LED nastal (pro oba casy tlacitek)
		GPIOB->BRR = (1 << 0); //vypnuti LED2
	}
}
/////////////////////////
/*
 void EXTI0_1_IRQHandler(void) //detekce zmacknuti tlacitka
 {
 if (EXTI->PR & EXTI_PR_PR0) { // check line 0 has triggered the IT
 EXTI->PR |= EXTI_PR_PR0; // clear the pending bit
 GPIOB->ODR ^= (1 << 0); // toggle
 }
 }
 */