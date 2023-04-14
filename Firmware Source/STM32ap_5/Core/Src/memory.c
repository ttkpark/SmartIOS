#include "main.h"
#include "cJSON.h"
#include "eth-main.h"
#include "Coder.h"
#include "Internet.h"
#include "memory.h"

extern SPI_HandleTypeDef hspi1;

Result SPI_Write(uint8_t *data,int len)
{
    HAL_StatusTypeDef res = HAL_SPI_Transmit(&hspi1,data,len,len*0.1 + 2);
    if(res != HAL_OK)
    {
        printf("SPI Write Error %d\n",res);
        return FAILED;
    }
    else return SUCCEED;
}
Result SPI_Read(uint8_t *data,int len)
{
    HAL_StatusTypeDef res = HAL_SPI_Receive(&hspi1,data,len,len*0.1 + 2);
    if(res != HAL_OK)
    {
        printf("SPI Read Error %d\n",res);
        return FAILED;
    }
    else return SUCCEED;
}
Result MEM_WriteInstruction(uint8_t inst)
{
    SPI_Select(1);
    if(SPI_Write(&inst,1) != SUCCEED)
    {
        SPI_Select(0);
        return FAILED;
    }
    SPI_Select(0);
    return SUCCEED;
}
//1:select 0:deselect
void SPI_Select(int state)
{
    HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(ENC_SS_GPIO_Port,ENC_SS_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(MEM_CS_GPIO_Port,MEM_CS_Pin,!state);
}
Result MEM_NormalRead(uint32_t addr,uint8_t *buffer,int readlen)
{
    SPI_Select(1);
    uint8_t inst[4] = {NORD,(addr>>16)&0xFF,(addr>>8)&0xFF,addr&0xFF};
    SPI_Write(inst,4);

    if(SPI_Read(buffer,readlen) != SUCCEED)
    {
        SPI_Select(0);
        return FAILED;
    }
    SPI_Select(0);
    return SUCCEED;
}
//don't forget to ENABLE write operation.
//and also to ERASE this sector.
// 256 bytes programming instruction.
Result MEM_PageProgram(uint32_t pageaddr,uint8_t *data,int len)
{
    MEM_WriteEnable();
    SPI_Select(1);
    uint8_t inst[4] = {PP,(pageaddr>>16)&0xFF,(pageaddr>>8)&0xFF,pageaddr&0xFF};
    SPI_Write(inst,4);
    if(SPI_Write(data,len) != SUCCEED)
    {
        SPI_Select(0);
        MEM_WriteDisable();
        return FAILED;
    }
    SPI_Select(0);

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)up_udelay(200);
    //check WIP(Status Register) byte until it becomes '0'.
    MEM_WriteDisable();
    return SUCCEED;
}
Result MEM_Program(uint32_t pageaddr,uint8_t *data,int len)
{
    // get the first page range
    printf("MEM_Program write %06Xh ~ %06Xh\n",pageaddr,pageaddr+len-1);
    while(len>0)
    {
        int pageend = ((pageaddr>>8) + 1)<<8;
        int written = pageend - pageaddr;
        if(written > len)written = len;

        printf("write %06Xh ~ %06Xh\n",pageaddr,pageaddr+written-1);
        if(MEM_PageProgram(pageaddr,data,written) == FAILED)return FAILED;
        len -= written;
        data += written;
        pageaddr = pageend;
    }
    return SUCCEED;
}

Result MEM_EraseSector(uint32_t pageaddr)
{
    MEM_WriteEnable();
    SPI_Select(1);
    uint8_t inst[4] = {SER,(pageaddr>>16)&0xFF,(pageaddr>>8)&0xFF,pageaddr&0xFF};
    if(SPI_Write(inst,4) != SUCCEED)
    {
        SPI_Select(0);
        MEM_WriteDisable();
        return FAILED;
    }
    SPI_Select(0);

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)HAL_Delay(25);
    //check WIP(Status Register) byte until it becomes '0'.
    MEM_WriteDisable();
    return SUCCEED;
}
//inst is BER32K or BER64K
Result MEM_EraseBlock(uint8_t instruction,uint32_t pageaddr,uint8_t data,int len)
{
    MEM_WriteEnable();
    SPI_Select(1);
    uint8_t inst[4] = {instruction,(pageaddr>>16)&0xFF,(pageaddr>>8)&0xFF,pageaddr&0xFF};
    if(SPI_Write(inst,4) != SUCCEED)
    {
        SPI_Select(0);
        MEM_WriteDisable();
        return FAILED;
    }
    SPI_Select(0);

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)HAL_Delay(50);
    //check WIP(Status Register) byte until it becomes '0'.
        MEM_WriteDisable();
    return SUCCEED;
}

