#ifndef ATTENDERAPI_C
#define ATTENDERAPI_C
#endif

#include "Internet.h"
#include "AttenderAPI.h"
#include "Coder.h"


cJSON *AJUAttender_API(char* page)
{
  Addstr_Str req; req.size = 0; //fix 
  if(AJUAttender_Page(page,&req) == SUCCEED)
  {
    cJSON *Object;
    Object = cJSON_Parse(req.str);
    
    //Debug("1:%s\n",cJSON_Print(Object));
    Addstr_destroy(&req);
    //Debug("2:%s\n",cJSON_Print(Object));
    return Object;
  }
  else return (void*)0;
}
Result AJUAttender_Page(char* page,Addstr_Str *request)
{
  Debug("DNS_Get!\n");
  if(AJUATTENDER_API_IP.Val == 0x00000000){
    //Update AJUATTENDER_API_IP.
    AJUATTENDER_API_IP = DNS_Get(AJUATTENDER_API_DOMAIN,3000);
    if(AJUATTENDER_API_IP.Val == 0x00000000)
      return FAILED;//ERROR
  }

  Debug("421!\n");
  Addstr_Str Req; Req.size = 0;
  Addstr_init(&Req);
  Addstr_add(&Req,"GET ");
  Addstr_add(&Req,page);
  Addstr_add(&Req,"\r\n");
  
  Addstr_add(&Req,"Host :");
  Addstr_add(&Req,AJUATTENDER_API_DOMAIN);
  Addstr_add(&Req,"\r\n");
  
  Addstr_add(&Req,"Connection: close\r\n");
  Addstr_add(&Req,"\r\n");
  char* require = Addstr_return(&Req);


  Debug("436!\n");
  Addstr_init(request);

  Result ret = HTTPSendGET(AJUATTENDER_API_IP,AJUATTENDER_API_PORT,require,request);
  if(ret != SUCCEED)return ret;

  Addstr_destroy(&Req);
  Addstr_replace(request,"\\\"","\"");
  Addstr_replace(request,"\"[","");
  Addstr_replace(request,"]\"","");

  //Debug("%s\n",request->str);
  //Debug("returns!\n");
  return ret;
}
Result HTTPSendGET(IP_ADDR ip,int port,char* require,Addstr_Str *request)
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
  
  if (!tcp_setup_socket_tx(socket))
    return FAILED;//Can't tx right now - try again next time

  //SEND THE PACKET
  tcp_write_array(require,strlen(require));
  tcp_socket_tx_packet(socket);

  //Waiting for response
  RES_GetWait res = TCP_Get_Wait(socket,TIMEOUT_FOR_WAIT_RESPONSE);

  switch (res){
  case TIMEOUT:
    tcp_close_socket(socket);
    return FAILED;

  case SUCCESS:
    request->size = TCP_Get_RemainingDataSize(socket);
    request->str  = (char*)malloc(request->size+1);

    BYTE *pointer = request->str;
    while(tcp_read_next_rx_byte(pointer++) == 1);
    *pointer = 0;

    Debug("len:%d\n",request->size);
    Debug("str:%s\n",request->str);

    //DUMP THE PACKET
    tcp_dump_rx_packet();
    break;
  case RETRIES_EXPIRED:
    Debug("TCP: re-sending Packet.\n");
    //RE-SEND LAST PACKET TRANSMITTED
    //(TCP requires resending of packets if they are not acknowledged and to
    //avoid requiring a large RAM buffer the application needs to remember
    //the last packet sent on a socket so it can be resent if requried).
    return FAILED;//we will not support re-sending.
  case DISCONNECTED:
    Debug("TCP: Host Disconnected.\n");
    return FAILED;
  }

  //Debug("TCP: try TCP_Disconnect.\n");
  if(!TCP_Disconnect(socket,500))
    if(!TCP_Disconnect(socket,500))
      if(!TCP_Disconnect(socket,500)){
        //Debug("TCP: try TCP_Disconnect failed.\n");
        return FAILED;//TCP: DisConnect failed.
      }

  //Debug("TCP: disconnect succeed.\n");
  return SUCCEED;//END.
}

