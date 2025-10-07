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
* Generic HAL Component of Reader Library Framework.
* $Author: santosh.araballi $
* $Revision: 1.69 $
* $Date: Fri Nov  2 07:25:09 2012 $
*
* History:
*  CHu: Generated 19. May 2009
*
*/
#include <config.h>

#if (ReaderType==NewRoutine)

#include <rdlib/types/RdLib.h>
#ifdef Simorgh50N
#include "Rc632/phhalHw_Rc632.h"
#endif
#ifdef Torgheh
#include "Rc523/phhalHw_Rc523.h"
#endif

phStatus_t phhalHw_Exchange(
                            void * pDataParams,
                            uint16_t wOption,
                            uint8_t * pTxBuffer,
                            uint16_t wTxLength,
                            uint8_t ** ppRxBuffer,
                            uint16_t * pRxLength
                            )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_Exchange((phhalHw_Rc632_DataParams_t *)pDataParams, wOption, pTxBuffer, wTxLength, ppRxBuffer, pRxLength);
    #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_Exchange((phhalHw_Rc523_DataParams_t *)pDataParams, wOption, pTxBuffer, wTxLength, ppRxBuffer, pRxLength);
    #endif

    return status;
}

phStatus_t phhalHw_WriteRegister(
                                 void * pDataParams,
                                 uint8_t bAddress,
                                 uint8_t bValue
                                 )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    phhalHw_Rc632_WriteRegister(bAddress, bValue);
	  return PH_ERR_SUCCESS;
    #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_WriteRegister((phhalHw_Rc523_DataParams_t *)pDataParams, bAddress, bValue);
    #endif
	
    return status;
}

phStatus_t phhalHw_ReadRegister(
                                void * pDataParams,
                                uint8_t bAddress,
                                uint8_t * pValue
                                )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    phhalHw_Rc632_ReadRegister(bAddress, pValue);
	  return PH_ERR_SUCCESS; 
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_ReadRegister((phhalHw_Rc523_DataParams_t *)pDataParams, bAddress, pValue);
	  #endif

    return status;
}

phStatus_t phhalHw_ExecuteCmd(
                              void * pDataParams,
                              uint8_t bCmd,
                              uint16_t wOption,
                              uint8_t bIrq0WaitFor,
                              uint8_t bIrq1WaitFor,
                              uint8_t * pTxBuffer,
                              uint16_t wTxLength,
                              uint16_t wRxBufferSize,
                              uint8_t * pRxBuffer,
                              uint16_t * pRxLength
                              )
{
    phStatus_t  status;

#ifdef NXPBUILD__PHHAL_HW_RC663
        status = phhalHw_Rc663_ExecuteCmd(
            (phhalHw_Rc663_DataParams_t *)pDataParams,
            bCmd,
            wOption,
            bIrq0WaitFor,
            bIrq1WaitFor,
            pTxBuffer,
            wTxLength,
            wRxBufferSize,
            pRxBuffer,
            pRxLength
            );
#endif /* NXPBUILD__PHHAL_HW_RC663 */

    return status;
}

phStatus_t phhalHw_ApplyProtocolSettings(
    void * pDataParams,
    uint8_t bCardType
    )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_ApplyProtocolSettings((phhalHw_Rc632_DataParams_t *)pDataParams, bCardType);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_ApplyProtocolSettings((phhalHw_Rc523_DataParams_t *)pDataParams, bCardType);
	  #endif

    return status;
}

phStatus_t phhalHw_SetConfig(
                             void * pDataParams,
                             uint16_t wConfig,
                             uint16_t wValue
                             )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_SetConfig((phhalHw_Rc632_DataParams_t *)pDataParams, wConfig, wValue);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_SetConfig((phhalHw_Rc523_DataParams_t *)pDataParams, wConfig, wValue);
	  #endif

    return status;
}

phStatus_t phhalHw_MfcAuthenticateKeyNo(
                                        void * pDataParams,
                                        uint8_t bBlockNo,
                                        uint8_t bKeyType,
                                        uint16_t wKeyNo,
                                        uint16_t wKeyVersion,
                                        uint8_t * pUid
                                        )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_MfcAuthenticateKeyNo(
            (phhalHw_Rc632_DataParams_t *)pDataParams,
            bBlockNo,
            bKeyType,
            wKeyNo,
            wKeyVersion,
            pUid);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_MfcAuthenticateKeyNo(
            (phhalHw_Rc523_DataParams_t *)pDataParams,
            bBlockNo,
            bKeyType,
            wKeyNo,
            wKeyVersion,
            pUid);
	  #endif

    return status;
}

phStatus_t phhalHw_MfcAuthenticate(
                                   void * pDataParams,
                                   uint8_t bBlockNo,
                                   uint8_t bKeyType,
                                   uint8_t * pKey,
                                   uint8_t * pUid
                                   )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_MfcAuthenticate(
            (phhalHw_Rc632_DataParams_t *)pDataParams,
            bBlockNo,
            bKeyType,
            pKey,
            pUid);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_MfcAuthenticate(
            (phhalHw_Rc523_DataParams_t *)pDataParams,
            bBlockNo,
            bKeyType,
            pKey,
            pUid);
	  #endif

    return status;
}

phStatus_t phhalHw_GetConfig(
                             void * pDataParams,
                             uint16_t wConfig,
                             uint16_t * pValue
                             )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_GetConfig((phhalHw_Rc632_DataParams_t *)pDataParams, wConfig, pValue);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_GetConfig((phhalHw_Rc523_DataParams_t *)pDataParams, wConfig, pValue);
	  #endif

    return status;
}

phStatus_t phhalHw_FieldOn(
                           void * pDataParams
                           )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_FieldOn((phhalHw_Rc632_DataParams_t *)pDataParams);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_FieldOn((phhalHw_Rc523_DataParams_t *)pDataParams);
	  #endif

    return status;
}

phStatus_t phhalHw_FieldOff(
                            void * pDataParams
                            )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_FieldOff((phhalHw_Rc632_DataParams_t *)pDataParams);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_FieldOff((phhalHw_Rc523_DataParams_t *)pDataParams);
	  #endif

    return status;
}

phStatus_t phhalHw_FieldReset(
                              void * pDataParams
                              )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_FieldReset((phhalHw_Rc632_DataParams_t *)pDataParams);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_FieldReset((phhalHw_Rc523_DataParams_t *)pDataParams);
	  #endif

    return status;
}

phStatus_t phhalHw_Wait(
                        void * pDataParams,
                        uint8_t bUnit,
                        uint16_t wTimeout
                        )
{
    phStatus_t  status;

    #ifdef Simorgh50N
    status = phhalHw_Rc632_Wait((phhalHw_Rc632_DataParams_t *)pDataParams, bUnit, wTimeout);
	  #endif
    #ifdef Torgheh
    status = phhalHw_Rc523_Wait((phhalHw_Rc523_DataParams_t *)pDataParams, bUnit, wTimeout);
	  #endif

    return status;
}

#endif