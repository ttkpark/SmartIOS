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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx_ll_crs.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_exti.h"
#include "stm32l0xx_ll_cortex.h"
#include "stm32l0xx_ll_utils.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_dma.h"
#include "stm32l0xx_ll_spi.h"
#include "stm32l0xx_ll_usart.h"
#include "stm32l0xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#define Debug printf
#define USE_DHCP 1

#define FLASH_FLAG_BSY             FLASH_SR_BSY        /*!< FLASH Busy flag */
#define FLASH_FLAG_EOP             FLASH_SR_EOP        /*!< FLASH End of Programming flag */
#define FLASH_FLAG_ENDHV           FLASH_SR_HVOFF      /*!< FLASH End of High Voltage flag */
#define FLASH_FLAG_READY           FLASH_SR_READY      /*!< FLASH Ready flag after low power mode */
#define FLASH_FLAG_WRPERR          FLASH_SR_WRPERR     /*!< FLASH Write protected error flag */
#define FLASH_FLAG_PGAERR          FLASH_SR_PGAERR     /*!< FLASH Programming Alignment error flag */
#define FLASH_FLAG_SIZERR          FLASH_SR_SIZERR     /*!< FLASH Size error flag  */
#define FLASH_FLAG_OPTVERR         FLASH_SR_OPTVERR    /*!< FLASH Option Validity error flag  */
#define FLASH_FLAG_RDERR           FLASH_SR_RDERR      /*!< FLASH Read protected error flag */
#define FLASH_FLAG_FWWERR          FLASH_SR_FWWERR     /*!< FLASH Write or Errase operation aborted */
#define FLASH_FLAG_NOTZEROERR      FLASH_SR_NOTZEROERR /*!< FLASH Read protected error flag */


#define HAL_IS_BIT_SET(REG, BIT)         (((REG) & (BIT)) == (BIT))
#define HAL_IS_BIT_CLR(REG, BIT)         (((REG) & (BIT)) == 0U)

#define __HAL_FLASH_GET_FLAG(__FLAG__)   (((FLASH->SR) & (__FLAG__)) == (__FLAG__))


#define FLASH_SIZE                (uint32_t)((*((uint32_t *)FLASHSIZE_BASE)&0xFFFF) * 1024U)
#define FLASH_PAGE_SIZE           (128U)  /*!< FLASH Page Size in bytes */

#define FLASH_END                 (FLASH_BASE + FLASH_SIZE - 1)    /*!< FLASH end address in the alias region */
#define __HAL_FLASH_CLEAR_FLAG(__FLAG__)   ((FLASH->SR) = (__FLAG__))
#define FLASH_PDKEY1               (0x04152637U) /*!< Flash power down key1 */
#define FLASH_PDKEY2               (0xFAFBFCFDU) /*!< Flash power down key2: used with FLASH_PDKEY1
                                                    to unlock the RUN_PD bit in FLASH_ACR */

#define FLASH_PEKEY1               (0x89ABCDEFU) /*!< Flash program erase key1 */
#define FLASH_PEKEY2               (0x02030405U) /*!< Flash program erase key: used with FLASH_PEKEY2
                                                     to unlock the write access to the FLASH_PECR register and
                                                     data EEPROM */

#define FLASH_PRGKEY1              (0x8C9DAEBFU) /*!< Flash program memory key1 */
#define FLASH_PRGKEY2              (0x13141516U) /*!< Flash program memory key2: used with FLASH_PRGKEY2
                                                     to unlock the program memory */

#define FLASH_OPTKEY1              (0xFBEAD9C8U) /*!< Flash option key1 */
#define FLASH_OPTKEY2              (0x24252627U) /*!< Flash option key2: used with FLASH_OPTKEY1 to
                                                              unlock the write access to the option byte block */
/**
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */


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

uint32_t rando();
#endif		//GLOBAL_DATA_TYPE_INIT

// SUCCEED = 0
// FAILED = 1
typedef enum {
    SUCCEED = 0,
    FAILED
} Result;



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
#define LED_LINE_Pin LL_GPIO_PIN_0
#define LED_LINE_GPIO_Port GPIOA
#define LED_SENSOR_Pin LL_GPIO_PIN_1
#define LED_SENSOR_GPIO_Port GPIOA
#define ENC_SS_Pin LL_GPIO_PIN_4
#define ENC_SS_GPIO_Port GPIOA
#define RST_Pin LL_GPIO_PIN_0
#define RST_GPIO_Port GPIOB
#define MEM_CS_Pin LL_GPIO_PIN_8
#define MEM_CS_GPIO_Port GPIOA
#define BLE_SS_Pin LL_GPIO_PIN_9
#define BLE_SS_GPIO_Port GPIOA
#define BLE_RST_Pin LL_GPIO_PIN_11
#define BLE_RST_GPIO_Port GPIOA
#define USART1_DE_Pin LL_GPIO_PIN_5
#define USART1_DE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
