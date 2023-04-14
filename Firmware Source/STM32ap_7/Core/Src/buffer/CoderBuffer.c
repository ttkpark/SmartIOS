// Codec file based on tiny-AES-c.
// https://github.com/kokke/tiny-AES-c

#include "main.h"
#include "buffer/CoderBuffer.h"
#include "buffer/aes.h"
#include "buffer/base64.h"
#include "JSON/tiny-json.h"
#include "Addstr.h"


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

  register bank = getCoderBuffBankNum(buffer);
  if(bank > (CODERBUFFER_BANKNUM - 4))
    Debug("CoderBuff Warning: DEINIT bank %d\n",bank);
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
  if(bank > (CODERBUFFER_BANKNUM - 4))
    Debug("CoderBuff Warning: malloc bank %d\n",bank);
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
    Debug("[%d]{ Wrong Memory. }\n",line);
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
    yield();
  }
  Debug(" }\n");
}
void CoderBuffer_print_nonpointer(CoderBuff buffer,int line)
{
  CoderBuffer_print(&buffer,line);
}
