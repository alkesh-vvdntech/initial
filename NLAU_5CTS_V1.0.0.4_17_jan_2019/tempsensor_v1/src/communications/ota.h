
#ifndef OTA_H
#define OTA_H

/***********************************HEADER FILES************************************************/
#include <stdio.h>
#define OTA_PKT_LENGTH	        250

#define READ_SMS	        "AT+CMGR=1\r\n"
#define DELETE_SMS	        "AT+CMGD=1,3\r\n"
#define NEW_SMS_INDICATION	"AT+CNMI=2,1,0,0\r\n"
#define AT			"AT\r\n"
#define SOFTWARE_VERSION 	"AT+CGMR\r\n"
#define NETWORK_REG_STATUS 	"AT+CREG?\r\n"
#define CFUN			"AT+CFUN=1\r\n"
#define CFUN_1		 	"AT+CFUN=1,1\r\n"
#define SIM_READY       	"AT+CPIN?\r\n"
#define GSM_BUSY		"AT+GSMBUSY=0\r\n"
#define GSM_BUSY_1   		"AT+GSMBUSY=1\r\n"
#define CANCEL_ECHO     	"ATE0\r\n"
#define ENABLE_ERROR		"AT+CMEE=2\r\n"
#define ACTIVE_PROFILE		"AT&W\r\n"
#define ACTIVATE_GPRS   	"AT+CGATT=1\r\n"
#define CHECK_GPRS     		"AT+CGATT?\r\n"
#define CONTYPE			"AT+SAPBR=3,1,\"contype\",\"GPRS\"\r\n"
#define SET_APN			"AT+SAPBR=3,1,\"APN\",\"%s\"\r\n"
#define DETACH_BEARER 		"AT+SAPBR=0,1\r\n"
#define SET_BEARER      	"AT+SAPBR=1,1\r\n"
#define CHECK_SAP_IP     	"AT+SAPBR=2,1\r\n"

#define DELETE_OTA_IMAGE	"AT+FSDEL=image.txt\r\n"
#define CREATE_OTA_IMAGE	"AT+FSCREATE=image.txt\r\n"

#define IMEI             	"AT+GSN\r\n"
#define TCP_SENDDATA     	"AT+CIPSEND\r\n"
#define TCP_CLOSE        	"AT+CIPCLOSE\r\n"
#define TCP_CLOSE_ALL    	"AT+CIPSHUT\r\n"
#define TCP_SERVER_CLOSE	"AT+CIPSERVER=0\r\n"
#define TCP_STATUS       	"AT+CIPSTATUS\r\n"
#define TCP_SINGLE_CONNECTION   "AT+CIPMUX=0\r\n"
#define TCP_APN			"AT+CSTT=luminous\r\n"
#define TCP_BRINGUP_WIRELESS    "AT+CIICR\r\n"
#define TCP_GET_LOCAL_IP        "AT+CIFSR\r\n"
#define TCP_SERVER		"AT+CIPSERVER=1,1010\r\n"
#define TCP_HEAD                "AT+CIPHEAD=1\r\n"
/*****************************************MACRO DEFINITIONS*************************************/
unsigned char ota_process(void);
unsigned int CRC_check(char *buffer);
void check_ota_failed();
void check_for_OTA(void);

#endif
/* EOF */
