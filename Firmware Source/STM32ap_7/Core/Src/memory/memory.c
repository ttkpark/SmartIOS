#include "main.h"
#include "JSON/tiny-json.h"
#include "ETH/eth-main.h"
#include "buffer/CoderBuffer.h"
#include "buffer/aes.h"
#include "buffer/base64.h"
#include "ETH/Internet.h"
#include "memory/memory.h"

#include "buffer/aesData.h"

//#define AES_RECOVERY

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
        watchdog();
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
        watchdog();
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

        #ifndef AES_RECOVERY
        extern BOOL isBackupNeeded;
        isBackupNeeded = TRUE;
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

Result NewMEM_ModifyMemory(SaveDataUpdate* update,SavedData* psaveData)
{
    //1. 현재 데이터와 비교하여 변경이 필요없으면 변경하지 않는다.
    //2. 기존 savedata와 json에 변경을 적용한다.(1.savedataJSON 갱신 2.savedData 갱신)
    //3. FLASH에 쓰고, 잘 써졌는지 확인. (실패하면.. 다시쓰고 또 실패하면.. 그대로 다시 읽는다.(savecode,JSON오두 원상복귀))
    //4. FLASH 확인하며 RAM 데이터와 FLASH 데이터를 동기화한다.

    //IF update->IdentifyDistance
    //IF update->BeaconTimeout
    if(psaveData->IdentifyDistance == 0){psaveData->IdentifyDistance = 100;update->IdentifyDistance = &psaveData->IdentifyDistance;}
    if(psaveData->BeaconTimeout == 0){psaveData->BeaconTimeout = 20;update->BeaconTimeout = &psaveData->BeaconTimeout;}

    //1. 현재 데이터와 비교하여 변경이 필요없으면 변경하지 않는다.
    if(psaveData == NULL || update == NULL)return FAILED;

    if(update->dist                == 0
    && update->apip                == 0
    && update->netmask             == 0
    && update->gwip                == 0
    && update->dnsip               == 0
    && update->dns2ip              == 0
    && update->IdentifyDistance    == 0
    && update->BeaconTimeout       == 0
    && update->enableOutDetecting  == 0
    && update->dbuglog             == 0
    && update->isDHCP              == 0
    && update->ipcdInterval        == 0
    && update->dbugudp             == 0
    && update->resnd               == 0
    ) 
    {
        printf("New");
        printf("Memory Update with NO Change.\n");
        return SUCCEED;
    }
        

    //2. 기존 savedata와 json에 변경을 적용한다.(1.savedataJSON 갱신 2.savedData 갱신)
    if(update->dist                != 0)psaveData->dist               = *update->dist;
    if(update->apip                != 0)psaveData->apip               = *update->apip;
    if(update->netmask             != 0)psaveData->netmask            = *update->netmask;
    if(update->gwip                != 0)psaveData->gwip               = *update->gwip;
    if(update->dnsip               != 0)psaveData->dnsip              = *update->dnsip;
    if(update->dns2ip              != 0)psaveData->dns2ip             = *update->dns2ip;
    if(update->IdentifyDistance    != 0)psaveData->IdentifyDistance   = *update->IdentifyDistance;
    if(update->BeaconTimeout       != 0)psaveData->BeaconTimeout      = *update->BeaconTimeout;
    if(update->enableOutDetecting  != 0)psaveData->enableOutDetecting = *update->enableOutDetecting;
    if(update->dbuglog             != 0)psaveData->dbuglog            = *update->dbuglog;
    if(update->isDHCP              != 0)psaveData->isDHCP             = *update->isDHCP;
    if(update->ipcdInterval        != 0)psaveData->ipcdInterval       = *update->ipcdInterval;
    if(update->dbugudp             != 0)psaveData->dbugudp            = *update->dbugudp;
    if(update->resnd               != 0)psaveData->resnd              = *update->resnd;


    //3. FLASH에 쓰고, 잘 써졌는지 확인. (실패하면.. 다시쓰고 또 실패하면.. 그대로 다시 읽는다.(savecode,JSON오두 원상복귀))
    //4. RAM 데이터와 FLASH 데이터를 동기화한다.
    
    return NewMEM_stNver_SaveData(psaveData,psaveData);
}
Result NewMEM_Read_SaveData(SavedData* pReadData){
    if(pReadData == NULL) return FAILED;
    return MEM_NormalRead(0x000000,pReadData,sizeof(SavedData));
}
Result NewMEM_Store_SaveData(SavedData* pStoreData){
    if(pStoreData == NULL) return FAILED;

    //pStoreData->Checksum = calculateChecksum8b(pStoreData,sizeof(SavedData) - sizeof(pStoreData));

    Result res = SUCCEED;

    if(res == SUCCEED)res = MEM_WriteEnable();
    if(res == SUCCEED)res = MEM_EraseSector(0x000000);
    if(res == SUCCEED)res = MEM_Program(0x000000,pStoreData,sizeof(SavedData));

    if(res == SUCCEED)res = MEM_WriteDisable();
    else MEM_WriteDisable();

    if(res == SUCCEED)res = MEM_Verify(0x000000,pStoreData,sizeof(SavedData));

    Debug("NewMEM_Store_SaveData end\n");
    return res;
}
Result NewMEM_stNver_SaveData(SavedData* pStoreData,SavedData* pReadData){
    if(pStoreData == NULL || pReadData == NULL) return FAILED;
    
    SavedData Read;
    Result res;
    for(int i=0;i<10;i++)
    {
        res = NewMEM_Store_SaveData(pStoreData);
        if(res == FAILED){Debug("NewMEM_Store_SaveData error");continue;}
        res = NewMEM_Read_SaveData(&Read);
        if(res == FAILED){Debug("NewMEM_Read_SaveData error");continue;}
        // if(calculateChecksum8b(&Read,sizeof(SavedData) - 4) != Read.Checksum)
        // {Debug("calculateChecksum8b %d != %d",calculateChecksum8b(&Read,sizeof(SavedData) - sizeof(pStoreData->Checksum)),Read.Checksum);continue;}
        break;
    }
    if(res == SUCCEED)
        memcpy(pReadData,&Read,sizeof(SavedData));
    return res;
}



void  MEM_Print(uint32_t addr,uint32_t len)
{
    extern IWDG_HandleTypeDef hiwdg;
    uint8_t buffer[16] = {0,};
    for(;len>0;)
    {
        int readlen = len>16 ? 16 : len;
        MEM_NormalRead(addr,buffer,readlen);
        watchdog();

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
uint64_t Date2Int(Date date)
{
    return ((((date.year)*(12) + (date.month-1))*31 + (date.day-1))*24 + (date.hour-1))*60 + date.minute;
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
    if(strlen(str) < 7)return ip; // more little than 7Byte of "0.0.0.0"

    ip.v[0] = atoi(str);
    for(int i=1;i<4;i++)
    {
        char* focus = strstr(str,".");
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
