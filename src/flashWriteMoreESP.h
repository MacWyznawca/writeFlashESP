/*
writeFlashESP and readFlashESP

Function that allow multiple write to flash memory with a significantly reduced number of erase cycles.
nr_of_sectors = 2 (8KB), nr_of_words = 2 (8 Bytes) are 1024 write of 6 Bytes user data for one erase cycle!

Functions for ESP8266, ESP8285 for NON_OS
They should also work with FreeRTOS and Arduino.

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
* nr_of_sectors - Blocs are 32 bit (4 byte) pices of flash memory. More blocks less erase flash and long live of module
* nr_of_words - Data block lenght by 32 bit words. Two blocks are 8 bytes (1 Magic,6 for user, 1 for crc)
* 'sector_address' may by: uint32_t sector_address = (SYSTEM_PARTITION_RF_CAL_ADDR / 0x1000) - nr_of_sectors;
* magic_byte // some characteristic number for search user data
*********************************************************************

* 1024 % nr_of_words must be 0 (nr_of_words may be: 1, 2, 4, 8)
* Number of write to flash witout erase: (nr_of_sectors * 1024) / nr_of_words
* nr_of_sectors = 2, nr_of_words = 2 are 1024 write of 6 bytes user data for one erase cycle!

*********************************************************************
!!!! You have for data (nr_of_words * 4) - 2.                    !!!!
!!!! For 2 words You have 6 bytes, for 4 words You have 14 bytes !!!!
*********************************************************************

*/

#ifndef _FLASH_WRITE_MORE_H
#define _FLASH_WRITE_MORE_H

#include "os_type.h"

#define WRITE_MORE_OK (0)
#define WRITE_MORE_ERROR (-1)
#define WRITE_MORE_EREASE_ERROR (-2)
#define WRITE_MORE_NOT_FOUND_ERROR (-3)

#ifdef __cplusplus
extern "C"
{
#endif

int8_t writeFlashESP(uint8_t *data, uint32_t sector_address, uint8_t nr_of_sectors, uint8_t nr_of_words, uint8_t magic_byte);
int8_t readFlashESP(uint8_t *data, uint32_t sector_address, uint8_t nr_of_sectors, uint8_t nr_of_words, uint8_t magic_byte);


#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif // _FLASH_WRITE_MORE_H