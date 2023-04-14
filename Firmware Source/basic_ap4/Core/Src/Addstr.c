#include "Addstr.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
#define Debug printf

//ADDSTR : 유동 문자열 길이 관리 함수 집합.
// 문자열에 내용을 추가할 수 있다.
// - (개발 포인트) 길이가 넘칠 때마다 전체 길이를 2배씩 늘려 정의한다.

char* Addstr;
int Addstr_len;
int Addstr_init(Addstr_Str* str) {
	return Addstr_initlen(str,2);
}
int Addstr_initlen(Addstr_Str* str,int len) {
	//Debug("Addstr_Init() start");
	Addstr_destroy(str);
	str->str = (char*)malloc(len);
	if(!str->str)Debug("14:malloc Error!! Mem leak.");
	str->size = len;
	memset(str->str, 0, str->size);
	//Debug("str           = %d", str);
	//Debug("str->str      = %d", str->str);
	//Debug("str->size     = %d", str->size);
	//Debug("strlen        = %d", strlen(str->str));
	return 0;
}
void Addstr_add(Addstr_Str* str, char* addstr) {
	Addstr_addlen(str, addstr, strlen(addstr));
}
//전체 문자열에 주어진 길이(addlen)만큼 더한다.
void Addstr_addlen(Addstr_Str* str, char* addstr, int addlen) {
	// Debug("Addstr_add() start");
	// Debug("addstr        = %s", addstr);
	// Debug("str           = %d", str);
	// Debug("str->str      = %d", str->str);
	// Debug("str->size     = %d", str->size);
	int s_len = strlen(str->str), pre_len=0;
	// Debug("Addstr_add() 1");
	// Debug("str           = %d = {len:%d,size:%d}", (int)str, strlen(str->str), str->size);
	// Debug("len(addstr)   = %d", strlen(addstr));
	// Debug("addlen        = %d", addlen);
	while ((s_len + addlen + 1) > str->size) {
		pre_len = str->size;
		str->size *= 2;
		//Debug("Addstr_add() 2");
		char* tmp = (char*)malloc(str->size);
		if (!tmp)Debug("31:malloc Error!! Mem leak.");
		memcpy(tmp, str->str, pre_len);
		memset(tmp + pre_len, 0, pre_len);
		free(str->str);
		str->str = tmp;
		// Debug("Addstr_add() 3");
		// Debug("str           = %d = {len:%d,size:%d}", (int)str, strlen(str->str),str->size);
	}
	memcpy(str->str+s_len, addstr, addlen + 1);
	// Debug("523:Addstr(%d)", strlen(addstr));
	// Debug("str           = %d = {len:%d,size:%d}", (int)str, strlen(str->str),str->size);
	// Debug("len(addstr)   = %d", strlen(addstr));
	// Debug("addlen        = %d", addlen);
}
void Addstr_destroy(Addstr_Str* str) {
	if (str->size != 0)
	{
		free(str->str);
		str->str = 0;
		str->size = 0;
	}
}
char* Addstr_return(Addstr_Str* str) {
	return str->str;
}


#ifdef __cplusplus
}
#endif