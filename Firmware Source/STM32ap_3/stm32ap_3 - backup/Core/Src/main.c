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
#include "eth-main.h"
#include "eth-nic.h"
#include "eth-arp.h"
#include "eth-ip.h"
#include "eth-dhcp.h"
#include "eth-dns.h"
#include "eth-icmp.h"
#include "eth-tcp.h"
#include "eth-udp.h"
//#include "eth-http-client.h"
#include <stdlib.h>
#include "Addstr.h"
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
UART_HandleTypeDef hlpuart1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
void EC_Init();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int mode = 0;
uint32_t iter_per_us = 0;
void up_udelay(uint32_t us)
{
    up_delayTick(us*iter_per_us);
}
void up_100ndelay(uint32_t us)
{
    up_delayTick(us*iter_per_us/10);
}
void up_delayTick(uint32_t tick)
{
    volatile uint32_t i;

    for (i=0; i<tick; i++) {
    }
}
static void calibrate(void)
{
    uint32_t time;
    volatile uint32_t i;

    iter_per_us = 1000000;

    time = HAL_GetTick();
    /* Wait for next tick */
    while (HAL_GetTick() == time) {
        /* wait */
    }
    for (i=0; i<iter_per_us; i++) {
    }
    iter_per_us /= ((HAL_GetTick()-time)*1000);
}

DWORD eth_dhcp_1sec_renewal_timer;
DWORD eth_dhcp_1sec_lease_timer;
WORD eth_dhcp_1ms_timer;
DWORD ethernet_10ms_clock_timer;
DWORD ethernet_10ms_clock_timer_working;
DWORD tcp_client_socket_timeout_timer;
BOOL isOnline = FALSE;

BYTE eth_dhcp_using_manual_settings;
BYTE *eth_dhcp_our_name_pointer;
	
int count10 = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM6) //1kHz Timer
  {
		//printf("2(%d,%d)\n",eth_dhcp_1ms_timer,ethernet_10ms_clock_timer_working);
    //----- NIC DHCP TIMER -----
    if (eth_dhcp_1ms_timer)
      eth_dhcp_1ms_timer--;


    //-----------------------------
    //----- HERE EVERY 10 mSec -----
    //-----------------------------
    
    //----- ETHERNET GENERAL TIMER -----
    count10++;
    if(count10==10)
    {
		  //printf("6(%d)\n",ethernet_10ms_clock_timer_working);
      ethernet_10ms_clock_timer_working++;
      count10=0;
    }
  
  }
  if (htim->Instance == TIM2) // 1Hz Timer
  {
		//printf("2(%d,%d)\n",eth_dhcp_1sec_renewal_timer,eth_dhcp_1sec_lease_timer);
    //-------------------------------
    //----- HERE EVERY 1 Second -----
    //-------------------------------
    
    //----- NIC DHCP TIMERS -----
    if (eth_dhcp_1sec_renewal_timer)
      eth_dhcp_1sec_renewal_timer--;
    if (eth_dhcp_1sec_lease_timer)
      eth_dhcp_1sec_lease_timer--;
    if(tcp_client_socket_timeout_timer)
      tcp_client_socket_timeout_timer--;

    checkConnectivity(TRUE);
    
  }
}

int _write(int file, char* p, int len)
{
    HAL_UART_Transmit(&hlpuart1,(uint8_t*)p,len,1000);
    return len;
}
void yield()
{
  tcp_ip_process_stack();
  //HAL_Delay(1);
}
BYTE UDP_Open(IP_ADDR AddrToOpen,WORD port_local,WORD port_remote);
int UDP_Send(DEVICE_INFO *device_info);
int UDP_WaitReq(BYTE Socket,int timeout);
int UDP_SendData(BYTE Socket,uint8_t* data,int length);

BYTE TCP_Disconnect(BYTE Socket, int timeout);
BYTE TCP_Get_Data(BYTE Socket,uint8_t* data,int buflen);
BYTE TCP_Get_RemainingDataSize(BYTE Socket);
//ret 0:succeed 1:timeout 2:disconnected 3:retries expired
BYTE TCP_Get_Wait(BYTE Socket,int timeout);
BYTE TCP_SendByte(BYTE Socket, uint8_t* data, int len);
BYTE TCP_Connect(IP_ADDR ip,int port,int timeout);

BYTE AJUAttender_Page(char* page,Addstr_Str *request);
BYTE HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str *request);

