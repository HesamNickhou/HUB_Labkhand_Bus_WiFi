
#include <rdlib/types/RdLib.h>
#include <includes.h>
#include <config.h>

#ifdef CLRC_CHIP

#ifdef Simorgh50N
phhalHw_Rc632_DataParams_t      halReader;
#endif
#ifdef Torgheh
phhalHw_Rc523_DataParams_t      halReader;
#endif

phpalI14443p3a_DataParams_t  palI14443p3a;
phpalI14443p4a_DataParams_t  palI14443p4a;
phpalI14443p4_DataParams_t   palI14443p4;
phpalMifare_DataParams_t     palMifare;
phalMfc_DataParams_t         alMifareC;
uint8_t bHalBufferReader[256];
uint8_t bBufferReader[0x60];
uint8_t **ppRxBuffer = (void *)&bBufferReader[0];
uint16_t bRxLength;

extern unsigned char spi2(unsigned char data);
///////////////////////////////////////////////////////////////////////////////
//          G E N E R I C    R E A D
///////////////////////////////////////////////////////////////////////////////
void WriteRC(unsigned char Address, unsigned char value)
{
unsigned char Byte;

   R_CS=0;
   Byte=(Address<<1)&0x7E;
   spi2(Byte);
   spi2(value);
   R_CS=1;
}

///////////////////////////////////////////////////////////////////////////////
//          G E N E R I C    R E A D
///////////////////////////////////////////////////////////////////////////////
unsigned char ReadRC(unsigned char Address)
{
unsigned char Byte;

   R_CS=0;
   Byte=spi2((Address<<1)|0x80);   
   Byte=spi2(0);   
   R_CS=1;
   return Byte;
   
}  

//===========================================================================
signed char PcdReset(void)
{
 char status = 0;
 unsigned int i=0;

 #ifdef Simorgh50N
 R_RST=1;
 GUI_Delay(500);
 R_RST=0;
 GUI_Delay(500);

 // while reset sequence in progress
 while ((ReadRC(PHHAL_HW_RC632_REG_COMMAND) & 0x3F))
   {
     if (++i>7000) 
       return 1;
   }

 WriteRC(0x00,0); //denay PageSelect method

 if (ReadRC(PHHAL_HW_RC632_REG_COMMAND) != 0x00)
   {          
    status = 250;
   }
	  
 #endif

 

   return status;
}

//===========================================================================
unsigned char InitializeRdLib(void)
{
unsigned char status=0; 
uint8_t pAtqa[2];
	
  PcdReset();
	
	#ifdef Simorgh50N

	status = phhalHw_Rc632_Init(
        &halReader,
        sizeof(phhalHw_Rc632_DataParams_t),
        NULL,
        bHalBufferReader,
        sizeof(bHalBufferReader),
        bHalBufferReader,
        sizeof(bHalBufferReader));
  #endif	
				
	#ifdef Torgheh
	status = phhalHw_Rc523_Init(
        &halReader,
        sizeof(phhalHw_Rc523_DataParams_t),
        NULL,
				NULL,
        bHalBufferReader,
        sizeof(bHalBufferReader),
        bHalBufferReader,
        sizeof(bHalBufferReader));
  #endif				

  status = phpalI14443p3a_Init(&palI14443p3a, sizeof(palI14443p3a), &halReader);
  status = phpalI14443p4a_Init(&palI14443p4a, sizeof(palI14443p4a),&halReader);
  status = phpalI14443p4_Init(&palI14443p4, sizeof(palI14443p4), &halReader);
  status = phpalMifare_Init(&palMifare, sizeof(palMifare),&halReader, &palI14443p4);
    
	phhalHw_FieldReset(&halReader);		
				
	phhalHw_ApplyProtocolSettings(&halReader, PHHAL_HW_CARDTYPE_ISO14443A);

  status = phpalI14443p3a_RequestA(&palI14443p3a, pAtqa);
	
  status = phhalHw_FieldReset(&halReader);
				
	return status;
	
}

//===========================================================================
unsigned char SendAPDU(unsigned char *Apdu, unsigned char ApduLen, unsigned char *Result, unsigned short *ResultLen)
{
unsigned char status;
unsigned int i;	
	
  memcpy(&bBufferReader[3], Apdu, ApduLen);
  status=phpalI14443p4_Exchange(&palI14443p4, PH_EXCHANGE_DEFAULT, &bBufferReader[3], ApduLen,
                        (void *)&bBufferReader[0], ResultLen);
	
	for (i=0; i<*ResultLen; i++)
	  Result[i] =ppRxBuffer[0][i];
	return status;
}


