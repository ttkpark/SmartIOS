// Codec file based on tiny-AES-c.
// https://github.com/kokke/tiny-AES-c

#include "main.h"
#include "aes.h"
#include "base64.h"
#include "aesData.h"
#include "Coder.h"

void CoderBuffer_deinit(CoderBuffer* buffer)
{
  if(buffer == NULL)return;

  buffer->len = 0;
  buffer->buffer = 0;
}
void CoderBuffer_free(CoderBuffer* buffer)
{
  if(buffer == NULL || buffer->buffer == NULL || buffer->len == 0)return;

  free(buffer->buffer);
  CoderBuffer_deinit(buffer);
}
void CoderBuffer_malloc(CoderBuffer* buffer,LENGTH len)
{
  Debug("CoderBuffer_malloc\n");
  if(buffer == NULL || buffer->buffer != NULL || buffer->len != 0)return;

  Debug("28\n");
  buffer->buffer = malloc(len);
  buffer->len = len;
  if(buffer->buffer == 0)Debug("malloc error\n");
}
CoderBuffer CoderBuffer_make(uint8_t *data,int len)
{
  CoderBuffer out;
  out.buffer = data;
  out.len    = len;
  return out;
}
CoderBuffer CoderBuffer_clone(uint8_t *data,int len)
{
  uint8_t* Newdata = malloc(len);
  memcpy(Newdata,data,len);
  return CoderBuffer_make(Newdata,len);
}
void CoderBuffer_print(CoderBuffer *buffer,int line)
{
  Debug("[%d]{\n",buffer->len);
  int pointer = 0;
  while(pointer <= buffer->len-1)
  {
    Debug(" %02X",buffer->buffer[pointer]);
    if(pointer%line == (line-1))Debug("\n");
    pointer++;
  }
  Debug(" }\n");
}
void CoderBuffer_print_nonpointer(CoderBuffer buffer,int line)
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



CoderBuffer *Coder_AESEncode(CoderBuffer *input,int code)
{
  if(input == NULL)return NULL;
  //make padding
  //(len:1~15 -> len:16)
  //(len:16~31 -> len:32)...

  int finallength = 16*(input->len/16) + 16;
  CoderBuffer ret,*out = &ret;
  CoderBuffer_deinit(out);
  CoderBuffer_malloc(out,finallength);
  makepadding(out->buffer,input->buffer,finallength,input->len);

  AESEncode(out->buffer,finallength,code);

  CoderBuffer_free(input);
  *input = ret;
  return input;
}
CoderBuffer *Coder_AESDecode(CoderBuffer *input,int code)
{
  if(input == NULL)return NULL;
  //decode padding
  //(len:1~15 <- len:16)
  //(len:16~31 <- len:32)...
  AESDecode(input->buffer,input->len,code);

  int padding = input->buffer[input->len-1];
  int finallength = input->len - padding;
  if(padding > 0x10 || padding <= 0x00)return NULL;
  if(finallength < 0)return NULL;

  CoderBuffer ret,*out = &ret;
  CoderBuffer_deinit(out);
  CoderBuffer_malloc(out,finallength);
  
  memcpy(out->buffer,input->buffer,finallength);

  CoderBuffer_free(input);
  *input = ret;
  return input;
}

CoderBuffer *Coder_Base64Encode(CoderBuffer *input)
{
  Debug("Coder_Base64Encode\n");
  if(input == NULL)return NULL;
  int finallength = text2base64_size(input->len);
  CoderBuffer ret,*out = &ret;
  
  Debug("164\n");
  CoderBuffer_deinit(out);
  Debug("165_%d\n",finallength);
  CoderBuffer_malloc(out,finallength);

  Debug("166\n");
  int res = base64_encoder(input->buffer,input->len,out->buffer,out->len);
  Debug("167\n");
  if(res!=finallength)
  {
    Debug("169\n");
    CoderBuffer_free(input);
    CoderBuffer_free(out);
    return NULL;
  }
  Debug("171\n");
  CoderBuffer_free(input);
  
  *input = ret;
  Debug("174\n");
  return input;
}
CoderBuffer *Coder_Base64Decode(CoderBuffer *input)
{
  if(input == NULL)return NULL;
  int finallength = base642text_size(input->buffer,input->len);
  CoderBuffer ret,*out = &ret;
  
  CoderBuffer_deinit(out);
  CoderBuffer_malloc(out,finallength);

  int res = base64_decoder(input->buffer,input->len,out->buffer,out->len);
  if(res!=finallength)
  {
    CoderBuffer_free(input);
    CoderBuffer_free(out);
    return NULL;
  }
  CoderBuffer_free(input);
  *input = ret;
  return input;
}