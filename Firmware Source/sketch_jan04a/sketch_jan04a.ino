/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include "EtherCard.h"
#include <IPAddress.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <sys/time.h>
#include "header.h"
#include <ArduinoJson.h>
#include <Crypto.h>
#include <SPIFFS.h>

StaticJsonDocument<2000> APConfig;
static String ServerURL = "";//"/api/att/";
static String APName = "";
static String APSerialNumber = "";//"52120e85-2876-4a1a-9cf8-67fc73fbc081";//"AP000001";
static String http = "";
static String host = "";//"www.eduwon.net";
static String hostip = "";
static int port = 0;//8807;
double IncircleDistance = 0;//3.00;
double IdentifyDistance = 0;//30.00;
double Timeout = 0;//20.0;
static int idxClient = 0;
static byte myip[4] = { 0, };// = { 192, 168, 0, 31};
static byte netmask[4] = { 0, };// = { 255, 255, 255, 0 };
static byte gwip[4] = { 0, };// = { 192, 168, 0, 1 };
static byte dnsip[4] = { 0, };
static byte dns2ip[4] = { 0, };
static byte mymac[] = { 0xd0 ,0x3a ,0x42 ,0x8f ,0x18 ,0xd6 };
int useYN = 0;
int enableOutDetecting = 0;
uint8_t isDHCP = 0;
uint8_t Mode = 0;
int ipcdInterval = 0;

int Error = 0;
int vError = 0;
time_t LastErrorTime = 0;

uint8_t IPChangeFlag = 0;
static String CDMURL = "";//"/api/cdm/";
uint8_t CDMFlag = 0;
uint8_t CDMTimerFlag = 0;
time_t CDMTimer = 0;
String CDMDomain = "";

int WaitingforSending = 0;

time_t StartTime = 0;

unsigned int localPort = 8888; // local port to listen for UDP packets
char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server


byte APSerialNumberBuf[16] = {0,};
// ethernet mac address - must be unique on your network


BLEScan* pBLEScan = 0;
bool BLEOnFlag = false;
byte Ethernet::buffer[800]; // tcp/ip send and receive buffer
Stash stash;
static byte session;
word Ethergetlen,Ethergetpos;

//IPAddress timeServer;
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
int TimeSynksuccess = 0;

//uint8_t mac[] = { 0x55, 0xeb, 0x7a, 0xa1, 0x91, 0xe0 };
//int scanTime = 0; //In seconds
char isOnline = 0;
int reset = 0;
String DebugerRcvStr = "";
volatile int Debug = 1;


//static int getIntArg(const char* data, const char* key, int value =-1) {
//    char temp[10];
//    if (ether.findKeyVal(data + 7, temp, sizeof temp, key) > 0)
//        value = atoi(temp);
//    return value;
//}
//
//
//static void configPage(const char* data, BufferFiller& buf) {
////    // pick up submitted data, if present
////    if (data[6] == '?') {
////        byte b = getIntArg(data, "b");
////        byte g = getIntArg(data, "g");
////        byte c = getIntArg(data, "c", 0);
////        word r = getIntArg(data, "r");
////        if (1 <= g && g <= 250 && 1 <= r && r <= 3600) {
////            // redirect to the home page
////            buf.emit_p(PSTR(
////                "HTTP/1.0 302 found\r\n"
////                "Location: /\r\n"
////                "\r\n"));
////            return;
////        }
////    }
////    // else show a configuration form
////    buf.emit_p(PSTR("$F\r\n"
////        "<h3>Server node configuration</h3>"
////        "<form>"
////          "<p>"
////    "Freq band <input type=text name=b value='1' size=1> (4, 8, or 9)<br>"
////    "Net group <input type=text name=g value='1' size=3> (1..250)<br>"
////    "Collect mode: <input type=checkbox name=c value='1' CHECKED> "
////        "Don't send ACKs<br><br>"
////    "Refresh rate <input type=text name=r value='3' size=4> (1..3600 seconds)"
////          "</p>"
////          "<input type=submit value=Set>"
////        "</form>"), okHeader);
//}

int SaveError(String errmsg){
  time_t t;
  struct tm *lt;
  struct timeval tv;
  
  if((t = gettimeofday(&tv, NULL)) == -1) {
    if(Debug)Serial.println("gettimeofday() call error");
    return 0;
  }
  if((lt = localtime(&tv.tv_sec)) == NULL)
  {
    if(Debug)Serial.println("localtime() call error");
    return 0;
  }
  
  if (!SPIFFS.begin(true)) {
    if(Debug)Serial.println("# ERROR: can not mount SPIFFS");
    while (1) ;
  }
  
  File f = SPIFFS.open("/Debug.txt", "a");
  if (!f) {  // can not open file
    if(Debug)Serial.println("# ERROR: can not open file");
    return 0;
  }
  
  if(Debug)f.printf("Error:%d  %04d-%02d-%02d %02d:%02d:%02d.%06d Msg:%s\r\n"
  , Error
  , lt->tm_year + 1900
  , lt->tm_mon + 1
  , lt->tm_mday
  , lt->tm_hour
  , lt->tm_min
  , lt->tm_sec
  , tv.tv_usec, errmsg.c_str());
  // Close the file
  f.close();
  
  return 1;
}
int LoadError(){
  if(Debug)Serial.println("LoadError()");
  if (!SPIFFS.begin(true)) {
    if(Debug)Serial.println("# ERROR: can not mount SPIFFS");
    while (1) ;
  }
  
  File f = SPIFFS.open("/Debug.txt", "r");
  File _f = SPIFFS.open("/_Debug.txt", "w");
  if (!f) {  // can not open file
    if(Debug)Serial.println("# ERROR(f): can not open file");
    return 0;
  }
  if (!_f) {  // can not open file
    if(Debug)Serial.println("# ERROR(_f): can not open file");
    return 0;
  }

  Serial.println("====== Reading from SPIFFS file =======");
  int lineNum=0;
  for (lineNum=1; f.available(); lineNum++){
    String s=f.readStringUntil('\n');
    Serial.print((String)lineNum + "\t: ");
    Serial.println(s + "\n");
    _f.println(s.substring(0,s.length()-2));
    yield();
  }
  f.close();
  _f.close();
  if(lineNum > 3){
    _f = SPIFFS.open("/_Debug.txt", "r");
    File fw = SPIFFS.open("/Debug.txt", "w");
    for (int i=1; _f.available(); i++){
      if(i>=lineNum-2){
        String s = _f.readStringUntil('\n');
        fw.println(s.substring(0,s.length()-2));
        yield();
      }
    }
    _f.close();
    fw.close();
  }
  
  return 1;
}
int DelError(){
  if(Debug)Serial.println("DelError()");
  return SPIFFS.remove("/Debug.txt");
}
int SaveConfig(){
  if (!SPIFFS.begin(true)) {
    if(Debug)Serial.println("# ERROR: can not mount SPIFFS");
    while (1) ;
  }
  
  File f = SPIFFS.open("/APConfig.json", FILE_WRITE);
  if (!f) {  // can not open file
    if(Debug)Serial.println("# ERROR: can not open file");
    return 0;
  }
  if (serializeJson(APConfig, f) == 0) {
    if(Debug)Serial.println(F("Failed to write to file"));
    return 0;
  }

  // Close the file
  f.close();
  
  return 1;
}
int LoadConfig(){
  if (!SPIFFS.begin(true)) {
    if(Debug)Serial.println("# ERROR: can not mount SPIFFS");
    while (1) ;
  }
  
  File f = SPIFFS.open("/APConfig.json");
  if (!f) {  // can not open file
    if(Debug)Serial.println("# ERROR: can not open file");
    return 0;
  }

  if(deserializeJson(APConfig,f)){
    if(Debug)Serial.println("deserializeJson failed");
    return 0;
  }
  f.close();
  return 1;
}

void scanCompleteCB(BLEScanResults mBLE){
  
}


typedef struct SensorList{
//  byte UUIDList[16];
  char PhoneNum[16];
  signed char RSSI;
//  signed char lastRSSI;
//  signed char secRSST;
  signed char TxPower;
  uint32_t lastTime;
  uint32_t lastinCircleTime;
  bool IsEmpty;
//  bool IsIn3Meter;
  bool IsEnter;
  
  byte IsDataReady;//Flag that need to be send to the server
  struct timeval SendTime;
}SensorList;


void NTPUDPCallbackFunc(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len){
  
  if(NTP_PACKET_SIZE > len)if(Debug)Serial.printf("[Error] len : %d\r\n", len);
  // We've received a packet, read the data from it
  //  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  memcpy(packetBuffer, data, NTP_PACKET_SIZE);

  // the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, extract the two words:

  uint32_t secsSince1900  = (uint32_t) packetBuffer[40] << 24
             | (uint32_t) packetBuffer[41] << 16
             | (uint32_t) packetBuffer[42] <<  8
             | (uint32_t) packetBuffer[43] <<  0;
             
  uint32_t frac  = (uint32_t) packetBuffer[44] << 24
             | (uint32_t) packetBuffer[45] << 16
             | (uint32_t) packetBuffer[46] <<  8
             | (uint32_t) packetBuffer[47] <<  0;
  uint32_t ussec = frac*0.23283064365386962890625/1000.0;
  
  
  // this is NTP time (seconds since Jan 1 1900):
  
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  if(Debug)Serial.print("Unix time = ");
  if(Debug)Serial.print(secsSince1900 - 2208988800UL);
  if(Debug)Serial.printf(".%06d\r\n",ussec);
  
  timeval epoch = {secsSince1900 - 2208988800UL + 32400, ussec};//UTC +9 Hour in Korea
  const timeval *tv = &epoch;
  timezone utc = {32400,0};
  const timezone *tz = &utc;
  settimeofday(tv, tz);
  
  TimeSynksuccess = 1;
}

uint32_t next = 0;
void SyncTimeviaUDP(){
  sendNTPpacket(timeServer); // wait to see if a reply is available
//  next = millis();TimeSynksuccess = 0;
//  while ((TimeSynksuccess==0) && (millis()-next<2000)){delay(1);}//TimeSynksuccess = udp.parsePacket();
//  if(Debug)Serial.printf("%d ms waited, TimeSynksuccess : %d\r\n", millis()-next, TimeSynksuccess);
}
time_t _mkgmtime(const struct tm *tm) 
{
    // Month-to-day offset for non-leap-years.
    static const int month_day[12] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    // Most of the calculation is easy; leap years are the main difficulty.
    int month = tm->tm_mon % 12;
    int year = tm->tm_year + tm->tm_mon / 12;
    if (month < 0) {   // Negative values % 12 are still negative.
        month += 12;
        --year;
    }

    // This is the number of Februaries since 1900.
    const int year_for_leap = (month > 1) ? year + 1 : year;

    time_t rt = tm->tm_sec                             // Seconds
        + 60 * (tm->tm_min                          // Minute = 60 seconds
        + 60 * (tm->tm_hour                         // Hour = 60 minutes
        + 24 * (month_day[month] + tm->tm_mday - 1  // Day = 24 hours
        + 365 * (year - 70)                         // Year = 365 days
        + (year_for_leap - 69) / 4                  // Every 4 years is     leap...
        - (year_for_leap - 1) / 100                 // Except centuries...
        + (year_for_leap + 299) / 400)));           // Except 400s.
    return rt < 0 ? -1 : rt;
}
time_t MakeUTCTime_t(String str)
{ 
  time_t rtn;
  int YYYY = 0; int MM = 0; int DD = 0; int hh = 0; int mi = 0; int ss = 0;
  if(sscanf(str.c_str(), "%4d-%2d-%2d %d:%d:%d",&YYYY, &MM, &DD, &hh, &mi, &ss))
  {
    struct tm st_tm;
    st_tm.tm_year = YYYY - 1900;
    st_tm.tm_mon = MM - 1;
    st_tm.tm_mday = DD;
    st_tm.tm_hour = hh;
    st_tm.tm_min = mi;
    st_tm.tm_sec = ss;

    rtn = _mkgmtime( &st_tm );
  }else rtn = 0;
  return rtn;
}