Result Addstr_replace(Addstr_Str *inout,char* Remove,char* Replace)
{
    // inout -> in -> result(Replaced) -> out(length optimized) -> inout
    Addstr_Str *result = malloc(sizeof(Addstr_Str));

    result->size = strlen(inout->str)*2;
    result->str = malloc(result->size);
    // make buffer variables spacing for two times of input string.

    char* ptr_result = result->str;
    char* ptr_in = inout->str;
    char* ptr_endofin = inout->str + strlen(inout->str);
    for(;;)
    {
        //  012345/"89/"2345
        // in=0 ,find=6   -> copy 0 to 5  (find - in) -> copy Replace to result -> in=find+len=8
        // in=8 ,find=10  -> copy 8 to 10 (find - in) -> copy Replace to result -> in=find+len=12
        // in=12,find=NULL-> copy 12 to 15(strlen - in) 

        //seek the String
        char* ptr_find = strstr(ptr_in,Remove);
        // if found
        if(ptr_find!=NULL)
        {
            //copy from now.
            int bytes_to_copy = ptr_find - ptr_in; //copy (find-in) characters
            memcpy(ptr_result,ptr_in,bytes_to_copy); // copy in 0 to 5
            ptr_result += bytes_to_copy;
            memcpy(ptr_result,Replace,strlen(Replace)); //in result.
            ptr_result += strlen(Replace);

            ptr_in = ptr_find + strlen(Remove);
        }
        else // if end
        {
            int bytes_to_copy = ptr_endofin - ptr_in; //copy (strlen - in) characters
            memcpy(ptr_result,ptr_in,bytes_to_copy); // copy in 12 to 15
            ptr_result += bytes_to_copy;
            *ptr_result = 0; // null termination
            ptr_result += 1;
            break;
        }
    }
    int after_len = strlen(result->str);
    if(after_len > result->size) 
        Debug("replace Warning!! see Addstr_replace.\n");

    free(inout->str);
    inout->size = after_len+1;
    inout->str = malloc(after_len+1); //null termination.
    memcpy(inout->str,result->str,strlen(result->str));// copy buffer to inout

    inout->str[strlen(result->str)] = 0; //null termination.

    free(result->str);
    free(result);
    
    return SUCCEED;
}

Result API_Echo()
{
  Debug("Send_API_Echo\n");
  cJSON *Object = AJUAttender_API("/api/att/echo_8754");
  if(Object == 0)
    return FAILED;

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");

  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  char* str_rtncode = cJSON_GetStringValue(rtnCode);

  Debug("returncode:%s\n",str_rtncode);

  Debug("Succeed!\n");
  cJSON_Delete(Object);
  
  return SUCCEED;
}
// /api/att/HttpUtility.UrlEncode(X값)
//          /HttpUtility.UrlEncode(Y값)
//          /HttpUtility.UrlEncode(A값)
//          /bb
//          /W값:1009+aes_no(8)
Result API_Send_BEACON(CoderBuffer* X,CoderBuffer* Y,CoderBuffer* A,char* W)
{
  Debug("API_Send_BEACON\n");

  Addstr_Str GETURL;
  Addstr_initlen(&GETURL,512);
  CoderBuffer_print(X,16);
  CoderBuffer_print(Y,16);
  CoderBuffer_print(A,16);
  
  Addstr_add(&GETURL,"/api/att");
  Addstr_add(&GETURL,"/");
  Addstr_addlen(&GETURL,X->buffer,X->len-1);
  Addstr_add(&GETURL,"/");
  Addstr_addlen(&GETURL,Y->buffer,Y->len-1);
  Addstr_add(&GETURL,"/");
  Addstr_addlen(&GETURL,A->buffer,A->len-1);
  Addstr_add(&GETURL,"/bb");
  Addstr_add(&GETURL,"/");
  Addstr_add(&GETURL,W);
  Debug("Request URL : %s",GETURL.str);

  cJSON *Object = AJUAttender_API(GETURL.str);

  Addstr_destroy(&GETURL);

  cJSON* rtnCode = cJSON_GetObjectItem(Object,"rtncode");
  if(rtnCode == 0){
    cJSON_Delete(Object);
    return FAILED;
  }

  char* str_rtncode = cJSON_GetStringValue(rtnCode);

  Debug("returncode:%s\n",str_rtncode);

  Debug("Succeed!\n");
  cJSON_Delete(Object);
  
  return SUCCEED;
}

