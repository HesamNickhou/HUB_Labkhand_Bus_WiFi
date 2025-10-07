//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
//#ifdef ECard

#include "ecard.h"
#include <Config.h>
#include <rtc/rtc.h>
#include <eiaes/eiaes.h>
#include <crc/crc16.h>
#include <crc/crc7.h>
#include <stdio.h>


//------------------------------------------------------------------------------
//         Internal constants
//------------------------------------------------------------------------------
unsigned char CreditSector=5;
unsigned char CreditBlock=20;
unsigned char CreditLogin=0x37;
unsigned char SOLUTION=0; 

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------
//#include <utility/eiaes.h>

#define MAXSUSPENDCARDS 2
#define OLD_MAXSUSPENDCARDS 5

TECardInfo CurECardInfo;
TECardInfo SuspendECardsInfo[MAXSUSPENDCARDS];
TECardInfo Old_SuspendECardsInfo[OLD_MAXSUSPENDCARDS];
TCardOP CardOPs[15];

unsigned long int CardIDThatProcessed=0;
unsigned char Old_RestoreIndex(void);
unsigned char Old_RestoreEtebar(void);  
//extern void ShowCardHoldMessage(void);

char LastStep=0;

//==========================================================================
//    Prototypes
//==========================================================================
void getCoderType() {
	switch (CoderType) {
    case ctTaxi:
      CreditSector = 5;
      CreditBlock  = 21;
      CreditLogin  = 0x37;
      SOLUTION 		 = 0x20; 
      break;
		
    case ctSABA:
      CreditSector = 4;
      CreditBlock  = 16;
      CreditLogin  = 0x39;
      SOLUTION 		 = 0; 
      break;
		
    case ctParyaft:
      CreditSector = 5;
      CreditBlock  = 21;
      CreditLogin  = 0x37;
      SOLUTION 		 = 0x40; 
      break;
		
    default:  
      CreditSector = 5;
      CreditBlock  = 20;
      CreditLogin  = 0x37;
      SOLUTION 		 = 0; 
      break;
  }
}

unsigned char Old_GetBlockIndex(unsigned char Idx) {
  switch (Idx) {
    case 0:  return 25;
    case 1:  return 26;
    case 2:  return 28;
    case 3:  return 29;
    case 4:  return 30;
    case 5:  return 32;
    case 6:  return 33;
    case 7:  return 34;
    case 8:  return 36;
    case 9:  return 37;
    case 10: return 38;
    case 11: return 40;
    case 12: return 41;
    case 13: return 42;
  }
}
//==========================================================================
void GenerateKey(unsigned char snr[4], unsigned char Key[16]) {
  Key[0]  = snr[1]^0x12;
  Key[1]  = crc16_table[snr[0]];
  Key[2]  = snr[2]^0xA1;
  Key[3]  = crc7_syndrome_table[snr[1]];
  Key[4]  = snr[0]^0x3F;
  Key[5]  = crc7_syndrome_table[snr[2]];
  Key[6]  = crc7_syndrome_table[snr[3]];
  Key[7]  = crc16_table[snr[3]];
  Key[8]  = snr[3]^0x8D;
  Key[9]  = crc16_table[snr[1]];
  Key[10] = crc7_syndrome_table[snr[0]];
  Key[11] = crc7_syndrome_table[snr[2]] ^ crc7_syndrome_table[snr[1]];
  Key[12] = 4;
  Key[13] = snr[0];
  Key[14] = crc16_table[snr[2]];
  Key[15] = 48;
}

//==========================================================================
unsigned char EncryptBlock(unsigned char snr[4], unsigned char Buf[16]) {
	unsigned long int ExpandedKey[44];
	unsigned char Key[16];

  GenerateKey(snr, Key);
  ExpandAESKeyForEncryption(Key, ExpandedKey);
  EncryptAES(Buf, ExpandedKey, Buf);
  return 0;
}

//==========================================================================
unsigned char DecryptBlock(unsigned char snr[4], unsigned char Buf[16]) {
	unsigned long int ExpandedKey[44];
	unsigned char i,Key[16];
  GenerateKey(snr, Key);
  ExpandAESKeyForEncryption(Key, ExpandedKey);
  ExpandAESKeyForDecryption(ExpandedKey);
  DecryptAES(Buf, ExpandedKey, Buf);
  return 0;
}

//==========================================================================
unsigned char GetCardEtebar(signed int *Etebar) {
	signed long int Payment, PreEtebar;
	unsigned char BCC, i, OP,_CoderType=ctNone;
	unsigned char snr[8];
	unsigned char buf[32];
	unsigned char IsOldFormat;
	signed long int Value;
	signed long int TempEtebar=0;

	*Etebar=0;
  getCoderType();

  if (ISO14443_SingleTagSelect(snr)) 
    if (ISO14443_SingleTagSelect(snr)) 
      return S_TimeOut;
	
  for (i=0; i<4; i++)
    CurECardInfo.CardSerialNumber[i] =snr[i];
   
  if (ISO14443_LoginE2(0,0x10)) 
    return S_TimeOut;

  if (ISO14443_ReadBlock(1, buf)) return S_TimeOut; 
  
  CurECardInfo.UC=buf[1];  CurECardInfo.UC<<=8;
  CurECardInfo.UC += buf[0];
  CurECardInfo.CardID=buf[5];  CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[4]; CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[3]; CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[2];
  if ((CurECardInfo.UC & 0x8000)==0x8000)
  {
    CurECardInfo.UC = CurECardInfo.UC & 0x7FFF;
    CurECardInfo.UC = CurECardInfo.UC ^ 0x6A29;
    CurECardInfo.CardID = CurECardInfo.CardID ^ 0xB2A6;
  }
  CardIDThatProcessed=CurECardInfo.CardID;
  
	

	
	
  if (ISO14443_LoginE2(CreditSector,CreditLogin)) return 10;
  if (ISO14443_ReadBlock(CreditBlock, buf)) return 20;  
  for (i=0;i<16; i++)
    CurECardInfo.EtebarBlock[i] =buf[i];
    
  DecryptBlock(CurECardInfo.CardSerialNumber, CurECardInfo.EtebarBlock);
    
  IsOldFormat=1;  
  for (i=0; i<4; i++) if (buf[i]!=buf[i+8]) IsOldFormat=0;  
  for (i=0; i<4; i++) if (buf[i+4]!=(buf[i]^0xFF)) IsOldFormat=0;  
  if ((buf[12]!=0) || (buf[13]!=0xFF) || (buf[14]!=0) || (buf[15]!=0xFF)) IsOldFormat=0;
	
  if (IsOldFormat)
  {                                      
    CurECardInfo.Etebar=buf[3]; CurECardInfo.Etebar<<=8; 
    CurECardInfo.Etebar += buf[2]; CurECardInfo.Etebar<<=8; 
    CurECardInfo.Etebar += buf[1]; CurECardInfo.Etebar<<=8; 
    CurECardInfo.Etebar += buf[0]; 

    if (ISO14443_LoginE2(6,0x14)) return 12;   
    if (ISO14443_ReadValueBlock(24, &Value)) 
      return 13;
    CurECardInfo.OPIndex=Value;
    
    *Etebar=CurECardInfo.Etebar;
      
    return 0;
  }

  BCC=0x4B;
  for (i=0; i<15; i++)
    BCC^=CurECardInfo.EtebarBlock[i];
  if (BCC!=CurECardInfo.EtebarBlock[15])
  {       
    return 3;
  }
  
  CurECardInfo.Etebar=CurECardInfo.EtebarBlock[3]; CurECardInfo.Etebar<<=8;    
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[2]; CurECardInfo.Etebar<<=8;    
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[1]; CurECardInfo.Etebar<<=8;    
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[0];

  Payment=CurECardInfo.EtebarBlock[9]; Payment<<=8;    
  Payment += CurECardInfo.EtebarBlock[8];

  PreEtebar=CurECardInfo.EtebarBlock[7]; PreEtebar<<=8;    
  PreEtebar += CurECardInfo.EtebarBlock[6]; PreEtebar<<=8;    
  PreEtebar += CurECardInfo.EtebarBlock[5]; PreEtebar<<=8;    
  PreEtebar += CurECardInfo.EtebarBlock[4];
  
  OP=CurECardInfo.EtebarBlock[10];
  
  switch (OP&0x0F)
  {
    case Increment:
      TempEtebar=PreEtebar+Payment;
      if (TempEtebar!=CurECardInfo.Etebar)
        return 5;     
      break;
    case Decrement:
      TempEtebar=PreEtebar-Payment;
      if (TempEtebar!=CurECardInfo.Etebar)
        return 6;     
      break;
    default:
      return 7;  
  }
    
  CurECardInfo.OPIndex=CurECardInfo.EtebarBlock[11] & 0x1F;    
  *Etebar=CurECardInfo.Etebar;

  return 0;        
}

