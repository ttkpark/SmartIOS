// Codec file based on tiny-AES-c.
// https://github.com/kokke/tiny-AES-c

#include "main.h"
#include "aes.h"
#include "base64.h"
#include "aesData.h"
#include "Coder.h"


#define CODERBUFFER_BUFSIZE 128
#define CODERBUFFER_BANKNUM 8

//BANK 개별 관리형 coderbuff
char coderBuff[CODERBUFFER_BANKNUM][CODERBUFFER_BUFSIZE];
CoderBuff coderBuff_Bank[CODERBUFFER_BANKNUM];

//return -1 when failed
int CoderBuff_seekEmptyBank()
{
	for(int i=0;i<CODERBUFFER_BANKNUM;i++)
	{
		//Debug("Codebuff..[%d]len=%d\n",i,coderBuff_Bank[i].len);
		if(coderBuff_Bank[i].len > CODERBUFFER_BUFSIZE || coderBuff_Bank[i].len == 0)
			return i;
	}
	return -1;
}
int getCoderBuffBankNum(CoderBuffer buffer)
{
	for(int i=0;i<CODERBUFFER_BANKNUM;i++)
	{
		if(buffer->buffer == coderBuff_Bank[i].buffer)
			return i;
	}
	return -1;
}

void CoderBuffer_deinit(CoderBuffer buffer)
{
  if(buffer == NULL)return;

  buffer->len = 0;
  //Debug("Coderbuffer_deinit bank %d\n",getCoderBuffBankNum(buffer));
}
void CoderBuffer_free(CoderBuffer buffer)
{
  if(buffer == NULL)return;

  // free(buffer->buffer);
  CoderBuffer_deinit(buffer);
}
Result CoderBuffer_malloc(CoderBuffer* bufptr,LENGTH len)
{
  if(bufptr == NULL)return FAILED;

  int bank = CoderBuff_seekEmptyBank();
  if(bank == -1)
  { 
    Debug("No Empty Bank!!\n");
    return FAILED;
  }

  //Debug("28\n");
  //buffer->buffer = malloc(len);
  //buffer->len = len;
  //if(buffer->buffer == 0)Debug("malloc error\n");

  *bufptr = &coderBuff_Bank[bank];
  (*bufptr)->buffer = coderBuff[bank];
  (*bufptr)->len = len;
  //Debug("CoderBuffer_malloc bank %d\n",getCoderBuffBankNum(*bufptr));
  return SUCCEED;
}
CoderBuff CoderBuffer_make(uint8_t *data,int len)
{
  CoderBuff out;
  out.buffer = data;
  out.len    = len;
  return out;
}
CoderBuffer CoderBuffer_clone(uint8_t *data,int len)
{
  CoderBuffer out;
  Result res = CoderBuffer_malloc(&out,len);
  if(res == FAILED)return NULL; 

  memcpy(out->buffer,data,len);
  return out;
}
void CoderBuffer_print(CoderBuffer buffer,int line)
{
  if(buffer == 0){
    Debug("[%d]{ Wrong Memory. }\n");
    return;
  }

  Debug("[%d]{",buffer->len);
  if(line > buffer->len)
    putchar('\n');

  int pointer = 0;
  while(pointer <= buffer->len-1)
  {
    Debug(" %02X",buffer->buffer[pointer]);
    if(pointer%line == (line-1) && (line < buffer->len))Debug("\n");
    pointer++;
  }
  Debug(" }\n");
}
void CoderBuffer_print_nonpointer(CoderBuff buffer,int line)
{
  CoderBuffer_print(&buffer,line);
}


uint8_t* addr_IV(int code)
{
  if(code>=NUM_KEYS)return 0;
  return AESKeyIVs[code]+32;
}
uint8_t* addr_Key(int code)
{
  if(code>=NUM_KEYS)return 0;
  return AESKeyIVs[code];
}



//input becomes outputs.
void AESEncode(uint8_t* input_output,int len,int code)
{
  struct AES_ctx ctx;

  AES_init_ctx_iv(&ctx, addr_Key(code), addr_IV(code));
  AES_CBC_encrypt_buffer(&ctx, input_output, len);
}

