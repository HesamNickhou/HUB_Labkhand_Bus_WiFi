#ifndef FIRMWARE_H
#define FIRMWARE_H

#include "Config.h"

#define BinPageSize 512

unsigned char CheckNewFirmware(void);
void ProcessFirmwarePacket(void);

typedef struct {
	unsigned char HaveData;
	char FirmwareFileName[31];
	unsigned int Length;
	unsigned int CheckSum;
	unsigned short SavedVer; 
	unsigned short SavedRelease; 
} FirmwareStruct;

extern FirmwareStruct      Firmware;
extern int InfoCounter; 
extern unsigned char 
	Which_DeviceFirmware,
	ActivePort,
	CheckFirmware,
	BD90_UpdateFirmwareFlag;

extern unsigned int BufferLen;

#endif