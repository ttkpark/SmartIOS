#ifndef ATTENDERAPI_C
#define ATTENDERAPI_C
#endif

#include "Internet.h"
#include "AttenderAPI.h"
#include "Coder.h"
#include "memory.h"


extern Date nowDate;
extern SavedData saveData;

cJSON *AJUAttender_API(char* page)
{
  Addstr_Str *req; //fix 
  if(AJUAttender_Page(page,&req) == SUCCEED)
  {
    cJSON *Object;
    Object = cJSON_Parse(req->str);
    
    Addstr_destroy(req);
    return Object;
  }
  else return (void*)0;
}
Result AJUAttender_Page(char* page,Addstr_Str **reqptr)
{
  //Debug("DNS_Get!\n");
  if(AJUATTENDER_API_IP.Val == 0x00000000){
    //Update AJUATTENDER_API_IP.
    AJUATTENDER_API_IP = DNS_Get(AJUATTENDER_API_DOMAIN,3000);
    if(AJUATTENDER_API_IP.Val == 0x00000000)
      return FAILED;//ERROR
  }

  //Debug("421!\n");
  Addstr_Str* Require;
  Addstr_init(&Require);
  Addstr_add(Require,"GET ");
  Addstr_add(Require,page);
  Addstr_add(Require,"\r\n");
  
  Addstr_add(Require,"Host :");
  Addstr_add(Require,AJUATTENDER_API_DOMAIN);
  Addstr_add(Require,"\r\n");
  
  Addstr_add(Require,"Connection: close\r\n");
  Addstr_add(Require,"\r\n");
  char* require = Addstr_return(Require);


  //Debug("436!\n");

  Result ret = HTTPSendGET(AJUATTENDER_API_IP,AJUATTENDER_API_PORT,require,reqptr);
  
  Addstr_destroy(Require);
  if(ret != SUCCEED)
  {
    Addstr_destroy(*reqptr);
    return ret;
  }

  Addstr_replace(*reqptr,"\"[","");
  Addstr_replace(*reqptr,"]\"","");
  Addstr_replace(*reqptr,"\\\"","\"");

  //Debug("%s\n",request->str);
  //Debug("returns!\n");
  return ret;
}
Result HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str **reqptr)
{
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
    Debug("TCP connection %d Retries expired.\n",2);
    return FAILED;
  }
  
  sending:
  if (!tcp_setup_socket_tx(socket))
  {
    Debug("TCP: Can't tx right now - try again next time\n");
    return FAILED;//Can't tx right now - try again next time
  }

  //SEND THE PACKET
  tcp_write_array(require,strlen(require));
  tcp_socket_tx_packet(socket);



  //Waiting for response
  RES_GetWait res = TCP_Get_Wait(socket,TIMEOUT_FOR_WAIT_RESPONSE);

  switch (res){
  case TIMEOUT:
    tcp_close_socket(socket);
    Debug("TCP: TCP_Get_Wait TIMEOUT\n");
    return FAILED;

  case SUCCESS:
    ;
    Addstr_initlen(reqptr, TCP_Get_RemainingDataSize(socket)+1);
    Addstr_Str* request = *reqptr;
    //Debug("TCP_Get_Wait SUCCESS!\n");
    Debug("request bank = %d\n",request->bank);
    // request->size = TCP_Get_RemainingDataSize(socket);
    // request->str  = (char*)malloc(request->size+1);

    BYTE *pointer = request->str;
    while(tcp_read_next_rx_byte(pointer++) == 1);
    *pointer = 0;

    //Debug("len:%d\n",request->size);
    //Debug("str:%s\n",request->str);

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
  if(!TCP_Disconnect(socket,500))
    if(!TCP_Disconnect(socket,500))
      if(!TCP_Disconnect(socket,500)){
        Debug("TCP: try TCP_Disconnect failed.\n");
        //return FAILED;//TCP: DisConnect failed.
      }

  //Debug("TCP: disconnect succeed.\n");
  return SUCCEED;//END.
}

