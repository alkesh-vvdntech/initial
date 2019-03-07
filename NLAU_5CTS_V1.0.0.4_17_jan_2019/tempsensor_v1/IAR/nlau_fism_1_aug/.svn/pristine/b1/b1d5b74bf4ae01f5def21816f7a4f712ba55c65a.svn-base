#ifndef _CT5_BOOTLOADER_
#include "thermalcanyon.h"
#include "minIni.h"

extern const uint32_t _LCD_VECTOR[11];

static void (**pf_lcd_clear) (void) = (void (**) (void)) (_LCD_VECTOR + 0);
static void (**pf_lcd_on) (void) = (void (**) (void)) (_LCD_VECTOR + 1);
static void (**pf_lcd_off) (void) = (void (**) (void)) (_LCD_VECTOR + 2);
static void (**pf_lcd_setaddr) (int8_t addr) = (void (**) (int8_t addr)) (_LCD_VECTOR + 3);
static void (**pf_lcd_print) (char* pcData) = (void (**) (char* pcData)) (_LCD_VECTOR + 4);
static void (**pf_lcd_printl) (int8_t iLine, const char* pcData) = (void (**) (int8_t iLine, const char* pcData)) (_LCD_VECTOR + 5);
static void (**pf_lcd_reset) (void) = (void (**) (void)) (_LCD_VECTOR + 6);
static void (**pf_lcd_blenable) (void) = (void (**) (void)) (_LCD_VECTOR + 7);
static void (**pf_lcd_bldisable) (void) = (void (**) (void)) (_LCD_VECTOR + 8);
static void (**pf_lcd_print_progress) (void) = (void (**) (void)) (_LCD_VECTOR + 9);
static char (**pf_getLCD_state) (void) = (char (**) (void)) (_LCD_VECTOR + 10);

char getLCD_state(void) {
    return (**pf_getLCD_state)();
}

void lcd_clear() {
    (**pf_lcd_clear)();
}

void lcd_on() {
    (**pf_lcd_on)();
}

void lcd_off() {
    (**pf_lcd_off)();
}

void lcd_setaddr(int8_t addr) {
    (**pf_lcd_setaddr)(addr);
}

void lcd_print(char* pcData) {
    (**pf_lcd_print)(pcData);
}

void lcd_printl(int8_t iLine, const char* pcData) {
    (**pf_lcd_printl)(iLine, pcData);
}

void lcd_reset() {
    (**pf_lcd_reset)();
}

void lcd_blenable() {
    (**pf_lcd_blenable)();
}

void lcd_bldisable() {
    (**pf_lcd_bldisable)();
}

void lcd_print_progress() {
    (**pf_lcd_print_progress)();
}

char* get_string(char *section, uint8_t Key){
	static char g_strLCDMsg[30];
	ini_gets(section, itoa_nopadding(Key),"", g_strLCDMsg, sizeof(g_strLCDMsg), STRING_INI_FILE);
	return g_strLCDMsg;
}

void lcd_display_config_sensor(int id) {
	TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[id];
	lcd_printf(LINEC, "%s Cold %dm %d", SensorName[id], (int) pAlertParams->maxSecondsCold / 60, (int) pAlertParams->threshCold);
	lcd_printf(LINE2, "Hot %dm %d", (int) pAlertParams->maxSecondsHot / 60, (int) pAlertParams->threshHot);
}

void lcd_display_config() {
	TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[0];
	char num1[TEMP_DATA_LEN + 1];
	char num2[TEMP_DATA_LEN + 1];

	getFloatNumber2Text(pAlertParams->threshCold, num1);
	getFloatNumber2Text(pAlertParams->threshHot, num2);

	/*
#ifdef _DEBUG
	checkStack();
	lcd_printf(LINEC, "%x %d %s %d %s", g_pSysCfg->stackLeft, (int) pAlertParams->maxSecondsCold / 60,
			&num1[0], (int) pAlertParams->maxSecondsHot / 60, &num2[0]);
#else
*/
	lcd_printf(LINEC, "C%d %s H%d %s", (int) pAlertParams->maxSecondsCold / 60,
			&num1[0], (int) pAlertParams->maxSecondsHot / 60, &num2[0]);
//#endif
	lcd_printf(LINE2, "S%d U%d L%d P%d", g_pDevCfg->cfgSelectedSIM_slot + 1,
			g_pDevCfg->sIntervalsMins.upload,
			g_pDevCfg->sIntervalsMins.sampling,
			g_pDevCfg->stBattPowerAlertParam.minutesPower);
}

