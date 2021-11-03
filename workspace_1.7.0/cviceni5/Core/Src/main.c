/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h> //aby fungoval vypis printf()

#include <string.h> //pro operace strtok a dalsi

#include <stdlib.h> //pro prevod retezcu vzniklych tokenizaci na cisla

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//Deklarace buffer a indexy cteni a zapisu
#define RX_BUFFER_LEN 64
static uint8_t uart_rx_buf[RX_BUFFER_LEN];
static volatile uint16_t uart_rx_read_ptr = 0;
//posouvani indexu cteni v bufferu dokun pointry cteni a zapisu nejsou stejne (zapis se posouva sam)
#define uart_rx_write_ptr (RX_BUFFER_LEN - hdma_usart2_rx.Instance->CNDTR)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define CMD_BUFFER_LEN 256 //vhodna velikost bufferu pro textovy prikaz

#define EEPROM_ADDR 0xA0 //0b10100000 // adresa EEPROM neboli = 0xA0
//#define I2C_MEMADD_SIZE_16BIT 16 //16 bitove adresovani EEPROM
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static void uart_byte_available(uint8_t c);
static void uart_process_command(char *cmd);
int _write(int file, char const *buf, int n);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//uklada tisknutelne znaky ASCII 32 az 126 do dalsiho bufferu, ktery obsahuje poskladany textovy prikaz
// az najde konec radku tak zavola obsluhu pro vyhodnoceni prikazu
static void uart_byte_available(uint8_t c) {
	static uint16_t cnt;
	static char data[CMD_BUFFER_LEN];
	if (cnt < CMD_BUFFER_LEN && c >= 32 && c <= 126)
		data[cnt++] = c;
	if ((c == '\n' || c == '\r') && cnt > 0) { //narazilo se na konec retezce
		data[cnt] = '\0'; //ukoncime nulou
		uart_process_command(data); //vypis celeho retezce
		cnt = 0;
	}
}

//vypis celeho retezce
static void uart_process_command(char *cmd) { //predani ukazatele
	printf("prijato: '%s'\n", cmd);

	char *token;

	uint8_t value;
	uint16_t addr;


	token = strtok(cmd, " ");
	if (strcasecmp(token, "HELLO") == 0) {
		printf("Komunikace OK\n");
	} else if (strcasecmp(token, "LED1") == 0) {
		token = strtok(NULL, " ");

		if (strcasecmp(token, "ON") == 0) {
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
		}

		else if (strcasecmp(token, "OFF") == 0) { //kdyz se rovna tak compare je 0
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
			printf("OK\n");
		}
	}

	else if (strcasecmp(token, "LED2") == 0) {
		token = strtok(NULL, " ");

		if (strcasecmp(token, "ON") == 0) {
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
		}

		else if (strcasecmp(token, "OFF") == 0) { //kdyz se rovna tak compare je 0 // strcasecmp zanedbani velikosti znaku
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
			printf("OK\n");
		}
	}

	else if (strcasecmp(token, "STATUS") == 0) {
		if (HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin)) {
			printf("LED1 ON\n");
		} else {
			printf("LED1 OFF\n");
		}

		if (HAL_GPIO_ReadPin(LED2_GPIO_Port, LED2_Pin)) {
			printf("LED2 ON\n");
		} else {
			printf("LED2 OFF\n");
		}
	}
// EEPROM


	else if (strcasecmp(token, "READ") == 0) {
		token = strtok(NULL, " ");
		addr = atoi(token);
		HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, addr, I2C_MEMADD_SIZE_16BIT, &value, 1, 1000);
		printf("na drese %d je hodnota %d\n", addr, value);

	}

	else if (strcasecmp(token, "WRITE") == 0) {
		token = strtok(NULL, " ");
		addr = atoi(token);
		token = strtok(NULL, " ");
		value = atoi(token);

		HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, addr, I2C_MEMADD_SIZE_16BIT, &value, 1, 1000);
		/* Check if the EEPROM is ready for a new operation */
		while (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_ADDR, 300, 1000) == HAL_TIMEOUT) {}
		printf("zapsano %d (%x) na adresu %d\n",value,value,addr);
		}


	else if (strcasecmp(token, "DUMP") == 0) {
		uint8_t dump_vect[16]; //vektor pro vypis pameti
		HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, 0x0000, I2C_MEMADD_SIZE_16BIT, dump_vect, 16, 1000); //pocatecni adresa je 0, dump_vect je uz pointr - nepise se *
		for ( uint8_t i = 0;  i < 16; i++) { //prochazeni vsech prvku

			printf("%02x ",dump_vect[i] ); //02 je po kolika clenech oddelovat a x ze je to hex cislo
			if (i == 7) {
				putchar('\n'); //novy radek po 8 bytech
			}

		}
		putchar('\n');
	}



}
//else if (strcasecmp...

// HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);

//funkce je definovana jako weak - tim se prepsala
int _write(int file, char const *buf, int n) {
	/* stdout redirection to UART2 */
	HAL_UART_Transmit(&huart2, (uint8_t*) (buf), n, HAL_MAX_DELAY);
	return n;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	HAL_UART_Receive_DMA(&huart2, uart_rx_buf, RX_BUFFER_LEN); //aktivace DMA cteni z UARTu
	//printf("prijato: '%s'\n", "text");

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		//uint8_t c;
		//HAL_UART_Receive(&huart2, &c, 1, HAL_MAX_DELAY); //klasicke polling - ceka se na jeden byte (znak) na 1 neznamenkovou promenou
		//HAL_UART_Transmit(&huart2, &c, 1, HAL_MAX_DELAY);

		//zpracovani jednotlivych bajtu z bufferu - kdyz dojde k zablokovani (po urcitou dobu ISR)
		// -> data se nahromadi v bufferu a zpracuji se jakmile je k dispozici procesorovy cas
		while (uart_rx_read_ptr != uart_rx_write_ptr) { //dokkud se cteci a zapisovaci pointr nerovna
			uint8_t b = uart_rx_buf[uart_rx_read_ptr];
			if (++uart_rx_read_ptr >= RX_BUFFER_LEN)
				uart_rx_read_ptr = 0; // increase read pointer
			uart_byte_available(b); // process every received byte with the RX state machine
		}

		////////////////////////
		// "set\0    led\0  on //postupne likviduje retezec a vraci jeho casti

		////////////////////////
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x2000090E;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 38400;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel4_5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, LED1_Pin | LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LED1_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LED1_Pin | LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : LED2_Pin */
	GPIO_InitStruct.Pin = LED2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
