/*
 * fatdata.h
 *
 *  Created on: May 22, 2015
 *      Author: sergioam
 */

#ifndef TEMPSENSOR_V1_FATDATA_H_
#define TEMPSENSOR_V1_FATDATA_H_

extern FATFS FatFs;
extern char g_bLogDisabled;
extern char g_bFatInitialized;
extern const char *g_szLastSD_CardError;
extern const char g_pDevReadyFN[];
extern const char g_pDevAlarmFN[];

#define FOLDER_LOG  "/LOG"
#define FOLDER_SYS  ""

// Web format data
#define EXTENSION_DATA "CSV"
#define FOLDER_DATA "/DATA"

// Old data transfer used for parsing and streaming
#define EXTENSION_TEXT "TXT"
#define FOLDER_TEXT "/TXT"

#define LOG_FILE_PATH FOLDER_LOG "/system.log"
#define LOG_MODEM_PATH FOLDER_LOG "/modem.log"
#define CONFIG_INI_FILE FOLDER_SYS "/thermal.ini"
#define APN_INI_FILE FOLDER_SYS "/apn.ini"
#define STRING_INI_FILE FOLDER_SYS "/string.ini"
#define CONFIG_LOG_FILE_PATH FOLDER_LOG "/config.log"
#define LOG_FILE_UNKNOWN FOLDER_DATA "/unknown.csv"

typedef enum {
	LOG_TYPE_SYSTEM,
	LOG_TYPE_MODEM,
	LOG_TYPE_CONFIG
}LOG_TYPE;

#define LOG_MSG "LOG"
typedef enum{
	LOG_REBOOT=0,
	LOG_SIM_SWP,
	LOG_NETDOWN,
	LOG_NETFAIL,
	LOG_ERROR,
	LOG_SIM_SEL_MODE,
	LOG_BATCH_TX,
	LOG_BATCH_RESULT,
	LOG_BATCH_FILE_RESULT,
}LOG_MESSAGE;

char* get_YMD_String(struct tm* timeData);
char* get_current_fileName(struct tm* timeData, const char *folder, const char *ext);
char* get_date_string(struct tm* timeData, const char *dateSeperator,
		const char *dateTimeSeperator, const char *timeSeparator, uint8_t includeTZ);
char* get_simplified_date_string(struct tm* timeData);
void parse_sensor_from_line(char* sensor, char* formattedLine);
void parse_time_from_line(struct tm* timeToConstruct, char* formattedLine);
void parse_sms_format_from_line(char* sensor, char* temp, char* time, char *type, char* line);
void offset_timestamp(struct tm* dateToOffset, int intervalMultiplier);
FRESULT fat_init_drive();
FRESULT fat_save_config(char *text);
void log_reboot(char *mess);
//FRESULT log_sample_to_disk(UINT* tbw);
FRESULT log_append_(LOG_TYPE type, const char *text);
FRESULT log_appendf(LOG_TYPE type, const char *_format, ...);
FRESULT log_sample_web_format(UINT* tbw);

FRESULT fat_create_dev_ready(UINT *tbw);
FRESULT fat_create_dev_alarm_conn(UINT *tbw);
FRESULT fat_create_dev_alarm_sens(UINT *tbw);
FRESULT fat_create_dev_alarm_batt(UINT *tbw);
FRESULT fat_create_dev_alarm_fw_error(UINT *tbw);

void log_disable();
void log_enable();

FRESULT log_modem(const char *text);

#endif /* TEMPSENSOR_V1_FATDATA_H_ */
