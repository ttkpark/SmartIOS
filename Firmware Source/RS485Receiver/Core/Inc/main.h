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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define LCD_SCREEN_CLEAR() 0x01
#define LCD_CURSOR_RETURN() 0x02

//cursor set i_d:movind direction(1=increment 0=decrement) S:shift=1
#define LCD_INPUT_SET(i_d,S) (0x04 | i_d<<1 | S<<0)  

//D:display ON/OFF, C:cursor ON/OFF, B:Blink ON/OFF
#define LCD_DISPLAY_SWITCH(D,C,B) (0x08 | D<<2 | C<<1 | B<<0) 

//sc:(1=display shift 0=cursor shift) rl:(1=Right, 0=Left)
#define LCD_SHIFT(sc,rl) (0x10 | sc<<4 | rl<<3)

// Set dl:Datalength(1=8b,0=4b), n:display line(1=2 Lines, 0=1 Line), f:font(1=5x10,0=5x7)
#define LCD_FUNCTION_SET(dl,n,f) (0x20 | dl<<4 | n<<3 | f<<2)

#define RS_Inst 0
#define RS_Data 1

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
#define LED_Pin_Pin GPIO_PIN_13
#define LED_Pin_GPIO_Port GPIOC
#define USART2_DM_Pin GPIO_PIN_4
#define USART2_DM_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