void lcd_turn_on() {
	lcd_blenable(); //enable backlight
	lcd_on(); 		//lcd reset
}

void lcd_turn_off() {
	lcd_bldisable();
    if(!g_pDevCfg->cfg.logs.text_persist) lcd_off();
}

void lcd_append_signal_info(char *lcdBuffer) {

  if(state_isSignalInRange()) {
		strcat(lcdBuffer, itoa_pad(state_getSignalPercentage()));
		strcat(lcdBuffer, "% ");
		if (state_isGPRS() == 1) {
			strcat(lcdBuffer, "G:YES     ");
		} else {
			strcat(lcdBuffer, "G:NO      ");
		}
	} else {
		strcat(lcdBuffer, " No signal ");
	}
}

void lcd_setDate(char *buffer) {
  int count =0;
	rtc_getlocal(&g_tmCurrTime);
        int tim[] = {g_tmCurrTime.tm_year, g_tmCurrTime.tm_mon, g_tmCurrTime.tm_mday, g_tmCurrTime.tm_hour, g_tmCurrTime.tm_min};
        char st[5][2]={"/","/"," ",":","\0"};
        
//        if (g_tmCurrTime.tm_year < 100) {
//            strcat(buffer, "00"); //g_tmCurrTime.tm_year += 1900;
//        } else if (g_tmCurrTime.tm_year < 1000) {
//            strcat(buffer, "0"); //g_tmCurrTime.tm_year += 1900;
//        }
//        sprintf(buffer, "%d/%d/%d %d:%d    ",g_tmCurrTime.tm_year, g_tmCurrTime.tm_mon, g_tmCurrTime.tm_mday, g_tmCurrTime.tm_hour, 
//          g_tmCurrTime.tm_min);
        
        while(count != 5) {
        strcat(buffer, itoa_pad(tim[count]));
        strcat(buffer, st[count]);
        count++;
        }
        
/*	strcat(buffer, itoa_pad(g_tmCurrTime.tm_year));
	strcat(buffer, "/");
	strcat(buffer, itoa_pad(g_tmCurrTime.tm_mon));
	strcat(buffer, "/");
	strcat(buffer, itoa_pad(g_tmCurrTime.tm_mday));
	strcat(buffer, " ");

	strcat(buffer, itoa_pad(g_tmCurrTime.tm_hour));
	strcat(buffer, ":");
	strcat(buffer, itoa_pad(g_tmCurrTime.tm_min));
*/
}

void lcd_setCustomDate(char *buffer, time_t time) {
	struct tm *p_tmStruct = localtime(&time);
    p_tmStruct->tm_year += 1900;
    p_tmStruct->tm_mon++;
    sprintf(buffer, "%d/%d/%d %d:%d",p_tmStruct->tm_year, p_tmStruct->tm_mon, p_tmStruct->tm_mday, p_tmStruct->tm_hour, 
          p_tmStruct->tm_min);
/*	strcat(buffer, itoa_pad(p_tmStruct->tm_year));
	strcat(buffer, "/");
	strcat(buffer, itoa_pad(p_tmStruct->tm_mon));
	strcat(buffer, "/");
	strcat(buffer, itoa_pad(p_tmStruct->tm_mday));
	strcat(buffer, " ");

	strcat(buffer, itoa_pad(p_tmStruct->tm_hour));
	strcat(buffer, ":");
	strcat(buffer, itoa_pad(p_tmStruct->tm_min));
*/
}

void lcd_setUptime(char *buffer) {
//  sprintf(buffer, "[%d:%u:%u] %d:%d",(iMinuteTick/60), (iMinuteTick%60), (iSecondTick%60), g_tmCurrTime.tm_hour, 
//          g_tmCurrTime.tm_min);
#ifndef _FISM_
        strcat(buffer, "[");
	strcat(buffer, itoa_pad(iMinuteTick/60));
	strcat(buffer, ":");
	strcat(buffer, itoa_pad(iMinuteTick%60));
	strcat(buffer, ":");   
        strcat(buffer, itoa_pad(iSecondTick%60));      
	
	strcat(buffer, "]");
	strcat(buffer, " ");
	strcat(buffer, itoa_pad(g_tmCurrTime.tm_hour));
	strcat(buffer, ":");
	strcat(buffer, itoa_pad(g_tmCurrTime.tm_min));
#endif
}

