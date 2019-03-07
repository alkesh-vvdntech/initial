#ifndef __TI_TXT_FILE__PARSER_H__
#define __TI_TXT_FILE__PARSER_H__

/*

	Author: Ben Altieri
	Date: 11/13/2015
	
	This interface serves as a translator/controller between
	a raw TI TXT formatted set of data from a configurable
	source and a rudimentary FRAM based FW image update routine.
	The currently targeted routine is based on the TI developed
	MSPBoot source code.

*/

//place includes for source and target routines here
#include <stdint.h>

//typedef for required routines
typedef void (*TI_TXT_InitLineFeed) (void);
typedef uint8_t (*TI_TXT_GetLine) (uint8_t *p_usBuffer, uint16_t usBuffer_n);
typedef void (*TI_TXT_ReleaseLineFeed) (void);

//input routines must defined in TI_TXT_File_Parser.c
extern const TI_TXT_InitLineFeed g_fpInitLineFeed;
extern const TI_TXT_GetLine g_fpGetLine;
extern const TI_TXT_ReleaseLineFeed g_fpReleaseLineFeed;

uint8_t TI_TXT_Program(void);
uint8_t TI_TXT_GetShortAtAddress(uint16_t usAddress, uint16_t *p_usData);

#endif  //__TI_TXT_FILE__PARSER_H__
