#include <config.h>

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------


#include <includes.h>
#include "mfrc531.h"
#include <string.h>

#define	uchar	unsigned char
#define	uint	unsigned int

#define DEBUG 0

//------------------------------------------------------------------------------
//         Local constants
//------------------------------------------------------------------------------


//=====================================================================
//
//                  MFRC531 ISO 14443AB Foundation
//
//=====================================================================

//=====================================================================
//                          Statics
//=====================================================================
// disable reader interrupt
//#define READER_INT_DISABLE    EIMSK&=0xDF; 
// enable reader interrupt
//#define READER_INT_ENABLE     EIMSK|=0x20;
// initialize reset pin and change port direction
//#define READER_RESET         RDRST=1;
// clear reset pin
//#define READER_CLEAR_RESET   RDRST=0;



void Write_MFRC522(uchar addr, uchar val);
uchar Read_MFRC522(uchar addr);


//=====================================================================
//=====================================================================
//=====================================================================
//                         IMPLEMENTATION
//=====================================================================
//=====================================================================
//=====================================================================

///////////////////////////////////////////////////////////////////////////////
//             M O D U L   V A R I A B L E S 
///////////////////////////////////////////////////////////////////////////////

/* ISO14443 Support Properties
* Some of the protokoll functions of ISO14443 needs information about
* the capability of the reader device, which are provided by this
* constants.
*/
//{
#define TCLFSDSNDMAX   8   ///< max. frame size send
#define TCLFSDRECMAX   8   ///< max. frame size rcv
#define TCLDSMAX       3   ///< max. baudrate divider PICC --> PCD
#define TCLDRMAX       3   ///< max. baudrate divider PCD --> PICC

#define TCLDSDFLT      0   ///< default baudrate divider PICC --> PCD
#define TCLDRDFLT      0   ///< default baudrate divider PCD --> PICC
//}

/* ISR communication structures
* All parameters are passed to the ISR via this structure.
*/
//{
// struct definition for a communication channel between function and ISR
typedef struct 
         {
            unsigned char  cmd;           //!< command code 
            char           status;        //!< communication status
            unsigned short nBytesSent;    //!< how many bytes already sent
            unsigned short nBytesToSend;  //!< how many bytes to send
            unsigned short nBytesReceived;//!< how many bytes received
            unsigned long  nBitsReceived; //!< how many bits received
            unsigned char  irqSource;     //!< which interrupts have occured
            unsigned char  collPos;       /*!< at which position occured a
                                          collision*/
            unsigned char  errFlags;      //!< error flags
            unsigned char  saveErrorState;//!< accumulated error flags for
                                          //!< multiple responses
            unsigned char  RxAlignWA;     //!< workaround for RxAlign = 7
            unsigned char  DisableDF;     //!< disable disturbance filter
         } MfCmdInfo;

// Convinience function for initialising the communication structure.
#define ResetInfo(info)    \
            info.cmd            = 0; \
            info.status         = MI_OK;\
            info.irqSource      = 0; \
            info.nBytesSent     = 0; \
            info.nBytesToSend   = 0; \
            info.nBytesReceived = 0; \
            info.nBitsReceived  = 0; \
            info.collPos        = 0; \
            info.errFlags       = 0; \
            info.saveErrorState = 0; \
            info.RxAlignWA      = 0; \
            info.DisableDF      = 0;

// In order to exchange some values between the ISR and the calling function,
// a struct is provided. 
volatile MfCmdInfo     MInfo;                  

// communication info stucture
static   volatile MfCmdInfo     *MpIsrInfo = 0; 
// ISR send buffer
static   volatile unsigned char *MpIsrOut = 0; 
// ISR receive buffer
static   volatile unsigned char *MpIsrIn = 0;   
//}


// storage of the last selected serial number including check byte.
// For multi level serial numbers, only the first 4 bytes are stored.
unsigned char MLastSelectedSnr[5];

unsigned char MFReaderStatus=0;

// storage buffer for receive and transmit routines
//{
#define MEMORY_BUFFER_SIZE    300   //Original 300 Bytes
volatile unsigned char MemPool[MEMORY_BUFFER_SIZE];

volatile unsigned char *MSndBuffer = MemPool; // pointer to the transmit buffer
volatile unsigned char *MRcvBuffer = MemPool; // pointer to the receive buffer
//}

/* Higher Baudrate Control
* attention: RegDecoderControl is modified in CascAnticoll
* Because of standard baudrate usage during anticollision, the 
* register can be written. For general purpose usage, only some bits 
* should be set.         
*
* Please pay attention, that the location of the configuration array is
* in ROM space, that means that on 16 bit microcontroller the access 
* should be word aligned.
*/
//{
typedef struct 
         {
            unsigned short  SubCarrierPulses; ///< RegRxControl1
            unsigned short  RxCoding;         ///< RegDecoderControl
            unsigned short  RxThreshold;      ///< RegRxThreshold
            unsigned short  BPSKDemControl;   ///< RegBPSKDemControl
         } t_DSCfg;

typedef struct 
         {
            unsigned short  CoderRate;        ///< RegCoderControl
            unsigned short  ModWidth;         ///< RegModWidth
         } t_DRCfg;
     
t_DSCfg  MDSCfg[4] = {{0x73,0x08,0x88,0x00}     // Manchaster 106 kBaud
                            ,{0x53,0x09,0x50,0x0C}     // BPSK 212 kBaud
                            ,{0x33,0x09,0x50,0x0C}     // BPSK 424 kBaud
                            ,{0x13,0x09,0x50,0x0C}};   // BPSK 848 kBaud
t_DRCfg  MDRCfg[4] = {{0x19,0x13}          // Miller 106 kBaud
                            ,{0x11,0x07}          // Miller 212 kBaud
                            ,{0x09,0x03}          // Miller 424 kBaud
                            ,{0x01,0x01}};        // Miller 848 kBaud

// data send baudrate divider PICC --> PCD
static unsigned char MDSI = TCLDSDFLT;    

// data send baudrate divider PCD --> PICC
static unsigned char MDRI = TCLDRDFLT;   
//}


//! Write one byte to the reader IC address space
/*!
* -o  address  (IN) reader ic register address
* -o  value    (IN) 8 bit value
* return: none
*
* This function determines the necessary page address of the 
* reader module and writes the page number to the page 
* register and the value to the specified address.
*/
void WriteRC(unsigned char Address, unsigned char value);

//! Write one byte to the reader IC address space
/*
* -o  address   (IN) reader IC register address
* return: value    8 bit data, read from the reader ic address space
*
* This function determines the necessary page address of the 
* reader module and writes the page number to the page 
* register and reads the value from the specified address.
*/
unsigned char ReadRC(unsigned char Address);


