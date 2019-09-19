/*
writeFlashESP and readFlashESP

Function that allow multiple write to flash memory with a significantly reduced number of erase cycles.
nr_of_sectors = 2 (8KB), nr_of_words = 2 (8 Bytes) are 1024 write of 6 Bytes user data for one erase cycle!

Functions for ESP8266, ESP8285 for NON_OS
They should also work with FreeRTOS and Arduino.
Arduino needs to be added: extern "C"

Copyright (C) 2019 by Jaromir Kopp <macwyznawca at me dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*********************************************************************
* nr_of_sectors - Sectors has 1024 word (x4 byte) of flash memory (4096 Bytes). More sectors less erase flash and long live of module
* nr_of_words - Data block lenght by 32 bit words. Two words are 8 bytes (1 Magic, 6 for user, 1 for crc). Free bytes for user is ((nr_of_words * 4) - 2)
* 'sector_address' may by: uint32_t sector_address = (SYSTEM_PARTITION_RF_CAL_ADDR / 0x1000) - nr_of_sectors;
* magic_byte // some characteristic number for search user data
*********************************************************************

* nr_of_words now can be any ( 1, 2, 3, 4, 5…) les than 256.
* Number of write to flash witout erase: (nr_of_sectors * 1024) / nr_of_words
* nr_of_sectors = 2, nr_of_words = 2 are 1024 write of 6 bytes user data for one erase cycle!

*********************************************************************
!!!! You have for own data (nr_of_words * 4) - 2.                !!!!
!!!! For 2 words You have 6 bytes, for 4 words You have 14 bytes !!!!
*********************************************************************

*/

#include "flashWriteMoreESP.h"

#include "ets_sys.h"
#include "osapi.h"
#include "user_config.h"
#include "mem.h"
#include "user_interface.h"

uint8_t ICACHE_FLASH_ATTR CRC8(const uint8_t *data, uint8_t length) {
	 uint8_t crc = 0x00;
	 uint8_t extract;
	 uint8_t sum;
	 for(int i=0;i<length;i++){
			extract = *data;
			for (uint8_t tempI = 8; tempI; tempI--){
				 sum = (crc ^ extract) & 0x01;
				 crc >>= 1;
				 if (sum)
						crc ^= 0x8C;
				 extract >>= 1;
			}
			data++;
	 }
	 return crc;
}

int8_t ICACHE_FLASH_ATTR eraseFlashESP(uint32_t sector_address, uint8_t nr_of_sectors){
	for (size_t i = 0; i < nr_of_sectors; i++){
		if (spi_flash_erase_sector(sector_address + i) != SPI_FLASH_RESULT_OK){
			os_printf("Erease error!\n");
			return WRITE_MORE_EREASE_ERROR; 
		}
	}
	return WRITE_MORE_OK;
}

int8_t ICACHE_FLASH_ATTR writeFlashESP(uint8_t *data, uint32_t sector_address, uint8_t nr_of_sectors, uint8_t nr_of_words, uint8_t magic_byte){
	uint32_t addr = 0;
	uint32_t tstBuff[nr_of_words];
	uint32_t ffFF[nr_of_words];
	os_memset(ffFF,0xFF,nr_of_words * 4); // Erased flash has only 1 (set) bit (all FF). Make patern to find first free block

	uint16_t blocsInAllSectors = (1024 * nr_of_sectors) / nr_of_words; // number of all blocks in all sectors for better calculate address of last block
	
	for (uint32_t i = 0; i < 1024 * nr_of_sectors; i += nr_of_words){
		addr = (sector_address * 1024 + blocsInAllSectors * nr_of_words) - (i + nr_of_words); // start from last block
		addr *= 4; // make fisical address
		if (spi_flash_read(addr, tstBuff, (nr_of_words * 4)) == SPI_FLASH_RESULT_OK){
			if (os_memcmp(tstBuff,ffFF,(nr_of_words * 4)) == 0){ // found first free (all FF) block
				break;
			}
		}
	}
	if (addr <  (sector_address * 4096) + (nr_of_words * 4)) { // no free blocks, must erase
		if (eraseFlashESP(sector_address, nr_of_sectors) == WRITE_MORE_EREASE_ERROR){
			os_printf("Erease error! Address: %0X\n",addr);
			return WRITE_MORE_EREASE_ERROR; 
		}	
		addr = (sector_address * 1024 + blocsInAllSectors * nr_of_words) - nr_of_words;  // first (last) addres after erease
		addr *= 4; // make fisical address
	}
	uint8_t writeBuff[(nr_of_words * 4)];
    os_bzero(writeBuff,(nr_of_words * 4));
	os_memcpy(writeBuff + 1, data, (nr_of_words * 4) - 2);
    writeBuff[0] = magic_byte; // first Byte "magic" for search in read function
	writeBuff[(nr_of_words * 4) - 1] = 29; // 29 is a fake number for CRC
	uint8_t crc = CRC8(writeBuff,(nr_of_words * 4)); // CRD with gake number
	writeBuff[(nr_of_words * 4) - 1] = crc; // Set CRC into place of fake number
	if (spi_flash_write (addr, (uint32_t *)writeBuff, (nr_of_words * 4)) != SPI_FLASH_RESULT_OK){
		os_printf("Write error! Address: %0X\n",addr);
		return WRITE_MORE_ERROR;
	}
	os_printf("Write OK! Address: %0X\n",addr);
	return WRITE_MORE_OK; // return 0
}

int8_t ICACHE_FLASH_ATTR readFlashESP(uint8_t *data, uint32_t sector_address, uint8_t nr_of_sectors, uint8_t nr_of_words, uint8_t magic_byte){
	uint32_t addr = 0;
	uint8_t readBuff[(nr_of_words * 4)];
    os_bzero(readBuff,(nr_of_words * 4));
	for (uint32_t i = 0; i < 1024 * nr_of_sectors; i += nr_of_words){ // search  for first (but last write) block with user data
		addr = sector_address * 4096 + i * 4;
		if (spi_flash_read(addr, (uint32_t*)readBuff, (nr_of_words * 4)) == SPI_FLASH_RESULT_OK){
			if (readBuff[0] == magic_byte){ // if first Byte of block is "magic" ise this data
				uint8_t crc = readBuff[(nr_of_words * 4) - 1]; // czeck CRC
				readBuff[(nr_of_words * 4) - 1] = 29;
				if (CRC8(readBuff,(nr_of_words * 4)) == crc){
					os_memcpy(data,readBuff+1,(nr_of_words * 4) - 2);
					os_printf("Read OK! Address: %0X\n",addr);
					return WRITE_MORE_OK; // return 0
				}
				else {
					os_printf("Read CRC err! Address: %0X\n",addr);
				}
			}
		}
	}
	os_printf("Read data not found! Address: %0X\n",addr);
	return WRITE_MORE_NOT_FOUND_ERROR;
}