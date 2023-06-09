/*
  EtherEEPROM.h -ported by Paolo Becchi to Esp32 from esp8266 EtherEEPROM
           -Modified by Elochukwu Ifediora <ifedioraelochukwuc@gmail.com>

  Uses a one sector flash partition defined in partition table
  OR
  Multiple sector flash partitions defined by the name column in the partition table

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROM_h
#define EEPROM_h
#ifndef EEPROM_FLASH_PARTITION_NAME
#define EEPROM_FLASH_PARTITION_NAME "eeprom"
#endif
#include <Arduino.h>
extern "C" {

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <esp_partition.h>
}

//
//   need to define AT LEAST a flash partition for EtherEEPROM  with above name
//
//           eeprom , data , 0x99, start address, 0x1000
//
class EtherEEPROMClass {
  public:
    EtherEEPROMClass(uint32_t sector);
    EtherEEPROMClass(const char* name, uint32_t user_defined_size);
    EtherEEPROMClass(void);
    ~EtherEEPROMClass(void);

    bool begin(size_t size);
    uint8_t read(int address);
    void write(int address, uint8_t val);
    uint16_t length();
    bool commit();
    void end();

    uint8_t * getDataPtr();

    template<typename T>
    T &get(int address, T &t) {
      if (address < 0 || address + sizeof(T) > _size)
        return t;

      memcpy((uint8_t*) &t, _data + address, sizeof(T));
      return t;
    }

    template<typename T>
    const T &put(int address, const T &t) {
      if (address < 0 || address + sizeof(T) > _size)
        return t;

      memcpy(_data + address, (const uint8_t*) &t, sizeof(T));
      _dirty = true;
      return t;
    }

    uint8_t readByte(int address);
	uint8_t eeprom_read_byte (int address);
    int8_t readChar(int address);
    uint8_t readUChar(int address);
    int16_t readShort(int address);
    uint16_t readUShort(int address);
    int32_t readInt(int address);
    uint32_t readUInt(int address);
    int32_t readLong(int address);
    uint32_t readULong(int address);
    int64_t readLong64(int address);
    uint64_t readULong64(int address);
    float_t readFloat(int address);
    double_t readDouble(int address);
    bool readBool(int address);
    size_t readString(int address, char* value, size_t maxLen);
    String readString(int address);
    size_t readBytes(int address, void * value, size_t maxLen);
    template <class T> T readAll (int address, T &);

    size_t writeByte(int address, uint8_t value);
    size_t writeChar(int address, int8_t value);
    size_t writeUChar(int address, uint8_t value);
    size_t writeShort(int address, int16_t value);
    size_t writeUShort(int address, uint16_t value);
    size_t writeInt(int address, int32_t value);
    size_t writeUInt(int address, uint32_t value);
    size_t writeLong(int address, int32_t value);
    size_t writeULong(int address, uint32_t value);
    size_t writeLong64(int address, int64_t value);
    size_t writeULong64(int address, uint64_t value);
    size_t writeFloat(int address, float_t value);
    size_t writeDouble(int address, double_t value);
    size_t writeBool(int address, bool value);
    size_t writeString(int address, const char* value);
    size_t writeString(int address, String value);
    size_t writeBytes(int address, const void* value, size_t len);
    template <class T> T writeAll (int address, const T &);

  protected:
    uint32_t _sector;
    uint8_t* _data;
    size_t _size;
    bool _dirty;
    const esp_partition_t * _mypart;
    const char* _name;
    uint32_t _user_defined_size;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
extern EtherEEPROMClass EtherEEPROM;
#endif

#endif
