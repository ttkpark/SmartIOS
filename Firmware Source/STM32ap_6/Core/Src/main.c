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
#include "ETH/Internet.h"
#include "API/AttenderAPI.h"
#include "buffer/CoderBuffer.h"
#include "buffer/aes.h"
#include "buffer/base64.h"
#include "JSON/cJSON.h"
#include "memory/memory.h"
#include "programmer/programmer.h"
#include "Sensor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define smartios_versionCode "v.A.kor.0.117"
#define smartios_versionDate "20220728220830"

const char* SMARTIOS_VER  = smartios_versionCode;
const char* SMARTIOS_DATE = smartios_versionDate;

// first settings
Date nowDate;
cJSON* saveDataJSON;
SavedData saveData;

#define USE_JSONSTR_RECOVER

#ifdef USE_JSONSTR_RECOVER
const char * jsonstr = 
  "{"
  "\"apcode\": 0,"
  "\"apip\": \"\","
  "\"domain\": \"\","
  "\"netmask\": \"\","
  "\"gwip\": \"\","
  "\"dnsip\": \"\","
  "\"dns2ip\": \"\","
  "\"useyn\": true,"
  "\"dist\": 0,"
  "\"idxClient\": 1,"
  "\"isDHCP\": true,"
  "\"IdentifyDistance\": 100,"
  "\"BeaconTimeout\": 20,"
  "\"ServerURL\": \"\","
  "\"dbuglog\": false,"
  "\"reset\": 0,"
  "\"enableOutDetecting\": 0,"
  "\"ipcdInterval\": 1800,"
  "\"date\": \"20210610172213\","
  "\"pgver\": \""smartios_versionCode"\","
  "\"pgverdate\": \""smartios_versionDate"\","
  "\"CDMURL\": \"\","
  "\"aes_no\": 0"
  "}";
//Adding "val" into SavedData
//1. add "val" into jsonstr.
//2. add "val" into struct SavedData and into struct SaveDataUpdate
//3. add 'parsing "val" element' code into of MEM_make_SavedData
//4. Modify MEM_ModifyMemory at two places
//  4-1. add statement if aes_no is nullptr.
//  4-2. add one line near the array of cJSON_ReplaceItemInObject.
//5. test it!
#endif

SensorList sensorlist[SENSORLIST_LEN];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
BOOL isCountNetWDG = FALSE;
uint32_t countNetWDG = 0;
uint32_t ipcdTimer;
uint32_t netCheckTimer;
uint32_t printtimer;
uint32_t SIGNAL_24V_Timer; BOOL isSIGNAL24V_Active = FALSE;
SOCKET debug_udp = UDP_INVALID_SOCKET;
IP_ADDR debug_udpIP = {255,255,255,255};
MAC_ADDR myMAC;

extern int IPCDResultCode;
int mode = 0;

