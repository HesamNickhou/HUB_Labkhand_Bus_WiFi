

#ifndef PH_RDLIB_H
#define PH_RDLIB_H

#define CARD_DESFIRE         1
#define CARD_MIFARECLASSIC   2
#define CARD_MIFAREPLUS      3
#define CARD_MIFAREUL        4


#include <config.h>

#ifdef CLRC_CHIP

#include <RdLib/types/ph_Status.h>
#include <RdLib/types/ph_NxpBuild.h>
#include <RdLib/types/ph_TypeDefs.h>
#include <RdLib/intfs/phKeyStore.h>
#include <RdLib/intfs/phTools.h>

#include <RdLib/intfs/phbalReg.h>

#include <RdLib/intfs/phhalHw.h>

#ifdef Simorgh50N
#include <RdLib/intfs/phhalHw_Rc632_Reg.h>                                        
#include <RdLib/intfs/phhalHw_Rc632_Cmd.h>                                        
#endif
#ifdef Torgheh
#include <RdLib/intfs/phhalHw_Rc523_Reg.h>                                        
#include <RdLib/intfs/phhalHw_Rc523_Cmd.h>                                        
#endif

#include <RdLib/intfs/phpalI14443p3a.h>
#include <RdLib/intfs/phpalI14443p3b.h>
#include <RdLib/intfs/phpalI14443p4.h>
#include <RdLib/intfs/phpalI14443p4a.h>
#include <RdLib/intfs/phpalMifare.h>
#include <RdLib/intfs/phalMfc.h>
#include <RdLib/intfs/phalMfdf.h>



#ifdef Simorgh50N
extern phhalHw_Rc632_DataParams_t      halReader;
#endif
#ifdef Torgheh
extern phhalHw_Rc523_DataParams_t      halReader;
#endif
extern phpalI14443p3a_DataParams_t  palI14443p3a;
extern phpalI14443p4a_DataParams_t  palI14443p4a;
extern phpalI14443p4_DataParams_t   palI14443p4;
extern phpalMifare_DataParams_t     palMifare;
extern phalMfc_DataParams_t         alMifareC;
extern phalMfdf_DataParams_t        alMfdf;                  /**< Software AL-MIFARE-DESFIRE parameter structure */


extern unsigned char InitializeRdLib(void);
extern unsigned char SendAPDU(unsigned char *Apdu, unsigned char ApduLen, unsigned char *Result, unsigned short *ResultLen);

extern unsigned char ISO14443_SingleTagSelect(unsigned char *snr);
extern unsigned char ISO14443_LoginE2(unsigned char Sector, unsigned char Key);
extern unsigned char ISO14443_Login(unsigned char Sector, unsigned char KeyType, unsigned char *Key);
extern unsigned char ISO14443_ReadBlock(unsigned char Block, unsigned char *buf);
extern unsigned char ISO14443_WriteBlock(unsigned char Block, unsigned char* data);
extern unsigned char ISO14443_WriteValue(unsigned char Block,  signed long int Val);
extern unsigned char ISO14443_ReadValueBlock(unsigned char Block, signed long int *Val);
extern unsigned char ISO14443_InternalReadValueBlock(unsigned char Block, unsigned char* value);
extern unsigned char MFRC531_WriteMasterKey(unsigned char Block, unsigned char* data);

#else
  #include "mfrc/mfrc531.h"
  #include "mfrc/iso14443ab.h"

#endif

#endif /* PH_RDLIB_H */
