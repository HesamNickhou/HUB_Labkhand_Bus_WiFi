#include <config.h>

#ifndef ISO14443AB_H
#define ISO14443AB_H

//------------------------------------------------------------------------------
//         Constants
//------------------------------------------------------------------------------
extern unsigned char current_card_snr[10];

extern unsigned char ISO14443_SingleTagSelect(unsigned char *snr);
extern unsigned char MFRC531_WriteMasterKey(unsigned char Block, unsigned char* data);
extern unsigned char ISO14443_LoginE2(unsigned char Sector, unsigned char Key);
extern unsigned char ISO14443_Login(unsigned char Sector, unsigned char KeyType, unsigned char *Key);
extern unsigned char ISO14443_ReadBlock(unsigned char Block, unsigned char *buf);
extern unsigned char ISO14443_WriteBlock(unsigned char Block, unsigned char* data);
extern unsigned char ISO14443_WriteValue(unsigned char Block,  signed long int Val);
extern unsigned char ISO14443_ReadValueBlock(unsigned char Block, signed long int *Val);
extern unsigned char ISO14443_InternalReadValueBlock(unsigned char Block, unsigned char* value);
extern unsigned char ISO14443_Increment(unsigned char Block, unsigned char* data);
extern unsigned char ISO14443_Decrement(unsigned char Block, signed long int Val);
extern unsigned char ISO14443_CopyValueBlock(unsigned char Source, unsigned char Destination);
unsigned char ISO14443_SingleTagSelect2(unsigned char *snr);
extern unsigned char MFRC531_Reset(void);

//------------------------------------------------------------------------------
//         Types
//------------------------------------------------------------------------------


#endif
