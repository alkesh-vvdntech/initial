#include "FAT_Update.h"
#include <intrinsics.h>
#include <string.h>

char g_bFatInitialized = false;

#define FOLDER_LOG  "/LOG123"
#define FOLDER_SYS  ""

// Web format data
#define EXTENSION_DATA "CSV123"
#define FOLDER_DATA "/DATA123"

// Old data transfer used for parsing and streaming
#define EXTENSION_TEXT "TXT123"
#define FOLDER_TEXT "/TXT123"


FATFS g_FatFs; /* Work area (file system object) for logical drive */
const char *g_szLastSD_CardError = NULL;
uint8_t g_fatFileCaptured = 0;
FIL g_fatFilr;
FILINFO g_fatFili;
DIR g_fatDir;
//-------------------------------------------------------------------------------------------

DIR *fat_getDirectory() {
    return &g_fatDir;
}

FILINFO *fat_getInfo() {
    return &g_fatFili;
}

FIL *fat_getFile() {
    return &g_fatFilr;
}

FRESULT fat_open(FIL **fobj, char *path, BYTE mode) {
    FRESULT res;

    if (g_fatFileCaptured == 1) {
	return FR_OK;
    }

    res = f_open(&g_fatFilr, path, mode);
    *fobj = &g_fatFilr;

    if (res == FR_OK) {
	g_fatFileCaptured = 1;
    }
    return res;
}

FRESULT fat_close() {
    if (g_fatFileCaptured == 0) {
	return FR_OK;
    }

    g_fatFileCaptured = 0;
    return f_close(&g_fatFilr);
}

//-------------------------------------------------------------------------------------------

const char * const FR_ERRORS[20] = { "OK", "DISK_ERR", "INT_ERR", "NOT_READY",
"NO_FILE", "NO_PATH", "INVALID_NAME", "DENIED", "EXIST",
"INVALID_OBJECT", "WRITE_PROTECTED", "INVALID_DRIVE", "NOT_ENABLED",
"NO_FILESYSTEM", "MKFS_ABORTED", "TIMEOUT", "LOCKED", "NOT_ENOUGH_CORE",
"TOO_MANY_OPEN_FILES", "INVALID_PARAMETER" };

DWORD get_fattime(void) {
    DWORD tmr;

    /* Default beginning of epoch here since bootloader will not be using rtc */
    tmr = ((DWORD)(0) << 25 | (DWORD)(1) << 21 | (DWORD)(1) << 16);
    return tmr;
}

void fat_check_error(FRESULT fr) {
    if (fr == FR_OK)  {
	fr = f_mkdir(FOLDER_LOG);
	if (fr != FR_EXIST)
	{
	}
	fr = f_mkdir(FOLDER_TEXT);
	if (fr != FR_EXIST)
	{
	}
	return;
    }
    if (fr == FR_DISK_ERR || fr == FR_NOT_READY)
	g_bFatInitialized = false;

    g_szLastSD_CardError = FR_ERRORS[fr];
}

FRESULT FAT_Update_Init_Drive() {
    FRESULT fr = FR_OK;
    FILINFO *fno = fat_getInfo();

    if(g_bFatInitialized == true)
	return fr;

    //zero out globals
    memset(&g_fatFili, 0, sizeof(g_fatFili));
    memset(&g_fatDir, 0, sizeof(g_fatDir));
    memset(&g_fatFilr, 0, sizeof(g_fatFilr));
    g_fatFileCaptured = 0;
    g_szLastSD_CardError = NULL;
    /* Register work area to the default drive */
    fr = f_mount(&g_FatFs, "", 0);
    fat_check_error(fr);
    if (fr == FR_OK) {
	// Fat is ready
	g_bFatInitialized = true;
    }

    return fr;
}

uint8_t FAT_Update_ValidImageFound(void) {
    FRESULT fr = FR_OK;
    FILINFO *p_fno = fat_getInfo();

    fr = FAT_Update_Init_Drive();
    fat_check_error(fr);
    if(fr == FR_OK) {
	//check for update image file
	fr = f_stat(UPDATE_FILE_PATH, p_fno);
    }

    //	return fr == FR_OK;
    return fr;
}

void FAT_Update_GetUpdateFile (void) {
    FRESULT fr = FR_OK;
    FIL *p_fil = fat_getFile();


    fr = FAT_Update_Init_Drive(); //make sure we are mounted
    if(fr == FR_OK) {
	//open the update file if one exists
	//  fr = fat_open(&p_fil, UPDATE_FILE_PATH, FA_READ | FA_OPEN_EXISTING);
	fr = fat_open(&p_fil,"UPDATE.txt", FA_READ | FA_OPEN_EXISTING);
	if(fr == FR_OK) {
	    //seek to beginning of file always
	    fr = f_lseek(p_fil, 0);
	}
    }
    fat_check_error(fr);
}

uint8_t FAT_Update_GetLine (uint8_t *p_usBuffer, uint16_t usBuffer_n) {
    FIL *p_fil = fat_getFile();
    TCHAR *res = f_gets((char *)p_usBuffer, (int)usBuffer_n, p_fil);

    if(res != 0) { //remove \r or \n
	int len = strlen((char const *)p_usBuffer);
	if(len > 2 && len <= usBuffer_n) {
	    if(p_usBuffer[len - 2] == '\r')
		p_usBuffer[len - 2] = 0;
	    else if(p_usBuffer[len - 1] == '\n')
		p_usBuffer[len - 1] = 0;
	}
    }

    return res != 0;
}

void FAT_Update_CloseUpdateFile (void) {
    fat_close();
}