uint8_t lcd_show() {

	static int8_t iItemId = -1;
	static time_t lastRefresh = 0;

	char lcdBuffer[LCD_DISPLAY_LEN + 1];
	int iIdx = 0;
	int iCnt = 0;

	// LCD is off
	if (getLCD_state() == 0)
		return 0;

	if (iItemId == g_iDisplayId &&
		lastRefresh == rtc_get_second_tick())
		return 0;

	iItemId = g_iDisplayId;
	lastRefresh = rtc_get_second_tick();

	lcd_clear();

	memset(lcdBuffer, 0, sizeof(lcdBuffer));

	/*
#ifdef _DEBUG
	lcd_setUptime(lcdBuffer);
#else
	lcd_setDate(lcdBuffer);
#endif
*/
	lcd_setDate(lcdBuffer);
        
	//get local time
	iIdx = strlen(lcdBuffer); //marker

	if (iItemId > 0 && iItemId <= 5) {
		iCnt = iItemId - 1;
 		if ((g_pSysState->temp.state[iCnt].status & 0x20)) {
			g_iDisplayId += 1;
			return 1;
		}
	}

       
	switch (iItemId) {
	case 0:
                if (g_pSysState->temp.state[1].status & 0x08) {
                  strcat(lcdBuffer, "---");
                } else {
                  strcat(lcdBuffer, temperature_getString(1));
                }
              strcat(lcdBuffer, "C ");
		strcat(lcdBuffer, itoa_pad(batt_getlevel()));
		strcat(lcdBuffer, "% ");
		if (state_isSignalInRange()) {
			if (modem_getNetworkService() == NETWORK_GPRS) {
				strcat(lcdBuffer, "G");
			} else {
				strcat(lcdBuffer, "S");
			}
			strcat(lcdBuffer, itoa_pad(state_getSignalPercentage()));
			strcat(lcdBuffer, "%   ");
		} else {
			strcat(lcdBuffer, "S --");
		}
		iCnt = 0xff;
		break;

	case 6:
		iCnt = 0xff;
		strcat(lcdBuffer, itoa_pad(batt_getlevel()));
		strcat(lcdBuffer, "%             ");
//sachin		strcat(lcdBuffer, state_get_batt_string());

		break;
		// added for new display//
	case 7:
		iCnt = 0xff;
                strcat(lcdBuffer, "SIM1 ");
		if (config_getSelectedSIM() != 0)
			strcat(lcdBuffer, "  --           ");
		else
			lcd_append_signal_info(lcdBuffer);
		break;
	case 8:
		iCnt = 0xff;
		strcat(lcdBuffer, "SIM2 ");	//current sim slot is 2
		if (config_getSelectedSIM() != 1)
			strcat(lcdBuffer, "  --           ");
		else
			lcd_append_signal_info(lcdBuffer);
		break;
    case 9:
     {
		const SIM_ERROR_ENTRY *errSlot = modem_getLastErrorsCacheEntry();
        iCnt = 0xff;
		if (errSlot->errStr[0] == '\0') {
			strcat(lcdBuffer, "NO SIM ERRORS  ");
		} else {
//sachin          lcdBuffer[0] = '\0';
//sachin         lcd_setCustomDate(lcdBuffer, errSlot->timeStamp);
           strcat(lcdBuffer, "SIM");
            strcat(lcdBuffer, itoa_nopadding(errSlot->simId + 1));
            strcat(lcdBuffer, " ERR: ");
            strcat(lcdBuffer, errSlot->errStr);
        }
		break;
     }

	default:
		break;
	}
	if (iCnt != 0xff) {
                if (g_pSysState->temp.state[iCnt].status & 0x08) {
			sprintf(&lcdBuffer[iIdx], "Sensor %s DISCONN", SensorName[iCnt]);
		} else if (g_pSysState->temp.state[iCnt].status & 0x07) {
			sprintf(&lcdBuffer[iIdx], "ALERT %s %sC   ", SensorName[iCnt], temperature_getString(iCnt));
		} else {
			sprintf(&lcdBuffer[iIdx], "Sensor %s %sC  ", SensorName[iCnt], temperature_getString(iCnt));
		}
	}

	//display the lines
	i2c_write(SLAVE_ADDR_DISPLAY, 0x40, LCD_LINE_LEN, (uint8_t *) lcdBuffer);
	lcd_setaddr(0x40);	//go to next line
	i2c_write(SLAVE_ADDR_DISPLAY, 0x40, LCD_LINE_LEN, (uint8_t *) &lcdBuffer[iIdx]);
	return 0;
}

