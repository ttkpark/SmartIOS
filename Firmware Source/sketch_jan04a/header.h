#include <string.h>
#include "base64/base64.hpp"
#include <Crypto.h>
#include <math.h>

byte AESKey[2][32] = {{0,},};
byte AESIV[2][16] = {{0,},};

unsigned char CryptBuf[4][500] = {{0,},};

unsigned char base64[4][500];
unsigned int base64_length;

char Codestr[512] = {0,};

double getDistance(int rssi, int txPower){
  return pow(10.0, (double)(txPower - rssi) / 20.0);
}
void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
     frac = (val - int(val)) * precision;
   else
      frac = (int(val)- val ) * precision;
   int frac1 = frac;
   while( frac1 /= 10 )
       precision /= 10;
   precision /= 10;
   while(  precision /= 10)
       Serial.print("0");

   Serial.println(frac,DEC) ;
}
const char* Hex2Str = "0123456789abcdef";
void printUUID(char Buf[], uint8_t *uuid){
  int bufindex = 0, index = 0;
  for(int i=0;i<4;i++){
    Buf[bufindex++] = Hex2Str[uuid[index]>>4];
    Buf[bufindex++] = Hex2Str[uuid[index++]&0xf];
  }
  for(int j=0;j<3;j++){
    Buf[bufindex++] = '-';
    for(int i=0;i<2;i++){
      Buf[bufindex++] = Hex2Str[uuid[index]>>4];
      Buf[bufindex++] = Hex2Str[uuid[index++]&0xf];
    }
  }
  Buf[bufindex++] = '-';
  for(int i=0;i<6;i++){
    Buf[bufindex++] = Hex2Str[uuid[index]>>4];
    Buf[bufindex++] = Hex2Str[uuid[index++]&0xf];
  }
  Buf[bufindex++] = 0;
}
bool isPhoneNumber(uint8_t *PhoneNumber, int len){
  if(len < 8 && len > 11)return 0;
  for(int i=0;i<len; i++){
    if(PhoneNumber[i] < '0' || PhoneNumber[i] > '9')return 0;
  }
  return 1;
}
void sprintIP(char Buf[], uint8_t *ip){
  sprintf(Buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}
bool isNullIP(uint8_t *ip){
  return (ip[0]==0) && (ip[1]==0) && (ip[2]==0) && (ip[3]==0);
}
void sscanIP(char *Buf, uint8_t *ip){
  if(!Buf)return;
  int argip[4] = {0,};
  sscanf(Buf, "%d.%d.%d.%d", &argip[0], &argip[1], &argip[2], &argip[3]);
  ip[0] = argip[0];ip[1] = argip[1];ip[2] = argip[2];ip[3] = argip[3];
}
int Hex2int(char ch){
  if(ch <= '0' && ch >= '9')return ch - '0';
  else if(ch <= 'A' && ch >= 'F')return ch - 55;
  else if(ch <= 'a' && ch >= 'f')return ch - 87;
}
int getUUID(uint8_t uuid[],char *Buf){
  if(strlen(Buf) != 36)return 0;

  int bufindex = 0, index = 0;
  for(int i=0;i<4;i++){
    uuid[index++] = Hex2int(Buf[bufindex++])<<4 | Hex2int(Buf[bufindex++]);
  }
  for(int j=0;j<3;j++){
    bufindex++;
    for(int i=0;i<2;i++){
      uuid[index++] = Hex2int(Buf[bufindex++])<<4 | Hex2int(Buf[bufindex++]);
    }
  }
  bufindex++;
  for(int i=0;i<6;i++){
    uuid[index++] = Hex2int(Buf[bufindex++])<<4 | Hex2int(Buf[bufindex++]);
  }
}

void PrintHex(char* data,int len=-1){
  if(len = -1)len = strlen(data);
  for(int i=0;i<(len-1 >> 4) + 1;i++){
    Serial.printf("%03X0h", i);
    for(int j=0;j<16;j++){
        if((i << 4) + j >= len)Serial.print("   ");
        else Serial.printf(" %02x", data[(i << 4) + j]);
    }
    Serial.print(" | ");
    for(int j=0;j<16;j++){
      if((i << 4) + j >= len)Serial.print(" ");
      else {
        if((data[(i << 4) + j] & 0x70) == 0) Serial.print(' ');
        else if(data[(i << 4) + j] & 0x80) Serial.print('.');
        else Serial.print(data[(i << 4) + j]);
      }
    }
    Serial.println();
  }
}
char* AESKeyIV[][2] = {
  {"RnylrE0lNr8yf0YAzUtVskJS9KqjUbhZCougPCwP8A8=", "5kKMVwg8LS1Xw7DmO79S8A=="},
  {"6u7vXieaa3Gtn0Uxqh2zaXCWfZbEPTVq27cZdLPruv4=", "bwqcL8ehp1kEHyTv8WOUgg=="},
  {"5NkTYUbbcAsDwcXWt0Ti2uTB4S80u9Ze2gpVAHC7Drk=", "IA2a3t4uiNXJ40rvA0jQbQ=="},
  {"4sKIzzmAIcUAvSJ5CFpJRRxfLmbKLJZcXrel42y5lPA=", "r1dn567ZWSwDVKDheM7shA=="},
  {"98DH4ePvLpPDDh729zCOTcmIPFZaOfIYn6RdUPyoETU=", "cmwiVEeg3WPH0JHN0GvXeQ=="},
  {"ad39MZJGtKkk1MjKyUybbPGuuYPRhVhivFNS26jHQPE=", "rptkpmDtKyjaW18mCrikVw=="},
  {"JncdkS0l086QUZWO00rSYSoNjTfYyLsfbyB2HftlJXY=", "5RWw7BH7jOdoZQWHabG5RA=="},
  {"vrAB0MVfJKtDn0IlGqUsRND2e3AbcO4pw0vXIlUc6yY=", "aCjcMbCxO0iN0QWFwLia3w=="},
  {"0mJJ0F0NS3xwPgslHSZtfpAlDC6GgO0m6AtCBPQZpgU=", "DfQt6sBYgZk0zW05yo4OJA=="},
  {"6wfUH2wAAcGiSXGh0zBNd609pXNGZrdpge3hCwJQCRc=", "mSgtamCSm617j8BHHxQypg=="},
  {"j90pYk2BI0aQJ9r8x9aV9pJ8v2XK21yJnD0Xj5nJbD0=", "ppr3HGEE4KPZGi4llFSSTQ=="},
  {"EGuRiPDwgiN77T0jeT5nw8TeFmRTJJInc7fKjQQomvM=", "ObYvF0IncDrhGnLvOTXh6w=="},
  {"k5eSgIjP5Ejfy75IEjxbYYeDTuiNobSyGkmGSS07Q3o=", "JvE77X7WoLOD6DtZoT56JA=="},
  {"406w1GFk0txXRM4kQtEDzRIv3cEBawZPMkAzjTRs8LA=", "rASbDloqZ26U5Ia6KvC3AA=="},
  {"02HLviNTwQ8JkKslEwcvGJaxE26qjjJ0MBJF8GBNS9U=", "bkC0x2yjqX0DJOmKgKdR3g=="},
  {"NXyYRjXtS1JQ0kdq1wwS9ZC3PJv0ErbgIGPgMPPBgeA=", "e5840tZxGbnqimINUrskkA=="},
  {"9M5C40xj5to85JEB7KUB33CV0R1zdnzYjciUTyXyIuc=", "bbCYCYmAwR1DJCiBgAxmPA=="},
  {"iI0IVH0jXit1PtdGY5DYIv7twIayHLxiIFLHkNBHxzk=", "2oCSjcp1mbcljQ7J0nO6xg=="},
  {"X7asxJcFBvWX6Z1den0Qap4DPeWerPbl50kTK6tLwuo=", "SC6le4tw9XN74dxU9i07kg=="},
  {"Lnd80vYgvVinb2G9xeTW6QXtOl4GJW5at0CgnzcIt48=", "v14cTy5EjSQv2TYtC3YJxg=="},
  {"pVtiRquV67OMuYliPkPWQ3YQOlLxvyH4wgbSiy6rDAE=", "KJcwGDtJP0joiOqvUt8k1A=="},
  {"DzlqcUP4b0GYRdu7G06shPHoAQhBXfblMGPgVj8HizY=", "EAMs71zx6O2B5IGESwqMXA=="},
  {"bjl1CX2PqNHqN0I0ClLu7noD4dB9CdW4zL30WRzQyJ0=", "kYlh5jKNhqc7PEJwM96CRg=="},
  {"ttwdz6ugWlfCYUMfmczDw0AeaZtqQKAmq6glqoGNP2g=", "JVxCCgSoQv1yL38OL6kWEQ=="},
  {"wgmPDvNcaBSuN5xss5DofmXcIbKdheoLVo7gwqwsm2w=", "E7YBuPyZz5M1XDpqlj721w=="},
  {"NPoN5tgYMtPCNnarlZDE0U9X5mCscd3L4SQ0eH23BtQ=", "N7FiahySIWXKW2RmKKdZ7w=="},
  {"BlAQwh1nH4VWkshiMunEXNpyPgodObfU0O8A8SeD3HM=", "4cM3yVky0Tk9XGwgOFt0qw=="},
  {"wIJyNdvPBPADmcayPbYOEN9sxl1VeaFT6dJb4OhUA2Y=", "TghokqzOEePhjZmNEsBeHA=="},
  {"pVGcXnfiFF6c1np9nqdRXjAfl83Jxsu164NZm2arAbw=", "Ti75F94a5bzL07rzD3g0FA=="},
  {"BlDZbyFMWz0P5YrX0YDyuuMZtUVYFkRV8xMgiDN6dH4=", "oHDLVq13LJ35LZOgEVuKeA=="}
};
/*
char array890[9][8] = {
                                     { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' } };
char array891[9][8] = {
                                     { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
};
char array892[9][8] = {
                                     { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
};
char array893[9][8] = {
                                     { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
};
char array894[9][8] = {
                                     { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
};
char array895[9][8] = {
                                     { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
};
char array896[9][8] = {
                                     { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
};
char array897[9][8] = {
                                     { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
};
char array898[9][8] = {
                                     { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
};
char array899[9][8] = {
                                     { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
};
char array900[8][9] = {
                                     { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' } };
char array901[8][9] = {
                                     { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
};
char array902[8][9] = {
                                     { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
};
char array903[8][9] = {
                                     { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
};
char array904[8][9] = {
                                     { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
};
char array905[8][9] = {
                                     { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
};
char array906[8][9] = {
                                     { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
};
char array907[8][9] = {
                                     { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
};
char array908[8][9] = {
                                     { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
};
char array909[8][9] = {
                                     { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
};

char array100[10][8] = {
                                     { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' } };
char array101[10][8] = {
                                     { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
};
char array102[10][8] = {
                                     { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
};
char array103[10][8] = {
                                     { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
};
char array104[10][8] = {
                                     { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
};
char array105[10][8] = {
                                     { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
};
char array106[10][8] = {
                                     { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
};
char array107[10][8] = {
                                     { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
};
char array108[10][8] = {
                                     { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
};
char array109[10][8] = {
                                     { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
};
char array110[8][10] = {
                                     { 'i', '+', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', '=', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', ';', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', '#', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', '@', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', '\'', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', ',', 'g', 'm', 'w', 'Z', 'k', 'A', '~', 'n' }
                                   , { '0', ':', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' } };
char array111[8][10] = {
                                     { '0', 'o', '+', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '=', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', ';', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '#', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', '@', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', '\'', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', ',', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', ':', 'm', 'w', 'Z', 'k', 'A', '~', 'n' }
};
char array112[8][10] = {
                                     { 'j', 'g', 'm', '+', 'w', 'Z', 'k', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', '=', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', ';', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', '#', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '@', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', '\'', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', ',', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', ':', '4', 'Q', '2', 'x', 'Y', '(' }
};
char array113[8][10] = {
                                     { 'W', 'T', 'X', '4', '+', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', '=', 'Z', 'k', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', ';', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', '#', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', '@', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', '\'', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', ',', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', ':', 't', 'f', '-', 'H', 'v' }
};
char array114[8][10] = {
                                     { '.', 'b', 'F', 'h', 't', '+', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '=', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', ';', 'k', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', '#', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', '@', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', '\'', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', ',', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', ':', 'U', '6', 'y', '!' }
};
char array115[8][10] = {
                                     { 'a', 'I', 'q', 'V', '5', 'U', '+', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '=', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', ';', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', '#', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', '@', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', '\'', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', ',', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', ':', 'R', 'c', 'r' }
};
char array116[8][10] = {
                                     { '8', 'z', '3', '_', 's', '^', 'R', '+', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', '=', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', ';', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', '#', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '@', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', '\'', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', ',', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', ':', '1', '|' }
};
char array117[8][10] = {
                                     { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '+', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', '=', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', ';', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', '#', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '@', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '~', '\'', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', '\0', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', '\0', 'E' }
};
char array118[8][10] = {
                                     { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!', '+' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E', '=' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(', ';' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|', '#' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '~', 'n', '@' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r', '\'' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v', ',' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')', ':' }
};
char array119[8][10] = {
                                     { '0', '+', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'W', '=', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'a', ';', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '8', '#', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'D', '@', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { 'j', '\'', 'g', 'm', 'w', 'Z', 'k', 'A', '~', 'n' }
                                   , { '.', ',', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'i', ':', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
};


/// <summary>
/// Packet 복호화. string denc = bizEncrypt.DecodingPacketString("890", str);
/// </summary>
/// <param name="mty">890 ~ 909</param>
/// <param name="str">472337534367287571~</param>
/// <returns></returns>
static char *DecodingPacketString(int mty, char* str)
{
  memset(Codestr,0,256);
  char* rtn = "";
  void**ary = 0;
  if (mty == 890) { ary = (void**)array890; }
  else if (mty == 891) { ary = (void**)array891; }
  else if (mty == 892) { ary = (void**)array892; }
  else if (mty == 893) { ary = (void**)array893; }
  else if (mty == 894) { ary = (void**)array894; }
  else if (mty == 895) { ary = (void**)array895; }
  else if (mty == 896) { ary = (void**)array896; }
  else if (mty == 897) { ary = (void**)array897; }
  else if (mty == 898) { ary = (void**)array898; }
  else if (mty == 899) { ary = (void**)array899; }
  else if (mty == 900) { ary = (void**)array900; }
  else if (mty == 901) { ary = (void**)array901; }
  else if (mty == 902) { ary = (void**)array902; }
  else if (mty == 903) { ary = (void**)array903; }
  else if (mty == 904) { ary = (void**)array904; }
  else if (mty == 905) { ary = (void**)array905; }
  else if (mty == 906) { ary = (void**)array906; }
  else if (mty == 907) { ary = (void**)array907; }
  else if (mty == 908) { ary = (void**)array908; }
  else if (mty == 909) { ary = (void**)array909; }
  else
  {
    return "";
  }
  int focus=0,len = strlen(str);
  for (int i = 0; i < len; i++)
  {
    Codestr[focus] = ((char**)ary)[str[2*i]-'1'][str[2*i+1]-'1'];focus++;
  }

  return Codestr;
  
}
/// <summary>
/// Packet 암호화. string enc = bizEncrypt.EncodingString("890", str);
/// </summary>
/// <param name="mty">890 ~ 909</param>
/// <param name="str">wzms_ko-KR_h00001/WorkZone01/bt77/er00^00^00/tp26.20^ox20.30^ps1018</param>
/// <returns></returns>
static char* EncodingPacketString(int mty, char *str)
{
  
  memset(Codestr,0,256);
  char rtn;
  void **ary;
  if (mty == 890) { ary = (void**)array890; }
  else if (mty == 891) { ary = (void**)array891; }
  else if (mty == 892) { ary = (void**)array892; }
  else if (mty == 893) { ary = (void**)array893; }
  else if (mty == 894) { ary = (void**)array894; }
  else if (mty == 895) { ary = (void**)array895; }
  else if (mty == 896) { ary = (void**)array896; }
  else if (mty == 897) { ary = (void**)array897; }
  else if (mty == 898) { ary = (void**)array898; }
  else if (mty == 899) { ary = (void**)array899; }
  else if (mty == 900) { ary = (void**)array900; }
  else if (mty == 901) { ary = (void**)array901; }
  else if (mty == 902) { ary = (void**)array902; }
  else if (mty == 903) { ary = (void**)array903; }
  else if (mty == 904) { ary = (void**)array904; }
  else if (mty == 905) { ary = (void**)array905; }
  else if (mty == 906) { ary = (void**)array906; }
  else if (mty == 907) { ary = (void**)array907; }
  else if (mty == 908) { ary = (void**)array908; }
  else if (mty == 909) { ary = (void**)array909; }
  else
  {
    return "";
  }
  int bufx=9,bufy=8,focus=0;
  int idx[2]={0,};
  if((int)(mty/10) == 89){bufx=8;bufy=9;}
  
  int len = strlen(str);
  for (int i = 0; i < len; i++){
    for (int i = 0; i < bufx; i++){
      for (int j = 0; j < bufy; j++){
        if (str[i] == ((char**)ary)[i][j]){
          Codestr[focus] = '1'+i;focus++;
          Codestr[focus] = '1'+j;focus++;
        }
  }}}
  return Codestr;
}


/// <summary>
/// Packet 복호화. string denc = bizEncrypt.DecodingPacketString("100", str);
/// </summary>
/// <param name="mty">100 ~ 119</param>
/// <param name="str">472337534367287571~</param>
/// <returns></returns>
static char *DecodingPacketString10(int mty, char* str)
{
  memset(Codestr,0,256);
  char* rtn = "";
  void**ary = 0;
  if (mty == 100) { ary = (void**)array100; }
  else if (mty == 101) { ary = (void**)array101; }
  else if (mty == 102) { ary = (void**)array102; }
  else if (mty == 103) { ary = (void**)array103; }
  else if (mty == 104) { ary = (void**)array104; }
  else if (mty == 105) { ary = (void**)array105; }
  else if (mty == 106) { ary = (void**)array106; }
  else if (mty == 107) { ary = (void**)array107; }
  else if (mty == 108) { ary = (void**)array108; }
  else if (mty == 109) { ary = (void**)array109; }
  else if (mty == 110) { ary = (void**)array110; }
  else if (mty == 111) { ary = (void**)array111; }
  else if (mty == 112) { ary = (void**)array112; }
  else if (mty == 113) { ary = (void**)array113; }
  else if (mty == 114) { ary = (void**)array114; }
  else if (mty == 115) { ary = (void**)array115; }
  else if (mty == 116) { ary = (void**)array116; }
  else if (mty == 117) { ary = (void**)array117; }
  else if (mty == 118) { ary = (void**)array118; }
  else if (mty == 119) { ary = (void**)array119; }
  else
  {
    return "";
  }
  int focus=0,len = strlen(str);
  int bufx=10;
  if((int)(mty/10) == 10)bufx=8;
  for (int i = 0; i < len/4; i++)
  {
    Codestr[focus++] = ((char*)ary)[(((str[4*i]-'0')*10 + (str[4*i+1]-'0'))-1)*bufx + (((str[4*i+2]-'0')*10 + (str[4*i+3]-'0'))-1)];
  }

  return Codestr;
  
}
/// <summary>
/// Packet 암호화. string enc = bizEncrypt.EncodingString("100", str);
/// </summary>
/// <param name="mty">100 ~ 119</param>
/// <param name="str">wzms_ko-KR_h00001/WorkZone01/bt77/er00^00^00/tp26.20^ox20.30^ps1018</param>
/// <returns></returns>
static char* EncodingPacketString10(int mty, char *str)
{
  
  memset(Codestr,0,256);
  char rtn;
  void **ary;
  if (mty == 100) { ary = (void**)array100; }
  else if (mty == 101) { ary = (void**)array101; }
  else if (mty == 102) { ary = (void**)array102; }
  else if (mty == 103) { ary = (void**)array103; }
  else if (mty == 104) { ary = (void**)array104; }
  else if (mty == 105) { ary = (void**)array105; }
  else if (mty == 106) { ary = (void**)array106; }
  else if (mty == 107) { ary = (void**)array107; }
  else if (mty == 108) { ary = (void**)array108; }
  else if (mty == 109) { ary = (void**)array109; }
  else if (mty == 110) { ary = (void**)array110; }
  else if (mty == 111) { ary = (void**)array111; }
  else if (mty == 112) { ary = (void**)array112; }
  else if (mty == 113) { ary = (void**)array113; }
  else if (mty == 114) { ary = (void**)array114; }
  else if (mty == 115) { ary = (void**)array115; }
  else if (mty == 116) { ary = (void**)array116; }
  else if (mty == 117) { ary = (void**)array117; }
  else if (mty == 118) { ary = (void**)array118; }
  else if (mty == 119) { ary = (void**)array119; }
  else
  {
    return "";
  }
  int bufx=10,bufy=8,focus=0;
  if((int)(mty/10) == 10){bufx=8;bufy=10;}
  
  int len = strlen(str);
  for (int h = 0; h < len; h++){
    for (int i = 0; i < bufx; i++){
      for (int j = 0; j < bufy; j++){
        if (str[h] == ((char*)ary)[j*bufx+i]){
          Codestr[focus] = '0'+(int)((j+1)/10);focus++;
          Codestr[focus] = '0'+(int)((j+1)%10);focus++;
          Codestr[focus] = '0'+(int)((i+1)/10);focus++;
          Codestr[focus] = '0'+(int)((i+1)%10);focus++;
        }
  }}}
  Codestr[focus] = 0;
  return Codestr;
}
*/
void bufferSize(uint8_t* text, int textlen, int &length)
{
  int i = textlen;
  int buf = round(i / 16) * 16;
  length = (buf <= i) ? buf + 16 : length = buf;
}
char* Enbase64(char* str, int &size, int num=0){
  if(size==0) size = strlen(str);
  if((size = encode_base64((byte*)str, size, base64[num]))==0)return 0;
  base64[num][size] = 0;
  return (char*)(base64[num]);
}
char* Debase64(char* str, int &size, int num=0){
  if((size = decode_base64((byte*)str, base64[num]))==0)return 0;
  base64[num][size] = 0;
  return (char*)(base64[num]);
}
void SetAESKeyIV(byte* key,byte* iv, int keynum = 0){
  memcpy(AESKey[keynum],key,32);
  memcpy(AESIV[keynum],iv,16);
}
char* EnAES256CBC(char* str, int &size, int num=0, int keynum=0){
  int strlength = size;
  if(strlength==0) strlength = strlen(str);
  bufferSize((byte*)str, strlength, size);
  memcpy(CryptBuf[num], str ,strlength);
  memset(CryptBuf[num]+strlength, size - strlength ,size - strlength);
  AES aes(AESKey[keynum], AESIV[keynum], AES::AES_MODE_256, AES::CIPHER_ENCRYPT);
  aes.process((uint8_t*)CryptBuf[num], CryptBuf[num], size);
  return (char*)(CryptBuf[num]);
}
char* DeAES256CBC(char* str, int &size, int num=0, int keynum=0){
  AES aes(AESKey[keynum], AESIV[keynum], AES::AES_MODE_256, AES::CIPHER_DECRYPT);
  aes.process((uint8_t*)str, (CryptBuf[num]), size);
  size = size - CryptBuf[num][size-1];
  CryptBuf[num][size] = 0;
  return (char*)(CryptBuf[num]);
}
