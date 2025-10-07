
//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <config.h>

#include "mfrc531.h"
#include "iso14443ab.h"

#ifdef CLRC_CHIP
#if (ReaderType==NewRoutine)
#include <RdLib\comps\phKeyStore\src\phKeyStore_Int.h>
#include <RdLib\comps\phKeyStore\src\Rc632\phKeyStore_Rc632.h>
#endif
#endif
//========================================================================
//
//                  Mifare & MFRC531 High Level Instructions
//                   
//========================================================================

extern volatile unsigned long int timestamp;
unsigned char current_card_snr[10];

//========================================================================
unsigned char MFRC531_Reset(void)
{

		//SendByte('A');
		
		#ifdef CLRC_CHIP
		
		//SendByte('B');
    OpenRC();
    if (Mf500PcdConfig() != MI_OK)
      return 1;//SendStrLn("Init Error Level 1");
    if (PcdRfReset(1) != MI_OK)   
      return 2;  //SendStrLn("Init Error Level 2");
		#endif

    #ifndef CLRC_CHIP
    MFRC522_Init();  
		#endif
	

  return 0;  
}

//========================================================================
unsigned char MFRC531_AntennaPower(char state)
{ 

		#ifdef CLRC_CHIP
    OpenRC();
    PcdRfReset(state);
    #endif		
		
    return 0;
	
}

//========================================================================
unsigned char MFRC531_GetMFRC531ID(void)
{
  return 0;
}

//========================================================================
unsigned char MFRC531_WriteMasterKey(unsigned char Block, unsigned char* data)
{

		#ifdef CLRC_CHIP
    /*
	#if (ReaderType==NewRoutine)
phStatus_t status;
phKeyStore_Rc632_DataParams_t      KeyStore;
uint8_t pNewKey[12];
uint16_t wKeyVersion;
	
  Mf500HostCodeKey(data, pNewKey);
	
  status = phKeyStore_Rc632_SetKey(&KeyStore, Block, 0, PH_KEYSTORE_KEY_TYPE_MIFARE, pNewKey, wKeyVersion);
	#else
	*/

	OpenRC();
    if (Mf500PcdLoadKeyE2(PICC_AUTHENT1A,Block,data) != MI_OK)
    {
      //printf("\n\rF");
    }
    else
    {
      //for (i=0; i<6; i++)
      //  SendHex(Key[i]);
      //SendStrLn("");  
      //printf("\n\rE");
    } 
		
   //#endif 	
		
   #endif		
	
  return 0;
}

unsigned char data[16]; 
//========================================================================
unsigned char ISO14443_SingleTagSelect(unsigned char *snr) {
	unsigned char i;
    #if (ReaderType==NewRoutine)

    if (MFReaderStatus != MI_OK)
      return 'F';


    OpenRC();
    for (size=0; size<10; size++) current_card_snr[size] =0; 
    if (Mf500PiccRequest(PICC_REQALL, tt) != MI_OK) return 'N';
	
    if (Mf500PiccAnticoll(0, current_card_snr) != MI_OK) return 'N';  

    if (Mf500PiccSelect(current_card_snr, &size) != MI_OK) return 'N'; 
	
  
    for (i=0; i<4; i++)
      snr[i] =current_card_snr[i];
    
		return 0;
		#endif

    #ifdef CLRC_CHIP

    if (MFReaderStatus != MI_OK)
      return 'F';


    OpenRC();
    for (size=0; size<10; size++) current_card_snr[size] =0; 
    if (Mf500PiccRequest(PICC_REQALL, tt) != MI_OK) return 'N';
	
    if (Mf500PiccAnticoll(0, current_card_snr) != MI_OK) return 'N';  

    if (Mf500PiccSelect(current_card_snr, &size) != MI_OK) return 'N'; 
	
  
    for (i=0; i<4; i++)
      snr[i] =current_card_snr[i];
    
		return 0;
		#endif
		
		
    #ifndef CLRC_CHIP

	
    for (i=0; i<10; i++) current_card_snr[i] =0; 
			 	 	

    if (MFRC522_Check(current_card_snr))
	 	  return 'N';
				
			

	  if (MFRC522_SelectTag(current_card_snr) == 0)
		  return 'N';
		  
	
  	//printf("\n\r%X%X%X%X",current_card_snr[0],current_card_snr[1],current_card_snr[2],current_card_snr[3]);
  
    for (i=0; i<4; i++)
      snr[i] =current_card_snr[i];
    
		#endif
  	
	
  return 0;
}

