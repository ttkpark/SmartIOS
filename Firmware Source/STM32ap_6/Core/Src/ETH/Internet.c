#include "ETH/Internet.h"
#include "memory.h"
#include <string.h>
#include "eth-arp.h"

extern SavedData saveData;
//'for' loop's count of 1us.
uint64_t iter_per_us = 0;
void up_udelay(uint64_t us)
{
    up_delayTick(us*iter_per_us);
}
void up_100ndelay(uint64_t us)
{
    up_delayTick(us*iter_per_us/10);
}
void up_delayTick(uint64_t tick)
{
    volatile uint64_t i;

    for (i=0; i<tick; i++) {
    }
}
void calibrate(void)
{
    uint32_t time;
    volatile uint64_t i;

    iter_per_us = 1000000;

    time = HAL_GetTick();
    /* Wait for next tick */
    while (HAL_GetTick() == time) {
        /* wait */
    }
    for (i=0; i<iter_per_us; i++) {
    }
    iter_per_us /= ((HAL_GetTick()-time)*1000);
    Debug("Clock Cailbrated : %d\n",iter_per_us);
}

extern DWORD eth_dhcp_1sec_renewal_timer;
extern DWORD eth_dhcp_1sec_lease_timer;
extern WORD eth_dhcp_1ms_timer;
extern DWORD ethernet_10ms_clock_timer;
extern DWORD ethernet_10ms_clock_timer_working;
DWORD tcp_client_socket_timeout_timer;
int isOnline = FALSE;
extern BYTE eth_dhcp_using_manual_settings;
extern BYTE *eth_dhcp_our_name_pointer;
int count10 = 0;// counting variable for 10ms timer
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM6) //1kHz Timer
  {
		//Debug("2(%d,%d)\n",eth_dhcp_1ms_timer,ethernet_10ms_clock_timer_working);
    //----- NIC DHCP TIMER -----
    if (eth_dhcp_1ms_timer)
      eth_dhcp_1ms_timer--;


    //-----------------------------
    //----- HERE EVERY 10 mSec -----
    //-----------------------------
    
    //----- ETHERNET GENERAL TIMER -----
    count10++;
    if(count10==10)// make 10ms
    {
		  //Debug("6(%d)\n",ethernet_10ms_clock_timer_working);
      ethernet_10ms_clock_timer_working++;
      count10=0;
    }
  
  }
  if (htim->Instance == TIM2) // 1Hz Timer
  {
		//Debug("2(%d,%d)\n",eth_dhcp_1sec_renewal_timer,eth_dhcp_1sec_lease_timer);
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

    
  }
}

extern IWDG_HandleTypeDef hiwdg;
// do some background work
// using in timeout loop of each Ethernet work.
void yield()
{
  HAL_IWDG_Refresh(&hiwdg);
  tcp_ip_process_stack();//background for Ethernet
  HAL_Delay(1);
}

char DHCPName[24];

//Ethernet init
void EC_Init(IP_ADDR myIP,IP_ADDR subMsk,IP_ADDR gateway,MAC_ADDR ourMAC, int isDHCP)
{
  if(!isDHCP)
  {
    //----- CONFIGURE ETHERNET -----
    eth_dhcp_using_manual_settings = 1;
    our_ip_address = myIP;
    our_subnet_mask = subMsk;
    our_gateway_ip_address = gateway;
  }
  else
  {
    sprintf(DHCPName,"SMARTIOS_AP%d",saveData.apcode);
    eth_dhcp_using_manual_settings = 0;
    eth_dhcp_our_name_pointer = DHCPName;
  }

  //----- SET OUR ETHENET UNIQUE MAC ADDRESS -----
  our_mac_address = ourMAC;
  //----- INITIALISE ETHERNET -----
  tcp_ip_initialise();
}