void printLocalTime(){
  time_t t;
  struct tm *lt;
  struct timeval tv;
  
  if((t = gettimeofday(&tv, NULL)) == -1) {
    if(Debug)Serial.println("gettimeofday() call error");
    return;
  }
  if((lt = localtime(&tv.tv_sec)) == NULL)
  {
    if(Debug)Serial.println("localtime() call error");
    return;
  }
  
  if(Debug)Serial.printf("지금시간: %04d-%02d-%02d %02d:%02d:%02d.%06d\r\n"
  , lt->tm_year + 1900
  , lt->tm_mon + 1
  , lt->tm_mday
  , lt->tm_hour
  , lt->tm_min
  , lt->tm_sec
  , tv.tv_usec);
}

time_t unixtime(){
  time_t t;
  struct timeval tv;
  
  if((t = gettimeofday(&tv, NULL)) == -1) {
    if(Debug)Serial.println("gettimeofday() call error");
    return -1;
  }
  return tv.tv_sec;
}


void sendNTPpacket(char* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  
  
  
  if (!ether.dnsLookup(timeServer))
    if(Debug)Serial.println("DNS failed");

  ether.printIp("SRV: ", ether.hisip);
    
  //static void sendUdp (char *data,uint8_t len,uint16_t sport, uint8_t *dip, uint16_t dport);
  ether.sendUdp((char*)packetBuffer, NTP_PACKET_SIZE, localPort, ether.hisip, 123 );
}


//callback that prints received packets to the serial port
void udpSerialPrint(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len){
  IPAddress src(src_ip[0],src_ip[1],src_ip[2],src_ip[3]);

  if(Debug)Serial.print("dest_port: ");
  if(Debug)Serial.println(dest_port);
  if(Debug)Serial.print("src_port: ");
  if(Debug)Serial.println(src_port);


  if(Debug)Serial.print("src_port: ");
  ether.printIp(src_ip);
  if(Debug)Serial.println("data: ");
  if(Debug)Serial.println(data);
}

void ReservationSending2Server(struct SensorList& sensorList){
  struct timeval tv;
  if(gettimeofday(&tv, NULL) == -1) {if(Debug)Serial.println("gettimeofday() call error");return;}
  
  sensorList.SendTime = tv;
}
char IsInEnter(struct SensorList list){
  return (getDistance(list.RSSI, list.TxPower) <= IncircleDistance);
}
char IsFirstEnter(struct SensorList list){
  return !list.IsEnter;
}
int SensorLen = 0;
SensorList *Lists = 0;// = (SensorList *)malloc(sizeof(SensorList));
bool InterruptingFlag = false;
void ProcessingData(BLEAdvertisedDevice advertisedDevice, int index, char* PhoneNum, char* StateBuf, int mode){
//  if(mode < 2)if(Debug)Serial.printf("[%dms] The [%d] %s Sensor %s\n", millis(), index+1, PhoneNum, StateBuf);
  
  if(mode)memcpy(Lists[index].PhoneNum, PhoneNum, strlen(PhoneNum));
  
//  if(mode)Lists[index].secRSST = advertisedDevice.getRSSI();
//  else Lists[index].secRSST = Lists[index].lastRSSI;
  
  Lists[index].RSSI = advertisedDevice.getRSSI();
  Lists[index].TxPower = ((signed char *) advertisedDevice.getManufacturerData().data())[18];
  Lists[index].lastTime = millis();
  Lists[index].IsEmpty = false;
  Lists[index].IsDataReady = false;
//  if(mode==0){

  if(IsInEnter(Lists[index])){
    Lists[index].lastinCircleTime = millis();
    if(IsFirstEnter(Lists[index])){
      if(Debug)Serial.printf("[%dms] The [%d] %s Sensor is Entered!!\n", millis(), index+1, PhoneNum);
      ReservationSending2Server(Lists[index]);
      Lists[index].IsDataReady = true;
      Lists[index].IsEnter = true;
    }
  }
    
//    else if(Lists[index].IsEnter)Lists[index].IsEnter = false;
//    if(IsSendLeave(Lists[index])){
//      if(Debug)Serial.printf("[%dms] The [%d] %s Sensor is Leaved!!\n", millis(), index+1, PhoneNum);
////      Lists[index].IsDataReady = 0;
//      Lists[index].IsIn3Meter = false;
//      Lists[index].IsEmpty = true;
//    }

//  }
}
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if(TimeSynksuccess == 0)return;
    if(!BLEOnFlag)return;
    uint32_t timespends = millis();
    while(WaitingforSending)delay(1);
    if(millis() - timespends && Debug)Serial.printf("timespends : %d ", millis() - timespends);
    WaitingforSending = 0;
    
    if(TimeSynksuccess == 0)return;
    if(!BLEOnFlag)return;    
    
    InterruptingFlag = true;
    
    if(advertisedDevice.haveManufacturerData()){
      uint8_t * Manufacturer_data = (uint8_t *) advertisedDevice.getManufacturerData().data();
      if(((uint8_t*)Manufacturer_data)[0] == 0xFF && ((uint8_t*)Manufacturer_data)[1] == 0xEF){
        //if(Debug)Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
        if(Debug)Serial.printf(".");
        int length = 0;
        SetAESKeyIV((byte*)Debase64(AESKeyIV[0][0],length,2),(byte*)Debase64(AESKeyIV[0][1],length,3),1);length = 0;
        char Buf[40] = {0,};length = 16;
        char* Data = DeAES256CBC((char*)(&Manufacturer_data[2]),length,3,1);
//        PrintHex(Data,length);
//        if(Debug)Serial.printf("length: %d \n", length);
//        if(Debug)Serial.printf("Data: %s \n", Data);
        
        if(isPhoneNumber((uint8_t*)Data, length)){
          memcpy(Buf, Data, length);Buf[length] = 0;
//          if(Debug)Serial.printf("[%dms]Find a Beacon : %s ", millis(), Data);//advertisedDevice.getAddress().toString().c_str());
          if(Debug)Serial.printf("%c", Data[3]);
          
          /*
           if(advertisedDevice.haveManufacturerData()){
            if(Debug)Serial.printf("[%dms]getManufacturerData() : ", millis());
            for (uint8_t i = 0; i < advertisedDevice.getManufacturerData().length(); i++) {
                 if(Debug)Serial.printf("%02X", (int)Manufacturer_data[i]);
            }
            if(Debug)Serial.print(", ");
          }
          // */
          if(advertisedDevice.haveRSSI()){
    //        if(Debug)Serial.printf("getRSSI() : %d  %d, ", advertisedDevice.getRSSI(),((signed char *) advertisedDevice.getManufacturerData().data())[18]);
    //        if(Debug)Serial.print("Distance : ");
    //        if(Debug)Serial.print(getDistance(advertisedDevice.getRSSI(), ((signed char *)Manufacturer_data)[18]));
    //        if(Debug)Serial.print("\n");
          }
          if(getDistance(advertisedDevice.getRSSI(), ((signed char *)Manufacturer_data)[18]) < IdentifyDistance)
            Sensorisfound(advertisedDevice, Data);
    //      else Serial.println();
        }
      }
    }
    InterruptingFlag = false;
  }
};
void Sensorisfound(BLEAdvertisedDevice advertisedDevice, char* PhoneNum){
  if(!isPhoneNumber((uint8_t*)PhoneNum, strlen(PhoneNum)))return;
  //find the lastly communicated data
  //uint8_t *m_address = (uint8_t*)advertisedDevice.getAddress().getNative();
  char Buf[40] = {0,};
  memcpy(Buf, PhoneNum, strlen(PhoneNum));
//  printUUID(Buf,((uint8_t *) advertisedDevice.getManufacturerData().data() + 4));
  signed char TxPower = ((signed char *) advertisedDevice.getManufacturerData().data())[18];
  int idx=0, Isfound = -1, IsEmpty = -1;
  
//  if(advertisedDevice.haveRSSI()){
//    if(Debug)Serial.printf("getRSSI() : %d  %d, ", advertisedDevice.getRSSI(),(signed char)TxPower);
//    if(Debug)Serial.print("Distance : ");
//    if(Debug)Serial.print(getDistance(advertisedDevice.getRSSI(), (signed char)TxPower));
////    if(Debug)Serial.print("\n");
//  }
  
  for(;idx<SensorLen;idx++){
    if(Lists[idx].IsEmpty){
      if(IsEmpty == -1)IsEmpty = idx;
    }else if(memcmp(Buf, Lists[idx].PhoneNum, strlen(Buf))==0){
      Isfound = idx;
    }
  }
  if(Isfound != -1){//find the seat
    ProcessingData(advertisedDevice, Isfound, Buf, "Updated", 0);
    
  }else{// a New device appeared
    
    if(IsEmpty == -1){// a new allocating is need
      
//      if(Debug)Serial.printf("[%dms] The [%d] %s Sensor Created\n", millis(), SensorLen+1, Buf);//advertisedDevice.getAddress().toString().c_str());
      
//      if(Debug)Serial.printf("{1}PhoneNum : ");
//      for(int i=0;i<strlen(Buf);i++)if(Debug)Serial.printf("%02x", Buf[i]);
//      if(Debug)Serial.printf("\n");
      if(Debug)Serial.printf("{1}PhoneNum : %s\r\n", Buf);
      
      //memory_reAllocate((void *)Lists,SensorLen*sizeof(SensorList),sizeof(SensorList));
      SensorList *goo = (SensorList*)malloc((SensorLen+1)*sizeof(SensorList));
      memset(goo, 0, (SensorLen+1)*sizeof(SensorList));
      if(Debug)Serial.printf("Addrs//%d %d",Lists,goo);
      if(SensorLen)memcpy((char*)goo, (char*)Lists, SensorLen*sizeof(SensorList));
      free(Lists);
      Lists = goo;
      
      ProcessingData(advertisedDevice, SensorLen, Buf, "", 2);
      SensorLen++;
      
    }else{//The seat is available
      ProcessingData(advertisedDevice, IsEmpty, Buf, "Replaced", 1);
    }
  }
}


int TCPget_Send_Preparing(){
  if(Debug)Serial.println("TCPget_Send_Preparing()");
  if(Debug)Serial.println("hostip = " + hostip);
  if(hostip==""){
    if(Debug)Serial.println("Get hostip");
    uint32_t start = millis();
//    while(!ether.isLinkUp())
//    {
//        if (millis() - start >= 5000){
//          if(Debug)Serial.println("timeout waiting for link");
//          return 0;
//        }
//    }
//    while(ether.clientWaitingDns())
//    {
//        ether.packetLoop(ether.packetReceive());
//        if (millis() - start >= 5000){
//          if(Debug)Serial.println("timeout waiting for gateway ARP");
//          return 0;
//        }
//    }
    
    uint32_t timeout = millis()+5000;
    bool ServerFlag = ether.dnsLookup((const char*)(host.c_str()));// Use DHCP
    while(!ServerFlag){
      if(timeout<millis())break;
      ServerFlag = ether.dnsLookup(host.c_str());
      if(Debug)Serial.printf("ServerFlag : %d\r\n", ServerFlag);
    }
    if(ServerFlag){
      ether.printIp("Server IP : ", ether.hisip);
      hostip = (String)ether.hisip[0] + "." + (String)ether.hisip[1] + "." + (String)ether.hisip[2] + "." + (String)ether.hisip[3];
      ether.hisport = port;
      if(Debug)Serial.println("Get hostip : " + hostip);
      return 1;
    }
    if(Debug)Serial.println("DNSLookup timeout");
  }else{
    ether.parseIp(ether.hisip, hostip.c_str());
    ether.printIp("Server IP : ", ether.hisip);
    ether.hisport = port;
    if(Debug)Serial.println("Get hostip : " + hostip);
    return 1;
  }
  
  return 0;
}

String RcvData = "";
int RcvFinishFlag = 0;

uint8_t www_fd = 0;
String url = "";
// called when the client request is complete
static void my_callback (uint8_t status, uint16_t off, uint16_t len);

