
#ifdef ATTENDERAPI_C
IP_ADDR SmartIOS_API_IP = {0x00000000};
char SmartIOS_API_DOMAIN_buff[32] = "www.eduwon.net";
char *SmartIOS_API_DOMAIN = SmartIOS_API_DOMAIN_buff;
int   SmartIOS_API_PORT   = 8807;
#else
extern IP_ADDR SmartIOS_API_IP;
extern char *SmartIOS_API_DOMAIN;
extern int   SmartIOS_API_PORT;
#endif

//be careful that request(Addstr_Str*) SHOULD be NOT allocated.
Result SmartIOS_Page(char* page,Addstr_Str **request);
Result HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str **request);
Result dbuglog(char* log,BOOL isCritical);
Result API_apipc(IP_ADDR apip,IP_ADDR gwip,IP_ADDR netmsk,IP_ADDR dnsip,IP_ADDR dnsip2);

#define TIMEOUT_FOR_CONNECTING    1000
#define TIMEOUT_FOR_WAIT_RESPONSE 1500
JSONBuffer SmartIOS_API(char* page);

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
JSONBuffer Beacon_SendAPI(SensorData sensor);

Result process_with_IPCD(JSONBuffer object,int AESKey);
Result BLEList_ProcessSensor(BLEInfo *sensor,SensorData *);
JSONBuffer API_Send_BEACON(CoderBuffer X,CoderBuffer Y,char* W);
Result GETFirmwareLocation(char* PGVer,char* VerDate,Addstr_Str* location);
Result start_FirmUpdate(char* pgver);

#define FILEGETHost "www.eduwon.net"
#define FILEGETPort 20133
Result GETFile(IP_ADDR ip,int port,char* require,int* filelen,int chunkSize);
Result getChunkCallback(int stat,int num,int addr,uint8_t* buf);
void CopyFirmwareAndBackup();
void firmware_backup();
void Jump_to_Programmer();

uint8_t tcp_read_until_CRLF(uint8_t* array,int maxlen);

Result TryingReconnectInternet(IP_ADDR apip,IP_ADDR gwip,IP_ADDR netmsk,IP_ADDR dnsip,IP_ADDR dnsip2);