//==========================================================================
unsigned char GetBlockIndex(unsigned char Idx) {
	switch (Idx) {
		case 0: return 44;
		case 1: return 45;
		case 2: return 46;
		case 3: return 48;
		case 4: return 49;
		case 5: return 50;
		case 6: return 52;
		case 7: return 53;
		case 8: return 54;
	}
}

//==========================================================================
unsigned char ReadCardOPs(void) {
	unsigned char Block, idx, BCC, i, NumOPs;
	unsigned char snr[10], buf[32];
	TCardOP CardOP;

  for (idx=0; idx<9; idx++)
  {
    CardOPs[idx].Date[0] =0;
    CardOPs[idx].Date[1] =0;
    CardOPs[idx].Date[2] =0;
    CardOPs[idx].Time[0] =0;
    CardOPs[idx].Time[1] =0;
    CardOPs[idx].OP=0;
    CardOPs[idx].PreEtebar=0;
    CardOPs[idx].Payment=0;
    CardOPs[idx].Etebar=0;
    CardOPs[idx].OPIndex=0;
    CardOPs[idx].DeviceID=0;
  }

  NumOPs=0;
  for (idx=0; idx<9; idx++)
  {
    //printf("\n\rRead Index %d", idx);
    Block=GetBlockIndex(idx);
    for (i=0; i<10; i++)
    {        
      if (i)
      {
        if (ISO14443_SingleTagSelect(snr)) continue;                
        for (BCC=0; BCC<4; BCC++)
          if (CurECardInfo.CardSerialNumber[BCC]!=snr[BCC])
            return 2;
      }
      if (ISO14443_LoginE2(Block/4,0x35)) continue;                
      if (ISO14443_ReadBlock(Block, buf)) continue;
      break;
    }
    if (i>=10) return 1; 

    DecryptBlock(CurECardInfo.CardSerialNumber, buf);
    BCC=0x8A; 
    for (i=0; i<15; i++)
      BCC^=buf[i];
    if (BCC!=buf[15])
    {
      //printf(" BCC Error");
      continue;
    }         
      
    //SendStrLn(" OK");
    CardOPs[NumOPs].Date[0] =(buf[0]>>4)+90;
    CardOPs[NumOPs].Date[1] =buf[0] & 0x0F;
    CardOPs[NumOPs].Date[2] =buf[2] & 0x1F;
    CardOPs[NumOPs].Time[0] =buf[1]>>3;
    CardOPs[NumOPs].Time[1] =((buf[1]&0x07)<<3)+(buf[2] & 0x07);
    CardOPs[NumOPs].OP=buf[3];
    CardOPs[NumOPs].PreEtebar=buf[6]; CardOPs[NumOPs].PreEtebar<<=8;
    CardOPs[NumOPs].PreEtebar += buf[5]; CardOPs[NumOPs].PreEtebar<<=8;
    CardOPs[NumOPs].PreEtebar += buf[4];
    CardOPs[NumOPs].Payment=buf[8]; CardOPs[NumOPs].Payment<<=8;
    CardOPs[NumOPs].Payment += buf[7];
    CardOPs[NumOPs].Etebar=buf[11]; CardOPs[NumOPs].Etebar<<=8;
    CardOPs[NumOPs].Etebar += buf[10]; CardOPs[NumOPs].Etebar<<=8;
    CardOPs[NumOPs].Etebar += buf[9];
    CardOPs[NumOPs].OPIndex=buf[12]&0x1F;  
    CardOPs[NumOPs].DeviceID=buf[14]; CardOPs[NumOPs].DeviceID<<=8;
    CardOPs[NumOPs].DeviceID += buf[13];  
    NumOPs++;
    //printf(" > %d/%d/%d %d:%d %d +- %d = %d",CardOPs[idx].Date[0],CardOPs[idx].Date[1],CardOPs[idx].Date[2],CardOPs[idx].Time[0],CardOPs[idx].Time[1],CardOPs[idx].PreEtebar,CardOPs[idx].Payment,CardOPs[idx].Etebar);
  }      
  //Set sort indexes
  for (idx=0; idx<NumOPs; idx++)
    if (CardOPs[idx].OPIndex==0)
    {
      for (i=0; i<9; i++)
        if (CardOPs[i].OPIndex<9)
          CardOPs[i].OPIndex += 18;
      break;
    }
  idx=0; 
  
  //Sorting
  while (1)
  {       
    if (CardOPs[idx+1].OPIndex>CardOPs[idx].OPIndex)
    {
      CardOP=CardOPs[idx];
      CardOPs[idx] =CardOPs[idx+1];
      CardOPs[idx+1] =CardOP;
      idx=0;
      continue;
    }
    idx++;
    if (idx>=NumOPs) break;          
  }    
  //Reset sort indexes
  for (idx=0; idx<NumOPs; idx++)
    if (CardOPs[idx].OPIndex>=18)
      CardOPs[idx].OPIndex -= 18;

  //printf("\n\rSotring...");
  //for (idx=0; idx<9; idx++)
  //  printf("\n\r%d> %d/%d/%d %d:%d %d +- %d = %d",idx,CardOPs[idx].Date[0],CardOPs[idx].Date[1],CardOPs[idx].Date[2],CardOPs[idx].Time[0],CardOPs[idx].Time[1],CardOPs[idx].PreEtebar,CardOPs[idx].Payment,CardOPs[idx].Etebar);
  
  return 0;

}

//==========================================================================
unsigned char RestoreEtebar(void)  
{
unsigned char i, BCC;
unsigned char EtebarBlock[16];
  
  if (ReadCardOPs())
    return 1;
    
  if (CardOPs[0].OP==0) //Card have not any OP 
  {
    return 2;
  }
  
  EtebarBlock[0] =(CardOPs[0].Etebar&0xFF);
  EtebarBlock[1] =((CardOPs[0].Etebar>>8)&0xFF);
  EtebarBlock[2] =((CardOPs[0].Etebar>>16)&0xFF);
  EtebarBlock[3] =((CardOPs[0].Etebar>>24)&0xFF);
  EtebarBlock[4] =(CardOPs[0].PreEtebar&0xFF);
  EtebarBlock[5] =((CardOPs[0].PreEtebar>>8)&0xFF);
  EtebarBlock[6] =((CardOPs[0].PreEtebar>>16)&0xFF);
  EtebarBlock[7] =((CardOPs[0].PreEtebar>>24)&0xFF);
  EtebarBlock[8] =(CardOPs[0].Payment&0xFF);
  EtebarBlock[9] =((CardOPs[0].Payment>>8)&0xFF);
  EtebarBlock[10] =CardOPs[0].OP;
  EtebarBlock[11] =CardOPs[0].OPIndex;
  EtebarBlock[12] =0;
  EtebarBlock[13] =0;
  EtebarBlock[14] =0;
  BCC=0x4B;
  for (i=0; i<15; i++)
    BCC^=EtebarBlock[i];
  EtebarBlock[15] =BCC;  
  EncryptBlock(CurECardInfo.CardSerialNumber, EtebarBlock);   

  if (ISO14443_LoginE2(CreditSector,CreditLogin)) return 3;
  if (ISO14443_WriteBlock(CreditBlock, EtebarBlock)) return 4;  
  return 0;
   
}