AttenderAPI_SendKey API_GenerateAESKey(int day,int month)
{
  AttenderAPI_SendKey key;
  key.C = rando()%10;//0~9
  key.D = (-rando())%9000 + 1000;//1000~9999

  key.key = ((day+month)/2 + key.C - 1)*10 + key.D%10;
  return key;
}

void process_with_IPCD(cJSON* object);

Result API_ipcd()
{
  Debug("[%dms] Send_API_ipcd\n",HAL_GetTick());

  AttenderAPI_SendKey key = API_GenerateAESKey(nowDate.month,nowDate.day);

  char buffer[20] = {0,};
  sprintf(buffer,"%d^%d",saveData.idxClient,saveData.apcode);
  Debug("object : %s\n",buffer);
  Debug("key : %d,%d,%d\n",key.C,key.D,key.key);
  CoderBuffer Clientcode_N_idxEquip = Coder_Base64Encode(Coder_Base64Encode(Coder_AESEncode(CoderBuffer_clone(buffer,strlen(buffer)),key.key)));
  
  Addstr_Str *sendbuffer;
  Addstr_initlen(&sendbuffer,30);
  Addstr_add(sendbuffer,"/api/sio/");
  Addstr_addlen(sendbuffer,Clientcode_N_idxEquip->buffer,Clientcode_N_idxEquip->len);

  CoderBuffer_free(Clientcode_N_idxEquip);

  sprintf(buffer,"/%d%d/ipcd",key.D,key.C);
  Addstr_add(sendbuffer,buffer);

  Debug("SendStr : %s\n",sendbuffer->str);


  cJSON *Object = AJUAttender_API(Addstr_return(sendbuffer));
  Addstr_destroy(sendbuffer);

  if(Object == 0)
    return FAILED;

  char* strBuffered = cJSON_PrintBuffered(Object,100,1);
  Debug("%s\n",strBuffered);
  free(strBuffered);

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");

  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  process_with_IPCD(Object);

  int rtncode = atoi(cJSON_GetStringValue(rtnCode));

  Debug("returncode:%d\n",rtncode);
  if(rtncode != 1 && rtncode != 16)
  {
    Debug("Failed.\n");
    cJSON_Delete(Object);
    return FAILED;
  }
  else
  {
    Debug("Succeed!\n");
    cJSON_Delete(Object);
    return SUCCEED;
  }
}
void process_with_IPCD(cJSON* object)
{
  nowDate = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(object,"dt")));//
  int ipcdinterval = (int)atoi(cJSON_GetStringValue(cJSON_GetObjectItem(object,"ipcdInterval")));//
  BOOL useYN = cJSON_GetStringValue(cJSON_GetObjectItem(object,"useyn"))[0] == 'Y';//
  BOOL debuglog = cJSON_GetStringValue(cJSON_GetObjectItem(object,"dbuglog"))[0] == 'Y';//
  float distance = atof(cJSON_GetStringValue(cJSON_GetObjectItem(object,"dist")));//
  char* ips_str = cJSON_GetStringValue(cJSON_GetObjectItem(object,"apip"));//APIPAddress_기본게이트웨이_서브넷마스크_DNS1_DNS2
  Date firmDate = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(object,"pgverdate")));
  char* pgver = cJSON_GetStringValue(cJSON_GetObjectItem(object,"pgver"));

  BOOL shouldUpdateMemory = FALSE;

  //if there's one thing different between savedData and nowDate
  if(Date2Int(nowDate) != Date2Int(saveData.date) || useYN != saveData.useyn || debuglog != saveData.dbuglog
    || distance != saveData.dist || ipcdinterval != saveData.ipcdInterval)
  {
    printf("nowDate : %s\n",Convert_Date2Str(nowDate));
    printf("useYN : %d\n",useYN);
    printf("debuglog : %d\n",debuglog);
    printf("distance : %g\n",distance);
    printf("ipcdinterval : %d\n",ipcdinterval);
    saveData.date = nowDate;
    saveData.useyn = useYN;
    saveData.dbuglog = debuglog;
    saveData.dist = distance;
    saveData.ipcdInterval = ipcdinterval;
    shouldUpdateMemory = TRUE;
  }

  CoderBuffer ips_buffer = Coder_Base64Decode(CoderBuffer_clone(ips_str,strlen(ips_str)));
  IP_ADDR *ips = ips_buffer->buffer;
  if(ips_buffer->len == 20)
  {
    printf("received IP\n");
    printf("apip : %s\n",Convert_IP2Str(ips[0]));
    printf("gwip : %s\n",Convert_IP2Str(ips[1]));
    printf("nmsk : %s\n",Convert_IP2Str(ips[2]));
    printf("dns1 : %s\n",Convert_IP2Str(ips[3]));
    printf("dns2 : %s\n",Convert_IP2Str(ips[4]));
    if(ips[0].Val != saveData.apip.Val || ips[1].Val != saveData.gwip.Val
    || ips[2].Val != saveData.netmask.Val || ips[3].Val != saveData.dnsip.Val || ips[4].Val != saveData.dns2ip.Val)
    {
      saveData.apip.Val = ips[0].Val;
      saveData.gwip.Val = ips[1].Val;
      saveData.netmask.Val  = ips[2].Val;
      saveData.dnsip.Val = ips[3].Val;
      saveData.dns2ip.Val = ips[4].Val;
      shouldUpdateMemory = TRUE;
    }
  }
  CoderBuffer_free(ips_buffer);

  if(shouldUpdateMemory)
  {
    printf("IPCD Memory Update!!\n");
    MEM_Store_SaveData();// store data.
  }

  //should we do Firware Update?!    
  if(strcmp(pgver,saveData.pgver) != 0 || Date2Int(saveData.pgverdate) < Date2Int(firmDate))
  {
    printf("WE SHOULD DO FIRMWARE UPDATE!!\n");
  }
}

