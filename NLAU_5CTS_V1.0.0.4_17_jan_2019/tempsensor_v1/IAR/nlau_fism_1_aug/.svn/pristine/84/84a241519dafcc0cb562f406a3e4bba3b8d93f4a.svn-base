#include "thermalcanyon.h"
#include "state_machine.h"
#include "json.h"
#include "alarms.h"
#include "jsonAPIs.h"

//extern FRESULT sync_fs( /* FR_OK: successful, FR_DISK_ERR: failed */
// FATFS* fs /* File system object */
//);

char g_szFatFileName[32] = "";
char g_bFatInitialized = false;
char g_bLogDisabled = false;

#if defined(__TI_COMPILER_VERSION__)
#pragma SET_DATA_SECTION(".aggregate_vars")
FATFS FatFs; /* Work area (file system object) for logical drive */
const char *g_szLastSD_CardError = NULL;
uint8_t g_fatFileCaptured = 0;
FILINFO g_fatFili;
DIR g_fatDir;
FIL g_fatFilr;
#pragma SET_DATA_SECTION()
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma location="AGGREGATE"
__no_init FATFS FatFs; /* Work area (file system object) for logical drive */
#pragma location="AGGREGATE"
__no_init const char *g_szLastSD_CardError;
#pragma location="AGGREGATE"
__no_init uint8_t g_fatFileCaptured;
#pragma location="AGGREGATE"
__no_init FILINFO g_fatFili;
#pragma location="AGGREGATE"
__no_init DIR g_fatDir;
#pragma location="AGGREGATE"
__no_init FIL g_fatFilr;
#else
#error Compiler not supported!
#endif

int fat_check_flag;
int fobj_flag;


const char g_pDevReadyFN[] = "dev_rdy.tmp";
const char g_pDevAlarmFN[] = "dev_alrm.tmp";

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
		_NOP();
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
		_NOP();
		return FR_INT_ERR;
	}

	g_fatFileCaptured = 0;
	return f_close(&g_fatFilr);
}

//-------------------------------------------------------------------------------------------

//const char * const FR_ERRORS[20] = { "OK", "DISK_ERR", "INT_ERR", "NOT_READY",
//		"NO_FILE", "NO_PATH", "INVALID_NAME", "DENIED", "EXIST",
//		"INVALID_OBJECT", "WRITE_PROTECTED", "INVALID_DRIVE", "NOT_ENABLED",
//		"NO_FILESYSTEM", "MKFS_ABORTED", "TIMEOUT", "LOCKED", "NOT_ENOUGH_CORE",
//		"TOO_MANY_OPEN_FILES", "INVALID_PARAMETER" };

const char * const FR_ERRORS[20] = { "OK", "DISK_ERR", "INT_ERR", "NT_RDY",
		"NO_FILE", "NO_PATH", "INVLD_NAME", "DENIED", "EXIST",
		"INVLD_OBJ", "WR_ERR", "INVLD_DRIVE", "NT_EN",
		"NO_FS", "MKFS_ERR", "TIMEOUT", "LOCKED", "NT_CORE",
		"MNY_OPEN_FILES", "INVLD_PARMTR" };

DWORD get_fattime(void) {
	DWORD tmr;

	rtc_getlocal(&g_tmCurrTime);
	/* Pack date and time into a DWORD variable */
	tmr = (((DWORD) g_tmCurrTime.tm_year - 1980) << 25)
			| ((DWORD) g_tmCurrTime.tm_mon << 21)
			| ((DWORD) g_tmCurrTime.tm_mday << 16)
			| (WORD) (g_tmCurrTime.tm_hour << 11)
			| (WORD) (g_tmCurrTime.tm_min << 5)
			| (WORD) (g_tmCurrTime.tm_sec >> 1);
	return tmr;
}

char* get_YMD_String(struct tm* timeData) {
	static char g_szYMDString[16];

	g_szYMDString[0] = 0;
	if (timeData->tm_year < 1900 || timeData->tm_year > 3000) // Working for 1000 years?
		strcpy(g_szYMDString, "0000");
	else
		strcpy(g_szYMDString, itoa_nopadding(timeData->tm_year));

	strcat(g_szYMDString, itoa_pad(timeData->tm_mon));
	strcat(g_szYMDString, itoa_pad(timeData->tm_mday));
	return g_szYMDString;
}

