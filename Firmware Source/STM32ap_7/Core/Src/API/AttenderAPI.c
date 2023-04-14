#ifndef ATTENDERAPI_C
#define ATTENDERAPI_C
#endif

#include "ETH/Internet.h"
#include "buffer/CoderBuffer.h"
#include "JSON/JSONBuffer.h"
#include "buffer/aes.h"
#include "buffer/base64.h"
#include "memory/memory.h"
#include "API/AttenderAPI.h"
#include "programmer/programmer.h"

const char* filename_AttenderAPI = "AttenderAPI.c";

extern const char* SMARTIOS_VER;
extern const char* SMARTIOS_DATE;

//reserved word with #   #\w+
//file "((\w+(/|\\))*)\w+(\.\w+)+"
//all the hex value 0x[0-9A-F]*
//all the pointer [^,/ )(\n]\w+( |)([*]+)

extern Date nowDate;
extern SavedData saveData;
int IPCDResultCode = 1; // if this becomes 3, ipcd3 is to be sended. and 0, ipcd0 is to be sended. finally 1, It's normal state.

extern int isOnline;
extern MAC_ADDR myMAC;
//
//API Code

void CleanTCPSocket(SOCKET socket)
{
  extern BYTE tcp_tx_resp_after_proc_rx_resp_flags;

  Debug("TCP: socket flags\n");
  Debug("DO WE NEED TO SEND A RESPONSE : (%d > 0) & (%d > 0)\n" ,tcp_rx_packet_is_waiting_to_be_processed ,tcp_tx_resp_after_proc_rx_resp_flags);
  Debug("IS SOCKET READY TO TX         : (%d == 1) || (%d == 0)\n" ,tcp_socket[socket].flags.tx_last_tx_awaiting_ack ,tcp_socket[socket].flags.ready_for_tx);

  tcp_close_socket(socket);
}
AttenderAPI_SendKey API_GenerateAESKey(int day,int month)
{
  AttenderAPI_SendKey key;
  key.C = saveData.apcode;//rando()%10;//0~9
  key.D = (-rando())%9000 + 1000;//1000~9999

  int mux = 1;
  while((mux=mux*10) <= key.C)yield();
  int DC = key.C + key.D*mux;
  //printf("DC = %d\n",DC);

  key.key = DC%NUM_KEYS;//saveData.aes_no;//((day+month)/2 + key.C - 1)*10 + key.D%10;
  return key;
}

//API call function which calls request and gets reponse.
Result SetSmartIOSAPI(char* ServerURL)
{
  // http://www.eduwon.net:8807
  // http :// www.eduwon.net : 8807
  char* firstColon = strchr(ServerURL,':');
  if(firstColon == NULL)return FAILED;

  char* secondColon = strchr(firstColon+1,':');

  if(secondColon == NULL)return FAILED; // if "http://" is gone -> It's error.

  int domainlen = strlen(firstColon) - strlen(secondColon) - 3;
  memcpy(SmartIOS_API_DOMAIN_buff, firstColon+3, domainlen);
  SmartIOS_API_DOMAIN_buff[domainlen] = '\0'; //null termination

  SmartIOS_API_PORT = atoi(secondColon+1);
  printf("domain : %s:%d",SmartIOS_API_DOMAIN,SmartIOS_API_PORT);
  return SUCCEED;
}
JSONBuffer SmartIOS_API(char* page)
{
  Addstr_Str *req = NULL; //fix 
  if(SmartIOS_Page(page,&req) == SUCCEED)
  {
    Debug("SmartIOS_Page ended\n");
    JSONBuffer Object = JSONBuffer_make(req);
    if(Object == NULL){
      Addstr_destroy(req);
      Debug("SmartIOS_API null failed\n");
      return NULL;
    }
    Debug("SmartIOS_Page.json_create start\n");
    Object->json = json_create(req->str,Object->pool,20);
    Debug("SmartIOS_Page.json_create end\n");
    
    Debug("SmartIOS_API ended\n");
    return Object;
  }
  else
  {
    Addstr_destroy(req);
    Debug("SmartIOS_API failed\n");
    return NULL;
  }
}
Result SmartIOS_Page(char* page,Addstr_Str **reqptr)
{
  uint32_t start = HAL_GetTick();
  Debug("SmartIOS_Page start: %dms\n",HAL_GetTick() - start);
  //Debug("DNS_Get!\n");
  if(SmartIOS_API_IP.Val == 0x00000000){
    //Update SmartIOS_API_IP.
    SmartIOS_API_IP = DNS_Get(SmartIOS_API_DOMAIN,4000);
    if(SmartIOS_API_IP.Val == 0x00000000)
    {
      Debug("DNS Failed.\n");
      return FAILED;//ERROR
    }
  }
  Debug("SmartIOS_Page dns end: %dms\n",HAL_GetTick() - start);
  
  Debug("Page : %s\n",page);

  //Debug("421!\n");
  Addstr_Str* Require;
  Addstr_init(&Require);
  Addstr_add(Require,"GET ");
  Addstr_add(Require,page);
  Addstr_add(Require,"\r\n");
  
  Addstr_add(Require,"Host :");
  Addstr_add(Require,SmartIOS_API_DOMAIN);
  Addstr_add(Require,"\r\n");
  
  Addstr_add(Require,"Connection: close\r\n");
  Addstr_add(Require,"\r\n");
  char* require = Addstr_return(Require);


  //Debug("436!\n");

  Debug("SmartIOS_Page require prep end: %dms\n",HAL_GetTick() - start);
  Result ret = HTTPSendGET(SmartIOS_API_IP,SmartIOS_API_PORT,require,reqptr);
  
  Addstr_destroy(Require);
  if(ret != SUCCEED)
  {
    Debug("133!\n");
    Addstr_destroy(*reqptr);
    return ret;
  }
  Debug("HTTPSend GET!\n");

  Addstr_replace(*reqptr,"\"[","");
  Addstr_replace(*reqptr,"]\"","");
  Addstr_replace(*reqptr,"\\\"","\"");

  Debug("SmartIOS_Page end: %dms\n",HAL_GetTick() - start);

  //Debug("%s\n",request->str);
  Debug("returns!\n");
  return ret;
}