void lcd_progress_wait(uint16_t delayTime) {
	int t;
	int count = delayTime / 100;
	if (count <= 0) {
		lcd_print_progress();
		return;
	}
/* saving space
#ifdef _DEBUG
	checkStack();
#endif
*/
	for (t = 0; t < count; t++) {
		delay(50);
		lcd_print_progress();
	}
}

void lcd_printf(int line, const char *_format, ...) {
	va_list _ap;
	char szTemp[33];

	/* saving space
#ifdef _DEBUG
	checkStack();
#endif
*/

	va_start(_ap, _format);
	vsnprintf(szTemp, sizeof(szTemp), _format, _ap);
	va_end(_ap);

	lcd_printl(line, szTemp);
}


int g_iLCDVerbose = VERBOSE_DISABLED; // Disable debug

void lcd_disable_verbose() {
	g_iLCDVerbose = VERBOSE_DISABLED;
}

void lcd_setVerboseMode(int value) {
	g_iLCDVerbose = value;
}

int lcd_getVerboseMode() {
	return g_iLCDVerbose;
}

void lcd_enable_verbose() {
	g_iLCDVerbose = VERBOSE_BOOTING;
}

void lcd_print_boot(const char* pcData, int line) {
	if (g_iLCDVerbose == VERBOSE_DISABLED)
		return;

#ifdef _DEBUG
	if (getLCD_state() == 0)
	return;

	if (line==1)
	lcd_clear();

	lcd_printl(line, pcData);
#else

	lcd_print_progress();
#endif
}
#else
/******************************************************/
//      BOOTLOADER HELD FUNCTIONALITY
/******************************************************/
#include "msp430.h"
#include <string.h>
#include "lcd.h"
#include "config.h"
#include "i2c.h"
#include "timer.h"

char g_bLCD_state = 0;

char getLCD_state(void) {
    return g_bLCD_state;
}

void lcd_setupIO() {
	PJDIR |= BIT6 | BIT7;      			// set LCD reset and Backlight enable
	PJOUT |= BIT6;							// LCD reset pulled high
	PJOUT &= ~BIT7;							// Backlight disable
}

void lcd_init() {
	uint8_t lcdBuffer[16];

    lcd_blenable(); //enable backlight
	lcd_on(); 		//lcd reset

	memset(lcdBuffer, 0, LCD_INIT_PARAM_SIZE);

	lcdBuffer[0] = 0x38; // Basic
	lcdBuffer[1] = 0x39; // Extended
	lcdBuffer[2] = 0x14; // OCS frequency adjustment
	lcdBuffer[3] = 0x78;
	lcdBuffer[4] = 0x5E;
	lcdBuffer[5] = 0x6D;
	lcdBuffer[6] = 0x0C; // display on
	lcdBuffer[7] = 0x01; // clear display
	lcdBuffer[8] = 0x06; // entry mode set

	i2c_write(SLAVE_ADDR_DISPLAY, 0, LCD_INIT_PARAM_SIZE, lcdBuffer);

	lcdBuffer[0] = 0x40 | 0x80;
	i2c_write(SLAVE_ADDR_DISPLAY, 0, 1, lcdBuffer);

#if TESTING_LCD
	// DISPLAY AB CD just for testing
	lcdBuffer[0] = 0x43;
	lcdBuffer[1] = 0x44;
	i2c_write(SLAVE_ADDR_DISPLAY, 0x40, 2, lcdBuffer);

	lcdBuffer[0] = 0x41;
	lcdBuffer[1] = 0x42;
	i2c_write(SLAVE_ADDR_DISPLAY,0x40,2,lcdBuffer);
#endif
	delay(1000);
}

void lcd_clear() {
	uint8_t lcdBuffer;
	lcdBuffer = 0x01;
	i2c_write(SLAVE_ADDR_DISPLAY, 0, 1, (uint8_t *) &lcdBuffer);
}

void lcd_on() {
	uint8_t lcdBuffer;
	lcdBuffer = 0x0C;
	i2c_write(SLAVE_ADDR_DISPLAY, 0, 1, (uint8_t *) &lcdBuffer);
	g_bLCD_state = 1;
}

void lcd_off() {
	uint8_t lcdBuffer = 0x08;
	i2c_write(SLAVE_ADDR_DISPLAY, 0, 1, (uint8_t *) &lcdBuffer);
	g_bLCD_state = 0;
}

