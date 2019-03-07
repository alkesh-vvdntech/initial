#ifndef __FAT_UPDATE_H__
#define __FAT_UPDATE_H__

/*

	Author: Ben Altieri
	Date: 12/19/2015
	
	Stemming from Coldtrace5 fat_data source this module interfaces
    with the ff11 library connected to an onboard sd card over SPI
    driven by the sd_raw library

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "ff.h"

#define UPDATE_FILE_PATH "UPDATE.TXT"

uint8_t FAT_Update_ValidImageFound(void);

void FAT_Update_GetUpdateFile (void);
uint8_t FAT_Update_GetLine (uint8_t *p_usBuffer, uint16_t usBuffer_n);
void FAT_Update_CloseUpdateFile (void);

#endif /* __FAT_UPDATE_H__ */