Result API_Echo()
{
  Debug("Send_API_Echo\n");

  char sendbuffer[20] = {0,};
  int sendCode = rando()%9000 + 1000;
  sprintf(sendbuffer,"/api/sio/echo_%04d",sendCode);
  cJSON *Object = AJUAttender_API(sendbuffer);
  if(Object == 0)
    return FAILED;

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");

  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  char* str_rtncode = cJSON_GetStringValue(rtnCode);

  Debug("returncode:%s\n",str_rtncode);

  int  code;
  //                   2021 06 05 095010 . 5794
  sscanf(str_rtncode,"%4d%2d%2d%2d%2d%2d.%4d",&nowDate.year,&nowDate.month,&nowDate.day,&nowDate.hour,&nowDate.minute,&nowDate.second,&code);

  if(sendCode != code) return FAILED;

  Debug("Time update : %s\n",Convert_Date2Str(nowDate));

  Debug("Succeed!\n");
  cJSON_Delete(Object);
  
  return SUCCEED;
}
// /api/sio/HttpUtility.UrlEncode(X값)
//          /HttpUtility.UrlEncode(Y값)
//          /HttpUtility.UrlEncode(A값)
//          /bb
//          /W값:1009+aes_no(8)
Result API_Send_BEACON(CoderBuffer X,CoderBuffer Y,char* W)
{
  Debug("API_Send_BEACON\n");

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
  Debug("Request URL : %s",GETURL->str);

  cJSON *Object = AJUAttender_API(GETURL->str);

  Addstr_destroy(GETURL);

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");
  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  char* str_rtncode = cJSON_Print(Object);

  Debug("returncode:%s\n",str_rtncode);

  free(str_rtncode);

  Debug("Succeed!\n");
  cJSON_Delete(Object);
  
  return SUCCEED;
}

