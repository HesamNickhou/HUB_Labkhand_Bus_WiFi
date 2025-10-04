#ifndef ECARD_H
#define ECARD_H


#define Increment          1
#define Decrement          2

#define S_Success          0
#define S_TimeOut          1
#define S_Restore          2
#define S_OutOfRange       3
#define S_InvalidSerial    4
#define S_InvalidType      5
#define S_IsOldFormat      6
#define S_BlackList        7
#define S_SuspendOperation 8
#define S_InvalidCardID    9
#define S_InvalidCardUC    10
#define S_SettingCard      11
#define S_TimeOut2         12
#define S_CardSuspend      13
#define S_RestoreOld       14
#define S_RestoreIndex     15
#define S_BajedarCard      16
#define S_ChargeCard       17
#define S_Cancel           18

typedef struct {
  unsigned char CardSerialNumber[4];
  unsigned long int CardID;
  unsigned int UC;  
  unsigned char OPIndex;
  unsigned char OP;
  unsigned int  DeviceID;  
  unsigned char  Hour, Min;  
  unsigned int  Payment;  
  signed long int Etebar;  
  signed long int PreEtebar;  
  unsigned char EtebarBlock[16];  
  unsigned char OPBlock[16];  
}TECardInfo;

typedef struct {
  unsigned char OPIndex;
  unsigned char OP;
  unsigned int  DeviceID;  
  unsigned char Date[3];  
  unsigned char Time[2];  
  unsigned int  Payment;  
  signed long int Etebar;  
  signed long int PreEtebar;
  unsigned char BCC;
}TCardOP;

extern unsigned char CreditSector;
extern unsigned char CreditBlock;
extern unsigned char CreditLogin;
extern unsigned char SOLUTION; 

extern unsigned char ExecTransaction(unsigned char Type, unsigned long int *CardID, unsigned int *UC, unsigned int Payment,
                                    signed long int MinEtebar, unsigned long int MaxEtebar, unsigned int DeviceID, 
                                    signed long int *Etebar, signed long int *PreEtebar, unsigned short *LastDevice, unsigned char *LastOP, unsigned char *OPIndex);

unsigned char Old_ExecTransaction(unsigned char Type, unsigned long int *CardID, unsigned int *UC, unsigned int Payment,
                                    signed long int MinEtebar, unsigned long int MaxEtebar, unsigned int DeviceID, 
                                    signed long int *Etebar, signed long int *PreEtebar, unsigned short *LastDevice, unsigned char *LastOP, unsigned char *OPIndex);

extern unsigned char GetCardEtebar(signed int *Etebar);
#endif //#ifndef ECARD_H

