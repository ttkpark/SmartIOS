#pragma once

#ifndef ADDSTR__h
#define ADDSTR__h

#ifdef __cplusplus
extern "C"
{
#endif


#define ADDSTR_BUFSIZE 400
#define ADDSTR_BANKNUM 4
#define ADDSTR_MAX_SIZE ADDSTR_BUFSIZE

struct __Addstr_Str {
	char* str;
	int size;
    int bank;
};
typedef struct __Addstr_Str Addstr_Str;

void Addstr_SystemInitalize();
char* Addstr_return(Addstr_Str* str);
void  Addstr_destroy(Addstr_Str* str);
Result Addstr_add(Addstr_Str* str, char* addstr);
Result Addstr_addlen(Addstr_Str* str, char* addstr, int addlen);
Result Addstr_init(Addstr_Str** str);
Result Addstr_initlen(Addstr_Str** str,int len);
Result Addstr_replace(Addstr_Str *inout,char* Remove,char* Replace);


#ifdef __cplusplus
}
#endif

#endif