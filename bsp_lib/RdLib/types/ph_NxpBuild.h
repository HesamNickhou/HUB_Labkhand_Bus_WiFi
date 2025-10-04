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
* Build System Definitions for Reader Library Framework.
* $Author: jayaprakash.desai $
* $Revision: 1.79 $
* $Date: Tue Oct 23 15:50:41 2012 $
*
* History:
*  MK: Generated 15. October 2009
*
*/

#ifndef PH_NXPBUILD_H
#define PH_NXPBUILD_H

/** \defgroup ph_NxpBuild NXP Build
* \brief Controls build behaviour of components.
* @{
*/

/* NXPBUILD_DELETE: included code lines should be always removed from code */

/* NXP Build defines                            */
/* use #define to include components            */
/* comment out #define to exclude components    */

/* DEBUG build mode */


/* BAL components */

#define NXPBUILD__PHBAL_REG_STUB                /**< Stub BAL definition */

#if defined (NXPBUILD__PHBAL_REG_STUB)          || \
    defined (NXPBUILD__PHBAL_REG_SERIALWIN)     || \
    defined (NXPBUILD__PHBAL_REG_PCSCWIN)       || \
    defined (NXPBUILD__PHBAL_REG_RD70XUSBWIN)   || \
    defined (NXPBUILD__PHBAL_REG_PIPELIN)       || \
    defined (NXPBUILD__PHBAL_REG_SOCKETWIN)
    #define NXPBUILD__PHBAL_REG                 /**< Generic BAL definition */
#endif

/* HAL components */

#define NXPBUILD__PHHAL_HW_RC632                /**< Rc632 HAL definition */

#if defined (NXPBUILD__PHHAL_HW_RC523)        || \
    defined (NXPBUILD__PHHAL_HW_RD70X)        || \
    defined (NXPBUILD__PHHAL_HW_RDCARDSIM)    || \
    defined (NXPBUILD__PHHAL_HW_CALLBACK)     || \
    defined (NXPBUILD__PHHAL_HW_RC663)        || \
    defined (NXPBUILD__PHHAL_HW_RC632)        || \
    defined (NXPBUILD__PHHAL_HW_RD710)
    #define NXPBUILD__PHHAL_HW                  /**< Generic HAL definition */
#endif

/* PAL ISO 14443-3A components */

#define NXPBUILD__PHPAL_I14443P3A_SW            /**< Software PAL ISO 14443-3A definition */

#if defined (NXPBUILD__PHPAL_I14443P3A_SW)          || \
    defined (NXPBUILD__PHPAL_I14443P3A_RD70X)       || \
    defined (NXPBUILD__PHPAL_I14443P3A_RD710)
    #define NXPBUILD__PHPAL_I14443P3A           /**< Generic PAL ISO 14443-3A definition */
#endif

/* PAL ISO 14443-3B components */

#define NXPBUILD__PHPAL_I14443P3B_SW            /**< Software PAL ISO 14443-3B definition */

#if defined (NXPBUILD__PHPAL_I14443P3B_SW)
    #define NXPBUILD__PHPAL_I14443P3B           /**< Generic PAL ISO 14443-3B definition */
#endif

/* PAL ISO 14443-4A components */

#define NXPBUILD__PHPAL_I14443P4A_SW            /**< Software PAL ISO 14443-4A definition */

#if defined (NXPBUILD__PHPAL_I14443P4A_SW)       || \
    defined (NXPBUILD__PHPAL_I14443P4A_RD710)
    #define NXPBUILD__PHPAL_I14443P4A           /**< Generic PAL ISO 14443-4A definition */
#endif

/* PAL ISO 14443-4 components */

#define NXPBUILD__PHPAL_I14443P4_SW             /**< Software PAL ISO 14443-4 definition */

#if defined (NXPBUILD__PHPAL_I14443P4_SW)       || \
    defined (NXPBUILD__PHPAL_I14443P4_RD710)
    #define NXPBUILD__PHPAL_I14443P4            /**< Generic PAL ISO 14443-4 definition */
#endif

/* PAL MIFARE components */

#define NXPBUILD__PHPAL_MIFARE_STUB             /**< Stub PAL MIFARE */

#if defined (NXPBUILD__PHPAL_MIFARE_STUB)       || \
    defined (NXPBUILD__PHPAL_MIFARE_SW)         || \
    defined (NXPBUILD__PHPAL_MIFARE_RD710)
    #define NXPBUILD__PHPAL_MIFARE              /**< Generic PAL MIFARE definition */
#endif

/* AL MIFARE Classic components */

#define NXPBUILD__PHAL_MFC_SW                   /**< Software MIFARE Classic */

#if defined (NXPBUILD__PHAL_MFC_SW)
    #define NXPBUILD__PHAL_MFC                  /**< Generic AL MIFARE Classic definition */
#endif

#define NXPBUILD__PHAL_MFDF_SW                  /**< Software MIFARE DESFire */

#if defined (NXPBUILD__PHAL_MFDF_SW)
    #define NXPBUILD__PHAL_MFDF                  /**< Generic AL MIFARE Desfire definition */
#endif


/* CID Manager components */

#define NXPBUILD__PH_CIDMANAGER_SW              /**< Software CID Manager */

#if defined (NXPBUILD__PH_CIDMANAGER_SW)       || \
    defined (NXPBUILD__PH_CIDMANAGER_RD710)
    #define NXPBUILD__PH_CIDMANAGER             /**< Generic CID Manager definition */
#endif

/* KeyStore components */

#define NXPBUILD__PH_KEYSTORE_RC632             /**< RC632 KeyStore */

#if defined (NXPBUILD__PH_KEYSTORE_SW)      || \
    defined (NXPBUILD__PH_KEYSTORE_RC663)   || \
    defined (NXPBUILD__PH_KEYSTORE_RC632)   || \
    defined (NXPBUILD__PH_KEYSTORE_RD710)
    #define NXPBUILD__PH_KEYSTORE               /**< Generic KeyStore definition */
#endif

/** @}
* end of ph_NxpBuild
*/

#endif /* PH_NXPBUILD_H */
