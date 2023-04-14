#include "main.h"
#include "programmer/programmer.h"
#include "buffer/Addstr.h"
#include "JSON/cJSON.h"
#include "ETH/Internet.h"
#include "memory/memory.h"

extern SOCKET debug_udp;
extern IP_ADDR debug_udpIP;
extern SavedData saveData;

extern UART_HandleTypeDef huart2;
const UART_HandleTypeDef *puart = &huart2;

char UARTBuffer[1024] = {0,};
int UARTBufferfocus = 0;

volatile uint8_t readch;
void UARTReceiver_init()
{
    UARTBuffer[0] = 0;
    HAL_UART_Receive_IT(puart,&readch,1);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == puart->Instance)
    {
        UARTReceiver_callback();
    }
}

BOOL UART_Ended = TRUE;
void UARTReceiver_callback()
{
    if(readch == 0x03){
        UART_Ended = TRUE;
        UARTBuffer[UARTBufferfocus] = 0;
        UARTBufferfocus=0;
        UARTReceiver_Process(UARTBuffer);
        printf("UART Received:%s\n",UARTBuffer);
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
    HAL_UART_Receive_IT(puart,&readch,1);
}

typedef enum{
    NORMAL = 0,
    ECHO,
    GET,
    WaitAmode_SET,
    PING
}WaitAmode;

WaitAmode Amode = 0;

void UARTReceiver_Process(char* msg)
{
    yield();
    if(Amode == WaitAmode_SET)
    {
        //get data and Store
        //printf("re-write the flash...\n");
        cJSON *json = cJSON_Parse(msg);

        if(Store_SaveData(json) != SUCCEED)
            HAL_UART_Transmit(puart,"\02ERROR\03",7,50);
        else 
            HAL_UART_Transmit(puart,"\02OK\03",4,50);
        cJSON_Delete(json);

        Amode = NORMAL;
    }
    else if(memcmp(msg,"ECHO",4) == 0)
    {
        //ECHO
        Amode = ECHO;
        HAL_UART_Transmit(puart,"\02OK\03",4,50);
        Amode = NORMAL;
    }
    else if(memcmp(msg,"GET",3) == 0)
    {
        //GET
        Amode = GET;
        extern cJSON* saveDataJSON;
        char* data = cJSON_Print(saveDataJSON);

        if(data != 0)
        {
            HAL_UART_Transmit(puart,"\02",1,50);
            HAL_UART_Transmit(puart,data,strlen(data),100);
            HAL_UART_Transmit(puart,"\03",1,50);
            debugUDP(data,strlen(data));
        }
        else 
            HAL_UART_Transmit(puart,"\02ERROR\03",9,50);
        

        free(data);
        
        Amode = NORMAL;
    }
    else if(memcmp(msg,"SET",3) == 0)
    {
        //SET
        //(Data)
        Amode = WaitAmode_SET;
    }
    else if(memcmp(msg,"PING",4) == 0)
    {
        //PING (IP Address)

        if(Send_Ping(msg+5,4) != SUCCEED)
            HAL_UART_Transmit(puart,"\02ERROR\03",7,50);
        else 
            HAL_UART_Transmit(puart,"\02OK\03",4,50);
    }
    else if(memcmp(msg,"UDPIP",5) == 0)
    {
        //UDPIP (IP Address)
        char* ipstr = msg+6;
        debug_udpIP = Convert_Str2IP(ipstr);

        if(debug_udpIP.Val == 0x00000000)//if INVLAID
            End_DebugUDP();
        else{
            printf("New DebugUDP IP : %s\n",Convert_IP2Str(debug_udpIP));
            Start_DebugUDP();
        }
    }
    else if(memcmp(msg,"IPCD",4) == 0)
    {
        //IPCD
        extern uint32_t ipcdTimer;
        extern SavedData saveData;
        ipcdTimer = HAL_GetTick() + saveData.ipcdInterval*1000 - 2000;//2 sec timeout
    }
    else if(memcmp(msg,"PRINT",5) == 0)
    {
        //PRINT
        extern uint32_t printtimer;
        printtimer = HAL_GetTick() +1000*60*5 - 2000;//2 sec timeout
    }
    else if(memcmp(msg,"RESET",5) == 0)
    {
        //RESET
        printf("Now I Reset This system!!\n");
        HAL_NVIC_SystemReset();
    }else if(memcmp(msg,"NETIF",5) == 0)
    {
        //NETIF
        //DHCP Message
        printf("IP  : %s\n",Convert_IP2Str(our_ip_address));
        printf("GW  : %s\n",Convert_IP2Str(our_gateway_ip_address));
        printf("Msk : %s\n",Convert_IP2Str(our_subnet_mask));
        printf("MAC : %02X:%02X:%02X:%02X:%02X:%02X\n"
            ,our_mac_address.v[0],our_mac_address.v[1],our_mac_address.v[2],our_mac_address.v[3],our_mac_address.v[4],our_mac_address.v[5]);
        printf("IP lease Time : %ds(%dh %dmin)\n",eth_dhcp_1sec_renewal_timer,(eth_dhcp_1sec_renewal_timer/3600),(eth_dhcp_1sec_renewal_timer/60)%60);
        
        extern Result API_Echo();
        API_Echo();
    }else if(memcmp(msg,"UPDATE",6) == 0)
    {
        //UPDATE (verCode)
        extern void start_FirmUpdate(char* pgver);
        
        start_FirmUpdate(msg+7);
    }else if(memcmp(msg,"IDSET",5) == 0)
    {
        SaveDataUpdate update;
        memset(&update,0,sizeof(SaveDataUpdate));
        update.apcode = atoi(msg+6);
        printf("set memory apcode to %d.\n",update.apcode);
        //if apcode is 0 due to error, that modifiance is to be ignored.
        Result res = MEM_ModifyMemory(&update);
        printf("IDSET %s.\n",res==SUCCEED?"Succeed":"failed");
    }
}
//AP UDP&UART Debug Terminal
// SET : \02 SET \03 \02 Data.. \03 : save "Data.." into FLASH.
// GET : \02 GET \03 : Read Data from FLASH
// ECHO : \02 ECHO \03 :  prints \02 OK \03
// PING : \02 PING (domain) \03 : Sends a PING Request.
// UDPIP : \02 UDPIP \03 :  UDPIP (IPAddr) : Determines IP to send UDP Debug Messages.
// IPCD : \02 IPCD \03 :  Requests IPCD API.
// PRINT : \02 PRINT \03 :  Prints status of each of Beacon Sensors
// RESET : \02 RESET \03 :  Resets the AP.
// NETIF : \02 NETIF \03 :  Prints Ethernet status
// UPDATE : \02 UPDATE (pgver) \03 : Starts firmware Update process. 
// IDSET : \02 IDSET (apcode) \03 : Modifies apcode only. 

void Start_DebugUDP()
{
    End_DebugUDP();
    DEVICE_INFO remote_device_info;
    DEVICE_INFO *device_info_ptr;
    if(debug_udpIP.Val == 0xFFFFFFFF)device_info_ptr = NULL;//broadcast
    else{
        device_info_ptr = &remote_device_info;
        remote_device_info.ip_address = debug_udpIP;
        SendARP(debug_udpIP);
        while(!arp_is_resolve_complete(&debug_udpIP,&remote_device_info.mac_address))yield();
    }
    
    debug_udp = udp_open_socket(NULL,31300,31300);
    
    if(debug_udp != UDP_INVALID_SOCKET)
    {
        char startBroadcast[64];
        sprintf(startBroadcast,"%s (apcode:%d) UDP Broadcast Start!!\n",Convert_IP2Str(our_ip_address),saveData.apcode);

        UDP_SendData(debug_udp,startBroadcast,strlen(startBroadcast));
        udp_close_socket(&debug_udp);
    }
    
    
    debug_udp = udp_open_socket(device_info_ptr,31300+saveData.apcode,31300+saveData.apcode);
}
void debugUDP(char* str,int len)
{
    if(debug_udp == UDP_INVALID_SOCKET) return;
    UDP_SendData(debug_udp,str,len);
}
void End_DebugUDP()
{
    if(debug_udp != UDP_INVALID_SOCKET)
        udp_close_socket(&debug_udp);
}
char buffer[32] = {0,};
void GetDebugRx()
{
    if (udp_check_socket_for_rx(debug_udp))
    {
        char* focus;
        printf("UDP len:%d\n",udp_socket[debug_udp].rx_data_bytes_remaining);

        if(udp_socket[debug_udp].rx_data_bytes_remaining > sizeof(buffer))//if datat is larger than buffer
        {
            //save it on UART Buffer.
            
            focus = UARTBuffer;
            while(udp_read_next_rx_byte(focus))
            {
                focus++;
                if(focus - UARTBuffer > sizeof(UARTBuffer))//buffer full
                    focus = UARTBuffer;
            }
            *focus = '\0';
            UARTBufferfocus = 0;
            UART_Ended = TRUE;
            
            //DUMP THE PACKET
            udp_dump_rx_packet();

            printf("UDPGet : %s\n",UARTBuffer);
            UARTReceiver_Process(UARTBuffer);
        }
        else
        {
            //put the data into UDP 32B Buffer.

            focus = buffer;

            //SOCKET HAS RECEIVED A PACKET - PROCESS IT
            //READ THE PACKET AS REQURIED
            while(udp_read_next_rx_byte(focus))
            {
                focus++;
                if(focus - buffer > sizeof(buffer))//buffer full
                    focus = buffer;
            }
            *focus = '\0';

            //DUMP THE PACKET
            udp_dump_rx_packet();

            printf("UDPGet : %s\n",buffer);
            UARTReceiver_Process(buffer);
        }
    }
}