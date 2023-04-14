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
#include "Internet.h"
#include "AttenderAPI.h"
#include "Coder.h"
#include "cJSON.h"
#include "memory.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// first settings
Date nowDate;
cJSON* saveDataJSON;
SavedData saveData;

const char * jsonstr = 
  "{"
  "\"apcode\": 1,"
  "\"apip\": \"192.168.0.146\","
  "\"domain\": \"http://www.eduwon.net:8807\","
  "\"netmask\": \"255.255.255.0\","
  "\"gwip\": \"192.168.0.1\","
  "\"dnsip\": \"168.126.63.1\","
  "\"dns2ip\": \"0.0.0.0\","
  "\"useyn\": true,"
  "\"dist\": 8.3,"
  "\"idxClient\": 1000,"
  "\"isDHCP\": true,"
  "\"IdentifyDistance\": 100,"
  "\"BeaconTimeout\": 20,"
  "\"ServerURL\": \"/api/sio/\","
  "\"dbuglog\": false,"
  "\"reset\": -1,"
  "\"enableOutDetecting\": 0,"
  "\"ipcdInterval\": 600,"
  "\"date\": \"20210605172213\","
  "\"pgver\": \"v.kr.0.1\","
  "\"pgverdate\": \"20210605100630\","
  "\"CDMURL\": \"/api/cdm/\""
  "}";