//========================================================================
unsigned char ISO14443_LoginE2(unsigned char Sector, unsigned char Key)
{
unsigned char Keys[7][16] ={{0x89,0x51,0x71,0xf2,0x1d,0xcb},
                           {0x8D,0x5F,0x30,0x7C,0xF0,0x5E},
                           {0xB2,0x2C,0xE4,0x24,0x7D,0x90},
                           {0x57,0x9B,0x07,0xB3,0xA4,0xE9},
													 {0xe3,0x2f,0x50,0xf2,0xe3,0xd0},
													 {0xe9,0xc1,0x25,0x4f,0xa0,0x7d},
                           {0xff,0xff,0xff,0xff,0xff,0xff}};



		#ifdef CLRC_CHIP
    //printf("E");
    if ((Key>=0x10) && (Key<=0x2F))
    {
      Auth=PICC_AUTHENT1A;  
      Key -= 0x10;
    }else if ((Key>=0x30) && (Key<=0x4F))
    {
      Key -= 0x30;
      Auth=PICC_AUTHENT1B;
    }else{                
      //printf("\n\rE");
      return 1;
    }  
    
    OpenRC();
    if (Mf500PiccAuthE2(Auth,current_card_snr,Key,(Sector*4)) != MI_OK)
      if (Mf500PiccAuthE2(Auth,current_card_snr,Key,(Sector*4)) != MI_OK)
      {
        //printf("\n\rF");
        return 1;
      }
    return 0;
				
    #endif
    
	

		#ifndef CLRC_CHIP
		
  switch (Key)
	{
		case 0x10:
       return MFRC522_Auth(0x60, Sector*4, Keys[0], current_card_snr);
		case 0x37:
       return MFRC522_Auth(0x61, Sector*4, Keys[1], current_card_snr);
		case 0x14:
       return MFRC522_Auth(0x60, Sector*4, Keys[2], current_card_snr);
		case 0x35:
       return MFRC522_Auth(0x61, Sector*4, Keys[3], current_card_snr);
		case 0x39:
       return MFRC522_Auth(0x61, Sector*4, Keys[4], current_card_snr);
		case 0x33:
       return MFRC522_Auth(0x61, Sector*4, Keys[5], current_card_snr);		
		case 0x12:
       return MFRC522_Auth(0x60, Sector*4, Keys[6], current_card_snr);		
		
		default: return 1;
	}
	
	#endif
  return 1;

  
}

//========================================================================
unsigned char ISO14443_Login(unsigned char Sector, unsigned char KeyType, unsigned char *Key)
{
unsigned char Auth=0;


		#ifdef CLRC_CHIP
    switch (KeyType)
    { 
      case 0xAA:
        Auth=PICC_AUTHENT1A;  
        break;
      case 0xBB:
        Auth=PICC_AUTHENT1B;  
        break;
      default:  
        //printf("\n\rF");
        return 1;  
    };
    
    OpenRC();
    if (Mf500PiccAuthKey(Auth,current_card_snr,Key,(Sector*4)) != MI_OK)
      return 2;
    else  
      return 0;
		#else

    switch (KeyType)
    { 
      case 0xAA:
        Auth=0x60;  
        break;
      case 0xBB:
        Auth=0x61;  
        break;
      default:  
        //printf("\n\rF");
        return 1;  
    };
 
		return MFRC522_Auth(Auth, Sector*4, Key, current_card_snr);
#endif
  
}
//========================================================================
unsigned char ISO14443_ReadBlock(unsigned char Block, unsigned char *buf)
{


		#ifdef CLRC_CHIP
    OpenRC();
    if (Mf500PiccRead(Block, buf) != MI_OK)
      return 1;
		#endif

	#ifndef CLRC_CHIP
    return MFRC522_Read(Block, buf);
#endif

  
  return 0;
}

