#ifndef __FATFS_LIB_H__
#define __FATFS_LIB_H__

/*

	Author: Ben Altieri
	Date: 1/14/2016
	
	Specifies an externally callable set of methods
    compiled from FatFs library source

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "ff.h"


#ifdef _CT5_BOOTLOADER_
//
//  External variables from linker file. 
//  Note that this file gets constants from linker file
//
extern const uint16_t _FATFS_VECTOR;            /*! Fat Library Vector Start Address */

/* bootloader held FatFs vector table */
extern const uint32_t FatFs_Vectors[];
#endif

#endif /* __FATFS_LIB_H__ */