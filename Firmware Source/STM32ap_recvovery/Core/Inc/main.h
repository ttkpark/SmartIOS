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

void up_udelay(uint64_t us);
void up_100ndelay(uint64_t us);
void up_delayTick(uint64_t tick);

#endif		//GLOBAL_DATA_TYPE_INIT

// SUCCEED = 0
// FAILED = 1
typedef enum {
    SUCCEED = 0,
    FAILED
} Result;

typedef struct{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} Date;

// SUCCEED = 0
// TIMEOUT = 1
// DISCONNECTED = 2
// RETRIES_EXPIRED = 3
typedef enum {
    SUCCEEDED = 0,
    TIMEOUT,
    DISCONNECTED,
    RETRIES_EXPIRED
} RES_GetWait;

typedef BYTE SOCKET;
typedef uint32_t LENGTH;

typedef struct
{
    LENGTH len;
    uint8_t* buffer;
} CoderBuff, *CoderBuffer;


//BLE Sensor structure
#define BLESendLen 10
typedef struct{
  uint8_t AESData[16];
  uint8_t AESNo;
  int8_t  rssi;
  int8_t  txPower;
}BLEInfo;

typedef struct 
{
  Date sensorDate;
  int HackingCode;
  int C;
  int idxUser;
  float distance;
} SensorData;

#define SENSORLIST_LEN 100
typedef struct 
{
  BOOL isAlive;
  int ioState;
  int idxUser; // creteria of distingushing sensors
  uint16_t hackingCode; // this value will not vary whinin 10sec.
  uint32_t beginTime; //time of acknowledging
  uint32_t exitTime; //time of exiting
  float distance; //distance
  Date date;//sensor packet data from BLE
  int C;//sensor packet data from BLE

  BOOL needtobeSended;
} SensorList;


/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

uint32_t rando();
void resetNetwork();
void User_DeInit();

void FLASH_MEM_Init();
BOOL shouldUpdateProgramVersion();
void updateProgramVersion();
BOOL checkAPCodeisInvalid();
void flash_reset();
Result flash_resetSucceed();
void system_break();

BOOL CheckIfOnline();
void process_Netdog();

void Timer_print();
void Timer_ipcd();
void Main_APCode0();
BOOL shouldFirmwareBackup();
void checkJSONValid();
Result updatepgRunningFlag(uint8_t value);
//Result UpdateAesnoAsDefault();
void LEDBlink(int count,uint32_t duration);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_LINE_Pin GPIO_PIN_0
#define LED_LINE_GPIO_Port GPIOA
#define LED_SENSOR_Pin GPIO_PIN_1
#define LED_SENSOR_GPIO_Port GPIOA
#define ENC_SS_Pin GPIO_PIN_4
#define ENC_SS_GPIO_Port GPIOA
#define RST_Pin GPIO_PIN_0
#define RST_GPIO_Port GPIOB
#define INT_Pin GPIO_PIN_1
#define INT_GPIO_Port GPIOB
#define MEM_CS_Pin GPIO_PIN_8
#define MEM_CS_GPIO_Port GPIOA
#define BLE_SS_Pin GPIO_PIN_9
#define BLE_SS_GPIO_Port GPIOA
#define BLE_READY_Pin GPIO_PIN_10
#define BLE_READY_GPIO_Port GPIOA
#define BLE_RST_Pin GPIO_PIN_11
#define BLE_RST_GPIO_Port GPIOA
#define SIGNAL24V_Pin GPIO_PIN_4
#define SIGNAL24V_GPIO_Port GPIOB
#define USART1_DE_Pin GPIO_PIN_5
#define USART1_DE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
