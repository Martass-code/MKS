/*
 * sct.h
 *
 *  Created on: 6. 10. 2021
 *      Author: Elias
 */

#ifndef SCT_H_
#define SCT_H_

#include <stdint.h>


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif



void sct_init(void); //povoleni hodin prislusnych portu (RCC_AHBENR_GPIOB), nastaveni pinu jako vystupnich (GPIO_MODER_MODERx_0),
// zapis nul do posuvneho registru (volání sct_led(0)), aktivování výstupu pomocí /OE (volání sct_noe(0))

void sct_led(uint32_t value); //zapise hodnoty na displej

void sct_value(uint16_t value); //preklad cislovek na odpovidajici segmenty

#endif /* SCT_H_ */
