
#ifdef ATTENDERAPI_C
IP_ADDR AJUATTENDER_API_IP = {0x00000000};
char AJUATTENDER_API_DOMAIN_buff[32] = "www.eduwon.net";
char *AJUATTENDER_API_DOMAIN = AJUATTENDER_API_DOMAIN_buff;
int   AJUATTENDER_API_PORT   = 8807;
#else
extern IP_ADDR AJUATTENDER_API_IP;
extern char *AJUATTENDER_API_DOMAIN;
extern int   AJUATTENDER_API_PORT;
#endif

//be careful that request(Addstr_Str*) SHOULD be NOT allocated.
Result AJUAttender_Page(char* page,Addstr_Str **request);
Result HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str **request);

#define TIMEOUT_FOR_CONNECTING    1000
#define TIMEOUT_FOR_WAIT_RESPONSE 1500
cJSON *AJUAttender_API(char* page);

typedef struct 
{
    int C; // 0~9
    int D; // 1000~9999
    int key; // 0~299

} AttenderAPI_SendKey;
AttenderAPI_SendKey API_GenerateAESKey(int day,int month);

Result API_Echo();
Result API_ipcd();
Result API_ipcdRequest(int ipcdCode);

Result BLEList_ProcessSensor(BLEInfo *sensor,SensorData *);
Result Beacon_SendAPI(SensorData);
Result API_Send_BEACON(CoderBuffer X,CoderBuffer Y,char* W);

#define FILEGETHost "www.eduwon.net"
#define FILEGETPort 20133
#define FILEGETFORMAT "/downloads/patch/%s/smartios.pgm"
Result GETFile(IP_ADDR ip,int port,char* require,int* filelen,int chunkSize,Result (*getChunkCallback)(int stat,int num,int addr,uint8_t* buf));
uint8_t tcp_read_until_CRLF(uint8_t* array,int maxlen);
void Jump_to_Programmer();

Result TryingReconnectInternet(IP_ADDR apip,IP_ADDR gwip,IP_ADDR netmsk,IP_ADDR dnsip,IP_ADDR dnsip2);
//900*16
//900/64 KByte
//13.1625KByte