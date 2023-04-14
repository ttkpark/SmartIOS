#ifndef ATTENDERAPI_C
#define ATTENDERAPI_C
#endif

#include "ETH/Internet.h"
#include "API/AttenderAPI.h"
#include "buffer/CoderBuffer.h"
#include "buffer/aes.h"
#include "buffer/base64.h"
#include "memory/memory.h"

//reserved word with #   #\w+
//file "((\w+(/|\\))*)\w+(\.\w+)+"
//all the hex value 0x[0-9A-F]*
//all the pointer [^,/ )(\n]\w+( |)([*]+)

extern Date nowDate;
extern SavedData saveData;
int IPCDResultCode = 1; // if this becomes 3, ipcd3 is to be sended. and 0, ipcd0 is to be sended. finally 1, It's normal state.


void CleanTCPSocket(SOCKET socket)
{
  extern BYTE tcp_tx_resp_after_proc_rx_resp_flags;

  Debug("TCP: socket flags\n");
  Debug("DO WE NEED TO SEND A RESPONSE : (%d > 0) & (%d > 0)\n" ,tcp_rx_packet_is_waiting_to_be_processed ,tcp_tx_resp_after_proc_rx_resp_flags);
  Debug("IS SOCKET READY TO TX         : (%d == 1) || (%d == 0)\n" ,tcp_socket[socket].flags.tx_last_tx_awaiting_ack ,tcp_socket[socket].flags.ready_for_tx);

  tcp_close_socket(socket);
}

Result SetAJUAttenderAPI(char* ServerURL)
{
  // http://www.eduwon.net:8807
  // http :// www.eduwon.net : 8807
  char* firstColon = strchr(ServerURL,':');
  if(firstColon == NULL)return FAILED;

  char* secondColon = strchr(firstColon+1,':');

  if(secondColon == NULL)return FAILED; // if "http://" is gone -> It's error.

  int domainlen = strlen(firstColon) - strlen(secondColon) - 3;
  memcpy(AJUATTENDER_API_DOMAIN_buff, firstColon+3, domainlen);
  AJUATTENDER_API_DOMAIN_buff[domainlen] = '\0'; //null termination

  AJUATTENDER_API_PORT = atoi(secondColon+1);
  printf("domain : %s:%d",AJUATTENDER_API_DOMAIN,AJUATTENDER_API_PORT);
  return SUCCEED;
}

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
  else
  {
    Addstr_destroy(req);
    return (void*)0;
  }
}
Result AJUAttender_Page(char* page,Addstr_Str **reqptr)
{
  //Debug("DNS_Get!\n");
  if(AJUATTENDER_API_IP.Val == 0x00000000){
    //Update AJUATTENDER_API_IP.
    AJUATTENDER_API_IP = DNS_Get(AJUATTENDER_API_DOMAIN,4000);
    if(AJUATTENDER_API_IP.Val == 0x00000000)
    HAL_Delay(300);
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
    Debug("request len = %d\n",TCP_Get_RemainingDataSize(socket));
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
  key.C = saveData.apcode;//rando()%10;//0~9
  key.D = (-rando())%9000 + 1000;//1000~9999

  key.key = saveData.aes_no;//((day+month)/2 + key.C - 1)*10 + key.D%10;
  return key;
}

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

  Debug("SendStr : %s\n",sendbuffer->str);


  cJSON *Object = AJUAttender_API(Addstr_return(sendbuffer));
  Addstr_destroy(sendbuffer);
  if(Object == 0)return FAILED;

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");
  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  int rtncode = atoi(cJSON_GetStringValue(rtnCode));

  Debug("returncode:%d\n",rtncode);
  
  if(rtncode == 8)
    UpdateAesnoAsDefault();
    
  if(rtncode != 1)
  {
    Debug("Failed.\n");
    cJSON_Delete(Object);
    return FAILED;
  }else{
    Debug("Succeed!\n");
    cJSON_Delete(Object);
    return SUCCEED;
  }
}

void process_with_IPCD(cJSON* object,int AESKey);