//==========================================================================
unsigned char EmptyECardInfo(TECardInfo *CardInfo)  
{
unsigned char i;

  (*CardInfo).CardID=0;
  (*CardInfo).UC=0;
  (*CardInfo).Etebar=0;
  (*CardInfo).Payment=0;
  (*CardInfo).PreEtebar=0;
  (*CardInfo).OP=0;
  (*CardInfo).OPIndex=0;
  (*CardInfo).DeviceID=0;
  (*CardInfo).Hour=0;
  (*CardInfo).Min=0;  
  for (i=0; i<16; i++)
    (*CardInfo).EtebarBlock[i] =0;
  for (i=0; i<16; i++)
    (*CardInfo).OPBlock[i] =0;  
  for (i=0; i<4; i++)
    (*CardInfo).CardSerialNumber[i] =0;  
    
  return 0;
}
//==========================================================================
unsigned char ReadECardInfo(void) {
	unsigned char 
		BCC, i,
		buf[32], snr[10],
		IsOldFormat;
	
	signed long int 
		Value,
		TempEtebar = 0,
		Payment,
		PreEtebar;
	
	unsigned int 
		_ID,
		_UC;

	if (ISO14443_SingleTagSelect(snr))
		if (ISO14443_SingleTagSelect(snr))  
			return 17;

  if (ISO14443_LoginE2(0, 0x10)) 
    if (ISO14443_LoginE2(0, 0x10))
      return 17;

  if (ISO14443_ReadBlock(1, buf)) 
		return 18;   

   GenerateCardIdUC(buf, snr, &(_UC), &(_ID));
	
   CurECardInfo.CardID = _ID;
   CurECardInfo.UC 		 = _UC;

		
		/*
  CurECardInfo.UC=buf[1];  CurECardInfo.UC<<=8;
  CurECardInfo.UC += buf[0];
  CurECardInfo.CardID=buf[5];  CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[4]; CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[3]; CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[2];
  if ((CurECardInfo.UC & 0x8000)==0x8000)
  {
    CurECardInfo.UC = CurECardInfo.UC & 0x7FFF;
    CurECardInfo.UC = CurECardInfo.UC ^ 0x6A29;
    CurECardInfo.CardID = CurECardInfo.CardID ^ 0xB2A6;
  }
	
	*/
  CardIDThatProcessed = CurECardInfo.CardID;
  
  if (CurECardInfo.UC == 251) 
    return S_SettingCard;
  
  if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
		return 10;
  
  if (ISO14443_ReadBlock(CreditBlock, buf)) 
		return 20;  
  for (i=0; i<16; i++)
    CurECardInfo.EtebarBlock[i] = buf[i];
    
  DecryptBlock(CurECardInfo.CardSerialNumber, CurECardInfo.EtebarBlock);
    
  IsOldFormat = 1;  
  for (i=0; i<4; i++) 
		if (buf[i] != buf[i+8]) 
			IsOldFormat = 0;  
  for (i=0; i<4; i++) 
		if (buf[i+4] != (buf[i] ^ 0xFF)) 
			IsOldFormat = 0;  
  if ((buf[12] != 0) || (buf[13] != 0xFF) || (buf[14] != 0) || (buf[15] != 0xFF)) 
		IsOldFormat = 0;
  if ((CurECardInfo.UC 		 == 6374) && 
			(CurECardInfo.CardID >= 30001) && 
			(CurECardInfo.CardID <= 40000))
    IsOldFormat = 0;
	
  if (IsOldFormat) {
    CurECardInfo.Etebar  = buf[3]; CurECardInfo.Etebar <<= 8; 
    CurECardInfo.Etebar += buf[2]; CurECardInfo.Etebar <<= 8; 
    CurECardInfo.Etebar += buf[1]; CurECardInfo.Etebar <<= 8; 
    CurECardInfo.Etebar += buf[0]; 

    if (ISO14443_LoginE2(6, 0x14)) 
			return 12;                
    if (ISO14443_ReadValueBlock(24, &Value)) {
      if (!Old_RestoreIndex())
        return 55;
      else  
        return 13;
    }
    CurECardInfo.OPIndex = Value;
    
    if (Value == 0)
      Value = 13;
    else
      Value--;
   
    i = Old_GetBlockIndex(Value);
    if (ISO14443_LoginE2(i / 4, 0x35)) 
			return 13;                
    if (ISO14443_ReadBlock(i, buf)) 
			return 14;
    CurECardInfo.OP = buf[5];
    CurECardInfo.DeviceID  = buf[12]; CurECardInfo.DeviceID <<= 8;
    CurECardInfo.DeviceID += buf[11];
    return 90;
  }

  BCC = 0x4B;
  for (i=0; i<15; i++)
    BCC ^= CurECardInfo.EtebarBlock[i];
  if (BCC != CurECardInfo.EtebarBlock[15]) {       
    //SendStrLn("Restore...");      
    BCC = RestoreEtebar();
    if (BCC) { 
      if (BCC == 2) { //May Card Be Old
        if (!Old_RestoreEtebar())
          return 44;
      }
      return 3;
    }
		else 
      return 4; //Restore Succeed
  }
  
  CurECardInfo.Etebar  = CurECardInfo.EtebarBlock[3]; CurECardInfo.Etebar <<= 8;
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[2]; CurECardInfo.Etebar <<= 8;
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[1]; CurECardInfo.Etebar <<= 8;
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[0];

  Payment  = CurECardInfo.EtebarBlock[9]; Payment <<= 8;
  Payment += CurECardInfo.EtebarBlock[8];

  PreEtebar  = CurECardInfo.EtebarBlock[7]; PreEtebar <<= 8;
  PreEtebar += CurECardInfo.EtebarBlock[6]; PreEtebar <<= 8;
  PreEtebar += CurECardInfo.EtebarBlock[5]; PreEtebar <<= 8;
  PreEtebar += CurECardInfo.EtebarBlock[4];
  
  CurECardInfo.OP = CurECardInfo.EtebarBlock[10];

  CurECardInfo.DeviceID  = CurECardInfo.EtebarBlock[14]; 
	CurECardInfo.DeviceID <<= 8;
  CurECardInfo.DeviceID += CurECardInfo.EtebarBlock[13];
  
  switch (CurECardInfo.OP & 0x0F) {
    case Increment:
      TempEtebar = PreEtebar+Payment;
      if (TempEtebar != CurECardInfo.Etebar)
        return 5;
      break;
    case Decrement:
      TempEtebar = PreEtebar-Payment;
      if (TempEtebar != CurECardInfo.Etebar)
        return 6;
      break;
    default:
      RestoreEtebar();
      return 7;
  }
    
  CurECardInfo.OPIndex = CurECardInfo.EtebarBlock[11] & 0x1F;
  return 0;
}
//==========================================================================
unsigned char PrepairECardBuffer(void) {
	unsigned char i, BCC = 0x4B;
  
  CurECardInfo.EtebarBlock[0] =(CurECardInfo.Etebar&0xFF);
  CurECardInfo.EtebarBlock[1] =((CurECardInfo.Etebar>>8)&0xFF);
  CurECardInfo.EtebarBlock[2] =((CurECardInfo.Etebar>>16)&0xFF);
  CurECardInfo.EtebarBlock[3] =((CurECardInfo.Etebar>>24)&0xFF);
  CurECardInfo.EtebarBlock[4] =(CurECardInfo.PreEtebar&0xFF);
  CurECardInfo.EtebarBlock[5] =((CurECardInfo.PreEtebar>>8)&0xFF);
  CurECardInfo.EtebarBlock[6] =((CurECardInfo.PreEtebar>>16)&0xFF);
  CurECardInfo.EtebarBlock[7] =((CurECardInfo.PreEtebar>>24)&0xFF);
  CurECardInfo.EtebarBlock[8] =(CurECardInfo.Payment&0xFF);
  CurECardInfo.EtebarBlock[9] =((CurECardInfo.Payment>>8)&0xFF);
  CurECardInfo.EtebarBlock[10] =CurECardInfo.OP;
  CurECardInfo.EtebarBlock[11] =CurECardInfo.OPIndex;
  CurECardInfo.EtebarBlock[12] =rand();
  CurECardInfo.EtebarBlock[13] =CurECardInfo.DeviceID&0xFF;      
  CurECardInfo.EtebarBlock[14] =(CurECardInfo.DeviceID>>8)&0xFF;
  for (i=0; i<15; i++)
    BCC^=CurECardInfo.EtebarBlock[i];
  CurECardInfo.EtebarBlock[15] =BCC;  
  EncryptBlock(CurECardInfo.CardSerialNumber, CurECardInfo.EtebarBlock);   
  
  CurECardInfo.OPBlock[0] =((Year-1390)<<4)+(Month&0x0F);
  CurECardInfo.OPBlock[1] =(Min>>3);
  CurECardInfo.OPBlock[1]|=(Hour<<3);
  CurECardInfo.OPBlock[2] =Min&0x07;
  CurECardInfo.OPBlock[2]<<=5;
  CurECardInfo.OPBlock[2]|=Day;
  CurECardInfo.OPBlock[3] =CurECardInfo.OP;
  CurECardInfo.OPBlock[4] =(CurECardInfo.PreEtebar&0xFF);
  CurECardInfo.OPBlock[5] =((CurECardInfo.PreEtebar>>8)&0xFF);
  CurECardInfo.OPBlock[6] =((CurECardInfo.PreEtebar>>16)&0xFF);
  CurECardInfo.OPBlock[7] =(CurECardInfo.Payment&0xFF);
  CurECardInfo.OPBlock[8] =((CurECardInfo.Payment>>8)&0xFF);
  CurECardInfo.OPBlock[9] =(CurECardInfo.Etebar&0xFF);
  CurECardInfo.OPBlock[10] =((CurECardInfo.Etebar>>8)&0xFF);
  CurECardInfo.OPBlock[11] =((CurECardInfo.Etebar>>16)&0xFF);
  CurECardInfo.OPBlock[12] =SOLUTION;
  CurECardInfo.OPBlock[12]&=0xE0;
  CurECardInfo.OPBlock[12]|=CurECardInfo.OPIndex;
  CurECardInfo.OPBlock[13] =CurECardInfo.DeviceID&0xFF;      
  CurECardInfo.OPBlock[14] =(CurECardInfo.DeviceID>>8)&0xFF;  
  BCC=0x8A;   
  for (i=0; i<15; i++)
    BCC^=CurECardInfo.OPBlock[i];
  CurECardInfo.OPBlock[15] =BCC;      
  EncryptBlock(CurECardInfo.CardSerialNumber, CurECardInfo.OPBlock);   
  
  return 0;
}