void LED_SetOnlineStatus(BOOL isonline)
{
  HAL_GPIO_WritePin(LED_LINE_GPIO_Port,LED_LINE_Pin,!isonline);
}
volatile uint32_t lastUnlinkedTime;
void checkConnectivity(BOOL print)
{
  volatile BOOL isprint = print;
  volatile static int connected; //before's connected or not for detecting.

  //if link is up and ip's valid in local network.
  if(nic_linked_and_ip_address_valid) {
    //Debug("linked%d\n",connected);
    if(connected == 0)// before, It was unconnected.(connecting status was changed)
    {
      if(isprint)Debug("IP  : %d.%d.%d.%d\n",our_ip_address.v[0]         ,our_ip_address.v[1]         ,our_ip_address.v[2]         ,our_ip_address.v[3]         );
      if(isprint)Debug("GW  : %d.%d.%d.%d\n",our_gateway_ip_address.v[0] ,our_gateway_ip_address.v[1] ,our_gateway_ip_address.v[2] ,our_gateway_ip_address.v[3] );
      if(isprint)Debug("Msk : %d.%d.%d.%d\n",our_subnet_mask.v[0]        ,our_subnet_mask.v[1]        ,our_subnet_mask.v[2]        ,our_subnet_mask.v[3]        );
      if(isprint)Debug("MAC : %02X:%02X:%02X:%02X:%02X:%02X\n"
        ,our_mac_address.v[0],our_mac_address.v[1],our_mac_address.v[2],our_mac_address.v[3],our_mac_address.v[4],our_mac_address.v[5]);
      if(isprint)Debug("IP lease Time : %ds(%dh %dmin)\n",eth_dhcp_1sec_renewal_timer,(eth_dhcp_1sec_renewal_timer/3600),(eth_dhcp_1sec_renewal_timer/60)%60);
      isOnline = TRUE;//TEMPORARY
      LED_SetOnlineStatus(TRUE);//TEMPORARY
    }
    /*
    if(!isOnline)
    {
      //check for Internet connect (getting IP of google.)->ECHO
      if(isprint)Debug("trying to send Echo..\n");
      if(!Send_API_Echo())//if rtn is 0, Internet failed.
      {
        if(isprint)Debug("Internet not working..");
        isOnline = 0;
        LED_SetOnlineStatus(FALSE);
      }
      else 
      {
        isOnline = 1;
        LED_SetOnlineStatus(TRUE);
        if(isprint)Debug("This device is ONLINE!\n");
      }
    }
    */
    connected = 1;
  }
  else {
    Debug("LNK_X%d\n",connected);
    if(connected != 0)
    {
      if(nic_is_linked && !nic_linked_and_ip_address_valid)
        if(isprint)Debug("IP Address is not valid. Please reset the program.\n");
      else if(!nic_is_linked)
        if(isprint)Debug("NIC is not Connected. Please reset the program.\n");
    }
    connected = 0;
    isOnline = 0;
    LED_SetOnlineStatus(FALSE);
  }
}
int checkUnlinkTimeout(uint32_t lastUnlinkedtime,int timeout)
{
  Debug("remain timeouts.. %dms\n",timeout - HAL_GetTick()-lastUnlinkedtime);
  return (HAL_GetTick()-lastUnlinkedtime) > timeout;
}
void Ethernet_Reset()
{
  tcp_ip_initialise();
}
void waitwithYield(uint16_t delay)
{
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick()-start < delay)
    yield();

}
Result SendARP(IP_ADDR ip)
{
  return !arp_resolve_ip_address(&ip);
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


SOCKET UDP_Open(IP_ADDR AddrToOpen,WORD port_local,WORD port_remote)
{
  DEVICE_INFO remote_device_info;
  SOCKET our_udp_socket = UDP_INVALID_SOCKET;
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
Result UDP_SendData(SOCKET Socket,uint8_t* data,int length)
{
  //----- TX PACKET TO REMOTE DEVICE -----
  //SETUP TX
  if (!udp_setup_tx(Socket))
  {
    return FAILED;
  }
  //WRITE THE TCP DATA
  udp_write_array(data,length);
  //You can also use udp_write_array()
  //SEND THE PACKET
  udp_tx_packet();
  return SUCCEED;
}
LENGTH UDP_WaitReq(SOCKET Socket,int timeout)
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
        Debug("Error - no more bytes in rx packet\n");
        return FAILED;
      }
      
      //DUMP THE PACKET
      udp_dump_rx_packet();
      break;
    }
    yield();
  }
  return udp_socket[Socket].rx_data_bytes_remaining;
}