Result API_ipcd()
{
  Debug("[%dms] Send_API_ipcd\n",HAL_GetTick());

  AttenderAPI_SendKey key = API_GenerateAESKey(nowDate.month,nowDate.day);

  char buffer[20] = {0,};
  sprintf(buffer,"%d^%g",saveData.apcode,23.5);
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

  int rtncode = atoi(cJSON_GetStringValue(rtnCode));

  Debug("returncode:%d\n",rtncode);
  if(rtncode == 8)
    UpdateAesnoAsDefault();

  if(rtncode != 1 && rtncode != 16)
  {
    Debug("Failed.\n");
    cJSON_Delete(Object);
    return FAILED;
  }
  else
  {
    Debug("Succeed!\n");
    
    process_with_IPCD(Object,key.key);

    cJSON_Delete(Object);
    return SUCCEED;
  }
}
void process_with_IPCD(cJSON* object,int AESKey)
{
  char* nowdt = cJSON_GetStringValue(cJSON_GetObjectItem(object,"nowdt"));
  nowDate = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(object,"dt")));//
  int ipcdinterval = (int)atoi(cJSON_GetStringValue(cJSON_GetObjectItem(object,"ipcdInterval")));//
  BOOL useYN = cJSON_GetStringValue(cJSON_GetObjectItem(object,"useyn"))[0] == 'Y';//
  BOOL debuglog = cJSON_GetStringValue(cJSON_GetObjectItem(object,"dbuglog"))[0] == 'Y';//
  float distance = atof(cJSON_GetStringValue(cJSON_GetObjectItem(object,"dist")));//
  char* ips_str = cJSON_GetStringValue(cJSON_GetObjectItem(object,"apip"));//APIPAddress_기본게이트웨이_서브넷마스크_DNS1_DNS2
  Date firmDate = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(object,"v_pgverdate")));
  char* pgver = cJSON_GetStringValue(cJSON_GetObjectItem(object,"v_pgver"));

  volatile BOOL shouldUpdateMemory = FALSE;
  volatile BOOL shouldResetInternet = FALSE;
  
  SaveDataUpdate updateData;
  memset(&updateData,0,sizeof(SaveDataUpdate));

  if(ipcdinterval <= 0 || ipcdinterval > 1000000000)return;

  //1.Firmware Check
  printf("saveData.pgverdate : %s %d\n",Convert_Date2Str(saveData.pgverdate),Date2Int(saveData.pgverdate));
  printf("firmDate           : %s %d\n",Convert_Date2Str(firmDate),Date2Int(firmDate));
  printf("saveData.pgver pgver : \"%s\" <> \"%s\" %d\n",saveData.pgver,pgver,strcmp(pgver,saveData.pgver));
  //should we do Firware Update?!    
  if(Date2Int(saveData.pgverdate) < Date2Int(firmDate))
  {
    printf("WE SHOULD DO FIRMWARE UPDATE!!\n");
    
    start_FirmUpdate(pgver);
  }


  //2. General Settings Update

  char* focus; int aes_no = -1;
  if((focus = strstr(nowdt,".")) != NULL)
  {   
    aes_no = atoi(focus+1);
    printf("aesno is %d.\n",aes_no);
  }else printf("aesno is NULL!!\n");
  

  if(nowDate.year != 0)printf("nowDate : %s\n",Convert_Date2Str(nowDate));else printf("nowDate : %s no change\n",Convert_Date2Str(saveData.date));
  if(distance > 0.1f)printf("distance : %g\n",distance);else printf("distance : %g no change\n",saveData.dist);
  printf("useYN : %d\n",useYN);
  printf("debuglog : %d\n",debuglog);
  printf("ipcdinterval : %d\n",ipcdinterval);

  if(nowDate.year != 0)updateData.date = &nowDate;
  if(distance > 0.1f)updateData.dist = distance;
  updateData.useyn = useYN;
  updateData.dbuglog = debuglog;
  updateData.ipcdInterval = ipcdinterval;
  if(aes_no != -1)updateData.aes_no = &aes_no;


  //3. IP Settings Update
  CoderBuffer ips_buffer_transit = Coder_AESDecode(Coder_Base64Decode(Coder_Base64Decode(CoderBuffer_clone(ips_str,strlen(ips_str)))),AESKey);
  CoderBuffer ips_buffer = Coder_Base64Decode(ips_buffer_transit);
  IP_ADDR *ips = ips_buffer->buffer;
  printf("[%d]",ips_buffer->len);
  if(ips_buffer->len == 20){
    printf("received IP\n");
    if(ips[0].Val != 0x00000000)printf("apip : %s\n",Convert_IP2Str(ips[0])); else printf("apip : %s no change\n",Convert_IP2Str(saveData.apip));
    if(ips[1].Val != 0x00000000)printf("gwip : %s\n",Convert_IP2Str(ips[1])); else printf("gwip : %s no change\n",Convert_IP2Str(saveData.gwip));
    if(ips[2].Val != 0x00000000)printf("nmsk : %s\n",Convert_IP2Str(ips[2])); else printf("nmsk : %s no change\n",Convert_IP2Str(saveData.netmask));
    if(ips[3].Val != 0x00000000)printf("dns1 : %s\n",Convert_IP2Str(ips[3])); else printf("dns1 : %s no change\n",Convert_IP2Str(saveData.dnsip));
    if(ips[4].Val != 0x00000000)printf("dns2 : %s\n",Convert_IP2Str(ips[4])); else printf("dns2 : %s no change\n",Convert_IP2Str(saveData.dns2ip));
    if( (ips[0].Val != saveData.apip.Val && ips[0].Val != 0)
     || (ips[1].Val != saveData.gwip.Val && ips[1].Val != 0)
     || (ips[2].Val != saveData.netmask.Val && ips[2].Val != 0)
     || (ips[3].Val != saveData.dnsip.Val && ips[3].Val != 0)
     || (ips[4].Val != saveData.dns2ip.Val && ips[4].Val != 0))
    {
      shouldResetInternet = TRUE;
    }
  }
  CoderBuffer_free(ips_buffer);

  if(shouldResetInternet)
  {
    if(TryingReconnectInternet(ips[0],ips[1],ips[2],ips[3],ips[4]) == SUCCEED)
    {
      Start_DebugUDP();
      IPCDResultCode = 3;

      updateData.apip = &ips[0];
      updateData.gwip = &ips[1];
      updateData.netmask  = &ips[2];
      updateData.dnsip = &ips[3];
      updateData.dns2ip = &ips[4];
      saveData.isDHCP = FALSE;
    }
    else IPCDResultCode = 0;
  }

  printf("IPCD Memory Update!!\n");
  MEM_ModifyMemory(&updateData);
}


