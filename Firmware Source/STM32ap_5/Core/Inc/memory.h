typedef struct{
  int apcode;//AP고유번호=idxEquip
  int idxClient;

  Date date;
  float dist;
  BOOL useyn;
  IP_ADDR apip;
  IP_ADDR netmask;
  IP_ADDR gwip;
  IP_ADDR dnsip;
  IP_ADDR dns2ip;

  float IdentifyDistance;
  int BeaconTimeout;
  int enableOutDetecting;

  BOOL dbuglog;
  int reset;
  BOOL isDHCP;
  int ipcdInterval;
  char* domain; //eduwon.net
  char* ServerURL; ///api/sio/
  char* pgver;
  Date  pgverdate;
  char* CDMURL;
} SavedData;


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

Result Read_SaveData(cJSON ** pjson);
Result Store_SaveData(cJSON* savedata);
void   MEM_Print(uint32_t addr,uint32_t len);
Result MEM_Program(uint32_t pageaddr,uint8_t *data,int len);

Result MEM_Store_SaveData();
Result MEM_Read_SaveData();
char* Convert_IP2Str(IP_ADDR ip);
IP_ADDR Convert_Str2IP(char* str);
char* Convert_Date2Str(Date date);
Date Convert_Str2Date(char* str);
uint32_t Date2Int(Date date);

#define NORD 0x03 //Normal read
#define FRD  0x0B //fast read
#define PP   0x02 //page program - 0.2m~1m

#define SER  0x20 //sector erase - 45m~300ms / D7h
#define BER32K 0x52 //Block 32K erase - 0.15~0.75
#define BER64K 0xD8 //Block 64K erase - 0.3~1.5
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