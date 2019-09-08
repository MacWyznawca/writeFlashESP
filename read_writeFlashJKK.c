/*

writeFlashJKK and readFlashJKK

Function that allow multiple write to flash memory with a significantly reduced number of erase cycles.
NR_OF_BLOCKS = 2 (8KB), LENGHT_IN_32BIT_WORDS = 2 (8 Bytes) are 1024 write of 6 Bytes user data for one erase cycle!

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

*/

#define NR_OF_BLOCKS (2) // Blocs are 32 bit (4 byte) pices of flash memory. More blocks less erase flash and long live of module
#define LENGHT_IN_32BIT_WORDS (2) // Data block lenght by 32 bit words. Two blocks are 8 bytes (1 Magic,6 for user, 1 for crc)
// 1024 % LENGHT_IN_32BIT_WORDS must be 0 (LENGHT_IN_32BIT_WORDS may be: 1, 2, 4, 8)
// Number of write to flash witout erase: (NR_OF_BLOCKS * 1024) / LENGHT_IN_32BIT_WORDS
// NR_OF_BLOCKS = 2, LENGHT_IN_32BIT_WORDS = 2 are 1024 write of 6 bytes user data for one erase cycle!
// 'flash_user_data_sector_address' may by:
// uint32_t flash_user_data_sector_address = (SYSTEM_PARTITION_RF_CAL_ADDR / 0x1000) - NR_OF_BLOCKS;

FLASH_MAGIC_BYTE (77) // some characteristic number for search user data

int8_t ICACHE_FLASH_ATTR writeFlash(uint8_t *data){
	uint32_t addr = 0;
	uint32_t tstBuff[LENGHT_IN_32BIT_WORDS] = {0};
	uint32_t ffFF[LENGHT_IN_32BIT_WORDS];
	os_memset(ffFF,0xFF,LENGHT_IN_32BIT_WORDS * 4); 
	
	// Erased flash was only set bit (FF), patern to search first free blocks
	for (uint32_t i = 0; i < 4096 * NR_OF_BLOCKS; i += (LENGHT_IN_32BIT_WORDS * 4)){
		addr = ((flash_user_data_sector_address + NR_OF_BLOCKS) * 4096) - (i + (LENGHT_IN_32BIT_WORDS * 4)); // start from last
		if (spi_flash_read(addr, tstBuff, (LENGHT_IN_32BIT_WORDS * 4)) == SPI_FLASH_RESULT_OK){
			if (os_memcmp(tstBuff,ffFF,(LENGHT_IN_32BIT_WORDS * 4)) == 0){ // founf dirst free (all FF) block
				break;
			}
		}
	}
	if (addr <  (flash_user_data_sector_address * 4096) + (LENGHT_IN_32BIT_WORDS * 4)) { // no free blocks, must to erase
		for (size_t i = 0; i < NR_OF_BLOCKS; i++){
			if (spi_flash_erase_sector(flash_user_data_sector_address + i) != SPI_FLASH_RESULT_OK){
				os_printf("Write erease error %0X\n",addr);
				return 2; // return error
			}
		}	
		addr = ((flash_user_data_sector_address + NR_OF_BLOCKS) * 4096) - (LENGHT_IN_32BIT_WORDS * 4); // first (last) addres after erease
	}
	uint8_t writeBuff[(LENGHT_IN_32BIT_WORDS * 4)] = {0};
	os_memcpy(writeBuff + 1, data, (LENGHT_IN_32BIT_WORDS * 4) - 2);
	writeBuff[0] = FLASH_MAGIC_BYTE; // first Byte "magic" for search in read function
	writeBuff[(LENGHT_IN_32BIT_WORDS * 4) - 1] = 29; // 29 is a fake number for CRC
	uint8_t crc = CRC8(writeBuff,(LENGHT_IN_32BIT_WORDS * 4)); // CRD with gake number
	writeBuff[(LENGHT_IN_32BIT_WORDS * 4) - 1] = crc; // Set CRC into place of fake number
	if (spi_flash_write (addr, (uint32_t *)writeBuff, (LENGHT_IN_32BIT_WORDS * 4)) != SPI_FLASH_RESULT_OK){
		os_printf("Write search addr writ err %0X\n",addr);
		return 1;
	}
	os_printf("Write search addr OK! %0X\n",addr);
	return 0;
}

int8_t ICACHE_FLASH_ATTR readFlash(uint8_t *data){
	uint32_t addr = 0;
	uint8_t tstBuff[(LENGHT_IN_32BIT_WORDS * 4)] = {0};
	for (uint32_t i = 0; i < 4096 * NR_OF_BLOCKS; i += (LENGHT_IN_32BIT_WORDS * 4)){ // search  for first (but last write) block with user data
		addr = (flash_user_data_sector_address * 4096) + i;
		if (spi_flash_read(addr, (uint32_t*)tstBuff, (LENGHT_IN_32BIT_WORDS * 4)) == SPI_FLASH_RESULT_OK){
			if (tstBuff[0] == FLASH_MAGIC_BYTE){ // if first Byte of block is "magic" ise this data
				uint8_t crc = tstBuff[(LENGHT_IN_32BIT_WORDS * 4) - 1]; // czeck CRC
				tstBuff[(LENGHT_IN_32BIT_WORDS * 4) - 1] = 29;
				if (CRC8(tstBuff,(LENGHT_IN_32BIT_WORDS * 4)) == crc){
					os_memcpy(data,tstBuff+1,(LENGHT_IN_32BIT_WORDS * 4) - 2);
					os_printf("Read search addr OK! %0X\n",addr);
					return 0;
				}
				else {
					os_printf("Read search addr CRC err %0X\n",addr);
				}
			}
		}
	}
	os_printf("Read search addr not found %0X\n",addr);
	return -1;
}
