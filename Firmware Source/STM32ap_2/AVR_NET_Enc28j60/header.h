#ifndef _HEADER_H_
#define _HEADER_H_

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "ethernetif.h"
#include "enc28j60.h"
#include "ethernet.h"
#include "network.h"
#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"
#define _delay_us(x) up_udelay(1)
#define _delay_ms(x) HAL_Delay(x)

void up_udelay(uint32_t us);
// define IP, MAC address for you AVR
#define NET_IP 192,168,0,28
#define NET_MAC 0x15, 0x8, 0x45, 0x89, 0x69, 0x99
// define RAM buffer size for network packets
#define NET_BUFFER_SIZE 400
// define maximum tcp connection can live in AVR (server/client connections)
#define TCP_MAX_CONNECTIONS 2

#define HTTP_MAX_URL_LENGTH 255
#define HTTP_MAX_HEADER_ROWS_LENGTH 100
#define HTTP_MAX_DATA_LENGTH 1024

#define HTTP_SERVER_PORT 80

// define your cpu frequency because of delay.h library
#define F_CPU 16000000UL

unsigned char Low(const unsigned short value);
unsigned char High(const unsigned short value);
void LowPut(unsigned short *value, const unsigned char ch);
void HighPut(unsigned short *value, const unsigned char ch);
unsigned short CharsToShort(const unsigned char *value);
void CharsPutShort(unsigned char *chars, const unsigned short value);
unsigned long CharsToLong(const unsigned char *value);
unsigned char ParseLong(unsigned long *number, const unsigned char *ch, const unsigned short length);
unsigned char CharsCmp(const unsigned char *ch1, const unsigned short length1, const unsigned char *ch2, const unsigned short length2);
void CharsCat(unsigned char *destination, unsigned short *destinationLength, const unsigned char *source, const unsigned short sourceLength);

#endif