//1  . 받은 IP로 수동 연결 해본다.
//2-1. 실패하면 원래 세팅으로 원상복귀 -> 실패 시 플래그를 켜놓고 대기. 바로라도 나중에라도 연결되면 반드시 ipc0을 보냄. 
//2-2. 성공하면 ipc3을 보냄.
extern int isOnline;
extern MAC_ADDR myMAC;
Result TryingReconnectInternet(IP_ADDR apip,IP_ADDR gwip,IP_ADDR netmsk,IP_ADDR dnsip,IP_ADDR dnsip2)
{
  printf("Trying Reconnect Internet via %s..\n",Convert_IP2Str(apip));
  IPCDResultCode = 0;//make default Failed state.

  Result echo_res = SUCCEED;

  End_DebugUDP();

  resetNetwork();//make offline

  uint32_t netResetTimeout = HAL_GetTick();//timeout 30s

  EC_Init(apip,netmsk,gwip,myMAC,FALSE);
  
  checkConnectivity(TRUE);

  //
  while(!(nic_linked_and_ip_address_valid && isOnline) && (netResetTimeout - HAL_GetTick() < 20000)); //go out if connected.

  if(netResetTimeout - HAL_GetTick() >= 20000)
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

void CopyFirmwareAndBackup();
// It is callbacked on every received "getFile's chunkSize" bytes.
Result getChunkCallback(int stat,int num,int addr,uint8_t* buf);

void start_FirmUpdate(char* pgver)
{
  Addstr_Str *str;
  Addstr_init(&str);
  sprintf(str->str,FILEGETFORMAT,pgver);
  IP_ADDR FirmUpdateIP = DNS_Get(FILEGETHost,1000);

  Addstr_Str* Require;
  Addstr_init(&Require);
  Addstr_add(Require,"GET ");
  Addstr_add(Require,str->str);
  Addstr_add(Require," HTTP/1.1\r\n");
  Addstr_add(Require,"Host :"FILEGETHost"\r\n");
  Addstr_add(Require,"Connection: close\r\n");
  Addstr_add(Require,"\r\n");
  char* require = Addstr_return(Require);
  printf(require);
  
  Addstr_destroy(str);

  if(FirmUpdateIP.Val != 0)
  {
    Result res = GETFile(FirmUpdateIP,FILEGETPort,require,NULL,1024,getChunkCallback);
    
    printf("GETFILE %s",res==SUCCEED?"SUCCEED\n":"FAILED\n");
    if(res==SUCCEED)CopyFirmwareAndBackup();
  }
  else printf("error finding IP of update server.\n");
  Addstr_destroy(Require);  
}
void  memory_Print(uint8_t *ptr,uint32_t len)
{
    uint8_t* origin = ptr;
    extern IWDG_HandleTypeDef hiwdg;
    for(;len>0;)
    {
      int readlen = len>16 ? 16 : len;
      HAL_Delay(1);
      HAL_IWDG_Refresh(&hiwdg);

      printf("%05X : ",ptr - origin);
      for(int j=0;j<readlen;j++)
          printf("%02X ",ptr[j]);
      printf("\n");

      ptr += readlen;
      len -= readlen;
    }
}
//0C0000h~0DFFFFh
Result getChunkCallback(int stat,int num,int addr,uint8_t* buf)
{
  static uint32_t contentlen;
  if(stat==1)
  {
    Debug("Write start!! ContentLen=%d\n",num);
    contentlen = num;
    
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
    Debug("Write ended!! written=%d\n",num);
    //MEM_Print(0x0A0000,num);
  }
  return SUCCEED;
}

void CopyFirmwareAndBackup()
{
  Debug("CopyFirmwareAndBackup start\n");
  uint8_t buf[512] = {0,};
  MEM_NormalRead(0xA0000,buf,511);
  buf[511] = 0;//null termination
  Debug("buf = %s\n",buf);

  int Content_length = 0;
  int Response = 0;

  volatile char HTTP_Array[48];
  char *focus,*focusCRLF,*bufpointer = buf;
  
  //parse HTTP Response Header.
  if((focus=strstr(bufpointer,"Content-Length:")) != 0)
    Content_length = atoi(focus + 15);
  
  if((focus=strstr(bufpointer,"HTTP/1.1 ")) != 0)
    Response = atoi(focus + 9);

  focus = strstr(bufpointer,"\r\n\r\n");

  Debug("Response = %d\n",Response);
  Debug("Content-Length = %d\n",Content_length);
  int HeaderLength = (uint32_t)((char*)focus - (char*)buf) + 4;
  int start_addr = 0xA0000 + HeaderLength;
  Debug("HeaderLength = %d\n",HeaderLength);

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

  Debug("New Firmware Write start.\n");
  Debug("erasing %05X -> %05X\n",0xA0000+HeaderLength,0xC0000);
  // Ready of erasing to place where will be written.(0xC0000~0xDFFFF)
  for(int i=0;i<((Content_length-1)/4096)+1;i++)
  {
    MEM_EraseSector(0xC0000 + i*4096);
    yield();
  }

  Debug("writing %05X -> %05X\n",0xA0000+HeaderLength,0xC0000);
  //Fetch and write.
  for(int i=0;i<((Content_length-1)/256)+1;i++)
  {
    Result res = SUCCEED;
    res |= MEM_NormalRead(0xA0000+HeaderLength+(i<<8),buf,256);

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

    res |= MEM_PageProgram(0xC0000+(i<<8),buf,256);
    if(res){i--;Debug("Fail.retrying..%05X\n",0xC0000+(i<<8));}
    yield();
  }
  Debug("New Firmware Write end.\n");
  
  MEM_Print(0x0C0000,256);

  Debug("Backup Firmware Write start.\n");
  Debug("erasing %08X -> %05X\n",0x08000000,0xE0000);
  for(int i=0;i<((0x1D000-1)/4096)+1;i++){
    MEM_EraseSector(0xE0000 + i*4096);
    yield();
  }
  Debug("writing %08X -> %05X\n",0x08003000,0xE0000);
  //Fetch and backup.
  for(int i=0;i<((0x1D000-1)/256)+1;i++)
  {
    Result res;
    res = MEM_PageProgram(0xE0000+(i<<8),0x08003000 + (i<<8),256);
    if(res)i--;
    yield();
  }
  Debug("Backup Firmware Write end.\n");

  Debug("Now Jump to Programmer. RESET\n");
  Debug("N0x0C0000. RESET\n");
  Jump_to_Programmer();
}
void Jump_to_Programmer()
{
  HAL_NVIC_SystemReset();
}


Result API_Echo()
{
  Debug("Send_API_Echo\n");

  char sendbuffer[25] = {0,};
  int sendCode = rando()%9000 + 1000;
  sprintf(sendbuffer,"/api/sio/echo_%04de%d",sendCode,saveData.apcode);
  cJSON *Object = AJUAttender_API(sendbuffer);
  if(Object == 0)
    return FAILED;

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");

  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  char* str_rtncode = cJSON_GetStringValue(rtnCode);

  if(strlen(str_rtncode) == 0)return SUCCEED;//idxEquip가 등록이 안 되어있음.(rtncode = "")

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
  Debug("Request URL : %s\n",GETURL->str);

  cJSON *Object = AJUAttender_API(GETURL->str);

  Addstr_destroy(GETURL);

  if(Object == 0){
    return -1;
  }

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");
  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  char* str_rtncode = cJSON_Print(rtnCode);

  int rtncode = atoi(str_rtncode+1);
  free(str_rtncode);

  Debug("returncode:%d\n",rtncode);

  cJSON_Delete(Object);
  
  if(rtncode == 91)
  {
    Debug("SIGNAL OPEN!!\n");
    extern uint32_t SIGNAL_24V_Timer;
    SIGNAL_24V_Timer = HAL_GetTick() - 1000;
  }
  if(rtncode == 8)
    UpdateAesnoAsDefault();

  if(rtncode == 1 || rtncode == 19 || rtncode == 91)
    return SUCCEED;
  else return FAILED;
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


  if(decryptedSensor.sensorDate.year != nowDate.year || decryptedSensor.sensorDate.month != nowDate.month || decryptedSensor.sensorDate.day != nowDate.day) // Date is different. this is hacking trial;
  {
    //error packet
    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,TRUE);HAL_Delay(62);
    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,FALSE);HAL_Delay(63);
    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,TRUE);HAL_Delay(62);
    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,FALSE);HAL_Delay(63);
    // printf("Error Packet!\n");
    // printf("Now : %04d.%02d.%02d != Data : %04d.%02d.%02d\n"
    //   ,nowDate.year,nowDate.month,nowDate.day,decryptedSensor.sensorDate.year,decryptedSensor.sensorDate.month,decryptedSensor.sensorDate.day);

    CoderBuffer_free(bufferDecrypted);
    return FAILED;
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
  extern UART_HandleTypeDef huart2;
  HAL_UART_Transmit_IT(&huart2,sendbuf,strlen(sendbuf));

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
  
  AttenderAPI_SendKey key =  API_GenerateAESKey(0,0);
  // D = 임의값(1000~9999) = 4262
  int D = key.D;//rando()%9000 + 1000;
  // W= D + (C) = 42628
  int C = key.C;//rando()%9000 + 1000;
 // int W = D*10 + sensor.C%10;

  // E=D일경우 = F(24)+D의 맨뒷자1자(2) =242
  int E = key.key;//F*10 + D%10;//this is W

  // 비콘 데이터는 출입에 영향을 주는 중요한 요소이므로 날짜가 달라도 보낸다.
  // if(sensor.sensorDate.year != nowDate.year || sensor.sensorDate.month != nowDate.month || sensor.sensorDate.day != nowDate.day) // Date is different. this is hacking trial;
  // {
  //   printf("Error Packet!\n");
  //   printf("Now : %04d.%02d.%02d != Data : %04d.%02d.%02d\n"
  //     ,nowDate.year,nowDate.month,nowDate.day ,sensor.sensorDate.year,sensor.sensorDate.month,sensor.sensorDate.day);
  //   return FAILED;
  // }

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
    printf("Afailed\n");
    return FAILED;
  }
  
  char Wdata[7];
  sprintf(Wdata,"%d%d",D,C);
  //Debug("prepare finished!\n");

  Result res = API_Send_BEACON(XData,AData,Wdata);

  CoderBuffer_free(XData);
  CoderBuffer_free(AData);

  Debug(res==SUCCEED?"SUCCEED\n":"FAILED\n");

  return res;//res;
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

