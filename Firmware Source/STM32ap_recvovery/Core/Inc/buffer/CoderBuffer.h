
#ifndef _CODERBUFF_H_
#define _CODERBUFF_H_

void CoderBuffer_deinit(CoderBuffer buffer);
void CoderBuffer_free(CoderBuffer buffer);
Result CoderBuffer_malloc(CoderBuffer* bufptr,LENGTH len);
CoderBuff   CoderBuffer_make(uint8_t *data,int len);
CoderBuffer CoderBuffer_clone(uint8_t *data,int len);
void CoderBuffer_print(CoderBuffer buffer,int line);
void CoderBuffer_print_nonpointer(CoderBuff buffer,int line);

void makepadding(uint8_t* dest,uint8_t* data,int dest_len,int data_len);

#endif //_CODERBUFF_H_