uint8_t AESKeyBuffer[48] = {0,};
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
  MX_USART2_UART_Init();
  MX_ADC_Init();
  MX_IWDG_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  UARTReceiver_init();
  printf("booting...\n");



  extern BOOL isOnline;
  isOnline = -2;
  
  HAL_GPIO_WritePin(MEM_CS_GPIO_Port,MEM_CS_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(ENC_SS_GPIO_Port,ENC_SS_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(BLE_RST_GPIO_Port,BLE_RST_Pin,GPIO_PIN_RESET);
  HAL_Delay(50);
  HAL_GPIO_WritePin(BLE_RST_GPIO_Port,BLE_RST_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(SIGNAL24V_GPIO_Port,SIGNAL24V_Pin,TRUE);

  printf("Starting SMARTIOS AP..\n");

  if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
  {
    printf("This reboot is WATCHDOG Reset!!\n");
    __HAL_RCC_CLEAR_RESET_FLAGS();
  }

  memset(sensorlist,0,sizeof(sensorlist));

  FLASH_MEM_Init();

  printf("----------------------------------------------\n");
  printf("Firmware Verson : %s\n",saveData.pgver ? saveData.pgver : "null");
  printf("Firmware Date   : %s\n",Convert_Date2Str(saveData.pgverdate));
  printf("----------------------------------------------\n");


  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim2);

  extern uint8_t UUID[16];

  myMAC = MACSet(0x12,0x23,0x34,(UUID[0]+UUID[3]+UUID[6]+UUID[9]+UUID[12]+UUID[15])&0xFF,(UUID[1]+UUID[4]+UUID[7]+UUID[10]+UUID[13])&0xFF,(UUID[2]+UUID[5]+UUID[8]+UUID[11]+UUID[14]) & 0xFF);

  //if it is blocked here, Please make SPI lines stablized. It may stuck during the SPI progress.
  if(saveData.isDHCP)printf("Starting DHCP...\n");
  EC_Init(saveData.apip,saveData.netmask,saveData.gwip,myMAC,saveData.isDHCP);
  //EC_Init(IPSet(192,168,0,28),IPSet(255,255,255,255),IPSet(192,168,0,1),myMAC,0);

  ipcdTimer = HAL_GetTick() - saveData.ipcdInterval*1000;
  countNetWDG = HAL_GetTick();
  
  checkConnectivity(TRUE);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { 
    yield();
    if(CheckIfOnline() != TRUE)continue;
    
    Debug("system start!\n");

    checkConnectivity(TRUE);
    Start_DebugUDP();
    while (nic_linked_and_ip_address_valid && isOnline)//retry internet.
    {
      if(saveData.apcode == 0)//is APCode0 Mode(if data lost)
        Main_APCode0();
      else
      {
        FetchBLEData();
        SensorListProcess();
        
        //IPCD Timer
        if(HAL_GetTick() - ipcdTimer > saveData.ipcdInterval*1000)
          Timer_ipcd();
        
        //Print Timer
        if(HAL_GetTick() - printtimer > 1000*60*5) //5min(300s) timer
          Timer_print();

        //NetChecker Timer
        if(HAL_GetTick() - netCheckTimer > 1000*10){
          netCheckTimer = HAL_GetTick();
          checkConnectivity(TRUE);
        }

        //Door Control
        if(HAL_GetTick() < SIGNAL_24V_Timer && !isSIGNAL24V_Active){
          isSIGNAL24V_Active = TRUE;
          HAL_GPIO_WritePin(SIGNAL24V_GPIO_Port,SIGNAL24V_Pin,FALSE);//ON
        }
        else if(HAL_GetTick() > SIGNAL_24V_Timer && isSIGNAL24V_Active){
          isSIGNAL24V_Active = FALSE;
          HAL_GPIO_WritePin(SIGNAL24V_GPIO_Port,SIGNAL24V_Pin,TRUE);//OFF
        }


        if(IPCDResultCode==0 || IPCDResultCode==3)
        {
          API_ipcdRequest(IPCDResultCode);
          IPCDResultCode = 1;//to normal state
        }

      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      //HAL_IWDG_Refresh(&hiwdg);
      yield();
      GetDebugRx();
    }
    End_DebugUDP();
    
    checkConnectivity(TRUE);
    countNetWDG = HAL_GetTick();//exit loop
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime = ADC_SAMPLETIME_39CYCLES_5;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerFrequencyMode = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */
  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 3000;
  hiwdg.Init.Reload = 3000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */
  
  /* USER CODE END IWDG_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_LINE_Pin|LED_SENSOR_Pin|BLE_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ENC_SS_Pin|MEM_CS_Pin|BLE_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RST_Pin|SIGNAL24V_Pin|USART1_DE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_LINE_Pin LED_SENSOR_Pin BLE_RST_Pin */
  GPIO_InitStruct.Pin = LED_LINE_Pin|LED_SENSOR_Pin|BLE_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ENC_SS_Pin MEM_CS_Pin BLE_SS_Pin */
  GPIO_InitStruct.Pin = ENC_SS_Pin|MEM_CS_Pin|BLE_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : RST_Pin SIGNAL24V_Pin USART1_DE_Pin */
  GPIO_InitStruct.Pin = RST_Pin|SIGNAL24V_Pin|USART1_DE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_Pin */
  GPIO_InitStruct.Pin = INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_READY_Pin */
  GPIO_InitStruct.Pin = BLE_READY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BLE_READY_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void User_DeInit()
{
  HAL_ADC_DeInit(&hadc);
  HAL_SPI_DeInit(&hspi1);
  HAL_TIM_Base_DeInit(&htim2);
  HAL_TIM_Base_DeInit(&htim6);
  HAL_UART_DeInit(&huart1);
  HAL_UART_DeInit(&huart2);
  HAL_GPIO_DeInit(GPIOA, LED_LINE_Pin|LED_SENSOR_Pin|BLE_RST_Pin);
  HAL_GPIO_DeInit(GPIOA, ENC_SS_Pin|MEM_CS_Pin|BLE_SS_Pin);
  HAL_GPIO_DeInit(GPIOB, RST_Pin|GPIO_PIN_3);
  HAL_GPIO_DeInit(INT_GPIO_Port, INT_Pin);
  HAL_GPIO_DeInit(BLE_READY_GPIO_Port, BLE_READY_Pin);
}

int _write(int file, char* p, int len)
{
  HAL_UART_Transmit(&huart2,(uint8_t*)p,len,1000);
  debugUDP(p,len-1);
  return len;
}
uint32_t rando()
{
  int t = HAL_GetTick();
  return ((t&0x00FF)<<16 | (t&0xFF00)>>16) * ((t&0x0F)<<8 | (t&0xF0)>>8) - t;
}
void resetNetwork()
{
  extern BOOL isOnline;
  isOnline = FALSE;
  nic_is_linked = FALSE;
}
uint8_t* getKeyIv(int code){
  if(MEM_NormalRead(Key_Start+code*48,AESKeyBuffer,48)==FAILED){
    printf("Key read CORRUPT.\n");
    return NULL;
  }
  return AESKeyBuffer;
}

void FLASH_MEM_Init()
{
  //Initalizing the FLASH memory
  if(MEM_INIT()==FAILED)system_break();

  //Reading flash data length
  int pgmlen = 0;
  MEM_NormalRead(0x000000,&pgmlen,4);
  if(pgmlen <= 0) flash_reset();

  //Reading flash data
  if(Read_SaveData(&saveDataJSON))
  {
    printf("important!! #1 Reading Configuration failed.\n");
    //system_break();
    
    if(Read_SaveData(&saveDataJSON)==FAILED)
    {
      #ifdef USE_JSONSTR_RECOVER
      flash_reset();
      #else
      system_break();
      #endif
    }      
  }
  MEM_make_SavedData(&saveData,saveDataJSON);

  //prints flash data
  MEM_Print(0,256);
  
  //check program version
  if(shouldUpdateProgramVersion())
    updateProgramVersion();
  
  //check APCode
  if(checkAPCodeisValid())
    flash_reset();

  //Print FLASH data to string
  char* str = cJSON_Print(saveDataJSON);
  if(str==0)
  {
    MEM_Print(0,(pgmlen/16 + 1) >> 16);
    system_break();
  }
  printf("savedData : %s\n",str);
  free(str);

  uint8_t pgRunningFlag = 0;
  MEM_NormalRead(0x008000,&pgRunningFlag,1);
  if(pgRunningFlag != 0xAA){
    printf("updating PGFlag %02X->%02X\n",pgRunningFlag,0xAA);
    pgRunningFlag = 0xAA;
    MEM_EraseSector(0x008000);
    MEM_Program(0x008000,pgRunningFlag,1);
  }

}
BOOL shouldUpdateProgramVersion(){
  return memcmp(SMARTIOS_VER,saveData.pgver,strlen(SMARTIOS_VER))!=0 || Date2Int(saveData.pgverdate) != Date2Int(Convert_Str2Date(SMARTIOS_DATE));
}
void updateProgramVersion()
{
  SaveDataUpdate update;
  Date pgverdate = Convert_Str2Date(SMARTIOS_DATE);
  memset(&update,0,sizeof(SaveDataUpdate));
  update.pgver = SMARTIOS_VER;
  update.pgverdate = &pgverdate;
  MEM_ModifyMemory(&update);
}
BOOL checkAPCodeisValid(){
  return saveData.apcode > 1000000 || saveData.apcode < 0;
}
void flash_reset(){
  //invalid data in EEPROM
  #ifdef USE_JSONSTR_RECOVER
  printf("re-write the flash...\n");
  cJSON *json = cJSON_Parse(jsonstr);
  
  if(Store_SaveData(json) != SUCCEED)
    system_break();
  cJSON_Delete(json);
  #endif

  if(Read_SaveData(&saveDataJSON)==FAILED){
    printf("reading FLASH after re-writing failed.\n");
    system_break();
  }
}
void system_break()
{
  printf("system break;\n");
  Error_Handler();
}

BOOL CheckIfOnline()
{
  if(HAL_GetTick() - countNetWDG > 30000)// processing Netdog
    process_Netdog();

  if(nic_is_linked == 0)return FALSE;
  if(nic_linked_and_ip_address_valid == 0)return FALSE;//retry internet.

  checkConnectivity(TRUE);
  waitwithYield(100);

  if(saveData.apcode == 0)
  {
    printf("apcode is zero. pass echo test phase.\n");
    return TRUE;
  }

  Result echo_res;
  for(int i=0;i<4;i++){
    echo_res = API_Echo();
    if(echo_res == SUCCEED)break;
    waitwithYield(500);
  }

  if(echo_res != SUCCEED)
  {
    printf("Echo attempt 4 of 4 failed.\n");
    resetNetwork();
  }
  
  return echo_res == SUCCEED;
}
void process_Netdog()
{
  resetNetwork();
  Debug("NET Reset!\n");
  EC_Init(saveData.apip,saveData.netmask,saveData.gwip,myMAC,saveData.isDHCP);
  countNetWDG = HAL_GetTick();
}

void Timer_print(){
  SensorList *row = &sensorlist[0];
  printf("[%dms]\n",HAL_GetTick());
  for(int i=0;i<SENSORLIST_LEN && row->idxUser != 0;i++)
  {
    if((HAL_GetTick() - row->beginTime) <= saveData.BeaconTimeout*1000)
    printf("Sensor[%d] : idx:%03d %s T:%08dms end:%05ds dist:%03.1fm hack:%05d io:%s needtosend:%d\n",i
      ,row->idxUser,row->isAlive?"Alive":"Dead" ,HAL_GetTick() - row->beginTime, (HAL_GetTick() - row->exitTime) /1000
      ,row->distance,row->hackingCode,!row->ioState?"in":"out",row->needtobeSended);
      
    row++;
  }
  printtimer = HAL_GetTick();
}
Result UpdateAesnoAsDefault()
{
  SaveDataUpdate update;
  memset(&update,0,sizeof(update));
  int aes_no = saveData.apcode % 10;
  update.aes_no = &aes_no;
  return MEM_ModifyMemory(&update);
}
void Timer_ipcd()
{
  if(API_ipcd() == FAILED)
  {
    static int ipcdFailCount;
    if((ipcdFailCount++)>6){
      ipcdFailCount = 0; 
      ipcdTimer = HAL_GetTick(); 
      return;
    }

    yield();
    if(API_Echo() == FAILED)
    {
      checkConnectivity(TRUE);
      printf("IPCD Error\n");
      if(Send_Ping("www.google.com",4) == FAILED)
        resetNetwork();
    }
      
    ipcdTimer = HAL_GetTick() + saveData.ipcdInterval*1000 - 5000;//5 sec timeout
  }
  else ipcdTimer = HAL_GetTick();
}
void Main_APCode0()
{
  static int APCode0timer;
  if(HAL_GetTick() - APCode0timer > 750)
  {
    APCode0timer = HAL_GetTick();
    HAL_GPIO_TogglePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin);//turn off and on LED.
    printf("Please connect to my IP: %s\n",Convert_IP2Str(our_ip_address));
  }
}

void Send_RS485(uint8_t * data)
{
  HAL_GPIO_WritePin(USART1_DE_GPIO_Port,USART1_DE_Pin,RESET);
  HAL_UART_Transmit(&huart1,"\02",1,100);
  HAL_UART_Transmit(&huart1,data,strlen(data),100);
  HAL_UART_Transmit(&huart1,"\03",1,100);
  HAL_GPIO_WritePin(USART1_DE_GPIO_Port,USART1_DE_Pin,SET);
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
    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_LINE_GPIO_Port,LED_LINE_Pin,GPIO_PIN_SET);
    up_udelay(1000*1000);
    
    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_LINE_GPIO_Port,LED_LINE_Pin,GPIO_PIN_RESET);
    up_udelay(1000*1000);
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