char* get_date_string(struct tm* timeData, const char* dateSeperator,
		const char* dateTimeSeperator, const char* timeSeparator,
		uint8_t includeTZ) {
	static char g_szDateString[24]; // "YYYY-MM-DD HH:MM:SS IST"
        static char g_szDate_String[5];
        
	g_szDateString[0] = 0;
	if (timeData->tm_year < 1900 || timeData->tm_year > 3000) // Working for 1000 years?
		strcpy(g_szDate_String, "0000");
	else
		strcpy(g_szDate_String, itoa_nopadding(timeData->tm_year));

        sprintf(g_szDateString, "%s%s%d%s%d%s%d%s%d%s%d", g_szDate_String, dateSeperator, timeData->tm_mon, dateSeperator, timeData->tm_mday, 
                dateTimeSeperator, timeData->tm_hour, timeSeparator, timeData->tm_min, timeSeparator,  timeData->tm_sec);
//sachin        strcat(g_szDateString, dateSeperator);
//	strcat(g_szDateString, itoa_pad(timeData->tm_mon));
//	strcat(g_szDateString, dateSeperator);
//	strcat(g_szDateString, itoa_pad(timeData->tm_mday));
//	strcat(g_szDateString, dateTimeSeperator);
//	strcat(g_szDateString, itoa_pad(timeData->tm_hour));
//	strcat(g_szDateString, timeSeparator);
//	strcat(g_szDateString, itoa_pad(timeData->tm_min));
//	strcat(g_szDateString, timeSeparator);
//	strcat(g_szDateString, itoa_pad(timeData->tm_sec));

	//[TODO] Check daylight saving time it doesnt work?

	/*
	 if (includeTZ && timeData->tm_isdst) {
	 strcat(g_szDateString, " DST");
	 }
	 */
	return g_szDateString;
}

// FORMAT IN FORMAT [YYYYMMDD:HHMMSS] Used for SMS timestamp
char* get_simplified_date_string(struct tm* timeData) {
	if (timeData == NULL)
		timeData = &g_tmCurrTime;

	return get_date_string(timeData, "", ":", "", 0);
}

void parse_time_from_line(struct tm* timeToConstruct, char* formattedLine) {
	char dateAttribute[5];
	char* token = NULL;
        int time_val[5],i,j;
	token = strtok(formattedLine, ",");

	if (token != NULL) {
          
          strncpy(dateAttribute, &token[4], 4);
		dateAttribute[4] = 0;
		timeToConstruct->tm_year = atoi(&dateAttribute[0]);
          
                for (i=8, j=0; i<=16; i=i+2, j++) {
                  strncpy(dateAttribute, &token[i], 2);
                  dateAttribute[2] = 0;
                  time_val[j] = atoi(&dateAttribute[0]);
          }
          timeToConstruct->tm_mon = time_val[0];
          timeToConstruct->tm_mday = time_val[1];
          timeToConstruct->tm_hour = time_val[2];
           timeToConstruct->tm_min = time_val[3];
          timeToConstruct->tm_sec = time_val[4];
          
//		strncpy(dateAttribute, &token[4], 4);
//		dateAttribute[4] = 0;
//		timeToConstruct->tm_year = atoi(&dateAttribute[0]);
//
//		strncpy(dateAttribute, &token[8], 2);
//		dateAttribute[2] = 0;
//		timeToConstruct->tm_mon = atoi(&dateAttribute[0]);
//
//		strncpy(dateAttribute, &token[10], 2);
//		dateAttribute[2] = 0;
//		timeToConstruct->tm_mday = atoi(&dateAttribute[0]);
//
//		strncpy(dateAttribute, &token[12], 2);
//		dateAttribute[2] = 0;
//		timeToConstruct->tm_hour = atoi(&dateAttribute[0]);
//
//		strncpy(dateAttribute, &token[14], 2);
//		dateAttribute[2] = 0;
//		timeToConstruct->tm_min = atoi(&dateAttribute[0]);
//
//		strncpy(dateAttribute, &token[16], 2);
//		dateAttribute[2] = 0;
//		timeToConstruct->tm_sec = atoi(&dateAttribute[0]);
	}
}

