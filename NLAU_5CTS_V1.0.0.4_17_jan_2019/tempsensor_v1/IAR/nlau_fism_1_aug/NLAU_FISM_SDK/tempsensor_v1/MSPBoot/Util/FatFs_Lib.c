#include "FatFs_Lib.h"

#ifdef _CT5_BOOTLOADER_

#if defined(__IAR_SYSTEMS_ICC__)
    /* static data buffer for acceleration */
    #pragma location = "FATFS_VTABLE"
__root const uint32_t FatFs_Vectors[] = {
    (uint32_t)f_open,            //_FATFS_VECTOR + 0
    (uint32_t)f_close,           //_FATFS_VECTOR + 1
//    (uint32_t)f_read,
    (uint32_t)f_write,           //_FATFS_VECTOR + 2
    (uint32_t)f_lseek,           //_FATFS_VECTOR + 3
//    (uint32_t)f_truncate,
//    (uint32_t)f_sync,
//    (uint32_t)f_opendir,
//    (uint32_t)f_closedir,
//    (uint32_t)f_readdir,
    (uint32_t)f_findfirst,       //_FATFS_VECTOR + 4
    (uint32_t)f_findnext,        //_FATFS_VECTOR + 5
    (uint32_t)f_mkdir,           //_FATFS_VECTOR + 6
    (uint32_t)f_unlink,          //_FATFS_VECTOR + 7
    (uint32_t)f_rename,          //_FATFS_VECTOR + 8
    (uint32_t)f_stat,            //_FATFS_VECTOR + 9
//    (uint32_t)f_chmod,
//    (uint32_t)f_utime,
//    (uint32_t)f_chdir,
//    (uint32_t)f_chdrive,
//    (uint32_t)f_getcwd,
//    (uint32_t)f_getfree,
//    (uint32_t)f_getlabel,
//    (uint32_t)f_getlabel,
    (uint32_t)f_mount,           //_FATFS_VECTOR + 10
//    (uint32_t)f_mkfs,
//    (uint32_t)f_fdisk,
//    (uint32_t)f_putc,
//    (uint32_t)f_puts,
//    (uint32_t)f_printf,
    (uint32_t)f_gets             //_FATFS_VECTOR + 11
   //  (uint32_t)f_sync
};
#else
#error "Compiler not supported!"
#endif

#else

//
//  External variables from linker file
//
/*! Address of table with Vectors/functions shared by boot:
FatFs:
    [0] = FRESULT (*f_close) (FIL* fp)
    [1] = FRESULT (*f_read) (FIL* fp, void* buff, UINT btr, UINT* br)
    [2] = FRESULT (*f_write) (FIL* fp, const void* buff, UINT btw, UINT* bw)
 */
extern const uint32_t _FATFS_VECTOR[12];

static FRESULT (**pf_open) (FIL* fp, const TCHAR* path, BYTE mode) = (FRESULT (**) (FIL* fp, const TCHAR* path, BYTE mode)) (_FATFS_VECTOR + 0);
static FRESULT (**pf_close) (FIL* fp) = (FRESULT (**) (FIL* fp)) (_FATFS_VECTOR + 1);
//static FRESULT (**pf_read) (FIL* fp, void* buff, UINT btr, UINT* br);
static FRESULT (**pf_write) (FIL* fp, const void* buff, UINT btw, UINT* bw) = (FRESULT (**) (FIL* fp, const void* buff, UINT btw, UINT* bw)) (_FATFS_VECTOR + 2);
static FRESULT (**pf_lseek) (FIL* fp, DWORD ofs) = (FRESULT (**) (FIL* fp, DWORD ofs)) (_FATFS_VECTOR + 3);

//static FRESULT (*pf_truncate) (FIL* fp);
//static FRESULT (*pf_sync) (FIL* fp);
//static FRESULT (*pf_opendir) (DIR* dp, const TCHAR* path);
//static FRESULT (*pf_closedir) (DIR* dp);
//static FRESULT (*pf_readdir) (DIR* dp, FILINFO* fno);

