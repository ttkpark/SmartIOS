#include "main.h"
#include "JSON/cJSON.h"
#include "ETH/eth-main.h"
#include "buffer/CoderBuffer.h"
#include "buffer/aes.h"
#include "buffer/base64.h"
#include "ETH/Internet.h"
#include "memory/memory.h"

#include "buffer/aesData.h"

extern SPI_HandleTypeDef hspi1;
uint8_t UUID[16];

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
//don't forget to ERASE this sector.
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
Result MEM_Verify(uint32_t pageaddr,uint8_t *compareData,int len)
{
    uint8_t data[256] = {0,};
    // get the first page range
    while(len>0)
    {
        int written = 256;
        if(written > len)written = len;

        //printf("verify %06Xh ~ %06Xh\n",pageaddr,pageaddr+written-1);
        while(MEM_NormalRead(pageaddr,data,written) == FAILED);
        if(memcmp(compareData,data,written) != 0)return FAILED;
        len -= written;
        pageaddr += written;
        compareData += written;
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

    //Reset the device
    MEM_WriteInstruction(RSTEN);
    MEM_WriteInstruction(RST);
    HAL_Delay(10);
    
    //RDJDID (JEDEC ID READ)
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

    if(inst[0] != 0x9D,inst[1] != 0x60)return FAILED;


    //Read Unique ID Number
    uint8_t instUUID[5] = {0x4B,0x00,0x00,0x00,0x00};
    
    SPI_Select(1);
    SPI_Write(instUUID,5);
    if(SPI_Read(UUID,16)==FAILED)
    {
        printf("SPI Reading UUID failed. Exit.\n");
        SPI_Select(0);
        return FAILED;
    }
    SPI_Select(0);
    printf("MAC will be : %02X:%02X:%02X:%02X:%02X:%02X\n"
        ,0x12,0x23,0x34,(UUID[0]+UUID[3]+UUID[6]+UUID[9]+UUID[12]+UUID[15])&0xFF,(UUID[1]+UUID[4]+UUID[7]+UUID[10]+UUID[13])&0xFF,(UUID[2]+UUID[5]+UUID[8]+UUID[11]+UUID[14]) & 0xFF);



//0xD5
    uint8_t first = 0,last = 0;
    if(MEM_NormalRead(Key_Start,&first,1) == FAILED);
    if(MEM_NormalRead(Key_Start+(SIZE_KEYS-1),&last,1) == FAILED);
    if(first != KeyFirstVal || last != KeyLastVal)
    {
        printf("AES Data is not avaliable.\n");

        //#define AES_RECOVERY

        #ifndef AES_RECOVERY
        while(1);
        #else
        for(int i=4;i<8;i++)
        {
            MEM_EraseSector(i*4096);
        }
        HAL_Delay(100);
        MEM_Program(Key_Start,AESKeyIVs,SIZE_KEYS);
        if(MEM_Verify(Key_Start,AESKeyIVs,SIZE_KEYS) == FAILED){
            printf("Verify Error!\n");
        }
        #endif
    }
    else printf("AES Data is ready.\n");
    // MEM_EraseSector(0x000000);
    
    // uint8_t buffer[498] = {0,};
    
    // for(int i=0;i<sizeof(buffer)/sizeof(uint8_t);i++)
    //     buffer[i] = i&0xFF;

    // MEM_Program(0x000000,buffer,sizeof(buffer));

    
    // MEM_Print(0,512);

    return SUCCEED;
}

extern char UARTBuffer[1024];

#define SECTOR_SAVEDATA 0
Result Store_SaveData(cJSON* savedata)
{
    Result res = SUCCEED;

    res |= MEM_WriteEnable();printf("%d",res);
    printf("stat : %02X",MEM_readStatus());
    res |= MEM_EraseSector(0x000000);printf("%d",res);

    char* data = cJSON_Print(savedata);
    int len = strlen(data);
    res |= MEM_PageProgram(0x000000,&len,4);printf("%d",res);
    res |= MEM_Program(0x000004,data,len);printf("%d",res);
    res |= MEM_WriteDisable();printf("%d",res);

    res |= MEM_Verify(0x000004,data,len);printf("%d",res);

    //printf("%s",data);
    free(data);
    return res != 0;
    //debug print inspection
    // 000000
    // 1. WriteEnable 2.Erase 3.Program(len) 4.Program(data) 5.WriteDisable 6.Verify
}
Result Read_SaveData(cJSON ** pjson)
{
    volatile uint32_t len = 0;
    HAL_Delay(1);
    if(MEM_NormalRead(0x000000,&len,4) == FAILED){
        printf("memory.c:%d Reading length failed.\n",__LINE__);
        return FAILED;
    }
    printf("len    : %02d\n",len);
    if(len > sizeof(UARTBuffer)){
        Debug("memory.c:%d buff full.\n",__LINE__);
        return FAILED;
    }

    printf("last operation of Serial Port will be cleaned.\n");

    HAL_Delay(1);
    if(MEM_NormalRead(0x000004,UARTBuffer,len) == FAILED){
        Debug("memory.c:%d Reading from MEM failed.\n",__LINE__);
        return FAILED;
    };
    UARTBuffer[len] = '\0';//null termination
    *pjson = cJSON_Parse(UARTBuffer);
    return SUCCEED;
}


// SavedData <-> cJSON
// Result MEM_make_SavedData(SavedData* saveData)
// {
//     Result res = SUCCEED;
//     saveDataJSON = cJSON_CreateObject();
//     if(saveDataJSON == NULL)return FAILED;
//
//     cJSON_AddNumberToObject(saveDataJSON,"apcode",(double)saveData->apcode);
//     cJSON_AddNumberToObject(saveDataJSON,"idxClient",(double)saveData->idxClient);
//     cJSON_AddStringToObject(saveDataJSON,"date",Convert_Date2Str(saveData->date));
//     cJSON_AddNumberToObject(saveDataJSON,"dist",(double)saveData->dist);
//     cJSON_AddBoolToObject  (saveDataJSON,"useyn",saveData->useyn);
//     cJSON_AddStringToObject(saveDataJSON,"apip",Convert_IP2Str(saveData->apip));
//     cJSON_AddStringToObject(saveDataJSON,"netmask",Convert_IP2Str(saveData->netmask));
//     cJSON_AddStringToObject(saveDataJSON,"gwip",Convert_IP2Str(saveData->gwip));
//     cJSON_AddStringToObject(saveDataJSON,"dnsip",Convert_IP2Str(saveData->dnsip));
//     cJSON_AddStringToObject(saveDataJSON,"dns2ip",Convert_IP2Str(saveData->dns2ip));
//     cJSON_AddNumberToObject(saveDataJSON,"IdentifyDistance",(double)saveData->IdentifyDistance);
//     cJSON_AddNumberToObject(saveDataJSON,"BeaconTimeout",(double)saveData->BeaconTimeout);
//     cJSON_AddNumberToObject(saveDataJSON,"enableOutDetecting",(double)saveData->enableOutDetecting);
//     cJSON_AddBoolTomObject  (saveDataJSON,"dbuglog",saveData->dbuglog);
//     cJSON_AddNumberToObject(saveDataJSON,"reset",(double)saveData->reset);
//     cJSON_AddBoolToObject  (saveDataJSON,"isDHCP",saveData->isDHCP);
//     cJSON_AddStringToObject(saveDataJSON,"CDMURL",saveData->CDMURL);
//     cJSON_AddNumberToObject(saveDataJSON,"ipcdInterval",(double)saveData->ipcdInterval);
//     cJSON_AddStringToObject(saveDataJSON,"domain",saveData->domain);
//     cJSON_AddStringToObject(saveDataJSON,"ServerURL",saveData->ServerURL);
//     cJSON_AddStringToObject(saveDataJSON,"pgver",saveData->pgver);
//     cJSON_AddStringToObject(saveDataJSON,"pgverdate",Convert_Date2Str(saveData->pgverdate));
//     cJSON_AddStringToObject(saveDataJSON,"CDMURL",saveData->CDMURL);
//  
//     Result res = Store_SaveData(saveDataJSON);
// 
//     cJSON_Delete(saveDataJSON);
//
//     return res;
// }
void MEM_make_SavedData(SavedData* destsaveData,cJSON* saveDataJSON)
{
    // cJSON* saveDataJSON;
    // if(Read_SaveData(&saveDataJSON) == FAILED)return FAILED;

    destsaveData->apcode             = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"apcode"));
    destsaveData->idxClient          = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"idxClient"));
    destsaveData->date               = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"date")));
    destsaveData->dist               = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"dist"));
    destsaveData->useyn              = cJSON_GetObjectItem(saveDataJSON,"useyn")->type == cJSON_True;
    destsaveData->apip               = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"apip")));
    destsaveData->netmask            = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"netmask")));
    destsaveData->gwip               = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"gwip")));
    destsaveData->dnsip              = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"dnsip")));
    destsaveData->dns2ip             = Convert_Str2IP(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"dns2ip"))); 
    destsaveData->IdentifyDistance   = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"IdentifyDistance"));
    destsaveData->BeaconTimeout      = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"BeaconTimeout"));
    destsaveData->enableOutDetecting = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"enableOutDetecting"));
    destsaveData->dbuglog            = cJSON_GetObjectItem(saveDataJSON,"dbuglog")->type == cJSON_True; 
    destsaveData->reset              = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"reset"));
    destsaveData->isDHCP             = cJSON_GetObjectItem(saveDataJSON,"isDHCP")->type == cJSON_True; 
    destsaveData->ipcdInterval       = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"ipcdInterval"));
    destsaveData->domain             = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"domain")); 
    destsaveData->ServerURL          = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"ServerURL"));
    destsaveData->pgver              = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"pgver"));
    destsaveData->pgverdate          = Convert_Str2Date(cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"pgverdate")));
    destsaveData->CDMURL             = cJSON_GetStringValue(cJSON_GetObjectItem(saveDataJSON,"CDMURL"));
    destsaveData->aes_no             = cJSON_GetNumberValue(cJSON_GetObjectItem(saveDataJSON,"aes_no"));
}