IP_ADDR IPSet(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4);
MAC_ADDR MACSet(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,uint8_t d5,uint8_t d6);

//if error, it returns 0.0.0.0
IP_ADDR DNS_Get(char* domain, int timeout);
void checkConnectivity(BOOL print);


BYTE SendARP(IP_ADDR ip);
//mode 0 : Waiting
//mode 10 : Request HTTP Client (Echo)
//mode 11 : waiting for HTTP Get Request
//mode 12 : Processing of HTTP Get Protocol
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
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LPUART1_UART_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  setvbuf(stdout,NULL,_IONBF,0);
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim2);

  EC_Init();

  printf("start!!\n"); 

  printf("connecting line...\n"); 

  // uint32_t start = HAL_GetTick();
  // while((HAL_GetTick()-start < 1000) && !(nic_is_linked && nic_linked_and_ip_address_valid))
  //  tcp_ip_process_stack();

  // checkConnectivity(TRUE);
  
  
  // static BYTE our_tcp_client_socket = TCP_INVALID_SOCKET;
	// static WORD our_tcp_client_local_port;
	// static DEVICE_INFO remote_device_info;
	// static BYTE eth_http_client_10ms_clock_timer_last;
	// static DWORD http_client_content_length;
	// static DWORD http_client_content_received;
	// static BYTE header_started;
	// static BYTE header_ended;
	// static BYTE data_buffer[4];
	// static BYTE content_length_match_char;
  
  //TCP example test

  // enum {
  //   SM_OPEN_SOCKET = 0,
  //   SM_WAIT_FOR_CONNECTION,
  //   SM_TX_PACKET,
  //   SM_WAIT_FOR_RESPONSE,
  //   SM_REQUEST_DISCONNECT,
  //   SM_WAIT_FOR_DISCONNECT,
  //   SM_COMMS_COMPLETE,
  //   SM_COMMS_FAILED
  // };
 
  // static BYTE our_tcp_client_socket = TCP_INVALID_SOCKET;
  // static WORD our_tcp_client_local_port;
  // static BYTE our_tcp_client_state = SM_OPEN_SOCKET;
  // static DEVICE_INFO remote_device_info;
 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    //HAL_Delay(1000);
    /*
    //MAC를 차단하는 것 같다.(Broadcast 많이 이용 시.)
    IP_ADDR iptoarp = IPSet(192,168,0,1);
    MAC_ADDR macfromArp = MACSet(0,0,0,0,0,0);
    printf("arp sending.. %d.%d.%d.%d\n",iptoarp.v[0],iptoarp.v[1],iptoarp.v[2],iptoarp.v[3]);
    if(arp_resolve_ip_address(&iptoarp))
    {
      uint32_t start = HAL_GetTick();
      while(HAL_GetTick()-start < 3000)
      {
        tcp_ip_process_stack();
        if(arp_is_resolve_complete(&iptoarp,&macfromArp))break;
      }
      printf("received : %02X:%02X:%02X:%02X:%02X:%02X\n"
        ,macfromArp.v[0],macfromArp.v[1],macfromArp.v[2],macfromArp.v[3],macfromArp.v[4],macfromArp.v[5]);
    }
    */

    //HAL_Delay(500);
    



    
    if(mode == 0)
    {
      //mode = 10;

      //mode = 88;//start tcp
      //mode = 89;//start udp
      //mode = 90;//start dns
      //mode = 91;//start httpGET
      //mode = 92;

      // IP_ADDR site_addr;
      // printf("DNS Testing\n");
      // site_addr = DNS_Get("eduwon.net",4000);
      // printf("eduwon.net -> %d.%d.%d.%d\n",site_addr.v[0],site_addr.v[1],site_addr.v[2],site_addr.v[3]);
      // site_addr = DNS_Get("eduwon.nea",4000);
      // printf("eduwon.nea -> %d.%d.%d.%d\n\n",site_addr.v[0],site_addr.v[1],site_addr.v[2],site_addr.v[3]);
    }
    /*
    else if(mode == 88)
    {
      mode = 1;
      static BYTE our_tcp_socket = UDP_INVALID_SOCKET;
      IP_ADDR addr = IPSet(192,168,0,20);
      printf("\n\nTCP Start!!");
      our_tcp_socket = TCP_Connect(addr,505,10000);
      if (our_tcp_socket == UDP_INVALID_SOCKET)
      {
        printf("TCP Connection failed!\n");
        continue;
      }
      printf("TCP Connection succeed!\n");
      
      if (!TCP_SendByte(our_tcp_socket,"Hi!\n",4))
      {
        //Can't tx right now - try again next Time
        TCP_Disconnect(our_tcp_socket,1000);
        printf("TCP setup tx Socket failed!\n");
        continue;
      }
      printf("TCP Sending succeed!\n");

      int res;
      if(res = TCP_Get_Wait(our_tcp_socket,30000) == 0)//if data avaliable
      {
        int len = TCP_Get_RemainingDataSize(our_tcp_socket);
        uint8_t* data = malloc(len+1);
        data[len] = 0;
        TCP_Get_Data(our_tcp_socket,data,len);
        printf("TCP Receiving Succeed! len:%d, %s\n",len,data);
      }else{
        printf("TCP Error %d\n",res);
        TCP_Disconnect(our_tcp_socket,1000);
        
        continue;
      }
      printf("TCP Receiving succeed!\n");

      if(TCP_Disconnect(our_tcp_socket,5000))
      {
        printf("TCP Disconnecting Succeed!\n");
      }else{
        printf("TCP Disconnecting timeout.\n");
        continue;
      }
      
      printf("TCP Succeed!\n");
    }
    else if(mode == 89)
    {      

      static BYTE our_udp_socket = UDP_INVALID_SOCKET;
      IP_ADDR addr;
      addr.v[0] = 192;
      addr.v[1] = 168;
      addr.v[2] = 0;
      addr.v[3] = 20;
      our_udp_socket = UDP_Open(addr,6450,6450);
      if (our_udp_socket == UDP_INVALID_SOCKET)
      {
        printf("UDP Invaild Socket!\n");
        continue;
      }
      
      if (!UDP_SendData(our_udp_socket,"Hi!",4))
      {
        //Can't tx right now - try again next Time
        printf("UDP setup tx Socket failed!\n");
        continue;
      }

      printf("UDP Request: %d\n",UDP_WaitReq(our_udp_socket,2000));

      printf("UDP Sending Succeed!\n");
      udp_close_socket(&our_udp_socket);
      continue;
    }
    else if(mode == 90)
    {
      IP_ADDR site_addr;
      char* siteDomain = "www.eduwon.sm";

      if(!do_dns_query(siteDomain,QNS_QUERY_TYPE_HOST))
      {
        extern BYTE dns_state;
        printf("dns failed. %d\n",dns_state);
        continue;
      }

      printf("DNS looking up start!\n");
      uint32_t start = HAL_GetTick();
      while(HAL_GetTick()-start < 4000)
      {
        yield();
        site_addr = check_dns_response();
        if(site_addr.Val == 0x00000000);//waiting
        else if(site_addr.Val == 0xFFFFFFFF)
        {
          printf("dns failed 1.\n");
          break;
        }
        else
        {
          printf("DNS Succeed!\n");
          printf("Resloved : %d.%d.%d.%d\n",site_addr.v[0],site_addr.v[1],site_addr.v[2],site_addr.v[3]);
          break;
        }
      }

      if(site_addr.Val == 0xFFFFFFFF)
      continue;

      
      // IP_ADDR iptoarp = site_addr;
      // MAC_ADDR macfromArp = MACSet(0,0,0,0,0,0);
      // printf("arp sending.. %d.%d.%d.%d\n",iptoarp.v[0],iptoarp.v[1],iptoarp.v[2],iptoarp.v[3]);
      // if(arp_resolve_ip_address(&iptoarp))
      // {
      //   uint32_t start = HAL_GetTick();
      //   while(HAL_GetTick()-start < 3000)
      //   {
      //     yield();
      //     if(arp_is_resolve_complete(&iptoarp,&macfromArp))break;
      //   }
      //   printf("received : %02X:%02X:%02X:%02X:%02X:%02X\n"
      //     ,macfromArp.v[0],macfromArp.v[1],macfromArp.v[2],macfromArp.v[3],macfromArp.v[4],macfromArp.v[5]);
      // }

    }
    else if(mode == 91)
    {
      mode = 1;
      //prepare for HTTP Get Sending
      char* host = "www.eduwon.net";
      int port = 8807;
      char* page = "/api/att/echo_4514";
      IP_ADDR host_addr;
      BYTE socket;
      
      int res;
      //TCP example test
      printf("TCP: SOCKET Open\n");
      host_addr = DNS_Get(host,2000);
      socket = TCP_Connect(host_addr,port,10000);
      if(socket == TCP_INVALID_SOCKET)
        continue;

      //----- TX PACKET TO REMOTE DEVICE -----
      if (!tcp_setup_socket_tx(socket))
      {
        printf("TCP: we can't send TX packet.\n");
        //Can't tx right now - try again next time
        continue;
      }
      //WRITE THE TCP DATA
      tcp_write_array("GET ",4);
      tcp_write_array(page,strlen(page));
      tcp_write_array(" HTTP/1.1\r\n",11);

      tcp_write_array("Host ",5);
      tcp_write_array(host,strlen(host));
      tcp_write_array("\r\n",2);

      tcp_write_array("Connection : close\r\n",20);
      tcp_write_array("\r\n",2);
      //You can also use tcp_write_array()

      //SEND THE PACKET
      tcp_socket_tx_packet(socket);



      res = TCP_Get_Wait(socket,10000);
      //----- WAIT FOR RESPONSE -----

      if (res==1)//tcp_client_socket_timeout_timer == 0)
      {
        printf("TCP: TX Timeout.\n");
        //WAIT FOR RESPOSNE TIMEOUT
        tcp_close_socket(socket);
        continue;
      }

      if (res==0)//tcp_check_socket_for_rx(socket))
      {
        printf("TCP: data :");
        //SOCKET HAS RECEIVED A PACKET - PROCESS IT

        BYTE data;
        printf("len:%d\n",TCP_Get_RemainingDataSize(socket));
        while(tcp_read_next_rx_byte(&data) == 1)
        {
          //yield();
          printf("%c",data);
        }
        printf("\nTCP: data ended!\n");
        // //READ THE PACKET AS REQURIED
        // if (tcp_read_next_rx_byte(&data) == 0)
        // {
        //   //Error - no more bytes in rx packet
        //   printf("\nTCP: data ended!\n");
        // }

        //DUMP THE PACKET
        tcp_dump_rx_packet();
        //go on.
      }

      if (res==3)//tcp_does_socket_require_resend_of_last_packet(socket))
      {
        printf("TCP: re-sending Packet.\n");
        //RE-SEND LAST PACKET TRANSMITTED
        //(TCP requires resending of packets if they are not acknowledged and to
        //avoid requiring a large RAM buffer the application needs to remember
        //the last packet sent on a socket so it can be resent if requried).
        continue;
      }

      if(res==2)//!tcp_is_socket_connected(socket))
      {
        printf("TCP: Host Disconnected.\n");
        //THE CLIENT HAS DISCONNECTED
        continue;
      }
      

      if(!TCP_Disconnect(socket,10000))
      {
        printf("TCP: DisConnect failed.");
        break;
      }
      printf("TCP: Communication Succeed!.\n");
    
    }
    else if(mode == 92)
    {
      mode = 1;
      Addstr_Str Request;
      
      int req = AJUAttender_Page("/api/att/echo_8754",&Request);
      if(!req)
      {
        printf("echo failed.\n");
        continue;
      }

      printf("Request len : %d\r\n",Request.size);
      printf("%s\r\n",Request.str);

      Addstr_destroy(&Request);

    }
    */
    
    /*else if(mode == 10)
    {
      printf("Connect to www.eduwon.net 123\n");
      
      char domain[26] = "www.eduwon.net";
      if(!do_dns_query(domain, QNS_QUERY_TYPE_HOST))
      {
        printf("DNS Failed.\n");
        mode = 1;
      }
      mode = 11;
      // if(start_http_client_request("www.eduwon.net","123"))
      // {
      //   mode == 11;
      // }else
      // {
      //   printf("Request Failed!\n");
      //   HAL_Delay(500);
      // }
    }else if(mode == 11)
    {
      remote_device_info.ip_address = check_dns_response();
      if (remote_device_info.ip_address.Val == 0xffffffff)
      {
        //DNS QUERY FAILED
        printf("Request Failed!\n");
        HAL_Delay(500);
        mode = 10;
      }
      else if (remote_device_info.ip_address.Val)
      {
        printf("DNS Succeed!\n");
        mode = 12;
        //DNS QUERY SUCESSFUL
      }
      else
      {
        //DNS NOT YET COMPLETE
      }
    }else if(mode == 12)
    {
      printf("Connect to %08X..\n",remote_device_info.ip_address.Val);
      remote_device_info.mac_address.v[0] = 0;		//Set to zero so TCP will automatically use ARP to find the MAC address
      remote_device_info.mac_address.v[1] = 0;
      remote_device_info.mac_address.v[2] = 0;
      remote_device_info.mac_address.v[3] = 0;
      remote_device_info.mac_address.v[4] = 0;
      remote_device_info.mac_address.v[5] = 0;
    
      //Connect to remote device port 80
      if (our_tcp_client_socket != TCP_INVALID_SOCKET)		//We shouldn't have a socket currently, but make sure
          tcp_close_socket(our_tcp_client_socket);
    
      our_tcp_client_socket = tcp_connect_socket(&remote_device_info, 80);	
      if (our_tcp_client_socket != TCP_INVALID_SOCKET)
      {	
        our_tcp_client_local_port = tcp_socket[our_tcp_client_socket].local_port;
        mode = 13;
      }
      //Could not open a socket - none currently available - keep trying
    }else if(mode == 13)
    {
      //TCP Waiting
      if (tcp_is_socket_connected(our_tcp_client_socket))
        mode = 14;
    
    }else if(mode == 14)
    {
      tcp_close_socket(our_tcp_client_socket);
      
      printf("Connect Succeed!\n");
      HAL_Delay(500);
    }
    */
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    //----- PROCESS ETHERNET STACK -----
    tcp_ip_process_stack();
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
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
  htim2.Init.Prescaler = 8000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
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
  htim6.Init.Prescaler = 320-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 100-1;
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
  //출처: https://rodneyhuh.tistory.com/22 [Rodney's Blog]
  /* USER CODE END TIM6_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, GoodLED_Pin|TestLED_Pin|ENC_SS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ENC_RST_GPIO_Port, ENC_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : GoodLED_Pin TestLED_Pin ENC_SS_Pin */
  GPIO_InitStruct.Pin = GoodLED_Pin|TestLED_Pin|ENC_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_RST_Pin */
  GPIO_InitStruct.Pin = ENC_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(ENC_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_INT1_Pin */
  GPIO_InitStruct.Pin = ENC_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENC_INT1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

/* USER CODE BEGIN 4 */
/*
BYTE UDP_Open(IP_ADDR AddrToOpen,WORD port_local,WORD port_remote)
{
  DEVICE_INFO remote_device_info;
  BYTE our_udp_socket = UDP_INVALID_SOCKET;
  if (!nic_linked_and_ip_address_valid)
  {
    return UDP_INVALID_SOCKET; //Exit as we can't do anything without a connection
  }
  //Set to broadcast on our subnet (alternatively set the IP and MAC address
  //to a remote devices address - use ARP first if the MAC address is unknown)
  remote_device_info.ip_address = AddrToOpen;
  // remote_device_info.mac_address.v[0] = 0xff;
  // remote_device_info.mac_address.v[1] = 0xff;
  // remote_device_info.mac_address.v[2] = 0xff;
  // remote_device_info.mac_address.v[3] = 0xff;
  // remote_device_info.mac_address.v[4] = 0xff;
  // remote_device_info.mac_address.v[5] = 0xff;
  //Set the port numbers as desired
  return udp_open_socket(&remote_device_info, port_local,port_remote);
}
int UDP_SendData(BYTE Socket,uint8_t* data,int length)
{
  //----- TX PACKET TO REMOTE DEVICE -----
  //SETUP TX
  if (!udp_setup_tx(Socket))
  {
    return 0;
  }
  //WRITE THE TCP DATA
  udp_write_array(data,length);
  //You can also use udp_write_array()
  //SEND THE PACKET
  udp_tx_packet();
  return 1;
}
int UDP_WaitReq(BYTE Socket,int timeout)
{
  uint8_t data;
  int len = 0;
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick()-start < timeout)
  {
    HAL_Delay(10);
    
    if (udp_check_socket_for_rx(Socket))
    {
      //SOCKET HAS RECEIVED A PACKET - PROCESS IT
      //READ THE PACKET AS REQURIED
      if (!udp_read_next_rx_byte(&data))
      {
        udp_close_socket(&Socket);
        //Error - no more bytes in rx packet
        printf("Error - no more bytes in rx packet\n");
        return 0;
      }
      
      //DUMP THE PACKET
      udp_dump_rx_packet();
      break;
    }
    yield();
  }
  return udp_socket[Socket].rx_data_bytes_remaining;
}

BYTE TCP_Connect(IP_ADDR ip,int port,int timeout)
{
  BYTE Socket;
  if (!nic_linked_and_ip_address_valid)
    return TCP_INVALID_SOCKET; 

  DEVICE_INFO remote_device_info;
  remote_device_info.ip_address = ip;
  remote_device_info.mac_address = MACSet(0,0,0,0,0,0);
  //Set to zero so TCP will automatically use ARP to find MAC address

  //SM_OPEN_SOCKET
  Socket = tcp_connect_socket(&remote_device_info,port);

  if(Socket == TCP_INVALID_SOCKET)
  {
    printf("TCP Could not open the socket.\n");
    //Could not open the socket.
    return TCP_INVALID_SOCKET; 
  }
  port = tcp_socket[Socket].local_port;

  printf("TCP: Trying to connect to 192.168.0.20...\n");
  tcp_client_socket_timeout_timer = timeout/10000;
  //Set our wait for connection timeout
  //uint32_t start = HAL_GetTick();
  int ret = 0;
  while(1)//HAL_GetTick() - start < timeout)
  {
    yield();
    //----- WAIT FOR SOCKET TO CONNECT -----
    if (tcp_is_socket_connected(Socket))
    {
      ret = Socket;//Opened the socket.
      printf("TCP: Connected!\n");
      break;
    }
    if(tcp_client_socket_timeout_timer==0)
    {
      ret = TCP_INVALID_SOCKET;//Timeout.
      printf("TCP: Connecting timeout.\n");
      break;
    }
  }
  //printf("TCP Connection time : %dms\n",HAL_GetTick() - start);
  printf("TCP Connection  remain_sec:%d\n",tcp_client_socket_timeout_timer);
  
  return ret;
}
BYTE TCP_SendByte(BYTE Socket, uint8_t* data, int len)
{
  if (!tcp_setup_socket_tx(Socket))
  {
    printf("TCP Cannot setup TX socket.\n");
    return 0;//Cannot setup TX socket.
  }

  if(tcp_write_array(data,len) == 0)
    return 0;//byte could not be written.(MAX_PACKET_SIZE reached.)

  tcp_socket_tx_packet(Socket);
  return len;
}
BYTE TCP_Get_Wait(BYTE Socket,int timeout)
{
  tcp_client_socket_timeout_timer = timeout/1000;			//Set our wait for response timeout
  while(1)
  {
    yield();
    if (tcp_check_socket_for_rx(Socket))
    {
      return 0;//INTERRUPT!
    }
    
    if (tcp_client_socket_timeout_timer == 0)
    {
      //WAIT FOR RESPOSNE TIMEOUT
      tcp_close_socket(Socket);
      return 1;//TIMEOUT
    }
    if (tcp_does_socket_require_resend_of_last_packet(Socket))
    {
      //RE-SEND LAST PACKET TRANSMITTED
      //(TCP requires resending of packets if they are not acknowledged and to
      //avoid requiring a large RAM buffer the application needs to remember
      //the last packet sent on a socket so it can be resent if requried).
      //return to SEND Section.
      return 3;//RETRIES_EXPIRED
    }
    if(!tcp_is_socket_connected(Socket))
    {
      return 2;//THE CLIENT HAS DISCONNECTED
    }
  }
  return 1;//TIMEOUT
  
}
BYTE TCP_Get_RemainingDataSize(BYTE Socket)
{
  return tcp_socket[Socket].rx_data_bytes_remaining;
}
BYTE TCP_Get_Data(BYTE Socket,uint8_t* data,int buflen)
{
  if (tcp_check_socket_for_rx(Socket))
    return tcp_read_rx_array(data,buflen);
  else
    return 0;
}
BYTE TCP_Disconnect(BYTE Socket, int timeout)
{
  tcp_request_disconnect_socket (Socket);
  
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick() - start < timeout)
  {
    yield();
    if (tcp_is_socket_closed(Socket))
      break;//Successfully closed.
  }

  if(!(HAL_GetTick() - start < timeout))
  {
    tcp_close_socket(Socket);
    return 0;//TIMEOUT
  }
  else return 1;//Successfully closed.
}
char* AJUATTENDER_API_DOMAIN = "www.eduwon.net";
int   AJUATTENDER_API_PORT   = 8807;
BYTE AJUAttender_Page(char* page,Addstr_Str *request)
{
  Addstr_Str Req;
  Addstr_init(&Req);
  Addstr_add(&Req,"GET ");
  Addstr_add(&Req,page);
  Addstr_add(&Req,"\r\n");
  
  Addstr_add(&Req,"Host :");
  Addstr_add(&Req,AJUATTENDER_API_DOMAIN);
  Addstr_add(&Req,"\r\n");
  
  Addstr_add(&Req,"Connection: close\r\n");
  Addstr_add(&Req,"\r\n");
  char* require = Addstr_return(&Req);
  IP_ADDR HostIP = DNS_Get(AJUATTENDER_API_DOMAIN,5000);

  int ret = HTTPSendGET(HostIP,AJUATTENDER_API_PORT,Addstr_return(&Req),request);

  Addstr_destroy(&Req);
  return ret;
}

BYTE HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str *request)
{
  BYTE socket;
  socket = TCP_Connect(ip,port,10000);
  if(socket == TCP_INVALID_SOCKET)
    return 0;
  
  if (!tcp_setup_socket_tx(socket))
    return 0;//Can't tx right now - try again next time

  //SEND THE PACKET
  tcp_write_array(require,strlen(require));
  tcp_socket_tx_packet(socket);

  //Waiting for response
  int res = TCP_Get_Wait(socket,10000);

  if (res==1)//tcp_client_socket_timeout_timer == 0)
  {
    printf("TCP: TX Timeout.\n");
    //WAIT FOR RESPOSNE TIMEOUT
    tcp_close_socket(socket);
    return 0;
  }

  if (res==0)//tcp_check_socket_for_rx(socket))
  {
    printf("TCP: data :");
    //SOCKET HAS RECEIVED A PACKET - PROCESS IT
    Addstr_init(request);

    BYTE data;
    printf("len:%d\n",TCP_Get_RemainingDataSize(socket));
    while(tcp_read_next_rx_byte(&data) == 1)
    {
      //yield();
      //printf("%c",data);
      Addstr_addlen(request,data,1);
    }
    printf("\nTCP: data ended!\n");
    printf("TCP: len: %d\n",request->size);
    printf("%s\n",request->str);

    //DUMP THE PACKET
    tcp_dump_rx_packet();
    //go on.
  }

  if (res==3)//tcp_does_socket_require_resend_of_last_packet(socket))
  {
    printf("TCP: re-sending Packet.\n");
    //RE-SEND LAST PACKET TRANSMITTED
    //(TCP requires resending of packets if they are not acknowledged and to
    //avoid requiring a large RAM buffer the application needs to remember
    //the last packet sent on a socket so it can be resent if requried).
    return 0;
  }

  if(res==2)//!tcp_is_socket_connected(socket))
  {
    printf("TCP: Host Disconnected.\n");
    //THE CLIENT HAS DISCONNECTED
    return 0;
  }

  if(!TCP_Disconnect(socket,10000))
    return 0;//TCP: DisConnect failed.

  else return 1;//END.
}

IP_ADDR DNS_Get(char* domain, int timeout)
{
  IP_ADDR ReturnErrorIP = IPSet(0,0,0,0);
  IP_ADDR WaitingIP = IPSet(0,0,0,0);
  IP_ADDR ErrorIP = IPSet(255,255,255,255);
  IP_ADDR domain_addr;

  if(!do_dns_query(domain,QNS_QUERY_TYPE_HOST))
  {
    return ReturnErrorIP;
  }

  //printf("DNS looking up start!\n");
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick()-start < timeout)
  {
    yield();
    domain_addr = check_dns_response();
        if(memcmp(&domain_addr,&WaitingIP,IP_ADDR_LENGTH) == 0);//waiting
    else if(memcmp(&domain_addr,&ErrorIP,IP_ADDR_LENGTH) == 0)
    {
      //printf("dns failed 1.\n");
      return ReturnErrorIP;
    }
    else
    {
      //printf("DNS Succeed!\n");
      //printf("Resloved : %d.%d.%d.%d\n",site_addr.v[0],site_addr.v[1],site_addr.v[2],site_addr.v[3]);
      return domain_addr;
    }
  }
  return ReturnErrorIP;//timeout
}

BYTE SendARP(IP_ADDR ip)
{
  return arp_resolve_ip_address(&ip);
}
IP_ADDR IPSet(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4)
{
  IP_ADDR ip;
  ip.v[0] = d1; ip.v[1] = d2;
  ip.v[2] = d3; ip.v[3] = d4;
  return ip;
}
MAC_ADDR MACSet(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,uint8_t d5,uint8_t d6)
{
  MAC_ADDR mac;
  mac.v[0] = d1; mac.v[1] = d2; mac.v[2] = d3; 
  mac.v[3] = d4; mac.v[4] = d5; mac.v[5] = d6; 
  return mac;
}
*/

void LED_SetOnlineStatus(BOOL isOnline)
{
  HAL_GPIO_WritePin(GPIOA,GoodLED_Pin,isOnline);
  HAL_GPIO_WritePin(GPIOA,TestLED_Pin,!isOnline);
}
void checkConnectivity(BOOL print)
{
  static int connected;
  if(nic_is_linked && nic_linked_and_ip_address_valid) {
    if(!connected && print)
    {
      printf("IP  : %d.%d.%d.%d\n",our_ip_address.v[0]         ,our_ip_address.v[1]         ,our_ip_address.v[2]         ,our_ip_address.v[3]         );
      printf("GW  : %d.%d.%d.%d\n",our_gateway_ip_address.v[0] ,our_gateway_ip_address.v[1] ,our_gateway_ip_address.v[2] ,our_gateway_ip_address.v[3] );
      printf("Msk : %d.%d.%d.%d\n",our_subnet_mask.v[0]        ,our_subnet_mask.v[1]        ,our_subnet_mask.v[2]        ,our_subnet_mask.v[3]        );
      printf("MAC : %02X:%02X:%02X:%02X:%02X:%02X\n"
        ,our_mac_address.v[0],our_mac_address.v[1],our_mac_address.v[2],our_mac_address.v[3],our_mac_address.v[4],our_mac_address.v[5]);
    
      
      IP_ADDR host_addr;
      // if(print)printf("trying to get IP of www.google.com\n");
      // if((host_addr = DNS_Get("www.google.com",5000)).Val == 0)//if rtn is 0.0.0.0
      // {
      //   if(print)printf("Internet not working..");
      //   isOnline = 0;
      //   LED_SetOnlineStatus(FALSE);
      // }
      // else 
      {
        isOnline = 1;
        LED_SetOnlineStatus(TRUE);
        if(print)printf("This device is ONLINE!\n");

        mode = 0;
      }
    }
    connected = 1;
  }
  else {
    if(connected && print)
    {
      if(nic_is_linked && !nic_linked_and_ip_address_valid)
        printf("IP Address is not valid. Please reset the program.\n");
      else if(!nic_is_linked)
        printf("NIC is not linked. Please reset the program.\n");
    }
    connected = 0;
    LED_SetOnlineStatus(FALSE);
  }
}



#define USE_DHCP 1
void EC_Init()
{
  #if !USE_DHCP
  //----- CONFIGURE ETHERNET -----
  eth_dhcp_using_manual_settings = 1;
  our_ip_address.v[0] = 192; //MSB
  our_ip_address.v[1] = 168;
  our_ip_address.v[2] = 0;
  our_ip_address.v[3] = 28; //LSB
  our_subnet_mask.v[0] = 255; //MSB
  our_subnet_mask.v[1] = 255;
  our_subnet_mask.v[2] = 255;
  our_subnet_mask.v[3] = 0; //LSB
  our_gateway_ip_address.v[0] = 192;
  our_gateway_ip_address.v[1] = 168;
  our_gateway_ip_address.v[2] = 0;
  our_gateway_ip_address.v[3] = 1;
  #else
  eth_dhcp_using_manual_settings = 0;
  eth_dhcp_our_name_pointer = "AJUAttender";
  #endif

  //----- SET OUR ETHENET UNIQUE MAC ADDRESS -----
  our_mac_address.v[0] = 0x14; //MSB
  our_mac_address.v[1] = 0x07;
  our_mac_address.v[2] = 0x37;
  our_mac_address.v[3] = 0x88;
  our_mac_address.v[4] = 0x62;
  our_mac_address.v[5] = 0x98; //LSB
  //----- INITIALISE ETHERNET -----
  tcp_ip_initialise();
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
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
