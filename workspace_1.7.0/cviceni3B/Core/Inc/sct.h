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

/*
#define sct_nla(x) do { if (x) GPIOB->BSRR = (1 << 5); else GPIOB->BRR = (1 << 5); } while (0) // PD4 /LA
#define sct_sdi(x) do { if (x) GPIOB->BSRR = (1 << 4); else GPIOB->BRR = (1 << 4); } while (0) // PD5 SDI
#define sct_clk(x) do { if (x) GPIOB->BSRR = (1 << 3); else GPIOB->BRR = (1 << 3); } while (0) // PD3 CLK
#define sct_noe(x) do { if (x) GPIOB->BSRR = (1 << 10); else GPIOB->BRR = (1 << 10); } while (0) // PD6 /OE
*/


void sct_init(void); //povoleni hodin prislusnych portu (RCC_AHBENR_GPIOB), nastaveni pinu jako vystupnich (GPIO_MODER_MODERx_0),
// zapis nul do posuvneho registru (volání sct_led(0)), aktivování výstupu pomocí /OE (volání sct_noe(0))

void sct_led(uint32_t value); //zapise hodnoty na displej

void sct_value(uint16_t value); //preklad cislovek na odpovidajici segmenty

#endif /* SCT_H_ */