//========================================================================
unsigned char ISO14443_WriteBlock(unsigned char Block, unsigned char* data)
{

		#ifdef CLRC_CHIP
    OpenRC();
    if (Mf500PiccWrite(Block, data) != MI_OK)
      return 1;
  
    //for (i=0; i<16; i++)
    //  printf("%X",data[i]);
		 return 0;
		#else
    return MFRC522_Write(Block, data);
    #endif 
}

//========================================================================
unsigned char ISO14443_WriteValue(unsigned char Block,  signed long int Val)
{
unsigned char i;
unsigned char data[4] ={1,0,0,0};
unsigned char FormattedData[16];
unsigned char d[16]; 


		#ifdef CLRC_CHIP
		
    data[0] =Val&0xFF;
    data[1] =(Val>>8)&0xFF;
    data[2] =(Val>>16)&0xFF;
    data[3] =(Val>>24)&0xFF;
  
    FormattedData[0] =data[0];
    FormattedData[1] =data[1];
    FormattedData[2] =data[2];
    FormattedData[3] =data[3];

    FormattedData[4] =data[0]^0xFF;
    FormattedData[5] =data[1]^0xFF;
    FormattedData[6] =data[2]^0xFF;
    FormattedData[7] =data[3]^0xFF; 
  
    FormattedData[8] =data[0];
    FormattedData[9] =data[1];
    FormattedData[10] =data[2];
    FormattedData[11] =data[3];
  
    FormattedData[12] =0;
    FormattedData[13] =0xFF;
    FormattedData[14] =0;
    FormattedData[15] =0xFF;

    OpenRC();
    if (Mf500PiccWrite(Block, FormattedData) != MI_OK)
    {
      //printf("\n\rF1");
      return 1;
    }
    if (Mf500PiccRead(Block, d) != MI_OK)
    {
      //printf("\n\rF2");
      return 1;
    }
    for (i=0; i<16; i++)
      if (d[i] != FormattedData[i]) break;
    if (i<16)
    {
      //printf("\n\rF3");
      return 1;
    }  
		
		#endif


    #ifndef CLRC_CHIP
		
    data[0] =Val&0xFF;
    data[1] =(Val>>8)&0xFF;
    data[2] =(Val>>16)&0xFF;
    data[3] =(Val>>24)&0xFF;
  
    FormattedData[0] =data[0];
    FormattedData[1] =data[1];
    FormattedData[2] =data[2];
    FormattedData[3] =data[3];

    FormattedData[4] =data[0]^0xFF;
    FormattedData[5] =data[1]^0xFF;
    FormattedData[6] =data[2]^0xFF;
    FormattedData[7] =data[3]^0xFF; 
  
    FormattedData[8] =data[0];
    FormattedData[9] =data[1];
    FormattedData[10] =data[2];
    FormattedData[11] =data[3];
  
    FormattedData[12] =0;
    FormattedData[13] =0xFF;
    FormattedData[14] =0;
    FormattedData[15] =0xFF;
  
    return MFRC522_Write(Block, FormattedData);
		#endif

  return 0;
}
//========================================================================
unsigned char ISO14443_ReadValueBlock(unsigned char Block, signed long int *Val)
{
union
{
  signed long int Li;
  unsigned char buf[4];
}Transfer;
unsigned char i, data[16]; 


		#ifdef CLRC_CHIP
    OpenRC();

    if (Mf500PiccRead(Block, data) != MI_OK)
    {
      //printf("\n\rF");
      return 1;
    } 
  
    for (i=0; i<4; i++) if (data[i] != data[i+8]) { return 2; }  
    for (i=0; i<4; i++) if (data[i+4] != (data[i]^0xFF)) { return 2; }  
    //if ((data[12] != 0) || (data[13] != 0xFF) || (data[14] != 0) || (data[15] != 0xFF)) { return 2; }  
  
    Transfer.buf[0] =data[0];
    Transfer.buf[1] =data[1];
    Transfer.buf[2] =data[2];
    Transfer.buf[3] =data[3];
    *Val=Transfer.Li;
		#endif
	
    #ifndef CLRC_CHIP
    if (MFRC522_Read(Block, data) != 0) return 1;

    for (i=0; i<4; i++) if (data[i] != data[i+8]) { return 1; }  
    for (i=0; i<4; i++) if (data[i+4] != (data[i]^0xFF)) { return 1; }  
    //if ((data[12] != 0) || (data[13] != 0xFF) || (data[14] != 0) || (data[15] != 0xFF)) { return 1; }  
  
    *Val=data[3]; *Val <<= 8;
  	*Val += data[2]; *Val <<= 8;
	  *Val += data[1]; *Val <<= 8;
	  *Val += data[0];
		#endif
	

  return 0;
}