static uint16_t www_client_internal_datafill_cb(uint8_t fd) {
    BufferFiller bfill = EtherCard::tcpOffset();
    if (fd==www_fd) {
      bfill.emit_p(PSTR("GET $F HTTP/1.1\r\n"
                        "Host: $F\r\n"
                        "Connection: close" "\r\n"
                        "\r\n"), url.c_str(),
                   host.c_str());
    }
    return bfill.position();
}
static uint8_t www_client_internal_result_cb(uint8_t fd, uint8_t statuscode, uint16_t datapos, uint16_t len_of_data) {
    if (fd!=www_fd)
        my_callback(4,0,0);
    else if (statuscode==0 && len_of_data>12) {
        uint8_t f = strncmp("200",(char *)&(ether.buffer[datapos+9]),3) != 0;
        my_callback(f, ((uint16_t)TCP_SRC_PORT_H_P+(ether.buffer[TCP_HEADER_LEN_P]>>4)*4),len_of_data);
    }
    return 0;
}
uint32_t nextSeq;
// called when the client request is complete
static void my_callback (uint8_t status, uint16_t off, uint16_t len) {
  Serial.printf("my_callback(%d,%d,%d)\r\n", status, off, len);

  if (strncmp_P((char*) Ethernet::buffer+off,PSTR("HTTP"),4) == 0) {
    Serial.println((String)millis() + ">>>");
    // first reply packet
    nextSeq = ether.getSequenceNumber();
  }

  if (nextSeq != ether.getSequenceNumber()) { RcvFinishFlag = 1;Serial.println((String)millis() + "<IGNORE DUPLICATE(?) PACKET>"); return; }

  uint16_t payloadlength = ether.getTcpPayloadLength();
  int16_t chunk_size   = 700-off-1;
  int16_t char_written = 0;
  int16_t char_in_buf  = chunk_size < payloadlength ? chunk_size : payloadlength;

  while (char_written < payloadlength) {
//    Serial.println((String)millis() + "char_written[" + (String)char_written + "] < payloadlength[" + (String)payloadlength + "]");
    RcvData += (char*)(Ethernet::buffer + off);
//    Serial.write((const uint8_t*) Ethernet::buffer+off,char_in_buf);
    char_written += char_in_buf;

    char_in_buf = ether.readPacketSlice((char*) Ethernet::buffer+off,chunk_size,off+char_written);
  }

  nextSeq += ether.getTcpPayloadLength();
  
  Serial.printf("[%d]End\r\n",millis());
  
  if(len != 550)RcvFinishFlag = 1;
  else if(status == 1 || status == 4)RcvFinishFlag = 1;
}
String TCPget_Send_Processing(String _url){
  if(Debug)Serial.println("TCPget_Send_Processing()");
  String rtnStr = "";
  url = _url;
  www_fd = ether.clientTcpReq(&www_client_internal_result_cb,&www_client_internal_datafill_cb,ether.hisport);
  RcvFinishFlag = 0;
//  ether.browseUrl(url.c_str(), "", website, "Connection: close", my_callback);
  uint32_t timer = millis()+5000;
  while(!RcvFinishFlag){ether.packetLoop(ether.packetReceive());if(millis() > timer || ether.tcp_client_st == 6)break;}
  if(millis() > timer || ether.tcp_client_st == 6)return "";
//  PrintHex((char*)RcvData.c_str(), RcvData.length());

  String Data = RcvData;
  
  int Index = 0, index1;
  while(Index < Data.length()){
    index1 = Data.indexOf('\r', Index);
    if(index1 == -1)index1 = Data.length();
    
    String line = Data.substring(Index+2, index1-1);
    line.replace("\\\"", "\"");
    
    StaticJsonDocument<500> root;
    if(deserializeJson(root,line));//{if(Debug)Serial.println("deserializeJson failed");}
    else{
      if(Debug)Serial.println("line : "+line+"\r\n");
      rtnStr = line;
      int rtncode = root[0]["rtncode"];
      
      if(Debug)Serial.print("rtncode : ");
      if(Debug)Serial.println(rtncode);
    }
    Index = index1+1;
  }
  RcvData = "";RcvFinishFlag = 0;
  return rtnStr;
}

//SendPacket_ Send BeaconData
int SendPacket2Server_5_BeaconData(struct SensorList sensorList, int inOut = 0){
  if(Debug)Serial.printf("\r\n[%dms], SendPacket2Server_5_BeaconData() ", millis());
  if(Debug)Serial.printf("Phone Number : %s\r\n",sensorList.PhoneNum);
  if(Debug)Serial.print("connecting to ");
  if(Debug)Serial.println(host);
  if(!TCPget_Send_Preparing())return 0;
      
  char Buf[40] = {0,};
  memcpy(Buf,sensorList.PhoneNum, strlen(sensorList.PhoneNum));
  
  time_t t;
  struct timeval tv;
  struct tm *lt;
  
  
  if((t = gettimeofday(&tv, NULL)) == -1) {if(Debug)Serial.println("gettimeofday() call error");return 0;}
  if((lt = localtime(&tv.tv_sec)) == NULL) {if(Debug)Serial.println("localtime() call error");return 0;}
  
  struct timeval sentv = sensorList.SendTime;
  struct tm *senlt;
  if((senlt = localtime(&sentv.tv_sec)) == NULL) {if(Debug)Serial.println("localtime() call error");return 0;}
  char timeStringBuff[50]; //50 chars should be enough
  sprintf(timeStringBuff, "%04d%02d%02d%02d%02d%02d%03d", senlt->tm_year + 1900  , senlt->tm_mon + 1  , senlt->tm_mday  , senlt->tm_hour  , senlt->tm_min  , senlt->tm_sec  , sentv.tv_usec/1000);

  int Sendno = random(0,10);
  int aes_no = (lt->tm_mon + 1 + lt->tm_mday)/2 + Sendno - 1;
  if(Debug)Serial.printf("Sendno : %d, ", Sendno);
  if(Debug)Serial.printf("aes_no : %d, ", aes_no);
  if(Debug)Serial.printf("timeStringBuff : %s\r\n", timeStringBuff);
  int length = 0;
  SetAESKeyIV((byte*)Debase64(AESKeyIV[aes_no][0],length,0),(byte*)Debase64(AESKeyIV[aes_no][1],length,1));
  
  // We now create a URI for the request
  String url = ServerURL;
  length = 0;
  
  url += (String)random(10,100);//Enbase64(Enbase64(EnAES256CBC(timeStringBuff,length),length),length,1);length = 0;//cordify date information
  url += "/";
  url += Enbase64(Enbase64(EnAES256CBC(Buf,length),length),length,1);length = 0;//module serial number(UUID)
  url += "/";
  url += Enbase64(Enbase64(EnAES256CBC((char*)APSerialNumber.c_str(),length),length),length,1);length = 0;//AP serial number
  url += "/";
  url += "b";// + (String)"IO"[inOut];
  url += "/";
  url += String(Sendno + (idxClient == 0 ? 1000 : idxClient)*10);
  
  if(Debug)Serial.print("Requesting URL: ");
  if(Debug)Serial.println(url);
  
  String line = TCPget_Send_Processing(url);
  if(line == "")return 0;
  
  StaticJsonDocument<500> root;
  if(deserializeJson(root,(char*)line.c_str()));//{if(Debug)Serial.println("deserializeJson failed");}
  else{
    if(Debug)Serial.println(line);
    JsonObject obj = root[0]["rtncode"].to<JsonObject>();
    int rtncode = obj.isNull();
    
    // The above line is equivalent to:
    
    if(Debug)Serial.print("rtncode is null : ");
    if(Debug)Serial.println(rtncode);
    if(!rtncode)return 1;
  }

  return 0;
}

//SendPacket_ Send Echo
int SendPacket2Server_1_3_Echo(){
  if(Debug)Serial.printf("\r\n[%dms], SendPacket2Server_1_3_Echo()\r\n", millis());
  if(Debug)Serial.print("connecting to ");
  if(Debug)Serial.println(host);
  if(!TCPget_Send_Preparing())return 0;
  
  // We now create a URI for the request
  String url = ServerURL;
  
  url += "echo_";
  url += String(random(1000,10000));
  
  if(Debug)Serial.print("Requesting URL: ");
  if(Debug)Serial.println(url);
  
  String line = TCPget_Send_Processing(url);
  
  StaticJsonDocument<500> root;
  
  if(deserializeJson(root,line));//{if(Debug)Serial.println("deserializeJson failed");}
  else{
    if(Debug)Serial.println("line : "+line+"\r\n");
    JsonObject obj = root[0]["rtncode"].to<JsonObject>();
    int rtncode = obj.isNull();
    
    // The above line is equivalent to:
    
    if(Debug)Serial.print("rtncode is null : ");
    if(Debug)Serial.println(rtncode);
    if(!rtncode)return 1;
  }

  return 0;
}

//SendPacket_ Send AP Register
int SendPacket2Server_1_APRegister(){
  if(Debug)Serial.printf("\r\n[%dms], SendPacket2Server_1_APRegister()\r\n", millis());
  if(Debug)Serial.print("connecting to ");
  if(Debug)Serial.println(host);
  if(!TCPget_Send_Preparing())return 0;
  
  time_t t;
  struct timeval tv;
  struct tm *lt;
  
  if((t = gettimeofday(&tv, NULL)) == -1) {if(Debug)Serial.println("gettimeofday() call error");return 0;}
  if((lt = localtime(&tv.tv_sec)) == NULL) {if(Debug)Serial.println("localtime() call error");return 0;}
  
  int Sendno = random(0,10);
  int aes_no = (lt->tm_mon + 1 + lt->tm_mday)/2 + Sendno - 1;
  if(Debug)Serial.printf("Sendno : %d\r\n", Sendno);
  if(Debug)Serial.printf("aes_no : %d\r\n", aes_no);
  int length = 0;
  SetAESKeyIV((byte*)Debase64(AESKeyIV[aes_no][0],length,0),(byte*)Debase64(AESKeyIV[aes_no][1],length,1));
  
  // We now create a URI for the request
  String url = ServerURL;
  length = 0;
  
  url += Enbase64(Enbase64(EnAES256CBC("10",length),length),length,1);length = 0;//cordify date information
  url += "/";
  url += Enbase64(Enbase64(EnAES256CBC((char*)APSerialNumber.c_str(),length),length),length,1);length = 0;//AP serial number
  url += "/";
  url += "bco";
  url += "/";
  url += String(IncircleDistance);
  url += "/";
  url += String(Sendno + (idxClient == 0 ? 1000 : idxClient)*10);
  url += "/";
  url += "in";
  
  if(Debug)Serial.print("Requesting URL: ");
  if(Debug)Serial.println(url);
  
  String line = TCPget_Send_Processing(url);
  
  StaticJsonDocument<500> root;
  if(deserializeJson(root,(char*)line.c_str()));//{if(Debug)Serial.println("deserializeJson failed");}
  else{
    if(Debug)Serial.println(line);
    int rtncode = root[0]["rtncode"];
    
    if(Debug)Serial.print("rtncode : ");
    if(Debug)Serial.println(rtncode);
    if(rtncode == 1){
      idxClient = ((String)(const char*)root[0]["cc"]).toInt();
      if(Debug)Serial.print("idxClient : ");
      if(Debug)Serial.println(idxClient);
      return 1;
    }
  }

  return 0;
}

