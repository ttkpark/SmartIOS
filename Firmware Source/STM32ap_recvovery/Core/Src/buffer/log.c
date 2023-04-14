#include "main.h"
#include "buffer/log.h"


#define LOGMAXLEN 92
#define LOGMAXNUM 15
uint8_t LOGBuffer_CRITICAL[LOGMAXNUM][LOGMAXLEN+1] = {0,};
int log_critical_focus = 0;

#define LOGDEBUG_MAXLEN 70
#define LOGDEBUG_MAXNUM 20
uint8_t LOGBuffer_DEBUG[LOGDEBUG_MAXNUM][LOGMAXLEN+1] = {0,};
int log_debug_focus = 0;


void Log(LogLEVEL LEVEL,char* log,int len)
{
    if(LEVEL == CRITICAL)
    {
        do{
            if(log_critical_focus < LOGMAXNUM)
            {
                int writelen = (len<=LOGMAXLEN) ? len : LOGMAXLEN;
                memcpy(LOGBuffer_CRITICAL[log_critical_focus],log,writelen);
                LOGBuffer_CRITICAL[log_critical_focus++][writelen] = 0;//null termination
                
                len -= LOGMAXLEN;
            }else printf("LOG_CRITICAL FULL!\n");
        }while(len >= 0);
    }
    else
    {
        do{
            if(log_debug_focus < LOGDEBUG_MAXNUM)
            {
                int writelen = (len<=LOGDEBUG_MAXLEN) ? len : LOGDEBUG_MAXLEN;
                memcpy(LOGBuffer_DEBUG[log_debug_focus],log,writelen);
                LOGBuffer_DEBUG[log_debug_focus++][writelen] = 0;//null termination
                
                len -= LOGDEBUG_MAXLEN;
            }else printf("LOG_DEBUG FULL!\n");
        }while(len >= 0);
    }
}

void Send_API_Log(LogLEVEL LEVEL)
{
    Debug("Send_API_Log\n");


}