//SaveData : (데이터들 + text원본(cJSON*))
// 용도
// 1) RAM에 데이터를 저장하고 있는다.(원본 필요함) -> 딱 하나 SavedData , cJSON* 하나씩 따로 저장.
// 2) 변경이 필요할 때가 있다. -> 
    // 1. 부팅 시 verCode 업데이트
    // 2. IPCD로부터의 데이터 변경
    // 3. 사용자 메모리 변경(프로그램 편의상 모든 내용을 그대로 덮어쓴다.)
// 위 2번 사항의 데이터형을 따로 만든다. 이는 변경사항 자료형이라 할 수 있다. SaveDataUpdate

extern cJSON* saveDataJSON;
extern SavedData saveData;
Result MEM_ModifyMemory(SaveDataUpdate* update)
{
    //1. 현재 데이터와 비교하여 변경이 필요없으면 변경하지 않는다.
    //2. 기존 savedata와 json에 변경을 적용한다.(1.savedataJSON 갱신 2.savedData 갱신)
    //3. FLASH에 쓰고, 잘 써졌는지 확인. (실패하면.. 다시쓰고 또 실패하면.. 그대로 다시 읽는다.(savecode,JSON오두 원상복귀))
    //4. FLASH 확인하며 RAM 데이터와 FLASH 데이터를 동기화한다.

    //1. 현재 데이터와 비교하여 변경이 필요없으면 변경하지 않는다.
    if(update->apcode              == 0
    && update->idxClient           == 0
    && update->date                == 0
    && update->dist                == 0
    // && update->useyn               == 0
    && update->apip                == 0
    && update->netmask             == 0
    && update->gwip                == 0
    && update->dnsip               == 0
    && update->dns2ip              == 0
    && update->IdentifyDistance    == 0
    && update->BeaconTimeout       == 0
    && update->enableOutDetecting  == 0
    // && update->dbuglog             == 0
    && update->reset               == 0
    //&& update->isDHCP              == 0
    && update->ipcdInterval        == 0
    && update->domain              == 0
    && update->ServerURL           == 0
    && update->pgver               == 0
    && update->pgverdate           == 0
    && update->CDMURL              == 0
    && update->aes_no              == 0) 
    {
        printf("Memory Update with NO Change.\n");
        return SUCCEED;
    }
        

    //2. 기존 savedata와 json에 변경을 적용한다.(1.savedataJSON 갱신 2.savedData 갱신)
    if(update->apcode              != 0)cJSON_ReplaceItemInObject(saveDataJSON,"apcode",cJSON_CreateNumber(update->apcode));
    if(update->idxClient           != 0)cJSON_ReplaceItemInObject(saveDataJSON,"idxClient",cJSON_CreateNumber(update->idxClient));
    if(update->date                != 0)cJSON_ReplaceItemInObject(saveDataJSON,"date",cJSON_CreateString(Convert_Date2Str(*update->date)));
    if(update->dist                != 0)cJSON_ReplaceItemInObject(saveDataJSON,"dist",cJSON_CreateNumber(update->dist));
    if(update->useyn               != 0)cJSON_ReplaceItemInObject(saveDataJSON,"useyn",cJSON_CreateBool(update->useyn));
    if(update->apip                != 0)cJSON_ReplaceItemInObject(saveDataJSON,"apip",cJSON_CreateString(Convert_IP2Str(*update->apip)));
    if(update->netmask             != 0)cJSON_ReplaceItemInObject(saveDataJSON,"netmask",cJSON_CreateString(Convert_IP2Str(*update->netmask)));
    if(update->gwip                != 0)cJSON_ReplaceItemInObject(saveDataJSON,"gwip",cJSON_CreateString(Convert_IP2Str(*update->gwip)));
    if(update->dnsip               != 0)cJSON_ReplaceItemInObject(saveDataJSON,"dnsip",cJSON_CreateString(Convert_IP2Str(*update->dnsip)));
    if(update->dns2ip              != 0)cJSON_ReplaceItemInObject(saveDataJSON,"dns2ip",cJSON_CreateString(Convert_IP2Str(*update->dns2ip)));
    if(update->IdentifyDistance    != 0)cJSON_ReplaceItemInObject(saveDataJSON,"IdentifyDistance",cJSON_CreateNumber(update->IdentifyDistance));
    if(update->BeaconTimeout       != 0)cJSON_ReplaceItemInObject(saveDataJSON,"BeaconTimeout",cJSON_CreateNumber(update->BeaconTimeout));
    if(update->enableOutDetecting  != 0)cJSON_ReplaceItemInObject(saveDataJSON,"enableOutDetecting",cJSON_CreateNumber(update->enableOutDetecting));
    if(update->dbuglog             != 0)cJSON_ReplaceItemInObject(saveDataJSON,"dbuglog",cJSON_CreateBool(update->dbuglog));
    if(update->reset               != 0)cJSON_ReplaceItemInObject(saveDataJSON,"reset",cJSON_CreateNumber(update->reset));
    if(update->isDHCP              != 0)cJSON_ReplaceItemInObject(saveDataJSON,"isDHCP",cJSON_CreateBool(update->isDHCP));
    if(update->ipcdInterval        != 0)cJSON_ReplaceItemInObject(saveDataJSON,"ipcdInterval",cJSON_CreateNumber(update->ipcdInterval));
    if(update->domain              != 0)cJSON_ReplaceItemInObject(saveDataJSON,"domain",cJSON_CreateString(update->domain));
    if(update->ServerURL           != 0)cJSON_ReplaceItemInObject(saveDataJSON,"ServerURL",cJSON_CreateString(update->ServerURL));
    if(update->pgver               != 0)cJSON_ReplaceItemInObject(saveDataJSON,"pgver",cJSON_CreateString(update->pgver));
    if(update->pgverdate           != 0)cJSON_ReplaceItemInObject(saveDataJSON,"pgverdate",cJSON_CreateString(Convert_Date2Str(*update->pgverdate)));
    if(update->CDMURL              != 0)cJSON_ReplaceItemInObject(saveDataJSON,"CDMURL",cJSON_CreateString(update->CDMURL));
    if(update->aes_no              != 0){
        if(cJSON_HasObjectItem(saveDataJSON,"aes_no") == FALSE)cJSON_AddNumberToObject(saveDataJSON,"aes_no",*update->aes_no);
        else cJSON_ReplaceItemInObject(saveDataJSON,"aes_no",cJSON_CreateNumber(*update->aes_no));
    }
    
    Result res;
    //3. FLASH에 쓰고, 잘 써졌는지 확인. (실패하면.. 다시쓰고 또 실패하면.. 그대로 다시 읽는다.(savecode,JSON오두 원상복귀))
    if(Store_SaveData(saveDataJSON) == FAILED && Store_SaveData(saveDataJSON) == FAILED)
        res = FAILED;
    else res = SUCCEED;

    
    //4. RAM 데이터와 FLASH 데이터를 동기화한다.
    cJSON_Delete(saveDataJSON);
    if(Read_SaveData(&saveDataJSON) == FAILED) printf("memory.c:%d WARNING!! Read FLASH corrupted.\n",__LINE__);

    MEM_make_SavedData(&saveData,saveDataJSON);
    return res;
}




