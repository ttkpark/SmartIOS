/**
  ******************************************************************************
  * @file    LwIP/LwIP_TCP_Echo_Server/Src/ethernetif.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "ethernetif.h"
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;
extern void up_udelay(uint32_t );

// void MX_SPI1_Init(void)
// {

//   hspi1.Instance = SPI1;
//   hspi1.Init.Mode = SPI_MODE_MASTER;
//   hspi1.Init.Direction = SPI_DIRECTION_2LINES;
//   hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
//   hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
//   hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
//   hspi1.Init.NSS = SPI_NSS_SOFT;
//   hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
//   hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
//   hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
//   hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//   hspi1.Init.CRCPolynomial = 7;
//   hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
//   hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
//   HAL_SPI_Init(&hspi1);
// }

// void ENC_Init()
// {
//   printf("ENC28J60 Resets!!\n");
  
//   GPIO_InitTypeDef GPIO_InitStruct = {0};

//   /* Enable GPIOs clocks */
//   __HAL_RCC_GPIOA_CLK_ENABLE();
//   __HAL_RCC_GPIOB_CLK_ENABLE();

// /* ENC28J60 pins configuration ************************************************/
//   /*
//         ENC28J60_CS ----------------------> PA4
//         ENC28J60_INT ---------------------> PB0
//   */

//   /*Configure GPIO pins : PA4 */
//   GPIO_InitStruct.Pin = GPIO_PIN_4;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
//   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//   /* Deselect ENC28J60 module */
//   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

// #ifdef ENC28J60_INTERRUPT
//   /*Configure GPIO pin : PB0 */
//   GPIO_InitStruct.Pin = GPIO_PIN_0;
//   GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
// #endif /* ENC28J60_INTERRUPT */

//   /* Initialize SPI */
//   MX_SPI1_Init();

// #ifdef ENC28J60_INTERRUPT
//   /* EXTI interrupt init*/
//   /* Sets the priority grouping field */
//   HAL_NVIC_SetPriority(EXTI1_IRQn, 4, 0);
//   HAL_NVIC_EnableIRQ(EXTI1_IRQn);
// #endif /* ENC28J60_INTERRUPT */

// }

/**
  * @brief  Returns the current time in milliseconds
  *         when LWIP_TIMERS == 1 and NO_SYS == 1
  * @param  None
  * @retval Current Time value
  */
uint32_t sys_now(void)
{
  return HAL_GetTick();
}

/**
  * Implement SPI single byte send and receive.
  * The ENC28J60 slave SPI must already be selected and wont be deselected after transmission
  * Must be provided by user code
  * param  command: command or data to be sent to ENC28J60
  * retval answer from ENC28J60
  */

uint8_t ENC_SPI_SendWithoutSelection(uint8_t command)
{
    HAL_SPI_TransmitReceive(&hspi1, &command, &command, 1, 1000);
    return command;
}

uint8_t ENC_SPI_Send(uint8_t command)
{
    /* Select ENC28J60 module */
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_RESET);
    up_udelay(1);

    HAL_SPI_TransmitReceive(&hspi1, &command, &command, 1, 1000);

    /* De-select ENC28J60 module */
    HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_SET);
    up_udelay(1);

    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    return command;
}

void ENC_SPI_SendBuf(uint8_t *master2slave, uint8_t *slave2master, uint16_t bufferSize)
{
    /* Select ENC28J60 module */
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_RESET);
    up_udelay(1);

    /* Transmit or receuve data */
    if (slave2master == NULL) {
        if (master2slave != NULL) {
            HAL_SPI_Transmit(&hspi1, master2slave, bufferSize, 1000);
        }
    } else if (master2slave == NULL) {
        HAL_SPI_Receive(&hspi1, slave2master, bufferSize, 1000);
    } else {
        HAL_SPI_TransmitReceive(&hspi1, master2slave, slave2master, bufferSize, 1000);
    }

    /* De-select ENC28J60 module */
    HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_SET);
    up_udelay(1);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void ENC_SPI_Select(uint8_t select)
{
    /* Select or de-select ENC28J60 module */
    if (select) {
        #ifdef ENC28J60_INTERRUPT
        HAL_NVIC_DisableIRQ(EXTI1_IRQn);
        #endif

        HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_RESET);
        up_udelay(1);
    } else {
        HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_SET);
        up_udelay(1);

        #ifdef ENC28J60_INTERRUPT
        HAL_NVIC_EnableIRQ(EXTI1_IRQn);
        #endif
    }
}