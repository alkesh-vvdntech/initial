/*
 * common.h
 *
 *  Created on: Mar 19, 2015
 *      Author: rajeevnx
 */

#ifndef TEMPSENSOR_V1_COMMON_H_
#define TEMPSENSOR_V1_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Defines for functionality configuration
 */
#define POWER_SAVING_ENABLED
#define ENABLE_SIM_SLOT		//needed to set on new board, comment it for old board
#define SEQUENCE
//#define FILE_TEST
//#define SAMPLE_POST
#define NO_CODE_SIZE_LIMIT
//#define BATTERY_DISABLED
//#define I2C_DISABLED

//#define NOTFROMFILE
#define BUZZER_DISABLED
#define ALERT_UPLOAD_DISABLED
//#define CALIBRATION			//set this flag whenever the device has to undergo calibration
#define MIN 14

/*
 * Communication
 */

#define GSM 1
#define GPRS 2

/*
 * Timeouts for modem transactions
 *
 * TODO Modify delay times with the documentation for the Telit modem
 * see section 3.2.4 Command Response Time-Out
 * https://www.sparkfun.com/datasheets/Cellular%20Modules/AT_Commands_Reference_Guide_r0.pdf
 *
 */

#ifdef _DEBUG
#define MODEM_TX_DELAY1		5000
#define MODEM_TX_DELAY2		10000
#else
#define MODEM_TX_DELAY1		5000
#define MODEM_TX_DELAY2		10000
#endif

/*
 * Defines for data sizes
 */

//len constants
#define IMEI_MAX_LEN		 16
#define IMEI_MIN_LEN		 15
#define GW_MAX_LEN		 15
#define APN_MAX_LEN     	 32
#define SID_MAX_LEN		20
#define FWV_MAX_LEN		16
#define ERROR_MAX_LEN   	 32
#define MCC_MAX_LEN     	 3
#define MNC_MAX_LEN     	 2
//#define UPLOAD_MODE_LEN 	 4
#define MAX_SMS_NUM			 2
#define SMS_NUM_LEN			 12

#define ENCODED_TEMP_LEN	 3

#define SMS_MSG_VERS	 	1
#define SMS_MSG_TYPE_TEMPDATA	 0
#define SMS_MSG_TYPE_DATAREADY	 2


#define SMS_TEMP_TYPE_RAW	 0
//#define SMS_TEMP_TYPE_EXCUR	 1
//#define SMS_TEMP_TYPE_INCUR	 2

/*
 * Constants for different uses
 */

#define MAX_DISPLAY_ID		10

//iStatus contants
#define MODEM_POWERED_ON 	0x0001
#define ALERT_UPLOAD_ON		0x0002
#define LOG_TIME_STAMP		0x0004
#define SPLIT_TIME_STAMP	0x0008

#define RESET_ALERT  		0x0020
#define BUZZER_ON	  		0x0040
#define BACKLOG_UPLOAD_ON	0x0100
#define SYSTEM_SETUP  		0x0400

#define LOW_RANGE_MIN 		0
#define LOW_RANGE_MAX 		10

#define MED_RANGE_MIN 		11
#define MED_RANGE_MAX 		20

#define HIGH_RANGE_MIN 		21
#define HIGH_RANGE_MAX 		31

#define NAME_LEN	        2

#ifdef __cplusplus
}
#endif


#endif /* TEMPSENSOR_V1_COMMON_H_ */
