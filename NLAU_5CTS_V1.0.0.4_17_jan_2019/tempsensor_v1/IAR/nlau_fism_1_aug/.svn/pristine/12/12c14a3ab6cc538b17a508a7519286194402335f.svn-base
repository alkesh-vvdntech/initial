/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

// TODO Clean all the returns and crazy status here from original developer :(

#include "diskio.h"		/* FatFs lower layer API */
#include <sd_raw.h>	    /* Header file of MMC/SDHC card control module */


DSTATUS stat;
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	//return the status code
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	uint8_t result = 0;

	//returns 0 on failure, 1 on success.
	result = sd_raw_init();
	if(result == 1){
		stat = RES_OK; //ready if 1
	}
	else{
		stat = RES_NOTRDY; //else failed, not ready
	}

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	uint8_t result = 0;
	int iIdx   = 0;

	for(iIdx = 0; ((iIdx < count) && (stat == RES_OK)); iIdx++)
	{
		//sd_raw_read(offset_t offset, uint8_t* buffer, uintptr_t length)
		result = sd_raw_read(sector*SECTOR_SIZE,(uint8_t *)  buff, SECTOR_SIZE);
		if(result == 1) //success on 1
		{
			stat = RES_OK;
			buff += SECTOR_SIZE;
		}
		else
		{
			stat = RES_ERROR;
		}
	}

	return (DRESULT) stat;
}

DRESULT disk_read_ex (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Bytes to read */
)
{
	uint8_t result = 0;
	//int iIdx   = 0;

	//for(iIdx = 0; ((iIdx < count) && (stat == RES_OK)); iIdx++)
	{
		result = sd_raw_read(sector*SECTOR_SIZE, buff, SECTOR_SIZE);
		if(result == 1)
		{
			stat = RES_OK;
			//buff += SECTOR_SIZE;
		}
		else
		{
			stat = RES_ERROR;
		}
	}


	return (DRESULT) stat;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	uint8_t result = 0;
	int iIdx   = 0;

	for(iIdx = 0; ((iIdx < count) && (stat == RES_OK)); iIdx++)
	{
		//sd_raw_write(offset_t offset, const uint8_t* buffer, uintptr_t length)
		//result = mmcWriteBlock(sector*SECTOR_SIZE, SECTOR_SIZE, (unsigned char *) buff);
		result = sd_raw_write(sector*SECTOR_SIZE, (const uint8_t *) buff, SECTOR_SIZE);
		if(result == 1)
		{
			stat = RES_OK;
			buff += SECTOR_SIZE;
		}
		else
		{
			stat = RES_ERROR;
		}
	}


	return (DRESULT) stat;

}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (cmd)
	{
	case CTRL_SYNC:
		stat = RES_OK; //as write is Write-around cache
		break;
	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (WORD) */
		*(DWORD*)buff = SECTOR_COUNT; //TODO write get function for sector count
		stat = RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*(DWORD*)buff = SECTOR_SIZE;
		stat = RES_OK;
		break;
	default:
		stat = RES_PARERR;
		break;
	}
	return (DRESULT) stat;
}
#endif
