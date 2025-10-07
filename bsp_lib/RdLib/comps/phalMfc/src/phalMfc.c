/*
*         Copyright (c), NXP Semiconductors Gratkorn / Austria
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Software MIFARE(R) Component of Reader Library Framework.
* $Author: santosh.d.araballi $
* $Revision: 1.33 $
* $Date: Fri Apr  1 15:35:13 2011 $
*
* History:
*  CHu: Generated 31. July 2009
*
*/
#include <config.h>

#if (ReaderType==NewRoutine)

#include <rdlib/types/RdLib.h>

#include "phalMfc.h"
#include "phalMfc_Int.h"

phStatus_t phalMfc_Init(
                           phalMfc_DataParams_t * pDataParams,
                           uint16_t wSizeOfDataParams,
                           void * pPalMifareDataParams,
                           void * pKeyStoreDataParams
                           )
{
    if (sizeof(phalMfc_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFC);
    }

    /* init private data */
    pDataParams->wId                    = PH_COMP_AL_MFC | PHAL_MFC_SW_ID;
    pDataParams->pPalMifareDataParams   = pPalMifareDataParams;
    pDataParams->pKeyStoreDataParams    = pKeyStoreDataParams;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFC);
}

phStatus_t phalMfc_Authenticate(
                                   phalMfc_DataParams_t * pDataParams,
                                   uint8_t bBlockNo,
                                   uint8_t bKeyType,
                                   uint16_t wKeyNo,
                                   uint16_t wKeyVersion,
                                   uint8_t * pUid,
                                   uint8_t bUidLength
                                   )
{
    phStatus_t   statusTmp;
    uint8_t      aKey[PHHAL_HW_MFC_KEY_LENGTH * 2];
    uint8_t *    pKey;
    uint16_t     bKeystoreKeyType;

    /* check if software key store is available. */
    if (pDataParams->pKeyStoreDataParams == NULL)
    {
        /* There is no software keystore available. */
        return phpalMifare_MfcAuthenticateKeyNo(
            pDataParams->pPalMifareDataParams,
            bBlockNo,
            bKeyType,
            wKeyNo,
            wKeyVersion,
            &pUid[bUidLength - 4]);
    }
    else
    {
        /* Software key store found. */

        /* Bail out if we haven't got a keystore */
        if (pDataParams->pKeyStoreDataParams == NULL)
        {
            return PH_ADD_COMPCODE(PH_ERR_KEY, PH_COMP_HAL);
        }

        /* retrieve KeyA & KeyB from keystore */
        PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
            pDataParams->pKeyStoreDataParams,
            wKeyNo,
            wKeyVersion,
            sizeof(aKey),
            aKey,
            &bKeystoreKeyType));

        /* check key type */
        if (bKeystoreKeyType != PH_KEYSTORE_KEY_TYPE_MIFARE)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* Evaluate which key to use */
        if ((bKeyType & 0x7F) == PHHAL_HW_MFC_KEYA)
        {
            /* Use KeyA */
            pKey = aKey;
        }
        else if ((bKeyType & 0x7F) == PHHAL_HW_MFC_KEYB)
        {
            /* Use KeyB */
            pKey = &aKey[PHHAL_HW_MFC_KEY_LENGTH];
        }
        else
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        return phpalMifare_MfcAuthenticate(
            pDataParams->pPalMifareDataParams,
            bBlockNo,
            bKeyType,
            pKey,
            &pUid[bUidLength - 4]);
    }
}

phStatus_t phalMfc_Read(
                           phalMfc_DataParams_t * pDataParams,
                           uint8_t bBlockNo,
                           uint8_t * pBlockData
                           )
{
    phStatus_t   statusTmp;
    uint8_t      aCommand[2];
    uint8_t *    pRxBuffer;
    uint16_t     wRxLength;

    /* build command frame */
    aCommand[0] = PHAL_MFC_CMD_READ;
    aCommand[1] = bBlockNo;

    /* transmit the command frame */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCommand,
        2,
        &pRxBuffer,
        &wRxLength
        ));

    /* check received length */
    if (wRxLength != PHAL_MFC_DATA_BLOCK_LENGTH)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFC);
    }

    /* copy received data block */
    memcpy(pBlockData, pRxBuffer, wRxLength);  /* PRQA S 3200 */

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFC);
}

phStatus_t phalMfc_ReadValue(
                                phalMfc_DataParams_t * pDataParams,
                                uint8_t bBlockNo,
                                uint8_t * pValue,
                                uint8_t * pAddrData
                                )
{
    phStatus_t   statusTmp;
    uint8_t      bBuffer[PHAL_MFC_DATA_BLOCK_LENGTH];

    /* perform read operation */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Read(pDataParams, bBlockNo, bBuffer));

    /* check format of value block */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Int_CheckValueBlockFormat(bBuffer));

    /* return received contents */
    *pAddrData = bBuffer[12];
    memcpy(pValue, bBuffer, PHAL_MFC_VALUE_BLOCK_LENGTH);  /* PRQA S 3200 */

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFC);
}

