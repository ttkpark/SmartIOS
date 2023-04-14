/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
 //#define USE_FULL_ASSERT
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
void process_http_client_request (unsigned char, unsigned long , unsigned char *, unsigned char *);
/*
IBEX UK LTD http://www.ibexuk.com
Electronic Product Design Specialists
RELEASED SOFTWARE

The MIT License (MIT)

Copyright (c) 2013, IBEX UK Ltd, http://ibexuk.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef Nop
#define Nop() __asm("nop")		//Null instruction
#endif



//----------------------
//----- OSCILLATOR -----
//----------------------
//Errata: Rev A devices: Code execution from internal flash is restricted to a maximum of 60MHz.
//4MHz Osc
//Fosc = 4Mhz
//CCLK (CPU Clock) = 32Mhz
//PCLK (Peripheral Clock) selectable per peripheral.  Default = cclk / 2 = 16Mhz
#define	INSTRUCTION_CLOCK_FREQUENCY				32000000




//----------------------
//----- INTERRUPTS -----
//----------------------
#define	ENABLE_INT	libarm_enable_irq()				//Enable all unmasket interupts
#define	DISABLE_INT	libarm_disable_irq()			//Disable all unmasket interupts

//----------------------
//----- WATCHDOG -----
//----------------------
#ifdef DEBUG
	#define	ClrWdt()   __asm("nop")
#else
	#define	ClrWdt()   DISABLE_INT; WDFEED = 0xaa; WDFEED = 0x55; ENABLE_INT					//Enable / reset the Watchdog timer by writing 0xAA followed by 0x55 to the WDFEED register
#endif







//****************************************
//****************************************
//***** GLOBAL DATA TYPE DEFINITIONS *****
//****************************************
//****************************************
#ifndef GLOBAL_DATA_TYPE_INIT				//(Include this section only once for each source file)
#define	GLOBAL_DATA_TYPE_INIT

#define	CONSTANT	const 				//Define used for this as some compilers require an additional qualifier such as 'rom' to signify that a constant should be stored in program memory

#undef BOOL
#undef TRUE
#undef FALSE
#undef BYTE
#undef SIGNED_BYTE
#undef WORD
#undef SIGNED_WORD
#undef DWORD
#undef SIGNED_DWORD

//BOOLEAN - 1 bit:
typedef enum _BOOL { FALSE = 0, TRUE } BOOL;
//BYTE - 8 bit unsigned:
typedef unsigned char BYTE;
//SIGNED_BYTE - 8 bit signed:
typedef signed char SIGNED_BYTE;
//WORD - 16 bit unsigned:
typedef unsigned short WORD;
//SIGNED_WORD - 16 bit signed:
typedef signed short SIGNED_WORD;
//DWORD - 32 bit unsigned:
typedef unsigned long DWORD;
//SIGNED_DWORD - 32 bit signed:
typedef signed long SIGNED_DWORD;
//QWORD - 64 bit unsigned:
typedef unsigned long long QWORD;
//SIGNED_QWORD - 64 bit signed:
typedef signed long long SIGNED_QWORD;

//BYTE BIT ACCESS:
typedef union _BYTE_VAL
{
    struct
    {
        unsigned char b0:1;
        unsigned char b1:1;
        unsigned char b2:1;
        unsigned char b3:1;
        unsigned char b4:1;
        unsigned char b5:1;
        unsigned char b6:1;
        unsigned char b7:1;
    } bits;
    BYTE Val;
} BYTE_VAL;

//WORD ACCESS
typedef union _WORD_VAL
{
    WORD val;
    struct
    {
        BYTE LSB;
        BYTE MSB;
    } byte;
    BYTE v[2];
} WORD_VAL;
#define LSB(a)          ((a).v[0])
#define MSB(a)          ((a).v[1])

//DWORD ACCESS:
typedef union _DWORD_VAL
{
    DWORD val;
    struct
    {
        BYTE LOLSB;
        BYTE LOMSB;
        BYTE HILSB;
        BYTE HIMSB;
    } byte;
    struct
    {
        WORD LSW;
        WORD MSW;
    } word;
    BYTE v[4];
} DWORD_VAL;
#define LOWER_LSB(a)    ((a).v[0])
#define LOWER_MSB(a)    ((a).v[1])
#define UPPER_LSB(a)    ((a).v[2])
#define UPPER_MSB(a)    ((a).v[3])

//EXAMPLE OF HOW TO USE THE DATA TYPES:-
//	WORD_VAL variable_name;				//Define the variable
//	variable_name = 0xffffffff;			//Writing 32 bit value
//	variable_name.LSW = 0xffff;			//Writing 16 bit value to the lower word 
//	variable_name.LOLSB = 0xff;			//Writing 8 bit value to the low word least significant byte
//	variable_name.v[0] = 0xff;			//Writing 8 bit value to byte 0 (least significant byte)

void up_udelay(uint32_t us);
void up_100ndelay(uint32_t us);
void up_delayTick(uint32_t tick);



#endif		//GLOBAL_DATA_TYPE_INIT


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GoodLED_Pin GPIO_PIN_0
#define GoodLED_GPIO_Port GPIOA
#define TestLED_Pin GPIO_PIN_1
#define TestLED_GPIO_Port GPIOA
#define ENC_SS_Pin GPIO_PIN_4
#define ENC_SS_GPIO_Port GPIOA
#define ENC_SCK_Pin GPIO_PIN_5
#define ENC_SCK_GPIO_Port GPIOA
#define ENC_MISO_Pin GPIO_PIN_6
#define ENC_MISO_GPIO_Port GPIOA
#define ENC_MOSI_Pin GPIO_PIN_7
#define ENC_MOSI_GPIO_Port GPIOA
#define ENC_RST_Pin GPIO_PIN_0
#define ENC_RST_GPIO_Port GPIOB
#define ENC_INT1_Pin GPIO_PIN_1
#define ENC_INT1_GPIO_Port GPIOB
#define ENC_INT1_EXTI_IRQn EXTI1_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