static FRESULT (**pf_findfirst) (DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern) = (FRESULT (**) (DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern)) (_FATFS_VECTOR + 4);
static FRESULT (**pf_findnext) (DIR* dp, FILINFO* fno) = (FRESULT (**) (DIR* dp, FILINFO* fno)) (_FATFS_VECTOR + 5);
static FRESULT (**pf_mkdir) (const TCHAR* path) = (FRESULT (**) (const TCHAR* path)) (_FATFS_VECTOR + 6);
static FRESULT (**pf_unlink) (const TCHAR* path) = (FRESULT (**) (const TCHAR* path)) (_FATFS_VECTOR + 7);
static FRESULT (**pf_rename) (const TCHAR* path_old, const TCHAR* path_new) = (FRESULT (**) (const TCHAR* path_old, const TCHAR* path_new)) (_FATFS_VECTOR + 8);
static FRESULT (**pf_stat) (const TCHAR* path, FILINFO* fno) = (FRESULT (**) (const TCHAR* path, FILINFO* fno)) (_FATFS_VECTOR + 9);

//static FRESULT (*pf_chmod) (const TCHAR* path, BYTE attr, BYTE mask);
//static FRESULT (*pf_utime) (const TCHAR* path, const FILINFO* fno);
//static FRESULT (*pf_chdir) (const TCHAR* path);
//static FRESULT (*pf_chdrive) (const TCHAR* path);
//static FRESULT (*pf_getcwd) (TCHAR* buff, UINT len);
//static FRESULT (*pf_getfree) (const TCHAR* path, DWORD* nclst, FATFS** fatfs);
//static FRESULT (*pf_getlabel) (const TCHAR* path, TCHAR* label, DWORD* vsn);
//static FRESULT (*pf_setlabel) (const TCHAR* label);

static FRESULT (**pf_mount) (FATFS* fs, const TCHAR* path, BYTE opt) = (FRESULT (**) (FATFS* fs, const TCHAR* path, BYTE opt)) (_FATFS_VECTOR + 10);

//static FRESULT (*pf_mkfs) (const TCHAR* path, BYTE sfd, UINT au);
//static FRESULT (*pf_fdisk) (BYTE pdrv, const DWORD szt[], void* work);
//static int (*pf_putc) (TCHAR c, FIL* fp);
//static int (*pf_puts) (const TCHAR* str, FIL* cp);
//static int (*pf_printf) (FIL* fp, const TCHAR* str, ...);

static TCHAR* (**pf_gets) (TCHAR* buff, int len, FIL* fp) = (TCHAR* (**) (TCHAR* buff, int len, FIL* fp)) (_FATFS_VECTOR + 11);


FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode) {
    return (**pf_open) (fp, path, mode);
}

FRESULT f_close (FIL* fp) {
    return (**pf_close) (fp);
}

FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw) {
    return (**pf_write) (fp, buff, btw, bw);
}

FRESULT f_lseek (FIL* fp, DWORD ofs) {
    return (**pf_lseek) (fp, ofs);
}


//FRESULT f_truncate (FIL* fp);
//FRESULT f_sync (FIL* fp);
//FRESULT f_opendir (DIR* dp, const TCHAR* path);
//FRESULT f_closedir (DIR* dp);
//FRESULT f_readdir (DIR* dp, FILINFO* fno);

FRESULT f_findfirst (DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern) {
    return (**pf_findfirst) (dp, fno, path, pattern);
}

FRESULT f_findnext (DIR* dp, FILINFO* fno) {
    return (**pf_findnext) (dp, fno);
}

FRESULT f_mkdir (const TCHAR* path) {
    return (**pf_mkdir) (path);
}

FRESULT f_unlink (const TCHAR* path) {
    return (**pf_unlink) (path);
}

FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new) {
    return (**pf_rename) (path_old, path_new);
}

FRESULT f_stat (const TCHAR* path, FILINFO* fno) {
    return (**pf_stat) (path, fno);
}


//FRESULT f_chmod (const TCHAR* path, BYTE attr, BYTE mask);
//FRESULT f_utime (const TCHAR* path, const FILINFO* fno);
//FRESULT f_chdir (const TCHAR* path);
//FRESULT f_chdrive (const TCHAR* path);
//FRESULT f_getcwd (TCHAR* buff, UINT len);
//FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);
//FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn);
//FRESULT f_setlabel (const TCHAR* label);

FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt) {
    return (**pf_mount) (fs, path, opt);
}


//FRESULT f_mkfs (const TCHAR* path, BYTE sfd, UINT au);
//FRESULT f_fdisk (BYTE pdrv, const DWORD szt[], void* work);
//int f_putc (TCHAR c, FIL* fp);
//int f_puts (const TCHAR* str, FIL* cp);
//int f_printf (FIL* fp, const TCHAR* str, ...);

TCHAR* f_gets (TCHAR* buff, int len, FIL* fp) {
    return (**pf_gets) (buff, len, fp);
}

#endif