//HTTP GET을 호출하여 Response를 받아오는 함수.
Result HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str **reqptr)
{
  uint32_t start = HAL_GetTick();
  Debug("HTTPSendGET start: %dms\n",HAL_GetTick() - start);
  SOCKET socket;
  
  int retries = 3;
  while((retries--) != 0)
  {
    yield();
    socket = TCP_Connect(ip,port,TIMEOUT_FOR_CONNECTING);
    if(socket != TCP_INVALID_SOCKET)
        break;
  }
  if(retries == 0)
  {
    Debug("TCP connection %d Retries expired.\n",2);
    return FAILED;
  }
  if(socket == TCP_INVALID_SOCKET)
    return FAILED;
  
  Debug("HTTPSendGET connected: %dms\n",HAL_GetTick() - start);
  sending:
  if (!tcp_setup_socket_tx(socket))
  {
    CleanTCPSocket(socket);
    Debug("TCP: Can't tx right now - try again next time\n");
    return FAILED;//Can't tx right now - try again next time
  }

  //SEND THE PACKET
  tcp_write_array(require,strlen(require));
  tcp_socket_tx_packet(socket);



  Debug("HTTPSendGET sent: %dms\n",HAL_GetTick() - start);
  //Waiting for response
  RES_GetWait res = TCP_Get_Wait(socket,TIMEOUT_FOR_WAIT_RESPONSE);

  switch (res){
  case TIMEOUT:
    tcp_close_socket(socket);
    Debug("TCP: TCP_Get_Wait TIMEOUT\n");
    return FAILED;

  case SUCCESS:
    ;
    Debug("HTTPSendGET got: %dms\n",HAL_GetTick() - start);
    Debug("request len = %ld\n",TCP_Get_RemainingDataSize(socket));
    if(TCP_Get_RemainingDataSize(socket) > ADDSTR_MAX_SIZE)
    {
      Debug("request len is too large. exit.\n");
      tcp_dump_rx_packet();
      CleanTCPSocket(socket);
      return FAILED;
    }
    Addstr_initlen(reqptr, TCP_Get_RemainingDataSize(socket)+1);
    Addstr_Str* request = *reqptr;
    //Debug("TCP_Get_Wait SUCCESS!\n");
    Debug("request bank = %d\n",request->bank);
    // request->size = TCP_Get_RemainingDataSize(socket);
    // request->str  = (char*)malloc(request->size+1);

    //if(tcp_check_socket_for_rx(socket) == 0) Debug("rx handle error!\n");
    BYTE *pointer = request->str;
    while(tcp_read_next_rx_byte(pointer++) == 1);
    *pointer = 0;

    //Debug("num:%d\n",(void*)pointer - (void*)request->str + 1);
    Debug("str:%s\n",request->str);
    Debug("HTTPSendGET buffered: %dms\n",HAL_GetTick() - start);

    //DUMP THE PACKET
    tcp_dump_rx_packet();
    break;
  case RETRIES_EXPIRED:
    Debug("TCP: re-sending Packet.\n");
    //RE-SEND LAST PACKET TRANSMITTED
    //(TCP requires resending of packets if they are not acknowledged and to
    //avoid requiring a large RAM buffer the application needs to remember
    //the last packet sent on a socket so it can be resent if requried).
    goto sending;
    //return FAILED;//we will not support re-sending.
  case DISCONNECTED:
    Debug("TCP: Host Disconnected.\n");
    return FAILED;
  }

  //Debug("TCP: try TCP_Disconnect.\n");
  if(TCP_Disconnect(socket,1000) != SUCCEEDED){
    Debug("TCP: try TCP_Disconnect failed.\n");
    //return FAILED;//TCP: DisConnect failed.
  }

  Debug("HTTPSendGET end: %dms\n",HAL_GetTick() - start);
  //Debug("TCP: disconnect succeed.\n");
  return SUCCEED;//END.
}


//IPC0,IPC3 = ipcd 결과 리턴 API (이걸 ipcdRequest으로 구현함.)
Result API_ipcdRequest(int ipcdCode)
{
  AttenderAPI_SendKey key = API_GenerateAESKey(nowDate.month,nowDate.day);

  char buffer[20];
  sprintf(buffer,"%d^%d",saveData.apcode,ipcdCode);
  Debug("buffer : %s",buffer);
  Debug("key : %d,%d,%d\n",key.C,key.D,key.key);
  CoderBuffer Encoded_id1 = Coder_Base64Encode(Coder_Base64Encode(Coder_AESEncode(CoderBuffer_clone(buffer,strlen(buffer)),key.key)));
  
  Addstr_Str *sendbuffer;
  Addstr_initlen(&sendbuffer,30);
  Addstr_add(sendbuffer,"/api/sio/");
  Addstr_addlen(sendbuffer,Encoded_id1->buffer,Encoded_id1->len);

  CoderBuffer_free(Encoded_id1);
  
  sprintf(buffer,"/%d%d/apflgc",key.D,key.C);
  Addstr_add(sendbuffer,buffer);

  JSONBuffer Object = SmartIOS_API(Addstr_return(sendbuffer));
  Addstr_destroy(sendbuffer);
  if(Object == 0)return FAILED;
  if(Object->json == 0){JSONBuffer_free(Object);return FAILED;}

  int rtncode = atoi(json_getPropertyValue(Object->json,"rtncode"));

  Debug("returncode:%d\n",rtncode);
  
  // if(rtncode == 8)
  //   UpdateAesnoAsDefault();
    
  if(rtncode != 1)
  {
    Debug("Failed.\n");
    JSONBuffer_free(Object);
    return FAILED;
  }else{
    Debug("Succeed!\n");
    JSONBuffer_free(Object);
    return SUCCEED;
  }
}