Result BLEList_ProcessSensor(BLEInfo *sensor)
{
  //Base64(AES.aes_no((yyyyMMdd 0001 010xxxxxxxx))
  //YYMMDD 4byte, S 2byte , Telephone 6~9byte  : 12~15Byte
  //20 21 04 25    03 16            01 06 89 17 11 2F
  //
  //FF EF  14   7C 01 02 57 A1 39 11 98 59 9A 87 7E 72 C4 7C 6D BE
  //|-- Z ----|--------------------- S ------------------------|--|

  int code = sensor->AESNo-1; //'code' becomes "F".
  int D = HAL_GetTick()%10; // D means 'random between 0~9'

  CoderBuffer bufferTobeDecrypted = CoderBuffer_clone(sensor->AESData,16);
  CoderBuffer *bufferDecrypted = Coder_AESDecode(&bufferTobeDecrypted,code);
  uint8_t* decrypted = bufferDecrypted->buffer; // setting output(actually, It has no meaning)

  //print_binary(decrypted,16);//decrypt

  //debug log from here
  //.1,-52[ 20 21 04 25 03 16 00 00 00 01 04 04 04 04 ]

  //HackingCode decoding
  char HackingBCode[16];
  int YYYY = ((decrypted[0]&0xF0)>>4)*1000 + (decrypted[0]&0x0F)*100 + ((decrypted[1]&0xF0)>>4)*10 + (decrypted[1]&0x0F);//2021
  int MMDD = ((decrypted[2]&0xF0)>>4)*1000 + (decrypted[2]&0x0F)*100 + ((decrypted[3]&0xF0)>>4)*10 + (decrypted[3]&0x0F);//0425
  int Hacking = decrypted[4]<<8 | decrypted[5];//0x0316
  sprintf(HackingBCode,"%04d%04d%04d",YYYY,MMDD,Hacking);

  uint32_t idxUser = decrypted[7]<<24 | decrypted[8]<<16 | decrypted[9]<<8 | decrypted[10];
  
  Debug("G : %s\n",HackingBCode);
  Debug("H : %d\n",idxUser);
  //NRF_LOG_RAW_INFO("G : %s\n",NRF_LOG_PUSH(HackingBCode));
  //NRF_LOG_RAW_INFO("H : %s\n",NRF_LOG_PUSH(Telephone));
  //NRF_LOG_FLUSH();

  CoderBuffer_free(bufferDecrypted);
  Result res = Beacon_SendAPI(HackingBCode,idxUser,code,D);
  
  Debug(".%d",idxUser);

  return res;
}

//.0,-43[ 20 21 04 25 03 71 01 06 89 17 11 2F 04 04 04 04 ]
//G : 202104250881
//H : 1
//24[ 00 04 00 20 81 0A 00 00 15 07 00 00 61 0A 00 00 1F 07 00 00 29 07 00 00 ]
//32[ 00 04 00 20 81 0A 00 00 15 07 00 00 61 0A 00 00 1F 07 00 00 29 07 00 00 33 07 00 00 00 00 00 00 ]


extern int ClientCode;
extern char* apcode;

Result Beacon_SendAPI(char* B,uint32_t idxUser,int F,int D)
{
  int NewCode = F;//F*10 + ClientCode%10;//this is W

  CoderBuffer_print_nonpointer(CoderBuffer_make(addr_IV(NewCode),16),16);
  CoderBuffer_print_nonpointer(CoderBuffer_make(addr_Key(NewCode),32),16);

  Debug("325\n");
  //X <= G(일자+해킹코드)[12]
  CoderBuffer XData_buffer = CoderBuffer_clone(B,strlen(B));
  Debug("326\n");
  //CoderBuffer_print_nonpointer(XData_buffer,16);
  CoderBuffer *XData_1 = Coder_AESEncode(&XData_buffer,NewCode);
  CoderBuffer_print(XData_1,16);
  Debug("327\n");
  CoderBuffer *XData_2 = Coder_Base64Encode(XData_1);
  Debug("328\n");
  CoderBuffer_print(XData_2,16);
  CoderBuffer *XData = Coder_Base64Encode(XData_2);
  Debug("329\n");
  CoderBuffer_print(XData,16);
  if(XData == 0)
  {
    printf("Xfailed\n");
    return FAILED;
  }
  CoderBuffer_print(XData,16);
  
  Debug("334\n");
  //Y <= H(idxUser)[15]
  uint8_t H[10] = {0,};
  sprintf(H,"%d",idxUser);
  CoderBuffer YData_buffer = CoderBuffer_clone(H,strlen(H));
  //CoderBuffer_print_nonpointer(YData_buffer,16);
  CoderBuffer *YData_1 = Coder_AESEncode(&YData_buffer,NewCode);
  CoderBuffer *YData = Coder_Base64Encode(Coder_Base64Encode(YData_1));
  if(YData == 0)
  {
    printf("Yfailed\n");
    return FAILED;
  }
  CoderBuffer_print(YData,16);


  Debug("346\n");
  CoderBuffer AData_buffer = CoderBuffer_clone(apcode,strlen(apcode));
  //CoderBuffer_print_nonpointer(AData_buffer,16);
  CoderBuffer *AData_1 = Coder_AESEncode(&AData_buffer,NewCode);
  CoderBuffer *AData = Coder_Base64Encode(Coder_Base64Encode(AData_1));
  if(AData == 0)
  {
    printf("Afailed\n");
    return FAILED;
  }
  CoderBuffer_print(AData,16);
  
  uint8_t Wdata[7];
  sprintf(Wdata,"%d%d",ClientCode,D);
  Debug("prepare finished!\n");

  //Result res = API_Send_BEACON(XData,YData,AData,Wdata);

  CoderBuffer_free(AData);
  CoderBuffer_free(XData);
  CoderBuffer_free(YData);

  return SUCCEED;//res;
}