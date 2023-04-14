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
#include "memory/memory.h"
#include "ETH/eth-nic.h"
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
IWDG_HandleTypeDef hiwdg;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_IWDG_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//Power On Self Test
void DO_POST();
int _write(int file, char* p, int len)
{
  LL_GPIO_SetOutputPin(USART1_DE_GPIO_Port,USART1_DE_Pin);

  int count = len;
  while(count--)
  {
    while(!LL_USART_IsActiveFlag_TXE(USART1));
    LL_USART_TransmitData8(USART1,*((uint8_t*)p++));
  }
  while(!LL_USART_IsActiveFlag_TC(USART1));

  LL_GPIO_ResetOutputPin(USART1_DE_GPIO_Port,USART1_DE_Pin);
  return len;
}
int mode = 0;
uint32_t rando()
{
  return 0;
}
void toggleLED()
{
  static BOOL toggle;
  toggle = !toggle;
  if(toggle)
  {
    LL_GPIO_SetOutputPin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin);
    LL_GPIO_SetOutputPin(LED_LINE_GPIO_Port,LED_LINE_Pin);
  }
  else{
    LL_GPIO_ResetOutputPin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin);
    LL_GPIO_ResetOutputPin(LED_LINE_GPIO_Port,LED_LINE_Pin);
  }
}

void Jump_to_Main();
void FLASH_Lock();
Result FLASH_UnLock();
int EraseProgramMemory(int EndPage,int StartPage);
int WriteProgramMemory(int BASE_Start,int len);

#define WRITE_ADDRESS          0x08003000
#define WRITE_SIZE             (116*1024)