//IPCD
Result API_ipcd()
{
  // > 화면에 Name, ID
  // 1) 구조 : api/sio/{id}/{val}/{id2}/{val2}
  //       /api/sio/암호화(1^v.B.kor.0.116^7.5)/암호화(1800^3)/aes_no/ipcd
  //       -- {id}암호화(idxEquip^pgver^dist) : Base64(AES암호화.aes_no(1^v.B.kor.0.116^7.5))
  //       -- {val}암호화(ipcdInterval^resnd^apip)
  //       -- {id2}aes_no : 임의값(1000~9999) + idxEquip
  //       -- {val2}ipcd (IP Change Detection)
  // > 리턴값이 {"rtncode" : "01", "msg", "암호화(0^0^0^0^)"} 이면 모두 무시하면 됨.

  Debug("[%lums] Send_API_ipcd\n",HAL_GetTick());

  AttenderAPI_SendKey key = API_GenerateAESKey(nowDate.month,nowDate.day);

  IP_ADDR *ips[5] = {&saveData.apip,&saveData.gwip,&saveData.netmask,&saveData.dnsip,&saveData.dns2ip};
  uint8_t ipsBuffer[20];
  for(int i=0;i<4*5;i++) ipsBuffer[i] = ips[i>>2]->v[i&3]; // i/4 => i>>2  i%4 => i&3
  CoderBuffer ipsBase64ed = Coder_Base64Encode(CoderBuffer_clone(ipsBuffer,20));
  ipsBase64ed->buffer[ipsBase64ed->len] = '\0';//null termination

  char buffer[64] = {0,};
  Debug("key : %d,%d,%d\n",key.C,key.D,key.key);
  sprintf(buffer,"%d^%s^%g",saveData.apcode,SMARTIOS_VER,saveData.dist); //%g는 float값이 정수 일때 소수점을 출력하지 않는다.
  CoderBuffer idEncoded = Coder_Base64Encode(Coder_Base64Encode(Coder_AESEncode(CoderBuffer_clone(buffer,strlen(buffer)),key.key)));
  Debug("id : %s\n",buffer);
  sprintf(buffer,"%d^%d^%s",saveData.ipcdInterval,saveData.resnd,ipsBase64ed->buffer);
  CoderBuffer valEncoded = Coder_Base64Encode(Coder_Base64Encode(Coder_AESEncode(CoderBuffer_clone(buffer,strlen(buffer)),key.key)));
  Debug("val : %s\n",buffer);

  Addstr_Str *sendbuffer;
  Addstr_initlen(&sendbuffer,30);
  Addstr_add(sendbuffer,"/api/sio/");
  Addstr_addlen(sendbuffer,idEncoded->buffer,idEncoded->len);
  Addstr_add(sendbuffer,"/");
  Addstr_addlen(sendbuffer,valEncoded->buffer,valEncoded->len);

  CoderBuffer_free(idEncoded);
  CoderBuffer_free(valEncoded);
  CoderBuffer_free(ipsBase64ed);

  sprintf(buffer,"/%d%d/ipcd",key.D,key.C);
  Addstr_add(sendbuffer,buffer);

  Debug("Timer_ipcd.API_ipcd.SmartIOS_API start\n");
  JSONBuffer Object = SmartIOS_API(Addstr_return(sendbuffer));
  Addstr_destroy(sendbuffer);
  Debug("Timer_ipcd.API_ipcd.SmartIOS_API end\n");

  if(Object == 0){
    Debug("Timer_ipcd.API_ipcd null ended\n");
    return FAILED;
  }

  char* rtnCode = json_getPropertyValue(Object->json,"rtncode");
  if(rtnCode == 0){ //if string is null, error code will activate.
    Debug("Timer_ipcd.API_ipcd rtnCode null ended\n");
    JSONBuffer_free(Object);
    return FAILED;
  }

  int rtncode = atoi(rtnCode);

  Debug("returncode:%d\n",rtncode);

  if(rtncode != 1 && rtncode != 16)
  {
    Debug("API_ipcd Failed.\n");
    JSONBuffer_free(Object);
    return FAILED;
  }
  else
  {    
    yield();
    Debug("Timer_ipcd.API_ipcd.process_with_IPCD start\n");
    Result code = process_with_IPCD(Object,key.key);
    Debug("Timer_ipcd.API_ipcd.process_with_IPCD end\n");

    JSONBuffer_free(Object);
    return code;
  }
}
Result process_with_IPCD(JSONBuffer object,int AESKey)
{
  //   정상 리턴 : {"rtncode" : "01", "msg", "암호화(1^5.5^1800^3^wKgAZsCoAAH///8AqH4/AQAAAAA=)"}
  //   또는 {"rtncode" : "01", "msg", "암호화(0^0^0^0^)"}
  //    : API에서는 해당 값이 변경되었을 경우에 값을, 아니면 0 또는 공백을 리턴한다.
  // - 01: 정상
  // - msg.split('^') : 
  // [0] 프로그램 패치여부 : 0=무시, 1=프로그램 패치 필요
  // [1] 거리값 : 0=무시, 값>0이면 거리값 변경
  // [2] ipcdInterval값 : 0=무시, 값>0이면 ipcdInterval값(AP가 서버에 변경정보를 구하고 Alive 하는 주기)으로 변경
  // [3] resnd값 : 0=무시, 값>0이면 resnd값(x분이 지난 후 재출입전송(AP->API))으로 변경
  // [4] apip: ''=무시, 값!=''이면 wKgAZsCoAAH///8AqH4/AQAAAAA=
  // > 나머진 오류 리턴 : {"rtncode" : "0?"}
  
  
  //printf("process_with_IPCD start\n");
  Debug("Timer_ipcd.API_ipcd.process_with_IPCD.json_getPropertyValue start\n");
  char* msgEncoded = json_getPropertyValue(object->json,"msg");
  Debug("Timer_ipcd.API_ipcd.process_with_IPCD.json_getPropertyValue end\n");

  if(msgEncoded == NULL){
    printf("%s:%d return FAILED.\n",filename_AttenderAPI,__LINE__);
    return FAILED;
  }
  Debug("Timer_ipcd.API_ipcd.process_with_IPCD.Coder_AESDecode start\n");
  CoderBuffer msgBuffer = Coder_AESDecode(Coder_Base64Decode(Coder_Base64Decode(CoderBuffer_clone(msgEncoded,strlen(msgEncoded)))),AESKey);
  char* msg = msgBuffer->buffer;
  if(msg != NULL)
    msg[msgBuffer->len] = '\0'; // null termination
  Debug("Timer_ipcd.API_ipcd.process_with_IPCD.Coder_AESDecode end\n");
  
  JSONBuffer_free(object);

  if(msg == NULL || strlen(msg) < 8){
    printf("%s:%d return FAILED.\n",filename_AttenderAPI,__LINE__);
    CoderBuffer_free(msgBuffer);
    return FAILED;
  }
  
  char* focuses[4];
  char* focus = msg-1;
  
  Debug("msg : %s\n",msg);

  //focuses 의 내용을 모두 채울때까지 '^'를 검색한다.
  for(int i=0;i<sizeof(focuses)/sizeof(char*);i++){
    //위치가 배열의 맨 끝이거나 그 너머에 있으면 FAILED
    if(focus >= msg + strlen(msg)){
      printf("%s:%d return FAILED.\n",filename_AttenderAPI,__LINE__);
      CoderBuffer_free(msgBuffer);
      return FAILED;
    }
    //현재위치의 다음부터 검색한다.
    focus = strstr(focus+1,"^");
    if(focus == NULL){
      printf("%s:%d return FAILED.\n",filename_AttenderAPI,__LINE__);
      CoderBuffer_free(msgBuffer);
      return FAILED;
    }
    focuses[i] = focus;
  }


  float distance = atof(focuses[0]+1);
  int ipcdInterval = atoi(focuses[1]+1);
  int resnd = atoi(focuses[2]+1);
  char* ips_str = focuses[3]+1;

  int firmverLen = focuses[0] - msg;
  *focuses[0] = '\0'; // null termination
  char* NewfirmwareVer = msg;//first parameter
  BOOL shouldFirmwareUpdate = (firmverLen>4) ? TRUE : FALSE;

  Debug("shouldUpdate : %d\n",shouldFirmwareUpdate);
  Debug("distance : %g\n",distance);
  Debug("ipcdinterval : %d\n",ipcdInterval);
  Debug("resnd : %d\n",resnd);
  Debug("ips_str : %s\n",ips_str);

  volatile BOOL shouldUpdateMemory = FALSE;
  volatile BOOL shouldResetInternet = FALSE;
  
  SaveDataUpdate updateData;
  memset(&updateData,0,sizeof(SaveDataUpdate));

  //1. General Settings Update
  if(distance != 0.f && distance > 0.1f)updateData.dist = &distance;
  if(ipcdInterval != 0 && ipcdInterval > 1)updateData.ipcdInterval = &ipcdInterval;
  if(resnd != 0 && resnd != saveData.resnd)updateData.resnd = &resnd;


  //2.Firmware Check
  //should we do Firware Update?!    
  if(shouldFirmwareUpdate)
  {
    printf("WE SHOULD DO FIRMWARE UPDATE!!\n");
    
    Addstr_Str *str;
    Addstr_initlen(&str,12);
    if(GETFirmwareLocation(NewfirmwareVer,SMARTIOS_DATE,str) == SUCCEED)
      if(start_FirmUpdate(str->str) == SUCCEED)CopyFirmwareAndBackup();

    Addstr_destroy(str);
  }

  //3. IP Settings Update
  CoderBuffer ips_buffer_transit = CoderBuffer_clone(ips_str,strlen(ips_str));
  CoderBuffer ips_buffer = Coder_Base64Decode(ips_buffer_transit);
  IP_ADDR *ips = NULL;
  if(ips_buffer != NULL)
  {
    ips = ips_buffer->buffer;
    printf("[%ld]",ips_buffer->len);
    if(ips_buffer->len == 20){
      printf("received IP\n");
      
      
      if(ips[0].Val != 0x00000000)printf("apip : %s\n",Convert_IP2Str(ips[0])); else printf("apip : %s no change\n",Convert_IP2Str(saveData.apip));
      if(ips[1].Val != 0x00000000)printf("gwip : %s\n",Convert_IP2Str(ips[1])); else printf("gwip : %s no change\n",Convert_IP2Str(saveData.gwip));
      if(ips[2].Val != 0x00000000)printf("nmsk : %s\n",Convert_IP2Str(ips[2])); else printf("nmsk : %s no change\n",Convert_IP2Str(saveData.netmask));
      if(ips[3].Val != 0x00000000)printf("dns1 : %s\n",Convert_IP2Str(ips[3])); else printf("dns1 : %s no change\n",Convert_IP2Str(saveData.dnsip));
      if(ips[4].Val != 0x00000000)printf("dns2 : %s\n",Convert_IP2Str(ips[4])); else printf("dns2 : %s no change\n",Convert_IP2Str(saveData.dns2ip));
      if((ips[0].Val != our_ip_address.Val && our_ip_address.Val != 0 && ips[0].Val != 0)
      || (ips[1].Val != our_gateway_ip_address.Val && our_gateway_ip_address.Val != 0 && ips[0].Val != 0)
      || (ips[2].Val != our_subnet_mask.Val && our_subnet_mask.Val != 0 && ips[0].Val != 0)
      )
      {
        shouldResetInternet = TRUE;
      }

      if( (ips[3].Val != saveData.dnsip.Val && ips[3].Val != 0)
      || (ips[4].Val != saveData.dns2ip.Val && ips[4].Val != 0))
      {
        updateData.dnsip = &ips[3];
        updateData.dns2ip = &ips[4];
      }
    }
  }
  CoderBuffer_free(ips_buffer);
  
  if(shouldResetInternet && ips != NULL)
  {
    if(TryingReconnectInternet(ips[0],ips[1],ips[2],ips[3],ips[4]) == SUCCEED)
    {
      printf("TryingReconnectInternet %s","Succeed!\n");
      IPCDResultCode = 3;
      saveData.isDHCP = FALSE;

      updateData.apip = &ips[0];
      updateData.gwip = &ips[1];
      updateData.netmask  = &ips[2];
      updateData.dnsip = &ips[3];
      updateData.dns2ip = &ips[4];
      updateData.isDHCP = &saveData.isDHCP;

      dbuglog("NET Reset Succeed",TRUE);
    }
    else 
    {
      printf("TryingReconnectInternet %s","Failed.\n");
      IPCDResultCode = 0;
      dbuglog("NET Reset Failed",TRUE);
      
      //Reset the Network
      resetNetwork();//make offline
      EC_Init(saveData.apip,saveData.netmask,saveData.gwip,myMAC,TRUE);
    }

  }
  CoderBuffer_free(msgBuffer);
  printf("IPCD Memory Update after IP Change.\n");
  if(NewMEM_ModifyMemory(&updateData,&saveData) == FAILED){
    printf("%s:%d return FAILED.\n",filename_AttenderAPI,__LINE__);
    return FAILED;
  }

  if(IPCDResultCode == 1)//if default value for IPCDResultCode
    IPCDResultCode = 3;//if there was no change(= it's not IP change mode), Send Result code 3(Done).
  
  return SUCCEED;
}