// SavedData MEM_VOID_SaveData()
// {
//     SavedData data;
//     memset(&data,0,sizeof(data));
//     return data;
// }
// BOOL isSaveDataSame(SavedData* prevData,SavedData* newData){
//     Date nullDate;
//     memset(&nullDate,0,sizeof(Date));
//
//     return (  (prevData->apcode             == newData->apcode              || newData->apcode              == 0                  )
//       && (prevData->idxClient          == newData->idxClient           || newData->idxClient           == 0                  )
//       && (Date2Int(prevData->date)     == Date2Int(newData->date)      || Date2Int(newData->date)      == Date2Int(nullDate) )
//       && (prevData->dist               == newData->dist                || newData->dist                == 0                  )
//       && (prevData->useyn              == newData->useyn               || newData->useyn               == 0                  )
//       && (prevData->apip.Val           == newData->apip.Val            || newData->apip.Val            == 0                  )
//       && (prevData->netmask.Val        == newData->netmask.Val         || newData->netmask.Val         == 0                  )
//       && (prevData->gwip.Val           == newData->gwip.Val            || newData->gwip.Val            == 0                  )
//       && (prevData->dnsip.Val          == newData->dnsip.Val           || newData->dnsip.Val           == 0                  )
//       && (prevData->dns2ip.Val         == newData->dns2ip.Val          || newData->dns2ip.Val          == 0                  )
//       && (prevData->IdentifyDistance   == newData->IdentifyDistance    || newData->IdentifyDistance    == 0                  )
//       && (prevData->BeaconTimeout      == newData->BeaconTimeout       || newData->BeaconTimeout       == 0                  )
//       && (prevData->enableOutDetecting == newData->enableOutDetecting  || newData->enableOutDetecting  == 0                  )
//       && (prevData->dbuglog            == newData->dbuglog             || newData->dbuglog             == 0                  )
//       && (prevData->reset              == newData->reset               || newData->reset               == 0                  )
//       && (prevData->isDHCP             == newData->isDHCP              || newData->isDHCP              == 0                  )
//       && (prevData->ipcdInterval       == newData->ipcdInterval        || newData->ipcdInterval        == 0                  )
//       && (prevData->domain             == newData->domain              || newData->domain              == 0                  )
//       && (prevData->ServerURL          == newData->ServerURL           || newData->ServerURL           == 0                  )
//       && (prevData->pgver              == newData->pgver               || newData->pgver               == 0                  )
//       && (Date2Int(prevData->pgverdate)== Date2Int(newData->pgverdate) || Date2Int(newData->pgverdate) == Date2Int(nullDate) )
//       && (prevData->CDMURL             == newData->CDMURL              || newData->CDMURL              == 0                  ));
// }
//
// Result ModifyMemory(cJSON* prevJSON, SavedData* modifiance)
// {
//     //1. 현재 데이터와 비교하여 변경이 필요없으면 변경하지 않는다.
//     //2. 새로운 데이터를 가공(Serialization)하고 확인한다(디버그 출력).
//     //3. FLASH에 쓰고, 잘 써졌는지 확인한다.
//     //4. 현재 데이터에 업데이트한다.
//  
//
//
//     // //1. 현재 데이터와 비교하여 변경이 필요없으면 변경하지 않는다.
//
//     // if(isSaveDataSame(prevData,newData))
//     //     return SUCCEED;//No changes were applied.
//
//     // SavedData WRtempData;
//     // //3. FLASH에 쓰고, 잘 써졌는지 확인한다.
//     // if(MEM_Store_SaveData(newData) == FALSE)
//     // {
//     //     Debug("memory.c:%d Writing new data failed.\n",__LINE__);
//     //     MEM_Store_SaveData(prevData);
//     //     return FALSE;
//     // }
//      
//     // SavedData RDtempData;
//     // MEM_Read_SaveData(&RDtempData)
//
//     // if(isSaveDataSame(tempData,newData))
//  
//     //4. 현재 데이터에 업데이트한다.
// }


void  MEM_Print(uint32_t addr,uint32_t len)
{
    extern IWDG_HandleTypeDef hiwdg;
    uint8_t buffer[16] = {0,};
    for(;len>0;)
    {
        int readlen = len>16 ? 16 : len;
        MEM_NormalRead(addr,buffer,readlen);
        HAL_Delay(1);
        HAL_IWDG_Refresh(&hiwdg);

        printf("%05X : ",addr);
        for(int j=0;j<readlen;j++)
            printf("%02X ",buffer[j]);
        printf("\n");

        addr += readlen;
        len -= readlen;
    }
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