#define BACKUP_ADDRESS         0xE0000
#define NEW_FIRMWARE_ADDRESS   0xC0000
void Do_Firmware_Update();
void Do_Firmware_Backup();
BOOL Do_Firmware(uint32_t startaddress);
void Update_pgRunninFlag();
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
  MX_SPI1_Init();
  MX_IWDG_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */


  printf("boot...\n");

  LL_GPIO_SetOutputPin(MEM_CS_GPIO_Port,MEM_CS_Pin);
  LL_GPIO_SetOutputPin(BLE_SS_GPIO_Port,BLE_SS_Pin);
  LL_GPIO_SetOutputPin(ENC_SS_GPIO_Port,ENC_SS_Pin);


  DO_POST();

  
  //printf("bootload temporarilly blocked because of inaccuracy of downloads;\n");
  //Jump_to_Main();

  //page 96~127  
  //0x08003000 ~ 0x0801FFFF (116K)
  //secotr 3~31(4KB*29)
  if(MEM_ByteRead(NEW_FIRMWARE_ADDRESS) == 0xFF && MEM_ByteRead(NEW_FIRMWARE_ADDRESS+1) == 0xFF && MEM_ByteRead(NEW_FIRMWARE_ADDRESS+2) == 0xFF){// No data to Program. Exit.
    printf("NO FIRMWARE Address!!\n");
    
    
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
      printf("This reboot is WATCHDOG Reset!!\n");
      __HAL_RCC_CLEAR_RESET_FLAGS();
      uint8_t pgRunningFlag = 0;
      MEM_NormalRead(0x008000,&pgRunningFlag,1);
      if(pgRunningFlag != 0x00) 
        Update_pgRunninFlag();
      else if(MEM_ByteRead(BACKUP_ADDRESS) != 0xFF ||  MEM_ByteRead(BACKUP_ADDRESS+1) != 0xFF ||  MEM_ByteRead(BACKUP_ADDRESS+2) != 0xFF)
      {
        //main program never be Runned. and Backup data is avaliable.
        //program backup will be proceeded.
        printf("Start Firware Backup!!\n");
        Do_Firmware_Backup();
      }
      else printf("there's wrong program and no backup program.\n");

    }
    
    
    Jump_to_Main();
    return;
  }

  printf("Start Firware Update!!\n");
  Do_Firmware_Update();

  printf("CLEAR PROGRAM Data.\n");
  //clear the data.
  for(int i=0;i<((2*128*1024/4096));i++){
    HAL_IWDG_Refresh(&hiwdg);
    MEM_EraseSector(0x0A0000 + i*4096);
  }

  Jump_to_Main();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_1)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_LSI_Enable();

   /* Wait till LSI is ready */
  while(LL_RCC_LSI_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_4, LL_RCC_PLL_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(32000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
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
  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
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

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA6   ------> SPI1_MISO
  PA7   ------> SPI1_MOSI
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  /**USART1 GPIO Configuration
  PB6   ------> USART1_TX
  PB7   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 256000;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);
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
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

  /**/
  LL_GPIO_ResetOutputPin(LED_LINE_GPIO_Port, LED_LINE_Pin);

  /**/
  LL_GPIO_ResetOutputPin(LED_SENSOR_GPIO_Port, LED_SENSOR_Pin);

  /**/
  LL_GPIO_ResetOutputPin(ENC_SS_GPIO_Port, ENC_SS_Pin);

  /**/
  LL_GPIO_ResetOutputPin(RST_GPIO_Port, RST_Pin);

  /**/
  LL_GPIO_ResetOutputPin(MEM_CS_GPIO_Port, MEM_CS_Pin);

  /**/
  LL_GPIO_ResetOutputPin(BLE_SS_GPIO_Port, BLE_SS_Pin);

  /**/
  LL_GPIO_ResetOutputPin(BLE_RST_GPIO_Port, BLE_RST_Pin);

  /**/
  LL_GPIO_ResetOutputPin(USART1_DE_GPIO_Port, USART1_DE_Pin);

  /**/
  GPIO_InitStruct.Pin = LED_LINE_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED_LINE_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LED_SENSOR_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED_SENSOR_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ENC_SS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(ENC_SS_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = RST_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(RST_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = MEM_CS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(MEM_CS_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = BLE_SS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(BLE_SS_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = BLE_RST_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(BLE_RST_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = USART1_DE_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(USART1_DE_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
char* print_error(Result res);
Result ETH_Test();

void DO_POST()
{
  Result entireResult;
  Result res;
  printf("POST Start\n");

  //FLASH Test
  entireResult |= res = MEM_INIT();
  printf("Memory %s\n",print_error(res));
  
  //Ethernet ENC28J60 test
  //entireResult |= res = ETH_Test();
  //printf("Memory %s\n",print_error(res));
  
  //Beacon Test
  // entireResult |= res = BLE_Test();
  // printf("Memory %s\n",print_error(res));
  
  if(entireResult != SUCCEED)
  {
    printf("bootloader init failed.\n");
    Error_Handler();
  }
}
char* print_error(Result res)
{
  if(res == SUCCEED)return "Succeed";
  else return "Failed.";
}

void nic_reset (void);
Result ETH_Test()
{
	nic_reset();

	nic_select_bank(NIC_ESTAT);
	//-------------------------------------------------
	//----- WRITE THE NIC CONFIGURATION REGISTERS ----- 
	//-------------------------------------------------

  BYTE b_temp;
	//Ensure that we can communicate with the ENC28J60 before proceeding - Wait for CLKRDY to become set.
	//ESTAT bit 3 is unimplemented and should be low.  If it reads as high there is something wrong with the SPI connection.
	do
	{
		b_temp = nic_read(NIC_ESTAT).val;
		printf("TEMP : %02X & 0x09\n",b_temp);
		LL_mDelay(10);
	} while((b_temp & 0x08) || (~b_temp & ESTAT_CLKRDY));
  return SUCCEED;
}

void Jump_to_Main()
{
  #define APPLICATION_ADDRESS 0x08003000
  typedef void (*fptr)(void);
  fptr jump_to_app;
  uint32_t jump_addr;

  //printf("boot loader jump start\n");               //prints Message.
  jump_addr = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
  jump_to_app = (fptr) jump_addr;

  /* init user app's sp */
  printf("start!\n");
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
  jump_to_app();
}
void FLASH_Lock()
{
  /* Lock the Flash to disable the flash control register access (recommended
    to protect the FLASH memory against possible unwanted operation) *********/
  /* Set the PRGLOCK Bit to lock the FLASH Registers access */
  SET_BIT(FLASH->PECR, /*0x02*/FLASH_PECR_PRGLOCK);
  
  /* Set the PELOCK Bit to lock the PECR Register access */
  SET_BIT(FLASH->PECR, /*0x01*/FLASH_PECR_PELOCK);
}
Result FLASH_UnLock()
{
  uint32_t primask_bit;

  /* Unlocking FLASH_PECR register access*/
  if(HAL_IS_BIT_SET(FLASH->PECR, FLASH_PECR_PELOCK))
  {
    /* Disable interrupts to avoid any interruption during unlock sequence */
    primask_bit = __get_PRIMASK();
    __disable_irq();

    WRITE_REG(FLASH->PEKEYR, FLASH_PEKEY1);
    WRITE_REG(FLASH->PEKEYR, FLASH_PEKEY2);

    /* Re-enable the interrupts: restore previous priority mask */
    __set_PRIMASK(primask_bit);

    if(HAL_IS_BIT_SET(FLASH->PECR, FLASH_PECR_PELOCK))
    {
      return FAILED;
    }
  }

  if (HAL_IS_BIT_SET(FLASH->PECR, FLASH_PECR_PRGLOCK))
  {
    /* Disable interrupts to avoid any interruption during unlock sequence */
    primask_bit = __get_PRIMASK();
    __disable_irq();

    /* Unlocking the program memory access */
    WRITE_REG(FLASH->PRGKEYR, FLASH_PRGKEY1);
    WRITE_REG(FLASH->PRGKEYR, FLASH_PRGKEY2);  

    /* Re-enable the interrupts: restore previous priority mask */
    __set_PRIMASK(primask_bit);

    if (HAL_IS_BIT_SET(FLASH->PECR, FLASH_PECR_PRGLOCK))
    {
      return FAILED;
    }
  }

  return SUCCEED; 
}
int EraseProgramMemory(int EndPage,int StartPage)
{
  int ifFailed = 0;
  uint32_t NbPages = (EndPage - StartPage + 1);

  FLASH_UnLock();
  //pre-check for erasing
  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY))HAL_IWDG_Refresh(&hiwdg);
  /* Check FLASH End of Operation flag  */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);

  /* Erase page by page to be done*/
  for(int i=0;i<NbPages;i++)
  {
    HAL_IWDG_Refresh(&hiwdg);
    toggleLED();
    uint32_t address = ( StartPage + i ) * FLASH_PAGE_SIZE;
    //printf("Erasing the device Page %d, Addr %08X\n",(address-(StartPage*FLASH_PAGE_SIZE))/FLASH_PAGE_SIZE,address);
    //FLASH_PageErase(address); start 3 000
    /* Set the ERASE bit */
    SET_BIT(FLASH->PECR, FLASH_PECR_ERASE);

    /* Set PROG bit */
    SET_BIT(FLASH->PECR, FLASH_PECR_PROG);

    /* Write 00000000h to the first word of the program page to erase */
    *(__IO uint32_t *)(uint32_t)(address & ~(FLASH_PAGE_SIZE - 1)) = 0x00000000;
    //FLASH_PageErase(address); end

    // /* Wait for last operation to be completed */
    while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY))HAL_IWDG_Refresh(&hiwdg);
    
    /* Check FLASH End of Operation flag  */
    if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
    {
      /* Clear FLASH End of Operation pending bit */
      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
    }
    if( __HAL_FLASH_GET_FLAG(FLASH_FLAG_WRPERR)  ||
        __HAL_FLASH_GET_FLAG(FLASH_FLAG_PGAERR)  ||
        __HAL_FLASH_GET_FLAG(FLASH_FLAG_SIZERR)  ||
        __HAL_FLASH_GET_FLAG(FLASH_FLAG_OPTVERR) ||
        __HAL_FLASH_GET_FLAG(FLASH_FLAG_RDERR)   ||
        __HAL_FLASH_GET_FLAG(FLASH_FLAG_FWWERR)  ||
        __HAL_FLASH_GET_FLAG(FLASH_FLAG_NOTZEROERR))
    {
      printf("Erase ERR.\n");
      ifFailed = 1;
      break;
    }
    
    /* If the erase operation is completed, disable the ERASE Bit */
    CLEAR_BIT(FLASH->PECR, FLASH_PECR_PROG);
    CLEAR_BIT(FLASH->PECR, FLASH_PECR_ERASE);
  }
  FLASH_Lock();
  return ifFailed;
}
int WriteProgramMemory(int BASE_Start,int len)
{
  #define writeUnit 512
  uint8_t buffer[writeUnit] = {0,};

  int ifFailed = 0;
  FLASH_UnLock();
  //Flash_Erase(BASE_Start,len);
  printf("Program the device..\n");
  for(int i=0;i<(len/writeUnit);i++)
  {
    HAL_IWDG_Refresh(&hiwdg);
    toggleLED();
    printf("%d of %d.. %d%%\n",i*writeUnit,len,(i*writeUnit*100/len));
    while(MEM_NormalRead(BASE_Start+i*writeUnit,buffer,writeUnit) == FAILED);
    HAL_IWDG_Refresh(&hiwdg);
    // if(i == 0)
    // {
    //   MEM_Print(BASE_Start+0x100,32);
    //   uint8_t a;
    //   for(int j=0;j<16;j++)
    //   {
    //     a = buffer[0x100 + j];
    //     buffer[0x100 + j] = buffer[0x100 + 31-j];
    //     buffer[0x100 + 31-j] = a;
    //   }
    // }

    uint32_t PAGEError;
    int sofar=0;
    /* Program the user Flash area word by word*/
    uint32_t StartPageAddress = WRITE_ADDRESS + i*writeUnit;
    while (sofar<(writeUnit/4))
    {
      HAL_IWDG_Refresh(&hiwdg);
      /* Wait for last operation to be completed */
      while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY));

      
      /*Program word (32-bit) at a specified address.*/
      *(__IO uint32_t *)StartPageAddress = ((uint32_t*)buffer)[sofar];

      /* Wait for last operation to be completed */
      while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY));

      /* Check FLASH End of Operation flag  */
      if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
      {
        /* Clear FLASH End of Operation pending bit */
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
      }
      if( __HAL_FLASH_GET_FLAG(FLASH_FLAG_WRPERR)   ||
          __HAL_FLASH_GET_FLAG(FLASH_FLAG_PGAERR)   ||
          __HAL_FLASH_GET_FLAG(FLASH_FLAG_SIZERR)   ||
          __HAL_FLASH_GET_FLAG(FLASH_FLAG_OPTVERR)  ||
          __HAL_FLASH_GET_FLAG(FLASH_FLAG_RDERR)    ||
          __HAL_FLASH_GET_FLAG(FLASH_FLAG_FWWERR)   ||
          __HAL_FLASH_GET_FLAG(FLASH_FLAG_NOTZEROERR))
      {printf("Erase ERROR.\n");ifFailed=1;}

      //if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartPageAddress, ) == HAL_OK)
      //if(status == HAL_OK){
      StartPageAddress += 4;  // use StartPageAddress += 2 for half word and 8 for double word
      sofar++;
    }
    if(MEM_Verify(BASE_Start + i*writeUnit,WRITE_ADDRESS + i*writeUnit,writeUnit) == FAILED)
    {
      HAL_IWDG_Refresh(&hiwdg);
      printf("Verfying error.%05X\n",i*writeUnit);
      ifFailed=1;
      //if(i!=0)ifFailed=1;
      //else printf("changes are well applyed.\n");
    }
  
    //Flash_Write_Data(WRITE_ADDRESS,buffer,writeUnit);
  }
  FLASH_Lock();
  return ifFailed;
}



