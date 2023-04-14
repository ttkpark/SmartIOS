#pragma once

#ifndef ADDSTR__h
#define ADDSTR__h

#ifdef __cplusplus
extern "C"
{
#endif
struct __Addstr_Str {
	char* str;
	int size;
};
typedef struct __Addstr_Str Addstr_Str;

char* Addstr_return(Addstr_Str* str);
void Addstr_destroy(Addstr_Str* str);
void Addstr_add(Addstr_Str* str, char* addstr);
void Addstr_addlen(Addstr_Str* str, char* addstr, int addlen);
int  Addstr_init(Addstr_Str* str);
int  Addstr_initlen(Addstr_Str* str,int len);


#ifdef __cplusplus
}
#endif

#endif