SOCKET TCP_Connect(IP_ADDR ip,int port,int timeout)
{
  SOCKET Socket;
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
    //Could not open the socket.
    return TCP_INVALID_SOCKET;
  }
  port = tcp_socket[Socket].local_port;


  //Set our wait for connection timeout
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick() - start < timeout)
  {
    yield();
    //----- WAIT FOR SOCKET TO CONNECT -----
    if (tcp_is_socket_connected(Socket))
      break;
  }
  //Debug("TCP Connection time : %dms\n",HAL_GetTick() - start);
  if(!(HAL_GetTick() - start < timeout))
  {
    TCP_Disconnect(Socket,1000);
    return TCP_INVALID_SOCKET;//Timeout.
  }
  else return Socket;//Opened the socket.
}
LENGTH TCP_SendByte(SOCKET Socket, uint8_t* data, int len)
{
  if (!tcp_setup_socket_tx(Socket))
  {
    CleanTCPSocket(Socket);
    Debug("TCP Cannot setup TX socket.\n");
    return FAILED;//Cannot setup TX socket.
  }

  if(tcp_write_array(data,len) == 0)
    return FAILED;//byte could not be written.(MAX_PACKET_SIZE reached.)

  tcp_socket_tx_packet(Socket);
  return len;
}
// Wait for data, disconnecting from the server, or retransmission require. 
RES_GetWait TCP_Get_Wait(SOCKET Socket,int timeout)
{
  //Debug("TCP Waiting.. timeout:%dms\n",timeout);
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick() - start < timeout)
  {
    yield();
    if (tcp_check_socket_for_rx(Socket))
    {
      //Debug("RECEIVED! : %dms\n",HAL_GetTick() - start);
      return SUCCEEDED;//INTERRUPT!
    }
    if (tcp_does_socket_require_resend_of_last_packet(Socket))
    {
      Debug("RE-SEND LAST PACKET TRANSMITTED : %dms\n",HAL_GetTick() - start);
      //RE-SEND LAST PACKET TRANSMITTED
      //(TCP requires resending of packets if they are not acknowledged and to
      //avoid requiring a large RAM buffer the application needs to remember
      //the last packet sent on a socket so it can be resent if requried).
      //return to SEND Section.
      return RETRIES_EXPIRED;//RETRIES_EXPIRED
    }
    if(!tcp_is_socket_connected(Socket))
    {
      Debug("THE CLIENT HAS DISCONNECTED : %dms\n",HAL_GetTick() - start);
      return DISCONNECTED;//THE CLIENT HAS DISCONNECTED
    }
  }
  return TIMEOUT;//TIMEOUT
  
}
LENGTH TCP_Get_RemainingDataSize(SOCKET Socket)
{
  return tcp_socket[Socket].rx_data_bytes_remaining;
}
LENGTH TCP_Get_Data(SOCKET Socket,uint8_t* data,int buflen)
{
  if (tcp_check_socket_for_rx(Socket))
    return tcp_read_rx_array(data,buflen);
  else
    return FAILED;
}
RES_GetWait TCP_Disconnect(SOCKET Socket, int timeout)
{
  tcp_request_disconnect_socket (Socket);
  
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick() - start < timeout)
  {
    //Debug("TCP_Disconeect\n");
    yield();
    if (tcp_is_socket_closed(Socket))
      break;//Successfully closed.
  }

  if(tcp_is_socket_closed(Socket))
    return SUCCEEDED;//Successfully closed.

  else return TIMEOUT;//TIMEOUT
}

IP_ADDR DNS_Get(char* domain, int timeout)
{
  IP_ADDR ReturnErrorIP = IPSet(0,0,0,0);
  IP_ADDR WaitingIP = IPSet(0,0,0,0);
  IP_ADDR ErrorIP = IPSet(255,255,255,255);
  IP_ADDR domain_addr;

  if(!do_dns_query((BYTE*)domain,QNS_QUERY_TYPE_HOST))
  {
    return ReturnErrorIP;
  }

  Debug("DNS looking up start!\n");
  uint32_t start = HAL_GetTick();
  while(HAL_GetTick()-start < timeout)
  {
    Debug(".");
    yield();
    Debug("y");
    domain_addr = check_dns_response();
    if(domain_addr.Val == WaitingIP.Val);//waiting
    else if(domain_addr.Val == ErrorIP.Val)
    {
      //Debug("dns failed 1.\n");
      return ReturnErrorIP;
    }
    else
    {
      //Debug("DNS Succeed!\n");
      //Debug("Resloved : %d.%d.%d.%d\n",domain_addr.v[0],domain_addr.v[1],domain_addr.v[2],domain_addr.v[3]);
      return domain_addr;
    }
  }
  return ReturnErrorIP;//timeout
}

