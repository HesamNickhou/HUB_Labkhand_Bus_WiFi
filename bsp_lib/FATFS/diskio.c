#include "sd/mmc_sd.h"
#include "diskio.h"
 		   
 
/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
SD_CardInfo SDCardInfo;
#define BlockSize            512 /* Block Size in Bytes [֢oքblockڬӥˇpʈȸqìSDߨքÿٶʈȸΪ512ؖޚ]*/
#define BufferWordsSize      (BlockSize >> 2)	//128ìڬӥӎ߼MAINאքޢˍìˇ128ٶ32λքۺԥȸìզ؅512ٶؖޚ
//u32 buff2[512/4];


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive [كگ˽ˇѨҪلдք]                                                   */
/* עӢúՓگ˽քպëʏ4ߴì̹νքdrv֢ٶӎ˽ìˇԃһٶ˽ؖ4պҭτݾӲݾք`эìՓǤswitchԯߤߴ
   drv = ATA = 0úҭʾˇIDEӲƌքһזޓࠚ
   drv = MMC = 1: ҭʾքˇMMCߨքޓࠚìҲߍˇSDߨ
   drv = USB = 2: ҭʾքˇUSBզԢʨѸքޓࠚ
   Փ֢oߴ4ìϒćӦكҪѡձքˇMMCޓࠚ,ˤɫӎ˽ҘѫΪ1*/
DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{	
  SD_Error Status;
  Status = SD_Init();
  if (Status == SD_OK)
  {
    /*----------------- Read CSD/CID MSD registers ------------------*/
    Status = SD_GetCardInfo(&SDCardInfo);
  }

  if (Status == SD_OK)
  {
    /*----------------- Select Card --------------------------------*/
    Status = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
  }

  if (Status == SD_OK)
  {
    Status = SD_EnableWideBusOperation(SDIO_BusWide_1b);
  }

  /* Set Device Transfer Mode to DMA [ʨ׃ԫˤģʽΪDMA׽ʽ]*/
  if (Status == SD_OK)
  {  
    //Status = SD_SetDeviceMode(SD_INTERRUPT_MODE);
	  Status = SD_SetDeviceMode(SD_POLLING_MODE);
  }
  if(Status == SD_OK)return 0;
  else return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status  [كگ˽ѨҪلд]                                                  */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s) [كگ˽ѨҪلд]                                       */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
  //memset(buff2, 0, sizeof(buff2));
	if(count==1)
        {
          SD_ReadBlock(sector << 9 ,(u32 *)(&buff[0]),BlockSize);
          //memcpy(buff,buff2,SECTOR_SIZE);
	}
	else
        {
          SD_ReadMultiBlocks(sector << 9 ,(u32 *)(&buff[0]),BlockSize,count);
          //memcpy(buff,buff2,SECTOR_SIZE * count);
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s) [كگ˽ѨҪلд]                                                      */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
  //memset(buff2, 0, sizeof(buff2));
	if(count==1)
        {
          //memcpy(buff2,buff,SECTOR_SIZE);
          SD_WriteBlock(sector << 9 ,(u32 *)(&buff[0]),BlockSize);
	}
	else
        {
          //memcpy(buff2,buff,SECTOR_SIZE * count);
          SD_WriteMultiBlocks(sector << 9 ,(u32 *)(&buff[0]),BlockSize,count);
	}
        
  return RES_OK;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions [كگ˽ѨҪلд]                                              */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_OK;
}

DWORD get_fattime (void)
{				 
	return 0;
}			 

