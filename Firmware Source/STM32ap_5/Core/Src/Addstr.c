#include "main.h"
#include "Addstr.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
#define Debug printf


//BANK 개별 관리형 Addstr
char AddstrBuff[ADDSTR_BANKNUM][ADDSTR_BUFSIZE];
Addstr_Str Addstr_Bank[ADDSTR_BANKNUM];

//ADDSTR : 유동 문자열 길이 관리 함수 집합.
// 문자열에 내용을 추가할 수 있다.
// - (개발 포인트) 길이가 넘칠 때마다 전체 길이를 2배씩 늘려 정의한다.

//return -1 when failed
int AddStrseekEmptyBank()
{
	for(int i=0;i<ADDSTR_BANKNUM;i++)
	{
		//Debug("seek..[%d]size=%d\n",i,Addstr_Bank[i].size);
		if(Addstr_Bank[i].size > ADDSTR_BUFSIZE || Addstr_Bank[i].size == 0)
			return i;
	}
	return -1;
}
void Addstr_SystemInitalize()
{
	memset(Addstr_Bank,0,sizeof(Addstr_Bank));
	memset(AddstrBuff,0,sizeof(AddstrBuff));
}

char* Addstr;
int Addstr_len;
Result Addstr_init(Addstr_Str** str) {
	return Addstr_initlen(str,16);
}
Result Addstr_initlen(Addstr_Str** str,int len) {
	int bank = AddStrseekEmptyBank();

	if(bank >= ADDSTR_BANKNUM || bank < 0)
	{
		Debug("No Available Banks!!\n");
		return FAILED;
	}
	if(len > ADDSTR_BUFSIZE || len < 0)
    {
        Debug("Overflow or Invalid length.\n");
		return FAILED;
    }

	*str = &Addstr_Bank[bank];
	(*str)->str = AddstrBuff[bank];
	(*str)->size = len;	
	(*str)->bank = bank;

	memset((*str)->str, 0, (*str)->size);
	// Debug("str           = %d", str);
	// Debug("str->str      = %d", str->str);
	// Debug("str->size     = %d", str->size);
	// Debug("strlen        = %d", strlen(str->str));
    //Debug("bank %d allocate\n",bank);
	return SUCCEED;
}
Result Addstr_add(Addstr_Str* str, char* addstr) {
	return Addstr_addlen(str, addstr, strlen(addstr));
}
//전체 문자열에 주어진 길이(addlen)만큼 더한다.
Result Addstr_addlen(Addstr_Str* str, char* addstr, int addlen) {
	int str_len = strlen(str->str);
	if(str_len + addlen > ADDSTR_BUFSIZE) return FAILED;

	memcpy(str->str+str_len, addstr, addlen);
	str->str[str_len+addlen] = '\0';//null termination

	str->size = str_len + addlen;
    //Debug("bank %d addlen %d->%d\n",str->bank,str_len,str_len+addlen);
	return SUCCEED;
}
void Addstr_destroy(Addstr_Str* str) {
	if(str->bank > ADDSTR_BANKNUM) return;
    //Debug("bank %d free\n",str->bank);
	memset(str->str,0,str->size);
	str->size = 0;
}
char* Addstr_return(Addstr_Str* str) {
	return str->str;
}
Result Addstr_replace(Addstr_Str *inout,char* Remove,char* Replace)
{
    //Debug("bank %d replace\n",inout->bank);
    // inout -> in -> result(Replaced) -> out(length optimized) -> inout
    Addstr_Str *result;
	Result res = Addstr_initlen(&result,strlen(inout->str)*1.4);
	if(res == FAILED)return FAILED;

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

        // if(ptr_find!=NULL)
        //     Debug("in=%2d,find=%2d -> copy %2d to %2d(find-in) -> in = find+len=%d\n",ptr_in-inout->str,ptr_find-inout->str,ptr_in-inout->str,ptr_find - inout->str -1,inout->str - ptr_find + strlen(Remove));
        
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
        Debug("replace Warning!! : %d\n",__LINE__);
	if(after_len+1 > ADDSTR_BUFSIZE)
		Debug("Addstr Memory Overflow!! : %d\n",__LINE__);

	inout->size = after_len+1;
    memcpy(inout->str,result->str,strlen(result->str));// copy buffer to inout
    inout->str[strlen(result->str)] = 0; //null termination.

    Addstr_destroy(result);
    
    return SUCCEED;
}


#ifdef __cplusplus
}
#endif