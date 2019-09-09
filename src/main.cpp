/*

Copyright (C) 2019 by Jaromir Kopp <macwyznawca at me dot com>

Demo of function: writeFlashESP and readFlashESP

Function that allow multiple write to flash memory with a significantly reduced number of erase cycles.
nr_of_sectors = 2 (8KB), nr_of_words = 2 (8 Bytes) are 1024 write of 6 Bytes user data for one erase cycle!

Functions for ESP8266, ESP8285 for NON_OS
They also work with FreeRTOS and Arduino.

*/

#include <Arduino.h>

#include "flashWriteMoreESP.h"

#define SPI_FLASH_SIZE_MAP 4 //2

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
//#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x06A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x081000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x0fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x0fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x0fd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x06A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x081000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x06A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x081000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x06A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x06A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 9)
#define SYSTEM_PARTITION_OTA_SIZE							0x06A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#else
#error "The flash map is not supported"
#endif

#define FLASH_MAGIC_BYTE (23)

uint8_t flashBuff[6] = {100,200,2,3,4,5};
uint8_t value = 0;

LOCAL uint32_t flash_user_data_sector_address = (SYSTEM_PARTITION_RF_CAL_ADDR / 0x1000) - 2;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.println("Write More test!");


 // writeFlashESP(flashBuff, flash_user_data_sector_address, 2, 2, FLASH_MAGIC_BYTE); // For test data read after power off commnt this!
}

void loop() {

  delay(1000);
  
  ++value;

  flashBuff[0] = 0;
  flashBuff[1] = 0;
  flashBuff[2] = 0;
  flashBuff[3] = 0;
  flashBuff[4] = 0;
  flashBuff[5] = 0;
  
	readFlashESP(flashBuff, flash_user_data_sector_address, 2, 2, FLASH_MAGIC_BYTE);

  Serial.println("");
  Serial.println("Data from flash:");
  Serial.print(flashBuff[0]);
  Serial.print("-");
  Serial.print(flashBuff[1]);
  Serial.print("-");
  Serial.print(flashBuff[2]);
  Serial.print("-");
  Serial.print(flashBuff[3]);
  Serial.print("-");
  Serial.print(flashBuff[4]);
  Serial.print("-");
  Serial.println(flashBuff[5]);
  Serial.println("");
  
	flashBuff[0] = value;

	writeFlashESP(flashBuff, flash_user_data_sector_address, 2, 2, FLASH_MAGIC_BYTE);
  
  Serial.println("");
  
}