//SendPacket_ [ipcd : 1] Send Synchronize Config
//SendPacket_ [ipcd : 0] Send if update failed
StaticJsonDocument<1500> rtnipcd;
//ipcd 
// 0 : ipc0, 1 : ipcd, 2: ipc3
int SendPacket2Server_3_SyncConfig(int ipcd){
  if(Debug)Serial.printf("\r\n[%dms], SendPacket2Server_3_SyncConfig(%s)\r\n", millis(),"ipc" + (String)("0d3"[ipcd]));
  if(Debug)Serial.print("connecting to ");
  if(Debug)Serial.println(host);
  if(!TCPget_Send_Preparing())return 0;
  
  time_t t;
  struct timeval tv;
  struct tm *lt;
  
  if((t = gettimeofday(&tv, NULL)) == -1) {if(Debug)Serial.println("gettimeofday() call error");return 0;}
  if((lt = localtime(&tv.tv_sec)) == NULL) {if(Debug)Serial.println("localtime() call error");return 0;}
  
  int Sendno = random(0,10);
  int aes_no = (lt->tm_mon + 1 + lt->tm_mday)/2 + Sendno - 1;
  if(Debug)Serial.printf("Sendno : %d\r\n", Sendno);
  if(Debug)Serial.printf("aes_no : %d\r\n", aes_no);
  int length = 0;
  SetAESKeyIV((byte*)Debase64(AESKeyIV[aes_no][0],length,0),(byte*)Debase64(AESKeyIV[aes_no][1],length,1));
  
  // We now create a URI for the request
  String url = ServerURL;
  length = 0;
  
  url += Enbase64(Enbase64(EnAES256CBC((char*)APSerialNumber.c_str(),length),length),length,1);length = 0;//AP serial number
  url += "/";
  if(ipcd == 0){//ipc0
    uint8_t ApipBundle[24] = {0,};int length = 0;
    memcpy(ApipBundle, myip, 4);
    memcpy(ApipBundle + 4, gwip, 4);
    memcpy( ApipBundle + 8, netmask,4);
    memcpy(ApipBundle + 12, dnsip, 4);
    memcpy(ApipBundle + 16, dns2ip, 4);
    PrintHex((char*)ApipBundle,20);length = 20;    
    url += Enbase64(Enbase64(EnAES256CBC(Enbase64((char*)ApipBundle,length),length),length),length,1);length = 0;//AP serial number
    url += "/";
  }
  
  url += String(Sendno + (idxClient == 0 ? 1000 : idxClient)*10);
  url += "/";
  url += "ipc" + (String)("0d3"[ipcd]);
  
  if(Debug)Serial.print("Requesting URL: ");
  if(Debug)Serial.println(url);
  
  String line = TCPget_Send_Processing(url);
  
//  int Sendno = 0;
//  int aes_no = 4;
//  int length = 0;
//  SetAESKeyIV((byte*)Debase64(AESKeyIV[aes_no][0],length,0),(byte*)Debase64(AESKeyIV[aes_no][1],length,1));length = 0;
//  String line = "[{\"rtncode\" : \"01\",\"apname\" : \"...\",\"apcode\" : \"VzZwYWhkTGtPdmNPNmFVenRTUkpYa01XMTVST1pvSFpyMEUySTh0dW1vWUVVRkVHRjh0SkVFV2VIY0VRR1plYw==\""
//  ", \"apip\" : \"ZytsRVJFMVYrempzVTFWWFpHOFhWV3lBTFVMZzM4elo5ZE1hTTdQa3Blcz0=\",\"dist\" : \"5.5\",\"useyn\" : \"Y\" "
//  ", \"domain\" : \"RmhzaHN2THJZTERJcWRxaktCd0N5ckVSemhBMXc5alQ3OHpMRDhoVGFzOD0=\"}]";
  
  StaticJsonDocument<1500> root;
  if(deserializeJson(root,(char*)line.c_str()));//{if(Debug)Serial.println("deserializeJson failed");}
  else{
//    if(Debug)Serial.println(line);
    int rtncode = root[0]["rtncode"];
    if(Debug)Serial.print("rtncode : ");
    if(Debug)Serial.println(rtncode);

    if(ipcd == 1){
      if(rtncode == 1){
//        serializeJsonPretty(root, Serial);
//        
//        if(Debug)Serial.println("APConfig : ");
//        serializeJsonPretty(APConfig, Serial);
        SaveConfig();
        BLEStart(0);
        if(Debug)Serial.println("data should be updated");
        
        if(Debug)Serial.println("!root[0][\"apcode\"].isNull() : " + (String)(!root[0]["apcode"].isNull()));
        if(Debug)Serial.println("!root[0][\"apip\"].isNull() : " + (String)(!root[0]["apip"].isNull()));
//        if(Debug)Serial.println("!root[0][\"domain\"].isNull() : " + (String)(!root[0]["domain"].isNull()));
        
//        if(Debug)Serial.print("rtnipcd[0][\"apcode\"] : ");
//        if(Debug)Serial.print(rtnipcd[0]["apcode"].as<const char*>());
//        if(Debug)Serial.println("");
        if(!root[0]["apcode"].isNull()){
          root[0]["apcode"] = (const char*)DeAES256CBC(Debase64(Debase64((char*)root[0]["apcode"].as<const char*>(),length,1),length),length);length = 0;
        }
//        memcpy(ApipBundle,(const char*)DeAES256CBC(Debase64(Debase64((char*)root[0]["apip"].as<const char*>(),length,1),length),length), 16);length = 0;
        
        if(!root[0]["apip"].isNull()){
          root[0]["apip"] = DeAES256CBC(Debase64(Debase64((char*)root[0]["apip"].as<const char*>(),length,1),length),length,1);length = 0;
        
//          if(Debug)Serial.println("root[0][\"apip\"] : ");
//          PrintHex((char*)root[0]["apip"].as<const char*>(), length);length = 0;
        }
        if(!root[0]["domain"].isNull()){
          root[0]["domain"] = (const char*)DeAES256CBC(Debase64(Debase64((char*)root[0]["domain"].as<const char*>(),length,1),length),length,2);length = 0;
        }
        
        
//        if(Debug)Serial.println("data should be updated");
//        root[0]["apcode"] = DeAES256CBC(Debase64(Debase64((char*)root[0]["apcode"],length),length),length,1);length = 0;
        
        String rootStr = "";
        rtnipcd = root;
//        serializeJson(root, rootStr);
//        deserializeJson(rtnipcd, rootStr);
        
        return 2;
      }
      if(rtncode == 16)return 1;
    }else{
      if(rtncode == 1)return 1;
    }
  }

  return 0;
}

//dcm
// 0 : ipc0, 1 : ipcd, 2: ipc3
int SendPacket2Server_0_DomainChanged(){
  if(Debug)Serial.printf("\r\n[%dms], SendPacket2Server_0_DomainChanged()\r\n", millis());
  if(Debug)Serial.print("connecting to ");
  if(Debug)Serial.println(host);
  if(!TCPget_Send_Preparing())return 0;
  
  time_t t;
  struct timeval tv;
  struct tm *lt;
  
  if((t = gettimeofday(&tv, NULL)) == -1) {if(Debug)Serial.println("gettimeofday() call error");return 0;}
  if((lt = localtime(&tv.tv_sec)) == NULL) {if(Debug)Serial.println("localtime() call error");return 0;}
  
  int Sendno = random(0,10);
  int aes_no = (lt->tm_mon + 1 + lt->tm_mday)/2 + Sendno - 1;
  if(Debug)Serial.printf("Sendno : %d\r\n", Sendno);
  if(Debug)Serial.printf("aes_no : %d\r\n", aes_no);
  int length = 0;
  SetAESKeyIV((byte*)Debase64(AESKeyIV[aes_no][0],length,0),(byte*)Debase64(AESKeyIV[aes_no][1],length,1));
  
  // We now create a URI for the request
  String clientnum = (String)idxClient;
  String url = CDMURL;
  length = 0;
  url += Enbase64(Enbase64(EnAES256CBC((char*)clientnum.c_str(),length),length),length,1);length = 0;//AP serial number
  url += "/";
  url += String(Sendno + random(1000,10000)*10);//(idxClient == 0 ? 1000 : idxClient)
  url += "/";
  url += "dm";
  
  if(Debug)Serial.print("Requesting URL: ");
  if(Debug)Serial.println(url);
  
  String line = TCPget_Send_Processing(url);
  
  StaticJsonDocument<1500> root;
  if(deserializeJson(root,(char*)line.c_str()));//{if(Debug)Serial.println("deserializeJson failed");}
  else{
//    if(Debug)Serial.println(line);
    if(root[0]["rtncode"].isNull())
    {
      char *dtstr = 0, *dmstr = 0;
      
      if(Debug)Serial.println("data should be updated");
      if(Debug)Serial.printf("(root.as<JsonArray>()).size() : %d\r\n", (root.as<JsonArray>()).size());

      if(!root[0]["dm"].isNull()){
        root[0]["dm"] = (const char*)DeAES256CBC(Debase64(Debase64((char*)root[0]["dm"].as<const char*>(),length,1),length),length,1);length = 0;
      }
      if(Debug)Serial.printf("dm : %s\r\n", (char*)root[0]["dm"].as<const char*>());
      
      if((root.as<JsonArray>()).size() == 2){
        if(!root[1]["dm"].isNull()){
          root[1]["dm"] = (const char*)DeAES256CBC(Debase64(Debase64((char*)root[1]["dm"].as<const char*>(),length,1),length),length,2);length = 0;
        }
        if(Debug)Serial.println("!root[1][\"dm\"].isNull() : " + (String)(!root[1]["dm"].isNull()));
        if(Debug)Serial.println("!root[1][\"dt\"].isNull() : " + (String)(!root[1]["dt"].isNull()));
        
        rtnipcd = root;
        return 2;
      }else if(root.as<JsonArray>().size() == 1){
        rtnipcd = root;
        return 1;
      }else{
        if(Debug)Serial.println("error occured : Return Json Data isn't exist");
        rtnipcd = root;
        return 0;
      }
    }else{
      if(Debug)Serial.print("rtncode : ");
      if(Debug)Serial.println((const char*)root[0]["rtncode"]);
      rtnipcd = root;
    }
  }

  return 0;
}

uint32_t mainTime = 0, mainTime1 = 0;
time_t mainTime2 = 0, mainTime3 = 0;
uint32_t InterruptingTime = 0;
uint8_t vOnlinefy = 0, vSync = 0, vService = 0, vChanges = 0, PacketRetry = 0;
int IPChanged = 0, SrvDomainChanged = 0, DataChanged = 0, IPstrChangedFlag = 0;

int StaticTry(){
  int timeout = 0;
  for(PacketRetry = 0;PacketRetry<3;PacketRetry++){ // keep trying until you succeed
    Serial.println("staticSetup()");
    ether.staticSetup(myip, gwip, dnsip, netmask);

    
    if (!ether.dnsLookup(host.c_str())){
      Serial.println("DNS failed");
      continue;
    }
    ether.printIp("SRV: ", ether.hisip);
    hostip = (String)ether.hisip[0] + "." + (String)ether.hisip[1] + "." + (String)ether.hisip[2] + "." + (String)ether.hisip[3];
    ether.hisport = port;
    
//    timeout = millis()+5000;
//    while (ether.clientWaitingGw() && timeout>millis())
//      ether.packetLoop(ether.packetReceive());
    
//    if(timeout<millis())continue;
    
    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
    
    break;
  }
  if(PacketRetry==3)return 0;
  else return 1;
}
int DHCPTry(){
  for(PacketRetry = 0;PacketRetry<3;PacketRetry++){ // keep trying until you succeed
    Serial.println("dhcpSetup()");
    if (!ether.dhcpSetup()){
      Serial.println("DHCP failed");
      continue;
    }
    
    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
    break;
  }
  if(PacketRetry==3)return 0;
  else return 1;
}

