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
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
//#define SetPinParams(RS,E,D7,D6,D5,D4) (RS<<7 | E<<6 | D7<<3 | D6<<2 | D5<<1 | D4<<0)
void ToggleE()
{
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, RESET);
  HAL_Delay(1);
}
void SetRS(uint8_t RS)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, RS);
}
void SetData4(uint8_t Data)
{
  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, Data&0x80);
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, Data&0x40);
  HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, Data&0x20);
  HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, Data&0x10);
  ToggleE();
}
void Inst4Bit(uint8_t inst)
{
  SetRS(RS_Inst);
  SetData4(inst);
}
void Inst8Bit(uint8_t inst)
{
  SetRS(RS_Inst);
  SetData4(inst);
  SetData4(inst<<4);
}
void Data8Bit(uint8_t data)
{
  SetRS(RS_Data);
  SetData4(data);
  SetData4(data<<4);
}
void init_LCD()
{
  HAL_Delay(15);//Wait 15ms

  Inst4Bit(LCD_FUNCTION_SET(1,0,0));
  HAL_Delay(5);//Wait 4.1ms

  Inst4Bit(LCD_FUNCTION_SET(1,0,0));
  HAL_Delay(1);//Wait 100us
  
  Inst4Bit(LCD_FUNCTION_SET(0,0,0));//4-bit length
  HAL_Delay(10);//Wait 10ms
  Inst8Bit(LCD_FUNCTION_SET(0,1,0));//4-bit length
  HAL_Delay(10);//Wait 10ms
  Inst8Bit(LCD_FUNCTION_SET(0,1,0));//4-bit length
  HAL_Delay(10);//Wait 10ms
  Inst8Bit(LCD_SCREEN_CLEAR());//disp clear
  HAL_Delay(10);//Wait 10ms
  Inst8Bit(LCD_DISPLAY_SWITCH(1,0,0));//disp ON ,cursor OFF
  HAL_Delay(10);//Wait 10ms
  Inst8Bit(LCD_INPUT_SET(1,0));//cursor increment
  HAL_Delay(10);//Wait 10ms

  Inst8Bit(LCD_SCREEN_CLEAR());//disp clear
  HAL_Delay(10);//Wait 10ms
}
void Write_character(int pos_x,int pos_y,char* ch)
{
  Inst8Bit(0x80 | (pos_y==1)<<6 | pos_x);

  while(*ch != 0)
    Data8Bit(*(ch++));
}
*/

void UARTReceiver_callback();
void UARTReceiver_Process(char* msg);


typedef enum _BOOL { FALSE = 0, TRUE } BOOL;

uint8_t UARTBufferfromRS485[128] = {0,};
uint32_t old_pos_fromRS485 = 0;

uint8_t UARTBufferfromUART[128] = {0,};
uint32_t old_pos_fromUART = 0;

void UARTReceiver_init()
{
  HAL_UART_Receive_DMA(&huart1,UARTBufferfromUART,sizeof(UARTBufferfromUART));
  HAL_UART_Receive_DMA(&huart2,UARTBufferfromRS485,sizeof(UARTBufferfromRS485));
  HAL_GPIO_WritePin(USART2_DM_GPIO_Port,USART2_DM_Pin,RESET);//Listening
}
void ProcessUARTData(UART_HandleTypeDef* huart,uint32_t *old_pos,uint8_t *DMABuffer,uint32_t DMABufferSize,void (*callback)(uint8_t*,size_t));
void SendDatatoUART(uint8_t* buf,size_t len);
void SendDatatoRS485(uint8_t* buf,size_t len);