Result MEM_EraseChip()
{
    if(MEM_WriteInstruction(CER) != SUCCEED)
        return FAILED;

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)HAL_Delay(50);
    //check WIP(Status Register) byte until it becomes '0'.
    return SUCCEED;
}

Result MEM_WriteEnable()
{
    return MEM_WriteInstruction(WREN);
}
Result MEM_WriteDisable()
{
    return MEM_WriteInstruction(WRDI);
}
uint8_t MEM_readStatus()
{
    uint8_t data;
    
    SPI_Select(1);
    uint8_t inst[1] = {RDSR};
    SPI_Write(inst,1);

    SPI_Read(&data,1);
    SPI_Select(0);
    return data;
}
Result MEM_writeStatus(uint8_t status)
{
    SPI_Select(1);
    uint8_t inst[2] = {WRSR,status};
    if(SPI_Write(inst,2) != SUCCEED)
    {
        SPI_Select(0);
        return FAILED;
    }
    SPI_Select(0);
    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)up_udelay(200);
    return SUCCEED;
}
Result MEM_INIT()
{
    printf("MEM chip Initalize\n");
    MEM_WriteEnable();
    MEM_writeStatus(0x00);
    MEM_WriteDisable();
    HAL_Delay(10);

    MEM_WriteInstruction(RSTEN);
    MEM_WriteInstruction(RST);
    HAL_Delay(10);
    
    SPI_Select(1);
    uint8_t inst[3] = {0x9F};
    SPI_Write(inst,1);
    if(SPI_Read(inst,3)==FAILED)
    {
        printf("SPI Interfacing Failed. Exit.\n");
        SPI_Select(0);
        return FAILED;
    }
    SPI_Select(0);

    printf("Manufacture : %02X\n",inst[0]);
    printf("Memory Type : %02X\n",inst[1]);
    printf("Capacity    : %02X\n",inst[2]);
    return SUCCEED;
}


#define SECTOR_SAVEDATA 0
Result Store_SaveData(cJSON* savedata)
{
    Result res = SUCCEED;

    res = MEM_WriteEnable();printf("%d",res);
    printf("stat : %02X",MEM_readStatus());
    res = MEM_EraseSector(0x000000);printf("%d",res);
    char* data = cJSON_Print(savedata);
    int len = strlen(data);
    res = MEM_PageProgram(0x000000,&len,4);printf("%d",res);
    res = MEM_Program(0x000004,data,len);printf("%d",res);
    res = MEM_WriteDisable();printf("%d",res);
    //printf("%s",data);
    free(data);
    return res != 0;
}
Result Read_SaveData(cJSON ** pjson)
{
    int len = 0;
    HAL_Delay(1);
    if(MEM_NormalRead(0x000000,&len,4) == FAILED)
        return FAILED;
    printf("len    : %02d\n",len);
    
    CoderBuffer buffer;
    if(CoderBuffer_malloc(&buffer,len+1) == FAILED)
        return FAILED;
    
    HAL_Delay(1);
    if(MEM_NormalRead(0x000004,buffer->buffer,len) == FAILED){
        CoderBuffer_free(buffer);
        return FAILED;
    };
    buffer->buffer[len] = '\0';//null termination
    *pjson = cJSON_Parse(buffer->buffer);
    CoderBuffer_free(buffer);
    return SUCCEED;
}


char ConvertBuff[23] = {0,};
Date Convert_Str2Date(char* str)
{
    Date date = {0,0,0,0,0,0};
    if(str==NULL)return date;
    sscanf(str,"%4d%2d%2d%2d%2d%2d",&date.year,&date.month,&date.day,&date.hour,&date.minute,&date.second);
    return date;
}
uint32_t Date2Int(Date date)
{
    return ((((date.year-2000)*(12) + date.month)*31 + date.day)*24 + date.hour)*60 + date.minute;
}
char* Convert_Date2Str(Date date)
{
    sprintf(ConvertBuff,"%04d%02d%02d%02d%02d%02d",date.year,date.month,date.day,date.hour,date.minute,date.second);
    return ConvertBuff;
}
IP_ADDR Convert_Str2IP(char* str)
{
    IP_ADDR ip = IPSet(0,0,0,0);
    if(str==NULL)return ip;
    //sscanf(str,"%d.%d.%d.%d",&ip.v[0],&ip.v[1],&ip.v[2],&ip.v[3]);

    ip.v[0] = atoi(str);
    for(int i=1;i<4;i++)
    {
        char* focus = strchr(str,'.');
        if(focus == NULL)break;

        ip.v[i] = atoi(focus+1);
        str = focus + 1;
    } 
    return ip;
}
char* Convert_IP2Str(IP_ADDR ip)
{
    sprintf(ConvertBuff,"%d.%d.%d.%d",ip.v[0],ip.v[1],ip.v[2],ip.v[3]);
    return ConvertBuff;
}