void lcd_setaddr(int8_t addr) {
	uint8_t lcdBuffer = addr | 0x80;
	i2c_write(SLAVE_ADDR_DISPLAY, 0, 1, (uint8_t *) &lcdBuffer);
}

void lcd_print(char* pcData) {
	size_t len = strlen(pcData);

	if (g_bLCD_state == 0)
		return;

	if (len > LCD_LINE_LEN) {
		len = LCD_LINE_LEN;
	}
	lcd_clear();
	i2c_write(SLAVE_ADDR_DISPLAY, 0x40, len, (uint8_t *) pcData);
}

void lcd_printl(int8_t iLine, const char* pcData) {
	size_t len = strlen(pcData);

	if (g_bLCD_state == 0)
		return;

	if (iLine == LINEC)
		lcd_clear();

	if (len > LCD_LINE_LEN)
		len = LCD_LINE_LEN;

	if (iLine == LINE2 || iLine == LINEH || iLine == LINEE)
		lcd_setaddr(0x40);
	else
		lcd_setaddr(0x0);

	i2c_write(SLAVE_ADDR_DISPLAY, 0x40, len, (uint8_t *) pcData);

	if (iLine == LINEE) {
		delay(HUMAN_DISPLAY_ERROR_DELAY);
	}

	if (iLine == LINEH)
		delay(HUMAN_DISPLAY_INFO_DELAY);
}

void lcd_reset() {
	PJOUT &= ~BIT6;
	delay(100);	//delay 100 ms
	PJOUT |= BIT6;
}

void lcd_blenable() {
	PJOUT |= BIT7;
}

void lcd_bldisable() {
	PJOUT &= ~BIT7;
}

const char display[4] = { '*', '|', '/', '-' };
void lcd_print_progress() {
	if (g_bLCD_state == 0)
		return;

	static char pos = 0;
	lcd_setaddr(0x4F);
	i2c_write(SLAVE_ADDR_DISPLAY, 0x40, 1, (uint8_t *) &display[(++pos) & 0x3]);
}


#if defined(__IAR_SYSTEMS_ICC__)
    /* static data buffer for acceleration */
    #pragma location = "LCD_VTABLE"
__root const uint32_t LCD_Vectors[] = {
    (uint32_t)lcd_clear,
    (uint32_t)lcd_on,
    (uint32_t)lcd_off,
    (uint32_t)lcd_setaddr,
    (uint32_t)lcd_print,
    (uint32_t)lcd_printl,
    (uint32_t)lcd_reset,
    (uint32_t)lcd_blenable,
    (uint32_t)lcd_bldisable,
    (uint32_t)lcd_print_progress,
    (uint32_t)getLCD_state
};
#else
#error "Compiler not supported!"
#endif
//}
#endif

/*void lcd_display_config_sensor(int id) {
	TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[id];
	lcd_printf(LINEC, "%s Cold %dm %d", SensorName[id], (int) pAlertParams->maxSecondsCold / 60, (int) pAlertParams->threshCold);
	lcd_printf(LINE2, "Hot %dm %d", (int) pAlertParams->maxSecondsHot / 60, (int) pAlertParams->threshHot);
	lcd_printf(LINE2, "Hot %dm %d", (int) pAlertParams->maxSecondsHot / 60, (int) pAlertParams->threshHot);
}*/

/*sachin void lcd_display_config() {
	TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[0];
	char num1[TEMP_DATA_LEN + 1];
	char num2[TEMP_DATA_LEN + 1];

	getFloatNumber2Text(pAlertParams->threshCold, num1);
	getFloatNumber2Text(pAlertParams->threshHot, num2);
*/
	/*
#ifdef _DEBUG
	checkStack();
	lcd_printf(LINEC, "%x %d %s %d %s", g_pSysCfg->stackLeft, (int) pAlertParams->maxSecondsCold / 60,
			&num1[0], (int) pAlertParams->maxSecondsHot / 60, &num2[0]);
#else
*/
/*sachin	lcd_printf(LINEC, "C%d %s H%d %s", (int) pAlertParams->maxSecondsCold / 60,
			&num1[0], (int) pAlertParams->maxSecondsHot / 60, &num2[0]);
//#endif
	lcd_printf(LINE2, "S%d U%d L%d P%d", g_pDevCfg->cfgSelectedSIM_slot + 1,
			g_pDevCfg->sIntervalsMins.upload,
			g_pDevCfg->sIntervalsMins.sampling,
			g_pDevCfg->stBattPowerAlertParam.minutesPower);
}*/
