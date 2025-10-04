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
* Software MIFARE(R) Application Component of Reader Library Framework.
* $Author: santosh.d.araballi $
* $Revision: 1.17 $
* $Date: Fri Apr  1 15:35:13 2011 $
*
* History:
*  CHu: Generated 31. July 2009
*
*/

#ifndef PHALMFC_SW_H
#define PHALMFC_SW_H

#include <rdlib/types/RdLib.h>

phStatus_t phalMfc_Authenticate(
                                   phalMfc_DataParams_t * pDataParams,
                                   uint8_t bBlockNo,
                                   uint8_t bKeyType,
                                   uint16_t wKeyNumber,
                                   uint16_t wKeyVersion,
                                   uint8_t * pUid,
                                   uint8_t bUidLength
                                   );

phStatus_t phalMfc_Read(
                           phalMfc_DataParams_t * pDataParams,
                           uint8_t bBlockNo,
                           uint8_t * pBlockData
                           );

phStatus_t phalMfc_ReadValue(
                                phalMfc_DataParams_t * pDataParams,
                                uint8_t bBlockNo,
                                uint8_t * pValue,
                                uint8_t * pAddrData
                                );

phStatus_t phalMfc_Write(
                            phalMfc_DataParams_t * pDataParams,
                            uint8_t bBlockNo,
                            uint8_t * pBlockData
                            );

phStatus_t phalMfc_WriteValue(
                                 phalMfc_DataParams_t * pDataParams,
                                 uint8_t bBlockNo,
                                 uint8_t * pValue,
                                 uint8_t bAddrData
                                 );

phStatus_t phalMfc_Increment(
                                phalMfc_DataParams_t * pDataParams,
                                uint8_t bBlockNo,
                                uint8_t * pValue
                                );

phStatus_t phalMfc_Decrement(
                                phalMfc_DataParams_t * pDataParams,
                                uint8_t bBlockNo,
                                uint8_t * pValue
                                );

phStatus_t phalMfc_Transfer(
                               phalMfc_DataParams_t * pDataParams,
                               uint8_t bBlockNo
                               );

phStatus_t phalMfc_Restore(
                              phalMfc_DataParams_t * pDataParams,
                              uint8_t bBlockNo
                              );

phStatus_t phalMfc_IncrementTransfer(
                                        phalMfc_DataParams_t * pDataParams,
                                        uint8_t bSrcBlockNo,
                                        uint8_t bDstBlockNo,
                                        uint8_t * pValue
                                        );

phStatus_t phalMfc_DecrementTransfer(
                                        phalMfc_DataParams_t * pDataParams,
                                        uint8_t bSrcBlockNo,
                                        uint8_t bDstBlockNo,
                                        uint8_t * pValue
                                        );

phStatus_t phalMfc_RestoreTransfer(
                                      phalMfc_DataParams_t * pDataParams,
                                      uint8_t bSrcBlockNo,
                                      uint8_t bDstBlockNo
                                      );

phStatus_t phalMfc_PersonalizeUid(
                                     phalMfc_DataParams_t * pDataParams,
                                     uint8_t bUidType
                                     );

#endif /* PHALMFC_SW_H */