// Onlinefy Task
// 1. try DHCP
// 2. If can't, try local default IP
// --------------------------------------(X) 3. If can't, try ping to 254 IP
// 4. If can't, wait for manually setting
int MakeOnline(){
  if(IPChangeFlag==1 || IPChangeFlag == 2){
    
//    isOnline = 1;
//    for(int i=0;i<2 && isOnline;i++){
//      if(Debug)Serial.println("try factory-satting Static IP..");
//      ether.staticSetup(myip, gwip, dnsip, netmask);
//      unsigned long timeout = millis();
//      while((isOnline = ether.clientWaitingGw()) == 1) {
//        ether.packetLoop(ether.packetReceive());
//        if (millis() - timeout > 3000){
//          if(Debug)Serial.println(">>> Gateway Timeout !");
//          break;
//        }
//      }
//    }

    if(StaticTry()){
      if(Debug)Serial.println("Static IP getting successful");
      isOnline = 1;Mode = 2;Serial.println("Mode=2;");return 1;
    }
    else {
      if(Debug)Serial.println("Static IP getting failed...");
      isOnline = 0;Mode = 8;Serial.println("Mode=8;");return 0;
    }
  }
  if(reset != 0){
    if(isDHCP){
      if(Debug)Serial.println("try DHCP..");
//      isOnline = ether.dhcpSetup();// Use DHCP
//      for(int i=0;i<2 && !isOnline;i++){
//        if(Debug)Serial.printf("Failed to get IP via DHCP.. Retry %d\r\n", i+1);
//        isOnline = ether.dhcpSetup();
//      }
//      if(isOnline){if(Debug)Serial.println("DHCP Succeed!");Mode = 2;return 1;}
//      if(Debug)Serial.println("DHCP Failed...");
      if(DHCPTry()){
        if(Debug)Serial.println("DHCP Succeed!");
        isOnline = 1;Mode = 2;Serial.println("Mode=2;");return 1;
      }else{
        if(Debug)Serial.println("DHCP Failed...");
        SaveError("DHCP Failed Error(Mode : 0), Line : " + (String)__LINE__);
        Error++;
        isOnline = 0;Mode = 0;Serial.println("Mode=0;");return 0;
      }
    }else{
//      isOnline = 1;
      
//      for(int i=0;i<5 && isOnline;i++){
//        if(Debug)Serial.println("try factory-satting Static IP..");
//        ether.staticSetup(myip, gwip, dnsip, netmask);
//        unsigned long timeout = millis()+5000;
//        while(isOnline = ether.clientWaitingGw()) {
//          ether.packetLoop(ether.packetReceive());
//          if (millis() > timeout){
//            if(Debug)Serial.println(">>> Gateway Timeout !");
//            break;
//          }
//        }
//      }
//      if(!isOnline){if(Debug)Serial.println("Gateway found");isOnline = 1;Mode = 2;return 1;}
//      else isOnline = 0;

      if(StaticTry()){
        if(Debug)Serial.println("Static IP getting successful");
        isOnline = 1;Mode = 2;Serial.println("Mode=2;");return 1;
      }else {
        if(Debug)Serial.println("Static IP getting failed...");
        SaveError("Static IP getting Failed Error(Mode : 0) - isDHCP turned on!, Line : " + (String)__LINE__);
        Error++;
        LoadConfig();
        isDHCP = 1;
        APConfig["isDHCP"] = isDHCP;
        APConfig["reset"] = 1;
        if(Debug)Serial.println("isDHCP = "+(String)isDHCP);
        if(Debug)Serial.println("reset = 1");
        SaveConfig();
        ESP.restart();
//        isOnline = 0;Mode = 0;Serial.println("Mode=0;");return 0;
      }
    }
  }
}
int doSetupOnlinefy(){
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  int res = 0;
  for(PacketRetry = 0;PacketRetry<3;PacketRetry++){ // keep trying until you succeed
    //Reinitialize ethernet module
    Serial.println("Reseting Ethernet...");
    digitalWrite(4, LOW);
    delay(1000);
    digitalWrite(4, HIGH);
    delay(500);

    // Change 'SS' to your Slave Select pin, if you arn't using the default pin
    if (ether.begin(sizeof Ethernet::buffer, mymac, 15) == 0){
      Serial.println( "Failed to access Ethernet controller");
      continue;
    }
    break;
  }
  if(PacketRetry == 3){//If failed(exceed timeout try)
    Serial.println("Reseting Ethernet failed... \r\nPlease connect Ethernet cable.");
    Mode = 0;Serial.println("Mode=0;");return 0;
  }
  Mode = 1;Serial.println("Mode=1;");
  pinMode(4, INPUT);
  ether.persistTcpConnection(true);
  if(reset == 0){Mode = 0;Serial.println("Mode=0;");}
  return 1;
}

void doOnlinefy(){
  if(Mode == 1 || (isOnline == 0 && Mode != 0)){
    MakeOnline();
    
    ether.printIp("IP address  :  ", ether.myip);
    ether.printIp("Netmask     :  ", ether.netmask);
    ether.printIp("broadcastip :  ", ether.broadcastip);
    ether.printIp("Gateway IP  :  ", ether.gwip);
    ether.printIp("DHCP server IP :  ", ether.dhcpip);
    ether.printIp("DNS server IP  :  ", ether.dnsip);

    for(int i=0;i<6;i++){mymac[i] = ether.mymac[i];}
    for(int i=0;i<4;i++){myip[i] = ether.myip[i];}
    for(int i=0;i<4;i++){netmask[i] = ether.netmask[i];}
    for(int i=0;i<4;i++){gwip[i] = ether.gwip[i];}
    for(int i=0;i<4;i++){dnsip[i] = ether.dnsip[i];}
    
    if(Debug)Serial.printf("isOnline : %d\r\n", isOnline);
    
    PacketRetry = 0;
  }
  if(Mode == 2){
    if(isOnline == 1){
      if(TCPget_Send_Preparing()){//Searching host's IP in Domain Table
        PacketRetry = 0;
        isOnline = 2;
        if(Debug)Serial.println("Server found : " + host);
        if(Debug)Serial.println("Server hostip : " + hostip);
        ether.printIp("Server IP : ", ether.hisip);
      }else {//Failed in searching host's IP
        PacketRetry = 0;isOnline = 0;
        if(IPChangeFlag == 1 || IPChangeFlag == 2){Mode = 8;Serial.println("Mode=8;");}//IP change Failed!
        else {Mode = 0;Serial.println("Mode=0;");}//boot Failed!
      }
//      char* hostname = (char*)host.c_str();
//      if(Debug)Serial.printf("hostname : %s\r\n", hostname);
//      bool ServerFlag = ether.dnsLookup(hostname);// Use DHCP
//      uint32_t timeout = millis()+1000; int retry = 0;
//      if(Debug)Serial.printf("ServerFlag : %d\r\n", ServerFlag);
//      while(!ServerFlag){
//        if(timeout<millis() || retry++ > 10)break;
//        ServerFlag = ether.dnsLookup(host.c_str());
//        if(Debug)Serial.printf("Failed : ServerFlag : %d\r\n", ServerFlag);
//      }
//      if(timeout<millis()){
//        PacketRetry = 0;
//        if(IPChangeFlag){isOnline = 0;Mode = 8;}else {isOnline = 0;Mode = 0;}
//      }
//      else {
//        PacketRetry = 0;
//        isOnline = 2;
//        if(Debug)Serial.println("Server found : " + host);
//        ether.printIp("Server IP : ", ether.hisip);
//      }
    }
    if(isOnline == 2){
      WaitingforSending = 1;
      int rtn = SendPacket2Server_1_3_Echo();
      WaitingforSending = 0;
      if(rtn){
        Mode = 3;Serial.println("Mode=3;");
        if(Debug)Serial.println("Echo Succeed!");
        PacketRetry = 0;
        if(IPChangeFlag==1 || IPChangeFlag == 3){
          Mode = 7;Serial.println("Mode=7;");//IP Change Success
        }else if(IPChangeFlag==2){
          Mode = 9;Serial.println("Mode=9;");//Return IP Success
        }
      }
      else{
        if(PacketRetry++ > 5){
          if(IPChangeFlag == 1 || IPChangeFlag == 2 || IPChangeFlag == 3){
            Mode = 8;Serial.println("Mode=8;");//IP Change Failed
          }else {Mode = 0;Serial.println("Mode=0;");}//IP Change Failed
        }
        if(Debug)Serial.println("Error has occured : Echo");
      }
    }
  }
  if(Mode == 4 && TimeSynksuccess == 1){
    if(reset == 1){
      WaitingforSending = 1;
      int rtn = SendPacket2Server_1_APRegister();
      WaitingforSending = 0;
      if(rtn == 1){
        reset = -1;
        SaveThisStatus();
        Mode=5; PacketRetry = 0;
//        SaveConfig();
      }else if(rtn == 0){
        if(Debug)Serial.printf("SendPacket2Server_3_SyncConfig Error occured\n");
        if(PacketRetry++ > 5){
          Mode = -1;Serial.println("Mode=-1;");//API Failed
          if(Debug)Serial.println("Error has occured : APRegister");
        }
        //{Mode=0;isOnline=0;}
      }
    }else {
      for(int i=0;i<6;i++){mymac[i] = ether.mymac[i];}
      for(int i=0;i<4;i++){myip[i] = ether.myip[i];}
      for(int i=0;i<4;i++){netmask[i] = ether.netmask[i];}
      for(int i=0;i<4;i++){gwip[i] = ether.gwip[i];}
      for(int i=0;i<4;i++){dnsip[i] = ether.dnsip[i];}
      if(isDHCP == 1)isDHCP = 0;//isDHCP == 2, keep status
      SaveThisStatus();
      StartTime = unixtime();
      BLEStart(1);
      Mode = 5;Serial.println("Mode=5;");
    }
  }
}