// IPCD.IP Change
//1  . 받은 IP로 수동 연결 해본다.
//2-1. 실패하면 원래 세팅으로 원상복귀 -> 실패 시 플래그를 켜놓고 대기. 바로라도 나중에라도 연결되면 반드시 ipc0을 보냄. 
//2-2. 성공하면 ipc3을 보냄.
Result TryingReconnectInternet(IP_ADDR apip,IP_ADDR gwip,IP_ADDR netmsk,IP_ADDR dnsip,IP_ADDR dnsip2)
{
  printf("Trying Reconnect Internet via %s..\n",Convert_IP2Str(apip));
  IPCDResultCode = 0;//make default Failed state.

  Result echo_res = SUCCEED;
  End_DebugUDP();

  resetNetwork();//make offline

  uint32_t netResetTimeout = HAL_GetTick();//timeout 20s

  EC_Init(apip,netmsk,gwip,myMAC,FALSE);
  
  checkConnectivity(TRUE);

  //
  while((!(nic_linked_and_ip_address_valid && isOnline)) && (HAL_GetTick() - netResetTimeout < 20000)){
    yield();
    if(nic_linked_and_ip_address_valid)
      checkConnectivity(TRUE);
  } //go out if connected.

  if(HAL_GetTick() - netResetTimeout >= 20000)
  {
    Debug("Reconnect Internet failed.(timeout)\n");
    return FAILED;
  }
  checkConnectivity(TRUE);

  printf("delay 1s...\n");
  waitwithYield(1000);

  printf("Echo test!\n");
  for(int i=0;i<4;i++){
    printf("Echo attempt %d of 4...\n",i);

    if(echo_res = API_Echo() == SUCCEED){
      Start_DebugUDP();
      checkConnectivity(TRUE);
      printf("Echo attempt succeed!\n");
      return SUCCEED;
    }
    waitwithYield(500);
  }

  checkConnectivity(TRUE);
  printf("Echo attempt 4 of 4 failed.\n");
  return FAILED;
}

//
// IPCD.Program Update
// It is callbacked on every received "getFile's chunkSize" bytes.

Result start_FirmUpdate(char* pgver)
{
  if(pgver == NULL)return FAILED;
  // Addstr_Str *str;
  // Addstr_init(&str);
  // sprintf(str->str,FILEGETFORMAT,pgver);
  IP_ADDR FirmUpdateIP = DNS_Get(FILEGETHost,4000);

  Addstr_Str* Require;
  Addstr_init(&Require);
  Addstr_add(Require,"GET ");
  Addstr_add(Require,pgver);
  Addstr_add(Require," HTTP/1.1\r\n");
  Addstr_add(Require,"Host :"FILEGETHost"\r\n");
  Addstr_add(Require,"Connection: close\r\n");
  Addstr_add(Require,"\r\n");
  char* require = Addstr_return(Require);
  printf(require);
  
  //Addstr_destroy(str);
  Result res = FAILED;
  if(FirmUpdateIP.Val != 0)
  {
    res = GETFile(FirmUpdateIP,FILEGETPort,require,NULL,1024);
    
    printf("GETFILE %s",res==SUCCEED?"SUCCEED\n":"FAILED\n");
  }
  else 
  {
    FirmUpdateIP = DNS_Get(FILEGETHost,4000);
    printf("error finding IP of update server.\n");
  }
  Addstr_destroy(Require);  
  return res;
}
void memory_Print(uint8_t *ptr,uint32_t len)
{
    uint8_t* origin = ptr;
    extern IWDG_HandleTypeDef hiwdg;
    for(;len>0;)
    {
      int readlen = len>16 ? 16 : len;
      watchdog();

      printf("%05X : ",ptr - origin);
      for(int j=0;j<readlen;j++)
          printf("%02X ",ptr[j]);
      printf("\n");

      ptr += readlen;
      len -= readlen;
    }
}
Result HTTPResponse(uint8_t *httpResponse,int *response,int *content_length,int *header_length)
{
  volatile char HTTP_Array[48];
  char *focus,*focusCRLF,*bufpointer = httpResponse;
  
  //parse HTTP Response Header.
  if((focus=strstr(bufpointer,"Content-Length:")) != 0)
    *content_length = atoi(focus + 15);
  
  if((focus=strstr(bufpointer,"HTTP/1.1 ")) != 0)
    *response = atoi(focus + 9);

  focus = strstr(bufpointer,"\r\n\r\n");

  Debug("Response = %d\n",*response);
  Debug("Content-Length = %d\n",*content_length);
  *header_length = (uint32_t)((char*)focus - (char*)httpResponse) + 4;
  Debug("HeaderLength = %d\n",*header_length);
  return SUCCEED;
}
void writeFirmware2FLASH_GETFileResult(uint32_t src,uint32_t dest,uint32_t length)
{
  uint8_t buf[256] = {0,};
  Debug("New Firmware Write start.\n");
  Debug("erasing %05X -> %05X\n",src,dest);
  for(int i=0;i<((length-1)/4096)+1;i++)
  {
    MEM_EraseSector(dest + i*4096);
    watchdog();
  }

  Debug("writing %05X -> %05X\n",src,dest);
  //Fetch and write.
  for(int i=0;i<((length-1)/256)+1;i++)
  {
    Result res = SUCCEED;
    res |= MEM_NormalRead(src+(i<<8),buf,256);

    if(i == 1)
    {
      uint8_t a;
      for(int j=0;j<16;j++)
      {
        a = buf[j];
        buf[j] = buf[31-j];
        buf[31-j] = a;
      }
    }

    res |= MEM_PageProgram(dest+(i<<8),buf,256);
    if(res){i--;Debug("Fail.retrying..%05X\n",dest+(i<<8));}
    watchdog();
  }
  
  Debug("New Firmware Write end.\n");
  MEM_Print(dest,256);
}