//input becomes outputs.
void AESDecode(uint8_t* input_output,int len,int code)
{
  struct AES_ctx ctx;

  AES_init_ctx_iv(&ctx, addr_Key(code), addr_IV(code));
  AES_CBC_decrypt_buffer(&ctx, input_output, len);
}

void makepadding(uint8_t* dest,uint8_t* data,int dest_len,int data_len)
{
  // data        : AA BB CC DD EE FF 00 00 00 00 00 00 00 00 00 00
  // destination : AA BB CC DD EE FF 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 
  // (dest_len=6,data_len=16)

  int i=0;
  for(;i<data_len;i++)
    dest[i] = data[i];

  for(i=0;i<dest_len;i++)
    dest[i+data_len] = dest_len-data_len;

  //memcpy(dest,data,data_len);//fill the data
  //memset(dest + data_len,dest_len-data_len,dest_len-data_len);//padding
}



CoderBuffer Coder_AESEncode(CoderBuffer input,int code)
{
  if(input == NULL)return NULL;
  //make padding
  //(len:1~15 -> len:16)
  //(len:16~31 -> len:32)...

  int finallength = 16*(input->len/16) + 16;
  CoderBuffer ret;
  Result res = CoderBuffer_malloc(&ret,finallength);
  if(res == FAILED)return NULL; 
  makepadding(ret->buffer,input->buffer,finallength,input->len);

  AESEncode(ret->buffer,finallength,code);

  //Debug("Coder_AESEncode (bank%d,len%d)->(bank%d,len%d)\n",getCoderBuffBankNum(input),input->len,getCoderBuffBankNum(ret),finallength);
  CoderBuffer_free(input);

  return ret;
}
CoderBuffer Coder_AESDecode(CoderBuffer input,int code)
{
  if(input == NULL)return NULL;
  //decode padding
  //(len:1~15 <- len:16)
  //(len:16~31 <- len:32)...
  AESDecode(input->buffer,input->len,code);

  int padding = input->buffer[input->len-1];
  int finallength = input->len - padding;
  if(padding > 0x10 || padding <= 0x00   ||   finallength < 0)
  {
    CoderBuffer_free(input);
    return NULL;
  }

  CoderBuffer ret;
  Result res = CoderBuffer_malloc(&ret,finallength);
  if(res == FAILED)return NULL;
  
  memcpy(ret->buffer,input->buffer,finallength);

  //Debug("Coder_AESDecode (bank%d,len%d)->(bank%d,len%d)\n",getCoderBuffBankNum(input),input->len,getCoderBuffBankNum(ret),finallength);
  CoderBuffer_free(input);
  return ret;
}

CoderBuffer Coder_Base64Encode(CoderBuffer input)
{
  //Debug("Coder_Base64Encode\n");
  if(input == NULL)return NULL;
  int finallength = text2base64_size(input->len);
  CoderBuffer ret;
  
  //Debug("165_%d\n",finallength);
  Result res = CoderBuffer_malloc(&ret,finallength);
  if(res == FAILED)return NULL;
  //Debug("166\n");
  res = base64_encoder(input->buffer,input->len,ret->buffer,ret->len);
  //Debug("167\n");
  if(res!=finallength)
  {
    //Debug("169\n");
    CoderBuffer_free(input);
    CoderBuffer_free(ret);
    return NULL;
  }
  //Debug("171\n");
  //Debug("Coder_Base64Encode (bank%d,len%d)->(bank%d,len%d)\n",getCoderBuffBankNum(input),input->len,getCoderBuffBankNum(ret),finallength);
  CoderBuffer_free(input);
  
  //Debug("174\n");
  return ret;
}
CoderBuffer Coder_Base64Decode(CoderBuffer input)
{
  if(input == NULL)return NULL;
  int finallength = base642text_size(input->buffer,input->len);
  CoderBuffer ret;
  
  Result res = CoderBuffer_malloc(&ret,finallength);
  if(res == FAILED)return NULL;

  res = base64_decoder(input->buffer,input->len,ret->buffer,ret->len);
  if(res!=finallength)
  {
    CoderBuffer_free(input);
    CoderBuffer_free(ret);
    return NULL;
  }
  //Debug("Coder_Base64Decode (bank%d,len%d)->(bank%d,len%d)\n",getCoderBuffBankNum(input),input->len,getCoderBuffBankNum(ret),finallength);
  CoderBuffer_free(input);
  return ret;
}