void DebugMode(String chr){
  Debug = 0;
  if(chr == (String)"SET"){
    StaticJsonDocument<2000> recvJSON;
    DebugerRcvStr = Serial.readStringUntil(3);DebugerRcvStr = DebugerRcvStr.substring(1);
//    PrintHex((char*)DebugerRcvStr.c_str(), DebugerRcvStr.length());
    if(deserializeJson(recvJSON,(char*)DebugerRcvStr.c_str())){
      Serial.print("\02NO\03");
    }
    else{
      Serial.print("\02OK\03");
      APConfig = recvJSON;
      SaveConfig();
    }
  }else if(chr == (String)"GET"){
    LoadConfig();
    Serial.write(2);
    serializeJsonPretty(APConfig, Serial);
    Serial.write(3);
  }else if(chr == (String)"RST"){
    ESP.restart();
  }else if(chr == (String)"ECH"){
    Serial.print("\02OK\03");
  }else if(chr == (String)"DBG"){
    Serial.println("Error : " + (String)Error);
    Serial.println("vError : " + (String)vError);
    Serial.println("Mode : " + (String)Mode);
    Serial.println("LastErrorTime : " + (String)(unixtime()-LastErrorTime));
    LoadError();
  }else if(chr == (String)"CLD"){
    if(DelError())
      Serial.print("\02OK\03");
    else
      Serial.print("\02NO\03");
  }
  Debug = 1;
}
// Synchronization Task
// 1. SPIFFS Data
// 2. NTP Time Sync(Every midnight)
// 3. Sync Data
void doSync(){
  if(Serial.available()){
    if(Serial.read() == 2){
      DebugerRcvStr = Serial.readStringUntil(3);
      DebugMode(DebugerRcvStr);
    }
  }
  if(Mode == 0){// Restarting Issue!
    if((unixtime() - StartTime) > 3600){
      if(Debug)Serial.println("Waiting time over an hour has done. Restart!");
      SaveError("Waiting time over an hour has done. Restart!, Line : " + (String)__LINE__);
      Error++;
      ESP.restart();
    }
  }
  if(unixtime() - LastErrorTime > 3600){
    LastErrorTime = unixtime();
    vError = 0;
  }
  if(Mode != 0 && vError > 5){
//    Mode = 1;Serial.println("Mode=1");
    hostip = "";
    if(TCPget_Send_Preparing()){//Searching host's IP in Domain Table
      vError = 0;
      ether.printIp("Server IP : ", ether.hisip);
    }else {//Failed in searching host's IP
//      isOnline = 0;
      if(Debug)Serial.printf("Error occured,,, Just Wait!\n");
      ESP.restart();
    }
  }
  if(Mode == 3){
    Mode = 4;Serial.println("Mode=4;");
    TimeSynksuccess = 0;PacketRetry = 0;
    //register udpSerialPrint() to port 1337
    ether.udpServerListenOnPort(&NTPUDPCallbackFunc, localPort);
    delay(100);
  }
  if((millis() - mainTime1) > 2560){
//    if(Debug)Serial.printf("unixTimer Test %d =? %d [%d]\n", unixtime(), mainTime2, unixtime() - mainTime2);
    mainTime1 = millis();
    if(Mode == 4){
      if(!TimeSynksuccess)SyncTimeviaUDP();
      printLocalTime();
    }
  }
  if((unixtime() - mainTime3) > 86400){
    SyncTimeviaUDP();
    mainTime3 = unixtime();
  }
  if(CDMTimerFlag){
    if(unixtime() > CDMTimer){//Timer rings
      Mode = 10;
      CDMTimerFlag = 0;
    }
  }
  if((unixtime() - mainTime2) > ipcdInterval){
    if(ipcdInterval == 0)ipcdInterval = 60;
    if(CDMFlag == 1){
      WaitingforSending = 1;
      int rtn = SendPacket2Server_0_DomainChanged();//cdm
      WaitingforSending = 0;
      
      Serial.println("rtn : " + (String)rtn);
      int DmEmpty = false;
      if(rtn == 1) DmEmpty = !(rtnipcd[0]["dm"].isNull());
      else if(rtn == 2) DmEmpty = !(rtnipcd[1]["dm"].isNull());
      Serial.println("DmEmpty : " + (String)DmEmpty);
      
      if((rtn==1 || rtn == 2) && DmEmpty){
        
        uint32_t timespends = millis();
        while(InterruptingFlag)delay(1);
        WaitingforSending = 1;
        if(millis() - timespends && Debug)Serial.printf("timespends : %d ", millis() - timespends);
        
        time_t rtntime = 0;
        if(rtn == 2){//set Alarm
          CDMDomain = (String)(char*)(const char*)rtnipcd[1]["dm"];
         
          rtntime = MakeUTCTime_t((String)(char*)(const char*)rtnipcd[1]["dt"]);
          if(rtntime != 0){CDMTimer = rtntime;CDMTimerFlag = 1;}
        }else{
          CDMDomain = (String)(char*)(const char*)rtnipcd[0]["dm"];
          rtntime = MakeUTCTime_t((String)(char*)(const char*)rtnipcd[0]["dt"]);
          
          if(rtntime > unixtime()){//set Alarm
            if(rtntime != 0){CDMTimer = rtntime;CDMTimerFlag = 1;}
          }else{//domain comparison
            
            String url = CDMDomain;
            int LastIdx = url.charAt(url.length()-1) == '/' ? url.length()-1 : url.length();
            String rcvhttp, rcvhost, rcvport = "";
            if(url.indexOf("://") > 0){
              int indx = url.indexOf("://");
              rcvhttp = url.substring(0, indx+3);
              rcvhost = url.substring(indx+3, url.indexOf(':',indx+3));
              String portstr = url.substring(url.indexOf(':',indx+3)+1, LastIdx);
              if(portstr == "")rcvport = 80; else rcvport = portstr.toInt();
            }else{
              rcvhttp = "";
              rcvhost = url.substring(0, url.indexOf(':'));
              String portstr = url.substring(url.indexOf(':')+1, LastIdx);
              if(portstr == "")rcvport = 80; else rcvport = portstr.toInt();
            }
            if(((String)rcvhost != (String)host) || ((String)rcvport != (String)port)){
              Mode = 10;//Change domain right now
              if(Debug)Serial.println("Mode=10;");
            }else{
              if(Debug)Serial.println("Data not Changed");
            }
            
          }
        }
        if(CDMTimerFlag){
          CDMTimer = rtntime;
          if(Debug)Serial.println("Reservation succeeded! Domain : " + (String)CDMDomain);
          if(Debug)Serial.printf("CDMTimer = %d\n", CDMTimer);
          if(Debug)Serial.printf("time remained : %ds\n", CDMTimer - unixtime());
        }
        WaitingforSending = 0;
      }
      else {
        BLEStart(1);
        if((String)(char*)(const char*)rtnipcd[rtn-1]["dt"] == "" || rtn == 0){//Error
          if(Debug)Serial.printf("SendPacket2Server_0_DomainChanged Error occured\n");
          SaveError("ipcd Error, Line : " + (String)__LINE__);
          Error++;vError++;
          if(Debug)Serial.println("cdm request failed");
        }else if(rtn == 1)Mode=5;//No Data
      }
      mainTime2 = unixtime();
      CDMFlag = 0;
    }else{
      if(Mode == 5){
  //      ether.persistTcpConnection(true);delay(100);
        WaitingforSending = 1;
        int rtn = SendPacket2Server_3_SyncConfig(1);//ipcd
        WaitingforSending = 0;
        CDMFlag = 1;
  //      ether.persistTcpConnection(false);delay(100);
        if(rtn == 2)Mode=6;//Datas are Available
        else {
          BLEStart(1);
          if(rtn == 1)Mode=5;//No Data
          else if(rtn == 0){//Error
            if(Debug)Serial.printf("SendPacket2Server_3_SyncConfig Error occured\n");
            SaveError("ipcd Error, Line : " + (String)__LINE__);
            Error++;vError++;
    //        for(int i=0;SendPacket2Server_3_SyncConfig(0)==0 && i<3; i++);
            if(Debug)Serial.println("ipcd request failed");
            //{Mode=0;isOnline=0;}
          }
        }
      }
    }
  }
  if(Mode == 6){//Change IP and Test.
    int Failed = 0;
    
    char* rtn = 0;
    if(Debug)Serial.println("Mode : " + (String)Mode);
    if(Debug)Serial.println("APConfig :");
    serializeJsonPretty(APConfig, Serial);
    if(Debug)Serial.println("rtnipcd :");
    serializeJsonPretty(rtnipcd, Serial);
    IPChanged = 0;
    SrvDomainChanged = 0;
    DataChanged = 0;

    char IPSaveBuf[100] = {0,};
    if(!rtnipcd[0]["apip"].isNull()){
      //update IP's
      uint8_t ApipBundle[24] = {0,};int length = 0;
      if((rtn = Debase64((char*)(const char*)rtnipcd[0]["apip"],length))==0){
        if(Debug)Serial.println("apip Decoding Base64 Failed");
        if(Debug)Serial.println("apip:");
        if(Debug)PrintHex((char*)(const char*)rtnipcd[0]["apip"]);
        Failed = 1;
      
        SaveError("IP Decode Error, Line : " + (String)__LINE__);
        Error++;vError++;
        
//        for(int i=0;SendPacket2Server_3_SyncConfig(0)==0 && i<3; i++);
      }else{
        //primary data Backup
        sprintIP(IPSaveBuf, myip);
        APConfig["apip"] = IPSaveBuf;
        sprintIP(IPSaveBuf, netmask);
        APConfig["netmask"] = IPSaveBuf;
        sprintIP(IPSaveBuf, gwip);
        APConfig["gwip"] = IPSaveBuf;
        sprintIP(IPSaveBuf, dnsip);
        APConfig["dnsip"] = IPSaveBuf;
        sprintIP(IPSaveBuf, dns2ip);
        APConfig["dns2ip"] = IPSaveBuf;
      
        memcpy(ApipBundle, rtn, 20);length = 0;
        IPstrChangedFlag = (isNullIP(ApipBundle)==0) || (isNullIP(ApipBundle+4)==0)<<1 || (isNullIP(ApipBundle+8)==0)<<2 || (isNullIP(ApipBundle+12)==0)<<3 || (isNullIP(ApipBundle+16)==0)<<4;
  
        if(Debug)Serial.println("isNullIP(ApipBundle) : " + (String)isNullIP(ApipBundle));
        if(Debug)Serial.println("isNullIP(ApipBundle+4) : " + (String)isNullIP(ApipBundle+4));
        if(Debug)Serial.println("isNullIP(ApipBundle+8) : " + (String)isNullIP(ApipBundle+8));
        if(Debug)Serial.println("isNullIP(ApipBundle+12) : " + (String)isNullIP(ApipBundle+12));
        if(Debug)Serial.println("isNullIP(ApipBundle+16) : " + (String)isNullIP(ApipBundle+16));
        if(Debug)Serial.println("IPstrChangedFlag : " + (String)IPstrChangedFlag);
        
        if(IPstrChangedFlag&0x01)memcpy(myip, ApipBundle, 4);
        if(IPstrChangedFlag&0x02)memcpy(gwip, ApipBundle + 4, 4);
        if(IPstrChangedFlag&0x04)memcpy(netmask, ApipBundle + 8, 4);
        if(IPstrChangedFlag&0x08)memcpy(dnsip, ApipBundle + 12, 4);
        if(IPstrChangedFlag&0x10)memcpy(dns2ip, ApipBundle + 16, 4);
        PrintHex((char*)ApipBundle,20);
  
      
        LoadConfig();
        if(IPstrChangedFlag&0x01){
          sprintIP(IPSaveBuf, myip);
          if(Debug)Serial.println("myip : " + (String)IPSaveBuf);
          IPChanged =  (strcmp(IPSaveBuf,(const char*)APConfig["apip"])==0?0:1);
        }
        if(IPstrChangedFlag&0x02){
          sprintIP(IPSaveBuf, gwip);
          if(Debug)Serial.println("gwip : " + (String)IPSaveBuf);
          IPChanged += (strcmp(IPSaveBuf,(const char*)APConfig["gwip"])==0?0:1);
        }
        if(IPstrChangedFlag&0x04){
          sprintIP(IPSaveBuf, netmask);
          if(Debug)Serial.println("netmask : " + (String)IPSaveBuf);
          IPChanged += (strcmp(IPSaveBuf,(const char*)APConfig["netmask"])==0?0:1);
        }
        if(IPstrChangedFlag&0x08){
          sprintIP(IPSaveBuf, dnsip);
          if(Debug)Serial.println("dnsip : " + (String)IPSaveBuf);
          IPChanged += (strcmp(IPSaveBuf,(const char*)APConfig["dnsip"])==0?0:1);
        }
        if(IPstrChangedFlag&0x10){
          sprintIP(IPSaveBuf, dns2ip);
          if(Debug)Serial.println("dns2ip : " + (String)IPSaveBuf);
          IPChanged += (strcmp(IPSaveBuf,(const char*)APConfig["dns2ip"])==0?0:1);
        }
        
        if(Debug)Serial.println("IPChanged : " + (String)(int)IPChanged);//(const char*)rtnipcd[0]["domain"]);
      }
    }
    if(!rtnipcd[0]["domain"].isNull()){
      PrintHex((char*)(const char*)APConfig["domain"], strlen((const char*)APConfig["domain"]));
      
      String url = (char*)(const char*)rtnipcd[0]["domain"];
      int LastIdx = url.charAt(url.length()-1) == '/' ? url.length()-1 : url.length();
      String rcvhttp, rcvhost, rcvport = "";
      if(url.indexOf("://") > 0){
        int indx = url.indexOf("://");
        rcvhttp = url.substring(0, indx+3);
        rcvhost = url.substring(indx+3, url.indexOf(':',indx+3));
        String portstr = url.substring(url.indexOf(':',indx+3)+1, LastIdx);
        if(portstr == "")rcvport = 80; else rcvport = portstr.toInt();
      }else{
        rcvhttp = "";
        rcvhost = url.substring(0, url.indexOf(':'));
        String portstr = url.substring(url.indexOf(':')+1, LastIdx);
        if(portstr == "")rcvport = 80; else rcvport = portstr.toInt();
      }
      SrvDomainChanged += ((String)rcvhost != (String)host);
      SrvDomainChanged += ((String)rcvport != (String)port);
//      if(Debug)Serial.println("http = "+(String)http);
//      if(Debug)Serial.println("host = "+(String)host);
//      if(Debug)Serial.println("port = "+(String)port);
      
//       = (String)(const char*)rtnipcd[0]["domain"] != (String)(const char*)APConfig["domain"];
    }
    
    char *useYNStr[2] = {"N", "Y"};
    if(!rtnipcd[0]["dist"].isNull())DataChanged += ((String)(double)APConfig["dist"] == (String)(double)rtnipcd[0]["dist"] ? 0 : 1);
    if(!rtnipcd[0]["useyn"].isNull())DataChanged += ((String)useYNStr[(int)APConfig["useyn"]] == (String)(const char*)rtnipcd[0]["useyn"] ? 0 : 1);
//    if(!rtnipcd[0]["apname"].isNull())DataChanged += ((String)(const char*)APConfig["apname"] == (String)(const char*)rtnipcd[0]["apname"] ? 0 : 1);
    if(!rtnipcd[0]["apcode"].isNull())DataChanged += ((String)(const char*)APConfig["apcode"] == (String)(const char*)rtnipcd[0]["apcode"] ? 0 : 1);
    if(!rtnipcd[0]["reterm"].isNull())DataChanged += ((String)(const char*)APConfig["ipcdInterval"] == (String)(const char*)rtnipcd[0]["reterm"] ? 0 : 1);
    


    
    if(Debug)Serial.println("IPChanged : " + (String)IPChanged);
    if(Debug)Serial.println("SrvDomainChanged : " + (String)SrvDomainChanged);
    if(Debug)Serial.println("DataChanged : " + (String)DataChanged);
    
    if(DataChanged){
      if(!rtnipcd[0]["dist"].isNull())APConfig["dist"] = (double)rtnipcd[0]["dist"];
      if(!rtnipcd[0]["useyn"].isNull())APConfig["useyn"] = strcmp((const char*)rtnipcd[0]["useyn"],"Y")==0?1:0;
//      if(!rtnipcd[0]["apname"].isNull())APConfig["apname"] = (const char*)rtnipcd[0]["apname"];
      if(!rtnipcd[0]["apcode"].isNull())APConfig["apcode"] = (const char*)rtnipcd[0]["apcode"];
      if(!rtnipcd[0]["reterm"].isNull())APConfig["ipcdInterval"] = (const char*)rtnipcd[0]["reterm"];
      
      //Data 변경
      Mode = 7;Serial.println("Mode=7;");
    }
    if(SrvDomainChanged){
      String url = (char*)(const char*)rtnipcd[0]["domain"];
      int LastIdx = url.charAt(url.length()-1) == '/' ? url.length()-1 : url.length();
      if(url.indexOf("://") > 0){
        int indx = url.indexOf("://");
        http = url.substring(0, indx+3);
        host = url.substring(indx+3, url.indexOf(':',indx+3));
        port = url.substring(url.indexOf(':',indx+3)+1, LastIdx).toInt();
      }else{
        http = "";
        host = url.substring(0, url.indexOf(':'));
        port = url.substring(url.indexOf(':')+1, LastIdx).toInt();
      }
      if(Debug)Serial.println("http = "+(String)http);
      if(Debug)Serial.println("host = "+(String)host);
      if(Debug)Serial.println("port = "+(String)port);
      
      IPChangeFlag = 3;//Domain Changed
      BLEStart(0);
      Mode = 2;Serial.println("Mode=2;");
    }
    if(IPChanged){
      IPChangeFlag = 1;//IP Changed
      Mode = 1;Serial.println("Mode=1;");
      BLEStart(0);
    }
    if(!DataChanged && !SrvDomainChanged && !IPChanged){
      if(Debug)Serial.println("Noting changed");
      //WaitingforSending = 1 : to set right next to SyncConfig function I made a mistake Manually!!
      for(int i=0,WaitingforSending = 1; SendPacket2Server_3_SyncConfig(2)==0 && (WaitingforSending = 1) && i<3; i++);//ipc3, Send packet to Server to notify that AP settings succeeded
      Mode = 5;Serial.println("Mode=5;");
    }
  }
  if(Mode == 7){//IP Change Succeeded
    if(Debug)Serial.println("Mode : " + (String)Mode);
    if(IPChanged){
      APConfig["apip"] = (String)myip[0] + "." + (String)myip[1] + "." + (String)myip[2] + "." + (String)myip[3];
      APConfig["netmask"] = (String)netmask[0] + "." + (String)netmask[1] + "." + (String)netmask[2] + "." + (String)netmask[3];
      APConfig["gwip"] = (String)gwip[0] + "." + (String)gwip[1] + "." + (String)gwip[2] + "." + (String)gwip[3];
      APConfig["dnsip"] = (String)dnsip[0] + "." + (String)dnsip[1] + "." + (String)dnsip[2] + "." + (String)dnsip[3];
      APConfig["dns2ip"] = (String)dns2ip[0] + "." + (String)dns2ip[1] + "." + (String)dns2ip[2] + "." + (String)dns2ip[3];
      
//      if(Debug)Serial.println("apip : " + (String)(const char*)APConfig["apip"]);
//      if(Debug)Serial.println("netmask : " + (String)(const char*)APConfig["netmask"]);
//      if(Debug)Serial.println("gwip : " + (String)(const char*)APConfig["gwip"]);
//      if(Debug)Serial.println("dnsip : " + (String)(const char*)APConfig["dnsip"]);
      
//      sprintIP(IPSaveBuf[0], myip);    APConfig["apip"] = IPSaveBuf[0]; 
//      sprintIP(IPSaveBuf[1], netmask); APConfig["netmask"] = IPSaveBuf[1]; 
//      sprintIP(IPSaveBuf[2], gwip);    APConfig["gwip"] = IPSaveBuf[2]; 
//      sprintIP(IPSaveBuf[3], dnsip);   APConfig["dnsip"] = IPSaveBuf[3]; 
    }
    if(SrvDomainChanged){
      APConfig["domain"] = rtnipcd[0]["domain"];
    }
    SaveConfig();
    doSetupSync();
    if(Debug)Serial.println("IP Change Succeeded");
      //WaitingforSending = 1 : to set right next to SyncConfig function I made a mistake Manually!!
      for(int i=0,WaitingforSending = 1; SendPacket2Server_3_SyncConfig(2)==0 && (WaitingforSending = 1) && i<3; i++);//ipc3, Send packet to Server to notify that AP settings succeeded
    BLEStart(1);
    Mode = 5;Serial.println("Mode=5;");
  }
  if(Mode == 8){//IP Change Failed
    if(Debug)Serial.println("Mode : " + (String)Mode);
    doSetupSync();
    if(isOnline == 0 || IPChangeFlag == 1 || IPChangeFlag == 3)IPChangeFlag = 2;
    else {
      if(IPChangeFlag == 2){
        if(Debug)Serial.println("IP Return Failed!");
          SaveError("ip return Error(Mode : 0), Line : " + (String)__LINE__);
          Error++;
         Mode = 0;if(Debug)Serial.println("Mode=0;");
      }
      Mode = 9;if(Debug)Serial.println("Mode=9;");
    }
  }
  if(Mode == 9){
    //WaitingforSending = 1 : to set right next to SyncConfig function I made a mistake Manually!!
    for(int i=0,WaitingforSending = 1; SendPacket2Server_3_SyncConfig(0)==0 && (WaitingforSending = 1) && i<3; i++);
    if(Debug)Serial.println("IP Change Failed");
//    BLEStart(useYN);
    BLEStart(1);
    Mode = 5;Serial.println("Mode=5;");
  }
  if(Mode == 10){
    String url = CDMDomain;
    int LastIdx = url.charAt(url.length()-1) == '/' ? url.length()-1 : url.length();
    if(url.indexOf("://") > 0){
      int indx = url.indexOf("://");
      http = url.substring(0, indx+3);
      host = url.substring(indx+3, url.indexOf(':',indx+3));
      port = url.substring(url.indexOf(':',indx+3)+1, LastIdx).toInt();
    }else{
      http = "";
      host = url.substring(0, url.indexOf(':'));
      port = url.substring(url.indexOf(':')+1, LastIdx).toInt();
    }
    if(Debug)Serial.println("http = "+(String)http);
    if(Debug)Serial.println("host = "+(String)host);
    if(Debug)Serial.println("port = "+(String)port);
    
    IPChangeFlag = 3;//Domain Changed
    BLEStart(0);
    Mode = 2;Serial.println("Mode=2;");
  }
//  if(Failed == 1){//If failed
//    
//    if(Debug)Serial.println("send SendPacket2Server_3_SyncConfig");
//    for(int i=0;SendPacket2Server_3_SyncConfig(0)==0 && i<3; i++);
//    if(Debug)Serial.println("SendPacket2Server_3_SyncConfig dending is done!!");
//    BLEStart(1);
//    Mode = 5;Serial.println("Mode=5;");
//  }
}