//0C0000h~0DFFFFh
// getFile에서 TCP로 데이터를 받으면 방대한 Response를 RAM에는 저장을 할 수 없기 때문에 FLASH에 저장해야 한다.
// FLASH를 데이터를 4KByte마다 지우고(erase), 한번에 256바이트까지 쓸 수 있다.
// 따라서 MTU를 256Byte의 배수인 1KByte로 함으로써 데이터를 받고 쪼갤 때 더 쉽게 플래시에 저장할 수 있도록 하였으며,
// 이 콜백을 1KByte마다 호출하여 FLASH에 쓰기 쉽도록 해주는 함수다.
void CopyFirmwareAndBackup()
{
  Debug("CopyFirmwareAndBackup start\n");
  uint8_t buf[512] = {0,};
  MEM_NormalRead(0xA0000,buf,511);
  buf[511] = 0;//null termination
  Debug("buf = %s\n",buf);

  int Content_length = 0;
  int Response = 0;
  int HeaderLength = 0;

  HTTPResponse(buf,&Response,&Content_length,&HeaderLength);
  int start_addr = 0xA0000 + HeaderLength;

  // if it's not 200 OK signal
  if(Response != 200){
    Debug("HTTP Response is NOT 200 OK. Resp = %d\n",Response);
    return;
  }

  //error check. if no expected header type, It can be Invalid file segment.
  if(HeaderLength == 0)
  {
    Debug("Invlaid header Error.\n");
    return;//exit this procedure.
  }
  yield();

  writeFirmware2FLASH_GETFileResult(0xA0000+HeaderLength,0xC0000,Content_length);

  Debug("Now Jump to Programmer. RESET\n");
  Debug("N0x0C0000. RESET\n");
  Jump_to_Programmer();
}
void firmware_backup()
{
  Debug("firmware_backup start\n");
  uint8_t buf[512] = {0,};
  MEM_NormalRead(0xA0000,buf,511);
  buf[511] = 0;//null termination
  Debug("buf = %s\n",buf);

  int Content_length = 0;
  int Response = 0;
  int HeaderLength = 0;

  HTTPResponse(buf,&Response,&Content_length,&HeaderLength);
  int start_addr = 0xA0000 + HeaderLength;

  // if it's not 200 OK signal
  if(Response != 200){
    Debug("HTTP Response is NOT 200 OK. Resp = %d\n",Response);
    return;
  }

  //error check. if no expected header type, It can be Invalid file segment.
  if(HeaderLength == 0)
  {
    Debug("Invlaid header Error.\n");
    return;//exit this procedure.
  }
  yield();

  writeFirmware2FLASH_GETFileResult(0xA0000+HeaderLength,0xE0000,Content_length);
}
void Jump_to_Programmer()
{
  HAL_NVIC_SystemReset();
}