BOOL UART_Ended = TRUE;
void UARTReceiver_callback()
{
  /*
    if(readch == 0x03){
        UART_Ended = TRUE;
        UARTBuffer[UARTBufferfocus] = 0;
        UARTBufferfocus=0;
        UARTReceiver_Process(UARTBuffer);
        HAL_GPIO_TogglePin(LED_Pin_GPIO_Port,LED_Pin_Pin);
    }

    if(!UART_Ended)
    {
        UARTBuffer[UARTBufferfocus++] = readch;
        if(UARTBufferfocus >= sizeof(UARTBuffer))
        {
            readch = 0x03;
            UARTReceiver_callback();
        }
    }

    if(readch == 0x02){
        UART_Ended = FALSE;
        UARTBuffer[0] = 0;
        UARTBufferfocus = 0;
    }
    HAL_UART_Receive_IT(&huart2,&readch,1);
    */
}
char buf[16];
void UARTReceiver_Process(char* msg)
{
  printf("msg %s \n",msg);
}
int _write(int file, char* p, int len)
{
  HAL_UART_Transmit(&huart1,(uint8_t*)p,len,1000);
  return len;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  printf("Start!!\n");
  //init_LCD();
  
  //Write_character(0,0,"Waiting for Data");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  UARTReceiver_init();
  //HAL_GPIO_WritePin(USART2_DM_GPIO_Port,USART2_DM_Pin,SET);//Listening
  while (1)
  {
    //Data8Bit('H');
    //HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    ProcessUARTData(&huart2,&old_pos_fromRS485,UARTBufferfromRS485,sizeof(UARTBufferfromRS485),SendDatatoUART);
    ProcessUARTData(&huart1,&old_pos_fromUART,UARTBufferfromUART,sizeof(UARTBufferfromUART),SendDatatoRS485);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 256000;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 256000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_Pin_GPIO_Port, LED_Pin_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USART2_DM_GPIO_Port, USART2_DM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin_Pin */
  GPIO_InitStruct.Pin = LED_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_Pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USART2_DM_Pin */
  GPIO_InitStruct.Pin = USART2_DM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USART2_DM_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void ProcessUARTData(UART_HandleTypeDef* huart,uint32_t *old_pos,uint8_t *DMABuffer,uint32_t DMABufferSize,void (*callback)(uint8_t*,size_t))
{
  uint32_t data_dma_pos = huart->RxXferSize - huart->hdmarx->Instance->CNDTR;
  uint32_t *dma_pos = &data_dma_pos;
  //DMA's data position

  //printf("data_dma_pos : %d = %d-%d\n",*dma_pos,huart->RxXferSize, huart->hdmarx->Instance->CNDTR);
  //printf("data_old_pos : %d\n",*old_pos);

  uint8_t sendDataBuffer[sizeof(UARTBufferfromRS485)-1];
  size_t rx_size = 0;

  // 이전 위치와 현재 위치가 다르다면 (= 데이터 처리 필요)
  if(*dma_pos != *old_pos)
  {
    //현재 위치 > 이전 위치 : 이전 위치에서 현재 위치까지 데이터를 가져옴.
    if(*dma_pos > *old_pos)
    {
      //이 차이만큼이 수신 바이트 크기가 된다.
      rx_size = *dma_pos - *old_pos;

      //데이터를 가져온다.
      memcpy(sendDataBuffer,DMABuffer + *old_pos,rx_size);
    }
    //현재 위치 < 이전 위치 : Circular Buffer에서 Overfolw되어 한 바퀴 넘어갔다.
    //첫번쩨로 현재부터 버퍼의 끝까지, 두번째로 처음부터 현재 위치까지 가져온다.
    else
    {
      // 현재 위치에 버퍼 길이를 더해준 위치와 이전위치의 차이가 수신 바이트 크기가 된다.
      rx_size = *dma_pos + sizeof(UARTBufferfromRS485) - *old_pos;

      //첫번째 전송을 위한 내용 : 버퍼끝까지 남은 바이트수
      uint32_t remain_size = sizeof(UARTBufferfromRS485) - *old_pos;

      //printf("buffer(d%d,o%d,%d,%d)\n",*dma_pos,*old_pos,rx_size,remain_size);

      // 첫번째 복사를 완료한다.
      //printf("0<-%d : l=%d\n",*old_pos,remain_size);
      memcpy(sendDataBuffer,DMABuffer + *old_pos,remain_size);

      // 두번째 복사를 완료한다.
      //printf("%d<-0 : l=%d\n",remain_size,*dma_pos);
      memcpy(sendDataBuffer + remain_size,DMABuffer,*dma_pos);
    }
    
    sendDataBuffer[rx_size] = 0;//null termination
    *old_pos = *dma_pos;

    if(callback != NULL)
      callback(sendDataBuffer,rx_size);
  }
}
void SendDatatoUART(uint8_t* buf,size_t len)
{
  //printf("485->UART(%d) : %s\n",len, buf);
  HAL_UART_Transmit(&huart1,buf,len,1000);
}
void SendDatatoRS485(uint8_t* buf,size_t len)
{
  //printf("UART->485(%d) : %s\n",len, buf);
  HAL_GPIO_WritePin(USART2_DM_GPIO_Port,USART2_DM_Pin,SET);
  HAL_UART_Transmit(&huart2,buf,len,1000);
  HAL_GPIO_WritePin(USART2_DM_GPIO_Port,USART2_DM_Pin,RESET);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
