/*
 * file_system.h
 *
 *  Created on: Aug 1, 2017
 *      Author: rachit
 */

#include "modem_uart.h"
   
#define GSM_FLASH_GET_DRIVE	        "AT+FSDRIVE=0\r\n"
#define GSM_FLASH_READ	                "AT+FSREAD=%s.txt,1,%d,%d\r\n"
#define GSM_FLASH_WRITE	                "AT+FSWRITE=%s.txt,0,%d,120\r\n"
#define GSM_FLASH_DEL	                "AT+FSDEL=%s.txt\r\n"
#define GSM_FLASH_CREATE_FILE	        "AT+FSCREATE=%s.txt\r\n"
#define GSM_FLASH_LIST_DIR              "AT+FSLS = C:\\\r\n"
#define GET_FILE_SIZE                   "AT+FSFLSIZE=%s.txt\r\n"
   
   
   
   
void file_system(void);
unsigned int read_file_size(char *apn_name);
void read_apn_config(char *apn_name,uint16_t apn_len);
unsigned char  write_apn_config(char * apn_code_name,char *apn_codes,uint16_t apn_len);