// {"vId":"nexleaf","data":[{"sim":0,"dId":"C","trId":"358072044878590","tmps":[{"sId":"A""time":3651163377,"typ":0,"tmp":27.1} ]}]}
void parse_sms_format_from_line(char* sensor, char* temp, char* time, char *type, char* line) {
	char* loc;
	char* locEnd;

	if(temp == NULL || line == NULL) return;

	loc = strstr(line,"\"sId\":");
	if(loc != NULL) {
		loc += 7;
		*sensor = *loc;
	} else
		*sensor = 0;
	
	loc = strstr(line,"\"typ\":");
	if(loc != NULL) {
		loc += 6;
		*type = *loc;
	} else
//		*type = 0;

#ifndef _EVIN_BUILD_
    *type = '0';
#else
    *type = 0;
#endif
	loc = strstr(line,"\"tmp\":");
	if(loc != NULL) {
		loc += 6;
		memcpy(temp, loc, 4);
		temp[4] = 0; //null term
	} else
		temp[0] = 0; //null term

#ifndef _EVIN_BUILD_
    if(*sensor == 'P') {
        loc = strstr(line,"\"pwr\":");
        if(loc != NULL) {
            loc += 6;
            *type = *loc;
        } else
            *type = 0;

        loc = strstr(line,"\"batt\":");
        if(loc != NULL) {
            loc += 7;
            memcpy(temp, loc, 2);
            temp[2] = 0; //null term
        } else
            temp[0] = 0; //null term
    }
#endif

	loc = strstr(line,"\"time\":");
	if(loc != NULL) {
		loc += 7;
		locEnd = strstr(loc,",");
		if(locEnd != NULL) {
			memcpy(time, loc, locEnd - loc);
			time[locEnd - loc] = 0;
			return;
		}
	}

	time[0] = 0; //timestamp missing

}

void offset_timestamp(struct tm* dateToOffset, int intervalMultiplier) {
	int timeVal;

	if (dateToOffset == NULL)
		return;

	timeVal = dateToOffset->tm_min
			+ (intervalMultiplier * g_pDevCfg->sIntervalsMins.sampling);
	if (timeVal >= 60) {
		while (timeVal >= 60) {
			timeVal -= 60;
			dateToOffset->tm_min = timeVal;
			dateToOffset->tm_hour += 1;
		}
	} else {
		dateToOffset->tm_min = timeVal;
	}
}

// Takes a string with the same format that is stored in file $TS=TIMESTAMP,INTERVAL,
static int date_within_interval(struct tm* timeToCompare, struct tm* baseTime,
		uint16_t interval) {
//	struct tm baseInterval;
//	int timeVal;
	unsigned long base = mktime(baseTime), compare = mktime(timeToCompare);

	if (timeToCompare == NULL || baseTime == NULL) return 0;

	if(compare > (base + ((uint32_t)interval * 60))) return 0;

	return 1;
	// Generically this needs to work for 31/12/xxxx 23:59:59 + 1 second
	// (which is 01/01/(xxxx+1) 00:00:00 - In our case a different day = different file
	// so it only needs to be accurate up to hours.
	// Leniancy ~+-1 minute (not guaranteed to be 60 seconds but min = 60 and max = 119)
//	memcpy(&baseInterval, baseTime, sizeof(struct tm));
//	timeVal = baseTime->tm_min + interval;
//	if (timeVal >= 60) {
//		while (timeVal >= 60) {
//			timeVal -= 60;
//			baseInterval.tm_min = timeVal;
//			baseInterval.tm_hour = baseTime->tm_hour + 1;
//		}
//	} else {
//		baseInterval.tm_min = timeVal;
//	}
//
//	if (timeToCompare->tm_hour != baseInterval.tm_hour) {
//		return 0;
//	}
//
//	if ((timeToCompare->tm_min <= baseInterval.tm_min + 1)
//			&& (timeToCompare->tm_min >= baseInterval.tm_min - 1)) {
//		return 1;
//	}

	//return 0;
}

char* get_current_fileName(struct tm* timeData, const char *folder,
		const char *ext) {
	if (timeData->tm_mday == 0 && timeData->tm_mon && timeData->tm_year == 0) {
		strcpy(g_szFatFileName, LOG_FILE_UNKNOWN);
		return g_szFatFileName;
	}

	sprintf(g_szFatFileName, "%s/%s.%s", folder, get_YMD_String(timeData), ext);
	return g_szFatFileName;
}