// http://www.eduwon.net:20133/downloads/patch/v.kr.0.101/smartios.pmg
//UPDATE /downloads/patch/v.B.kor.0.118/smartios.pmg
// http://www.eduwon.net:20133/downloads/patch/v.B.kor.0.118/smartios.pmg
Result GETFirmwareLocation(char* PGVer,char* VerDate,Addstr_Str* location)
{
  if(PGVer == NULL)return FAILED;
  if(VerDate == NULL)return FAILED;
  if(location == NULL)return FAILED;
  // 1) 구조 : api/pat/{id}/{val}/{id2}
  // /api/pat/암호화(v.kr.0.100)/aes_no/get
  // -- {id}암호화(PGVer) : Base64(AES암호화.aes_no(v.B.kr.0.100))
  // -- {val}aes_no : 임의값(1000~9999) + idxEquip
  // -- {id2}get

  AttenderAPI_SendKey key = API_GenerateAESKey(nowDate.month,nowDate.day);

  char buffer[48] = {0,};
  sprintf(buffer,"%s",PGVer);
  Debug("key : %d,%d,%d\n",key.C,key.D,key.key);
  Debug("buffer : %s\n",buffer);
  CoderBuffer Clientcode_N_idxEquip = Coder_Base64Encode(Coder_Base64Encode(Coder_AESEncode(CoderBuffer_clone(buffer,strlen(buffer)),key.key)));
  
  Addstr_Str *sendbuffer;
  Addstr_initlen(&sendbuffer,30);
  Addstr_add(sendbuffer,"/api/pat/");
  Addstr_addlen(sendbuffer,Clientcode_N_idxEquip->buffer,Clientcode_N_idxEquip->len);

  CoderBuffer_free(Clientcode_N_idxEquip);

  sprintf(buffer,"/%d%d/get",key.D,key.C);
  Addstr_add(sendbuffer,buffer);


  JSONBuffer Object = SmartIOS_API(Addstr_return(sendbuffer));
  Addstr_destroy(sendbuffer);

  if(Object == 0){
    Debug("GetFirmware null ended\n");
    return FAILED;
  }

  int rtncode = atoi(json_getPropertyValue(Object->json,"rtncode"));

  printf("returncode:%d\n",rtncode);

  char* data = json_getPropertyValue(Object->json,"data");
  if(data == 0)Debug("data:%d\n",data);

  if(rtncode == 1)
  {
    char* str = strstr(data,"/downloads/patch");
    if(str == NULL){
      JSONBuffer_free(Object);
      return FAILED;
    }else{
      printf("IPCD %s!!\n",str);
      Addstr_add(location,str);
      JSONBuffer_free(Object);
      return SUCCEED;
    }
  }
  else
  {
    JSONBuffer_free(Object);
    return FAILED;
  }
  //리턴 패킷 : {"rtncode" : "01", "data":"웹 폴더/파일명.bin"}
}
//getChunkCallback : stat==1 : begin() stat==2 : callback(num=len,addr=Addr,buf=Buffer) stat==3:end(num=wholedatalen)
Result GETFile(IP_ADDR ip,int port,char* require,int* filelen,int chunkSize)
{
  getChunkCallback(1,(int)NULL,(int)NULL,NULL);

  SOCKET socket;
  
  int retries = 2;
  while((retries--) != 0)
  {
    yield();
    socket = TCP_Connect(ip,port,TIMEOUT_FOR_CONNECTING);
    if(socket != TCP_INVALID_SOCKET)
        break;
  }
  if(retries == 0)
  {
    printf("GETFILE TCP connection %d Retries expired.\n",2);
    return FAILED;
  }
  if(socket == TCP_INVALID_SOCKET)
    return FAILED;

  sending:
  if (!tcp_setup_socket_tx(socket))
  {
    CleanTCPSocket(socket);
    Debug("TCP: Can't tx right now - try again next time\n");
    return FAILED;//Can't tx right now - try again next time
  }

  //SEND THE PACKET
  tcp_write_array((BYTE*)require,strlen(require));
  tcp_socket_tx_packet(socket);

  int TCP_Count = 0;
  Addstr_Str *request;
  BYTE *pointer;
  int Data_Actual_Length = 0;
  uint32_t firstACKN,nowACKN; firstACKN = 0,nowACKN = 0;

  Addstr_initlen(&request, ADDSTR_MAX_SIZE);
  pointer = (BYTE*)request->str;

  int packetlen = 0;
  int quit = 0;
  //continue Receiving Packet.
  while(!quit)
  {
    switch (TCP_Get_Wait(socket,TIMEOUT_FOR_WAIT_RESPONSE)){
    case TIMEOUT:tcp_close_socket(socket);Debug("TCP: TCP_Get_Wait TIMEOUT\n");Addstr_destroy(request);return FAILED;
    case SUCCESS:;
      packetlen = tcp_socket[socket].rx_data_bytes_remaining;
      if(TCP_Count==0)
      {
        firstACKN = tcp_socket[socket].send_acknowledgement_number - packetlen;
      }
      nowACKN = tcp_socket[socket].send_acknowledgement_number;
      if(packetlen < MAX_TCP_DATA_LEN)nowACKN -= 1;

      //Debug("received:(Count=%d,len=%d,cosket=%d,seq=%d,ack=%d)\n",TCP_Count,packetlen,socket,tcp_socket[socket].send_sequence_number,nowACKN-firstACKN);
      for(int i=0;i<packetlen;i++){
        //dividing by chunkSize
        if((int)pointer - (int)request->str >= chunkSize)
        {
          //if ChunkSize Buffer full
          *pointer = 0;//null termination for print
          
          if(Data_Actual_Length-chunkSize != nowACKN-firstACKN + i- chunkSize - packetlen)
            Debug("Position is NOT Accurate! nowpos=%d or %ld\n",Data_Actual_Length-chunkSize,nowACKN-firstACKN + i - chunkSize - packetlen);
          
          Debug("i=%d,len=%d,addr=%05lX\n",i,(int)pointer - (int)request->str,nowACKN-firstACKN + i - chunkSize - packetlen);
          //show 256BData
          Result res = FAILED;
          if(getChunkCallback != NULL)
            res = getChunkCallback(2,chunkSize,nowACKN-firstACKN + i - chunkSize - packetlen,request->str);

          if(res == FAILED)
          {
            quit = 2;break;
          }

          pointer = request->str;//reset the cursor(pointer) to start point
        }
        if(tcp_read_next_rx_byte((BYTE*)pointer++) != 1)break;//read the byte
        Data_Actual_Length++;
      }
      //DUMP THE PACKET, one segmentation ended. be ready about next packet.
      tcp_dump_rx_packet();
      TCP_Count++;
      break;
    case RETRIES_EXPIRED:Debug("TCP: re-sending Packet.\n");Addstr_destroy(request);goto sending;//RE-SENDING Support.
    case DISCONNECTED:quit=1;break;
    }
  }
  if(quit == 2)
  {
    Addstr_destroy(request);
    printf("Error while writing FILE. maybe next time.\n");
    return FAILED;
  }
  //show residents
  int len = (int)pointer - (int)request->str;
  int lastpos = nowACKN-firstACKN - len;
  Debug("i=end,len=%d,addr=%05X\n",len,lastpos);
  getChunkCallback(2,len,lastpos,(uint8_t*)request->str);

  Addstr_destroy(request);
  //Debug("TCP: Host Disconnected.\n");
  //Debug("count=%d,Data_Length=%d.\n",TCP_Count,Data_Actual_Length);

  //end download.
  getChunkCallback(3,Data_Actual_Length,(int)NULL,NULL);

  return SUCCEED;//END.
}
Result getChunkCallback(int stat,int num,int addr,uint8_t* buf)
{
  if(stat==1)
  {
    printf("GETFILE -> FLASH %d start!!\n",0xA000);
    
    for(int i=0;i<((128*1024/4096));i++)
    {
      yield();
      if(MEM_EraseSector(0x0A0000 + i*4096) == FAILED)
        i--;//retry this step
    }
  }
  else if(stat==2)
  {
    //Debug("%s\n",buf);
    //CoderBuffer_print_nonpointer(CoderBuffer_make(buf,256),16);
    MEM_Program(addr + 0x0A0000,buf,num);

    if(MEM_Verify(addr + 0x0A0000,buf,num) == FAILED)
    {
      printf("Write Corrupt!! Verify failed.\n");
      if(addr == 0)
      {
        printf("retrying..\n");
        MEM_EraseSector(0x0A0000);
        MEM_Print(0x0A0000,1024);

        MEM_Program(0x0A0000,buf,num);
        
        if(MEM_Verify(0x0A0000,buf,num) == FAILED)
        {
          MEM_Print(0x0A0000,1024);
          memory_Print(buf,1024);
          printf(buf);
          printf("failed..\n");
          return FAILED;
        }
        printf("succeed..\n");
      }
      return FAILED;
    }
    return SUCCEED;
  }
  else if(stat==3)
  {
    printf("GETFILE -> FLASH ended!!\n");
    //MEM_Print(0x0A0000,num);
  }
  return SUCCEED;
}

Result dbuglog(char* log,BOOL isCritical)
{
  if(saveData.dbuglog == FALSE)return SUCCEED;
  if(log == NULL)return FAILED;
  // dbuglog=Y일때만 서버로 보낸다.
  // 1) 구조 : api/sio/{id}/{val}/{val2}
  //       /api/sio/Base64(로그내용)/aes_no/dbuglog
  //       -- {id}Base64(로그내용)
  //       -- {val}aes_no : idxEquip(0001)4자리 + 필수여부(0 또는 1) : 0=필수로그, 1=디버깅 로그
  //       -- {val2}dbuglog

  AttenderAPI_SendKey key = API_GenerateAESKey(nowDate.month,nowDate.day);

  Debug("key : %d,%d,%d\n",key.C,key.D,key.key);
  Debug("message : %s\n",log);
  CoderBuffer Encoded_log = Coder_Base64Encode(CoderBuffer_clone(log,strlen(log)));
  
  Addstr_Str *sendbuffer;
  Addstr_initlen(&sendbuffer,30);
  Addstr_add(sendbuffer,"/api/sio/");
  Addstr_addlen(sendbuffer,Encoded_log->buffer,Encoded_log->len);

  CoderBuffer_free(Encoded_log);

  char buffer[48] = {0,};
  sprintf(buffer,"/%04d%d/dbuglog",saveData.apcode,!isCritical);
  Addstr_add(sendbuffer,buffer);

  JSONBuffer Object = SmartIOS_API(Addstr_return(sendbuffer));
  Addstr_destroy(sendbuffer);

  if(Object == 0)
    return FAILED;

  int rtncode = atoi(json_getPropertyValue(Object->json,"rtncode"));

  printf("returncode:%d\n",rtncode);

  JSONBuffer_free(Object);
  if(rtncode == 1)
    return SUCCEED;
  else
    return FAILED;
}

