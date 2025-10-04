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
* Internal functions for Rc523 specific HAL-Component of Reader Library Framework.
* $Author: jayaprakash.desai $
* $Revision: 1.4 $
* $Date: Mon Jun 27 08:27:01 2011 $
*
* History:
*  CHu: Generated 07. October 2010
*
*/
#ifdef Torgheh

#include <config.h>

#if(ReaderType==NewRoutine)

#include <rdlib/types/RdLib.h>
#include <string.h>
#include <math.h>

#include "phhalHw_Rc523.h"
#include "phhalHw_Rc523_Int.h"
#include "phhalHw_Rc523_Config.h"


phStatus_t phhalHw_Rc523_WriteFifo(
                                   phhalHw_Rc523_DataParams_t * pDataParams,
                                   uint8_t * pData,
                                   uint16_t wLength,
                                   uint16_t * pBytesWritten
                                   )
{
    phStatus_t   status = PH_ERR_SUCCESS;
    phStatus_t   statusTmp;
    uint8_t      bValue;

    /* Read out the FiFo Level register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_FIFOLEVEL, &bValue));
    bValue &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_FLUSHBUFFER;

    /* Check if FiFO has enough space */
    if (wLength > (PHHAL_HW_RC523_FIFOSIZE - bValue))
    {
        /* Truncate byte count */
        wLength = PHHAL_HW_RC523_FIFOSIZE - bValue;

        /* Return buffer overflow error */
        status = PH_ERR_BUFFER_OVERFLOW;
    }

    /* Perform actual write to FiFo */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteData(pDataParams, PHHAL_HW_RC523_REG_FIFODATA, pData, wLength));

    /* Return number of written bytes */
    *pBytesWritten = wLength;

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_ReadFifo(
                                  phhalHw_Rc523_DataParams_t * pDataParams,
                                  uint16_t wBufSize,
                                  uint8_t * pData,
                                  uint16_t * pLength
                                  )
{
    phStatus_t   status;
    phStatus_t   statusTmp;
    uint8_t      bValue;

    /* read out the FiFo Level register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_FIFOLEVEL, &bValue));
    *pLength = (uint16_t)(bValue & (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_FLUSHBUFFER);

    /* check for buffer overflow */
    if (*pLength > wBufSize)
    {
        /* read maximum possible number of bytes */
        *pLength = wBufSize;

        /* return buffer overflow status */
        status = PH_ERR_BUFFER_OVERFLOW;
    }
    else
    {
        /* Buffer is big enough */
        status = PH_ERR_SUCCESS;
    }

    /* Perform actual reading operation */
    if (*pLength > 0)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadData(pDataParams, PHHAL_HW_RC523_REG_FIFODATA, *pLength, pData));
    }

    /* Clear FIFO on buffer overflow */
    if ((status & PH_ERR_MASK) == PH_ERR_BUFFER_OVERFLOW)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FlushFifo(pDataParams));
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_FlushFifo(
                                   phhalHw_Rc523_DataParams_t * pDataParams
                                   )
{
    return phhalHw_Rc523_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_FIFOLEVEL, PHHAL_HW_RC523_BIT_FLUSHBUFFER);
}