void SaveThisStatus(){
  if(Debug)Serial.println("SaveThisStatus()");
  
  APConfig["dist"] = IncircleDistance;
//  if(Debug)Serial.println("IncircleDistance = "+(String)IncircleDistance);
  APConfig["IdentifyDistance"] = IdentifyDistance;
//  if(Debug)Serial.println("IncircleDistance = "+(String)IdentifyDistance);

  APConfig["BeaconTimeout"] = Timeout;
//  if(Debug)Serial.println("Timeout = "+(String)Timeout);
  APConfig["ipcdInterval"] = ipcdInterval;
//  if(Debug)Serial.println("ipcdInterval = "+(String)ipcdInterval);
  
  APConfig["apcode"] = APSerialNumber;
//  if(Debug)Serial.println("APSerialNumber = "+(String)APSerialNumber);
  APConfig["useyn"] = useYN;
//  if(Debug)Serial.println("useYN = "+(String)useYN);
  APConfig["enableOutDetecting"] = enableOutDetecting;
//  if(Debug)Serial.println("enableOutDetecting = "+(String)enableOutDetecting);

  APConfig["ServerURL"] = ServerURL;
//  if(Debug)Serial.println("ServerURL = "+(String)ServerURL);
  APConfig["CDMURL"] = CDMURL;
//  if(Debug)Serial.println("CDMURL = "+(String)CDMURL);
//  APConfig["apname"] = APName;
//  if(Debug)Serial.println("APName = "+(String)APName);
  APConfig["idxClient"] = idxClient;
//  if(Debug)Serial.println("idxClient = "+(String)idxClient);

  APConfig["domain"] = http + host + (String)(port == 80 ? "" : ":" + (String)port);

//  if(Debug)Serial.println("url = "+(String)(const char*)APConfig["domain"]);

  
  APConfig["apip"] = (String)myip[0] + "." + (String)myip[1] + "." + (String)myip[2] + "." + (String)myip[3];
  APConfig["netmask"] = (String)netmask[0] + "." + (String)netmask[1] + "." + (String)netmask[2] + "." + (String)netmask[3];
  APConfig["gwip"] = (String)gwip[0] + "." + (String)gwip[1] + "." + (String)gwip[2] + "." + (String)gwip[3];
  APConfig["dnsip"] = (String)dnsip[0] + "." + (String)dnsip[1] + "." + (String)dnsip[2] + "." + (String)dnsip[3];
  APConfig["dns2ip"] = (String)dns2ip[0] + "." + (String)dns2ip[1] + "." + (String)dns2ip[2] + "." + (String)dns2ip[3];
  
//  if(Debug)Serial.println("apip : " + (String)(const char*)APConfig["apip"]);
//  if(Debug)Serial.println("netmask : " + (String)(const char*)APConfig["netmask"]);
//  if(Debug)Serial.println("gwip : " + (String)(const char*)APConfig["gwip"]);
//  if(Debug)Serial.println("dnsip : " + (String)(const char*)APConfig["dnsip"]);
  
//  sprintIP((char*)(const char*)APConfig["apip"], myip);
//  sprintIP((char*)(const char*)APConfig["netmask"], netmask);
//  sprintIP((char*)(const char*)APConfig["gwip"], gwip);
//  sprintIP((char*)(const char*)APConfig["dnsip"], dnsip);
  
  APConfig["reset"] = reset;
  if(Debug)Serial.println("reset = "+(String)reset);
  APConfig["isDHCP"] = isDHCP;
  if(Debug)Serial.println("isDHCP = "+(String)isDHCP);
  
  serializeJsonPretty(APConfig, Serial);
  SaveConfig();
}
void doSetupSync(){
  LoadConfig();
  
  serializeJsonPretty(APConfig, Serial);
  
//  if(Debug)Serial.printf("apcode = %s\r\n", (const char*)APConfig["apcode"]);
  IncircleDistance = (double)APConfig["dist"];
  if(Debug)Serial.println("IncircleDistance = "+(String)IncircleDistance);
  IdentifyDistance = (double)APConfig["IdentifyDistance"];
  if(Debug)Serial.println("IdentifyDistance = "+(String)IdentifyDistance);

  Timeout = (double)APConfig["BeaconTimeout"];
  if(Debug)Serial.println("Timeout = "+(String)Timeout);
  ipcdInterval = (double)APConfig["ipcdInterval"];
  if(Debug)Serial.println("ipcdInterval = "+(String)ipcdInterval);
  
  APSerialNumber = (const char*)APConfig["apcode"];
  if(Debug)Serial.println("APSerialNumber = "+(String)APSerialNumber);
  idxClient = (int)APConfig["idxClient"];
  if(Debug)Serial.println("idxClient = "+(String)idxClient);
  useYN = (int)APConfig["useyn"];
  if(Debug)Serial.println("useYN = "+(String)useYN);
  enableOutDetecting = (int)APConfig["enableOutDetecting"];
  if(Debug)Serial.println("enableOutDetecting = "+(String)enableOutDetecting);

  ServerURL = (const char*)APConfig["ServerURL"];
  if(Debug)Serial.println("ServerURL = "+(String)ServerURL);
  CDMURL = (const char*)APConfig["CDMURL"];
  if(Debug)Serial.println("CDMURL = "+(String)CDMURL);
//  APName = (const char*)APConfig["apname"];
//  if(Debug)Serial.println("APName = "+(String)APName);

  String url = (const char*)APConfig["domain"];
  if(Debug)Serial.println("url = "+(String)url);
  int LastIdx = url.charAt(url.length()-1) == '/' ? url.length()-1 : url.length();
  if(url.indexOf("://") > 0){
    int indx = url.indexOf("://");
    http = url.substring(0, indx+3);
    host = url.substring(indx+3, url.indexOf(':',indx+3));
    port = url.substring(url.indexOf(':',indx+3)+1, LastIdx).toInt();
  }else{
    http = "";
    host = url.substring(0, url.indexOf(':'));
    port = url.substring(url.indexOf(':')+1, LastIdx).toInt();
  }
  if(Debug)Serial.println("http = "+(String)http);
  if(Debug)Serial.println("host = "+(String)host);
  if(Debug)Serial.println("port = "+(String)port);
  
  sscanIP((char*)(const char*)APConfig["apip"], myip);
  sscanIP((char*)(const char*)APConfig["netmask"], netmask);
  sscanIP((char*)(const char*)APConfig["gwip"], gwip);
  sscanIP((char*)(const char*)APConfig["dnsip"], dnsip);
  sscanIP((char*)(const char*)APConfig["dns2ip"], dns2ip);
  
  ether.printIp("IP address  :  ", myip);
  ether.printIp("Netmask     :  ", netmask);
  ether.printIp("Gateway IP  :  ", gwip);
  ether.printIp("DNS server IP  :  ", dnsip);
  ether.printIp("DNS server IP  :  ", dns2ip);
  
  reset = (int)APConfig["reset"];
  if(Debug)Serial.println("reset = "+(String)reset);
  isDHCP = (int)APConfig["isDHCP"];
  if(Debug)Serial.println("isDHCP = "+(String)isDHCP);
  
  SaveConfig();
}