//IP Change
Result API_apipc(IP_ADDR apip,IP_ADDR gwip,IP_ADDR netmsk,IP_ADDR dnsip,IP_ADDR dnsip2)
{
  // 1) 구조 : api/sio/{id}/{val}/{id2}
  // /api/sio/암호화(1^****)/aes_no/apipc
  // -- {id}암호화(idxEquip^IPAddress) : Base64(AES암호화.aes_no(1^APIPAddress_기본게이트웨이_서브넷마스크_DNS1_DNS2))
  // -- {val} aes_no : 임의값(1000~9999) + idxEquip
  // -- {id2}apipc
  uint8_t ips[20];
  memcpy(ips+0,apip.v,4);
  memcpy(ips+4,gwip.v,4);
  memcpy(ips+8,netmsk.v,4);
  memcpy(ips+12,dnsip.v,4);
  memcpy(ips+16,dnsip2.v,4);
  CoderBuffer Encoded_Base64_ips = Coder_Base64Encode(CoderBuffer_clone(ips,sizeof(ips)));

  Encoded_Base64_ips->buffer[Encoded_Base64_ips->len] = '\0'; //null termination

  AttenderAPI_SendKey key = API_GenerateAESKey(nowDate.month,nowDate.day);

  char buffer[64] = {0,};
  sprintf(buffer,"%d^%s",saveData.apcode,Encoded_Base64_ips->buffer);
  Debug("key : %d,%d,%d\n",key.C,key.D,key.key);
  Debug("buffer : %s\n",buffer);
  CoderBuffer Clientcode_N_idxEquip = Coder_Base64Encode(Coder_Base64Encode(Coder_AESEncode(CoderBuffer_clone(buffer,strlen(buffer)),key.key)));
  
  Addstr_Str *sendbuffer;
  Addstr_initlen(&sendbuffer,30);
  Addstr_add(sendbuffer,"/api/sio/");
  Addstr_addlen(sendbuffer,Clientcode_N_idxEquip->buffer,Clientcode_N_idxEquip->len);

  CoderBuffer_free(Clientcode_N_idxEquip);
  CoderBuffer_free(Encoded_Base64_ips);

  sprintf(buffer,"/%d%d/apipc",key.D,key.C);
  Addstr_add(sendbuffer,buffer);

  JSONBuffer Object = SmartIOS_API(Addstr_return(sendbuffer));
  Addstr_destroy(sendbuffer);

  if(Object == 0){
    printf("API_apipc: rtn JSON is NULL.\n");
    return FAILED;
  }

  int rtncode = atoi(json_getPropertyValue(Object->json,"rtncode"));

  printf("returncode:%d\n",rtncode);

  JSONBuffer_free(Object);
  if(rtncode == 1)
    return SUCCEED;
  else
    return FAILED;
  //리턴 패킷 : {"rtncode" : "01", "data":"웹 폴더/파일명.bin"}
}

Result API_Echo()
{
  Debug("Send_API_Echo\n");

  char sendbuffer[25] = {0,};
  int sendCode = rando()%9000 + 1000;
  sprintf(sendbuffer,"/api/sio/echo_%04de%d",sendCode,saveData.apcode);
  JSONBuffer Object = SmartIOS_API(sendbuffer);
  if(Object == 0){
    printf("API_Echo: rtn JSON is null\n");
    return FAILED;
  }

  char* str_rtncode = json_getPropertyValue(Object->json,"rtncode");

  if(strlen(str_rtncode) == 0){JSONBuffer_free(Object); return FAILED;}//idxEquip가 등록이 안 되어있음.(rtncode = "")

  printf("returncode:%s\n",str_rtncode);

  int  code;
  //                   2021 06 05 095010 . 5794
  sscanf(str_rtncode,"%4d%2d%2d%2d%2d%2d.%4d",&nowDate.year,&nowDate.month,&nowDate.day,&nowDate.hour,&nowDate.minute,&nowDate.second,&code);

  if(sendCode != code) { JSONBuffer_free(Object); return FAILED;}

  printf("rtn time : %s\n",Convert_Date2Str(nowDate));

  printf("API_Echo Succeed!\n");
  JSONBuffer_free(Object);
  
  return SUCCEED;
}

JSONBuffer Beacon_SendAPI(SensorData sensor)
{
  Debug("BeaconSendAPI start : %dms\n",HAL_GetTick() - sensor.tenSecBeginTime);
  // reference : https://docs.google.com/presentation/d/1Nmay7KBIvLIYq51I3xr2xXuu28jftsa8/edit#slide=id.ge53e02a9b7_0_2
  //F : 암호화키배열 : 암호화.Z에서 분리한 값
  int F = (int)((sensor.sensorDate.day + sensor.sensorDate.month)/2) + sensor.C - 1;
  
  AttenderAPI_SendKey key =  API_GenerateAESKey(0,0);
  // D = 임의값(1000~9999) = 4262
  int D = key.D;//rando()%9000 + 1000;
  // W= D + (C) = 42628
  int C = key.C;//rando()%9000 + 1000;
 // int W = D*10 + sensor.C%10;

  // E=D일경우 = F(24)+D의 맨뒷자1자(2) =242
  int E = key.key;//F*10 + D%10;//this is W

  // 비콘 데이터는 출입에 영향을 주는 중요한 요소이므로 날짜가 달라도 보낸다.

  char G[16];
  // G=S에서 F(24)를 이용하여 복호화(일자+해킹코드)로 구한다.
  sprintf(G,"%04d%02d%02d%04d" ,sensor.sensorDate.year,sensor.sensorDate.month,sensor.sensorDate.day ,sensor.HackingCode);

  // X=암호화키배열에서 E(242)번째 값을 이용하여
  //         “G(일자+해킹코드)”를 암호화한다.
  //X <= G(일자+해킹코드)[12]
  CoderBuffer XData = Coder_Base64Encode(Coder_Base64Encode(Coder_AESEncode(CoderBuffer_clone(G,strlen(G)),key.key)));
  if(XData == NULL)
  {
    CoderBuffer_free(XData);
    Debug("Xfailed\n");
    return NULL;
  }
  
  // A=암호화키배열에서 E(242)번째 값을 이용하여
  //     “내장 AP고유번호(idxEquip)^H(idxUser)”를 암호화
  // A <= idxEquip_User(idxEquip^idxUser)[15]
  char idxEquip_User[15] = {0,};
  sprintf(idxEquip_User,"%d^%d",saveData.apcode,sensor.idxUser);

  
  Debug("YYYYMMDDnnnn = %s\n",G);
  Debug("idxEquip^idxUser = %s\n",idxEquip_User);
  Debug("random = %d\n",D);
  Debug("Key = %d\n",E);


  CoderBuffer AData_1 = Coder_AESEncode(CoderBuffer_clone((uint8_t*)idxEquip_User,strlen((char*)idxEquip_User)),E);
  CoderBuffer AData = Coder_Base64Encode(Coder_Base64Encode(AData_1));
  if(AData == NULL)
  {
    CoderBuffer_free(XData);
    CoderBuffer_free(AData);
    Debug("Afailed\n");
    return NULL;
  }
  
  char Wdata[7];
  sprintf(Wdata,"%d%d",D,C);
  //Debug("prepare finished!\n");

  Debug("BeaconSendAPI prepare end : %dms\n",HAL_GetTick() - sensor.tenSecBeginTime);
  JSONBuffer res = API_Send_BEACON(XData,AData,Wdata);

  CoderBuffer_free(XData);
  CoderBuffer_free(AData);

  Debug(res!=NULL?"SUCCEED\n":"FAILED\n");

  return res;//res;
}
JSONBuffer API_Send_BEACON(CoderBuffer X,CoderBuffer Y,char* W)
{
  Debug("API_Send_BEACON\n");
  
  uint32_t start = HAL_GetTick();

  Addstr_Str *GETURL;
  Addstr_initlen(&GETURL,128);
  //CoderBuffer_print(X,16);
  //CoderBuffer_print(Y,16);
  //CoderBuffer_print(A,16);
  
  ///api/sio/암호화(yyyyMMdd0000)/암호화(1^11)/aes_no/hpio

  Addstr_add(GETURL,"/api/sio");
  Addstr_add(GETURL,"/");
  Addstr_addlen(GETURL,X->buffer,X->len);//yyyyMMdd0000
  Addstr_add(GETURL,"/");
  Addstr_addlen(GETURL,Y->buffer,Y->len);//1^11
  //Addstr_add(GETURL,"/");
  //Addstr_addlen(GETURL,A->buffer,A->len-1);
  Addstr_add(GETURL,"/");
  Addstr_add(GETURL,W);//aes_no
  Addstr_add(GETURL,"/hpio");
  Debug("Request URL : %s\n",GETURL->str);

  printf("API_Send_BEACON GET prep end: %dms\n",HAL_GetTick() - start);
  JSONBuffer Object = SmartIOS_API(GETURL->str);

  Addstr_destroy(GETURL);
  if(Object == NULL){Debug("API_Send_BEACON.SmartIOS_API null ended\n");return NULL;}
  Debug("API_Send_BEACON.SmartIOS_API ended\n");

  char* str_rtncode = json_getPropertyValue(Object->json,"rtncode");
  if(str_rtncode == NULL){
    Debug("API_Send_BEACON.SmartIOS_API rtncode null ended\n");
    JSONBuffer_free(Object);
    return NULL;
  }

  int rtncode = atoi(str_rtncode+1);

  Debug("returncode:%d\n",rtncode);
  
  if(rtncode == 91)
  {
    Debug("DOOR SIGNAL OPEN!!\n");
    extern uint32_t SIGNAL_24V_Timer;
    SIGNAL_24V_Timer = HAL_GetTick() - 1000;
  }
  // if(rtncode == 8)
  //   UpdateAesnoAsDefault();
  
  Debug("API_Send_BEACON getting result: %dms\n",HAL_GetTick() - start);

  if(rtncode == 1 || rtncode == 19 || rtncode == 91)
    return Object;
  else {
    Debug("API_Send_BEACON.SmartIOS_API rtncode error ended\n");
    JSONBuffer_free(Object);
    return NULL;
  }
}
// /api/sio/HttpUtility.UrlEncode(X값)
//          /HttpUtility.UrlEncode(Y값)
//          /HttpUtility.UrlEncode(A값)
//          /bb
//          /W값:1009+aes_no(8)