phStatus_t phhalHw_Rc523_WriteData(
                                   phhalHw_Rc523_DataParams_t * pDataParams,
                                   uint8_t bAddress,
                                   uint8_t * pData,
                                   uint16_t wLength
                                   )
{
    phStatus_t   statusTmp;
    uint16_t     wIndex;
    uint16_t     wBytesRead;
    uint8_t      bBackup;


    /* Write each single data byte */
    for (wIndex = 0; wIndex < wLength; ++wIndex)
    {
       PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, bAddress, pData[wIndex]));
    }


    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_ReadData(
                                  phhalHw_Rc523_DataParams_t * pDataParams,
                                  uint8_t bAddress,
                                  uint16_t wLength,
                                  uint8_t * pData
                                  )
{
    phStatus_t   statusTmp;
    uint16_t     wIndex;
    uint16_t     wBytesRead;
    uint8_t      aProtocol[1];
    uint8_t      bBackup;

    /* Read each single data byte */
    for (wIndex = 0; wIndex < wLength; ++wIndex)
    {
      PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, bAddress, &pData[wIndex]));
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_WaitIrq(
                                 phhalHw_Rc523_DataParams_t * pDataParams,
                                 uint8_t bIrq0WaitFor,
								 uint8_t bIrq1WaitFor,
                                 uint8_t * pIrq0Reg,
								 uint8_t * pIrq1Reg
                                 )
{
    phStatus_t  statusTmp;
    uint8_t     bIrq0EnReg;
    uint8_t     bIrq1EnReg;
	uint8_t     bRegister;


    /* Parameter check */
    if (!bIrq0WaitFor && !bIrq1WaitFor)
    {
        return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_HAL);
    }

    /* Enable IRQ0 interrupt sources */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIEN, &bIrq0EnReg));
    bIrq0EnReg &= PHHAL_HW_RC523_BIT_IRQINV;
    bIrq0EnReg |= bIrq0WaitFor;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIEN, bIrq0EnReg));
    /* Enable IRQ1 interrupt sources */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_DIVIEN, &bIrq1EnReg));
    bIrq1EnReg &= PHHAL_HW_RC523_BIT_IRQPUSHPULL;
    bIrq1EnReg |= bIrq1WaitFor;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_DIVIEN, bIrq1EnReg));

    /* wait until an IRQ occurs */
    do
    {
        /* INSERT CODE HERE */
        
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_STATUS1, &bRegister));
    }
    while (!(bRegister & PHHAL_HW_RC523_BIT_IRQ));

    /* Clear IRQ0 interrupt sources */
    bIrq0EnReg &= PHHAL_HW_RC523_BIT_IRQINV;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIEN, bIrq0EnReg));
    /* Clear IRQ1 interrupt sources */
    bIrq1EnReg &= PHHAL_HW_RC523_BIT_IRQPUSHPULL;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_DIVIEN, bIrq1EnReg));

    /* Return contents of IRQ0 register */
    if (pIrq0Reg)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, pIrq0Reg));
    }

    /* Return contents of IRQ1 register */
    if (pIrq1Reg)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_DIVIRQ, pIrq1Reg));
    }
    
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_ExchangeTransmit(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t bCmdCode,
    uint8_t * pTxBuffer,
    uint16_t wTxLength
    )
{
    phStatus_t   status;
    phStatus_t   statusTmp;
    uint16_t     wFifoBytes;
    uint8_t      bIrq0WaitFor;
    uint8_t      bIrq1WaitFor;
    uint8_t      bIrq0Rq;
    uint8_t      bIrq1Rq;
    uint8_t      bRegister;
    uint8_t *    pTmpBuffer;
    uint16_t     wTmpBufferLen;
    uint16_t     wTmpBufferSize;
    uint16_t     wNumPrecachedBytes;

    /* retrieve transmit buffer */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetTxBuffer(pDataParams, PH_ON, &pTmpBuffer, &wTmpBufferLen, &wTmpBufferSize));

    /* Precache buffered contents into FIFO */
    wNumPrecachedBytes = (wTmpBufferLen > pDataParams->wMaxPrecachedBytes) ? pDataParams->wMaxPrecachedBytes : wTmpBufferLen;
    if (wNumPrecachedBytes > 0)
    {
        /* Perform actual write to FIFO */
        status = phhalHw_Rc523_WriteFifo(pDataParams, pTmpBuffer, wNumPrecachedBytes, &wFifoBytes);

        /* Ignore FiFo overflow warning */
        if ((status & PH_ERR_MASK) != PH_ERR_BUFFER_OVERFLOW)
        {
            PH_CHECK_SUCCESS(status);
        }

        /* Update buffer pointer and length */
        pTmpBuffer += wFifoBytes;
        wTmpBufferLen = wTmpBufferLen - wFifoBytes;
    }
    else
    {
        status = PH_ERR_SUCCESS;
    }

    /* Precache given buffer contents into FIFO */
    if ((status & PH_ERR_MASK) != PH_ERR_BUFFER_OVERFLOW)
    {
        wNumPrecachedBytes = (wTxLength > (pDataParams->wMaxPrecachedBytes - wNumPrecachedBytes)) ? (pDataParams->wMaxPrecachedBytes - wNumPrecachedBytes) : wTxLength;

        if (wNumPrecachedBytes > 0)
        {
            /* Perform actual write to FIFO */
            status = phhalHw_Rc523_WriteFifo(pDataParams, pTxBuffer, wNumPrecachedBytes, &wFifoBytes);

            /* Ignore FiFo overflow warning */
            if ((status & PH_ERR_MASK) != PH_ERR_BUFFER_OVERFLOW)
            {
                PH_CHECK_SUCCESS(status);
            }

            /* Update buffer pointer and length */
            pTxBuffer += wFifoBytes;
            wTxLength = wTxLength - wFifoBytes;
        }
    }

    /* Write the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, bCmdCode));

    /* start sending */
    if ((bCmdCode == PHHAL_HW_RC523_CMD_TRANSCEIVE) || (pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_BITFRAMING, &bRegister));
        bRegister |= PHHAL_HW_RC523_BIT_STARTSEND;
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_BITFRAMING, bRegister));
    }

    /* Write the remaining data into the Fifo */
    /* (Only possible if interface to RC is fast enough!) */
    while ((wTmpBufferLen != 0) || (wTxLength != 0))
    {
        /* Switch from prechache buffer to given buffer if it's empty */
        if (wTmpBufferLen == 0)
        {
            pTmpBuffer = pTxBuffer;
            wTmpBufferLen = wTxLength;
            wTxLength = 0;
        }
        /* */
        /* Check for FIFO underflow */
        /* */
#ifdef PHHAL_HW_RC523_FEATURE_FIFO_UNDERFLOW_CHECK
        /* read interrupt status */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, &bIrq0Rq));

        /* If we got data to transmit but the Tx-command aborted, we were too slow! */
        if ((bIrq0Rq & PHHAL_HW_RC523_BIT_IDLEI) || (bIrq0Rq & PHHAL_HW_RC523_BIT_TXI))
        {
            /* stop the command */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_COMMAND,
                PHHAL_HW_RC523_CMD_IDLE));

            /* Flush FiFo */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FlushFifo(pDataParams));

            return PH_ADD_COMPCODE(PH_ERR_INTERFACE_ERROR, PH_COMP_HAL);
        }