SensorList sensorlist[SENSORLIST_LEN];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char* p, int len)
{
  HAL_UART_Transmit(&huart1,(uint8_t*)p,len,1000);
  return len;
}
int mode = 0;
uint32_t rando()
{
  int t = HAL_GetTick();
  return ((t&0x00FF)<<16 | (t&0xFF00)>>16) * ((t&0x0F)<<8 | (t&0xF0)>>8) - t;
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
  calibrate();
  Addstr_SystemInitalize();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  extern BOOL isOnline;
  isOnline = -2;
  
  HAL_GPIO_WritePin(MEM_CS_GPIO_Port,MEM_CS_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(ENC_SS_GPIO_Port,ENC_SS_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(BLE_RST_GPIO_Port,BLE_RST_Pin,GPIO_PIN_RESET);
  HAL_Delay(50);
  HAL_GPIO_WritePin(BLE_RST_GPIO_Port,BLE_RST_Pin,GPIO_PIN_SET);

  Debug("start!\n"); 

  memset(sensorlist,0,sizeof(sensorlist));

  MEM_INIT();
  
  // cJSON *json = cJSON_Parse(jsonstr);
  // Store_SaveData(json);
  // cJSON_Delete(json);


  if(MEM_Read_SaveData()==FAILED)
  {
    printf("important!! Reading Configuration failed.\n");
    while(1)
    {
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
      HAL_Delay(500);
      
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
      HAL_Delay(500);
    }
  }

  //MEM_Store_SaveData();

  char* str = cJSON_Print(saveDataJSON);
  printf("savedData : %s\n",str);
  free(str);

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim2);

  MAC_ADDR myMAC = MACSet(0x12,0x23,0x34,0x45,0x56,0x67);

  //if it is blocked here, Please make SPI lines stablized. It may stuck during the SPI progress.
  if(saveData.isDHCP)printf("Starting DHCP...\n");
  EC_Init(saveData.apip,saveData.netmask,saveData.gwip,myMAC,saveData.isDHCP);
  //EC_Init(IPSet(192,168,0,28),IPSet(255,255,255,255),IPSet(192,168,0,1),myMAC,0);

  int ipcdTimer = HAL_GetTick() - saveData.ipcdInterval*1000;
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    //connecting process..
    Debug("connecting line...\n"); 
	
    uint32_t start = HAL_GetTick();
    while((HAL_GetTick()-start < 1000) && !(nic_is_linked))yield();
    if(!nic_is_linked)
    {
      Debug("nic is NOT linked...\n"); 
      nic_initialise(NIC_INIT_SPEED);
      Debug("nic was initalized.\n");
      continue;
    }
    else Debug("nic is linked!...\n"); 


    while((HAL_GetTick()-start < 30000) && !(nic_linked_and_ip_address_valid))yield();
    if(!nic_linked_and_ip_address_valid)
         Debug("Obtaining IP failed...\n"); 
    else Debug("It's online!\n"); 
    checkConnectivity(TRUE);

    if(isOnline == 0)continue;//retry internet.

    //echo test + Updating Date
    Result echo_res;
    for(int i=0;i<4;i++)
    {
      echo_res = API_Echo();
      if(echo_res == SUCCEED)break;
    }
    if(echo_res != SUCCEED)nic_initialise(NIC_INIT_SPEED);

    Debug("system start!\n");
    while (1)
    {
      if(isOnline == 0)break;//retry internet.


      //READ the Pin_READY. (0:waiting 1:free)
      if(HAL_GPIO_ReadPin(BLE_READY_GPIO_Port,BLE_READY_Pin) == GPIO_PIN_RESET)
      {
        uint8_t txbuffer[8] = "this.TX";
        BLEInfo rxbuffer[1] = {0,};

        HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_RESET);
        HAL_SPI_Receive(&hspi1, rxbuffer, sizeof(rxbuffer), 1000);
        HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_SET);
        if(rxbuffer[0].AESData[0] != 0 && rxbuffer[0].AESData[1] != 0xFF)
        {
          //Debug("LE%d(%d)\n",rxbuffer[0].AESNo,rxbuffer[0].rssi);

          // Debug("AESNo : %d\3",rxbuffer[0].AESNo);
          // Debug("rssi  : %d\3",rxbuffer[0].rssi);
          // Debug("txPWR : %d\3",rxbuffer[0].txPower);

          // Debug("SPI SEND : %s\3",txbuffer);
          // uint8_t* pointer = rxbuffer;
          // Debug("{%02X %02X %02X %02X %02X %02X %02X %02X}\3"
          //   ,pointer[0],pointer[1],pointer[2],pointer[3]
          //   ,pointer[4],pointer[5],pointer[6],pointer[7]);
          SensorData sensor;
          Result res = BLEList_ProcessSensor(rxbuffer,&sensor);
          //Debug("BLEList_ProcessSensor %s \n",(res==SUCCEED?"Succeed":"failed"));
          if(res==FAILED)checkConnectivity(TRUE);
          else
          {
            int i=0;
            BOOL isinRange = sensor.distance < saveData.IdentifyDistance;
            //search
            for(i=0;i<SENSORLIST_LEN;i++)
            {
              if(sensorlist[i].idxUser==sensor.idxUser)break;
            }
            if(i<SENSORLIST_LEN);//if found
            else //unless found
            {
              for(i=0;i<SENSORLIST_LEN;i++)
              {
                if(sensorlist[i].isAlive == FALSE)break;
              }
              if(i>=SENSORLIST_LEN)printf("ERROR! sensorLIST FULL\n");
              
              sensorlist[i].ioState = 1;//OUT
            }
            //printf("device %d\n",i);
            sensorlist[i].idxUser = sensor.idxUser;
            sensorlist[i].isAlive = isinRange;
            sensorlist[i].beginTime = HAL_GetTick();
            sensorlist[i].distance = sensor.distance;
            sensorlist[i].hackingCode = sensor.HackingCode;

            
            //investigate if It's entering range(0,saveData.dist)&&iostate:out
            if(sensorlist[i].ioState == 1 && sensorlist[i].distance < saveData.dist)
            {
              printf("NEW Sensor entered!\n");
              Result res = Beacon_SendAPI(sensor);
              
              Debug("Beacon_SendAPI %s \n",(res==SUCCEED?"Succeed":"failed"));
              
              sensorlist[i].ioState = 0;//IN
            }

            
            //then now search if It's exiting range(saveData.dist,INF)&&iostate:in&&saveData.enableOutDetecting==true
            else if(sensorlist[i].ioState == 0 && sensorlist[i].distance >= saveData.dist)
            {
              sensorlist[i].ioState = 1;//OUT
              printf("Sensor %d exited!\n",i);
              if(saveData.enableOutDetecting == TRUE){
                Result res = Beacon_SendAPI(sensor);
                Debug("Beacon_SendAPI %s \n",(res==SUCCEED?"Succeed":"failed"));
              }
            }
            
            //Beacon_SendAPI(decryptedSensor);
          }
        }
      }
      //else Debug("empty line..\n");
      
      if(mode == 0)
      {
        mode = 3;
      }
      else if(mode == 2)
      {

      }
      else if(mode == 3 && isOnline)
      {
        //Debug("Send_API_Echo~\n");
        // mode = 1;
        // if(isOnline)
        // if(API_Echo() == FAILED)
        //  checkConnectivity(TRUE);
        
      }

      //IPCD Timer Trigger
      if(HAL_GetTick() - ipcdTimer > saveData.ipcdInterval*1000)
      {
        ipcdTimer = HAL_GetTick();
        Result res = API_ipcd();
        if(res != SUCCEED)
        {
          printf("IPCD Error\n");
        }
      }
      
      static int printtimer;
      if(HAL_GetTick() - printtimer > 1000)
      {
        printtimer = HAL_GetTick();

        // printf("sensorlist GET\n");
        for(int i=0;i<SENSORLIST_LEN;i++)
        {
          if(sensorlist[i].idxUser == 0)break;

          SensorList *row = &sensorlist[i];
          printf("\n Sensor[%d] : idx:%d %s T:%dms dist:%gm hack:%04d io:%s\n",i,row->idxUser,row->isAlive?"Alive":"Dead",HAL_GetTick() - row->beginTime
            ,row->distance,row->hackingCode,!row->ioState?"in":"out");
        }
      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      yield();
    }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 32000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 32-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1000-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
  huart1.Init.BaudRate = 115200;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ENC_SS_GPIO_Port, ENC_SS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RST_Pin|LD4_Pin|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MEM_CS_Pin|BLE_SS_Pin|BLE_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_SS_Pin */
  GPIO_InitStruct.Pin = ENC_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ENC_SS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RST_Pin LD4_Pin LD3_Pin */
  GPIO_InitStruct.Pin = RST_Pin|LD4_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_Pin */
  GPIO_InitStruct.Pin = INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MEM_CS_Pin BLE_SS_Pin BLE_RST_Pin */
  GPIO_InitStruct.Pin = MEM_CS_Pin|BLE_SS_Pin|BLE_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_READY_Pin */
  GPIO_InitStruct.Pin = BLE_READY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLE_READY_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
