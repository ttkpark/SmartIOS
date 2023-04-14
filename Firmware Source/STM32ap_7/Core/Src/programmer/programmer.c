#include "main.h"
#include "programmer/programmer.h"
#include "buffer/Addstr.h"
#include "JSON/tiny-json.h"
#include "ETH/Internet.h"
#include "buffer/CoderBuffer.h"
#include "memory/memory.h"
#include "JSON/JSONBuffer.h"

extern SOCKET debug_udp;
extern IP_ADDR debug_udpIP;
extern SavedData saveData;

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
const UART_HandleTypeDef *puart = &huart1;

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
extern void Send_RS485(uint8_t * data);

void UARTReceiver_Process(char* msg)
{
    yield();
    if(Amode == WaitAmode_SET)
    {
        //get data and Store
        printf("re-write the flash...\n");
        
        Addstr_Str* str;
        Addstr_initlen(&str,strlen(msg));
        memcpy(str->str,msg,strlen(msg));

        JSONBuffer jsondata = JSONBuffer_make(str);
        jsondata->json = json_create(str->str,jsondata->pool,20);
        SavedData updateObj;
        SaveDataUpdate update;
        memset(&update,0,sizeof(update));

        if(jsondata != NULL)
        {   
            printf("getting..\n");
            if(jsondata->json != NULL)
            {
                json_t* obj;

                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"dist")) != NULL){updateObj.dist = atoi(json_getValue(obj)); update.dist = &updateObj.dist;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"apip")) != NULL){updateObj.apip = Convert_Str2IP(json_getValue(obj)); update.apip = &updateObj.apip;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"netmask")) != NULL){updateObj.netmask = Convert_Str2IP(json_getValue(obj)); update.netmask = &updateObj.netmask;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"gwip")) != NULL){updateObj.gwip = Convert_Str2IP(json_getValue(obj)); update.gwip = &updateObj.gwip;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"dnsip")) != NULL){updateObj.dnsip = Convert_Str2IP(json_getValue(obj)); update.dnsip = &updateObj.dnsip;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"dns2ip")) != NULL){updateObj.dns2ip = Convert_Str2IP(json_getValue(obj));  update.dns2ip = &updateObj.dns2ip;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"IdentifyDistance")) != NULL){updateObj.IdentifyDistance = atoi(json_getValue(obj)); update.IdentifyDistance = &updateObj.IdentifyDistance;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"BeaconTimeout")) != NULL){updateObj.BeaconTimeout = atoi(json_getValue(obj)); update.BeaconTimeout = &updateObj.BeaconTimeout;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"enableOutDetecting")) != NULL){updateObj.enableOutDetecting = atoi(json_getValue(obj)); update.enableOutDetecting = &updateObj.enableOutDetecting;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"dbuglog")) != NULL){updateObj.dbuglog = json_getBoolean(obj) == true;  update.dbuglog = &updateObj.dbuglog;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"isDHCP")) != NULL){updateObj.isDHCP = json_getBoolean(obj) == true;  update.isDHCP = &updateObj.isDHCP;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"ipcdInterval")) != NULL){updateObj.ipcdInterval = atoi(json_getValue(obj)); update.ipcdInterval = &updateObj.ipcdInterval;}
                printf("%d,",__LINE__);if((obj=json_getProperty(jsondata->json,"dbugudp")) != NULL){updateObj.dbugudp = json_getBoolean(obj) == true; update.dbugudp = &updateObj.dbugudp;}
                printf("%dn",__LINE__);if((obj=json_getProperty(jsondata->json,"resnd")) != NULL){updateObj.resnd = atoi(json_getValue(obj)); update.resnd = &updateObj.resnd;}
                
                NewMEM_ModifyMemory(&update,&saveData);
            }
            Send_RS485("OK");
        }
        else Send_RS485("ERROR");
    
        JSONBuffer_free(jsondata); 


        // if(Store_SaveData(msg) != SUCCEED) // TODO : Change memory type
        //     Send_RS485("ERROR");
        // else 
        //     Send_RS485("OK");
        
        //isReadSaveDataFailed();
        Amode = NORMAL;
    }
    else if(memcmp(msg,"ECHO",4) == 0)
    {
        //ECHO
        Send_RS485("OK");
        Amode = NORMAL;
    }
    else if(memcmp(msg,"GET",3) == 0)
    {
        //GET
        MEM_Print(0x00000,sizeof(saveData));

        printf("{\n");
        printf("\"apcode\" : %d,\n",saveData.apcode);
        printf("\"dist\" : %g,\n",saveData.dist);
        printf("\"apip\" : %s,\n",Convert_IP2Str(saveData.apip));
        printf("\"gwip\" : %s,\n",Convert_IP2Str(saveData.gwip));
        printf("\"netmask\" : %s,\n",Convert_IP2Str(saveData.netmask));
        printf("\"dnsip\" : %s,\n",Convert_IP2Str(saveData.dnsip));
        printf("\"dns2ip\" : %s,\n",Convert_IP2Str(saveData.dns2ip));
        printf("\"IdentifyDistance\" : %g,\n",saveData.IdentifyDistance);
        printf("\"BeaconTimeout\" : %d,\n",saveData.BeaconTimeout);
        printf("\"enableOutDetecting\" : %d,\n",saveData.enableOutDetecting);
        printf("\"dbuglog\" : %d,\n",saveData.dbuglog);
        printf("\"dbugudp\" : %d,\n",saveData.dbugudp);
        printf("\"isDHCP\" : %d,\n",saveData.isDHCP);
        printf("\"ipcdInterval\" : %d,\n",saveData.ipcdInterval);
        printf("\"resnd\" : %d\n}\n",saveData.resnd);
        Send_RS485("OK");
        // free(data);
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
            Send_RS485("ERROR");
        else 
            Send_RS485("OK");
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
        printf("IP lease reamin Time : %ds(%dh %dmin)\n",eth_dhcp_1sec_renewal_timer,(eth_dhcp_1sec_renewal_timer/3600),(eth_dhcp_1sec_renewal_timer/60)%60);

        extern uint32_t networkONtimer;
        extern int isOnline;
        uint32_t onlineTime = HAL_GetTick();
        printf("BOOT Time : %ds(%dd %dh %dmin)\n",onlineTime/1000, onlineTime/(24*3600000), (onlineTime/3600000)%24, (onlineTime/60000)%60);
        onlineTime -= networkONtimer;
        printf("Online Time : %ds(%dd %dh %dmin)\n",onlineTime/1000, onlineTime/(24*3600000), (onlineTime/3600000)%24, (onlineTime/60000)%60);
        printf("isOnline : %d\n",isOnline);
        printf("Msk : %s\n",Convert_IP2Str(our_subnet_mask));
        
        extern Result API_Echo();
        API_Echo();
    }else if(memcmp(msg,"UPDATE",6) == 0)
    {
        //UPDATE (verCode)
        if(start_FirmUpdate(msg+7) == SUCCEED)CopyFirmwareAndBackup();
    }else if(memcmp(msg,"IDSET",5) == 0)
    {
        int apcode = 0;
        apcode = atoi(msg+6);
        if(apcode > 0)
        {
            saveData.apcode = apcode;
            printf("set memory apcode to %d.\n",apcode);
            
            MEM_EraseSector(0x009000);
            MEM_Program(0x009000,&saveData.apcode,4);
            NewMEM_Store_SaveData(&saveData);

            Send_RS485("OK");
        }
        else Send_RS485("ERROR");
            
    }
    else if(memcmp(msg,"DBUGUDP",7) == 0)
    {
        SaveDataUpdate update; BOOL dbugudp;
        memset(&update,0,sizeof(SaveDataUpdate));
        dbugudp = (atoi(msg+8) == 1);
        update.dbugudp = &dbugudp;
        
        if(NewMEM_ModifyMemory(&update,&saveData) != SUCCEED)
            Send_RS485("ERROR");
        else 
            Send_RS485("OK");
    }
    else if(memcmp(msg,"RECOVERY",8) == 0)
    {
        updatepgRunningFlag(0x00);
        while(1);
    }
    else if(memcmp(msg,"BACKUP",6) == 0)
    {
        if(start_FirmUpdate("/downloads/patch/v.B.kor.recovery/recovery.pgm") == SUCCEED)
        {
            Send_RS485("OK");
            firmware_backup();
        }
        else{
            Send_RS485("ERROR");
        }
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
// DBUGUDP : \02 DBUGUDP (1 or 0) \03 : Set if we use dbuglog.
// RECOVERY : \02 RECOVERY \03 : Recovery. performs program backup.
// BACKUP : \02 BACKUP \03 : Recovery. gets backup firmware and store.

void Start_DebugUDP()
{
    End_DebugUDP();
    DEVICE_INFO remote_device_info;
    DEVICE_INFO *device_info_ptr;

    // if(debug_udpIP.Val == 0xFFFFFFFF)
        device_info_ptr = NULL;//broadcast
    // else{
    //     device_info_ptr = &remote_device_info;
    //     remote_device_info.ip_address = debug_udpIP;
    //     SendARP(debug_udpIP);
    //     while(!arp_is_resolve_complete(&debug_udpIP,&remote_device_info.mac_address))yield();
    // }
    
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
    if(saveData.dbugudp == FALSE) return;
    if(debug_udp == UDP_INVALID_SOCKET) return;
    UDP_SendData(debug_udp,str,len);
}
void End_DebugUDP()
{
    if(debug_udp != UDP_INVALID_SOCKET)
        udp_close_socket(&debug_udp);
    debug_udpIP.Val = 0xFFFFFFFF;
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