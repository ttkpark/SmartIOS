#include "main.h"
#include "ETH/eth-main.h"
#include "ETH/eth-nic.h"
#include "ETH/eth-arp.h"
#include "ETH/eth-ip.h"
#include "ETH/eth-dhcp.h"
#include "ETH/eth-dns.h"
#include "ETH/eth-icmp.h"
#include "ETH/eth-tcp.h"
#include "ETH/eth-udp.h"
#include <stdlib.h>
#include "buffer/Addstr.h"
#include "JSON/cJSON.h"
#include <string.h>

void yield();
void watchdog();
void EC_Init(IP_ADDR myIP,IP_ADDR subMsk,IP_ADDR gateway,MAC_ADDR ourMAC, int isDHCP);

void calibrate(void);

int checkUnlinkTimeout(uint32_t lastUnlinkedTime,int timeout);

void Ethernet_Reset();
void waitwithYield(uint16_t delay);

SOCKET UDP_Open(IP_ADDR AddrToOpen,WORD port_local,WORD port_remote);
LENGTH UDP_WaitReq(SOCKET Socket,int timeout);
Result UDP_SendData(SOCKET Socket,uint8_t* data,int length);


RES_GetWait TCP_Disconnect(SOCKET Socket, int timeout);
LENGTH TCP_Get_Data(SOCKET Socket,uint8_t* data,int buflen);
LENGTH TCP_Get_RemainingDataSize(SOCKET Socket);
RES_GetWait TCP_Get_Wait(SOCKET Socket,int timeout);
LENGTH TCP_SendByte(SOCKET Socket, uint8_t* data, int len);
SOCKET TCP_Connect(IP_ADDR ip,int port,int timeout);

IP_ADDR IPSet(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4);
MAC_ADDR MACSet(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,uint8_t d5,uint8_t d6);

//if error, it returns 0.0.0.0
IP_ADDR DNS_Get(char* domain, int timeout);
void checkConnectivity(BOOL print);


Result SendARP(IP_ADDR ip);
Result Send_Ping(char* host,int count);