//========================================================================
unsigned char ISO14443_InternalReadValueBlock(unsigned char Block, unsigned char* value)
{
unsigned char i, data[16]; 


		#ifdef CLRC_CHIP
    OpenRC();
    if (Mf500PiccRead(Block, data) != MI_OK)
    {
      //printf("\n\rF");
      return 1;
    }

    for (i=0; i<4; i++) if (data[i] != data[i+8]) { return 1; }  
    for (i=0; i<4; i++) if (data[i+4] != (data[i]^0xFF)) { return 1; }  
    if ((data[12] != 0) || (data[13] != 0xFF) || (data[14] != 0) || (data[15] != 0xFF)) { return 1; }  
  
    value[0] =data[3];
    value[1] =data[2];
    value[2] =data[1];
    value[3] =data[0];
		#endif

  return 0;
}

//========================================================================
unsigned char ISO14443_Increment(unsigned char Block, unsigned char* data)
{
unsigned char Value[5];


		#ifdef CLRC_CHIP
    OpenRC();
    if (Mf500PiccValue(PICC_INCREMENT, Block, data, Block) != MI_OK)
    {
      //printf("\n\rF");
      return 1;
    }
    if (ISO14443_InternalReadValueBlock(Block, Value))
    {
      //printf("\n\rF");
      return 1;
    }     

    /*
    for (i=0; i<4; i++)
      if (data[i] != Value[i])
      {
        printf("\n\rI");
        return 1;
      }
    */
  
    //printf("\n\r%X%X%X%X",Value[0],Value[1],Value[2],Value[3]);
		#endif

  return 0;
}
                                 
//========================================================================
unsigned char ISO14443_Decrement(unsigned char Block, signed long int Val)
{
/*
union
{
  signed long int Li;
  unsigned char buf[4];
}Transfer;
  */
signed int i;
unsigned char data[4] ={0,0,0,0};

    data[0] =Val&0xFF;
    data[1] =(Val>>8)&0xFF;
    data[2] =(Val>>16)&0xFF;
    data[3] =(Val>>24)&0xFF;

		#ifdef CLRC_CHIP
    OpenRC();
    //Transfer.Li=Val;
    //printf("\n\r- %ld  %X %X %X %X", Transfer.Li,Transfer.buf[0],Transfer.buf[1],Transfer.buf[2],Transfer.buf[3]);
    i=Mf500PiccValue(PICC_DECREMENT, Block, data, Block);
    if (i != MI_OK)
    {
      //printf("\n\rF1 %d\n",i);
      return 1;
    }
    //if (ISO14443_InternalReadValueBlock(Block, Transfer.buf))
    //{
     //    printf("\n\rF2");
    //    return 1;
    //}     
  
    /*
    for (i=0; i<4; i++)
      if (data[i] != Value[i])
      {
        printf("\n\rI");
        return 1;
      }
    */
  
    //*Val=Transfer.Li;
		#else
		i=MFRC522_Decrement(Block,data); 
    if (i != MI_OK)
    {
      //printf("\n\rF1 %d",i);
      return 1;
    }
		#endif

  return 0;
}

//========================================================================
unsigned char ISO14443_CopyValueBlock(unsigned char Source, unsigned char Destination)
{   

		#ifdef CLRC_CHIP
    OpenRC();
    if (Mf500PiccValue(PICC_RESTORE, Source, data, Destination) != MI_OK)
    {
      //printf("\n\rF");
      return 1;
    }
    if (Mf500PiccRead(Source, d1) != MI_OK)
    {
      //printf("\n\rF");
      return 1;
    }
    if (Mf500PiccRead(Destination, d2) != MI_OK)
    {
      //printf("\n\rF");
      return 1;
    }
    for (i=0; i<16; i++)
      if (d1[i] != d2[i])
      {
        //printf("\n\rF");
        return 1;
      }
  #endif

  return 0;
}