//==========================================================================
unsigned char WriteTransaction(void) {
	unsigned char Block;
	unsigned char buf[32];

  Block = GetBlockIndex(CurECardInfo.OPIndex%9);
  if (ISO14443_LoginE2(Block/4, 0x35)) 
		return 1;   
  if (ISO14443_WriteBlock(Block, CurECardInfo.OPBlock)) 
		return 2;
  if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
		return 3;
  if (ISO14443_WriteBlock(CreditBlock, CurECardInfo.EtebarBlock)) 
		return 4;
  if (ISO14443_ReadBlock(CreditBlock, buf)) 
		return 5;
  for (Block=0; Block<16; Block++)
    if (CurECardInfo.EtebarBlock[Block] != buf[Block])
      return 6;
  return 0;
}

unsigned char ExecTransaction(unsigned char      Type,      
															unsigned long int *CardID,     
															unsigned int  		*UC,      
															unsigned int       Payment,
                              signed   long int  MinEtebar, 
															unsigned long int  MaxEtebar,  
															unsigned int  		 DeviceID, 
															signed 	 long int *Etebar,    
															signed 	 long int *PreEtebar, 
															unsigned short    *LastDevice, 
															unsigned char 		*LastOP,  
															unsigned char     *OPIndex) {

  //========================= Variables ========================================
	unsigned int SelectTimeout = 0 , _CoderType = ctNone;
	unsigned char snr[10];
	unsigned char Res, i, j, ShowMsg, CardIsSuspend = 0;
	signed int Diff;
	signed long int TempEtebar;
 //=============================================================================
	
  getCoderType();  
  ShowMsg = 0;
  //=====================================
  // STEP 1
  // Prepair card for transaction
  // Reading Card Informations 
  //=====================================
  *Etebar    = 0;                                                 
  *PreEtebar = 0;
  while (SelectTimeout < 20) {
	  WDTR;
		if (ISO14443_SingleTagSelect(snr))
			if (ISO14443_SingleTagSelect(snr)) {
				SelectTimeout++; 
				continue; 
			}
 
    ShowMsg++;       
    SelectTimeout = 0;
    for (i = 0; i<4; i++)
      CurECardInfo.CardSerialNumber[i] = snr[i];
		
    Res = ReadECardInfo(); 	
    if (Res) {           
      *CardID = CurECardInfo.CardID;
      switch (Res) {
        case 4:
          return S_Restore;
        case 90:
          return S_IsOldFormat;
        case 44:
          return S_RestoreOld;
        case 55:
          return S_RestoreIndex;
        case S_SettingCard:
          *UC     = CurECardInfo.UC;        
          *CardID = CurECardInfo.CardID;
          return S_SettingCard;
        default:
          continue;
      }
    }

    if (*CardID)
      if (CurECardInfo.CardID != *CardID)
        return S_InvalidCardID;
    *CardID = CurECardInfo.CardID;
    if (CurECardInfo.UC != *UC) {
			if ((Config.UC == 6375) && (CurECardInfo.UC == 2790)) {
				if ((CurECardInfo.CardID >= 29500) && (CurECardInfo.CardID <= 32800))
					*UC = 6375;
			}
			else {
				*UC = CurECardInfo.UC;
				return S_InvalidCardUC;
			}
    }
    *UC = CurECardInfo.UC;        
 
    //Checking for suspended card 
    for (j = 0; j < MAXSUSPENDCARDS; j++) {
      for (i = 0; i < 4; i++)
        if (CurECardInfo.CardSerialNumber[i] != SuspendECardsInfo[j].CardSerialNumber[i])
          break;   
        
      if ((i>=4) && (j<MAXSUSPENDCARDS)) {
        Diff = (((Hour * 60) + Min) - ((SuspendECardsInfo[j].Hour * 60) + SuspendECardsInfo[j].Min));  
        if (Diff < 0) 
					Diff *= -1; 
        if (Diff < 10) {//Retry write card 
          CurECardInfo = SuspendECardsInfo[j];
          CardIsSuspend = 1;
        }
        EmptyECardInfo(&SuspendECardsInfo[j]);
      }
    }
    break; //Normally done while
  }
  if (SelectTimeout >= 20)
    return S_TimeOut;
	
  *LastOP 		= CurECardInfo.OP;
  *LastDevice = CurECardInfo.DeviceID;
  *Etebar     = CurECardInfo.Etebar;  
  if (CardIsSuspend == 0) {
    CurECardInfo.PreEtebar = CurECardInfo.Etebar;  
    CurECardInfo.Payment 	 = Payment;  
    CurECardInfo.OP 			 = Type;  
    CurECardInfo.Hour 		 = Hour;
    CurECardInfo.Min 			 = Min;
    CurECardInfo.DeviceID  = DeviceID;
    CurECardInfo.OPIndex++;
		
    if (CurECardInfo.OPIndex >= 18)
      CurECardInfo.OPIndex = 0;
    *OPIndex = CurECardInfo.OPIndex;
    switch (Type & 0x0F) {              
      case Increment:
        TempEtebar = (CurECardInfo.Etebar+Payment);      
        if (TempEtebar > MaxEtebar)
          return S_OutOfRange;
        CurECardInfo.Etebar += Payment;  
        break;
      case Decrement:
        #if (DeviceType == BUSDOOR)
					if (CurECardInfo.Etebar < Payment)
						return S_OutOfRange;
					if (CurECardInfo.Etebar < Payment) {
						Payment = CurECardInfo.Etebar;
						CurECardInfo.Payment = Payment;  
					}
        #else
					TempEtebar = CurECardInfo.Etebar - Payment;
					if (TempEtebar < MinEtebar)
						return S_OutOfRange;
        #endif
        if (CurECardInfo.Etebar > MaxEtebar)
          return S_OutOfRange;
        CurECardInfo.Etebar -= Payment;  
        break;
				
      default:
        return S_InvalidType;
    }
    PrepairECardBuffer();
  }

  //=====================================
  // STEP 1
  // Perform transaction
  // Write Data 
  //=====================================
	
  if (ShowMsg == 1) 
		ShowMsg = 0;  
  SelectTimeout = 0;  
  while (SelectTimeout < 20) {
    if (ShowMsg == 1) 
      //ShowCardHoldMessage();
    ShowMsg++;
		WDTR;
    if (ShowMsg > 1) {  
      if (ISO14443_SingleTagSelect(snr)) { 
				SelectTimeout++; 
				continue; 
			}  
      for (i = 0; i < 4; i++)
        if (CurECardInfo.CardSerialNumber[i] != snr[i])
          return S_InvalidSerial;
    }

    SelectTimeout = 0;
    Res = WriteTransaction();
    if (Res > 1) 
			CardIsSuspend = 1;
    if (Res) 
			continue;            
    CardIsSuspend = 0;
    break;
  }//while 

  if (CardIsSuspend) {
    for (i = 0; i < MAXSUSPENDCARDS; i++)
      if (SuspendECardsInfo[i].CardID == 0)
        break;
    if (i >= MAXSUSPENDCARDS)
      i = 0;
    SuspendECardsInfo[i] = CurECardInfo;
  }

  if (SelectTimeout >= 20)
    return S_TimeOut;
     
  *Etebar    = CurECardInfo.Etebar;
  *PreEtebar = CurECardInfo.PreEtebar; 
  return S_Success;
}