// Base64(AES.aes_no((yyyyMMdd 0001 010xxxxxxxx))
// YYMMDD 4byte, S 2byte , Telephone 6~9byte  : 12~15Byte
// 20 21 04 25    03 16            01 06 89 17 11 2F
// 
// FF EF  14   7C 01 02 57 A1 39 11 98 59 9A 87 7E 72 C4 7C 6D BE
// |-- Z ----|--------------------- S ------------------------|--|

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
    CoderBuffer_print(bufferDecrypted,16);
    return FAILED;
  }
  decryptedSensor.sensorDate.year = ((decrypted[0]&0xF0)>>4)*1000 + (decrypted[0]&0x0F)*100 + ((decrypted[1]&0xF0)>>4)*10 + (decrypted[1]&0x0F);//2021
  int MMDD = ((decrypted[2]&0xF0)>>4)*1000 + (decrypted[2]&0x0F)*100 + ((decrypted[3]&0xF0)>>4)*10 + (decrypted[3]&0x0F);//0425
  decryptedSensor.HackingCode = decrypted[4]<<8 | decrypted[5];//0x0316

  // (C)=일자와 F(24)을 이용하여 구한다. = 8
  decryptedSensor.sensorDate.day   = ((decrypted[3]&0xF0)>>4)*10 + (decrypted[3]&0x0F);
  decryptedSensor.sensorDate.month = ((decrypted[2]&0xF0)>>4)*10 + (decrypted[2]&0x0F);


  if(decryptedSensor.sensorDate.year != nowDate.year || decryptedSensor.sensorDate.month != nowDate.month || decryptedSensor.sensorDate.day != nowDate.day) // Date is different. this is hacking trial;
  {
    printf("Error Packet!\n");
    printf("Now : %04d.%02d.%02d != Data : %04d.%02d.%02d\n"
      ,nowDate.year,nowDate.month,nowDate.day,decryptedSensor.sensorDate.year,decryptedSensor.sensorDate.month,decryptedSensor.sensorDate.day);

    CoderBuffer_free(bufferDecrypted);
    return FAILED;
  }
  
  //F = trunc(Day + Month)/2 + C - 1
  decryptedSensor.C = (F + 1) - (int)((decryptedSensor.sensorDate.day + decryptedSensor.sensorDate.month)/2);

  // H=S에서 F(24)를 이용하여 복호화(idxUser) 구한다.
  decryptedSensor.idxUser = decrypted[6]<<24 | decrypted[7]<<16 | decrypted[8]<<8 | decrypted[9];
  
  CoderBuffer_free(bufferDecrypted);
  
  printf("%d,%d",(int8_t)sensor->txPower,(int8_t)sensor->rssi);
  int RSSIdifference = sensor->txPower-sensor->rssi+9;//+9dBm
  decryptedSensor.distance = powf(10,RSSIdifference/(2*10.f));

  if(p_decryptedSensor != NULL) *p_decryptedSensor = decryptedSensor;

  
  Debug(".%d",decryptedSensor.idxUser);

  return SUCCEED;
}

//.0,-43[ 20 21 04 25 03 71 01 06 89 17 11 2F 04 04 04 04 ]
//G : 202104250881
//H : 1
//24[ 00 04 00 20 81 0A 00 00 15 07 00 00 61 0A 00 00 1F 07 00 00 29 07 00 00 ]
//32[ 00 04 00 20 81 0A 00 00 15 07 00 00 61 0A 00 00 1F 07 00 00 29 07 00 00 33 07 00 00 00 00 00 00 ]