// 비콘 데이터 수신 형태
//
// Base64(AES.aes_no((yyyyMMdd 0001 010xxxxxxxx))
// YYMMDD 4byte, S 2byte , Telephone 6~9byte  : 12~15Byte
// 20 21 04 25    03 16            01 06 89 17 11 2F
//
// FF EF  14   7C 01 02 57 A1 39 11 98 59 9A 87 7E 72 C4 7C 6D BE
// |-- Z ----|--------------------- S ------------------------|--|
//
// F=암호화키배열(24) = 암호화.Z에서 분리된 값
// G=S에서 F(24)를 이용하여 복호화(일자+해킹코드) 구한다.
// (C)=일자와 F(24)을 이용하여 구한다. = 8
// D = 임의값(1000~9999) = 4262
// W= D + (C) = 42628
// H=S에서 F(24)를 이용하여 복호화(idxUser) 구한다.
// E=D일경우 = F(24)+D의 맨뒷자1자(2) =242
// A=암호화키배열에서 E(242)번째 값을 이용하여
//     “내장 AP고유번호(idxEquip)^H(idxUser)”를 암호화
// X=암호화키배열에서 E(242)번째 값을 이용하여
//         “G(일자+해킹코드)”를 암호화한다.

#include <math.h>
Result BLEList_ProcessSensor(BLEInfo *sensor,SensorData* p_decryptedSensor)
{

  SensorData decryptedSensor;

  // F=암호화키배열(24) = 암호화.Z에서 분리된 값
  int F = sensor->AESNo; //'code' becomes "F".  

  CoderBuffer preDecrypted = CoderBuffer_clone(sensor->AESData,16);
  CoderBuffer bufferDecrypted = Coder_AESDecode(preDecrypted,F);
  uint8_t* decrypted = bufferDecrypted->buffer; // setting output(actually, It has no meaning)

  


  if(bufferDecrypted == NULL || bufferDecrypted->len != 10 )
  {
    printf("Error Packet!\n");

    printf("IV:");
    CoderBuffer_print_nonpointer(CoderBuffer_make(addr_IV(F),16),16);
    printf("Key:");
    CoderBuffer_print_nonpointer(CoderBuffer_make(addr_Key(F),32),16);
    printf("preDecrypted:");
    CoderBuffer_print_nonpointer(CoderBuffer_make(sensor->AESData,16),16);
    printf("bufferDecrypted:");
    if(bufferDecrypted->len == 0)
      CoderBuffer_print(bufferDecrypted,16);
    CoderBuffer_free(bufferDecrypted);
    return FAILED;
  }
  decryptedSensor.sensorDate.year = ((decrypted[0]&0xF0)>>4)*1000 + (decrypted[0]&0x0F)*100 + ((decrypted[1]&0xF0)>>4)*10 + (decrypted[1]&0x0F);//2021
  int MMDD = ((decrypted[2]&0xF0)>>4)*1000 + (decrypted[2]&0x0F)*100 + ((decrypted[3]&0xF0)>>4)*10 + (decrypted[3]&0x0F);//0425
  decryptedSensor.HackingCode = decrypted[4]<<8 | decrypted[5];//0x0316

  // (C)=일자와 F(24)을 이용하여 구한다. = 8
  decryptedSensor.sensorDate.day   = ((decrypted[3]&0xF0)>>4)*10 + (decrypted[3]&0x0F);
  decryptedSensor.sensorDate.month = ((decrypted[2]&0xF0)>>4)*10 + (decrypted[2]&0x0F);

  //2021-11-16 patch. 비콘카드 날짜 에러 고침
  if(decryptedSensor.sensorDate.year == 0 && decryptedSensor.sensorDate.month == 0 && decryptedSensor.sensorDate.day == 0){
    //printf("BeaconCard Entered!(date=00000000)\n");
    decryptedSensor.sensorDate = nowDate;
  }
  //1. 하루가 지나 날짜가 달라지면 2. 달이 넘어가 x월 1일이면 3. 년이 넘어가 x년 1월 1일이면 해서 날짜를 업데이트한다.
  else if(decryptedSensor.sensorDate.year != nowDate.year || decryptedSensor.sensorDate.month != nowDate.month || decryptedSensor.sensorDate.day != nowDate.day){
    if(   decryptedSensor.sensorDate.day == nowDate.day + 1
       || decryptedSensor.sensorDate.month == nowDate.month + 1 && decryptedSensor.sensorDate.day == 1
       || decryptedSensor.sensorDate.year == nowDate.year + 1 && decryptedSensor.sensorDate.month == 1 && decryptedSensor.sensorDate.day == 1)
      API_Echo();//update the nowDate
  }
  
  //F = trunc(Day + Month)/2 + C - 1
  decryptedSensor.C = (F + 1) - (int)((decryptedSensor.sensorDate.day + decryptedSensor.sensorDate.month)/2);

  // H=S에서 F(24)를 이용하여 복호화(idxUser) 구한다.
  decryptedSensor.idxUser = decrypted[6]<<24 | decrypted[7]<<16 | decrypted[8]<<8 | decrypted[9];
  
  CoderBuffer_free(bufferDecrypted);
  
  //printf("%d,%d",(int8_t)sensor->txPower,(int8_t)sensor->rssi);
  int RSSIdifference = sensor->txPower-sensor->rssi+9;//+9dBm
  decryptedSensor.distance = powf(10,RSSIdifference/(2*10.f));

  if(p_decryptedSensor != NULL) *p_decryptedSensor = decryptedSensor;

  char sendbuf[10];
  sprintf(sendbuf,".%d",decryptedSensor.idxUser);
  Send_UART1(sendbuf,strlen(sendbuf));

  return SUCCEED;
}


//if error, returns 0.
uint8_t tcp_read_until_CRLF(uint8_t* array,int maxlen)
{
  memset(array,0,2);
  if(tcp_read_next_rx_byte(array++) != 1)return 0;
  while(tcp_read_next_rx_byte(array) == 1)
  {
    if(*(array-1) == '\r' && array[0] == '\n'){array[1] = 0;return 1;}
    array++;
    yield();
  }

  //error
  *(array+1) = 0;
  return 0;
}