// http://www.eduwon.net:20133/downloads/patch/v.kr.0.101/smartios.pmg

//getChunkCallback : stat==1 : begin() stat==2 : callback(num=len,addr=Addr,buf=Buffer) stat==3:end(num=wholedatalen)
Result GETFile(IP_ADDR ip,int port,char* require,int* filelen,int chunkSize,Result (*getChunkCallback)(int stat,int num,int addr,uint8_t* buf))
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
    Debug("TCP connection %d Retries expired.\n",2);
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
            Debug("Position is NOT Accurate! nowpos=%ld or %ld\n",Data_Actual_Length-chunkSize,nowACKN-firstACKN + i - chunkSize - packetlen);
          
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
    printf("Error while writing. maybe next time.\n");
    return FAILED;
  }
  //show residents
  int len = (int)pointer - (int)request->str;
  int lastpos = nowACKN-firstACKN - len;
  Debug("i=end,len=%d,addr=%05X\n",len,lastpos);
  getChunkCallback(2,len,lastpos,(uint8_t*)request->str);

  Addstr_destroy(request);
  Debug("TCP: Host Disconnected.\n");
  Debug("count=%d,Data_Length=%d.\n",TCP_Count,Data_Actual_Length);

  //end download.
  getChunkCallback(3,Data_Actual_Length,(int)NULL,NULL);

  return SUCCEED;//END.
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
  }

  //error
  *(array+1) = 0;
  return 0;
}