void fat_check_error(FRESULT fr) {
	if (fr == FR_OK)
		return;

	if (fr == FR_DISK_ERR || fr == FR_NOT_READY)
		g_bFatInitialized = false;

	g_szLastSD_CardError = FR_ERRORS[fr];
        g_pSysState->state.alarms.SD_card_failure = STATE_ON;//instead of fucntion we call below two statements
	g_pSysState->system.switches.send_SD_alarm = true;
//sachin	state_SD_card_problem(fr, FR_ERRORS[fr]);

//	lcd_turn_on();
//sachin	lcd_printl(LINEC, SDCARD_FAILED);
//	lcd_printf(LINEH, "SD FAIL:%s", FR_ERRORS[fr]);
        lcd_printf(LINEC, "SD_FAIL:%s", FR_ERRORS[fr]);
}

FRESULT fat_create_folders() {
	FRESULT fr;

        
	fr = f_mkdir(FOLDER_LOG);
	if (fr != FR_EXIST)
		fat_check_error(fr);

	fr = f_mkdir(FOLDER_TEXT);
	if (fr != FR_EXIST)
		fat_check_error(fr);

	fr = f_mkdir(FOLDER_DATA);
	if (fr != FR_EXIST)
		fat_check_error(fr);

        if(fr == FR_OK || fr == FR_EXIST) {
          g_bFatInitialized = true;
          g_pSysState->state.alarms.SD_card_failure = STATE_OFF;
        }
	// Remount fat, we have to check why is it not updating the structure in memory
	fr = f_mount(&FatFs, "", 0);
//	fat_check_error(fr);
//	if (fr != FR_OK)
//		return fr;

	return fr;
}

FRESULT fat_init_drive() {
	FRESULT fr;
	FILINFO fno;

	g_szLastSD_CardError = NULL;
	/* Register work area to the default drive */
	fr = f_mount(&FatFs, "", 0);
	fat_check_error(fr);
	if (fr != FR_OK)
		return fr;

//	fr_chk = f_stat(FOLDER_DATA, &fno);
//        if (fr_chk == FR_NO_FILE)
		fr = fat_create_folders();
//	fr_chk = f_stat(FOLDER_LOG, &fno);
//        if(fr_chk == FR_NO_FILE)
//		fr = fat_create_folders();
//	fr_chk = f_stat(FOLDER_TEXT, &fno);
//        if(fr_chk == FR_NO_FILE)
//		fr = fat_create_folders();

	fat_check_error(fr);
//sachin	if (fr != FR_OK)
//sachin		return fr;

	// Fat is ready
//sachin	g_bFatInitialized = true;
//sachin       g_pSysState->state.alarms.SD_card_failure = STATE_OFF;//instead of below fucntion call we can add this line
//	state_SD_card_OK();

        // clear out any opened/bad state
        fat_close();
        
	// Delete old config files
	f_unlink(LOG_MODEM_PATH);
	f_unlink(CONFIG_LOG_FILE_PATH);

	//check system log and make sure it is less the 64kB
	fr = f_stat(LOG_FILE_PATH, &fno);
	if(fr == FR_OK && fno.fsize > 65536) {
		f_unlink(LOG_FILE_PATH);
	}
	return fr;
}

FRESULT fat_save_config(char *text) {
	return log_append_(LOG_TYPE_CONFIG, text);
}