//==========================================================================
unsigned char WriteOnce(void) {
	unsigned int SelectTimeout=0;
	unsigned char snr[10];
	unsigned char Res, i, j, ShowMsg, CardIsSuspend=0;
	signed int Diff;
	signed long int TempEtebar;

  ShowMsg=0;
  //=====================================
  // STEP 1
  // Prepair card for transaction
  // Reading Card Informations 
  //=====================================

  CurECardInfo.Etebar=40000;  
  CurECardInfo.PreEtebar=0;  
  CurECardInfo.Payment=40000;  
  CurECardInfo.OP=1;  
  CurECardInfo.Hour=Hour;
  CurECardInfo.Min=Min;
  CurECardInfo.DeviceID=1;
  CurECardInfo.OPIndex=0;
  if (ISO14443_SingleTagSelect(snr)) 
    if (ISO14443_SingleTagSelect(snr)) 
      return 1;
  for (i=0; i<4; i++)
    CurECardInfo.CardSerialNumber[i] =snr[i];
  //=====================================
  // STEP 1
  // Perform transaction
  // Write Data 
  //=====================================
  PrepairECardBuffer();

  if (ShowMsg==1) ShowMsg=0;  
  SelectTimeout=0;  
  while (SelectTimeout<10)
  {  
    if (ShowMsg==1) 
      //ShowCardHoldMessage();
    ShowMsg++;
    if (ShowMsg>1)
      if (ISO14443_SingleTagSelect(snr)) 
      { 
        SelectTimeout++; 
        continue; 
      }  

    SelectTimeout=0;
    Res=WriteTransaction();
    if (Res) continue;            
    break;
  }//while 
  
  if (SelectTimeout>=10)
    return 2;

  return S_Success;
}

//==========================================================================
//==========================================================================
//                        Old E-Card Routines
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
unsigned char GetOpBlockByIndex(unsigned char idx)
{
  switch (idx)
  {
    case 0: return 25;
    case 1: return 26;
    case 2: return 28;
    case 3: return 29;
    case 4: return 30;
    case 5: return 32;
    case 6: return 33;
    case 7: return 34;
    case 8: return 36;
    case 9: return 37;
    case 10: return 38;
    case 11: return 40;
    case 12: return 41;
    case 13: return 42;
  }
  return 0;
}

