# writeFlashESP
writeFlashJKK and readFlashJKK

Function that allow multiple write to flash memory with a significantly reduced number of erase cycles.

NR_OF_BLOCKS = 2 (8KB), LENGHT_IN_32BIT_WORDS = 2 (8 Bytes) are 1024 write of 6 Bytes user data for one erase cycle!

Functions for ESP8266, ESP8285 for NON_OS. They should also work with FreeRTOS and Arduino. Arduino needs to be added: extern "C"

#define NR_OF_BLOCKS (2) // Blocs are 32 bit (4 byte) pices of flash memory. More blocks less erase flash and long live of module

#define LENGHT_IN_32BIT_WORDS (2) // Data block lenght by 32 bit words. Two blocks are 8 bytes (1 Magic,6 for user, 1 for crc)

1024 % LENGHT_IN_32BIT_WORDS must be 0 (LENGHT_IN_32BIT_WORDS may be: 1, 2, 4, 8)

Number of write to flash witout erase: (NR_OF_BLOCKS * 1024) / LENGHT_IN_32BIT_WORDS

NR_OF_BLOCKS = 2, LENGHT_IN_32BIT_WORDS = 2 are 1024 write of 6 bytes user data for one erase cycle!

'flash_user_data_sector_address' may by: uint32_t flash_user_data_sector_address = (SYSTEM_PARTITION_RF_CAL_ADDR / 0x1000) - NR_OF_BLOCKS;

FLASH_MAGIC_BYTE  // some characteristic number for search user data
