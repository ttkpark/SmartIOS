typedef struct{
  int apcode;//AP고유번호=idxEquip

  float dist;
  IP_ADDR apip;
  IP_ADDR gwip;
  IP_ADDR netmask;
  IP_ADDR dnsip;
  IP_ADDR dns2ip;

  float IdentifyDistance;
  int BeaconTimeout;
  int enableOutDetecting;

  BOOL dbuglog;
  BOOL dbugudp;
  BOOL isDHCP;
  int ipcdInterval;
  int resnd;

  //uint8_t Checksum;
} SavedData;

typedef struct{
  int* apcode;//AP고유번호=idxEquip

  float* dist;
  IP_ADDR* apip;
  IP_ADDR* netmask;
  IP_ADDR* gwip;
  IP_ADDR* dnsip;
  IP_ADDR* dns2ip;

  float* IdentifyDistance;
  int* BeaconTimeout;
  int* enableOutDetecting;

  BOOL* dbuglog;
  BOOL* dbugudp;
  BOOL* isDHCP;
  int* ipcdInterval;
  int* resnd;
} SaveDataUpdate;


Result SPI_Write(uint8_t *data,int len);
Result SPI_Read(uint8_t *data,int len);
Result MEM_WriteInstruction(uint8_t inst);
void SPI_Select(int state);

Result MEM_NormalRead(uint32_t addr,uint8_t *buffer,int readlen);
Result MEM_PageProgram(uint32_t pageaddr,uint8_t *data,int len);
Result MEM_EraseSector(uint32_t pageaddr);
Result MEM_EraseBlock(uint8_t inst,uint32_t pageaddr,uint8_t data,int len);
Result MEM_EraseChip();
Result MEM_WriteEnable();
Result MEM_WriteDisable();
uint8_t MEM_readStatus();
Result MEM_writeStatus(uint8_t status);
Result MEM_INIT();

void   MEM_Print(uint32_t addr,uint32_t len);
Result MEM_Program(uint32_t pageaddr,uint8_t *data,int len);

// Result Read_SaveData(SavedData* psaveData); // TODO : Change memory type
// Result Store_SaveData(char*); // TODO : Change memory type
// Result MEM_ModifyMemory(SaveDataUpdate* update); // TODO : Change memory type
// Result MEM_Store_SaveData(SavedData* saveData); // TODO : Change memory type
// void MEM_make_SavedData(SavedData* destsaveData,cJSON* saveDataJSON); // TODO : Change memory type
// BOOL isReadSaveDataFailed(); // TODO : Change memory type

Result NewMEM_ModifyMemory(SaveDataUpdate* update,SavedData* psaveData);
Result NewMEM_Read_SaveData(SavedData* pReadData);
Result NewMEM_Store_SaveData(SavedData* pStoreData);
Result NewMEM_stNver_SaveData(SavedData* pStoreData,SavedData* pReadData);


char* Convert_IP2Str(IP_ADDR ip);
IP_ADDR Convert_Str2IP(char* str);
char* Convert_Date2Str(Date date);
Date Convert_Str2Date(char* str);
uint64_t Date2Int(Date date);
Result MEM_Verify(uint32_t pageaddr,uint8_t *compareData,int len);

#define Key_Start 0x004000
#define KeyFirstVal 0x35
#define KeyLastVal 0xD5
#define SIZE_KEYS 14400

#define NORD 0x03 //Normal read
#define FRD  0x0B //fast read
#define PP   0x02 //page program - 0.2m~1ms

#define SER  0x20 //sector erase - 45m~300ms / D7h
#define BER32K 0x52 //Block 32K erase - 0.15~0.75s
#define BER64K 0xD8 //Block 64K erase - 0.3~1.5s
#define CER  0x60 // chip erase / 60h

#define WREN 0x06 //Write enable
#define WRDI 0x04 //Write disable
#define RDSR 0x05 //Status Register
#define WRSR 0x01
#define RDFR 0x48 //Function REgister
#define WRFR 0x42

#define DP   0xB9 //deep sleep
#define RDPD 0xAB //awake sleep
#define RDID 0xAB //read product indenfication
#define RSTEN 0x66 //reset enable
#define RST  0x99 //reset