//==========================================================================
unsigned char Old_ReadCardOPs(void)      ///??????????????????????????????
{
unsigned char Block, idx, BCC, i;
unsigned char snr[10];
unsigned char buf[32];
TCardOP CardOP;

  for (idx=0; idx<15; idx++)
  {
    CardOPs[idx].Date[0] =0;
    CardOPs[idx].Date[1] =0;
    CardOPs[idx].Date[2] =0;
    CardOPs[idx].Time[0] =0;
    CardOPs[idx].Time[1] =0;
    CardOPs[idx].OP=0;
    CardOPs[idx].PreEtebar=0;
    CardOPs[idx].Payment=0;
    CardOPs[idx].Etebar=0;
    CardOPs[idx].OPIndex=0;
    CardOPs[idx].DeviceID=0;
  }

  for (idx=0; idx<15; idx++)
  {
    //SendStr("Read Index "); SendNum(idx);
    Block=Old_GetBlockIndex(idx);
    for (i=0; i<10; i++)
    {        
      if (i)
      {
        if (ISO14443_SingleTagSelect(snr)) continue;                
        for (BCC=0; BCC<4; BCC++)
          if (CurECardInfo.CardSerialNumber[BCC]!=snr[BCC])
            return 2;
      }
      if (ISO14443_LoginE2(Block/4,0x35)) continue;                
      if (ISO14443_ReadBlock(Block, buf)) continue;
      break;
    }
    if (i>=10) return 1; 
    //DecryptBlock(CurECardInfo.CardSerialNumber, buf);
    BCC=0x8A; 
    for (i=0; i<15; i++)
      BCC^=buf[i];
    if (BCC!=buf[15])
    {
      //SendStrLn(" BCC Error");
      continue;
    }         
    //SendStrLn(" OK");
    CardOPs[idx].Date[0] =(buf[0]>>4)+90;
    CardOPs[idx].Date[1] =buf[0] & 0x0F;
    CardOPs[idx].Date[2] =buf[2] & 0x1F;
    CardOPs[idx].Time[0] =buf[1]>>3;
    CardOPs[idx].Time[1] =((buf[1]&0x07)<<3)+(buf[2] & 0x07);
    CardOPs[idx].OP=buf[3];
    CardOPs[idx].PreEtebar=buf[6]; CardOPs[idx].PreEtebar<<=8;
    CardOPs[idx].PreEtebar += buf[5]; CardOPs[idx].PreEtebar<<=8;
    CardOPs[idx].PreEtebar += buf[4];
    CardOPs[idx].Payment=buf[8]; CardOPs[idx].Payment<<=8;
    CardOPs[idx].Payment += buf[7];
    CardOPs[idx].Etebar=buf[11]; CardOPs[idx].Etebar<<=8;
    CardOPs[idx].Etebar += buf[10]; CardOPs[idx].Etebar<<=8;
    CardOPs[idx].Etebar += buf[9];
    CardOPs[idx].OPIndex=buf[12];  
    CardOPs[idx].DeviceID=buf[14]; CardOPs[idx].DeviceID<<=8;
    CardOPs[idx].DeviceID += buf[13];  
    /*
    SendNum(CardOPs[idx].Date[0]);
    SendNum(CardOPs[idx].Date[1]);
    SendNum(CardOPs[idx].Date[2]);
    SendNum(CardOPs[idx].Time[0]);
    SendNum(CardOPs[idx].Time[1]);
    SendNum(CardOPs[idx].PreEtebar);
    SendNum(CardOPs[idx].Payment);
    SendNum(CardOPs[idx].Etebar);
    SendNum(CardOPs[idx].OPIndex);
    SendNum(CardOPs[idx].DeviceID);    
    SendStrLn("");
    */
  }      

  for (idx=0; idx<15; idx++)
    if (CardOPs[idx].OPIndex==0)
    {
      for (i=0; i<15; i++)
        if (CardOPs[i].OPIndex<15)
          CardOPs[i].OPIndex += 30;
      break;
    }
  idx=0;        
  while (1)
  {       
    if (CardOPs[idx+1].OPIndex>CardOPs[idx].OPIndex)
    {
      CardOP=CardOPs[idx];
      CardOPs[idx] =CardOPs[idx+1];
      CardOPs[idx+1] =CardOP;
      idx=0;
      continue;
    }
    idx++;
    if (idx>=14) break;          
  } 
  for (idx=0; idx<15; idx++)
    if (CardOPs[idx].OPIndex>=30)
      CardOPs[idx].OPIndex -= 30;
     
  /*
  SendStrLn("Sotring...");
  for (idx=0; idx<15; idx++)
  {
    SendStr("Idx");
    SendNum(idx);
    SendNum(CardOPs[idx].Date[0]);
    SendNum(CardOPs[idx].Date[1]);
    SendNum(CardOPs[idx].Date[2]);
    SendNum(CardOPs[idx].Time[0]);
    SendNum(CardOPs[idx].Time[1]);
    SendNum(CardOPs[idx].PreEtebar);
    SendNum(CardOPs[idx].Payment);
    SendNum(CardOPs[idx].Etebar);
    SendNum(CardOPs[idx].OPIndex);
    SendNum(CardOPs[idx].DeviceID);    
    SendStrLn("");
  } 
  */
  
  
  return 0;

}

//==========================================================================
unsigned char Old_RestoreEtebar(void) {
	int i;
	signed long int loc, Etebar;
	unsigned char buf[32];
	unsigned char Buffer[16];
	unsigned char Res, j, BCC=0, Block;  

  //Recovery
  if (ISO14443_LoginE2(6,0x14))
    return 1;                
  if (Res=ISO14443_ReadValueBlock(24, &loc))
  {
    if (Res==2) 
    {
      Old_RestoreIndex();
      return 1;   
    }
  }else{
    if (loc>13)
    {
      Old_RestoreIndex();
      return 1;
    }  
    if (loc==0)
      loc=13;
    else
      loc--;  
  }//if (MSR_ReadVa  
  
  
  //SendStr("Loc="); SendNum(loc); SendStrLn("");
  Block=GetOpBlockByIndex(loc);
  //SendStr("Block="); SendNum(Block); SendStrLn("");
  if (ISO14443_LoginE2(Block/4,0x35))
    return 8;                
  //SendStrLn("Read");
  if (ISO14443_ReadBlock(Block, buf))
    return 9;
  //SendStrLn("OK");
  BCC=0;
  for (i=0; i<15; i++)
    BCC^=buf[i]; 
  //SendStr("BCC="); SendNum(ReaderReceiveBuf[18]); SendStrLn("");
  //SendStr("CalcBCC="); SendNum(BCC); SendStrLn("");
  if (buf[15]!=BCC)
    {
     Old_RestoreIndex();
     return 10;
   }       

  Etebar=buf[10]; Etebar<<=8;
  Etebar += buf[9]; Etebar<<=8;
  Etebar += buf[8]; 
              
  //SendStr("Etebar="); SendNum(Etebar); SendStrLn("");
  if (ISO14443_LoginE2(CreditSector,CreditLogin)) { return 11; }
  if (ISO14443_WriteValue(CreditBlock,Etebar)) { return 12; }
  return 0;
}

//==========================================================================
unsigned char Old_RestoreIndex(void)  
{
int i;
unsigned char buf[32];
unsigned char loc;
unsigned char CurYear, CurMonth, CurDay, CurHour, CurMin;
unsigned char OldYear=0, OldMonth=0, OldDay=0, OldHour=0, OldMin=0;
unsigned char j, BCC=0, Block; 
unsigned long int CurEtebar, OldRemEtebar=0; 

  loc=200;
  for (i=0; i<14; i++)
  {              
    Block=Old_GetBlockIndex(i);
    if (ISO14443_LoginE2(Block/4,0x35)) return 3;                
    if (ISO14443_ReadBlock(Block, buf)) return 4;
    BCC=0;
    for (j=0; j<15; j++)
      BCC^=buf[j]; 
    if ((buf[15] =BCC) || (BCC==0) || (BCC==0xFF)) //Invalid OP 
      continue;

    CurYear=(buf[6]>>4)+86;
    CurMonth=buf[6] & 0x0F;
    CurDay=buf[7];
    CurHour=buf[13];
    CurMin=buf[14];   

    CurEtebar=buf[4]; CurEtebar<<=8;
    CurEtebar += buf[3]; CurEtebar<<=8;
    CurEtebar += buf[2];
    
    if ((CurMonth>12) || (CurDay>31) || (CurHour>23) || (CurMin>59))
      continue;

    if (CurYear>OldYear) //Year
      loc=i;
    else if (CurYear==OldYear) //Year
    {
      if (CurMonth>OldMonth)  //Month
        loc=i;
      else if (CurMonth==OldMonth)
      {
        if (CurDay>OldDay) //Day
          loc=i;
        else if (CurDay==OldDay)
        {
          if (CurHour>OldHour)
            loc=i;          
          else if (CurHour==OldHour)
          {
            if (CurMin>OldMin)
              loc=i;
            else if (CurMin==OldMin)
            {
              if (OldRemEtebar==CurEtebar)
                loc=i;
            }  
          }  
        }
      }  
    }    
    
    if (i==loc)
    {
      OldYear=CurYear;
      OldMonth=CurMonth;
      OldDay=CurDay;
      OldHour=CurHour;
      OldMin=CurMin;  
      OldRemEtebar=buf[10]; OldRemEtebar<<=8;
      OldRemEtebar += buf[9]; OldRemEtebar<<=8;
      OldRemEtebar += buf[8];
    }   
  }//for 

  if (loc==200)
    loc=13;   
    
  loc++;
  if (loc>13) loc=0;  

  if (ISO14443_LoginE2(6,0x35))
    return 1;                
  if (ISO14443_WriteValue(24, loc))
    return 1; 

  return 0;
}

