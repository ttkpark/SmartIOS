
#ifdef ATTENDERAPI_C
IP_ADDR AJUATTENDER_API_IP = {0x00000000};
char* AJUATTENDER_API_DOMAIN = "www.eduwon.net";
int   AJUATTENDER_API_PORT   = 8807;
#else
extern IP_ADDR AJUATTENDER_API_IP;
extern char* AJUATTENDER_API_DOMAIN;
extern int   AJUATTENDER_API_PORT;
#endif

//be careful that request(Addstr_Str*) SHOULD be NOT allocated.
Result AJUAttender_Page(char* page,Addstr_Str *request);
Result HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str *request);
Result Addstr_replace(Addstr_Str *inout,char* Remove,char* Replace);

#define TIMEOUT_FOR_CONNECTING    1000
#define TIMEOUT_FOR_WAIT_RESPONSE 10000
cJSON *AJUAttender_API(char* page);

Result API_Echo();

Result BLEList_ProcessSensor(BLEInfo *sensor);
Result Beacon_SendAPI(char* B,uint32_t idxUser,int F,int D);
Result API_Send_BEACON(CoderBuffer* X,CoderBuffer* Y,CoderBuffer* A,char* W);

//900*16
//900/64 KByte
//13.1625KByte