FRESULT log_append_(LOG_TYPE type, const char *text) {
	FRESULT fr;
	FIL *fobj;
	int len = 0;
	int bw = 0;
	char *szLog;
	char szPath[20];
	int t = 0;


	if (g_bLogDisabled || !g_bFatInitialized)
		return FR_NOT_READY;

	if (text == NULL || strlen(text) == 0)
		return FR_OK;

	switch(type)
	{
		case LOG_TYPE_SYSTEM:
		{
			if (!g_pDevCfg->cfg.logs.system_log) return FR_OK;
			strcpy(szPath, LOG_FILE_PATH);
		} break;
		case LOG_TYPE_MODEM:
		{
			if (!g_pDevCfg->cfg.logs.modem_transactions) return FR_OK;
			strcpy(szPath, LOG_MODEM_PATH);
		} break;
		case LOG_TYPE_CONFIG:
		{
			if (!g_pDevCfg->cfg.logs.server_config) return FR_OK;
			strcpy(szPath, CONFIG_LOG_FILE_PATH);
		} break;
		default:
		{
			return FR_OK; //unknown log type
		}

	}

	fr = fat_open(&fobj, szPath, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);

	if (fr == FR_OK) {
		if (fobj->fsize) {
			//append to the file
			f_lseek(fobj, fobj->fsize);
		}
	} else {
		fat_check_error(fr);
		return fr;
	}
      
	if(type == LOG_TYPE_SYSTEM) {
		rtc_getlocal(&g_tmCurrTime);
		szLog = getStringBufferHelper(NULL);
		strcpy(szLog, "[");
		if (g_tmCurrTime.tm_year > 2000) {
			strcat(szLog, get_date_string(&g_tmCurrTime, "", " ", "", 0));
		} else {
			for (t = 0; t < 15; t++)
				strcat(szLog, "*");
		}
		strcat(szLog, "] ");

		len = strlen(szLog);
		fr = f_write(fobj, szLog, len, (UINT *) &bw);
		releaseStringBufferHelper();
		if (fr != FR_OK || bw != len) {
                  goto ret;
//                        fat_close();
//			return fr;
		}
	}

	len = strlen(text);
	fr = f_write(fobj, text, len, (UINT *) &bw);
	if(fr != FR_OK) {    
           goto ret;
//		fat_close();
//		return fr;
	}

  
	//make sure we end with a new line
	if(strcmp(text + len - 2, "\r\n") != 0) {
		fr = f_write(fobj, "\r\n", 2, (UINT *) &bw);
		if(fr != FR_OK) {
                   goto ret;
//			fat_close();
//			return fr;
		}
	}
        
ret:      return fat_close();
} 

FRESULT log_modem(const char *text) {
	return log_append_(LOG_TYPE_MODEM, text);
}

void log_disable() {
	g_bLogDisabled = true;
}

void log_enable() {
	g_bLogDisabled = false;
}

void log_reboot(char *mess)
{
    // Set Counter Clear bit
  char message[25];
    uint16_t value = HWREG16(0x00180 + 0x1E);
  sprintf(message, "s=%x:", value);
    
    log_enable();
	log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_REBOOT), strcat(message, mess));
}

// This function is called with the stack really full. We get this array from the stack for it to not create strange behaviour
FRESULT log_appendf(LOG_TYPE type, const char *_format, ...) {
	va_list _ap;
	char szTemp[40];

	
#ifdef _DEBUG
	//checkStack();
#endif

	va_start(_ap, _format);
	vsnprintf(szTemp, sizeof(szTemp), _format, _ap);
	va_end(_ap);
	if (g_bLogDisabled)
		return FR_NOT_READY;

	return log_append_(type, szTemp);
}

#ifndef _DEBUG
const char HEADER_CSV[] = "\"Date\",\"Batt\",\"Power\","
		"\"Sensor A\",\"Sensor B\",\"Sensor C\",\"Sensor D\",\"Sensor E\","
		"\"Signal\",\"Net\"\r\n";
#else
const char HEADER_CSV[] = "D,B,P,A,B,C,D,E,S,N\r\n";
#endif

FRESULT log_write_header(FIL *fobj, UINT *pBw) {
	return f_write(fobj, HEADER_CSV, sizeof(HEADER_CSV) - 1, pBw);
}

//const char * const POWER_STATE[] = { "Power Outage", "Power Available" };

// 0 Power outage
//  Power available
const char *getPowerStateString() {
	return get_string("PWR", POWER_ON);
}

FRESULT log_write_temperature(FIL *fobj, UINT *pBw) {
	char *szLog;
	UINT bw = 0;
	char *date;
	FRESULT fr;
	int8_t iBatteryLevel;
	int8_t iSignalLevel;
	char *network_state;

	date = get_date_string(&g_tmCurrTime, "-", " ", ":", 1);
	fr = f_write(fobj, date, strlen(date), &bw);
	if (fr != FR_OK)
		return fr;

	*pBw += bw;

	iBatteryLevel = batt_getlevel();
	iSignalLevel = state_getSignalPercentage();
	network_state = state_getNetworkState();
      
	szLog = getStringBufferHelper(NULL);
	sprintf(szLog, ",\"%d%%\",\"%s\",%s,%s,%s,%s,%s,%d,%s\r\n",
			(int) iBatteryLevel, getPowerStateString(),
			temperature_getString(0), temperature_getString(1),
			temperature_getString(2), temperature_getString(3),
			temperature_getString(4), (int) iSignalLevel, network_state);
	fr = f_write(fobj, szLog, strlen(szLog), &bw);
	releaseStringBufferHelper();
	return fr;
}

