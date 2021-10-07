/*
 * sct.c
 *
 *  Created on: 6. 10. 2021
 *      Author: Elias
 */
#include "sct.h"

void sct_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // povoleni hodin portu B
	//RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //povoleni hodin SYSCFG

	GPIOB->MODER |= GPIO_MODER_MODER5_0; // PB4 /LA  - output
	GPIOB->MODER |= GPIO_MODER_MODER4_0; // PB5 SDI - output
	GPIOB->MODER |= GPIO_MODER_MODER3_0; // PB3 CLK - output
	GPIOB->MODER |= GPIO_MODER_MODER10_0; // PB6 /OE - output

	sct_led(0); //zapis 0 do posuvnych registru

	sct_noe(0); //aktivovani vystupu /OE -aktivovani chipu (budou mozna videt duchove pri shiftovani)
	//for (volatile uint32_t i = 0; i < 1000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)
	//sct_noe(1);
}


void sct_led(uint32_t value) {

	for (uint8_t i = 0; i < 32; i++) {	// nasunuti 32 bitu z value

		uint8_t zapis_SDI = value & (1 << 0); //precteni LSB bitu
		sct_sdi(zapis_SDI); //zapsani LSB bitu do SDI
		value >>= 1; // bit shift do prava

		for (volatile uint32_t i = 0; i < 1000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)

		sct_clk(1); // na nabeznou hranu se ovzorkuje z SDI a shiftnou se data
		for (volatile uint32_t i = 0; i < 1000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)
		sct_clk(0);
		for (volatile uint32_t i = 0; i < 1000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)

	}

	sct_nla(1); // zapis do vystupnich registru
	for (volatile uint32_t i = 0; i < 1000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)
	sct_nla(0);

	for (volatile uint32_t i = 0; i < 1000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)
	//sct_noe(0);
	//for (volatile uint32_t i = 0; i < 1000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)
	//sct_noe(1);
	//for (volatile uint32_t i = 0; i < 10000; i++) {} //cekaci smycka - pro ustaleni pinu (asi neni potreba)


}