//========================================================================
unsigned char phpalI14443p3a_MyActivateCard(
    phpalI14443p3a_DataParams_t * pDataParams,
    uint8_t * pUidIn,
    uint8_t bLenUidIn,
    uint8_t * pUidOut,
    uint8_t * pLenUidOut
    )
{
    phStatus_t   status;
    phStatus_t   statusTmp;
    uint8_t      bCascadeLevel;
    uint8_t      bCascadeIndex;
    uint8_t      bUidIndex;
    uint8_t      bNvbUid;
    uint8_t      bAtqa[2];
    uint8_t      bUid[4];
    uint8_t      bSak;
    uint8_t      bMoreCardsAvailable;

    /* Clear Uid */
    memset(bUid, 0x00, 4);  /* PRQA S 3200 */

    /* Clear Uid lengths */
    bUidIndex = 0;
    *pLenUidOut = 0;

    /* Clear the more cards available flag */
    bMoreCardsAvailable = PH_OFF;

    /* UidIn is not given -> ReqA */
    if (bLenUidIn == 0)
    {
        status = phpalI14443p3a_RequestA(pDataParams, bAtqa);
    }
    /* UidIn is given -> WupA */
    else
    {
        status = phpalI14443p3a_WakeUpA(pDataParams, bAtqa);
    }

    /* Collision error may happen */
    if ((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
    {
        /* Set the more cards available flag */
        bMoreCardsAvailable = PH_ON;
    }
    /* Status check */
    else
    {
      if (PH_ERR_SUCCESS != status) 
		    return 0;
    }

    /* Go through all cascade levels */
    for (bCascadeIndex = 0; bCascadeIndex < 3; bCascadeIndex++)
    {
        /* Set cascade level tags */
        switch (bCascadeIndex)
        {
        case 0:
            bCascadeLevel = PHPAL_I14443P3A_CASCADE_LEVEL_1;
            break;
        case 1:
            bCascadeLevel = PHPAL_I14443P3A_CASCADE_LEVEL_2;
            break;
        case 2:
            bCascadeLevel = PHPAL_I14443P3A_CASCADE_LEVEL_3;
            break;
        }

        /* Copy know Uid part if neccessary */
        if (bLenUidIn == (bUidIndex + 4))
        {
            /* Copy whole Uid if this is the only cascade level */
            memcpy(&bUid[0], &pUidIn[bUidIndex], 4);  /* PRQA S 3200 */

            /* [Incrementing Uid index would have no effect] */

            /* All bits except BCC are valid */
            bNvbUid = 0x40;
        }
        else if (bLenUidIn > (bUidIndex + 4))
        {
            /* Prepend cascade tag if we expect more cascade levels*/
            bUid[0] = 0x88U; //PHPAL_I14443P3A_CASCADE_TAG;

            /* Only three more uid bytes are valid for this level */
            memcpy(&bUid[1], &pUidIn[bUidIndex], 3);  /* PRQA S 3200 */

            /* Increment Uid index */
            bUidIndex += 3;

            /* All bits except BCC are valid */
            bNvbUid = 0x40;
        }
        else
        {
            /* No Uid given */
            bNvbUid = 0;
        }

        /* Anticollision Loop */
        while (bNvbUid != 0x40)
        {
            /* Perform single anticollision command */
            status = phpalI14443p3a_Anticollision(pDataParams, bCascadeLevel, bUid, bNvbUid, bUid, &bNvbUid);

            /* Collision error occured, add one (zero-)bit to Uid */
            if ((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
            {
                /* Set the more cards available flag */
                bMoreCardsAvailable = PH_ON;

                /* Increment NvbUid by one bit */
                if ((bNvbUid & 0x07) < 7)
                {
                    bNvbUid++;
                }
                /* This is more difficult if we have to increment the bytecount */
                else
                {
                    bNvbUid = (uint8_t)((((bNvbUid & 0xF0) >> 4) + 1) << 4);
                }
            }
            else
            {
              if (PH_ERR_SUCCESS != status) 
		            return 0;
            }
        }

        /* Select cascade level */
        statusTmp=phpalI14443p3a_Select(pDataParams, bCascadeLevel, bUid, &bSak);
        if (PH_ERR_SUCCESS != statusTmp) 
	  	    return 0;

        /* Cascade Bit is cleared -> no further cascade levels */
        if (!(bSak & 0x04))
        {
            break;
        }
    }

    if (PH_ERR_SUCCESS != status) 
 	    return 0;
    /* Return UID */
    memcpy(pUidOut, pDataParams->abUid, pDataParams->bUidLength);  /* PRQA S 3200 */
    *pLenUidOut = pDataParams->bUidLength;
		
	  if ((bSak & 0x20) && (bAtqa[1] & 0x03))
  		return CARD_DESFIRE;
    else if ((bSak & 0x20) && (bAtqa[1] == 0))
		  return CARD_MIFAREPLUS;
    else if (0x08 == (bSak & 0x08))
		  return CARD_MIFARECLASSIC;
    else
		  return CARD_MIFAREUL;

}

//========================================================================
unsigned char ISO14443_SingleTagSelect2(unsigned char *snr)
{
unsigned char status,str[10];	
uint8_t pUidOut[10], bLenUidOut;
uint8_t pAtqa[2];
	
  status = phpalI14443p3a_MyActivateCard(&palI14443p3a, NULL, 0, snr, &bLenUidOut);

  return status;
	
  
	
}
/*
//========================================================================
unsigned char ISO14443_LoginE2(unsigned char Sector, unsigned char Key)
{
}

//========================================================================
unsigned char ISO14443_Login(unsigned char Sector, unsigned char KeyType, unsigned char *Key)
{
}

//========================================================================
unsigned char ISO14443_ReadBlock(unsigned char Block, unsigned char *buf)
{
}

//========================================================================
unsigned char ISO14443_WriteBlock(unsigned char Block, unsigned char* data)
{
}

//========================================================================
unsigned char ISO14443_WriteValue(unsigned char Block,  signed long int Val)
{
}

//========================================================================
unsigned char ISO14443_ReadValueBlock(unsigned char Block, signed long int *Val)
{
}

//========================================================================
unsigned char ISO14443_InternalReadValueBlock(unsigned char Block, unsigned char* value)
{
}

//========================================================================
unsigned char MFRC531_WriteMasterKey(unsigned char Block, unsigned char* data)
{
}
*/
#endif