extern SavedData saveData;
extern cJSON *saveDataJSON;
Result MEM_Store_SaveData()
{
    Result res = SUCCEED;
    cJSON * json = saveDataJSON;
    saveDataJSON = cJSON_CreateObject();
    if(saveDataJSON == NULL)return FAILED;

    cJSON_AddNumberToObject(saveDataJSON,"apcode",(double)saveData.apcode);
    cJSON_AddNumberToObject(saveDataJSON,"idxClient",(double)saveData.idxClient);
    cJSON_AddStringToObject(saveDataJSON,"date",Convert_Date2Str(saveData.date));
    cJSON_AddNumberToObject(saveDataJSON,"dist",(double)saveData.dist);
    cJSON_AddBoolToObject  (saveDataJSON,"useyn",saveData.useyn);
    cJSON_AddStringToObject(saveDataJSON,"apip",Convert_IP2Str(saveData.apip));
    cJSON_AddStringToObject(saveDataJSON,"netmask",Convert_IP2Str(saveData.netmask));
    cJSON_AddStringToObject(saveDataJSON,"gwip",Convert_IP2Str(saveData.gwip));
    cJSON_AddStringToObject(saveDataJSON,"dnsip",Convert_IP2Str(saveData.dnsip));
    cJSON_AddStringToObject(saveDataJSON,"dns2ip",Convert_IP2Str(saveData.dns2ip));
    cJSON_AddNumberToObject(saveDataJSON,"IdentifyDistance",(double)saveData.IdentifyDistance);
    cJSON_AddNumberToObject(saveDataJSON,"BeaconTimeout",(double)saveData.BeaconTimeout);
    cJSON_AddNumberToObject(saveDataJSON,"enableOutDetecting",(double)saveData.enableOutDetecting);
    cJSON_AddBoolToObject  (saveDataJSON,"dbuglog",saveData.dbuglog);
    cJSON_AddNumberToObject(saveDataJSON,"reset",(double)saveData.reset);
    cJSON_AddBoolToObject  (saveDataJSON,"isDHCP",saveData.isDHCP);
    cJSON_AddStringToObject(saveDataJSON,"CDMURL",saveData.CDMURL);
    cJSON_AddNumberToObject(saveDataJSON,"ipcdInterval",(double)saveData.ipcdInterval);
    cJSON_AddStringToObject(saveDataJSON,"domain",saveData.domain);
    cJSON_AddStringToObject(saveDataJSON,"ServerURL",saveData.ServerURL);
    cJSON_AddStringToObject(saveDataJSON,"pgver",saveData.pgver);
    cJSON_AddStringToObject(saveDataJSON,"pgverdate",Convert_Date2Str(saveData.pgverdate));
    cJSON_AddStringToObject(saveDataJSON,"CDMURL",saveData.CDMURL);
    
    cJSON_Delete(json);

    return Store_SaveData(saveDataJSON);
}
Result MEM_Read_SaveData()
{
    if(Read_SaveData(&saveDataJSON) == FAILED)return FAILED;


    saveData.apcode             = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"apcode"));
    saveData.idxClient          = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"idxClient"));
    saveData.date               = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"date")));
    saveData.dist               = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"dist"));
    saveData.useyn              = cJSON_GetObjectItem(saveDataJSON,"useyn")->type == cJSON_True;
    saveData.apip               = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"apip")));
    saveData.netmask            = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"netmask")));
    saveData.gwip               = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"gwip")));
    saveData.dnsip              = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"dnsip")));
    saveData.dns2ip             = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"dns2ip"))); 
    saveData.IdentifyDistance   = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"IdentifyDistance"));
    saveData.BeaconTimeout      = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"BeaconTimeout"));
    saveData.enableOutDetecting = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"enableOutDetecting"));
    saveData.dbuglog            = cJSON_GetObjectItem(saveDataJSON,"dbuglog")->type == cJSON_True; 
    saveData.reset              = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"reset"));
    saveData.isDHCP             = cJSON_GetObjectItem(saveDataJSON,"isDHCP")->type == cJSON_True; 
    saveData.ipcdInterval       = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"ipcdInterval"));
    saveData.domain             = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"domain")); 
    saveData.ServerURL          = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"ServerURL"));
    saveData.pgver              = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"pgver"));
    saveData.pgverdate          = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"pgverdate")));
    saveData.CDMURL             = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"CDMURL"));
    return SUCCEED;
}



void  MEM_Print(uint32_t addr,uint32_t len)
{
    uint8_t buffer[16] = {0,};
    for(;len>0;)
    {
        int readlen = len>16 ? 16 : len;
        MEM_NormalRead(addr,buffer,readlen);
        HAL_Delay(1);

        printf("%04X : ",addr);
        for(int j=0;j<readlen;j++)
            printf("%02X ",buffer[j]);
        printf("\n");

        addr += readlen;
        len -= readlen;
    }
}