FRESULT log_sample_web_format(UINT *tbw) {
	FIL *fobj;
	UINT bw = 0;	//bytes written
	FRESULT fr;

	if (!g_pDevCfg->cfg.logs.web_csv)
		return FR_OK;

	if (!g_bFatInitialized)
		return FR_NOT_READY;

	/*
#ifdef _DEBUG
	lcd_print(get_string(LCD_MSG, SAVING_SAMPLE));
#else
*/
//sachin	lcd_print_progress();
//#endif

	rtc_getlocal(&g_tmCurrTime);
	char* fn = get_current_fileName(&g_tmCurrTime, FOLDER_DATA, EXTENSION_DATA);

        fr = fat_open(&fobj, fn, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
        if (fr == FR_OK) {
            if (fobj->fsize) {
                //append to the file
                f_lseek(fobj, fobj->fsize);
            } else {
                fr = log_write_header(fobj, &bw);
               if (fr != FR_OK) {
//sachin                    fat_check_error(fr);
//                    fat_close();
//                    return fr;
                 goto ret;
                }
            }     
        } else {
          goto ret;
// sachin           fat_check_error(fr);
//            return fr;
        }
        
        fr = log_write_temperature(fobj, &bw);
        *tbw = bw;


//sachin	if (fr == FR_OK) {
////		/*
////#ifdef _DEBUG
////		lcd_printf(LINE2, "OK %d", *tbw);
////#else
////		event_force_event_by_id(EVT_DISPLAY, 0);
////#endif
////*/
//		_NOP(); //nothing
//	} else {
//		fat_check_error(fr);
//	}
        if (fr != FR_OK)
ret:         fat_check_error(fr);
        
	return fat_close();
}

UINT g_logToDisk_bw = 0;

int json_printer_callback_ff(void *userdata, const char *s, uint32_t length) {
	FRESULT fr = FR_OK;
	UINT bw = 0;

	fr = f_write(userdata, s, length, (UINT *) &bw);
	if (bw > 0) {
		g_logToDisk_bw += bw;
	}

	return (int)fr;
}
/*
FRESULT log_sample_to_disk(UINT *tbw) {
	FIL *fobj;
	struct tm tempDate;
	char *szLog = NULL;
//	int iBatteryLevel = 0;
	char* fn;
	FRESULT fr = FR_OK;
	EVENT *evt;
	uint16_t iUploadPeriod = 0;
	int i = 0; //loop var
	int bw = 0;	//bytes written
	uint8_t fileOpened = 0;
    static struct tm lastTimeStamp;

	//json stack variable
	json_printer print;
	g_logToDisk_bw = 0; //used by libjson callback to track bytes written

	if (!g_bFatInitialized)
		return FR_NOT_READY;
        
//        delay(2000);
	//get time in both timer_t and tm struct formats
        if(g_pSysState->time_flag != 1) {
            g_pSysState->last_sample_time = rtc_getUTC(&g_tmCurrTime, &tempDate);
        }
       //	if( g_tmCurrTime.tm_year <= 2015 || g_tmCurrTime.tm_year >= 2025)
//		return 0;

	// If current time is out of previous upload interval, log a new time stamp
	evt = events_find(EVT_UPLOAD_SAMPLES);
	iUploadPeriod = (uint16_t)event_getIntervalMinutes(evt);

	if (!date_within_interval(&tempDate, &lastTimeStamp, iUploadPeriod)) {
		g_pSysState->system.switches.timestamp_on = 1;
		memcpy(&lastTimeStamp, &tempDate, sizeof(struct tm));
	}

	fn = get_current_fileName(&g_tmCurrTime, FOLDER_TEXT, EXTENSION_TEXT);

	SENSOR_STATUS *sensorState;
	for(i = 0; i < SYSTEM_NUM_SENSORS; i++) {
		sensorState = getAlarmsSensor(i);
		if(sensorState->state.disconnected || sensorState->state.disabled) continue;
		if(!strcmp(temperature_getString(i),"----")) continue;

		if(fileOpened == 0) {
			fr = fat_open(&fobj, fn, FA_READ | FA_WRITE | FA_OPEN_ALWAYS); //currently only supports 8.3 OEM character filenames
			if (fr == FR_OK) {
				if (fobj->fsize) {
					//append to the file
					f_lseek(fobj, fobj->fsize);
				} else {
					g_pSysState->system.switches.timestamp_on = 1;
					memcpy(&lastTimeStamp, &tempDate, sizeof(struct tm));
				}
				fileOpened = 1;
			} else {
				fat_check_error(fr);
				return fr;
			}
		}

		// Log time stamp  as necessary - only if samples are being recorded
		if (g_pSysState->system.switches.timestamp_on) {
			szLog = getStringBufferHelper(NULL);
                        if( g_pSysState->time_flag == 1) {
                        rtc_getUTC(&g_tmCurrTime, &tempDate);
                        }
 //                   sprintf(szLog, "$TS=\"%d\"\"%d\"\"%d\"\"%d\"\"%d\"\"%d\",\n",tempDate.tm_year + 1900,tempDate.tm_mon + 1, tempDate.tm_mday, tempDate.tm_hour,
   //                             tempDate.tm_min, tempDate.tm_sec);
            sprintf(szLog, "$TS=%d%d%d%d%d%d,",tempDate.tm_year + 1900,tempDate.tm_mon + 1, tempDate.tm_mday, tempDate.tm_hour,
                                tempDate.tm_min, tempDate.tm_sec);
//			 strcat(szLog, "$TS=");
//			strcat(szLog, itoa_pad((tempDate.tm_year + 1900)));
//			strcat(szLog, itoa_pad(tempDate.tm_mon + 1));
//			strcat(szLog, itoa_pad(tempDate.tm_mday));
//			strcat(szLog, itoa_pad(tempDate.tm_hour));
//			strcat(szLog, itoa_pad(tempDate.tm_min));
//			strcat(szLog, itoa_pad(tempDate.tm_sec));
//			strcat(szLog, ",");
			strcat(szLog, "\n");
			fr = f_write(fobj, szLog, strlen(szLog), (UINT *) &bw);

			if (bw > 0) {
				*tbw += bw;
			}
			releaseStringBufferHelper();
			g_pSysState->system.switches.timestamp_on = 0;
		}

		//each line is a fully formed 'tmps' entry so we start a fresh printer each time
		json_print_init(&print, json_printer_callback_ff, fobj);
		json_print_temp_obj(&print, i, g_pSysState->last_sample_time);

		fr = f_write(fobj, "\n", 1, (UINT *) &bw);
		if (bw > 0) {
			*tbw += bw;
		}
	}
    
        //for nexleaf config
//#ifndef _EVIN_BUILD_
//    if(fileOpened == 0) {
//        fr = fat_open(&fobj, fn, FA_READ | FA_WRITE | FA_OPEN_ALWAYS); //currently only supports 8.3 OEM character filenames
//        if (fr == FR_OK) {
//            if (fobj->fsize) {
//                //append to the file
//                f_lseek(fobj, fobj->fsize);
//            } else {
//                g_pSysState->system.switches.timestamp_on = 1;
//                memcpy(&lastTimeStamp, &tempDate, sizeof(struct tm));
//            }
//            fileOpened = 1;
//        } else {
//            fat_check_error(fr);
//            return fr;
//        }
//    }
//
//    //each line is a fully formed 'tmps' entry so we start a fresh printer each time
//    json_print_init(&print, json_printer_callback_ff, fobj);
//    json_print_pwrinfo_obj(&print, timeUTC);
//    
//    fr = f_write(fobj, "\n", 1, (UINT *) &bw);
//    if (bw > 0) {
//        *tbw += bw;
//    }
//#endif
        
#ifndef _EVIN_BUILD_
        f_lseek(fobj, fobj->fsize);
        json_print_init(&print, json_printer_callback_ff, fobj);
        json_print_pwrinfo_obj(&print, g_pSysState->last_sample_time);
    
        fr = f_write(fobj, "\n", 1, (UINT *) &bw);
        if (bw > 0) {
                *tbw += bw;
        }

#endif

	if (g_logToDisk_bw > 0) {
		*tbw += g_logToDisk_bw;
	}

	return fat_close();

}
*/
FRESULT fat_create_dev_ready(UINT *tbw) {
	FRESULT fr;
	FIL *fobj;
	//char buffer[17];
	//json stack variable
	json_printer print;
	g_logToDisk_bw = 0; //used by libjson callback to track bytes written

	if (!g_bFatInitialized)
		return FR_NOT_READY;

	fr = fat_open(&fobj, (char *)g_pDevReadyFN, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if (fr != FR_OK) {
		fat_check_error(fr);
		return fr;
    }

	json_print_init(&print, json_printer_callback_ff, fobj);

	json_print_api_header(&print, 0);

	json_print_data_header(&print);

	if(g_pDevCfg->SIM[0].simOperational) {
		json_print_simDevRdy_obj(&print, 0);
	}
	//only include alt sim if one is available
	if(g_pDevCfg->SIM[1].simOperational) {
		json_print_simDevRdy_obj(&print, 1);
	}
	json_print_devDevRdy_obj(&print);

	json_print_actSnsDevRdy_obj(&print);
    
	json_print_asm_pair(&print);

	json_print_data_footer(&print);

	json_print_api_footer(&print);

	if (g_logToDisk_bw > 0) {
		*tbw += g_logToDisk_bw;
	}

	return fat_close();
}

FRESULT fat_create_dev_alarm(uint8_t alarm, UINT *tbw) {
	FRESULT fr;
	FIL *fobj;
	int sensor;
	//json stack variable
	json_printer print;
	g_logToDisk_bw = 0; //used by libjson callback to track bytes written

	if (!g_bFatInitialized)
		return FR_NOT_READY;

	//get time in both timer_t and tm struct formats
	time_t timeUTC = rtc_getUTCsecs();

	fr = fat_open(&fobj, (char *)g_pDevAlarmFN, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if (fr != FR_OK) {
		fat_check_error(fr);
		return fr;
    }

	json_print_init(&print, json_printer_callback_ff, fobj);

	//Begin Device Alarm
	json_print_api_header(&print, 0);

	for(sensor = 0; sensor < SYSTEM_NUM_SENSORS; sensor++) { //loop over sensors
		SENSOR_STATUS *s = getAlarmsSensor(sensor);

                // skip loop if disabled
                if (s->state.disabled == true) continue;
                
	  	//skip loop for specific alarm checks - like only sending a changed state
	  	if(alarm == DEV_ALARM_CONN && s->state.connStateChange == false) continue;
	  
	  
		json_print_data_header(&print);

		//For sensor specific alarms include sId
		if(alarm & (DEV_ALARM_CONN | DEV_ALARM_SENSOR)) {
			json_print_sId_pair(&print, sensor);
		} //End sId

		//Begin dvc object
		if (alarm & (DEV_ALARM_CONN | DEV_ALARM_SENSOR | DEV_ALARM_BATTERY )) {
			json_print_dvcAlarm_header(&print);

			if (alarm & DEV_ALARM_CONN) {
				json_print_extSensAlarm_obj(&print, sensor, timeUTC);
				if(s->state.connStateChange) s->state.connStateChange = false;
			}

			if ((alarm & DEV_ALARM_BATTERY) && sensor == 0) {
				json_print_battAlarm_obj(&print, timeUTC);
			}
                        

			json_print_dvcAlarm_footer(&print);
		} //End dvc object

		//Begin errs array
		if ((alarm & DEV_ALARM_FW_ERR) && sensor == 0) {
                  json_print_fwErrorAlarm_arr(&print, timeUTC);
		}
                
		json_print_data_footer(&print);
		//if not sensor based break for loop
		if(!(alarm & (DEV_ALARM_CONN | DEV_ALARM_SENSOR))) break;
	}//sensor loop
	json_print_api_footer(&print);
	//End Device Alarm
	
	if (g_logToDisk_bw > 0) {
		*tbw += g_logToDisk_bw;
	}

	return fat_close();
}

FRESULT fat_create_dev_alarm_conn(UINT *tbw) {
	return fat_create_dev_alarm(DEV_ALARM_CONN, tbw);
}

FRESULT fat_create_dev_alarm_sens(UINT *tbw) {
	return fat_create_dev_alarm(DEV_ALARM_SENSOR, tbw);
}

FRESULT fat_create_dev_alarm_batt(UINT *tbw) {
	return fat_create_dev_alarm(DEV_ALARM_BATTERY, tbw);
}

//FRESULT fat_create_dev_alarm_fw_error(UINT *tbw) {
//	return fat_create_dev_alarm(DEV_ALARM_FW_ERR, tbw);
//}