Result Send_Ping(char* host,int count)
{
  IP_ADDR HostIP = DNS_Get(host,1000);
  MAC_ADDR HostMAC;
  if(HostIP.Val == 0x00)HostIP = Convert_Str2IP(host);//if domain fails, get from ip address.

  if(HostIP.Val == 0x00)
  {
    printf("DNS lookup Error. cannot resolve %s to IP address.\n",host);
    return FAILED;
  }
  SendARP(HostIP);
  
  uint32_t ARPTimer = HAL_GetTick();
  while(HAL_GetTick() - ARPTimer < 1000)
  {
    yield();
    if (arp_is_resolve_complete (&HostIP, &HostMAC))
		{
      printf("APR succeed. %02X:%02X:%02X:%02X:%02X:%02X\n",HostMAC.v[0],HostMAC.v[1],HostMAC.v[2],HostMAC.v[3],HostMAC.v[4],HostMAC.v[5]);
			break;
		}
  }
  if(HAL_GetTick() - ARPTimer >= 1000)
  {
    printf("APR failed.\n");
    return FAILED;
  }

	static DEVICE_INFO remote_device_info;

	static BYTE icmp_data_buffer[ICMP_MAX_DATA_LENGTH];
	static WORD icmp_id;
	static WORD icmp_sequence;
  remote_device_info.ip_address = HostIP;
  remote_device_info.mac_address = HostMAC;

  const int requestCount = 4;
  int successCount = 0;
  printf("Ping %s[%d.%d.%d.%d] using 32 bytes:\n",host,HostIP.v[0],HostIP.v[1],HostIP.v[2],HostIP.v[3]);
  for(int i=0;i<requestCount;i++)
  {
    destination_ICMP_ip_address = HostIP;
    
    uint32_t timer = HAL_GetTick();
    extern SOCKET debug_udp;
    SOCKET temp_dbugudp = debug_udp;
    
    debug_udp = 0xff;
    while(nic_read(NIC_ECON1).ECON1bits.TXRTS && HAL_GetTick() - timer < 1000)
      printf("internet.c:%d MY code : little waits until transmission success..\n",__LINE__);
    debug_udp = temp_dbugudp;

    //Send ICMP.
    if (nic_setup_tx())
    {
      icmp_sequence++;
      icmp_send_packet(&remote_device_info,ICMP_ECHO_REQUEST, &icmp_data_buffer[0], 32, &icmp_id, &icmp_sequence);
    }
    else
    {
      printf("CAN'T TX OUR REQUEST NOW\n");
      waitwithYield(500);
      continue;
    }

    //Get ICMP reply.
    uint32_t ICMPTimer = HAL_GetTick();
    while(HAL_GetTick() - ICMPTimer < 2000)
    {
      yield();
      if(destination_ICMP_ip_address.Val == 0x00)
      {
        printf("reply from %d.%d.%d.%d : byte=32 time=%dms\n",HostIP.v[0],HostIP.v[1],HostIP.v[2],HostIP.v[3],HAL_GetTick() - ICMPTimer);
        successCount++;
        waitwithYield(100);
        break;
      }
      else if(destination_ICMP_ip_address.Val == 0xffffffff)
      {
        printf("Malformed Packet!!\n");
        break;
      }
    }
    if(destination_ICMP_ip_address.Val == HostIP.Val)
    {
      printf("No reply from Host.\n");
    }
  }

  printf("Success : %d, Failed : %d. %d of %d succeed(%d%%).\n",successCount,requestCount - successCount,successCount,requestCount,(successCount*100)/requestCount);
  printf("Ping ended.\n");
  if(successCount == 0) return FAILED;
  else return SUCCEED;
}