void Do_Firmware_Update()
{
  BOOL ifFailed = Do_Firmware(NEW_FIRMWARE_ADDRESS);

  if(ifFailed){
    printf("write failed. RESTORING..\n");
    Do_Firmware_Backup();//program update
  }
  Update_pgRunninFlag();
}
void Do_Firmware_Backup()
{
  BOOL ifFailed = Do_Firmware(BACKUP_ADDRESS);

  Update_pgRunninFlag();
}
BOOL Do_Firmware(uint32_t startaddress)
{
  volatile BOOL ifFailed = FALSE;
  const uint32_t StartPage = 96; //0x3000
  const uint32_t EndPageAdress = 0x1FFFF;
  const uint32_t EndPage = 1023;  //0x20000 -> 
  MEM_Print(startaddress,128);

  for(int j=0;j<3;j++){
    printf("Erasing the device..\n");

    if(EraseProgramMemory(EndPage,StartPage) == 1)
    {
      printf("erase failed. retrying..%d\n",j);
      continue;
    }
    
    ifFailed = WriteProgramMemory(startaddress,WRITE_SIZE);
    if(ifFailed == 0)break;
    else
      printf("write failed. retrying..%d\n",j);
  }
  return ifFailed;
}
void Update_pgRunninFlag()
{
  uint8_t pgRunningFlag = 0;
  MEM_NormalRead(0x008000,&pgRunningFlag,1);
  printf("updating PGFlag %02X->%02X\n",pgRunningFlag,pgRunningFlag = 0x00);
  MEM_EraseSector(0x008000);
  MEM_Program(0x008000,&pgRunningFlag,1);
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
