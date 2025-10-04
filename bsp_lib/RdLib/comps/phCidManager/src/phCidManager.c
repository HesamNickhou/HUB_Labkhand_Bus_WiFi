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
* Cid Manager Component of Reader Library Framework.
* $Author: jayaprakash.desai $
* $Revision: 1.15 $
* $Date: Tue Oct 23 15:51:11 2012 $
*
* History:
*  CHu: Generated 27. July 2009
*
*/

#include <ph_Status.h>
#include <phCidManager.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_CIDMANAGER_SW
#include "Sw/phCidManager_Sw.h"
#endif /* NXPBUILD__PH_CIDMANAGER_SW */

#ifdef NXPBUILD__PH_CIDMANAGER_RD710
#include "Rd710/phCidManager_Rd710.h"
#endif /* NXPBUILD__PH_CIDMANAGER_RD710 */

#ifdef NXPBUILD__PH_CIDMANAGER

phStatus_t phCidManager_GetFreeCid(
                                   void * pDataParams,
                                   uint8_t * pCid
                                   )
{
   phStatus_t PH_MEMLOC_REM status;

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_CIDMANAGER)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CIDMANAGER);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CIDMANAGER_SW
    case PH_CIDMANAGER_SW_ID:
        status = phCidManager_Sw_GetFreeCid((phCidManager_Sw_DataParams_t *)pDataParams, pCid);
        break;
#endif /* NXPBUILD__PH_CIDMANAGER_SW */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CIDMANAGER);
        break;
    }

    return status;
}

phStatus_t phCidManager_FreeCid(
                                void * pDataParams,
                                uint8_t bCid
                                )
{
   phStatus_t PH_MEMLOC_REM status;

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_CIDMANAGER)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CIDMANAGER);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CIDMANAGER_SW
    case PH_CIDMANAGER_SW_ID:
        status = phCidManager_Sw_FreeCid((phCidManager_Sw_DataParams_t *)pDataParams, bCid);
        break;
#endif /* NXPBUILD__PH_CIDMANAGER_SW */


    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CIDMANAGER);
        break;
    }
    return status;
}

#endif /* NXPBUILD__PH_CIDMANAGER */
