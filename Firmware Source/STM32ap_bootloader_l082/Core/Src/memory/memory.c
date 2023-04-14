#include "main.h"
#include "memory/memory.h"

#define MEM_SPI SPI1
uint8_t UUID[16];
extern IWDG_HandleTypeDef hiwdg;

void SPI_Transmit(SPI_TypeDef* spi,uint8_t *data,int len)
{
    if(!LL_SPI_IsEnabled(spi)){
        LL_SPI_Enable(spi);
    }
    while(len--){
        while(!LL_SPI_IsActiveFlag_TXE(spi));
        LL_SPI_TransmitData8(spi,*(data++));
        while(!LL_SPI_IsActiveFlag_RXNE(spi));
        LL_SPI_ReceiveData8(spi);
    }
}
void SPI_Receive(SPI_TypeDef* spi,uint8_t *data,int len)
{
    if(!LL_SPI_IsEnabled(spi)){
        LL_SPI_Enable(spi);
    }
    while(len--){
        while(!LL_SPI_IsActiveFlag_TXE(spi));
        LL_SPI_TransmitData8(spi,0x00);
        while(!LL_SPI_IsActiveFlag_RXNE(spi));
        *(data++) = LL_SPI_ReceiveData8(spi);
    }
}

Result SPI_Write(uint8_t *data,int len)
{
    SPI_Transmit(MEM_SPI,data,len);
    return SUCCEED;
}
Result SPI_Read(uint8_t *data,int len)
{
    SPI_Receive(MEM_SPI,data,len);
    return SUCCEED;
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
    LL_GPIO_SetOutputPin(BLE_SS_GPIO_Port,BLE_SS_Pin);
    LL_GPIO_SetOutputPin(ENC_SS_GPIO_Port,ENC_SS_Pin);
    if(!state)LL_GPIO_SetOutputPin(MEM_CS_GPIO_Port,MEM_CS_Pin);
    else LL_GPIO_ResetOutputPin(MEM_CS_GPIO_Port,MEM_CS_Pin);
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
uint8_t MEM_ByteRead(uint32_t addr)
{
    SPI_Select(1);
    uint8_t inst[4] = {NORD,(addr>>16)&0xFF,(addr>>8)&0xFF,addr&0xFF};
    SPI_Write(inst,4);

    if(SPI_Read(inst,1) != SUCCEED)
    {
        SPI_Select(0);
        return -1;
    }
    SPI_Select(0);
    return *inst;
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

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)(200);
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
        HAL_IWDG_Refresh(&hiwdg);
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
        if(MEM_NormalRead(pageaddr,data,written) == FAILED)return FAILED;
        if(memcmp(compareData,data,written) != 0)return FAILED;
        len -= written;
        pageaddr += written;
        compareData += written;
        HAL_IWDG_Refresh(&hiwdg);
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

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)LL_mDelay(25);
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

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)LL_mDelay(50);
    //check WIP(Status Register) byte until it becomes '0'.
        MEM_WriteDisable();
    return SUCCEED;
}

Result MEM_EraseChip()
{
    if(MEM_WriteInstruction(CER) != SUCCEED)
        return FAILED;

    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)LL_mDelay(50);
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
    while((MEM_readStatus() & 0x01/*WIP*/) == 0x01)(200);
    return SUCCEED;
}
Result MEM_INIT()
{
    printf("MEM chip Initalize\n");
    MEM_WriteEnable();
    MEM_writeStatus(0x00);
    MEM_WriteDisable();
    LL_mDelay(10);

    //Reset the device
    MEM_WriteInstruction(RSTEN);
    MEM_WriteInstruction(RST);
    LL_mDelay(10);
    
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
    printf("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n"
        ,UUID[0],UUID[1],UUID[2],UUID[3],UUID[4],UUID[5],UUID[6],UUID[7]
        ,UUID[8],UUID[9],UUID[10],UUID[11],UUID[12],UUID[13],UUID[14],UUID[15]);
    printf("MAC = 12:23:34:%02X:%02X:%02X\n"
        ,(UUID[0]+UUID[3]+UUID[6]+UUID[9]+UUID[12]+UUID[15])&0xFF,(UUID[1]+UUID[4]+UUID[7]+UUID[10]+UUID[13])&0xFF,(UUID[2]+UUID[5]+UUID[8]+UUID[11]+UUID[14]) & 0xFF);

    return SUCCEED;
}
void MEM_Print(uint32_t addr,uint32_t len)
{
    uint8_t buffer[16] = {0,};
    for(;len>0;)
    {
        int readlen = len>16 ? 16 : len;
        MEM_NormalRead(addr,buffer,readlen);
        LL_mDelay(1);

        printf("%05X : ",addr);
        for(int j=0;j<readlen;j++)
            printf("%02X ",buffer[j]);
        printf("\n");

        addr += readlen;
        len -= readlen;
    }
}