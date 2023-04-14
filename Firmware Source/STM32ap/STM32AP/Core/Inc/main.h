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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

// #define DEST_IP_ADDR0   192
// #define DEST_IP_ADDR1   168
// #define DEST_IP_ADDR2   0
// #define DEST_IP_ADDR3   30

#define DEST_PORT       7
 
/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   28
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

#define MAC_ADDR0   0x8f
#define MAC_ADDR1   0x7b
#define MAC_ADDR2   0xed
#define MAC_ADDR3   0x06
#define MAC_ADDR4   0x91
#define MAC_ADDR5   0xe0
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
void ENC_SPI1_Init(void);
void ENC_GPIO_Init(void);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