phStatus_t phalMfc_Write(
                            phalMfc_DataParams_t * pDataParams,
                            uint8_t bBlockNo,
                            uint8_t * pBlockData
                            )
{
    phStatus_t   statusTmp;
    uint8_t      aCommand[2];
    uint8_t *    pRxBuffer;
    uint16_t     wRxLength;

    /* build command frame */
    aCommand[0] = PHAL_MFC_CMD_WRITE;
    aCommand[1] = bBlockNo;

    /* transmit the command frame (first part) */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCommand,
        2,
        &pRxBuffer,
        &wRxLength
        ));

    /* transmit the data (second part) */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        pBlockData,
        PHAL_MFC_DATA_BLOCK_LENGTH,
        &pRxBuffer,
        &wRxLength
        ));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFC);
}

phStatus_t phalMfc_WriteValue(
                                 phalMfc_DataParams_t * pDataParams,
                                 uint8_t bBlockNo,
                                 uint8_t * pValue,
                                 uint8_t bAddrData
                                 )
{
    phStatus_t   statusTmp;
    uint8_t      bBlockData[PHAL_MFC_DATA_BLOCK_LENGTH];

    /* build a valid value block */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Int_CreateValueBlock(pValue, bAddrData,bBlockData ));

    /* perform the write operation */
    return phalMfc_Write(pDataParams, bBlockNo, bBlockData);
}

phStatus_t phalMfc_Increment(
                                phalMfc_DataParams_t * pDataParams,
                                uint8_t bBlockNo,
                                uint8_t * pValue
                                )
{
    return phalMfc_Int_Value(pDataParams->pPalMifareDataParams, PHAL_MFC_CMD_INCREMENT, bBlockNo, pValue);
}

phStatus_t phalMfc_Decrement(
                                phalMfc_DataParams_t * pDataParams,
                                uint8_t bBlockNo,
                                uint8_t * pValue
                                )
{
    return phalMfc_Int_Value(pDataParams->pPalMifareDataParams, PHAL_MFC_CMD_DECREMENT, bBlockNo, pValue);
}

phStatus_t phalMfc_Transfer(
                               phalMfc_DataParams_t * pDataParams,
                               uint8_t bBlockNo
                               )
{
    /* transmit the command frame */
    return phalMfc_Int_Transfer(
        pDataParams->pPalMifareDataParams,
        bBlockNo
        );
}

phStatus_t phalMfc_Restore(
                              phalMfc_DataParams_t * pDataParams,
                              uint8_t bBlockNo
                              )
{
    uint8_t  bBuffer[PHAL_MFC_VALUE_BLOCK_LENGTH];

    /* restore needs four dummy bytes */
    memset(bBuffer, 0x00, PHAL_MFC_VALUE_BLOCK_LENGTH);  /* PRQA S 3200 */

    return phalMfc_Int_Value(pDataParams->pPalMifareDataParams, PHAL_MFC_CMD_RESTORE, bBlockNo, bBuffer);
}

phStatus_t phalMfc_IncrementTransfer(
                                        phalMfc_DataParams_t * pDataParams,
                                        uint8_t bSrcBlockNo,
                                        uint8_t bDstBlockNo,
                                        uint8_t * pValue
                                        )
{
    phStatus_t  statusTmp;
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Increment(pDataParams, bSrcBlockNo, pValue));
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Transfer(pDataParams, bDstBlockNo));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFC);
}

phStatus_t phalMfc_DecrementTransfer(
                                        phalMfc_DataParams_t * pDataParams,
                                        uint8_t bSrcBlockNo,
                                        uint8_t bDstBlockNo,
                                        uint8_t * pValue
                                        )
{
    phStatus_t  statusTmp;
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Decrement(pDataParams, bSrcBlockNo, pValue));
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Transfer(pDataParams, bDstBlockNo));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFC);
}

phStatus_t phalMfc_RestoreTransfer(
                                      phalMfc_DataParams_t * pDataParams,
                                      uint8_t bSrcBlockNo,
                                      uint8_t bDstBlockNo
                                      )
{
    phStatus_t  statusTmp;
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Restore(pDataParams, bSrcBlockNo));
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfc_Transfer(pDataParams, bDstBlockNo));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFC);
}

phStatus_t phalMfc_PersonalizeUid(
                                     phalMfc_DataParams_t * pDataParams,
                                     uint8_t bUidType
                                     )
{
    uint8_t      aCommand[2];
    uint8_t      * pRecv;
    uint16_t     wRxlen;

    /* build command frame */
    aCommand[0] = PHAL_MFC_CMD_PERSOUID;
    aCommand[1] = bUidType;

    /* transmit the command frame */
    return phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCommand,
        2,
        &pRecv,
        &wRxlen
        );
}

#endif