Result Beacon_SendAPI(SensorData sensor)
{

  int F = (int)((sensor.sensorDate.day + sensor.sensorDate.month)/2) + sensor.C - 1;
  
  // D = 임의값(1000~9999) = 4262
  int D = rando()%9000 + 1000;
  // W= D + (C) = 42628
  int W = D*10 + sensor.C%10;

  // E=D일경우 = F(24)+D의 맨뒷자1자(2) =242
  int E = F*10 + D%10;//this is W


  if(sensor.sensorDate.year != nowDate.year || sensor.sensorDate.month != nowDate.month || sensor.sensorDate.day != nowDate.day) // Date is different. this is hacking trial;
  {
    printf("Error Packet!\n");
    printf("Now : %04d.%02d.%02d != Data : %04d.%02d.%02d\n"
      ,nowDate.year,nowDate.month,nowDate.day ,sensor.sensorDate.year,sensor.sensorDate.month,sensor.sensorDate.day);
    return FAILED;
  }

  char G[16];
  
  // G=S에서 F(24)를 이용하여 복호화(일자+해킹코드) 구한다.
  // G Code decoding
  sprintf(G,"%04d%02d%02d%04d",sensor.sensorDate.year,sensor.sensorDate.month,sensor.sensorDate.day,sensor.HackingCode);

  // Debug("E = %d\n",E);
  // CoderBuffer_print_nonpointer(CoderBuffer_make(addr_IV(E),16),16);
  // CoderBuffer_print_nonpointer(CoderBuffer_make(addr_Key(E),32),16);

  // X=암호화키배열에서 E(242)번째 값을 이용하여
  //         “G(일자+해킹코드)”를 암호화한다.
  //Debug("325\n");
  //X <= G(일자+해킹코드)[12]
  CoderBuffer XData_1 = Coder_AESEncode(CoderBuffer_clone(G,strlen(G)),E);
  CoderBuffer XData = Coder_Base64Encode(Coder_Base64Encode(XData_1));
  //Debug("329\n");
  //CoderBuffer_print(XData,16);
  if(XData == NULL)
  {
    CoderBuffer_free(XData);
    printf("Xfailed\n");
    return FAILED;
  }
  
  // A=암호화키배열에서 E(242)번째 값을 이용하여
  //     “내장 AP고유번호(idxEquip)^H(idxUser)”를 암호화
  // A <= idxEquip_User(idxEquip^idxUser)[15]
  uint8_t idxEquip_User[15] = {0,};
  sprintf(idxEquip_User,"%d^%d",saveData.apcode,sensor.idxUser);
  CoderBuffer AData_1 = Coder_AESEncode(CoderBuffer_clone(idxEquip_User,strlen(idxEquip_User)),E);
  CoderBuffer AData = Coder_Base64Encode(Coder_Base64Encode(AData_1));
  if(AData == NULL)
  {
    CoderBuffer_free(XData);
    CoderBuffer_free(AData);
    printf("Afailed\n");
    return FAILED;
  }
  
  uint8_t Wdata[7];
  sprintf(Wdata,"%d",W);
  //Debug("prepare finished!\n");

  Result res = API_Send_BEACON(XData,AData,Wdata);

  CoderBuffer_free(XData);
  CoderBuffer_free(AData);

  return SUCCEED;//res;
}
/*

Result Beacon_SendAPI(char* G,uint32_t idxUser,int F,int D,int W)
{
  // E=D일경우 = F(24)+D의 맨뒷자1자(2) =242
  int E = F*10 + D%10;//this is W

  // Debug("E = %d\n",E);
  // CoderBuffer_print_nonpointer(CoderBuffer_make(addr_IV(E),16),16);
  // CoderBuffer_print_nonpointer(CoderBuffer_make(addr_Key(E),32),16);

  // X=암호화키배열에서 E(242)번째 값을 이용하여
  //         “G(일자+해킹코드)”를 암호화한다.
  //Debug("325\n");
  //X <= G(일자+해킹코드)[12]
  CoderBuffer XData_1 = Coder_AESEncode(CoderBuffer_clone(G,strlen(G)),E);
  CoderBuffer XData = Coder_Base64Encode(Coder_Base64Encode(XData_1));
  //Debug("329\n");
  //CoderBuffer_print(XData,16);
  if(XData == NULL)
  {
    CoderBuffer_free(XData);
    printf("Xfailed\n");
    return FAILED;
  }
  
  // A=암호화키배열에서 E(242)번째 값을 이용하여
  //     “내장 AP고유번호(idxEquip)^H(idxUser)”를 암호화
  // A <= idxEquip_User(idxEquip^idxUser)[15]
  uint8_t idxEquip_User[15] = {0,};
  sprintf(idxEquip_User,"%d^%d",saveData.apcode,idxUser);
  CoderBuffer AData_1 = Coder_AESEncode(CoderBuffer_clone(idxEquip_User,strlen(idxEquip_User)),E);
  CoderBuffer AData = Coder_Base64Encode(Coder_Base64Encode(AData_1));
  if(AData == NULL)
  {
    CoderBuffer_free(XData);
    CoderBuffer_free(AData);
    printf("Afailed\n");
    return FAILED;
  }
  
  uint8_t Wdata[7];
  sprintf(Wdata,"%d",W);
  //Debug("prepare finished!\n");

  Result res = API_Send_BEACON(XData,AData,Wdata);

  CoderBuffer_free(XData);
  CoderBuffer_free(AData);

  return SUCCEED;//res;
}
*/