//==========================================================================
unsigned char Old_ReadECardInfo(void) {
	unsigned char BCC, i,snr[10];	
	unsigned char buf[32];
	signed long int Value;
	unsigned int _ID,_UC;

	if (ISO14443_SingleTagSelect(snr))
		if (ISO14443_SingleTagSelect(snr))  
			return 1;
	
  if (ISO14443_LoginE2(0, 0x10)) 
		return 1;
  if (ISO14443_ReadBlock(1, buf)) 
		return 2;
	
  GenerateCardIdUC(buf,snr,&(_UC),&(_ID));
	
  CurECardInfo.CardID = _ID;
  CurECardInfo.UC 		= _UC;
	/*
	
  	
  CurECardInfo.UC=buf[1];  CurECardInfo.UC<<=8;
  CurECardInfo.UC += buf[0];
  CurECardInfo.CardID=buf[5];  CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[4]; CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[3]; CurECardInfo.CardID<<=8;
  CurECardInfo.CardID += buf[2];
  if ((CurECardInfo.UC & 0x8000)==0x8000)
  {
    CurECardInfo.UC = CurECardInfo.UC & 0x7FFF;
    CurECardInfo.UC = CurECardInfo.UC ^ 0x6A29;
    CurECardInfo.CardID = CurECardInfo.CardID ^ 0xB2A6;
  }
  */
	
	
  CardIDThatProcessed = CurECardInfo.CardID;
  
  if (ISO14443_LoginE2(6, 0x14)) return 10;                
  if (ISO14443_ReadValueBlock(24, &Value)) return 11;
  CurECardInfo.OPIndex=Value;

  if (Value==0)
    Value=13;
  else
    Value--;
   
  i=Old_GetBlockIndex(Value);
  if (ISO14443_LoginE2(i/4,0x35)) return 13;                
  if (ISO14443_ReadBlock(i, buf)) return 14;
  CurECardInfo.OP=buf[5];
  CurECardInfo.DeviceID=buf[12]; CurECardInfo.DeviceID<<=8;
  CurECardInfo.DeviceID += buf[11];     

  if (ISO14443_LoginE2(CreditSector,CreditLogin)) return 12;
  if (ISO14443_ReadValueBlock(CreditBlock, &CurECardInfo.Etebar))  
  {       
    //SendStrLn("Restore...");
    if (Old_RestoreEtebar()) 
      return 3;
    else 
      return 4; //Restore Succeed
  }
  
  return 0;        
  
}

//==========================================================================
unsigned char Old_PrepairECardBuffer(void)
{
unsigned char i, BCC;

  CurECardInfo.OPBlock[0] =CurECardInfo.Payment&0x00FF;
  CurECardInfo.OPBlock[1] =(CurECardInfo.Payment>>8);
  CurECardInfo.OPBlock[2] =CurECardInfo.PreEtebar&0x000000FF;
  CurECardInfo.OPBlock[3] =(CurECardInfo.PreEtebar>>8)&0x000000FF;
  CurECardInfo.OPBlock[4] =(CurECardInfo.PreEtebar>>16)&0x000000FF;
  CurECardInfo.OPBlock[5] =CurECardInfo.OP;
  CurECardInfo.OPBlock[6] =((Year-1386)<<4)+(Month&0x0F);
  CurECardInfo.OPBlock[7] =Day;
  CurECardInfo.OPBlock[8] =CurECardInfo.Etebar&0x000000FF;
  CurECardInfo.OPBlock[9] =(CurECardInfo.Etebar>>8)&0x000000FF;
  CurECardInfo.OPBlock[10] =(CurECardInfo.Etebar>>16)&0x000000FF;
  CurECardInfo.OPBlock[11] =CurECardInfo.DeviceID&0xFF;      
  CurECardInfo.OPBlock[12] =(CurECardInfo.DeviceID>>8)&0xFF;  
  CurECardInfo.OPBlock[13] =Hour;  
  CurECardInfo.OPBlock[14] =Min;  
  BCC=0;   
  for (i=0; i<15; i++)
    BCC^=CurECardInfo.OPBlock[i];
  CurECardInfo.OPBlock[15] =BCC;  

  
  return 0;
}
//==========================================================================
unsigned char Old_WriteTransaction(void)
{
unsigned char Block;
signed long int Etebar;
unsigned char snr[10];


  Block=Old_GetBlockIndex(CurECardInfo.OPIndex);
  //printf("\n\rOld_WriteTransaction OPIndex=%d  LastStep=%d Block=%d",CurECardInfo.OPIndex,LastStep, Block); 
  switch (LastStep)
        {
         case 0:

                LastStep=0;
                if (ISO14443_LoginE2(Block/4,0x35)) return 1;
                if (ISO14443_WriteBlock(Block,CurECardInfo.OPBlock))  return 2;

                LastStep=1;
                if (ISO14443_LoginE2(6,0x35))
                  {
                   if (ISO14443_SingleTagSelect(snr))
                     {
                      if (ISO14443_SingleTagSelect(snr))
                         return 3;
                     }
                   if (ISO14443_LoginE2(6,0x35)) {return 3;}
                  }
                Block=CurECardInfo.OPIndex+1;
                if (Block>13)
                   Block=0;  
                if (ISO14443_WriteValue(24, Block)) return 4;
                
                LastStep=2;
                if (ISO14443_LoginE2(CreditSector,CreditLogin))
                  {
                   if (ISO14443_SingleTagSelect(snr))
                     {
                      if (ISO14443_SingleTagSelect(snr))
                         return 5;
                     }
                   if (ISO14443_LoginE2(CreditSector,CreditLogin)) return 5;
                  }                
                if (ISO14443_WriteValue(CreditBlock, CurECardInfo.Etebar)) return 6;
                LastStep=3;
                if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) return 7;
                if (Etebar!=CurECardInfo.Etebar)
                  return 8; 
                LastStep=0;
            
                break;
         //....................................................................................................................................................... 
         case 1:
     
                LastStep=1;
                if (ISO14443_LoginE2(6,0x35)) {return 3;}
                Block=CurECardInfo.OPIndex+1;
                if (Block>13)
                   Block=0;  
                if (ISO14443_WriteValue(24, Block)) return 4;
                
                LastStep=2;
                if (ISO14443_LoginE2(CreditSector,CreditLogin))
                  {
                   if (ISO14443_SingleTagSelect(snr))
                     {
                      if (ISO14443_SingleTagSelect(snr))
                         return 5;
                     }
                   if (ISO14443_LoginE2(CreditSector,CreditLogin)) return 5;
                  }                 
                if (ISO14443_WriteValue(CreditBlock, CurECardInfo.Etebar)) return 6;
                LastStep=3;
                if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) return 7;
                if (Etebar!=CurECardInfo.Etebar)
                  return 8;  
                LastStep=0;
                             
                break;
         //.......................................................................................................................................................                
         case 2:
      
                LastStep=2;
                if (ISO14443_LoginE2(CreditSector,CreditLogin)) return 5;
                if (ISO14443_WriteValue(CreditBlock, CurECardInfo.Etebar)) return 6;
                LastStep=3;
                if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) return 7;
                if (Etebar!=CurECardInfo.Etebar)
                 return 8;  

                LastStep=0;
                break;
         //....................................................................................................................................................... 
         case 3:
       
                LastStep=3;
                if (ISO14443_LoginE2(CreditSector,CreditLogin)) return 5;                
                if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) return 7;
                if (Etebar!=CurECardInfo.Etebar)
                  return 8;
                LastStep=0;
           
                break;
         //.......................................................................................................................................................                 
       }
  

  return 0;    
}