////////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////////
void OpenRC(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//          G E N E R I C    W R I T E
////////////////////////////////////////////////////////////////////////////////
unsigned char spi(unsigned char data)
{
u8 count=0; 	  
u8 Num=0;
	
	#ifndef Doorfak
	#ifdef STM32F2XX
	
  #ifdef Atlas
	Num=SPI2_ReadWriteByte(data);
	#else
	SCK(0);	
	MOSI(0);
	for(count=0;count<8;count++)  
	{ 	  
		if(data&0x80) MOSI(1);  
		else          MOSI(0);   
		data<<=1;    
		SCK(1); 	 
		Num<<=1; 	 
		if(MISO) Num++; 		 
		SCK(0);		       
	}		
  #endif 	
	
  #else	
	SCK=0;	
	MOSI=0;
	for(count=0;count<8;count++)  
	{ 	  
		if(data&0x80)  MOSI=1;  
		else           MOSI=0;   
		
		data<<=1;    
		SCK=1; 	 
		Num<<=1; 	 
		if(MISO) Num++; 		 
		SCK=0;		       
	}		 	
  #endif	
	
	//Num>>=4;
	return(Num);   
#endif
}

///////////////////////////////////////////////////////////////////////////////
//          G E N E R I C    R E A D
///////////////////////////////////////////////////////////////////////////////

#if(ReaderType!=NewRoutine)
void WriteRC(unsigned char Address, unsigned char value)
{
unsigned char Byte;

	 #ifdef STM32F2XX
   R_CS(0);
   Byte=(Address<<1)&0x7E;
   spi(Byte);
   spi(value);
   R_CS(1);
	 #else
	
   R_CS=0;
   Byte=(Address<<1)&0x7E;
   spi(Byte);
   spi(value);
   R_CS=1;
	 #endif
}

///////////////////////////////////////////////////////////////////////////////
//          G E N E R I C    R E A D
///////////////////////////////////////////////////////////////////////////////
unsigned char ReadRC(unsigned char Address)
{
unsigned char Byte;

#ifdef STM32F2XX
   R_CS(0);
   Byte=spi((Address<<1)|0x80);   
   Byte=spi(0);   
   R_CS(1);
	#else
   R_CS=0;
   Byte=spi((Address<<1)|0x80);   
   Byte=spi(0);   
   R_CS=1;
	#endif
   return Byte;
   
}  
#endif
///////////////////////////////////////////////////////////////////////////////
//             Prototypes for local functions 
///////////////////////////////////////////////////////////////////////////////

///  Internal Authentication State Switch
/*!
* -o  auth_mode (IN) 
*                  enum: selects master key A or master key B 
*                   - PICC_AUTHENT1A
*                   - PICC_AUTHENT1B 
*                  
* -o  *snr      (IN) 
*                  4 byte serial number of the card, which should be 
*                  authenticated
* -o  sector (IN) Range [0..15] 
*               specifies the key RAM address 
*               from which the keys should be taken
* return: enum:
*          - MI_OK
*          - CCE
*          - MI_BITCOUNTERR  wrong number of bits received
*          - MI_AUTHERR      wrong keys for selected card
*          - MI_KEYERR       error while loading keys
*         
* 
* Internal authentication state function.
*/
char Mf500PiccAuthState(unsigned char auth_mode,// PICC_AUTHENT1A, PICC_AUTHENT1B
                       unsigned char *snr,    // 4 byte serial number
                       unsigned char sector); // 0 <= sector <= 15  
                                            // sector address for authentication

/// Write Baudrate Divider
/*!
* -o     none
* return:   MI_OK
*
* Write function for baudrate divider and PCD properties
*/
char  Mf500PcdWriteAttrib(void);

// Command issuer for Single Response Commands 
/*
* -o  cmd  (IN)
*             Command type
*             enum:
*              - PCD_IDLE
*              - PCD_WRITEE2
*              - PCD_READE2
*              - PCD_LOADCONFIG
*              - PCD_LOADKEYE2
*              - PCD_AUTHENT1
*              - PCD_CALCCRC
*              - PCD_AUTHENT2
*              - PCD_RECEIVE
*              - PCD_LOADKEY
*              - PCD_TRANSMIT
*              - PCD_TRANSCEIVE
*             
*       send  (IN)
*             byte stream of variable length, which should be send to
*             the PICC, the length of stream has to be specified
*             in the info - structure
*       rcv   (OUT) 
*             byte stream of variable length, which was received 
*             from the PICC. The length can be obtained from the
*             info - structure
*       info  (OUT)
*             communication and status structure
* return: enum:
*          - MI_OK               operation without error
*          - MI_NOTAGERR         no tag in rf field
*          - MI_ACCESSTIMEOUT    RIC is not responding in time
*          - MI_COLLERR          collision in during rf data transfer
*          - MI_PARITYERR        parity error while receiving data
*          - MI_FRAMINGERR       framing error - start bit not valid
*          - MI_OVFLERR          FIFO overflow - to many data bytes
*          - MI_CRCERR           CRC error of received data
*          - MI_NY_IMPLEMENTED   internal error - source not identified
*         
*
* This function provides the central interface to the reader module.
* Depending on the "cmd"-value, all necessary interrupts are enabled
* and the communication is started. While the processing is done by
* the reader module, this function waits for its completion.
*
* It's notable, that the data in the send byte stream is written 
* to the FIFO of the reader module by the ISR itself. Immediate after 
* enabling the interrupts, the LoAlert interrupt is activated.
*
* The ISR writes the data to the FIFO. This function is not directly involved
* in writing or fetching data from FIFO, all work is done by the 
* corresponding ISR.After command completion, the error status is evaluated and 
* returned to the calling function.
*/ 
char PcdSingleResponseCmd(unsigned char cmd,
                volatile unsigned char* send, 
                volatile unsigned char* rcv,
                volatile MfCmdInfo *info);

/// Basic Register definitions
/*!
* return: none
*/
char PcdBasicRegisterConfiguration(void);

/// Set Reader IC Register Bit
/*!
* -o  reg  (IN)
*             register address
* -o  mask (IN)
*             Bit mask to set
* return:     none
*              
* This function performs a read - modify - write sequence
* on the specified register. All bits with a 1 in the mask
* are set - all other bits keep their original value.
*/
void SetBitMask(unsigned char reg,unsigned char mask);

/// Clear Reader IC Register Bit
/*!
* -o  reg  (IN)
*             register address
* -o  mask (IN)
*             Bit mask to clear
* return: none
*              
* This function performs a read - modify - write sequence
* on the specified register. All bits with a 1 in the mask
* are cleared - all other bits keep their original value.
*/
void ClearBitMask(unsigned char reg,unsigned char mask);

/// Flush remaining data from the FIFO
/*!
* -o  none
* return: none
*              
* This function erases  all remaining data in the MF RC 500's FIFO .
* Before writing new data or starting a new command, all remaining data 
* from former  commands should be deleted.
*/
void FlushFIFO(void);

///////////////////////////////////////////////////////////////////////
//      M I F A R E   M O D U L E   C O N F I G U R A T I O N
///////////////////////////////////////////////////////////////////////
signed char Mf500PcdConfig(void)
{
   signed char status = MI_RESETERR;
      
   #if(DEBUG==1)
   printf("\n\rPCDReset");
   #endif

   status = PcdReset(); 
   
   #if(DEBUG==1)
   printf("\n\rPCDReset Status %d", status);
   #endif

   if (status == MI_OK)
   {
     #if(DEBUG==1)
     printf("\n\rPCDReset OK");
     #endif
     if ((status = PcdBasicRegisterConfiguration()) == MI_OK);
     {
        #if(DEBUG==1)
        printf("\n\rPCDBasicRegisterConfiguration OK");
        #endif
        Mf500PcdWriteAttrib(); // write current modulation parameters
        //status=ReadRC(0x03);//RegTxControl);
        //SendStr("RegTxControl = "); SendNum(status); printf("");
        //WriteRC(RegTxControl,0x58);
        //status=ReadRC(RegTxControl);
        //SendStr("RegTxControl2 = "); SendNum(status); printf("");
        PcdRfReset(10); // Rf - reset and enable output driver    
        //*/
     }
   }
   MFReaderStatus=status;
     
   return status;
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   R E M O T E   A N T E N N A
//  Configuration of slave module
///////////////////////////////////////////////////////////////////////
signed char Mf500ActiveAntennaSlaveConfig(void)
{
   char status = MI_OK;

   FlushFIFO();    // empty FIFO
   ResetInfo(MInfo);   
   MSndBuffer[0] = 0x10; // addr low byte
   MSndBuffer[1] = 0x00; // addr high byte

   MSndBuffer[2] = 0x00; // Page
   MSndBuffer[3] = 0x7B; // RegTxControl modsource 11,InvTx2,Tx2RFEn,TX1RFEn
   MSndBuffer[4] = 0x3F; // RegCwConductance
   MSndBuffer[5] = 0x3F; // RFU13
   MSndBuffer[6] = 0x19; // RFU14
   MSndBuffer[7] = 0x13; // RegModWidth     
   MSndBuffer[8] = 0x00; // RFU16
   MSndBuffer[9] = 0x00; // RFU17
 
   MSndBuffer[10] = 0x00; // Page
   MSndBuffer[11] = 0x73; // RegRxControl1 
   MSndBuffer[12] = 0x08; // RegDecoderControl
   MSndBuffer[13] = 0x6c; // RegBitPhase     
   MSndBuffer[14] = 0xFF; // RegRxThreshold  
   MSndBuffer[15] = 0x00; // RegBPSKDemControl
   MSndBuffer[16] = 0x00; // RegRxControl2   
   MSndBuffer[17] = 0x00; // RegClockQControl

   MSndBuffer[18] = 0x00; // Page
   MSndBuffer[19] = 0x06; // RegRxWait
   MSndBuffer[20] = 0x03; // RegChannelRedundancy
   MSndBuffer[21] = 0x63; // RegCRCPresetLSB    
   MSndBuffer[22] = 0x63; // RegCRCPresetMSB    
   MSndBuffer[23] = 0x0;  // RFU25
   MSndBuffer[24] = 0x04; // RegMfOutSelect enable mfout = manchester HT
   MSndBuffer[25] = 0x00; // RFU27
     
   // PAGE 5      FIFO, Timer and IRQ-Pin Configuration
   MSndBuffer[26] = 0x00; // Page
   MSndBuffer[27] = 0x08; // RegFIFOLevel       
   MSndBuffer[28] = 0x07; // RegTimerClock      
   MSndBuffer[29] = 0x06; // RegTimerControl    
   MSndBuffer[30] = 0x0A; // RegTimerReload     
   MSndBuffer[31] = 0x02; // RegIRqPinConfig    
   MSndBuffer[32] = 0x00; // RFU    
   MSndBuffer[33] = 0x00; // RFU
   MInfo.nBytesToSend   = 34;
         
   status = PcdSingleResponseCmd(PCD_WRITEE2,
                   MSndBuffer,
                   MRcvBuffer,
                   &MInfo); // write e2
   return status;
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   R E M O T E   A N T E N N A
//  Configuration of master module
///////////////////////////////////////////////////////////////////////
signed char Mf500ActiveAntennaMasterConfig(void)
{
   char status = MI_OK;

   WriteRC(RegRxControl2,0x42);
   WriteRC(RegTxControl,0x10);
   WriteRC(RegBitPhase,0x11);
   WriteRC(RegMfOutSelect,0x04);

   return status;
}     
                  
///////////////////////////////////////////////////////////////////////
//          M I F A R E    R E Q U E S T 
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccRequest(unsigned char req_code, // request code ALL = 0x52 
                                           // or IDLE = 0x26 
                   unsigned char *atq)     // answer to request
{
  return Mf500PiccCommonRequest(req_code,atq);
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   C O M M O N   R E Q U E S T 
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccCommonRequest(unsigned char req_code, 
                            unsigned char *atq)
{
   char status = MI_OK;

   #if(DEBUG==1)
   printf("\n\rCommonRequest");
   #endif
    //************* initialize ******************************
   if ((status = Mf500PcdSetDefaultAttrib()) == MI_OK)
   {
   
      #if(DEBUG==1)
      printf("\n\rAttrib OK");
      #endif
      PcdSetTmo(60);
      
      WriteRC(RegChannelRedundancy,0x03); // RxCRC and TxCRC disable, parity enable
      ClearBitMask(RegControl,0x08);      // disable crypto 1 unit   
      WriteRC(RegBitFraming,0x07);        // set TxLastBits to 7 
      
      ResetInfo(MInfo);   
      MSndBuffer[0] = req_code;
      MInfo.nBytesToSend = 1;   
      MInfo.DisableDF = 1;
      status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                         MSndBuffer,
                         MRcvBuffer,
                         &MInfo);
      if ((status == MI_OK) && (MInfo.nBitsReceived != 16)) // 2 bytes expected
      {
         status = MI_BITCOUNTERR;
         #if(DEBUG==1)
         printf("\n\rMI_BITCOUNTERR");
         #endif
      } 
      if ((status == MI_COLLERR) && (MInfo.nBitsReceived == 16)) //
         status = MI_OK; // all received tag-types are combined to the 16 bit
         
      // in any case, copy received data to output - for debugging reasons
      if (MInfo.nBytesReceived >= 2)
      {
         atq[0]=MRcvBuffer[0];      
         atq[1]=MRcvBuffer[1];      
      }
      else
      {
         if (MInfo.nBytesReceived == 1)
            atq[0] = MRcvBuffer[0];
         else
            atq[0] = 0x00;
         atq[1] = 0x00;
      }
   }
   return status; 
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E    A N T I C O L L I S I O N
// for standard select
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccAnticoll (unsigned char bcnt,
                     unsigned char *snr)
{
   return Mf500PiccCascAnticoll(0x93,bcnt,snr); // first cascade level
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E    A N T I C O L L I S I O N
// for extended serial numbers
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccCascAnticoll (unsigned char select_code,
                         unsigned char bcnt,       
                         unsigned char *snr)       
{
   int   k=0;
   char  status = MI_OK;
   char  snr_in[4];         // copy of the input parameter snr
   char  nbytes = 0;        // how many bytes received
   char  nbits = 0;         // how many bits received
   char  complete = 0;      // complete snr recived
   short i        = 0;
   char  byteOffset = 0;
   unsigned char snr_crc;   // check byte calculation
   unsigned char snr_check;
   unsigned char dummyShift1;       // dummy byte for snr shift
   unsigned char dummyShift2;       // dummy byte for snr shift   
 
   //************* Initialisierung ******************************
   if ((status = Mf500PcdSetDefaultAttrib()) == MI_OK)
   {
      PcdSetTmo(106);
      
      snr_in[0]=snr[0];   
      snr_in[1]=snr[1];   
      snr_in[2]=snr[2];   
      snr_in[3]=snr[3];   
      
      WriteRC(RegDecoderControl,0x28); // ZeroAfterColl aktivieren   
      ClearBitMask(RegControl,0x08);    // disable crypto 1 unit
         
      //************** Anticollision Loop ***************************
      complete=0;
		  k=0;
      while (!complete && (status == MI_OK) )
      {
         if(++k>1000)
           break;
         // if there is a communication problem on the RF interface, bcnt 
         // could be larger than 32 - folowing loops will be defective.
         if (bcnt > 32)
         {
            status = MI_WRONG_PARAMETER_VALUE;
            continue;
         }
         ResetInfo(MInfo);
         MInfo.cmd = select_code;   // pass command flag to ISR        
         MInfo.DisableDF = 1;
         WriteRC(RegChannelRedundancy,0x03); // RxCRC and TxCRC disable, parity enable
         nbits = bcnt % 8;   // remaining number of bits
         if (nbits)
         {
            WriteRC(RegBitFraming,nbits << 4 | nbits); // TxLastBits/RxAlign auf nb_bi
            nbytes = bcnt / 8 + 1;   
            // number of bytes known
   
            // in order to solve an inconsistancy in the anticollision sequence
            // (will be solved soon), the case of 7 bits has to be treated in a
            // separate way
            if (nbits == 7 )
            {
            	MInfo.RxAlignWA = 1;
               MInfo.nBitsReceived = 7; // set flag for 7 bit anticoll, which is evaluated
                                        // in the ISRnBitsReceived        
               WriteRC(RegBitFraming,nbits); // reset RxAlign to zero
            }
         } 
         else
         {
            nbytes = bcnt / 8;
         }
  
         MSndBuffer[0] = select_code;
         MSndBuffer[1] = 0x20 + ((bcnt/8) << 4) + nbits; //number of bytes send
                  
         for (i = 0; i < nbytes; i++)  // Sende Buffer beschreiben
         {
            MSndBuffer[i + 2] = snr_in[i];
         }
         MInfo.nBytesToSend   = 2 + nbytes;    

         status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                            MSndBuffer,
                            MRcvBuffer,
                            &MInfo);

          // in order to solve an inconsistancy in the anticollision sequence
          // (will be solved soon), the case of 7 bits has to be treated in a
          // separate way 
         if (MInfo.RxAlignWA)
         {
            // reorder received bits
            dummyShift1 = 0x00;
            for (i = 0; i < MInfo.nBytesReceived; i++)
            {
                dummyShift2 = MRcvBuffer[i];
                MRcvBuffer[i] = (dummyShift1 >> (i+1)) | (MRcvBuffer[i] << (7-i));
                dummyShift1 = dummyShift2;
            }
            MInfo.nBitsReceived -= MInfo.nBytesReceived; // subtract received parity bits
            // recalculation of collision position
            if ( MInfo.collPos ) MInfo.collPos += 7 - (MInfo.collPos + 6) / 9;
         }
         
         if ( status == MI_OK || status == MI_COLLERR)    // no other occured
         {

            byteOffset = 0;
            if ( nbits != 0 )           // last byte was not complete
            {
               snr_in[nbytes - 1] = snr_in[nbytes - 1] | MRcvBuffer[0];
               byteOffset = 1;
            }
            for ( i =0; i < (4 - nbytes); i++)     
            {
               snr_in[nbytes + i] = MRcvBuffer[i + byteOffset];
            }
            // R e s p o n s e   P r o c e s s i n g   
            if ( MInfo.nBitsReceived != (40 - bcnt) ) // not 5 bytes answered
            {
               status = MI_BITCOUNTERR;
            } 
            else 
            {
               if (status != MI_COLLERR ) // no error and no collision
               {
                  // SerCh check
                  snr_crc = snr_in[0] ^ snr_in[1] ^ snr_in[2] ^ snr_in[3];
                  snr_check = MRcvBuffer[MInfo.nBytesReceived - 1];
                  if (snr_crc != snr_check)
                  {
                     status = MI_SERNRERR;
                  } 
                  else   
                  {
                     complete = 1;
                  }
               }
               else                   // collision occured
               {
                  bcnt = bcnt + MInfo.collPos - nbits;
                  status = MI_OK;
               }
            }
        }
      }
   }
   // transfer snr_in to snr - even in case of an error - for 
   // debugging reasons
   snr[0]=snr_in[0];
   snr[1]=snr_in[1];
   snr[2]=snr_in[2];
   snr[3]=snr_in[3];

   //----------------------Einstellungen aus Initialisierung ruecksetzen 
   ClearBitMask(RegDecoderControl,0x20); // ZeroAfterColl disable
   
   return status;  
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E    S E L E C T 
// for std. select
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccSelect(unsigned char *snr, 
                  unsigned char *sak)
{
   return Mf500PiccCascSelect(0x93,snr,sak); // first cascade level
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E    C A S C A D E D   S E L E C T 
//  for extended serial number
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccCascSelect(unsigned char select_code, 
                        unsigned char *snr,
                        unsigned char *sak)
{
   char   status = MI_OK; 

   if ((status = Mf500PcdSetDefaultAttrib()) == MI_OK)
   {
      PcdSetTmo(106);
    
      WriteRC(RegChannelRedundancy,0x0F); // RxCRC,TxCRC, Parity enable
      ClearBitMask(RegControl,0x08);    // disable crypto 1 unit
   
      //************* Cmd Sequence ********************************** 
      ResetInfo(MInfo);   
      MSndBuffer[0] = select_code;
      MSndBuffer[1] = 0x70;         // number of bytes send
      
      MSndBuffer[2]=snr[0];
      MSndBuffer[3]=snr[1];
      MSndBuffer[4]=snr[2];
      MSndBuffer[5]=snr[3];

      MSndBuffer[6] = MSndBuffer[2] 
                      ^ MSndBuffer[3] 
                      ^ MSndBuffer[4] 
                      ^ MSndBuffer[5];
      MInfo.nBytesToSend   = 7;
      MInfo.DisableDF = 1;
      status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                          MSndBuffer,
                          MRcvBuffer,
                          &MInfo);
   
      *sak = 0;   
      if (status == MI_OK)    // no timeout occured
      {
         if (MInfo.nBitsReceived != 8)    // last byte is not complete
         {
            status = MI_BITCOUNTERR;
         }
         else
         {
            MLastSelectedSnr[0]=snr[0];            
            MLastSelectedSnr[1]=snr[1];            
            MLastSelectedSnr[2]=snr[2];            
            MLastSelectedSnr[3]=snr[3];            
         }
      }
      // copy received data in any case - for debugging reasons
      *sak = MRcvBuffer[0];
   }
   return status;
}

///////////////////////////////////////////////////////////////////////
//       M I F A R E   P I C C   A C T I V A T I O N    S E Q E N C E
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccActivateIdle(unsigned char br,
                           unsigned char *atq, 
                           unsigned char *sak, 
                           unsigned char *uid, 
                           unsigned char *uid_len)
{
  unsigned char cascade_level;
  unsigned char sel_code;
  unsigned char uid_index;
  signed char status;
  unsigned char cmdASEL;
  int k=0;

  *uid_len      = 0;

  //call activation with def. divs
  status = Mf500PcdSetDefaultAttrib();
  if (status == MI_OK)
  {
     status = Mf500PiccCommonRequest(PICC_REQIDL,atq);
  }
  if (status == MI_OK)
  {
     if((atq[0] & 0x1F) == 0x00) // check lower 5 bits, for tag-type
                                 // all tags within this 5 bits have to
                                 // provide a bitwise anticollision
     {
        status = MI_NOBITWISEANTICOLL;
     }
  }
  if (status == MI_OK)
  {
      //Get UID in 1 - 3 levels (standard, [double], [triple] )
      //-------
      switch(br)
      {
         case 0: cmdASEL = PICC_ANTICOLL1; break;
         case 1: cmdASEL = PICC_ANTICOLL11; break;
         case 2: cmdASEL = PICC_ANTICOLL12; break;
         case 3: cmdASEL = PICC_ANTICOLL13; break;
         default:
              status = MI_BAUDRATE_NOT_SUPPORTED; break;
      }
  }
  if (status == MI_OK)
  {
      cascade_level = 0;
      uid_index     = 0;
		  k=0;
      do
      {
        if(++k>1000)
          break;
        //Select code depends on cascade level
        sel_code   = cmdASEL + (2 * cascade_level);
        cmdASEL = PICC_ANTICOLL1; // reset anticollistion level for calculation
        //ANTICOLLISION
        status = Mf500PiccCascAnticoll(sel_code, 0, &uid[uid_index]);
        //SELECT
        if (status == MI_OK)
        {
           status = Mf500PiccCascSelect(sel_code, &uid[uid_index], sak);
           if (status == MI_OK)
           {
              cascade_level++;

              //we differ cascaded and uncascaded UIDs
              if (*sak & 0x04) // if cascaded, bit 2 is set in answer to select
              {
                 //this UID is cascaded, remove the cascaded tag that is
                 //0x88 as first of the 4 byte received
                 uid[uid_index+0]=uid[uid_index+1];
                 uid[uid_index+1]=uid[uid_index+2];
                 uid[uid_index+2]=uid[uid_index+3];
                 uid_index += 3;
                 *uid_len += 3;
              }
              else
              {
                 //this UID is not cascaded -> the length is 4 bytes
                 uid_index += 4;
                 *uid_len += 4;
              }
           }
        }
      }
      while((status == MI_OK)        // error status
            && (*sak & 0x04)         // no further cascade level
            && (cascade_level < 3)); // highest cascade level is reached
   }
   if (status == MI_OK)
   {
      //Exit function, if cascade level is triple and sak indicates another
      //cascase level.
      if ((cascade_level == 3) && (*sak & 0x04))
      {
         *uid_len = 0;
         status = MI_CASCLEVEX;
      }
      Mf500PcdSetAttrib(br,br);
   }
   return (status);
}

///////////////////////////////////////////////////////////////////////
//       M I F A R E   P I C C   A C T I V A T I O N    S E Q E N C E
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccActivateWakeup(unsigned char br,
                             unsigned char *atq, 
                             unsigned char *sak,
                             unsigned char *uid, 
                             unsigned char uid_len)
{
   int k=0;
   unsigned char cascade_level;
   unsigned char uid_index;
   unsigned char tmpuid[4];
   unsigned char sel_code;
   unsigned char cmdASEL;
   signed char   status;

   //call activation with def. divs
   status = Mf500PcdSetDefaultAttrib();
   if (status == MI_OK)
   {
      status = Mf500PiccCommonRequest(PICC_REQALL,atq);
   }
   if (status == MI_OK)
   {
      if ((atq[0] & 0x1F) == 0x00) // check lower 5 bits, for tag-type
                                   // all tags within this 5 bits have to
                                   // provide a bitwise anticollision
      {
         status = MI_NOBITWISEANTICOLL;
      }
   }
   if (status == MI_OK)
   {
      //Get UID in 1 - 3 levels (standard, [double], [triple] )
      //-------
      switch(br)
      {
         case 0: cmdASEL = PICC_ANTICOLL1; break;
         case 1: cmdASEL = PICC_ANTICOLL11; break;
         case 2: cmdASEL = PICC_ANTICOLL12; break;
         case 3: cmdASEL = PICC_ANTICOLL13; break;
         default:
              status = MI_BAUDRATE_NOT_SUPPORTED; break;
      }
   }
   if (status == MI_OK)
   {
      //Select UID in up to 3 cascade levels (standard, [double], [triple] )
      //------------------------------------
     
      cascade_level = 0;
      uid_index     = 0;
      tmpuid[0] = 0x88;     //first byte of cascaded UIDs is 0x88 (cascaded tag)
      k=0;
      do
      {
        if(++k>1000)
          break;
        sel_code   = cmdASEL + (2 * cascade_level);
        cmdASEL = PICC_ANTICOLL1; // reset anticollistion level for calculation
        //get the next UID part if we need to cascade
        if((uid_len - uid_index) > 4)
        {
          //ok, we need to cascade the UID
          tmpuid[1]=uid[uid_index+0];
          tmpuid[2]=uid[uid_index+1];
          tmpuid[3]=uid[uid_index+2];
          uid_index += 3;
        }
        else
        {
          //ah, how nice. no need to cascade
          tmpuid[0]=uid[uid_index+0];
          tmpuid[1]=uid[uid_index+1];
          tmpuid[2]=uid[uid_index+2];
          tmpuid[3]=uid[uid_index+3];
          uid_index += 4;
        }

        status = Mf500PiccCascSelect(sel_code, tmpuid, sak);

        if(status == MI_OK)
        {
          cascade_level++;
        }
      }
      while((status == MI_OK )    // error occured
            && (*sak & 0x04)       // no further cascade level
            && ((uid_index + 1) < uid_len) // all bytes of snr sent
            && (cascade_level < 3)); // highest cascade level reached
   }
   if ( status == MI_OK) 
   {
      //Exit function, if UID length is not of expected length
      if ((uid_index) != uid_len)
      {
         status =  MI_SERNRERR ;
      }
   }
   if (status == MI_OK)
   {
      //Exit function, if cascade level is triple and sak indicates another
      //cascase level.
      if ((cascade_level == 3) && (*sak & 0x04))
      {
         status = MI_SERNRERR;
      }
   }
   return status;
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E      A U T H E N T I C A T I O N
//   calling compatible version    
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccAuth(unsigned char key_type,    // PICC_AUTHENT1A or PICC_AUTHENT1B
                   unsigned char key_addr,    // key address in reader storage
                   unsigned char block)       // block number which should be 
                                              // authenticated
{
   char            status = MI_OK;

   status = Mf500PiccAuthE2(  key_type,
                              MLastSelectedSnr,
                              key_addr,
                              block);
   return status;
}

///////////////////////////////////////////////////////////////////////
//                  A U T H E N T I C A T I O N   
//             W I T H   K E Y S   F R O M   E 2 P R O M
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccAuthE2(   unsigned char auth_mode,   // PICC_AUTHENT1A or PICC_AUTHENT1B
                     unsigned char *snr,        // 4 bytes card serial number
                     unsigned char key_sector,  // 0 <= key_sector <= 15                     
                     unsigned char block)      //  0 <= block <= 256
{
   char status = MI_OK;
   // eeprom address calculation
   // 0x80 ... offset
   // key_sector ... sector
   // 0x18 ... 2 * 12 = 24 = 0x18
   //unsigned short e2addr = 0x80 + key_sector * 0x18;
   unsigned short e2addr = 0x80 + key_sector * 12;
   unsigned char *e2addrbuf = (unsigned char*)&e2addr;
   
   PcdSetTmo(106);

   //if (auth_mode == PICC_AUTHENT1B)
   //   e2addr += 12; // key B offset   
   FlushFIFO();    // empty FIFO
   ResetInfo(MInfo);

   MSndBuffer[0]=e2addrbuf[0]; // write low and high byte of address
   MSndBuffer[1]=e2addrbuf[1];
   MInfo.nBytesToSend   = 2;
    // write load command
   if ((status=PcdSingleResponseCmd(PCD_LOADKEYE2,MSndBuffer,MRcvBuffer,&MInfo)) == MI_OK)
   {      
      // execute authentication
      status = Mf500PiccAuthState(auth_mode,snr,block);  
   }
   return status;
}                        

///////////////////////////////////////////////////////////////////////
//                      C O D E   K E Y S  
///////////////////////////////////////////////////////////////////////
signed char Mf500HostCodeKey(  unsigned char *uncoded, // 6 bytes key value uncoded
                     unsigned char *coded)   // 12 bytes key value coded
{
   char status = MI_OK;
   unsigned char cnt = 0;
   unsigned char ln  = 0;     // low nibble
   unsigned char hn  = 0;     // high nibble
   
   for (cnt = 0; cnt < 6; cnt++)
   {
      ln = uncoded[cnt] & 0x0F;
      hn = uncoded[cnt] >> 4;
      coded[cnt * 2 + 1]     =  (~ln << 4) | ln;
      coded[cnt * 2 ] =  (~hn << 4) | hn;
   }
   return MI_OK;
}

///////////////////////////////////////////////////////////////////////
//                  A U T H E N T I C A T I O N   
//             W I T H   P R O V I D E D   K E Y S
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccAuthKey(  unsigned char auth_mode,
                     unsigned char *snr,       
                     unsigned char *keys,      
                     unsigned char block)      
{
   char status     = MI_OK;
   unsigned char i = 0;
   
   PcdSetTmo(106);
   FlushFIFO();    // empty FIFO
   ResetInfo(MInfo);
   for(i=0; i<12; i++)         // write 12 bytes of the key
     MSndBuffer[i]=keys[i];
   MInfo.nBytesToSend = 12;
    // write load command
   if ((status=PcdSingleResponseCmd(PCD_LOADKEY,MSndBuffer,MRcvBuffer,&MInfo)) == MI_OK)
   {      
      // execute authentication
      status = Mf500PiccAuthState(auth_mode,snr,block); 
   }
   return status;
}

///////////////////////////////////////////////////////////////////////
//        S T O R E   K E Y S   I N   E E P R O M
///////////////////////////////////////////////////////////////////////
signed char Mf500PcdLoadKeyE2(unsigned char key_type,
                       unsigned char sector,
                       unsigned char *uncoded_keys)
{
   // eeprom address calculation
   // 0x80 ... offset
   // key_sector ... sector
   // 0x18 ... 2 * 12 = 24 = 0x18
   signed char status = MI_OK;
   //unsigned int e2addr = 0x80 + (sector * 0x18);
   unsigned int e2addr = 0x80 + (sector * 12);
   unsigned char coded_keys[12];

   //if (key_type == PICC_AUTHENT1B)
   //   e2addr += 12; // key B offset   
   if ((status = Mf500HostCodeKey(uncoded_keys,coded_keys)) == MI_OK)
      status = PcdWriteE2(  e2addr,12,coded_keys);
   return status;
}                       
                          
///////////////////////////////////////////////////////////////////////
//        A U T H E N T I C A T I O N   S T A T E S
///////////////////////////////////////////////////////////////////////
char Mf500PiccAuthState(unsigned char auth_mode,
                        unsigned char *snr,
                        unsigned char block)
{
   char status = MI_OK;
   unsigned char i = 0;
   
   WriteRC(RegChannelRedundancy,0x07); // RxCRC disable,TxCRC, Parity enable

   PcdSetTmo(150);
   MSndBuffer[0] = auth_mode;        // write authentication command

   MSndBuffer[1] = block;    // write block number for authentication
   MSndBuffer[2]=snr[0]; // write 4 bytes card serial number 
   MSndBuffer[3]=snr[1];
   MSndBuffer[4]=snr[2];
   MSndBuffer[5]=snr[3];
   ResetInfo(MInfo);
   MInfo.nBytesToSend = 6;
   if ((status = PcdSingleResponseCmd(PCD_AUTHENT1,
                            MSndBuffer,
                            MRcvBuffer,
                            &MInfo)) == MI_OK)
   {
      if (ReadRC(RegSecondaryStatus) & 0x07) // RxLastBits mu? leer sein
      {
         status = MI_BITCOUNTERR;
      }
      else
      {
         WriteRC(RegChannelRedundancy,0x03); // RxCRC,TxCRC disable, Parity enable
         ResetInfo(MInfo);
         MInfo.nBytesToSend = 0;
         if ((status = PcdSingleResponseCmd(PCD_AUTHENT2,
                                  MSndBuffer,
                                  MRcvBuffer,
                                  &MInfo)) == MI_OK) 
         {
            if ( ReadRC(RegControl) & 0x08 ) // Crypto1 activated
            {
                status = MI_OK;
            }
            else
            {
                status = MI_AUTHERR;
            }
         }
      }
   }
   return status;
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   R E A D   
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccRead(  unsigned char addr,
                  unsigned char* data)
{
   return Mf500PiccCommonRead(PICC_READ16,addr,16,data);
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   R E A D   
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccCommonRead(  unsigned char cmd,
                           unsigned char addr,
                           unsigned char datalen,
                           unsigned char *data)
{
   char status = MI_OK;
   char bitsExpected;
   char tmp    = 0;
   char i;  
   int j;


   FlushFIFO();    // empty FIFO

   PcdSetTmo(640);  // long timeout 

   WriteRC(RegChannelRedundancy,0x0F); // RxCRC, TxCRC, Parity enable
   
   // ************* Cmd Sequence ********************************** 
   ResetInfo(MInfo);   
   MSndBuffer[0] = cmd;   // read command code
   MSndBuffer[1] = addr;
   MInfo.nBytesToSend   = 2;   
   status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                       MSndBuffer,
                       MRcvBuffer,
                       &MInfo);

   if (status != MI_OK)
   {
      if (status != MI_NOTAGERR ) // no timeout occured
      {
         if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
         {
            // upper nibble should be equal to lower nibble, otherwise 
            // there is a coding error on card side.
            if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
               bitsExpected = 8;
            else
            {
               status = MI_CODINGERR;
               bitsExpected = 0; // data is copied to output
            }
         }
         else
            bitsExpected = 4;  
         if (MInfo.nBitsReceived == bitsExpected)  // NACK
         {
             MRcvBuffer[0] &= 0x0f;  // mask out upper nibble
             if ((MRcvBuffer[0] & 0x0a) == 0)
             {
                status = MI_NOTAUTHERR;
             }
             else
             {
                status = MI_CODEERR;
             }
          }
          else
          {
             // return data - even if an error occured - for debugging reasons
             if (MInfo.nBytesReceived >= datalen) 
               for(j=0; j<datalen; j++) data[j]=MRcvBuffer[j];
             else
             {
                for(j=0; j<MInfo.nBytesReceived; j++) data[j]=MRcvBuffer[j];
                for (i = MInfo.nBytesReceived; i < datalen; i++)
                {
                   data[i] = 0x00;
                }
             }
          }
             
      }
      else
         for(j=0; j<datalen; j++) data[j]='0'; // in case of an error initialise 
                                             // data
   }
   else   // Response Processing
   {
      if (MInfo.nBytesReceived != datalen)
      {
         status = MI_BYTECOUNTERR;
         // return data, even if an error occured
         if (MInfo.nBytesReceived >= datalen)
            for(j=0; j<datalen; j++) data[j]=MRcvBuffer[j];
         else
         {
            for(j=0; j<MInfo.nBytesReceived; j++) data[j]=MRcvBuffer[j];
            for (i = MInfo.nBytesReceived; i < datalen; i++)
            {
               data[i] = 0x00;
            }
         }   
      }
      else
      {
         for(j=0; j<datalen; j++) data[j]=MRcvBuffer[j];
      }
   }
   return status; 
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   W R I T E     
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccWrite( unsigned char addr,
                  unsigned char *data)
{
   return Mf500PiccCommonWrite(PICC_WRITE16,addr,16,data);
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   W R I T E     
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccWrite4( unsigned char addr,
                      unsigned char *data)
{
   unsigned char bitsExpected;
   char status = MI_OK;

     // ************* Cmd Sequence ********************************** 
   PcdSetTmo(1000);     // long timeout 

   WriteRC(RegChannelRedundancy,0x07); //  TxCRC, Parity enable
     
   ResetInfo(MInfo);   
   MSndBuffer[0] = PICC_WRITE4;  // Write command code
   MSndBuffer[1] = addr;
   MSndBuffer[2]=data[0];
   MSndBuffer[3]=data[1];
   MSndBuffer[4]=data[2];
   MSndBuffer[5]=data[3];
   MInfo.nBytesToSend   = 6;
   status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                         MSndBuffer,
                         MRcvBuffer,
                         &MInfo);

   if (status != MI_NOTAGERR)   // no timeout error
   {
      if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
      {
         // upper nibble should be equal to lower nibble, otherwise 
         // there is a coding error on card side.
         if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
            bitsExpected = 8;
         else
         {
            bitsExpected = 0;
         }
      }
      else
         bitsExpected = 4;  
      if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
      {
         if (bitsExpected == 0)
            status = MI_CODINGERR;
         else
            status = MI_BITCOUNTERR;
      }
      else                     // 4 bit received
      {
         MRcvBuffer[0] &= 0x0f; // mask out upper nibble
         switch(MRcvBuffer[0])
         {
            case 0x00: 
               status = MI_NOTAUTHERR;
               break;
            case 0x0a:
               status = MI_OK;
               break;
            default:
               status = MI_CODEERR;
               break;
         }
      }
   }
   return status;
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E   W R I T E     
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccCommonWrite( unsigned char cmd,
                           unsigned char addr,
                           unsigned char datalen,
                           unsigned char *data)
{
   unsigned char bitsExpected;
   char status = MI_OK;  
   int j;

     // ************* Cmd Sequence ********************************** 
   PcdSetTmo(1000);     // long timeout 

   WriteRC(RegChannelRedundancy,0x07); //  TxCRC, Parity enable
     
   ResetInfo(MInfo);   
   MSndBuffer[0] = cmd;        // Write command code
   MSndBuffer[1] = addr;
   MInfo.nBytesToSend   = 2;
   status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                         MSndBuffer,
                         MRcvBuffer,
                         &MInfo);
   if (status != MI_NOTAGERR)   // no timeout error
   {
      if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
      {
         // upper nibble should be equal to lower nibble, otherwise 
         // there is a coding error on card side.
         if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
            bitsExpected = 8;
         else
         {
            bitsExpected = 0;
         }
      }
      else
         bitsExpected = 4;  
      bitsExpected = 8;  //////////////////////////
      if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
      {
         if (bitsExpected == 0)
            status = MI_CODINGERR;
         else
            status = MI_BITCOUNTERR;
      }
      else                     // 4 bit received
      {
         MRcvBuffer[0] &= 0x0f; // mask out upper nibble
         switch(MRcvBuffer[0])
         {
            case 0x00: 
               status = MI_NOTAUTHERR;
               break;
            case 0x0a:
               status = MI_OK;
               break;
            default:
               status = MI_CODEERR;
               break;
         }
      }
   }

   if ( status == MI_OK)
   {
      ResetInfo(MInfo);   
      for(j=0; j<datalen; j++) MSndBuffer[j]=data[j]; 
      MInfo.nBytesToSend   = datalen;
      status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                            MSndBuffer,
                            MRcvBuffer,
                            &MInfo);
        
      if (status != MI_NOTAGERR)    // no timeout occured
      {
         if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
         {
            // upper nibble should be equal to lower nibble, otherwise 
            // there is a coding error on card side.
            if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
               bitsExpected = 8;
            else
            {
               bitsExpected = 0;
            }
         }
         else
            bitsExpected = 4;  
         bitsExpected = 8;  //////////////////////////
         if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
         {
            if (bitsExpected == 0)
               status = MI_CODINGERR;
            else
               status = MI_BITCOUNTERR;
         }
         else                     // 4 bit received
         {
            MRcvBuffer[0] &= 0x0f; // mask out upper nibble
            switch(MRcvBuffer[0])
            {
               case 0x00: 
                  status = MI_WRITEERR;
                  break;
               case 0x0a:
                  status = MI_OK;
                  break;
               default:
                  status = MI_CODEERR;
                  break;
            }
         }
      }        
   }
   return (status*(-1));
}


///////////////////////////////////////////////////////////////////////
//                V A L U E   M A N I P U L A T I O N 
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccValue(unsigned char dd_mode, 
                   unsigned char addr, 
                   unsigned char *value,
                   unsigned char trans_addr)
{
unsigned char bitsExpected;
char status = MI_OK;

   PcdSetTmo(106);
   
   WriteRC(RegChannelRedundancy,0x07); // TxCRC, Parity enable
   
   // ************* Cmd Sequence ********************************** 
   ResetInfo(MInfo);   
   MSndBuffer[0] = dd_mode;        // Inc,Dec command code
   MSndBuffer[1] = addr;
   MInfo.nBytesToSend   = 2;
   status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                       MSndBuffer,
                       MRcvBuffer,
                       &MInfo);

   if (status != MI_NOTAGERR)   // no timeout error
   {
        if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
        {
           // upper nibble should be equal to lower nibble, otherwise 
           // there is a coding error on card side.
           if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
              bitsExpected = 8;
           else
           {
              bitsExpected = 0;
           }
        }
        else
           bitsExpected = 4;  
           
        bitsExpected=8; /////////////////   
        if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
        {
           if (bitsExpected == 0)
              status = MI_CODINGERR;
           else
              status = MI_BITCOUNTERR;
        }
        else                     // 4 bit received
        {
           MRcvBuffer[0] &= 0x0f; // mask out upper nibble
           switch(MRcvBuffer[0])
           {
              case 0x00: 
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0a:
                 status = MI_OK;
                 break;
              case 0x01:
                 status = MI_VALERR;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
     }

     if ( status == MI_OK)
     {
        PcdSetTmo(1000);     // long timeout 

        ResetInfo(MInfo);   
        MSndBuffer[0]=value[0];
        MSndBuffer[1]=value[1];
        MSndBuffer[2]=value[2];
        MSndBuffer[3]=value[3];
        MInfo.nBytesToSend   = 4;
        status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                            MSndBuffer,
                            MRcvBuffer,
                            &MInfo);
			 
			 
        if (status == MI_OK)    // no timeout occured
        {
	    		  /*
            if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
            {
               // upper nibble should be equal to lower nibble, otherwise 
               // there is a coding error on card side.
               if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
                  bitsExpected = 8;
               else
               {
                  bitsExpected = 0;
               }
            }
            else
               bitsExpected = 4;  
            bitsExpected=0; /////////////////   
            if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
            {
               if (bitsExpected == 0)
                  status = MI_CODINGERR;
               else
                  status = MI_BITCOUNTERR;
            }
            else                     // 4 bit received
            {
               MRcvBuffer[0] &= 0x0f; // mask out upper nibble
               switch(MRcvBuffer[0])
               {
                  case 0x00: 
                     status = MI_NOTAUTHERR;
                     break;
                  case 0x01:
                     status = MI_VALERR;
                     break;
                  default:
                     status = MI_CODEERR;
                     break;
               }
            }
						*/
         }        
         else
         if (status == MI_NOTAGERR )
            status = MI_OK;  // no response after 4 byte value - 
                             // transfer command has to follow
     }
		
     if ( status == MI_OK)
     {
        ResetInfo(MInfo);   
        MSndBuffer[0] = PICC_TRANSFER;        // transfer command code
        MSndBuffer[1] = trans_addr;
        MInfo.nBytesToSend   = 2;
        status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                            MSndBuffer,
                            MRcvBuffer,
                            &MInfo);
        
        if (status != MI_NOTAGERR)    // timeout occured
        {
            if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
            {
               // upper nibble should be equal to lower nibble, otherwise 
               // there is a coding error on card side.
               if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
                  bitsExpected = 8;
               else
               {
                  bitsExpected = 0;
               }
            }
            else
               bitsExpected = 4;  
            bitsExpected=8; /////////////////   
            if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
            {
               if (bitsExpected == 0)
                  status = MI_CODINGERR;
               else
                  status = MI_BITCOUNTERR;
            }
            else                     // 4 bit received
            {
               MRcvBuffer[0] &= 0x0f; // mask out upper nibble
               switch(MRcvBuffer[0])
               {
                  case 0x00: 
                     status = MI_NOTAUTHERR;
                     break;
                  case 0x0a:
                     status = MI_OK;
                     break;
                  case 0x01:
                     status = MI_VALERR;
                     break;
                  default:
                     status = MI_CODEERR;
                     break;
               }
            }
        }        
     }
   return status;
}

///////////////////////////////////////////////////////////////////////
//                V A L U E   M A N I P U L A T I O N 
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccCopyBlock(unsigned char addr,
                               unsigned char trans_addr)
{
   unsigned char bitsExpected;
   char status = MI_OK;

   PcdSetTmo(106);
   
   WriteRC(RegChannelRedundancy,0x07); // TxCRC, Parity enable
   
   // ************* Cmd Sequence ********************************** 
   ResetInfo(MInfo);   
   MSndBuffer[0] = PICC_RESTORE;        // Inc,Dec command code
   MSndBuffer[1] = addr;
   MInfo.nBytesToSend   = 2;
   status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                       MSndBuffer,
                       MRcvBuffer,
                       &MInfo);

   if (status != MI_NOTAGERR)   // no timeout error
   {
        if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
        {
           // upper nibble should be equal to lower nibble, otherwise 
           // there is a coding error on card side.
           if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
              bitsExpected = 8;
           else
           {
              bitsExpected = 0;
           }
        }
        else
           bitsExpected = 4;  
           
        bitsExpected=8; /////////////////   
        if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
        {
           if (bitsExpected == 0)
              status = MI_CODINGERR;
           else
              status = MI_BITCOUNTERR;
        }
        else                     // 4 bit received
        {
           MRcvBuffer[0] &= 0x0f; // mask out upper nibble
           switch(MRcvBuffer[0])
           {
              case 0x00: 
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0a:
                 status = MI_OK;
                 break;
              case 0x01:
                 status = MI_VALERR;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
     }

     if ( status == MI_OK)
     {
        ResetInfo(MInfo);   
        MSndBuffer[0] = PICC_TRANSFER;        // transfer command code
        MSndBuffer[1] = trans_addr;
        MInfo.nBytesToSend   = 2;
        status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                            MSndBuffer,
                            MRcvBuffer,
                            &MInfo);
        
        if (status != MI_NOTAGERR)    // timeout occured
        {
            if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
            {
               // upper nibble should be equal to lower nibble, otherwise 
               // there is a coding error on card side.
               if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
                  bitsExpected = 8;
               else
               {
                  bitsExpected = 0;
               }
            }
            else
               bitsExpected = 4;  
            bitsExpected=8; /////////////////   
            if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
            {
               if (bitsExpected == 0)
                  status = MI_CODINGERR;
               else
                  status = MI_BITCOUNTERR;
            }
            else                     // 4 bit received
            {
               MRcvBuffer[0] &= 0x0f; // mask out upper nibble
               switch(MRcvBuffer[0])
               {
                  case 0x00: 
                     status = MI_NOTAUTHERR;
                     break;
                  case 0x0a:
                     status = MI_OK;
                     break;
                  case 0x01:
                     status = MI_VALERR;
                     break;
                  default:
                     status = MI_CODEERR;
                     break;
               }
            }
        }        
     }
   return status;
}

///////////////////////////////////////////////////////////////////////
//   V A L U E   M A N I P U L A T I O N   W I T H   B A C K U P
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccValueDebit(unsigned char dd_mode, 
                         unsigned char addr, 
                         unsigned char *value)
{
   unsigned char bitsExpected;
   char status = MI_OK;
   
   PcdSetTmo(106);
   
   WriteRC(RegChannelRedundancy,0x07); // RxCRC,TxCRC, Parity enable   
   
   ResetInfo(MInfo);   
   MSndBuffer[0] = dd_mode;        // Inc,Dec command code
   MSndBuffer[1] = addr;
   MInfo.nBytesToSend   = 2;
   status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                       MSndBuffer,
                       MRcvBuffer,
                       &MInfo);

   if (status != MI_NOTAGERR)   // no timeout error
   {
        if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
        {
           // upper nibble should be equal to lower nibble, otherwise 
           // there is a coding error on card side.
           if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
              bitsExpected = 8;
           else
           {
              bitsExpected = 0;
           }
        }
        else
           bitsExpected = 4;  
        if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
        {
           if (bitsExpected == 0)
              status = MI_CODINGERR;
           else
              status = MI_BITCOUNTERR;
        }
        else                     // 4 bit received
        {
           MRcvBuffer[0] &= 0x0f; // mask out upper nibble
           switch(MRcvBuffer[0])
           {
              case 0x00: 
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0a:
                 status = MI_OK;
                 break;
              case 0x01:
                 status = MI_VALERR;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
     }

     if ( status == MI_OK)
     {
        PcdSetTmo(1000);     // long timeout 

        ResetInfo(MInfo);   
        MSndBuffer[0]=value[0];
        MSndBuffer[1]=value[1];
        MSndBuffer[2]=value[2];
        MSndBuffer[3]=value[3];
        MInfo.nBytesToSend   = 4;
        status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                            MSndBuffer,
                            MRcvBuffer,
                            &MInfo);
        
        if (status == MI_OK)    // no timeout occured
        {
            if (MInfo.nBitsReceived == 8 && (ReadRC(RegDecoderControl) & 0x01))
            {
               // upper nibble should be equal to lower nibble, otherwise 
               // there is a coding error on card side.
               if ((MRcvBuffer[0] & 0x0f) == ((MRcvBuffer[0] >> 4) & 0x0f))
                  bitsExpected = 8;
               else
               {
                  bitsExpected = 0;
               }
            }
            else
               bitsExpected = 4;  
            if (MInfo.nBitsReceived != bitsExpected)  // ACK / NACK expected
            {
               if (bitsExpected == 0)
                  status = MI_CODINGERR;
               else
                  status = MI_BITCOUNTERR;
            }
            else                     // 4 bit received
            {
               MRcvBuffer[0] &= 0x0f; // mask out upper nibble
               switch(MRcvBuffer[0])
               {
                  case 0x00: 
                     status = MI_NOTAUTHERR;
                     break;
                  case 0x0a:
                     status = MI_OK;
                     break;
                  case 0x05:
                  case 0x01:
                     status = MI_VALERR;
                     break;
                  default:
                     status = MI_CODEERR;
                     break;
               }
            }
        }        
     }

   return status;
}

///////////////////////////////////////////////////////////////////////
//          M I F A R E     H A L T
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccHalt(void)
{
   char status = MI_CODEERR;

   PcdSetTmo(106);
   
   WriteRC(RegChannelRedundancy,0x07); // RxCRC,TxCRC, Parity enable
   
   // ************* Cmd Sequence ********************************** 
   ResetInfo(MInfo);   
   MSndBuffer[0] = PICC_HALT ;     // Halt command code
   MSndBuffer[1] = 0x00;         // dummy address
   MInfo.nBytesToSend   = 2;
   status = PcdSingleResponseCmd(PCD_TRANSCEIVE,
                       MSndBuffer,
                       MRcvBuffer,
                       &MInfo);   
   if (status)
   {
     // timeout error ==> no NAK received ==> OK
     if (status == MI_NOTAGERR || status == MI_ACCESSTIMEOUT)
        status = MI_OK;
   }
    //reset command register - no response from tag
   WriteRC(RegCommand,PCD_IDLE);
   return status; 
}

//////////////////////////////////////////////////////////////////////
//        S E T   D E F A U L T    C O M M   A T T R I B S
///////////////////////////////////////////////////////////////////////
signed char Mf500PcdSetDefaultAttrib(void)
{
   char   status = MI_OK;

   // switch to 106 kBaud (default)
   // if last reader receive baud rate is different to the default value
   // or last reader send baud rate is different to the default value
   if ((MDSI != TCLDSDFLT) || (MDRI != TCLDRDFLT))
   {
      status = Mf500PcdSetAttrib(TCLDSDFLT,TCLDRDFLT);
   }
   return status;
}

//////////////////////////////////////////////////////////////////////
//        G E T   C O M M    A B I L I T I E S 
///////////////////////////////////////////////////////////////////////
signed char Mf500PcdGetAttrib(unsigned char *FSCImax,
                      unsigned char *FSDImax,
                      unsigned char *DSsupp,
                      unsigned char *DRsupp,
                      unsigned char *DREQDS)
{
    *FSCImax = TCLFSDSNDMAX; // max. frame size send
    *FSDImax = TCLFSDRECMAX; // max. frame size rcv
    *DSsupp  = TCLDSMAX;     // max. baudrate PICC --> PCD
    *DRsupp  = TCLDRMAX;     // max. baudrate PCD --> PICC
    *DREQDS  = 0x00; // different send and receive baudrates are
                     // possible
    return(MI_OK);
}

//////////////////////////////////////////////////////////////////////
//        S E T   C O M M    P A R A M E T E R S 
///////////////////////////////////////////////////////////////////////
signed char Mf500PcdSetAttrib(unsigned char DSI,
                      unsigned char DRI)
{
   char   status = MI_OK;
  
  // Return error, if adjusted baudrate not supported by PCD
  if ( ( DSI > TCLDSMAX ) || ( DRI > TCLDRMAX ) )
  {
    status = MI_BAUDRATE_NOT_SUPPORTED ;
  }
  else
  {
    MDSI = DSI;
    MDRI = DRI;
    Mf500PcdWriteAttrib();
  }
  return status;
}

//////////////////////////////////////////////////////////////////////
//        W R I T E   C O M M    P A R A M E T E R S 
///////////////////////////////////////////////////////////////////////
char Mf500PcdWriteAttrib(void)
{
    char   status = MI_OK;

    #if(DEBUG==1)
    printf("\n\r-------------------------");   
    #endif
    // adjust baudrate and pauselength of reader
    WriteRC(RegBPSKDemControl,0x0e);   // RegBPSKDemControl  
    #if(DEBUG==1)
    status=ReadRC(RegBPSKDemControl); 
    printf("\n\rRegBPSKDemControl = %d", status);
    #endif
    
    // set reader send baudrate
    WriteRC(RegCoderControl,MDRCfg[MDRI].CoderRate);
    #if(DEBUG==1)
    status=ReadRC(RegCoderControl); 
    printf("\n\rRegCoderControl = %d", status);
    #endif
    
    WriteRC(RegModWidth,MDRCfg[MDRI].ModWidth);
    #if(DEBUG==1)
    status=ReadRC(RegModWidth); 
    printf("\n\rRegModWidth = %d", status);
    #endif
    
    // set reader receive baudrate
    WriteRC(RegRxControl1,MDSCfg[MDSI].SubCarrierPulses);
    #if(DEBUG==1)
    status=ReadRC(RegRxControl1); 
    printf("\n\rRegRxControl1  = %d", status);
    #endif
    
    WriteRC(RegDecoderControl,MDSCfg[MDSI].RxCoding);      
    #if(DEBUG==1)
    status=ReadRC(RegDecoderControl); 
    printf("\n\rRegDecoderControl  = %d", status);
    #endif
    
    WriteRC(RegRxThreshold,MDSCfg[MDSI].RxThreshold);
    #if(DEBUG==1)
    status=ReadRC(RegRxThreshold); 
    printf("\n\rRegRxThreshold  = %d", status);
    #endif
    WriteRC(RegBPSKDemControl,MDSCfg[MDSI].BPSKDemControl);
    
    #if(DEBUG==1)
    status=ReadRC(RegBPSKDemControl); 
    printf("\n\rRegBPSKDemControl  = %d", status);
    #endif
    
    status = MI_OK;
    return(status);
}
 
//////////////////////////////////////////////////////////////////////
//           P I C C   E X C H A N G E   B L O C K
///////////////////////////////////////////////////////////////////////
signed char Mf500PiccExchangeBlock(unsigned char *send_data,
                           unsigned short send_bytelen,
                           unsigned char *rec_data,  
                           unsigned short *rec_bytelen,
                           unsigned char append_crc, 
                           unsigned long timeout )
{
   char   status = MI_OK;

   if (append_crc)
   {
      // RxCRC and TxCRC enable, parity enable
      WriteRC(RegChannelRedundancy,0x0F); 
      send_bytelen -= 2;
   }
   else
   {
      // RxCRC and TxCRC disable, parity enable
      WriteRC(RegChannelRedundancy,0x03); 
   }
   	
	PcdSetTmo(timeout);

   status = ExchangeByteStream(PCD_TRANSCEIVE,
                               send_data,
                               send_bytelen,
                               rec_data,
                               rec_bytelen);
   // even if an error occured, the data should be
   // returned - for debugging reasons

   if (append_crc)
   {
      *rec_bytelen += 2; // for two CRC bytes
      rec_data[*rec_bytelen - 2] = 0x00;
      rec_data[*rec_bytelen - 1] = 0x00;
   }

   return status;
}                      

///////////////////////////////////////////////////////////////////////
//         S e t   T i m e o u t   L E N G T H
///////////////////////////////////////////////////////////////////////
signed char PcdSetTmo(unsigned long tmoLength)
{
    unsigned char prescale = 7;
    unsigned long reload = tmoLength;

    while (reload > 255)
    {
       prescale++;
       reload = reload >> 1; // division by 2
    }
    // if one of the higher bits are set, the prescaler is set
    // to the largest value
    if (prescale > 0x15)
    {
       prescale = 0x15;
       reload   = 0xFF;
    }

    // fcnt = 13560000 / (2^prescale)
    // T = (reload - counter) / fcnt
    WriteRC(RegTimerClock,prescale); // TAutoRestart=0,TPrescale=prescale
    WriteRC(RegTimerReload,reload);// TReloadVal = reload
    
    return MI_OK;
}

///////////////////////////////////////////////////////////////////////
//      M I F A R E   M O D U L E   R E S E T 
///////////////////////////////////////////////////////////////////////

#if(ReaderType!=NewRoutine)

signed char PcdReset(void)
{
char status = MI_OK;
unsigned int i=0;

   #ifdef STM32F2XX
   R_RST(1);
   GUI_Delay(500);
   R_RST(0);
   GUI_Delay(500);
   #else
   R_RST=1;
   GUI_Delay(500);
   R_RST=0;
   GUI_Delay(500);

	#endif
	
	
	
   #if(DEBUG==1)
   printf("\n\rHReset Done");
   #endif

   // wait until reset command recognized
   //while (((ReadRC(RegCommand) & 0x3F) != 0x3F));

   // while reset sequence in progress
   while ((ReadRC(RegCommand) & 0x3F))
   {
     if(++i>7000) 
       return(MI_INTERFACEERR);
   }

   #if(DEBUG==1)
   printf("\n\rCommand Recognized");
   #endif
   
   WriteRC(0x00,0); //denay PageSelect method

   //WriteRC(RegPage,0x80); // Dummy access in order to determine the bus 
                             // configuration
   // necessary read access 
   // after first write access, the returned value
   // should be zero ==> interface recognized

   if (ReadRC(RegCommand) != 0x00)
   {          
       //status = MI_INTERFACEERR;
   }

   return status;
}
#endif

///////////////////////////////////////////////////////////////////////
//           E X C H A N G E   B Y T E   S T R E A M
///////////////////////////////////////////////////////////////////////
signed char ExchangeByteStream(unsigned char Cmd,
                        unsigned char *send_data,
                        unsigned short send_bytelen,
                        unsigned char *rec_data,  
                        unsigned short *rec_bytelen)
{
   signed char status = MI_OK;
   int j;
   
   FlushFIFO();    // empty FIFO
   ResetInfo(MInfo); // initialise ISR Info structure

   if (send_bytelen > 0)
   {
      for(j=0; j<send_bytelen; j++) MSndBuffer[j]=send_data[j]; // write n bytes
      MInfo.nBytesToSend = send_bytelen;
      // write load command
      status = PcdSingleResponseCmd(Cmd,
                      MSndBuffer,
                      MRcvBuffer,
                      &MInfo);
      *rec_bytelen = MInfo.nBytesReceived;
      // copy data to output, even in case of an error
      if (*rec_bytelen)
      {
        for(j=0; j<*rec_bytelen; j++) rec_data[j]=MRcvBuffer[j];
      }
   }
   else
   {
      status = MI_WRONG_PARAMETER_VALUE;
   }
   return status;
}                                   

///////////////////////////////////////////////////////////////////////
//          R E A D   S N R   O F   R E A D E R   I C
///////////////////////////////////////////////////////////////////////
signed char PcdGetSnr(unsigned char* snr)
{
   signed char status;
   
   status = PcdReadE2(0x08,0x04,snr);
   return status;
}


///////////////////////////////////////////////////////////////////////
//          E E P R O M   R E A D   
///////////////////////////////////////////////////////////////////////
signed char PcdReadE2(unsigned short startaddr,
               unsigned char length,
               unsigned char* data)
{
   char status = MI_OK;
   int j;
   
   //PcdSetTmo(3400);
   PcdSetTmo(6800);
   // ************* Cmd Sequence ********************************** 
   ResetInfo(MInfo);   
   MSndBuffer[0] = startaddr & 0xFF;
   MSndBuffer[1] = (startaddr >> 8) & 0xFF;
   MSndBuffer[2] = length;
   MInfo.nBytesToSend   = 3;
   status = PcdSingleResponseCmd(PCD_READE2,
                         MSndBuffer,
                         MRcvBuffer,
                         &MInfo);
   // return data, even in case of an error - for debugging reasons                         
   if (MInfo.nBytesReceived >= length)
      for(j=0; j<length;j++) data[j]=MRcvBuffer[j]; 
   else
      for(j=0; j<MInfo.nBytesReceived;j++) data[j]=MRcvBuffer[j]; 
   return status ;
}


///////////////////////////////////////////////////////////////////////
//          E E P R O M   W R I T E 
///////////////////////////////////////////////////////////////////////
signed char PcdWriteE2(  unsigned int startaddr,
                      unsigned char length,
                      unsigned char* data)
{
   char status = MI_OK;  
   int j;

   PcdSetTmo(6800);
     // ************* Cmd Sequence ********************************** 
   ResetInfo(MInfo);   
   MSndBuffer[0] = startaddr & 0xFF;
   MSndBuffer[1] = (startaddr >> 8) & 0xFF;
   for(j=0; j<length;j++) MSndBuffer[j+2]=data[j]; 
   MInfo.nBytesToSend   = length + 2;
         
   status = PcdSingleResponseCmd(PCD_WRITEE2,
                   MSndBuffer,
                   MRcvBuffer,
                   &MInfo); // write e2
   return status;
}   

//////////////////////////////////////////////////////////////////////
//                 R E S E T 
///////////////////////////////////////////////////////////////////////
signed char PcdRfReset(unsigned short ms)
{
   char   status = MI_OK;

   ClearBitMask(RegTxControl,0x03);  // Tx2RF-En, Tx1RF-En disablen
   if (ms > 0)
   {
  		GUI_Delay(ms);
      SetBitMask(RegTxControl,0x03);    // Tx2RF-En, Tx1RF-En enable
   }  
   return status;
}

//////////////////////////////////////////////////////////////////////
//   S E T   A   B I T   M A S K 
///////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask) // 
{
   char   tmp    = 0x0;


	#ifdef CLRC_CHIP
  tmp = ReadRC(reg);
  WriteRC(reg,tmp | mask);  // set bit mask
	#endif

	

	#ifndef CLRC_CHIP
  tmp = Read_MFRC522(reg);
  Write_MFRC522(reg,tmp | mask);  // set bit mask
  #endif

	
}

//////////////////////////////////////////////////////////////////////
//   C L E A R   A   B I T   M A S K 
///////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask) // 
{
   unsigned char   tmp    = 0x0;


		#ifdef CLRC_CHIP
    tmp = ReadRC(reg);
    WriteRC(reg,tmp & ~mask);  // clear bit mask
		#endif

  	#ifndef CLRC_CHIP
  	tmp = Read_MFRC522(reg);
    Write_MFRC522(reg,tmp & ~mask);  // clear bit mask
		#endif

}

///////////////////////////////////////////////////////////////////////
//                  F L U S H    F I F O
///////////////////////////////////////////////////////////////////////
void FlushFIFO(void)
{  
   SetBitMask(RegControl,0x01);
}

//////////////////////////////////////////////////////////////////////
//       W R I T E   A   P C D   C O M M A N D 
///////////////////////////////////////////////////////////////////////
char  PcdSingleResponseCmd(unsigned char cmd,
               volatile unsigned char* send, 
               volatile unsigned char* rcv,
               volatile MfCmdInfo *info)
{     
int           k=0;
char          status    = MI_OK;
unsigned char lastBits;
unsigned char validErrorFlags = 0x1F; 
unsigned char rxMultiple = 0x00;
unsigned char irqEn     = 0x00;
unsigned char waitFor   = 0x00;
unsigned char timerCtl  = 0x02;
unsigned char irqBits;
unsigned char irqMask;            
unsigned char nbytes;
unsigned char cnt, i;
unsigned char waterLevelBackup;
unsigned long int Li=0;

   WriteRC(RegInterruptEn,0x7F); // disable all interrupts
   WriteRC(RegInterruptRq,0x7F); // reset interrupt requests

   // please pay attention to the sequence of following commands
   // at first empty the FIFO
   // second wait for probably e2 programming in progress
   // third cancel command
   //
   // ATTENTION: the guard timer must not expire earlier than 10 ms
   FlushFIFO();            // flush FIFO buffer

   #if(DEBUG==1)
   printf("\n\rPcdSingleResponseCmd");
   #endif

   // wait until e2 programming is finished
	 k=0;
   while (((ReadRC(RegSecondaryStatus) & 0x40) == 0)) if(++k>100) break;

   #if(DEBUG==1)
   printf("\n\rE2 Done");
   #endif

   WriteRC(RegCommand,PCD_IDLE); // terminate probably running command

   // Set water level to the default value (see. 'PcdBasicRegisterConfiguration()')   
   waterLevelBackup = ReadRC(RegFIFOLevel);
   WriteRC(RegFIFOLevel,0x20);
   
   // save info structures to module pointers
   MpIsrInfo = info;  
   MpIsrOut  = send;
   MpIsrIn   = rcv;

   info->irqSource = 0x0; // reset interrupt flags

   // depending on the command code, appropriate interrupts are enabled (irqEn)
   // and the commit interrupt is choosen (waitFor).
   switch(cmd)
   {
      case PCD_IDLE:                   // nothing else required
         irqEn = 0x00;
         waitFor = 0x00;
         break;
      case PCD_WRITEE2:                // LoAlert and TxIRq
         timerCtl = 0x00;              // start and stop timer manually            
         irqEn = 0x11;
         waitFor = 0x10;
		 validErrorFlags &= ~0x08;     // hb 20.06.2001 don't check CRC errors         
         break;
      case PCD_READE2:                 // HiAlert, LoAlert and IdleIRq
         timerCtl = 0x00;              // start and stop timer manually            
         irqEn = 0x07;
         waitFor = 0x04;
		 validErrorFlags &= ~0x08;     // wu 15.05.2001 don't check CRC errors
         break;
      case PCD_LOADKEYE2:              // IdleIRq and LoAlert
         timerCtl = 0x00;              // start and stop timer manually            
		 validErrorFlags &= ~0x08;     // hb 20.06.2001 don't check CRC errors               
         irqEn = 0x05;
         waitFor = 0x04;
         break;
      case PCD_LOADCONFIG:             // IdleIRq and LoAlert
         timerCtl = 0x00;              // start and stop timer manually            
         irqEn = 0x05;
         waitFor = 0x04;
         break;
      case PCD_AUTHENT1:               // IdleIRq and LoAlert
         irqEn = 0x05;
         waitFor = 0x04;
         break;
      case PCD_CALCCRC:                // LoAlert and TxIRq
         timerCtl = 0x00;              // start and stop timer manually            
         irqEn = 0x11;
         waitFor = 0x10;
         break;
      case PCD_AUTHENT2:               // IdleIRq
         irqEn = 0x04;
         waitFor = 0x04;
         break;
      case PCD_RECEIVE:                // HiAlert and IdleIRq
         info->nBitsReceived = -(ReadRC(RegBitFraming) >> 4);      
         timerCtl = 0x04;              // start timer manually and stop
                                       // with first bit received
         irqEn = 0x06;
         waitFor = 0x04;
         break;
      case PCD_LOADKEY:                // IdleIRq
         timerCtl = 0x00;              // start and stop timer manually            
         irqEn = 0x05;
         waitFor = 0x04;
         break;
      case PCD_TRANSMIT:               // LoAlert and IdleIRq
         timerCtl = 0x00;              // start and stop timer manually            
         irqEn = 0x05;
         waitFor = 0x04;
         break;
      case PCD_TRANSCEIVE:             // TxIrq, RxIrq, IdleIRq and LoAlert
         info->nBitsReceived = -(ReadRC(RegBitFraming) >> 4);
         irqEn = 0x3D;
         waitFor = 0x04;
         break;
      default:
         status = MI_UNKNOWN_COMMAND;
   }  
   
   if (status == MI_OK)
   {
      rxMultiple = ReadRC(RegDecoderControl) & 0x40;
      if (!rxMultiple)
         SetBitMask(RegDecoderControl,0x40);
      
      // Initialize uC Timer for global Timeout management
      irqEn |= 0x20;                        // always enable timout irq
      waitFor |= 0x20;                      // always wait for timeout 

      WriteRC(RegInterruptEn,irqEn | 0x80);  //necessary interrupts are enabled // count up from 1
      
      WriteRC(RegTimerControl,timerCtl);
      if (~timerCtl & 0x02) // if no start condition could be detected, then
                              // start timer manually
      {
         SetBitMask(RegControl,0x02);
      }
      
      MpIsrInfo->nBytesSent=0;
      
                    // write remaining data to the FIFO
      for ( cnt = 0;cnt < MpIsrInfo->nBytesToSend;cnt++)
                    {
                       WriteRC(RegFIFOData,MpIsrOut[MpIsrInfo->nBytesSent]);
                       #if(DEBUG==1)
                       printf("%d ,",MpIsrOut[MpIsrInfo->nBytesSent]);
                       #endif
                       MpIsrInfo->nBytesSent++;
                    }

      WriteRC(RegCommand,cmd);               //start command   
      
      //irqMask = ReadRC(RegInterruptEn); // read enabled interrupts
      //printf("\n\RegInterruptEn2=%X %X",irqEn, irqMask);
      // wait for commmand completion
      // a command is completed, if the corresponding interrupt occurs
      // or a timeout is signaled  
      while (!(MpIsrInfo->irqSource & waitFor))
                                         // wait for cmd completion or timeout
      {
        
        if(++Li>500) //100000
        {
          Mf500PcdConfig();
          //#if(DeviceType==Simorgh)
          //SaveExtraOP(39);
          //#endif
          return(MI_NY_IMPLEMENTED);
        }
        
        if(ReadRC(RegPrimaryStatus) & 0x08) // loop while IRQ pending
              {
                 //printf("\n\rInt. pending");
                 MpIsrInfo->errFlags = ReadRC(RegErrorFlag) & 0x0F; // save error state

                 irqMask = ReadRC(RegInterruptEn); // read enabled interrupts

                 // read pending interrupts
                 //irqBits = ReadRC(RegInterruptRq);

                 irqBits = ReadRC(RegInterruptRq) & irqMask;
                 
                 //printf("\n\rIRQ=%X %X",irqMask,irqBits);
                 
                 MpIsrInfo->irqSource |= irqBits; // save pending interrupts
        
                 //************ LoAlertIRQ ******************
                 if (irqBits & 0x01)    // LoAlert
                 {  
                    #if(DEBUG==1)
                    printf("\n\rLoAlert");
                    #endif
                    /*
                    nbytes = DEF_FIFO_LENGTH - ReadRC(RegFIFOLength);
                    // less bytes to send, than space in FIFO
                    if ((MpIsrInfo->nBytesToSend - MpIsrInfo->nBytesSent) <= nbytes)
                    {
                       nbytes = MpIsrInfo->nBytesToSend - MpIsrInfo->nBytesSent;
                     WriteRC(RegInterruptEn,0x01); // disable LoAlert IRQ
                    }
                    // write remaining data to the FIFO
                    for ( cnt = 0;cnt < nbytes;cnt++)
                    {
                       WriteRC(RegFIFOData,MpIsrOut[MpIsrInfo->nBytesSent]);
                       #if(DEBUG==1)
                       printf("%d,",MpIsrOut[MpIsrInfo->nBytesSent]);
                       #endif
                       MpIsrInfo->nBytesSent++;
                    } 
                    */
										GUI_Delay(1);
                    WriteRC(RegInterruptRq,0x01);  // reset IRQ bit
                 }
              
                 //************* TxIRQ Handling **************
                 if (irqBits & 0x10)       // TxIRQ
                 {
                    #if(DEBUG==1)
                    printf("\n\rTxIRQ");
                    #endif
                    WriteRC(RegInterruptRq,0x10);    // reset IRQ bit 
                    WriteRC(RegInterruptEn,0x82);    // enable HiAlert Irq for
                                                        // response
                    if (MpIsrInfo->RxAlignWA) // if cmd is anticollision and 7 bits are known
                    {                                    // switch off parity generation
                       WriteRC(RegChannelRedundancy,0x02); // RxCRC and TxCRC disable, parity disable               
                    }
                 }
                 //**************** RxIRQ Handling *******************************
                 if (irqBits & 0x08) // RxIRQ - possible End of response processing
                 {
                    #if(DEBUG==1)
                    printf("\n\rRxIRQ");
                    #endif
                     // no error or collision during
                    if (MpIsrInfo->DisableDF || (MpIsrInfo->errFlags == 0x00))
                    {
                       WriteRC(RegCommand,0x00); // cancel current command
                       irqBits |= 0x04; // set idle flag in order to signal the end of
                                        // processing. For single reponse processing, this
                                        // flag is already set.
                    }
                    else // error occured - flush data and continue receiving
                    {
                       MpIsrInfo->saveErrorState = MpIsrInfo->errFlags; // save error state
                       MpIsrInfo->errFlags = 0; // reset error flags for next receiption
                       WriteRC(RegControl,0x01);
                       MpIsrInfo->nBytesReceived = 0x00;
                       irqBits &= ~0x08; // clear interrupt request
                       WriteRC(RegInterruptRq,0x08);
                    }
                 }
        
                 //************* HiAlertIRQ or RxIRQ Handling ******************
                 if (irqBits & 0x0E) // HiAlert, Idle or valid RxIRQ
                 {
                   
                    #if(DEBUG==1)
                    printf("\n\rHiAlert");
                    #endif
                    // read some bytes ( length of FIFO queue)              
                    // into the receive buffer
                    nbytes = ReadRC(RegFIFOLength);
                    // read date from the FIFO and store them in the receive buffer
									  k=0;
                    do
                    {
                        if(++k>1000)
                          break;
                        for ( cnt = 0; cnt < nbytes; cnt++)               
                        {
                           // accept no more data, than reserved memory space
                           if (MpIsrInfo->nBytesReceived < MEMORY_BUFFER_SIZE) 
                           {
                              MpIsrIn[MpIsrInfo->nBytesReceived] = ReadRC(RegFIFOData);
                              MpIsrInfo->nBytesReceived++;
                           }
                           else
                           {
                              MpIsrInfo->status = MI_RECBUF_OVERFLOW;
                              break;
                           }
                        }
                        // check for remaining bytes
                        nbytes = ReadRC(RegFIFOLength);
                    }
                    while (MpIsrInfo->status == MI_OK && nbytes > 0);
                    
                    WriteRC(RegInterruptRq,0x0A & irqBits);  
                                               // reset IRQ bit - idle irq will
                                               // be deleted in a seperate section
                 }   
        
                 //************* additional HiAlertIRQ Handling ***
                 if (irqBits & 0x02)
                 {
                    #if(DEBUG==1)
                    printf("\n\radditional HiAlertIRQ");
                    #endif
                    // if highAlertIRQ is pending and the receiver is still
                    // running, then the timeout counter should be stopped,
                    // otherwise a timeout could occure while receiving 
                    // correct data
                    if ((ReadRC(RegPrimaryStatus) & 0x70) == 0x70)
                    {      
                       cnt = ReadRC(RegControl); // read control register
                       WriteRC(RegControl,cnt|0x04); // stop reader IC timer
                                                        // write modified register
                    }
                 }
        
                 //************** additional IdleIRQ Handling ******
                 if (irqBits & 0x04)     // Idle IRQ
                 {
                    #if(DEBUG==1)
                    printf("\n\rIdle IRQ");
                    #endif
                    WriteRC(RegInterruptEn,0x20); // disable Timer IRQ
                    WriteRC(RegInterruptRq,0x24); // disable Timer IRQ request
                    irqBits &= ~0x20;   // clear Timer IRQ in local var
                    MpIsrInfo->irqSource &= ~0x20; // clear Timer IRQ in info var
                                                // when idle received, then cancel
                                                // timeout
                    MpIsrInfo->irqSource |= 0x04; // set idle-flag in case of valid rx-irq
                    // status should still be MI_OK
                    // no error - only used for wake up
                 }
               
                 //************* TimerIRQ Handling ***********
                 if (irqBits & 0x20)       // timer IRQ
                 {
                    #if(DEBUG==1)
                    printf("\n\rtimer IRQ");
                    #endif
                    WriteRC(RegInterruptRq,0x20); // reset IRQ bit 
                    // only if no other error occured
                    if (MpIsrInfo->status == MI_OK)
                    {
											 if((cmd==PCD_TRANSCEIVE)&&(MpIsrInfo->nBytesToSend==4))//MM
											   MpIsrInfo->status = MI_OK;  //ignore error
                       else
											   MpIsrInfo->status = MI_NOTAGERR; // timeout error
                    }
                 }
              }
      }//while                                   
      
   #if(DEBUG==1)
   printf("\n\rCurrent Command Status=%d",ReadRC(RegCommand));
      
   printf("\n\rCurrent IRQ Register=%d",ReadRC(RegInterruptRq));
   
   printf("\n\rCurrent Modem Status=%d",ReadRC(RegPrimaryStatus));

   printf("\n\rCurrent Error Status=%d",ReadRC(RegErrorFlag));
   #endif

   // restore the previous value for the FIFO water level
   WriteRC(RegFIFOLevel, waterLevelBackup);
      #if(DEBUG==1)
         lastBits = ReadRC(RegSecondaryStatus) & 0x07;
         printf("\n\r1>LastBits:%d",lastBits);
      printf("\n\rMpIsrInfo");
      printf("\n\r===================");
      printf("\n\rcmd=%d",MpIsrInfo->cmd);
      printf("\n\rstatus=%d",MpIsrInfo->status);
      printf("\n\rnBytesSent=%d",MpIsrInfo->nBytesSent); 
      printf("\n\rnBytesToSend=%d",MpIsrInfo->nBytesToSend);
      printf("\n\rnBytesReceived=%d",MpIsrInfo->nBytesReceived);
      printf("\n\rnBitsReceived=%d",MpIsrInfo->nBitsReceived); 
      printf("\n\rirqSource=%d",MpIsrInfo->irqSource); 
      printf("\n\rcollPos=%d",MpIsrInfo->collPos); 
      printf("\n\rerrFlags=%d",MpIsrInfo->errFlags);
      printf("\n\rsaveErrorState=%d",MpIsrInfo->saveErrorState); 
      printf("\n\rRxAlignWA=%d",MpIsrInfo->RxAlignWA);
      printf("\n\rDisableDF=%d",MpIsrInfo->DisableDF); 
      
      printf("\n\rReceived Data:");
      for(i=0; i<MpIsrInfo->nBytesReceived; i++)
        printf("%X", MpIsrIn[i]);
      printf("\n\r////////////////////////");
      #endif

      
      SetBitMask(RegControl,0x04);         // stop timer now
      WriteRC(RegInterruptEn,0x7F);          // disable all interrupts
      WriteRC(RegInterruptRq,0x7F);          // clear all interrupt requests
      
      WriteRC(RegCommand,PCD_IDLE);          // reset command register

      status = MpIsrInfo->status;          // set status
      if (MpIsrInfo->irqSource & 0x20) // if timeout expired - look at old error state
         MpIsrInfo->errFlags |= MpIsrInfo->saveErrorState;
      MpIsrInfo->errFlags &=  validErrorFlags;
      if (MpIsrInfo->errFlags) // error occured
      {
         if (MpIsrInfo->errFlags & 0x01)   // collision detected
         {
            info->collPos = ReadRC(RegCollPos); // read collision position
            status = MI_COLLERR;
         }
         else
         {
            if (MpIsrInfo->errFlags & 0x02)   // parity error
            {
               status = MI_PARITYERR;
            }
         }
         if (MpIsrInfo->errFlags & 0x04)   // framing error
         {
            status = MI_FRAMINGERR;
         }
         else
         if (MpIsrInfo->errFlags & 0x10)   // FIFO overflow
         {
            FlushFIFO();
            status = MI_OVFLERR;
         }
         else
         if (MpIsrInfo->errFlags & 0x08) // CRC error
         {
            status = MI_CRCERR;
         }
         else
         if (status == MI_OK)
            status = MI_NY_IMPLEMENTED;
         // key error occures always, because of 
         // missing crypto 1 keys loaded
      }
      // if the last command was TRANSCEIVE, the number of 
      // received bits must be calculated - even if an error occured
      if (cmd == PCD_TRANSCEIVE || cmd == PCD_RECEIVE)
      {
         // number of bits in the last byte
         lastBits = ReadRC(RegSecondaryStatus) & 0x07;
         if (lastBits)
            info->nBitsReceived += (info->nBytesReceived-1) * 8 + lastBits;
         else
            info->nBitsReceived += info->nBytesReceived * 8; 
      }
      
      if (!rxMultiple)
         ClearBitMask(RegDecoderControl,0x40);
   }//if(Status==MI_OK)

   MpIsrInfo = 0;         // reset interface variables for ISR
   MpIsrOut  = 0;
   MpIsrIn   = 0; 


   return status;

}   

/////////////////////////////////////////////////////////////////////
//          B A S I C   R E G I S T E R   S E T T I N G S
///////////////////////////////////////////////////////////////////////
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TxAutoReg             0x15
#define     ModeReg               0x11
char PcdBasicRegisterConfiguration(void)
{
unsigned char status;

  // test clock Q calibration - value in the range of 0x46 expected
  WriteRC(RegClockQControl,0x0);
  #if(DEBUG==1)
  status=ReadRC(RegClockQControl); //OK
  printf("\n\rRegClockQControl = %d",status);
  #endif
  
  WriteRC(RegClockQControl,0x40);
  #if(DEBUG==1)
  status=ReadRC(RegClockQControl); //ok
  printf("\n\rRegClockQControl = %d",status);
  #endif
  
	GUI_Delay(10);
  ClearBitMask(RegClockQControl,0x40); // clear bit ClkQCalib for 
                                       // further calibration
  // enable auto power down
  WriteRC(RegRxControl2,0x41);
  #if(DEBUG==1)
  status=ReadRC(RegRxControl2); //ok
  printf("\n\rRegRxControl2 = %d",status);
  #endif
  
  WriteRC(RegIRqPinConfig,0x3); // interrupt active low enable
  #if(DEBUG==1)
  status=ReadRC(RegIRqPinConfig); //ok
  printf("\n\rRegIRqPinConfig = %d",status);
  #endif
	

  return MI_OK;
}    
//==============================================================================
void ConfigureMFRC531(void)
{
    Mf500PcdConfig();
}

//////////////////////////////////////////////////////////////////////
//       W R I T E   A   P C D   C O M M A N D 
///////////////////////////////////////////////////////////////////////
char  MFRCWait(unsigned long times)
{     
int           k=0;
char          status    = MI_OK;
unsigned char lastBits;
unsigned char validErrorFlags = 0x1F; 
unsigned char rxMultiple = 0x00;
unsigned char irqEn     = 0x00;
unsigned char waitFor   = 0x00;
unsigned char timerCtl  = 0x01;
unsigned char irqBits;
unsigned char irqMask;            
unsigned char nbytes;
unsigned char cnt, i;
unsigned char waterLevelBackup;
unsigned long int Li=0;

   OpenRC();
   /*
   Timer frequency must be calculate in these steps
   f(clock frequency)=13560000(Hz)/2^Prescaler
   Sample
   When Prescaler is 18 then
   f=13560000/2^18 = 51.727 Hz
   
   timer executed in each 1000(ms)/51.727=19.33ms
   then for 4sec delay you must set Reload value to 4000(ms)/19.33(ms)=207
   */
   
   //4 Sec
   if(times==1)
   {
     WriteRC(RegTimerClock,0x12); // TAutoRestart=0,TPrescale=128
     WriteRC(RegTimerReload,207);// TReloadVal = '
   }
   if(times==2)
   {
     WriteRC(RegTimerClock,0x15); // TAutoRestart=0,TPrescale=128
     WriteRC(RegTimerReload,194);// TReloadVal = '
   }
   
   ResetInfo(MInfo); 
   
   WriteRC(RegInterruptEn,0x7F); // disable all interrupts
   WriteRC(RegInterruptRq,0x7F); // reset interrupt requests

   // ATTENTION: the guard timer must not expire earlier than 10 ms
   FlushFIFO();            // flush FIFO buffer

   // wait until e2 programming is finished
	 k=0;
   while (((ReadRC(RegSecondaryStatus) & 0x40) == 0)) if(++k>100) break;

   WriteRC(RegCommand,PCD_IDLE); // terminate probably running command

   // Set water level to the default value (see. 'PcdBasicRegisterConfiguration()')   
   waterLevelBackup = ReadRC(RegFIFOLevel);
   WriteRC(RegFIFOLevel,0x20);
   
   // depending on the command code, appropriate interrupts are enabled (irqEn)
   // and the commit interrupt is choosen (waitFor).
   irqEn = 0x00;
   waitFor = 0x00;
   
   if (status == MI_OK)
   {
      rxMultiple = ReadRC(RegDecoderControl) & 0x40;
      if (!rxMultiple)
         SetBitMask(RegDecoderControl,0x40);
      
      // Initialize uC Timer for global Timeout management
      irqEn |= 0x20;                        // always enable timout irq
      waitFor |= 0x20;                      // always wait for timeout 

      WriteRC(RegInterruptEn,irqEn | 0x80);  //necessary interrupts are enabled // count up from 1
      
      WriteRC(RegTimerControl,timerCtl);
      if (~timerCtl & 0x02) // if no start condition could be detected, then
                              // start timer manually
      {
         SetBitMask(RegControl,0x02);
      }
      
      MpIsrInfo->nBytesSent=0;
      
      WriteRC(RegCommand,PCD_IDLE);               //start command   
      
      while (1)  // wait for cmd completion or timeout
      {
        /*
        if(++Li>500) //100000
        {
          Mf500PcdConfig();
          #if(DeviceType==Simorgh)
          SaveExtraOP(39);
          #endif
          return(MI_NY_IMPLEMENTED);
        }
        */
        
        if(ReadRC(RegPrimaryStatus) & 0x08) // loop while IRQ pending
              {
                 MpIsrInfo->errFlags = ReadRC(RegErrorFlag) & 0x0F; // save error state

                 irqMask = ReadRC(RegInterruptEn); // read enabled interrupts

                 // read pending interrupts
                 //irqBits = ReadRC(RegInterruptRq);

                 irqBits = ReadRC(RegInterruptRq) & irqMask;
                 
                 //printf("\n\rIRQ=%X %X",irqMask,irqBits);
                 
                 MpIsrInfo->irqSource |= irqBits; // save pending interrupts
        
                 //************* TimerIRQ Handling ***********
                 if (irqBits & 0x20)       // timer IRQ
                 {
                    //printf("\n\rtimer IRQ");
                    WriteRC(RegInterruptRq,0x20); // reset IRQ bit 
                    MpIsrInfo->status == MI_OK;
                    break;
                 }
              }
      }//while                                   
      

   // restore the previous value for the FIFO water level
   WriteRC(RegFIFOLevel, waterLevelBackup);
      
    SetBitMask(RegControl,0x04);         // stop timer now
    WriteRC(RegInterruptEn,0x7F);          // disable all interrupts
    WriteRC(RegInterruptRq,0x7F);          // clear all interrupt requests
      
    WriteRC(RegCommand,PCD_IDLE);          // reset command register

    status = MpIsrInfo->status;          // set status
    if (MpIsrInfo->irqSource & 0x20) // if timeout expired - look at old error state
       MpIsrInfo->errFlags |= MpIsrInfo->saveErrorState;
    MpIsrInfo->errFlags &=  validErrorFlags;
  }//if(Status==MI_OK)

  MpIsrInfo = 0;         // reset interface variables for ISR
  MpIsrOut  = 0;
  MpIsrIn   = 0; 

  return status;

}   

//data array maxium length
#define MAX_LEN 16

/////////////////////////////////////////////////////////////////////
//set the pin
/////////////////////////////////////////////////////////////////////
const int chipSelectPin = 10;
const int NRSTPD = 5;


//4 bytes Serial number of card, the 5 bytes is verfiy bytes
uchar serNum[5];

uchar  writeData[16]={
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100};  //initialize to 100 USD
uchar  moneyConsume = 18 ;  //Deduct 18 USD
uchar  moneyAdd = 10 ;  //Charge up 10 USD
//buffer A password, 16 buffer, the passowrd of every buffer is 6 byte 
uchar sectorKeyA[16][16] = {
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
  //{0x19, 0x84, 0x07, 0x15, 0x76, 0x14},
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
};
uchar sectorNewKeyA[16][16] = {
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14  }
  ,
  //you can set another ket , such as  " 0x19, 0x84, 0x07, 0x15, 0x76, 0x14 "
  //{0x19, 0x84, 0x07, 0x15, 0x76, 0x14, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14},
  // but when loop, please set the  sectorKeyA, the same key, so that RFID module can read the card
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x33,0x07,0x15,0x34,0x14  }
  ,
};

////////////////////////////////////////////////////////////////////////////////
//          G E N E R I C    W R I T E
////////////////////////////////////////////////////////////////////////////////
/*
 * Function:Write_MFRC5200
 * Description:write a byte data into one register of MR RC522
 * Input parameter:addr--register address;val--the value that need to write in
 * Return:Null
 */
void Write_MFRC522(uchar addr, uchar val)
{
unsigned char Byte;

	 #ifdef STM32F2XX
   R_CS(0);
   Byte=(addr<<1)&0x7E;
   spi(Byte);
   spi(val);
   R_CS(1);
	 #else
   R_CS=0;
   Byte=(addr<<1)&0x7E;
   spi(Byte);
   spi(val);
   R_CS=1;
	 #endif
}


/*
 * Function:Read_MFRC522
 * Description:read a byte data into one register of MR RC522
 * Input parameter:addr--register address
 * Return:return the read value
 */
uchar Read_MFRC522(uchar addr)
{
unsigned char Byte;
   #ifdef STM32F2XX
   R_CS(0);
   Byte=spi((addr<<1)|0x80);   
   Byte=spi(0);   
   R_CS(1);
   #else
   R_CS=0;
   Byte=spi((addr<<1)|0x80);   
   Byte=spi(0);   
   R_CS=1;
	 #endif
	
   return Byte;
}




/*
 * Function:AntennaOn
 * Description:Turn on antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter:null
 * Return:null
 */
void AntennaOn(void)
{
  uchar temp;

  temp = Read_MFRC522(TxControlReg);

	
  if (!(temp & 0x03))
  {
    SetBitMask(TxControlReg, 0x03);
  }
}


/*
 * Function:AntennaOff
 * Description:Turn off antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter:null
 * Return:null
 */
void AntennaOff(void)
{
  ClearBitMask(TxControlReg, 0x03);
}


/*
 * Function:ResetMFRC522
 * Description: reset RC522
 * Input parameter:null
 * Return:null
 */
void MFRC522_Reset(void)
{
  Write_MFRC522(CommandReg, MFRC522_PCD_RESETPHASE);
}


/*
 * Function:InitMFRC522
 * Description:initilize RC522
 * Input parameter:null
 * Return:null
 */
void MFRC522_Init(void)
{
	#ifdef STM32F2XX
	R_RST(0);
	GUI_Delay(100);
	R_RST(1);
	GUI_Delay(100);
	#else
	R_RST=0;
	GUI_Delay(100);
	R_RST=1;
	GUI_Delay(100);
	#endif

	
  MFRC522_Reset();

	Write_MFRC522(MFRC522_REG_T_PRESCALER, 0x3E);

	Write_MFRC522(MFRC522_REG_T_MODE, 0x8D);
	Write_MFRC522(MFRC522_REG_T_RELOAD_L, 30);
	Write_MFRC522(MFRC522_REG_T_RELOAD_H, 0);
	Write_MFRC522(MFRC522_REG_TX_AUTO, 0x40);
	Write_MFRC522(MFRC522_REG_MODE, 0x3D);
  ClearBitMask(MFRC522_REG_STATUS2, 0x08);
	Write_MFRC522(MFRC522_REG_MODE, 0x3D);
	Write_MFRC522(MFRC522_REG_RX_SELL, 0x86);
	Write_MFRC522(MFRC522_REG_RF_CFG, 0x7F);
	Write_MFRC522(MFRC522_REG_T_RELOAD_L, 30);
	Write_MFRC522(MFRC522_REG_T_RELOAD_H, 0);
	Write_MFRC522(MFRC522_REG_T_MODE, 0x8D);
	Write_MFRC522(MFRC522_REG_T_PRESCALER, 0x3E);
  AntennaOn();		//turn on antenna
	


	//SendByte(Read_MFRC522(MFRC522_REG_T_MODE));

}


/*
 * Function:MFRC522_Request
 * Description:Searching card, read card type
 * Input parameter:reqMode--search methods,
 *			 TagType--return card types
 *			 	0x4400 = Mifare_UltraLight
 *				0x0400 = Mifare_One(S50)
 *				0x0200 = Mifare_One(S70)
 *				0x0800 = Mifare_Pro(X)
 *				0x4403 = Mifare_DESFire
 * return:return MI_OK if successed
 */
uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
  uchar status;  
  uint backBits;			//the data bits that received

  Write_MFRC522(BitFramingReg, 0x07);		//TxLastBists = BitFramingReg[2..0]	???

  TagType[0] = reqMode;
  status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

  if ((status != MI_OK) || (backBits != 0x10))
  {    
    status = MI_ERR;
  }

  return status;
}

uchar MFRC522_Check(uint8_t* id) 
{
 uchar status;

 /* Must Clear Bit MFCrypto1On in Status2 reg in order to return to the card detect mode*/
 ClearBitMask(MFRC522_REG_STATUS2,(1<<3));

 //Find cards, return card type
 status = MFRC522_Request(0x52, id); //PICC_CMD_WUPA

 if(status == MI_OK) {
		//Card detected
		//Anti-collision, return card serial number 4 bytes
		status = MFRC522_Anticoll(id);
		//printf("\n\rCard detected [%X]",status);
	}
	return status;
}

/*
 * Function:MFRC522_ToCard
 * Description:communicate between RC522 and ISO14443
 * Input parameter:command--MF522 command bits
 *			 sendData--send data to card via rc522
 *			 sendLen--send data length		 
 *			 backData--the return data from card
 *			 backLen--the length of return data
 * return:return MI_OK if successed
 */
uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
  uchar status = MI_ERR;
  uchar irqEn = 0x00;
  uchar waitIRq = 0x00;
  uchar lastBits;
  uchar n;
  uint i;

  switch (command)
  {
  case MFRC522_PCD_AUTHENT:		//verify card password
    {
      irqEn = 0x12;
      waitIRq = 0x10;
      break;
    }
  case MFRC522_PCD_TRANSCEIVE:	//send data in the FIFO
    {
      irqEn = 0x77;
      waitIRq = 0x30;
      break;
    }
  default:
    break;
  }

  Write_MFRC522(CommIEnReg, irqEn|0x80);	//Allow interruption
  ClearBitMask(CommIrqReg, 0x80);			//Clear all the interrupt bits
  SetBitMask(FIFOLevelReg, 0x80);			//FlushBuffer=1, FIFO initilizate

  Write_MFRC522(CommandReg, MFRC522_PCD_IDLE);	//NO action;cancel current command	???

  //write data into FIFO
  for (i=0; i<sendLen; i++)
  {   
    Write_MFRC522(FIFODataReg, sendData[i]);    
  }

  //procceed it
  Write_MFRC522(CommandReg, command);
  if (command == MFRC522_PCD_TRANSCEIVE)
  {    
    SetBitMask(BitFramingReg, 0x80);		//StartSend=1,transmission of data starts  
  }   

  //waite receive data is finished
  //i = 2000;	//i should adjust according the clock, the maxium the waiting time should be 25 ms???
 	i=OS_TimeMS;
  do 
  {
    //CommIrqReg[7..0]
    //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
    n = Read_MFRC522(CommIrqReg);
		if((OS_TimeMS-i)>50)
		{
			i=0;
			break;
		}
    //i--;
  }
  while (!(n&0x01) && !(n&waitIRq));

  ClearBitMask(BitFramingReg, 0x80);			//StartSend=0

    if (i != 0)
  {    
    if(!(Read_MFRC522(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
    {
      status = MI_OK;
      if (n & irqEn & 0x01)
      {   
        status = MI_NOTAGERR;			//??   
      }

      if (command == MFRC522_PCD_TRANSCEIVE)
      {
        n = Read_MFRC522(FIFOLevelReg);
        lastBits = Read_MFRC522(ControlReg) & 0x07;
        if (lastBits)
        {   
          *backLen = (n-1)*8 + lastBits;   
        }
        else
        {   
          *backLen = n*8;   
        }

        if (n == 0)
        {   
          n = 1;    
        }
        if (n > MAX_LEN)
        {   
          n = MAX_LEN;   
        }

        //read the data from FIFO
        for (i=0; i<n; i++)
        {  
          backData[i] = Read_MFRC522(FIFODataReg);    
        }
      }
    }
    else
    {   
      status = MI_ERR;  
    }

  }

  //SetBitMask(ControlReg,0x80);           //timer stops
  //Write_MFRC522(CommandReg, PCD_IDLE); 
  return status;
}


/*
 * Function:MFRC522_Anticoll
 * Description:Prevent conflict, read the card serial number 
 * Input parameter:serNum--return the 4 bytes card serial number, the 5th byte is recheck byte
 * return:return MI_OK if successed
 */
uchar MFRC522_Anticoll(uchar *serNum)
{
  uchar status;
  uchar i;
  uchar serNumCheck=0;
  uint unLen;


  //ClearBitMask(Status2Reg, 0x08);		//TempSensclear
  //ClearBitMask(CollReg,0x80);			//ValuesAfterColl
  Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]

  serNum[0] = PICC_ANTICOLL;
  serNum[1] = 0x20;
  status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

  if (status == MI_OK)
  {
    //Verify card serial number
    for (i=0; i<4; i++)
    {   
      serNumCheck ^= serNum[i];
    }
    if (serNumCheck != serNum[i])
    {   
      status = MI_ERR;    
    }
  }

  //SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1

  return status;
} 


/*
 * Function:CalulateCRC
 * Description:Use MF522 to caculate CRC
 * Input parameter:pIndata--the CRC data need to be read,len--data length,pOutData-- the caculated result of CRC
 * return:Null
 */
void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
  uchar i, n;

  ClearBitMask(DivIrqReg, 0x04);			//CRCIrq = 0
  SetBitMask(FIFOLevelReg, 0x80);			//Clear FIFO pointer
  //Write_MFRC522(CommandReg, PCD_IDLE);

  //Write data into FIFO	
  for (i=0; i<len; i++)
  {   
    Write_MFRC522(FIFODataReg, *(pIndata+i));   
  }
  Write_MFRC522(CommandReg, MFRC522_PCD_CALCCRC);

  //waite CRC caculation to finish
  i = 0xFF;
  do 
  {
    n = Read_MFRC522(DivIrqReg);
    i--;
  }
  while ((i!=0) && !(n&0x04));			//CRCIrq = 1

  //read CRC caculation result
  pOutData[0] = Read_MFRC522(CRCResultRegL);
  pOutData[1] = Read_MFRC522(CRCResultRegM);
}


/*
 * Function:MFRC522_SelectTag
 * Description:Select card, read card storage volume
 * Input parameter:serNum--Send card serial number
 * return:return the card storage volume
 */
uchar MFRC522_SelectTag(uchar *serNum)
{
  uchar i;
  uchar status;
  uchar size;
  uint recvBits;
  uchar buffer[9]; 

  //ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0

  buffer[0] = PICC_SElECTTAG;
  buffer[1] = 0x70;
  for (i=0; i<5; i++)
  {
    buffer[i+2] = *(serNum+i);
  }
  CalulateCRC(buffer, 7, &buffer[7]);		//Fill [7:8] with 2byte CRC
  status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

  if ((status == MI_OK) && (recvBits == 0x18))
  {   
    size = buffer[0]; 
  }
  else
  {   
    size = 0;    
  }

  return size;
}


/*
 * Function:MFRC522_Auth
 * Description:verify card password
 * Input parameters:authMode--password verify mode
 0x60 = verify A passowrd key 
 0x61 = verify B passowrd key 
 BlockAddr--Block address
 Sectorkey--Block password
 serNum--Card serial number ,4 bytes
 * return:return MI_OK if successed
 */
uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
  uchar status;
  uint recvBits;
  uchar i;
  uchar buff[12]; 

  //Verify command + block address + buffer password + card SN
  buff[0] = authMode;
  buff[1] = BlockAddr;
  for (i=0; i<6; i++)
  {    
    buff[i+2] = *(Sectorkey+i);   
  }
  for (i=0; i<4; i++)
  {    
    buff[i+8] = *(serNum+i);   
  }
  status = MFRC522_ToCard(MFRC522_PCD_AUTHENT, buff, 12, buff, &recvBits);

  if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
  {   
    status = MI_ERR;   
  }

  return status;
}


/*
 * Function:MFRC522_Read
 * Description:Read data 
 * Input parameters:blockAddr--block address;recvData--the block data which are read
 * return:return MI_OK if successed
 */
uchar MFRC522_Read(uchar blockAddr, uchar *recvData)
{
  uchar status;
  uint unLen;

  recvData[0] = PICC_READ;
  recvData[1] = blockAddr;
  CalulateCRC(recvData,2, &recvData[2]);
  status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

  if ((status != MI_OK) || (unLen != 0x90))
  {
    status = MI_ERR;
  }

  return status;
}


/*
 * Function:MFRC522_Write
 * Description:write block data
 * Input parameters:blockAddr--block address;writeData--Write 16 bytes data into block
 * return:return MI_OK if successed
 */
uchar MFRC522_Write(uchar blockAddr, uchar *writeData)
{
  uchar status;
  uint recvBits;
  uchar i;
  uchar buff[18]; 

  buff[0] = PICC_WRITE;
  buff[1] = blockAddr;
  CalulateCRC(buff, 2, &buff[2]);
  status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

  if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
  {   
    status = MI_ERR;   
  }

  if (status == MI_OK)
  {
    for (i=0; i<16; i++)		//Write 16 bytes data into FIFO
    {    
      buff[i] = *(writeData+i);   
    }
    CalulateCRC(buff, 16, &buff[16]);
    status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {   
      status = MI_ERR;   
    }
  }

  return status;
}


/*
 * Function:MFRC522_Decrement
 * Description:write block data
 * Input parameters:blockAddr--block address;writeData--Write 16 bytes data into block
 * return:return MI_OK if successed
 */
uchar MFRC522_Decrement(uchar blockAddr, unsigned char *value)
{
uchar status;
uint recvBits;
uchar i;
uchar buff[18]; 

  buff[0] = PICC_DECREMENT;
  buff[1] = blockAddr;
  CalulateCRC(buff, 2, &buff[2]);
  status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

  if ((status != MI_OK))
  {   
    status = MI_ERR;   
  }

  if (status == MI_OK)
  {
    buff[0]=value[0];
    buff[1]=value[1];
    buff[2]=value[2];
    buff[3]=value[3];
    CalulateCRC(buff, 4, &buff[4]);
    status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buff, 6, buff, &recvBits);

		if(status == MI_NOTAGERR)
			status = MI_OK;  // no response after 4 byte value -
		                   // transfer command has to follow
    if ((status == MI_OK))
    {   
		
      buff[0] = PICC_TRANSFER;
      buff[1] = blockAddr;
      CalulateCRC(buff, 2, &buff[2]);
      status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
		}
	
  }

  return status;
}

/*
 * Function:MFRC522_Halt
 * Description:Command the cards into sleep mode
 * Input parameters:null
 * return:null
 */
void MFRC522_Halt(void)
{
  uchar status;
  uint unLen;
  uchar buff[4]; 

  buff[0] = PICC_HALT;
  buff[1] = 0;
  CalulateCRC(buff, 2, &buff[2]);

  status = MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}

void MFRC522_WakeUp(void){
	uint unLen;
	uchar buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	CalulateCRC(buff, 2, &buff[2]);

	MFRC522_ToCard(MFRC522_PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