#endif
        /* write remaining data to transmit into FiFo buffer */
        status = phhalHw_Rc523_WriteFifo(pDataParams, pTmpBuffer, wTmpBufferLen, &wFifoBytes);

        /* Ignore FiFo overflow warning */
        if ((status & PH_ERR_MASK) != PH_ERR_BUFFER_OVERFLOW)
        {
            PH_CHECK_SUCCESS(status);
        }

        /* Update buffer pointer and length */
        pTmpBuffer += wFifoBytes;
        wTmpBufferLen = wTmpBufferLen - wFifoBytes;
    }

    /* Set wait IRQs */
    bIrq0WaitFor = PHHAL_HW_RC523_BIT_TXI | PHHAL_HW_RC523_BIT_IDLEI;
    bIrq1WaitFor = 0x00;

    /* wait until the command is finished */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WaitIrq(
        pDataParams,
        bIrq0WaitFor,
        bIrq1WaitFor,
        &bIrq0Rq,
        &bIrq1Rq));

    /* Check if an error occured */
    if (bIrq0Rq & PHHAL_HW_RC523_BIT_ERRI)
    {
        /* read the error register */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_ERROR, &bRegister));

        /* buffer overflow */
        if (bRegister & PHHAL_HW_RC523_BIT_BUFFEROVFL)
        {
            status = PH_ERR_BUFFER_OVERFLOW;
        }
        /* Fifo write error */
        else if (bRegister & PHHAL_HW_RC523_BIT_WRERR)
        {
            status = PH_ERR_READ_WRITE_ERROR;
        }
        /* No error */
        else
        {
            status = PH_ERR_SUCCESS;
        }
    }
    /* No error */
    else
    {
        status = PH_ERR_SUCCESS;
    }

    /* Reset TxLastBits */
    if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXLASTBITS] != 0)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetConfig(pDataParams, PHHAL_HW_CONFIG_TXLASTBITS, 0x00));
    }

    /* Reset buffered bytes */
    pDataParams->wTxBufLen = 0;

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_ExchangeReceive(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t ** ppRxBuffer,
    uint16_t * pRxLength
    )
{
    phStatus_t   status;
    phStatus_t   statusTmp;
    uint16_t     wFifoBytes;
    uint8_t      bIrq0WaitFor;
    uint8_t      bIrq1WaitFor;
    uint8_t      bIrq0Rq;
    uint8_t      bIrq1Rq;
    uint8_t      bRegister;
    uint32_t     dwTimingSingle;
    uint8_t *    pTmpBuffer;
    uint16_t     wTmpBufferLen;
    uint16_t     wTmpBufferSize;

    /* Set wait IRQs */
    bIrq0WaitFor = PHHAL_HW_RC523_BIT_RXI | PHHAL_HW_RC523_BIT_ERRI | PHHAL_HW_RC523_BIT_HIALERTI | PHHAL_HW_RC523_BIT_TIMERI | PHHAL_HW_RC523_BIT_IDLEI;
    bIrq1WaitFor = 0x00;

    /* wait until the command is finished */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WaitIrq(
        pDataParams,
        bIrq0WaitFor,
        bIrq1WaitFor,
        &bIrq0Rq,
        &bIrq1Rq));

    /* Timeout handling */
    if (bIrq0Rq & PHHAL_HW_RC523_BIT_TIMERI)
    {
        /* Reset after timeout behaviour */
        if (pDataParams->bRfResetAfterTo != PH_OFF)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FieldReset(pDataParams));
        }

        status = PH_ERR_IO_TIMEOUT;
    }
    else
    {
        status = PH_ERR_SUCCESS;
    }

    /* Reset receive buffer length */
    pDataParams->wRxBufLen = 0;

    /* Retrieve receive buffer properties */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetRxBuffer(pDataParams, PH_ON, &pTmpBuffer, &wTmpBufferLen, &wTmpBufferSize));

    /* Do the following if no timeout occured */
    if (status == PH_ERR_SUCCESS)
    {
        /* mask out high-alert */
        bIrq0WaitFor &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_HIALERTI;

        /* retrieve fifo bytes */
        do
        {
            /* read interrupt status */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, &bIrq0Rq));

            /* retrieve bytes from FiFo */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadFifo(
                pDataParams,
                wTmpBufferSize,
                &pTmpBuffer[wTmpBufferLen],
                &wFifoBytes));

            /* advance receive buffer */
            wTmpBufferLen = wTmpBufferLen + wFifoBytes;
            wTmpBufferSize = wTmpBufferSize - wFifoBytes;
        }
        while ((!(bIrq0Rq & bIrq0WaitFor)) || (wFifoBytes != 0));

        /* Check for errors */
        if (bIrq0Rq & PHHAL_HW_RC523_BIT_ERRI)
        {
            /* read the error register */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_ERROR, &bRegister));

            /* Fifo write error */
            if (bRegister & PHHAL_HW_RC523_BIT_WRERR)
            {
                status = PH_ERR_READ_WRITE_ERROR;
            }
            /* temperature error */
            else if (bRegister & PHHAL_HW_RC523_BIT_TEMPERR)
            {
                status = PH_ERR_TEMPERATURE_ERROR;
            }
            /* RF error (should not happen) */
            /*
            if (bRegister & PHHAL_HW_RC523_BIT_RFERR)
            {
            status = PH_ERR_RF_ERROR;
            }
            */
            /* buffer overflow */
            else if (bRegister & PHHAL_HW_RC523_BIT_BUFFEROVFL)
            {
                status = PH_ERR_BUFFER_OVERFLOW;
            }
            /* collision */
            else if (bRegister & PHHAL_HW_RC523_BIT_COLLERR)
            {
                /* No bytes received -> timeout */
                if (wTmpBufferLen == 0)
                {
                    status = PH_ERR_IO_TIMEOUT;
                }
                /* Else perform collision handling */
                else
                {
                    status = PH_ERR_COLLISION_ERROR;

                    /* Retrieve collision position */
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_COLL, &bRegister));

                    /* check if collpos is valid */
                    if (!(bRegister & PHHAL_HW_RC523_BIT_COLLPOSNOTVALID))
                    {
                        /* retrieve collpos */
                        bRegister &= PHHAL_HW_RC523_MASK_COLLPOS;

                        /* 0 means collision in the 32th bit */
                        if (bRegister == 0)
                        {
                            bRegister = 0x1F;
                        }
                        else
                        {
                            --bRegister;
                        }
                    }
                    else
                    {
                        /* collision position out of range --> max. */
                        bRegister = 0x1F;
                    }

                    /* Restore wRxBufSize */
                    wTmpBufferSize = wTmpBufferSize + wTmpBufferLen;

                    /* Check for buffer overflow */
                    if ((uint16_t)(bRegister >> 3) > wTmpBufferSize)
                    {
                        /* Return the maximum possible size of data */
                        wTmpBufferLen = wTmpBufferSize;
                        status = PH_ERR_BUFFER_OVERFLOW;
                    }
                    else
                    {
                        /* Adjust valid byte count */
                        wTmpBufferLen = (uint16_t)(bRegister >> 3);
                    }

                    /* Mask out valid bits of last byte */
                    bRegister &= PHHAL_HW_RC523_MASK_RXBITS;

                    /* Incomplete byte also counts as received byte */
                    if (bRegister)
                    {
                        /* Check for buffer overflow */
                        if (wTmpBufferLen == wTmpBufferSize)
                        {
                            status = PH_ERR_BUFFER_OVERFLOW;
                        }
                        else
                        {
                            ++wTmpBufferLen;
                        }
                    }

                    /* ValuesAfterColl is enabled -> mask out invalid bits */
                    if (bRegister)
                    {
                        pTmpBuffer[wTmpBufferLen - 1] &= (uint8_t)(0xFF >> (8 - bRegister));
                    }

                    /* Store valid bits of last byte */
                    pDataParams->wAdditionalInfo = bRegister;
                }
            }
            /* protocol error */
            else if (bRegister & PHHAL_HW_RC523_BIT_PROTERR)
            {
                status = PH_ERR_PROTOCOL_ERROR;
            }
            /* CRC / parity error */
            else if ((bRegister & PHHAL_HW_RC523_BIT_CRCERR) || (bRegister & PHHAL_HW_RC523_BIT_PARITYERR))
            {
                status = PH_ERR_INTEGRITY_ERROR;
            }
            /* No error */
            else
            {
                status = PH_ERR_SUCCESS;
            }
        }
    }

    /* Receive was successfull */
    if (status == PH_ERR_SUCCESS)
    {
        /* No bytes received -> timeout */
        if (wTmpBufferLen == 0)
        {
            status = PH_ERR_IO_TIMEOUT;
        }
        /* Else retrieve valid bits of last byte */
        else
        {
            /* Retrieve RxBits */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, &bRegister));

            /* Mask out valid bits of last byte */
            bRegister &= PHHAL_HW_RC523_MASK_RXBITS;

            /* Set RxLastBits */
            pDataParams->wAdditionalInfo = bRegister;

            /* Set incomplete byte status if applicable */
            if (bRegister != 0x00)
            {
                status = PH_ERR_SUCCESS_INCOMPLETE_BYTE;
            }
        }
    }

    /* Stop the command / flush the FIFO on error */
    if ((status != PH_ERR_SUCCESS) &&
        (status != PH_ERR_SUCCESS_INCOMPLETE_BYTE))
    {
        /* stop the command */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_IDLE));

        /* Flush FiFo */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FlushFifo(pDataParams));
    }

    /* Store received data length in dataparams */
    pDataParams->wRxBufLen = pDataParams->wRxBufStartPos + wTmpBufferLen;

    /* Return RxBuffer pointer */
    if (ppRxBuffer != NULL)
    {
        *ppRxBuffer = pDataParams->pRxBuffer;
    }

    /* Return RxBuffer length */
    if (pRxLength != NULL)
    {
        *pRxLength = pDataParams->wRxBufLen;
    }

    /* Timing */
    if ((pDataParams->wTimingMode & (uint16_t)~(uint16_t)PHHAL_HW_TIMING_MODE_OPTION_MASK) == PHHAL_HW_TIMING_MODE_FDT)
    {
        /* Calculate FDT Value */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetFdt(pDataParams, status, &dwTimingSingle));

        /* FDT single -> replace the previous value */
        if (pDataParams->wTimingMode & PHHAL_HW_TIMING_MODE_OPTION_AUTOCLEAR)
        {
            pDataParams->dwTimingUs = dwTimingSingle;
        }
        /* FDT -> add current value to current count */
        else
        {
            /* Check for overflow */
            if ((0xFFFFFFFF - pDataParams->dwTimingUs) < dwTimingSingle)
            {
                pDataParams->dwTimingUs = 0xFFFFFFFF;
            }
            /* Update global timing value */
            else
            {
                pDataParams->dwTimingUs = pDataParams->dwTimingUs + dwTimingSingle;
            }
        }
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_SetFdt(
                                phhalHw_Rc523_DataParams_t * pDataParams,
                                uint8_t bUnit,
                                uint16_t wTimeout
                                )
{
    phStatus_t   statusTmp;
    float32_t    fTimeoutUs;
    float32_t    fTemp;
    uint8_t      bRegister;
    uint16_t     wDataRate;
    uint16_t     wPrescaler;
    uint16_t     wReload;

    /* Retrieve RxDataRate */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetConfig(pDataParams, PHHAL_HW_CONFIG_RXDATARATE, &wDataRate));
    wDataRate++;

    /* Handle timeout in microseconds */
    if (bUnit == PHHAL_HW_TIME_MICROSECONDS)
    {
        fTimeoutUs = (float32_t)wTimeout;
    }
    else
    {
        fTimeoutUs = (float32_t)wTimeout * 1000.0f;
    }

    /* Add timershift to timeout value */
    fTimeoutUs = fTimeoutUs + (PHHAL_HW_RC523_TIMER_SHIFT * (PHHAL_HW_RC523_ETU_106 / (float32_t)wDataRate));

    /* Add digital delay for Felica */
    if ((pDataParams->bCardType == PHHAL_HW_CARDTYPE_FELICA) ||
        (
        (pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPI) &&
        (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_RXDATARATE] > PHHAL_HW_RF_DATARATE_106)
        ))
    {
        fTimeoutUs = fTimeoutUs + PHHAL_HW_RC523_FELICA_ADD_DELAY_US;
    }

    /* Max. timeout check */
    if (fTimeoutUs > PHHAL_HW_RC523_TIMER_MAX_VALUE_US)
    {
        return PH_ADD_COMPCODE(PH_ERR_PARAMETER_OVERFLOW, PH_COMP_HAL);
    }

    /* Calculate the prescaler value */
    /* [timeout[us]  = (1 + 2n) / 13.56] */
    fTemp = (((fTimeoutUs * PHHAL_HW_RC523_TIMER_FREQ) / 65535.0f) - 1.0f) / 2.0f;

    /* Except for a special case, the float32_t value will have some commas
    and therefore needs to be "ceiled" */
    if (fTemp > (float32_t)(uint16_t)fTemp)
    {
        wPrescaler = (uint16_t)(fTemp + 1);
    }
    else
    {
        wPrescaler = (uint16_t)fTemp;
    }

    /* Calculate the reload value */
    fTemp = (fTimeoutUs * PHHAL_HW_RC523_TIMER_FREQ) / (1.0f + (2.0f * (float32_t)wPrescaler));

    /* Except for a special case, the float32_t value will have some commas
    and therefore needs to be "ceiled" */
    if (fTemp > (float32_t)(uint16_t)fTemp)
    {
        wReload = (uint16_t)(fTemp + 1);
    }
    else
    {
        wReload = (uint16_t)fTemp;
    }

    /* write Prescaler value */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TPRESCALER, (uint8_t)(wPrescaler & 0xFF)));
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TMODE, &bRegister));
    bRegister &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_TPRESCALER_HI;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TMODE, bRegister | (uint8_t)((wPrescaler >> 8) & 0x0F)));

    /* write Counter value */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TRELOADLO, (uint8_t)(wReload & 0xFF)));
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TRELOADHI, (uint8_t)((wReload >> 8) & 0xFF)));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_GetFdt(
                                phhalHw_Rc523_DataParams_t * pDataParams,
                                phStatus_t wExchangeStatus,
                                uint32_t * pTime
                                )
{
    phStatus_t   statusTmp;
    uint8_t      bRegister;
    uint16_t     wDataRate;
    uint16_t     wPsReload;
    uint16_t     wTReload;
    uint16_t     wTValue;
    float32_t    fTimeout;

    /* Retrieve RxDataRate */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetConfig(pDataParams, PHHAL_HW_CONFIG_RXDATARATE, &wDataRate));
    wDataRate++;

    /* Get prescaler value */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TMODE, &bRegister));
    wPsReload = (uint16_t)(bRegister & PHHAL_HW_RC523_MASK_TPRESCALER_HI) << 8;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TPRESCALER, &bRegister));
    wPsReload |= (uint16_t)bRegister;

    /* Get reload value */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TRELOADHI, &bRegister));
    wTReload = (uint16_t)bRegister << 8;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TRELOADLO, &bRegister));
    wTReload |= (uint16_t)bRegister;

    /* Get counter value */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TCOUNTERVALHI, &bRegister));
    wTValue = (uint16_t)bRegister << 8;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TCOUNTERVALLO, &bRegister));
    wTValue |= (uint16_t)bRegister;

    /* Subtract reload- and counter values */
    wTValue = wTReload - wTValue;

    /* Calculate the timeout value */
    fTimeout = ((float32_t)wTValue * (1.0f + (2.0f * (float32_t)wPsReload))) / PHHAL_HW_RC523_TIMER_FREQ;

    /* Subtract timershift from timeout value */
    fTimeout = fTimeout - (PHHAL_HW_RC523_TIMER_SHIFT * (PHHAL_HW_RC523_ETU_106 / (float32_t)wDataRate));

    /* Return the value */
    *pTime = (uint32_t)fTimeout;

    /* Round the value if neccessary */
    if ((fTimeout - (float32_t)*pTime) >= 0.5)
    {
        ++(*pTime);
    }

    /* Subtract digital delay from timeout */
    if ((wExchangeStatus & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
    {
        /* Felica */
        if ((pDataParams->bCardType == PHHAL_HW_CARDTYPE_FELICA) ||
            (
            (pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPI) &&
            (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_RXDATARATE] > PHHAL_HW_RF_DATARATE_106)
            ))
        {
            if (*pTime > PHHAL_HW_RC523_FELICA_ADD_DELAY_US)
            {
                *pTime = *pTime - PHHAL_HW_RC523_FELICA_ADD_DELAY_US;
            }
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_GetTxBuffer(
                                     phhalHw_Rc523_DataParams_t * pDataParams,
                                     uint8_t bIsExchange,
                                     uint8_t ** pTxBuffer,
                                     uint16_t * pTxBufferLen,
                                     uint16_t * pTxBufferSize
                                     )
{
    /* Exchange command -> RxBuffer overwriting is allowed */
    if (bIsExchange != PH_OFF)
    {
        /* We need to watch that we do not overwrite content below the RxStartPos though */
        if (pDataParams->pTxBuffer == pDataParams->pRxBuffer)
        {
            *pTxBuffer = &pDataParams->pTxBuffer[pDataParams->wRxBufStartPos];
            *pTxBufferSize = pDataParams->wTxBufSize - pDataParams->wRxBufStartPos;
        }
        /* Else just return the actual Buffer */
        else
        {
            *pTxBuffer = pDataParams->pTxBuffer;
            *pTxBufferSize = pDataParams->wTxBufSize;
        }

        /* Return stored length */
        *pTxBufferLen = pDataParams->wTxBufLen;
    }
    /* Normal command -> do not overwrite Tx- and Rx-Contents */
    else
    {
        /* Redundant code */
        return phhalHw_Rc523_GetRxBuffer(pDataParams, bIsExchange, pTxBuffer, pTxBufferLen, pTxBufferSize);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_GetRxBuffer(
                                     phhalHw_Rc523_DataParams_t * pDataParams,
                                     uint8_t bIsExchange,
                                     uint8_t ** pRxBuffer,
                                     uint16_t * pRxBufferLen,
                                     uint16_t * pRxBufferSize
                                     )
{
    /* Exchange command -> TxBuffer overwriting is allowed */
    if (bIsExchange != PH_OFF)
    {
        *pRxBuffer = &pDataParams->pRxBuffer[pDataParams->wRxBufStartPos];
        *pRxBufferSize = pDataParams->wRxBufSize - pDataParams->wRxBufStartPos;
        *pRxBufferLen = pDataParams->wRxBufLen;
    }
    /* Normal command -> do not overwrite Tx- and Rx-Contents */
    else
    {
        /* Command buffering is NOT supported by this HAL */
        *pRxBufferLen = 0;

        /* TxBuffer equals RxBuffer */
        /* And the buffer contains something that needs to be preserved */
        if ((pDataParams->pTxBuffer == pDataParams->pRxBuffer) &&
            ((pDataParams->wTxBufLen != 0) || (pDataParams->wRxBufStartPos != 0)))
        {
            /* Start at TxLenght if neccessary */
            if ((pDataParams->wRxBufStartPos + pDataParams->wTxBufLen) >= pDataParams->wRxBufStartPos)
            {
                *pRxBuffer = &pDataParams->pTxBuffer[pDataParams->wRxBufStartPos + pDataParams->wTxBufLen];
                *pRxBufferSize = pDataParams->wTxBufSize - (pDataParams->wRxBufStartPos + pDataParams->wTxBufLen);
            }
            /* Else start at wRxBufStartPos */
            else
            {
                *pRxBuffer = &pDataParams->pRxBuffer[pDataParams->wRxBufStartPos];
                *pRxBufferSize = pDataParams->wRxBufSize - pDataParams->wRxBufStartPos;
            }
        }
        /* Buffers are different */
        else
        {
            /* Use TxBuffer if it has more space than RxBuffer */
            if ((pDataParams->wTxBufSize - pDataParams->wTxBufLen) >= (pDataParams->wRxBufSize - pDataParams->wRxBufStartPos))
            {
                *pRxBuffer = &pDataParams->pTxBuffer[pDataParams->wTxBufLen];
                *pRxBufferSize = pDataParams->wTxBufSize - pDataParams->wTxBufLen;
            }
            /* Else use RxBuffer */
            else
            {
                *pRxBuffer = &pDataParams->pRxBuffer[pDataParams->wRxBufStartPos];
                *pRxBufferSize = pDataParams->wRxBufSize - pDataParams->wRxBufStartPos;
            }
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

#endif 

#endif 