//==============================================================================
unsigned char Old_ExecTransaction(unsigned char 		Type, 
																	unsigned long int *CardID, 
																	unsigned 			int *UC, 
																	unsigned 			int Payment,
                                  signed 	 long int MinEtebar, 
																	unsigned long int MaxEtebar, 
																	unsigned 			int DeviceID, 
                                  signed 	 long int *Etebar, 
																	signed 	 long int *PreEtebar, 
																	unsigned short 		*LastDevice, 
																	unsigned char 		*LastOP, 
																	unsigned char 		*OPIndex) {
	//=============================== Variables ==================================
	unsigned int 
		Grouh,
		SelectTimeout = 0;
	
	unsigned char 
		Res, 
		j, 
		i, 
		snr[10],
		buf[32],
		ShowMsg, 
		FirstLoop 		= 1,
		CardIsSuspend = 0;
	
	signed int Diff;
	signed long int TempEtebar = 0;

	//=============================== Source Codes ===============================
  getCoderType();	
	*Etebar  	 = 0;
  LastStep 	 = 0;
  ShowMsg    = 0;
	*PreEtebar = 0;
  //Prepair card for transaction                                                 
  
  while (SelectTimeout < 5) {      
    //if (ShowMsg==1)
      //ShowCardHoldMessage();
    ShowMsg++; 

    if (!FirstLoop) {
      if (ISO14443_SingleTagSelect(snr))
				if (ISO14443_SingleTagSelect(snr)) {
					SelectTimeout++; 
					continue; 
				}

      SelectTimeout = 0;
      for (i=0; i<4; i++)
        CurECardInfo.CardSerialNumber[i] = snr[i];
      
      if (Res = Old_ReadECardInfo()) {
        *CardID = CurECardInfo.CardID;
        switch (Res) {
          case 4:
            return S_RestoreOld;
          default:
            continue;
        }
      }
			*Etebar    = CurECardInfo.Etebar;
			*PreEtebar = CurECardInfo.PreEtebar;  
			return 0;
	
    }//if (!FirstLoop   
    
    FirstLoop = 0;

    if (*CardID)
      if (CurECardInfo.CardID != *CardID)
        return S_InvalidCardID;
    *CardID = CurECardInfo.CardID; 
    
    if (*UC == 5436) {
      if ((CurECardInfo.UC != 5436) && (CurECardInfo.UC != 7584)) {
        *UC = CurECardInfo.UC;
        if (CurECardInfo.UC == 251)
          return S_SettingCard;
        else  
          return S_InvalidCardUC;
      }
    }    

    if (CurECardInfo.UC != *UC) {
      if ((Config.UC == 6375) && (CurECardInfo.UC == 2790)) {
         if ((CurECardInfo.CardID >= 29500) && (CurECardInfo.CardID <= 32800))          
            *UC = 6375;
			}
      else {
				*UC = CurECardInfo.UC;        
				if (CurECardInfo.UC == 251)
					return S_SettingCard;
				else  
					return S_InvalidCardUC;
			}
    }
    *UC 				= CurECardInfo.UC;
    *LastOP 		= CurECardInfo.OP;
    *LastDevice = CurECardInfo.DeviceID;
    *OPIndex 		= CurECardInfo.OPIndex;
			
    if (*CardID == 0)
      continue;    

    //Checking for suspended card 
    for (j=0; j<OLD_MAXSUSPENDCARDS; j++) {  
      for (i=0; i<4; i++)
        if (CurECardInfo.CardSerialNumber[i] != 
					Old_SuspendECardsInfo[j].CardSerialNumber[i])
          break;  
      
      if (i >= 4) {
        Diff = (((Hour * 60) + Min) - 
					((Old_SuspendECardsInfo[j].Hour * 60) 
					+ Old_SuspendECardsInfo[j].Min));
       
        if (Diff < 0) 
					Diff *= -1; 
        if (Diff < 10) {//Retry write card
          CardIsSuspend = 1;
          CurECardInfo = Old_SuspendECardsInfo[j];
        }
        EmptyECardInfo(&Old_SuspendECardsInfo[j]);
        break;
      }
    }
    break; //Normally done while
  }
  

  if (SelectTimeout >= 5)
    return S_TimeOut;
  
  if (CardIsSuspend == 0) { 
    *Etebar 							 = CurECardInfo.Etebar;  
    CurECardInfo.PreEtebar = CurECardInfo.Etebar;  
    CurECardInfo.Payment 	 = Payment;  
    CurECardInfo.OP 			 = Type;
    CurECardInfo.Hour 		 = Hour;
    CurECardInfo.Min 			 = Min;
    CurECardInfo.DeviceID  = DeviceID;
    if (CurECardInfo.OPIndex > 13) 
			CurECardInfo.OPIndex = 0;
    switch (Type & 0x0F) { 
			
      case Increment:
        TempEtebar = CurECardInfo.Etebar + Payment;
        if (TempEtebar > MaxEtebar)
          return S_OutOfRange;
        CurECardInfo.Etebar += Payment;  
        break;

      case Decrement:
        TempEtebar = CurECardInfo.Etebar - Payment;
        if (TempEtebar<MinEtebar)
          return S_OutOfRange;
        if (TempEtebar > 0)
          if (TempEtebar > MaxEtebar)
            return S_OutOfRange;
        CurECardInfo.Etebar -= Payment;  
        break;

      default:
        return S_InvalidType;
    }
    Old_PrepairECardBuffer();
  }
        
  if (ShowMsg == 1) 
		ShowMsg = 0;  
  SelectTimeout = 0;    
  //Perform transaction
  while (SelectTimeout < 20) {
	  WDTR;
    //if (ShowMsg==1) 
      //ShowCardHoldMessage(); 
    ShowMsg++;

    if (ShowMsg > 1) {
      if (ISO14443_SingleTagSelect(snr))
				if (ISO14443_SingleTagSelect(snr)){ 
					SelectTimeout++; 
					continue; 
				}
      if ((snr[0] == 0) && (snr[1] == 0) && (snr[2] == 0) && (snr[3] == 0)) { 
				SelectTimeout++; 
				continue; 
			}
      for (i=0; i<4; i++)
        if (CurECardInfo.CardSerialNumber[i] != snr[i])
          break;
      if (i < 4) {
        if (CardIsSuspend == 0)
          return S_InvalidSerial;
        CardIsSuspend = S_InvalidSerial;
        break;
      }
    }

    SelectTimeout = 0;
    Res = Old_WriteTransaction();
			if (Res > 1) 
				CardIsSuspend = 1;
    if (Res) 
			continue;
    CardIsSuspend = 0;
    break;
  }//while  

	if (*CardID == 0)
		CardIsSuspend = 1;
      
  *Etebar = CurECardInfo.Etebar;
  *PreEtebar = CurECardInfo.PreEtebar;  
  if (CardIsSuspend) {                 
    for (i=0; i<OLD_MAXSUSPENDCARDS; i++) {
			if (Old_SuspendECardsInfo[i].CardID == 0)
				break;
		}
    if (i >= OLD_MAXSUSPENDCARDS)
      i = 0;

    Old_SuspendECardsInfo[i] = CurECardInfo;
    if (CardIsSuspend == S_InvalidSerial)
      return S_InvalidSerial;
    else
      return S_CardSuspend;
  }
  
  if (SelectTimeout >= 20)
    return (S_TimeOut2 + CardIsSuspend); 
       
  for (j=0; j<OLD_MAXSUSPENDCARDS; j++)
    if (CurECardInfo.CardID == Old_SuspendECardsInfo[j].CardID)
      EmptyECardInfo(&Old_SuspendECardsInfo[j]);

  return S_Success;
}

//#endif