// Service Task
// 1. Turing on BLE
// 2. BLE Table management
void doService(){
  if(TimeSynksuccess == 0)return;
  if(InterruptingFlag)return;
  if(Mode != 5)return;
//  uint32_t looptimeout = millis();
//  while(InterruptingFlag){ether.packetLoop(ether.packetReceive());yield();}
//  InterruptingTime = millis();
  
  
  for(int i=0;i<SensorLen;i++){
    if(Lists[i].IsDataReady){
      if(Debug)Serial.printf("Data of Lists[%d] is Ready\n",i+1);
      char Buf[40] = {0,};
      memcpy(Buf,Lists[i].PhoneNum, strlen(Lists[i].PhoneNum));
      
      struct timeval sentv = Lists[i].SendTime;
      struct tm *senlt;
      if((senlt = localtime(&sentv.tv_sec)) == NULL) {if(Debug)Serial.println("localtime() call error");}
      char timeStringBuff[50]; //50 chars should be enough
      sprintf(timeStringBuff, "%04d%02d%02d%02d%02d%02d%03d", senlt->tm_year + 1900  , senlt->tm_mon + 1  , senlt->tm_mday  , senlt->tm_hour  , senlt->tm_min  , senlt->tm_sec  , sentv.tv_usec/1000);
  
      if(Debug)Serial.printf("---Idx[%d] %s %d,%d,%d, %6dms,%5.2fm I:%d,S:%d [%c%c] %s\n",i+1,Buf
      ,Lists[i].RSSI, Lists[i].TxPower, Lists[i].lastTime
      , millis() - Lists[i].lastinCircleTime, getDistance(Lists[i].RSSI, Lists[i].TxPower)
      ,Lists[i].IsEnter, Lists[i].IsDataReady
      ,"OI"[Lists[i].IsEnter],"DW"[Lists[i].IsDataReady], timeStringBuff
      );
      
      for(int j=0;; j++){
        WaitingforSending = 1;
        int rtn = SendPacket2Server_5_BeaconData(Lists[i]);
        WaitingforSending = 0;
        if(rtn == 1)break;
        if(j==2)
        {
          SaveError("BeaconData Error(retry 3), Line : " + (String)__LINE__);
          Error++;vError++;
          break;
        }
      }
      Lists[i].IsDataReady = 0;
    }
  }
  if((millis() - mainTime) > 2560){
    mainTime = millis();
//    if(Debug)Serial.printf("---%d ms waited",InterruptingTime - looptimeout);
    // put your main code here, to run repeatedly:
    uint32_t Now = millis();
    time_t unixtm = unixtime();
    struct tm *lt = localtime(&unixtm);
    if(Debug)Serial.printf("[%dms] %04d-%02d-%02d %02d:%02d:%02d, Heap %dB free  {%d}  O%d,M%d,T%d\n", millis()
    , lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec
    , ESP.getFreeHeap(), sizeof(SensorList),isOnline,Mode,TimeSynksuccess);
    for(int i=0;i<SensorLen;i++){
      //if(Lists[i].IsEmpty){if(Debug)Serial.printf("---Idx[%d]\n",i+1);continue;}
      char Buf[40] = {0,};
      memcpy(Buf,Lists[i].PhoneNum, strlen(Lists[i].PhoneNum));
      
      struct timeval sentv = Lists[i].SendTime;
      struct tm *senlt;
      if((senlt = localtime(&sentv.tv_sec)) == NULL) {if(Debug)Serial.println("localtime() call error");}
      char timeStringBuff[50]; //50 chars should be enough
      sprintf(timeStringBuff, "%04d%02d%02d%02d%02d%02d%03d", senlt->tm_year + 1900  , senlt->tm_mon + 1  , senlt->tm_mday  , senlt->tm_hour  , senlt->tm_min  , senlt->tm_sec  , sentv.tv_usec/1000);
  
      if(Debug)Serial.printf("---Idx[%d] %s %d,%d,%d, %6dms,%5.2fm I:%d,S:%d [%c%c%c] %s   %d>%d || %f>%f : %d\n",i+1,Buf
      , Lists[i].RSSI, Lists[i].TxPower, Lists[i].lastTime
      , Now - Lists[i].lastinCircleTime, getDistance(Lists[i].RSSI, Lists[i].TxPower)
      , Lists[i].IsEnter, Lists[i].IsDataReady
      , "OI"[Lists[i].IsEnter],"DW"[Lists[i].IsDataReady]," E"[Lists[i].IsEmpty], timeStringBuff
      , Now - Lists[i].lastTime, (uint32_t)(Timeout*1000) ,getDistance(Lists[i].RSSI, Lists[i].TxPower), IdentifyDistance
      , (Now - Lists[i].lastTime > (uint32_t)(Timeout*1000) || getDistance(Lists[i].RSSI, Lists[i].TxPower) >= IdentifyDistance)
      );
    }
    
    Now = millis();
    //if(Debug)Serial.print("\n");
    for(int i=0;i<SensorLen;i++){
      if(Lists[i].IsEmpty == 0){
        if(Lists[i].IsEnter == true && Now - Lists[i].lastinCircleTime > (uint32_t)(Timeout*1000)){
          Lists[i].IsDataReady = 0;
          Lists[i].IsEnter = false;
          if(enableOutDetecting){
            for(int j=0;; j++){
              WaitingforSending = 1;
              int rtn = SendPacket2Server_5_BeaconData(Lists[i],1);
              WaitingforSending = 0;
              if(rtn == 1)break;
              if(j==2){
                SaveError("BeaconData Error(retry 3), Line : " + (String)__LINE__);
                Error++;vError++;
                break;
              }
            }
          }
        }
        if(Now - Lists[i].lastTime > (uint32_t)(Timeout*1000) || 
          getDistance(Lists[i].RSSI, Lists[i].TxPower) >= IdentifyDistance){
          Lists[i].IsEmpty = true;
        }
      }
    }
  }
}
void BLEStart(bool on){
  if(BLEOnFlag==on)return;
  BLEOnFlag = on;
  if(BLEOnFlag){
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->start(0, &scanCompleteCB, true);
  }else{
    pBLEScan->stop();
  }
}
void doSetupService(){
  if(Debug)Serial.println("BLE Turing on...");
  delay(100);
  
  BLEDevice::init("");
  //BLEStart(1);
  //  BLEStart(useYN);
}

//static BufferFiller bfill;  // used as cursor while filling the buffer
//char* Recvdata = "";
//const char okHeader[] =
//    "HTTP/1.0 200 OK\r\n"
//    "Content-Type: text/html\r\n"
//    "Pragma: no-cache\r\n"
//;
//static void homePage(const char* data, BufferFiller& buf) {
//  char DebugBuf[80] = {0,};
//  printf(DebugBuf, "[%dms], Heap %dB free  {%d}  O%d,M%d,T%d\n", millis(), ESP.getFreeHeap(), sizeof(SensorList),isOnline,Mode,TimeSynksuccess);
//  String Sendbuf = DebugBuf;
//  for(int i=0;i<SensorLen;i++){
//    if(Lists[i].IsEmpty){Sendbuf += "---Idx[" + String(i+1) + "]\r\n";continue;}
//    char Buf[40] = {0,};
//    memcpy(Buf,Lists[i].PhoneNum, strlen(Lists[i].PhoneNum));
//    sprintf(DebugBuf, "---Idx[%d] %s %d,%d,%d, %6dms,%5.2fm I:%d,S:%d [%c%c]\n",i+1, Buf
//    ,Lists[i].RSSI, Lists[i].TxPower, Lists[i].lastTime
//    , millis() - Lists[i].lastinCircleTime, getDistance(Lists[i].RSSI, Lists[i].TxPower)
//    ,Lists[i].IsEnter, Lists[i].IsDataReady
//    ,"OI"[Lists[i].IsEnter],"DW"[Lists[i].IsDataReady]
//    );
//    Sendbuf += DebugBuf;
//  }
//  buf.emit_p(PSTR("$F\r\n"
//      "<meta http-equiv='refresh' content='3'/>"
//      "<title>Detacting Bluetooth List</title>"
//      "<p>$F</p>"), okHeader, Sendbuf);
//  buf.emit_p(PSTR(" (Heap $D bytes free)"), ESP.getFreeHeap());
//}
//void doWebDebug(){
//  if(Mode < 2)return;
//  int oldport = 0;
//  if(Ethergetpos != 0)return;
//  bfill = ether.tcpOffset();
//  Recvdata = (char *) Ethernet::buffer + Ethergetpos;
//  if(strncmp("GET ", Recvdata, 4) != 0)return;
//  if(Debug)Serial.println("doWebDebug()");
//  if(Debug)Serial.println("Recvdata : " + (String)Recvdata);
//  
//  // receive buf hasn't been clobbered by reply yet
//  if (strncmp("GET / ", Recvdata, 6) == 0)homePage(Recvdata, bfill);
//  else bfill.emit_p(PSTR(
//        "HTTP/1.0 401 Unauthorized\r\n"
//        "Content-Type: text/html\r\n"
//        "\r\n"
//        "<h1>401 Unauthorized</h1>"));
//  ether.httpServerReply(bfill.position()); // send web page data
//}

void setup() {
  Serial.begin(115200);
//  Serial.setTimeout(1000);//default - for debug readStringUntil
  if(Debug)Serial.printf("[%dms], Heap %dB free\n", millis(), ESP.getFreeHeap());
  delay(100);
  
  if(Debug)Serial.println("starting...");
  
  doSetupSync();
  doSetupOnlinefy();
  doSetupService();


}
void loop() {
  if(isOnline == 2 || Mode == 0)Ethergetpos = ether.packetLoop(Ethergetlen = ether.packetReceive());
//  doWebDebug();
  doSync();
  doService();
  doOnlinefy();
}
