/*
* config.c
*
*  Created on: Feb 25, 2015
*      Author: rajeevnx
*/
#define CONFIG_C_

#include "thermalcanyon.h"
#include "time.h"
#include "stringutils.h"
#include "hardware_buttons.h"
#include "time.h"
#include "events.h"
#include "state_machine.h"
#include "FatFs_Lib.h"
#include "fatdata.h"
#include "spi_flash.h"
#include "config_profiles.h"

#ifdef USE_MININI
#include "minIni.h"
FRESULT config_read_ini_file();
#endif

#ifdef _DEBUG
//#define DEBUG_SEND_CONFIG
#endif

// Setup mode in which we are at the moment
// Triggered by the Switch 3 button
int8_t g_iSystemSetup = -1;
char sensorId;
void default_param_reset();
char * delete_word(char str[], char word[], int index);
char * remove_character(char * a1_buf, char *remove_word);
int search(char str[], char word[]);
char * delete_word_upto_string(char str[], char word[], int index);
char* remove_string_upto_word(char * a1_buf,  char word[]);
char * find_json_value(char * a1_buf,  char word[], char word_1[]);


/************************** BEGIN CONFIGURATION MEMORY ****************************************/

#if defined(__TI_COMPILER_VERSION__)
#pragma SET_DATA_SECTION(".ConfigurationArea")
CONFIG_DEVICE g_ConfigDevice; // configuration of the device, APN, gateways, etc.
CONFIG_SYSTEM g_ConfigSystem; // is system initialized, number of runs, and overall stats
#pragma SET_DATA_SECTION()
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma location="CONFIGURATION"
__no_init CONFIG_DEVICE g_ConfigDevice; // configuration of the device, APN, gateways, etc.
#pragma location="CONFIGURATION"
__no_init CONFIG_SYSTEM g_ConfigSystem; // is system initialized, number of runs, and overall stats
#else
#error Compiler not supported!
#endif

// Since we use the area outside the 16 bits we have to use the large memory model.
// The compiler gives a warning accessing the structure directly and it can fail. Better accessing it through the pointer.
CONFIG_DEVICE *g_pDevCfg = &g_ConfigDevice;
CONFIG_SYSTEM* g_pSysCfg = &g_ConfigSystem;

// Checks if this system has been initialized. Reflashes config and runs calibration in case of being first flashed.

/************************** END CONFIGURATION MEMORY ****************************************/

// Checks if a memory address is not initialized (used for strings)
uint8_t check_address_empty(uint8_t mem) {
    return ((uint8_t) mem == 0xFF || (uint8_t) mem == 0);
}

void config_reset_error(SIM_CARD_CONFIG *sim) {
    sim->simErrorToken = '\0';
    sim->simErrorState = NO_ERROR;
    state_SIM_operational();
}

/*
void config_display_config() {
#ifdef _DEBUG
int t = 0;
lcd_printl(LINEC, get_string(LCD_MSG, SMS_GATEWAY));
lcd_printl(LINEH, g_pDevCfg->cfgGatewaySMS);

lcd_printl(LINEC, get_string(LCD_MSG, GATEWAY_IP));
lcd_printl(LINEH, g_pDevCfg->cfgGatewayIP);

for (t = 0; t < 2; t++) {
lcd_printf(LINEC, "APN %d", t + 1);
lcd_printl(LINEH, g_pDevCfg->SIM[t].cfgAPN);
	}

lcd_printl(LINEC, get_string(LCD_MSG, CONFIG_URL));
lcd_printl(LINEH, g_pDevCfg->cfgConfig_URL);

lcd_printl(LINEC, get_string(LCD_MSG, UPLOAD_URL));
lcd_printl(LINEH, g_pDevCfg->cfgUpload_URL);
#endif
lcd_display_config();
}
*/

//TODO This should go to the state machine

void config_setSIMError(SIM_CARD_CONFIG *sim, char errorToken, uint16_t errorID,
			const char *error) {
#ifndef _FISM_
			    char CM[4] = "CMX";
#else
			    char cm[4] = "CMX";
#endif
			    if (error == NULL || sim == NULL)
				return;

			    sim->simErrorState = errorID;
			    sim->simErrorToken = errorToken;

			    // 69 - "Requested facility not implemented"
			    // This cause indicates that the network is unable to provide the requested short message service.

			    if (errorID == 0)
				return;
#ifndef _FISM_
			    CM[2] = errorToken;
			    ini_gets(CM, itoa_nopadding(errorID), error, sim->simLastError,
				     sizeof(sim->simLastError), "errors.ini");
#else
			    cm[2] = errorToken;
#ifndef _FISM_
			    ini_gets(cm, itoa_nopadding(errorID), error, sim->simLastError,
				     sizeof(sim->simLastError), "errors.ini");
#endif
#endif


			    //	if (errorID == 69)
			    //		state_setSMS_notSupported(sim);

			    state_sim_failure(sim);
			    modem_check_working_SIM();

			    //sachin	event_LCD_turn_on();
			}

// Returns the current structure containing the info for the current SIM selected
uint16_t config_getSIMError(int slot) {
    return g_pDevCfg->SIM[slot].simErrorState;
}

// Returns the current structure containing the info for the current SIM selected
SIM_CARD_CONFIG *config_getSIM() {
    int8_t slot = g_pDevCfg->cfgSIM_slot;
    if (slot < 0 || slot > 1)
	g_pDevCfg->cfgSIM_slot = 0;

    return &g_pDevCfg->SIM[slot];
}

uint8_t config_getSelectedSIM() {
    if (g_pDevCfg->cfgSIM_slot < 0 || g_pDevCfg->cfgSIM_slot > 1)
	g_pDevCfg->cfgSIM_slot = 0;

    return g_pDevCfg->cfgSIM_slot;
}

uint8_t config_is_SIM_configurable(int simSlot) {
    SIM_CARD_CONFIG *sim;

    if (simSlot >= SYSTEM_NUM_SIM_CARDS) {
	return 0;
    }

    sim = &g_pDevCfg->SIM[simSlot];

    if (sim->cfgAPN[0] == '\0' && sim->cfgPhoneNum[0] == '\0') {
	return 0;
    }

    return 1;
}

uint16_t config_getSimLastError(char *charToken) {

    uint8_t slot = g_pDevCfg->cfgSIM_slot;
    if (charToken != NULL)
	*charToken = g_pDevCfg->SIM[slot].simErrorToken;

    return g_pDevCfg->SIM[slot].simErrorState;
}

void config_SafeMode() {
    _NOP();
}

// Runs the system in configuration/calibration mode
void config_reconfigure() {
    g_pSysCfg->memoryInitialized = 0xFF;
    PMM_trigBOR();
    while (1)
	;
}

CONFIG_APP_MODE cfgGetAppMode(void) {
    return g_pDevCfg->cfgAppMode;
}

//must remember to modify this array whenever additional
//CONFIG_APP_MODE enums added
static const char *g_szAppModeStrings[] = {
    APP_MODE_CT5_STRING,
    APP_MODE_ST5_STRING,
    ""
};

static void config_validate_app_mode() {
    strcpy(g_pDevCfg->cfgVersion, APP_MODE_CT5_STRING);
    if(strlen(g_pDevCfg->cfgVersion)) {
	int i;
	for(i = 0; i < APP_MODE_INVALID; i++) {
	    if(!strcmp(g_pDevCfg->cfgVersion, g_szAppModeStrings[i])) {
		g_pDevCfg->cfgAppMode = (CONFIG_APP_MODE)i;
		break;
	    }
	}
    }
}

void default_param_reset()
{
  // Default Configuration
  memset(g_pDevCfg,0,sizeof(CONFIG_DEVICE));
  strcpy(g_pDevCfg->cfgGatewayIP,   NEXLEAF_DEFAULT_SERVER_IP); // HTTP server nextleaf
  strcpy(g_pDevCfg->cfgGatewaySMS,  NEXLEAF_SMS_GATEWAY); // Gateway to nextleaf
  strcpy(g_pDevCfg->cfgMsgSenderId, DEFAULT_SMS_SENDER_ID);
  strcpy(g_pDevCfg->cfgGatewayKey,  DEFAULT_SMS_GATEWAY_KEY);
  strcpy(g_pDevCfg->cfgConfig_URL,  CONFIGURATION_URL_PATH);
  strcpy(g_pDevCfg->cfgUpload_URL, DATA_UPLOAD_URL_PATH);    //shubh test
  g_pDevCfg->sIntervalsMins.sampling = PERIOD_SAMPLING;     
  g_pDevCfg->sIntervalsMins.upload   = PERIOD_UPLOAD;
  g_pDevCfg->cfgUploadMode           = TX_MODE_GPRS;
  event_setInterval_by_id_secs(EVT_SUBSAMPLE_TEMP, MINUTES_(g_pDevCfg->sIntervalsMins.sampling));
  event_setInterval_by_id_secs(EVT_UPLOAD_SAMPLES,MINUTES_(g_pDevCfg->sIntervalsMins.upload));

  read_indexes(0x1900,(char *)&flash_memory,sizeof(flash_memory));
  debug_print("read done ");

  memset(flash_memory.server_url,'\0',sizeof(flash_memory.server_url));
  strcpy(flash_memory.server_url,  NEXLEAF_DEFAULT_SERVER_IP);
  update_put_get_idx();
  flags.factory_reset = 1;
  debug_print(" update done ");

  // Setup internal system counters and checks
  memset(g_pSysCfg, 0, sizeof(CONFIG_SYSTEM));
  // Value to check to make sure the structure is still the same size;
  g_pSysCfg->configStructureSize = sizeof(CONFIG_SYSTEM);
  g_pSysCfg->memoryInitialized = 1;
  config_default_configuration();
  debug_print(" config_default_configuration ");

  config_validate_app_mode();
}

char g_bServiceMode = false;

void config_init() {
  default_param_reset();
  // Service Button was pressed during bootup. Rerun calibration
if (switch_check_service_pressed()) {
g_bServiceMode = true;
}
RED_LED_ON
if (g_bServiceMode == true)
{
// Buzzer service mode test//
lcd_printl(LINEC, get_string(LCD_MSG, BUZZER_TEST));
state_alarm_turnon_buzzer();
lcd_printl(LINE2, get_string(LCD_MSG, PRESS_STOP_ALARM));
while(P2IN & BIT2);
state_clear_alarm_state();
}
  
#ifndef _FISM_
        //   int idx = 0;
	// Check if the user is pressing the service mode
	// Service Button was pressed during bootup. Rerun calibration
	if (switch_check_service_pressed()) {
		g_bServiceMode = true;

		lcd_printl(LINEC, get_string(LCD_MSG, SERVICE_MODE));
		delay(HUMAN_DISPLAY_LONG_INFO_DELAY);

		/* LCD service mode test */
        	lcd_printl(LINEC, get_string(LCD_MSG, LCD_INIT_1));
		lcd_printl(LINE2, get_string(LCD_MSG, LCD_INIT_2));
		delay(HUMAN_DISPLAY_LONG_INFO_DELAY);

	} else if (!check_address_empty(g_pSysCfg->memoryInitialized)) {
        // Data structure changed, something failed.
        // Check firmware version?
          
		if (g_pSysCfg->configStructureSize != sizeof(CONFIG_SYSTEM)) {
			config_SafeMode();
		}

		g_pSysCfg->numberConfigurationRuns++;

		if(g_pSysCfg->numberConfigurationRuns == 2) {     
                  config_read_ini_file();
                }

		return; // Memory was initialized, we are fine here.
	}

        // Init Config
	memset(g_pDevCfg, 0, sizeof(CONFIG_DEVICE));

	strcpy(g_pDevCfg->cfgGatewayIP, NEXLEAF_DEFAULT_SERVER_IP); // HTTP server nextleaf
	strcpy(g_pDevCfg->cfgGatewaySMS, NEXLEAF_SMS_GATEWAY); // Gateway to nextleaf
        strcpy(g_pDevCfg->cfgMsgSenderId, DEFAULT_SMS_SENDER_ID);
        strcpy(g_pDevCfg->cfgGatewayKey, DEFAULT_SMS_GATEWAY_KEY);
	strcpy(g_pDevCfg->cfgConfig_URL, CONFIGURATION_URL_PATH);
	//strcpy(g_pDevCfg->cfgUpload_URL, DATA_UPLOAD_URL_PATH);
	config_setSIMError(&g_pDevCfg->SIM[0], '+', NO_ERROR, "1 OK");
	config_setSIMError(&g_pDevCfg->SIM[1], '+', NO_ERROR, "2 OK");
        // Init System internals

        // Setup internal system counters and checks
	memset(g_pSysCfg, 0, sizeof(CONFIG_SYSTEM));

        // First run
        if(g_bServiceMode == true)
	g_pSysCfg->numberConfigurationRuns = 1;
         else
        g_pSysCfg->numberConfigurationRuns = 2;
        
        // Value to check to make sure the structure is still the same size;
	g_pSysCfg->configStructureSize = sizeof(CONFIG_SYSTEM);
	g_pSysCfg->memoryInitialized = 1;

#ifdef _DEBUG_COUNT_BUFFERS
	g_pSysCfg->maxSamplebuffer = 0;

	g_pSysCfg->maxRXBuffer = 0;
	g_pSysCfg->maxTXBuffer = 0;
#endif
	config_default_configuration();
#ifdef USE_MININI
	config_read_ini_file();
#endif
        config_validate_app_mode();

	if (g_bServiceMode == true)
	{
		/* Buzzer service mode test */
		lcd_printl(LINEC, get_string(LCD_MSG, BUZZER_TEST));
		state_alarm_turnon_buzzer();
		lcd_printl(LINE2, get_string(LCD_MSG, PRESS_STOP_ALARM));
		while(P2IN & BIT2);
		state_clear_alarm_state();
	}

#ifdef _EVIN_BUILD_
    lcd_printf(LINEC, "CFG VER: %s", g_pDevCfg->cfgVersion);
    delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
    lcd_clear();
#endif

#else
    g_pSysCfg->numberConfigurationRuns++;

    if(g_pSysCfg->numberConfigurationRuns >= 3) 
    {     
        g_pSysCfg->memoryInitialized = 0;
        g_pSysCfg->numberConfigurationRuns =0;
    }
    
    if (!check_address_empty(g_pSysCfg->memoryInitialized)) {
      // Data structure changed, something failed.
      if (g_pSysCfg->configStructureSize != sizeof(CONFIG_SYSTEM)) {
            config_SafeMode();
      }
      return; // Memory was initialized, we are fine here.
    }

#ifdef _FDEBUG_
      debug_print("Factory Reset\n\r");
#endif
      
    default_param_reset();
#endif
}

/*
void config_init()
{
  // Check if the user is pressing the service mode
  // Service Button was pressed during bootup. Rerun calibration
  if (switch_check_service_pressed())
  {
    debug_print("BOOT IN FACTORY_RESET");
    g_bServiceMode = true;
    lcd_printl(LINEC, get_string(LCD_MSG, SERVICE_MODE));
    delay(HUMAN_DISPLAY_LONG_INFO_DELAY);

    // LCD service mode test 
    lcd_printl(LINEC, get_string(LCD_MSG, LCD_INIT_1));
    lcd_printl(LINE2, get_string(LCD_MSG, LCD_INIT_2));
    delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
  }
  else if (!check_address_empty(g_pSysCfg->memoryInitialized))
  {
    // Data structure changed, something failed.
    // Check firmware version?
    if (g_pSysCfg->configStructureSize != sizeof(CONFIG_SYSTEM))
    {
      config_SafeMode();
    }

    g_pSysCfg->numberConfigurationRuns++;
    if(g_pSysCfg->numberConfigurationRuns == 2)
    {
      config_read_ini_file();
    }
    return; // Memory was initialized, we are fine here.
  }

  // Init Config
  memset(g_pDevCfg, 0, sizeof(CONFIG_DEVICE));

  strcpy(g_pDevCfg->cfgGatewayIP, NEXLEAF_DEFAULT_SERVER_IP); // HTTP server nextleaf
  strcpy(g_pDevCfg->cfgGatewaySMS, NEXLEAF_SMS_GATEWAY); // Gateway to nextleaf
  strcpy(g_pDevCfg->cfgMsgSenderId, DEFAULT_SMS_SENDER_ID);
  strcpy(g_pDevCfg->cfgGatewayKey, DEFAULT_SMS_GATEWAY_KEY);
  strcpy(g_pDevCfg->cfgConfig_URL, CONFIGURATION_URL_PATH);
  strcpy(g_pDevCfg->cfgUpload_URL, DATA_UPLOAD_URL_PATH);
  strcpy(g_pDevCfg->cfgDeviceReady_URL, DEVICE_READY_URL_PATH);
  strcpy(g_pDevCfg->cfgDeviceAlarm_URL, DEVICE_ALARM_URL_PATH);
  config_setSIMError(&g_pDevCfg->SIM[0], '+', NO_ERROR, "1 OK");
  config_setSIMError(&g_pDevCfg->SIM[1], '+', NO_ERROR, "2 OK");
  // Init System internals
  // Setup internal system counters and checks
  memset(g_pSysCfg, 0, sizeof(CONFIG_SYSTEM));

  // First run
  if(g_bServiceMode == true)
  {
     g_pSysCfg->numberConfigurationRuns = 1;
  }
  else
  {
    g_pSysCfg->numberConfigurationRuns = 2;
  }

  // Value to check to make sure the structure is still the same size;
  g_pSysCfg->configStructureSize = sizeof(CONFIG_SYSTEM);
  g_pSysCfg->memoryInitialized = 1;

#ifdef _DEBUG_COUNT_BUFFERS
  g_pSysCfg->maxSamplebuffer = 0;
  g_pSysCfg->maxRXBuffer = 0;
  g_pSysCfg->maxTXBuffer = 0;
#endif
  config_default_configuration();

#ifdef USE_MININI
  config_read_ini_file();
#endif

  config_validate_app_mode();
  if (g_bServiceMode == true)
  {
    // Buzzer service mode test 
    lcd_printl(LINEC, get_string(LCD_MSG, BUZZER_TEST));
    state_alarm_turnon_buzzer();
    lcd_printl(LINE2, get_string(LCD_MSG, PRESS_STOP_ALARM));
    while(P2IN & BIT2);
    state_clear_alarm_state();
  }

  lcd_printf(LINEC,"CFG VER: %s", g_pDevCfg->cfgVersion);
  delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
  lcd_clear();

  g_pSysCfg->numberConfigurationRuns++;
  debug_print("g_pSysCfg->numberConfigurationRuns = ");
  debug_print((char *) itoa_nopadding(g_pSysCfg->numberConfigurationRuns));
  debug_print("\n\r");

  if(g_pSysCfg->numberConfigurationRuns >= 3)
  {
    g_pSysCfg->memoryInitialized       = 0;
    g_pSysCfg->numberConfigurationRuns = 0;
    debug_print("g_pSysCfg->memoryInitialized = ");
    debug_print((char *) itoa_nopadding(g_pSysCfg->memoryInitialized));
    debug_print("\n\r");
  }

  if (!check_address_empty(g_pSysCfg->memoryInitialized)) {
      // Data structure changed, something failed.
      if (g_pSysCfg->configStructureSize != sizeof(CONFIG_SYSTEM)) {
          config_SafeMode();
      }
      return; // Memory was initialized, we are fine here.
  }

  state_alarm_turnon_buzzer();
  //if(g_pSysCfg->memoryInitialized == 0)
  { debug_print("\n\rshubham.............");
    debug_print("Factory Reset BOOT");
    default_param_reset();
  }
}
*/

void config_update_system_time() {
    /*
    // Gets the current time and stores it in FRAM
    rtc_getlocal(&g_tmCurrTime);
    memcpy(&g_pDevCfg->lastSystemTime, &g_tmCurrTime, sizeof(g_tmCurrTime));
    */}
static void config_url_error_show(void) {
    lcd_printl(LINEE, "URL overflow...");
}

int8_t parse_config_pull(const char *value, int type);

//char sensorId[3] = "\0";
int16_t dur_hot,dur_cold;
float temp_hot_float,temp_cold_float;
uint8_t al_flag=0;

/*sachin const Key_ValueString g_KeyValueString[]
= {
{"gw",g_ConfigDevice.cfgGatewaySMS, sizeof g_ConfigDevice.cfgGatewaySMS},
{"ip",g_ConfigDevice.cfgGatewayIP, sizeof g_ConfigDevice.cfgGatewayIP},
{"sId",sensorId, sizeof sensorId},
{"s1apn",g_ConfigDevice.SIM[0].cfgAPN, sizeof g_ConfigDevice.SIM[0].cfgAPN},
{"s2apn",g_ConfigDevice.SIM[1].cfgAPN, sizeof g_ConfigDevice.SIM[1].cfgAPN},
//{"u",g_ConfigDevice.cfgConfig_URL, sizeof g_ConfigDevice.cfgConfig_URL},
{"si",g_ConfigDevice.cfgMsgSenderId, sizeof g_ConfigDevice.cfgMsgSenderId},
{"dk",g_ConfigDevice.cfgGatewayKey, sizeof g_ConfigDevice.cfgGatewayKey}
//{"cn",temp[2]},
//{"ln",temp[3]},
};
*/
/*sachin const Key_ValuePipe g_KeyValuePipe[]
= {
{"in",&setIn},
{"al",&parse_alarm_var},
{"p",&parse_phone_number},
{"tz",&parse_time_zone},
{"dr",&parse_alarm_repeat_variable},
{"pb",&parse_battery_alarm_variable},
{"u",&parse_config_pull}
//{"nt",&showValue},
//{"wn",&showValue},
};
*/

/* sachin const Key_ValueInt g_KeyValueInt[]
= {
{"c",&g_ConfigDevice.cfgUploadMode, 1} // evin is 0,1,2 but we need 1,2,3
//{"v",&Temp[1]},
};
*/
/*sachin const Key_ValueBitFlag g_KeyValueBitFlag[]
= {
{"bd",&g_ConfigDevice.cfg.status, 6}, //g_pDevCfg->cfg.logs.buzzer_disable is bit 6 of cfg.status byte
{"drSMS",&g_ConfigDevice.cfgSMSmode.byte, 0}, //g_pDevCfg->cfgSMSmode.bits.BootDeviceReady is bit 0 of cfgSMSmode
{"foSMS",&g_ConfigDevice.cfgSMSmode.byte, 1} //g_pDevCfg->cfgSMSmode.bits.APIFailover is bit 1 of cfgSMSmode
};
*/
char g_keyval[20][3] = {"in","tz","dr","pb","p","al","u","gw","ip","si","dk","c","bd","a1","a2","sId","\0",
"fwu","\0"};

int8_t config_parse_configuration_sms(char *msgbuf)
{
    char *token,*val,*sms_val = NULL;
    char value[3] = {"\0"};
    int do_stop=0;
    lcd_printl(LINEC,"Parsing Config...");
    g_pSysState->type = 1;
    token = strstr(msgbuf,g_ConfigDevice.cfgMsgSenderId);
    if(token == NULL)
    {
	flags.sms_config = 0;
	return -1;
    }
    if(!flags.sms_config)
    {
	/* Sender ID in GPRS config comes after t=1 hence to avoid confliction update upload mode here for gprs config*/
	sms_val = token - 7;
	memset(value,'\0',sizeof(value));

	value[0] = sms_val[2];
	g_ConfigDevice.cfgUploadMode = atoi(value);
	debug_print("upload mode = ");
	debug_print(itoa_pad(g_pDevCfg->cfgUploadMode));       // rachit debug // before
	debug_print("\r\n");
    }
    flags.param_updated = 1;
    token = token + strlen(g_ConfigDevice.cfgMsgSenderId) + 1; //+1 is for space
    token = strtok(token,",");
    while(!do_stop && token != NULL)
    {
	unsigned int eqPos = strcspn(token, "="); //non-destrucively find equal sign pos
	if(strlen(token) == eqPos)
	{
	    val = NULL; //pass val as NULL to signal that no equal sign was found
	}
	else
	{
	    token[eqPos] = '\0'; //turn eq into null term
	    val = token + eqPos + 1; //set val to start after that pos (may be null also)
	}
	do_stop = process_kvp(token, val);
	token = strtok(NULL,",");
    }
    if(al_flag)
    {
	process_sId();
    }
    lcd_printl(LINE2,"configuring...");
    // aditya delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
    return 0;
}


//void config_parse_configuration_http(void) {
int config_parse_configuration_http(void)
{
//    return config_parse_configuration_sms((char *)uart_getRXHead());
    char *token,*val,*sms_val = NULL;
    char value[3] = {"\0"};
    int do_stop=0,index = 0;
    char *location_1 = 0;
    char *location_2 = 0;
    char demo_buffer[200];
    char upload_url[500],sample_interval[2];
    char json_buffer[3000] = ("{\"data\": {\"comm\": {\"chnl\": 1, \"tmpUrl\": \"https://undp.coldtrace.org/uploads/l/v3/\", \"cfgUrl\": \"https://undp.coldtrace.org/ct5demo/config/json/v1/\", \"alrmUrl\": \"https://undp.coldtrace.org/intel/config/\", \"statsUrl\": \"https://undp.coldtrace.org/intel/config/\", \"devRyUrl\": \"https://undp.coldtrace.org/intel/config/\", \"smsGyPh\": \"+918096590590\", \"senderId\": \"COLDTR\", \"smsGyKey\": \"cltr\", \"tmpNotify\": false, \"incExcNotify\": true, \"statsNotify\": false, \"devAlrmsNotify\": true, \"tmpAlrmsNotify\": false, \"samplingInt\": 10, \"pushInt\": 60, \"usrPhones\": [\"+919879581848\"], \"wifi\": {\"ssid\": \"testssid\", \"pwd\": \"testpass\", \"security\": \"\"}}, \"highAlarm\": {\"temp\": 8, \"dur\": 300}, \"lowAlarm\": {\"temp\": 2, \"dur\": 60}, \"highWarn\": {\"temp\": 8, \"dur\": 300}, \"lowWarn\": {\"temp\": 2, \"dur\": 60}, \"sensors\": [{\"sId\": \"a\", \"comm\": {\"tmpNotify\": false, \"incExcNotify\": true, \"statsNotify\": false, \"devAlrmsNotify\": true, \"tmpAlrmsNotify\": false, \"samplingInt\": 10, \"pushInt\": 60}, \"highAlarm\": {\"temp\": 8.0, \"dur\": 0}, \"lowAlarm\": {\"temp\": 2.0, \"dur\": 0}, \"highWarn\": {\"temp\": 8.0, \"dur\": 0}, \"lowWarn\": {\"temp\": 2.0, \"dur\": 0}, \"notf\": {\"dur\": \"360\", \"num\": \"28\"}}, {\"sId\": \"b\", \"comm\": {\"tmpNotify\": false, \"incExcNotify\": true, \"statsNotify\": false, \"devAlrmsNotify\": true, \"tmpAlrmsNotify\": false, \"samplingInt\": 10, \"pushInt\": 60}, \"highAlarm\": {\"temp\": 8.0, \"dur\": 0}, \"lowAlarm\": {\"temp\": 2.0, \"dur\": 0}, \"highWarn\": {\"temp\": 8.0, \"dur\": 0}, \"lowWarn\": {\"temp\": 2.0, \"dur\": 0}, \"notf\": {\"dur\": \"360\", \"num\": \"28\"}}, {\"sId\": \"c\", \"comm\": {\"tmpNotify\": false, \"incExcNotify\": true, \"statsNotify\": false, \"devAlrmsNotify\": true, \"tmpAlrmsNotify\": false, \"samplingInt\": 10, \"pushInt\": 60}, \"highAlarm\": {\"temp\": 8.0, \"dur\": 0}, \"lowAlarm\": {\"temp\": 2.0, \"dur\": 0}, \"highWarn\": {\"temp\": 8.0, \"dur\": 0}, \"lowWarn\": {\"temp\": 2.0, \"dur\": 0}, \"notf\": {\"dur\": \"360\", \"num\": \"28\"}}, {\"sId\": \"d\", \"comm\": {\"tmpNotify\": false, \"incExcNotify\": true, \"statsNotify\": false, \"devAlrmsNotify\": true, \"tmpAlrmsNotify\": false, \"samplingInt\": 10, \"pushInt\": 60}, \"highAlarm\": {\"temp\": 8.0, \"dur\": 0}, \"lowAlarm\": {\"temp\": 2.0, \"dur\": 0}, \"highWarn\": {\"temp\": 8.0, \"dur\": 0}, \"lowWarn\": {\"temp\": 2.0, \"dur\": 0}, \"notf\": {\"dur\": \"360\", \"num\": \"28\"}}, {\"sId\": \"e\", \"comm\": {\"tmpNotify\": false, \"incExcNotify\": true, \"statsNotify\": false, \"devAlrmsNotify\": true, \"tmpAlrmsNotify\": false, \"samplingInt\": 10, \"pushInt\": 60}, \"highAlarm\": {\"temp\": 8.0, \"dur\": 0}, \"lowAlarm\": {\"temp\": 2.0, \"dur\": 0}, \"highWarn\": {\"temp\": 8.0, \"dur\": 0}, \"lowWarn\": {\"temp\": 2.0, \"dur\": 0}, \"notf\": {\"dur\": \"360\", \"num\": \"28\"}}], \"notf\": {\"dur\": \"360\", \"num\": \"28\"}, \"locale\": {\"tz\": \"5.5\", \"cn\": \"IN\", \"ln\": \"en\"}, \"poa\": {\"dur\": 240}, \"lba\": {\"lmt\": 20}}}");
//    char json_buffer[251];
    int find_index = 0;
    lcd_printl(LINEC,"Parsing Config...");
    g_pSysState->type = 1;

    //    location_1 = strtok(json_buffer+10,"{}\":");
    //    location_1 = remove_character(json_buffer,"data");
    //    location_2 = strstr(json_buffer,"data");
    //    location_2 = strchr(json_buffer,'\"');
    //    index = search(json_buffer, "lowAlarm");


    char at_cmd_str[32];
    char * ptr_at_responce;

/*    gsm_init();
    http_open_connection_upload(0,CONFIGURATION_URL_PATH);


    send_cmd_wait("AT+HTTPACTION=0\r\n", ",", 150,3);
    memset(at_cmd_str,'\0',sizeof(at_cmd_str));
    sprintf(at_cmd_str,"AT+HTTPREAD=480,250\r\n");
    memset(GSM_buffer, 0, sizeof(GSM_buffer));
    memset(json_buffer, 0, sizeof(json_buffer));
    if(send_cmd_wait(at_cmd_str, "+HTTPREAD", 100, 2))
    {
	strcpy(json_buffer,GSM_buffer);
    }
    debug_print(json_buffer);
*/

    char * print_demo = 0;
    location_1 =  find_json_value(json_buffer,"samplingInt",",");
    debug_print("\n\rSampling Interval-->");
    json_buffer[0] = *location_1;
    debug_print(json_buffer);


    char upload_buffer[200];
    FRESULT fr;
    FIL *fobj;
    char* fn;
    uint16_t lineSize = 0;
    char *line = getStringBufferHelper(&lineSize);

    memset(upload_buffer,'\0',sizeof(upload_buffer));

    if (!g_bFatInitialized)
	return FR_NOT_READY;

    fr = fat_open(&fobj, CONFIG_LOG_FILE_PATH, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK)
    {
	fat_check_error(fr);
	return fr;
    }
    f_lseek(fobj, 0);
    sprintf(upload_buffer,"Sample Interval");
    sprintf(upload_buffer,"%s",location_1);
    //sprintf(upload_buffer,'\n');
    fat_close();


    location_1 =  find_json_value(json_buffer,"pushInt",",");
    debug_print("\n\rUpload Interval-->");
    debug_print(location_1);
    location_1 = remove_string_upto_word(json_buffer,"wifi");
    json_buffer[0] = *location_1;
    location_1 =  find_json_value(json_buffer,"ssid",",");
    debug_print("\n\rssid-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"pwd",",");
    debug_print("\n\rPassword-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"dur","}");
    debug_print("\n\rhigh Alarm duration-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"dur","}");
    debug_print("\n\rlow Alarm duration-->");
    debug_print(location_1);
    send_cmd_wait("AT+HTTPACTION=0\r\n", ",", 150,3);
    memset(at_cmd_str,'\0',sizeof(at_cmd_str));
    sprintf(at_cmd_str,"AT+HTTPREAD=940,250\r\n");
    memset(GSM_buffer, 0, sizeof(GSM_buffer));
    memset(json_buffer, 0, sizeof(json_buffer));
    if(send_cmd_wait(at_cmd_str, "+HTTPREAD", 100, 2))
    {
	strcpy(json_buffer,GSM_buffer);
    }
    debug_print(json_buffer);

    location_1 = remove_string_upto_word(json_buffer,"highAlarm");
    json_buffer[0] = *location_1;
    location_1 =  find_json_value(json_buffer,"temp",",");
    debug_print("\n\rHigh Alarm Temp A-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"temp",",");
    debug_print("\n\rlow Alarm Temp A-->");
    debug_print(location_1);


    send_cmd_wait("AT+HTTPACTION=0\r\n", ",", 150,3);
    memset(at_cmd_str,'\0',sizeof(at_cmd_str));
    sprintf(at_cmd_str,"AT+HTTPREAD=1120,250\r\n");
    memset(GSM_buffer, 0, sizeof(GSM_buffer));
    memset(json_buffer, 0, sizeof(json_buffer));
    if(send_cmd_wait(at_cmd_str, "+HTTPREAD", 100, 2))
    {
	strcpy(json_buffer,GSM_buffer);
    }
    debug_print(json_buffer);


    location_1 = remove_string_upto_word(json_buffer,"highAlarm");
    json_buffer[0] = *location_1;
    location_1 =  find_json_value(json_buffer,"temp",",");
    debug_print("\n\rHigh Alarm Temp B-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"temp",",");
    debug_print("\n\rlow Alarm Temp B-->");
    debug_print(location_1);


    send_cmd_wait("AT+HTTPACTION=0\r\n", ",", 150,3);
    memset(at_cmd_str,'\0',sizeof(at_cmd_str));
    sprintf(at_cmd_str,"AT+HTTPREAD=2640,250\r\n");
    memset(GSM_buffer, 0, sizeof(GSM_buffer));
    memset(json_buffer, 0, sizeof(json_buffer));
    if(send_cmd_wait(at_cmd_str, "+HTTPREAD", 100, 2))
    {
	strcpy(json_buffer,GSM_buffer);
    }
    debug_print(json_buffer);

    location_1 = remove_string_upto_word(json_buffer,"poa");
    json_buffer[0] = *location_1;
    location_1 =  find_json_value(json_buffer,"dur","}");
    debug_print("\n\rPower outage duration alarm-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"lmt","}");
    debug_print("\n\rPower outage duration alarm-->");
    debug_print(location_1);

/*    location_1 =  find_json_value(json_buffer,"tmpUrl",",");
    debug_print("\n\rtmpUrl -->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"cfgUrl",",");
    debug_print("\n\rcfgUrl-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"alrmUrl",",");
    debug_print("\n\ralrmUrl-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"statsUrl",",");
    debug_print("\n\rstatsUrl-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"devRyUrl",",");
    debug_print("\n\rdevRyUrl-->");
    debug_print(location_1);

    location_1 = remove_string_upto_word(json_buffer,"temp");
    json_buffer[0] = *location_1;
    location_1 =  find_json_value(json_buffer,"temp",",");
    debug_print("\n\rHigh Alarm Temp C-->");
    debug_print(location_1);
    location_1 =  find_json_value(json_buffer,"temp",",");
    debug_print("\n\rlow Alarm Temp C-->");
    debug_print(location_1);
*/
}

char * find_json_value(char * a1_buf,  char word[],char word_1[])
{
    char *loc = 0;
    int index = 0;
    char msg_buffer[100];
    memset(msg_buffer, 0, sizeof(msg_buffer));
    loc = remove_string_upto_word(a1_buf, word);
    a1_buf[0] = *loc;
    loc = remove_character(a1_buf,":");
    a1_buf[0] = *loc;
    index = search(a1_buf,word_1);
    for(int temp_abc = 0;temp_abc<index;temp_abc++)
    {
	msg_buffer[temp_abc] = a1_buf[temp_abc+1];
    }
    *loc = msg_buffer[0];
    return msg_buffer;
    debug_print("\n\r");
    debug_print(msg_buffer);
    debug_print("\n\r");
    return msg_buffer;
}



char* remove_character(char * a1_buf,  char word[])
{
    int temp_index = 0;
    char *loc = 0;
    temp_index = search(a1_buf, word);
    loc = delete_word(a1_buf,word,temp_index);
    return a1_buf;
}
char* remove_string_upto_word(char * a1_buf,  char word[])
{
    int temp_index = 0;
    char *loc = 0;
    temp_index = search(a1_buf, word);
    loc = delete_word_upto_string(a1_buf,word,temp_index);
    return a1_buf;
}

int search(char str[], char word[])
{
    int l, i, j;
    for (l = 0; word[l] != '\0'; l++);
    for (i = 0, j = 0; str[i] != '\0' && word[j] != '\0'; i++)
    {
        if (str[i] == word[j])
        {
            j++;
        }
        else
        {
            j = 0;
        }
    }
   if (j == l)
    {
        return (i - j);
    }
    else
    {
        return  - 1;
    }
}
char * delete_word(char str[], char word[], int index)
{
    int i, l;
    for (l = 0; word[l] != '\0'; l++);
    for (i = index; str[i] != '\0'; i++)
    {
        str[i] = str[i + l + 1];
    }
return str;
}

char * delete_word_upto_string(char str[], char word[], int index)
{
    int i, l;
    for (l = 0; word[l] != '\0'; l++);
    for (i = 0; str[i] != '\0'; i++)
    {
	str[i] = str[i+index+l];
    }
    return str;
}



#define PULL_URL_BUF_SIZE 100 //just for buffer checking
void config_pull_configuration(const char* urlPath, int type) {
    char szTemp[PULL_URL_BUF_SIZE]; //for holding temp gen'd null-term strings
    int szTemp_c = 0;
    char str [23];

    //prepare string buffer
    szTemp[0] = '\0';
    szTemp_c = PULL_URL_BUF_SIZE;

    szTemp_c -= strlen(urlPath); //account for space of copy
    if(szTemp_c < 1) { //check space before strncat
	config_url_error_show();
	return; //error
    }
    strcat(szTemp, urlPath); //copy base path

    szTemp_c -= 10 + strlen(g_pDevCfg->cfgIMEI); //account for space of copy
    if(szTemp_c < 1) { //check space before strncat
	config_url_error_show();
	return; //error
    }
    if (type == 1) {
	if(strrchr(szTemp, '/') - szTemp + 1 != strlen(szTemp)) strcat(szTemp, "/"); //add trailing slash if necessary
	strcat(szTemp, "Nexleaf/"); //vId
	strcat(szTemp, g_pDevCfg->cfgIMEI); //sId
	strcat(szTemp, "/");
    }
    if (type == 2) {
	strcpy(str, "uploads/coldtrace5.txt");
	strcat(szTemp, str);
    }
    if (http_get(szTemp, &config_parse_configuration_http, type) == UART_SUCCESS) {
	state_alarm_reset_all();
	//      lcd_printl(LINE2,"Sending ConfAck");
	//      delay(2000);
	if (data_send_api_packet(API_PACKET_DEV_READY) != 0 && g_pDevCfg->cfgSMSmode.bits.APIFailover) {
	    sms_send_dataready();
	}
    }
}
#undef PULL_URL_BUF_SIZE
void set_sen(int i)
{
    char temp_val[50];

    TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[i];
    pAlertParams->maxSecondsCold = MINUTES_(dur_cold); // We work in seconds internally
    pAlertParams->threshCold = temp_cold_float;
    pAlertParams->maxSecondsHot = MINUTES_(dur_hot); // We work in seconds internally
    pAlertParams->threshHot = temp_hot_float;

    debug_print("high alarm threshold=");
    getFloatNumber2Text(g_pDevCfg->stTempAlertParams[i].threshHot, temp_val);
    debug_print(temp_val);
    debug_print("\n\r");
    debug_print("high alarm delay=");
    debug_print(itoa_pad(g_pDevCfg->stTempAlertParams[i].maxSecondsHot));
    debug_print("\n\r");
    debug_print("low alarm threshold=");
    getFloatNumber2Text(g_pDevCfg->stTempAlertParams[i].threshCold, temp_val);
    debug_print(temp_val);
    debug_print("\n\r");
    debug_print("low alarm delay=");
    debug_print(itoa_pad(g_pDevCfg->stTempAlertParams[i].maxSecondsCold));
    debug_print("\n\r");
}
int id_sel(char ch) {
    if((ch > 'A' - 1) && (ch < 'E' + 1))
	return (ch - 'A');

    else if((ch > 'a' - 1) && (ch < 'e' + 1))
	return (ch - 'a');

    return -1;
}
/*sachin int parse_test_bool(const char *value)
{
int ret = -1; //invalid

switch(*value) {
       case '0':
       case 'F':
       case 'f':
       case 'N':
       case 'n':
{
ret = 0;
	} break;

       case '1':
       case 'T':
       case 't':
       case 'Y':
       case 'y':
{
ret = 1;
	} break;
    }

return(ret);
}
*/
int setvalue(int8_t key, const char * value) {
    int interval;
    int16_t val;
    int8_t i = -1;
    float tz = atof(value);
    int8_t negateTz = 0;
    int d1, len;
    float d2;
    char BUFF[GW_MAX_LEN + 1], *crc, *addr;
    int8_t j = 0;
    switch(key) {
      case 0:
	value = parse_value(value,&interval);
	g_pDevCfg->sIntervalsMins.sampling = interval;
	event_setInterval_by_id_secs(EVT_SUBSAMPLE_TEMP, MINUTES_(interval));
	event_setInterval_by_id_secs(EVT_SAVE_SAMPLE_TEMP, MINUTES_(interval));
	value = parse_value(value,&interval);
	g_pDevCfg->sIntervalsMins.upload = interval;
	event_setInterval_by_id_secs(EVT_UPLOAD_SAMPLES, MINUTES_(interval));
	debug_print("sample interval = ");
	debug_print(itoa_pad(g_pDevCfg->sIntervalsMins.sampling));
	debug_print("\n\r");
	debug_print("upload interval = ");
	debug_print(itoa_pad(g_pDevCfg->sIntervalsMins.upload));
	debug_print("\n\r");
	break;
      case 1:
	if(tz < 0) {
	    tz = tz * -1;
	    negateTz = 1;
	}
	d1 = (int)tz;
	d2 = (tz - d1)*100;
	d1 = d1 *4;
	if(d2 > 74) {
	    d1 += 3;
	} else if(d2 > 49) {
	    d1 += 2;
	} else if(d2 > 24) {
	    d1 += 1;
	}
	if(negateTz) {
	    d1 = d1 * -1;
	}
	if(d1 > 68 || d1 < -48) {
	    return -1;
	}
	g_ConfigDevice.cfgTimeZone = d1;
	debug_print("Timezone = ");
	debug_print(itoa_pad(g_ConfigDevice.cfgTimeZone));
	debug_print("\n\r");

	break;
      case 2:
	value = parse_value(value,&val);
	g_ConfigDevice.stAlarmRepeatParam.repeat_interval_secs = MINUTES_(val);
	value = parse_value(value,&val);
	g_ConfigDevice.stAlarmRepeatParam.repeat_count_max = val;
	debug_print("alarm repeat minute=");
	debug_print(itoa_pad(g_pDevCfg->stAlarmRepeatParam.repeat_interval_secs));
	debug_print("\n\r");
	debug_print("alarm repeat times=");
	debug_print(itoa_pad(g_pDevCfg->stAlarmRepeatParam.repeat_count_max));
	debug_print("\n\r");

	g_pSysState->count_batteryWarn = 0;
	break;
      case 3:
	value = parse_value(value,&val);
	g_ConfigDevice.stBattPowerAlertParam.minutesPower = val;
	value = parse_value(value,&val);
	g_ConfigDevice.stBattPowerAlertParam.battThreshold = val;
	debug_print("low battery alarm %=");
	debug_print(itoa_pad(g_pDevCfg->stBattPowerAlertParam.battThreshold));
	debug_print("\n\r");
	debug_print("power outage delay=");
	debug_print(itoa_pad(g_pDevCfg->stBattPowerAlertParam.minutesPower));
	debug_print("\n\r");

	break;
      case 4:
	if (*value != '+') {
	    goto finish;
	}
	for(j=0;j<5;j++){
	    i=0;
	    while(*value != ':') {
		BUFF[i++] = *value;
		value++;
		if(*value == '\0' || *value == ',') {
		    BUFF[i] = '\0';
		    strcpy(g_ConfigDevice.cfgSMSNumbers[j++].cfgReportSMS,BUFF);
		    goto finish;
		}
	    }
	    BUFF[i] = '\0'; value++;
	    strcpy(g_ConfigDevice.cfgSMSNumbers[j].cfgReportSMS,BUFF);
	}

      finish:
	while (j < 5) {
	    memset(g_ConfigDevice.cfgSMSNumbers[j].cfgReportSMS,'\0',sizeof(g_ConfigDevice.cfgSMSNumbers[j].cfgReportSMS));
	    j++;
	}
	for(int index=0; index < MAX_SMS_NUMBERS; index++)  {
	    debug_print("number = ");
	    debug_print(g_pDevCfg->cfgSMSNumbers[index].cfgReportSMS);
	    debug_print("\n\r");
	}

	break;

      case 5:
	//http version has sensorId in front of alarm values
	//which we can assume mutually exclusive to sms format
	//    if((value[0] > 'A' - 1) && (value[0] < 'E' + 1)) {
	//        i = value[0] - 'A';
	//        value += 2; //skip sensorId and colon
	//    }
	//    else if((value[0] > 'a' - 1) && (value[0] < 'e' + 1)) {
	//        i = value[0] - 'a';
	//        value += 2; //skip sensorId and colon
	//    }

	i = id_sel(value[0]);
	if (i!= -1) value += 2;

	value = parse_value_float(value,&temp_hot_float);
	value = parse_value(value,&dur_hot);
	value = parse_value_float(value,&temp_cold_float);
	value = parse_value(value,&dur_cold);

	if(i < 0) {
	    al_flag = 1;
	} else { //http version has sensorId so we don't need to parse after the fact
	    set_sen(i);
	    //        TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[i];
	    //        pAlertParams->maxSecondsCold = MINUTES_(dur_cold); // We work in seconds internally
	    //        pAlertParams->threshCold = temp_cold;
	    //        pAlertParams->maxSecondsHot = MINUTES_(dur_hot); // We work in seconds internally
	    //        pAlertParams->threshHot = temp_hot;

	}

	break;
      case 6:
	//parse_config_pull(value, g_pSysState->type);//1 for reading the configuration
	break;
      case 7:
	memset(g_ConfigDevice.cfgGatewaySMS,'\0',sizeof(g_ConfigDevice.cfgGatewaySMS)); //null term and cat with space for the termination
	strcpy(g_ConfigDevice.cfgGatewaySMS, value);
	debug_print("gateway=");
	debug_print(g_pDevCfg->cfgGatewaySMS);
	debug_print("\n\r");
	break;
      case 8:
	memset(g_ConfigDevice.cfgGatewayIP,'\0',sizeof(g_ConfigDevice.cfgGatewayIP));
	strcpy(g_ConfigDevice.cfgGatewayIP, value);
	strcpy(flash_memory.server_url, g_ConfigDevice.cfgGatewayIP);
	update_put_get_idx();
	debug_print("ip address=");
	debug_print(g_pDevCfg->cfgGatewayIP);
	debug_print("\n\r");
	break;
      case 9:
	memset(g_ConfigDevice.cfgMsgSenderId,'\0',sizeof(g_ConfigDevice.cfgMsgSenderId));
	strcpy(g_ConfigDevice.cfgMsgSenderId, value);
	debug_print("sender ID=");
	debug_print(g_pDevCfg->cfgMsgSenderId);
	debug_print("\n\r");

	break;
      case 10:
	memset(g_ConfigDevice.cfgGatewayKey,'\0',sizeof(g_ConfigDevice.cfgGatewayKey));
	strcpy(g_ConfigDevice.cfgGatewayKey, value);
	debug_print("sms gateway key=");
	debug_print(g_pDevCfg->cfgGatewayKey);
	debug_print("\n\r");

	break;
      case 11:
	g_ConfigDevice.cfgUploadMode = atoi(value);
	//debug_print("value = ");
	//g_ConfigDevice.cfgUploadMode = 1;              // rachit testing with hardcoded value working
	debug_print("upload mode = ");
	debug_print(itoa_pad(g_pDevCfg->cfgUploadMode));       // rachit debug // before
	debug_print("\n\r");

	break;
      case 12:
	g_ConfigDevice.cfg.logs.buzzer_disable =  atoi(value);
	//    g_ConfigDevice.cfg.status |= atoi(value) << 6;
	break;

      case 13:
	strcpy(g_ConfigDevice.SIM[0].cfgAPN, value);
	debug_print("cfg apn sim 1= ");
	//      debug_print(g_pDevCfg->SIM[0]);
	debug_print("\n\r");

	break;

      case 14:
	strcpy(g_ConfigDevice.SIM[1].cfgAPN, value);
	debug_print("cfg apn sim 2= ");
	//     debug_print(g_pDevCfg.SIM[1]->cfgAPN);
	debug_print("\n\r");

	break;

      case 15:
	sensorId = *value;
	debug_print("Sensor ID= ");
	debug_print(itoa_pad(sensorId));
	debug_print("\n\r");

	break;

      case 17:
	strcpy(g_ConfigDevice.ser_url, value);
	crc = strstr(value, "crc");
	crc = crc+4;
	strcpy(g_ConfigDevice.crc, crc);

	addr = strrchr(value,';');
	len = addr - crc ;
	g_ConfigDevice.crc[len] = '\0';

	strcpy(g_ConfigDevice.ver, addr+1);
	g_ConfigDevice.ver[3] = '\0';

	addr = strrchr(value,':');
	len = addr - value;
	g_ConfigDevice.ser_url[len] = '\0'; //getting url

	g_pSysState->type = 2;
	//watchdog_timer_touch();
	//parse_config_pull(g_ConfigDevice.ser_url, 2);
	break;

      default:
	// lcd_printf(LINEC, "ERR=%s", value);
	// delay(1000);
	break;
    }
    return 0;
}
//relies on strtok being initated to
//start with first key at the beginning
int8_t process_kvp(const char* key, const char* val)
{
    int8_t i=0;
    //       char res = 0, res1 = 0;

    //sachin  if(key == NULL || val == NULL) return 1;
    for(i=0;i<=20;i++) {
	if(!strcmp(g_keyval[i],key)){// || (res = (!strcmp(g_mode[i], key))) || (res1 = (!strcmp(g_mode[i], key)))) {
	    if(strlen(val)) {
		//              if(res || res1)
		//                i = ;

		return setvalue(i, val);//parse function can signal end of overall parsing

	    } else
		return 1;
	}
    }
    return 0;
}


/*	for(i=0;i<sizearray(g_KeyValuePipe);i++){
if(!strcmp(g_KeyValuePipe[i].key,key)){
if (strlen(val))
return g_KeyValuePipe[i].setvalue(val); //parse function can signal end of overall parsing
		    else
return 1;
		}
	}

for(i=0;i<sizearray(g_KeyValueString);i++){
if(!strcmp(g_KeyValueString[i].key,key)){
g_KeyValueString[i].value[0] = '\0'; //null term and cat with space for the termination
strncat(g_KeyValueString[i].value, val, g_KeyValueString[i].size - 1);
		}
	}

for(i=0;i<sizearray(g_KeyValueInt);i++){
if(!strcmp(g_KeyValueInt[i].key,key) && strlen(val)){
//lcd_printf(LINEC,"Match:%s",g_KeyValueInt[i].key);delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
*g_KeyValueInt[i].value = atoi(val) + g_KeyValueInt[i].mod;
		}
	}

for(i=0;i<sizearray(g_KeyValueBitFlag);i++){
if(!strcmp(g_KeyValueBitFlag[i].key,key)){
int flag = parse_test_bool(val);

if(g_KeyValueBitFlag[i].bit < 8) { //only supports 8bit fields!!
if(flag == 1) {
*(g_KeyValueBitFlag[i].bitField) |= 1 << g_KeyValueBitFlag[i].bit;
			} else if(flag == 0) {
*(g_KeyValueBitFlag[i].bitField) &= ~(1 << g_KeyValueBitFlag[i].bit);
			}
		    }
		}
	}
return 0;


}*/

void process_sId(){
    uint8_t Id;
    //	TEMP_ALERT_PARAM *pAlertParams;
    /*	int8_t i=0;
    //lcd_printl(LINEC,"Processing sId");delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
    if(strcmp(sensorId,"\0")) {
    while (i < SYSTEM_NUM_SENSORS) {
    pAlertParams = &g_pDevCfg->stTempAlertParams[i];
    pAlertParams->maxSecondsCold = MINUTES_(dur_cold); // We work in seconds internally
    pAlertParams->threshCold = temp_cold;
    pAlertParams->maxSecondsHot = MINUTES_(dur_hot); // We work in seconds internally
    pAlertParams->threshHot = temp_hot;
    i++;
}
} else {
    if(sensorId[0] > 64 && sensorId[0] < 70)
    i = sensorId[0] - 65;
		else if(sensorId[0] > 96 && sensorId[0] < 102)
    i = sensorId[0] - 97;
		else
    return;
    pAlertParams = &g_pDevCfg->stTempAlertParams[i];
    pAlertParams->maxSecondsCold = MINUTES_(dur_cold); // We work in seconds internally
    pAlertParams->threshCold = temp_cold;
    pAlertParams->maxSecondsHot = MINUTES_(dur_hot); // We work in seconds internally
    pAlertParams->threshHot = temp_hot;
}
    strcpy(sensorId,"\0");
    */
    //        if((sensorId > 'A' - 1) && (sensorId < 'E' + 1))
    //        Id = sensorId - 'A';
    //
    //        else if((sensorId > 'a' - 1) && (sensorId < 'e' + 1))
    //        Id = sensorId - 'a';
    Id = id_sel(sensorId);
    if(Id < 0)
	return;

    al_flag = 0;
    set_sen(Id);
    //sachin        pAlertParams = &g_pDevCfg->stTempAlertParams[Id];
    //        	        pAlertParams->maxSecondsCold = MINUTES_(dur_cold); // We work in seconds internally
    //               		pAlertParams->threshCold = temp_cold;
    //             	   	pAlertParams->maxSecondsHot = MINUTES_(dur_hot); // We work in seconds internally
    //               		pAlertParams->threshHot = temp_hot;
}

//int8_t showValue(const char *key, const char *value) {
//	lcd_printf(LINEC,"K=%s,V=%s",value);delay(1000);
//	return 0;
//}

const char* parse_value(const char *token,int16_t *temp){
    char val[5];
    int8_t i=0;
    while(*token != ':') {
	val[i++] = *token;
	token++;
	if(*token == '\0')
	    break;
    }
    token++;
    val[i] = '\0';
    *temp = atoi(val);
    return token;

}

const char* parse_value_float(const char *token,float *temp){
    char val[5];
    int8_t i=0;
    while(*token != ':') {
	val[i++] = *token;
	token++;
	if(*token == '\0')
	    break;
    }
    token++;
    val[i] = '\0';
    *temp = atof(val);
    return token;
}

//void event_next_by_id(EVENT_IDS id);
/*
int8_t setIn(const char *value){
int16_t interval=1;
//lcd_printl(LINEC,"Set Interval");delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
value = parse_value(value,&interval);
g_pDevCfg->sIntervalsMins.sampling = interval;
event_setInterval_by_id_secs(EVT_SUBSAMPLE_TEMP, MINUTES_(interval));
event_next_by_id(EVT_SUBSAMPLE_TEMP);
event_setInterval_by_id_secs(EVT_SAVE_SAMPLE_TEMP, MINUTES_(interval));
event_next_by_id(EVT_SAVE_SAMPLE_TEMP);
value = parse_value(value,&interval);
g_pDevCfg->sIntervalsMins.upload = interval;
event_setInterval_by_id_secs(EVT_UPLOAD_SAMPLES, MINUTES_(interval));
event_next_by_id(EVT_UPLOAD_SAMPLES);
return 0;
}*/

/*sachin int8_t parse_alarm_var(const char *value) {
int8_t i = -1;

//http version has sensorId in front of alarm values
//which we can assume mutually exclusive to sms format
if((value[0] > 'A' - 1) && (value[0] < 'E' + 1)) {
i = value[0] - 'A';
value += 2; //skip sensorId and colon
    }
    else if((value[0] > 'a' - 1) && (value[0] < 'e' + 1)) {
i = value[0] - 'a';
value += 2; //skip sensorId and colon
    }

//lcd_printl(LINEC,"Parse Alarm Var");delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
value = parse_value(value,&temp_hot);
value = parse_value(value,&dur_hot);
value = parse_value(value,&temp_cold);
value = parse_value(value,&dur_cold);

if(i < 0) {
al_flag = 1;
    } else { //http version has sensorId so we don't need to parse after the fact
TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[i];
pAlertParams->maxSecondsCold = MINUTES_(dur_cold); // We work in seconds internally
pAlertParams->threshCold = temp_cold;
pAlertParams->maxSecondsHot = MINUTES_(dur_hot); // We work in seconds internally
pAlertParams->threshHot = temp_hot;

    }
return 0;
}

int8_t parse_phone_number(const char *phone_number) {
char BUFF[GW_MAX_LEN + 1];
int8_t i = 0,j = 0;

if (*phone_number != '+') {
goto finish;
	}
for(j=0;j<5;j++){
i=0;
while(*phone_number != ':') {
BUFF[i++] = *phone_number;
phone_number++;
if(*phone_number == '\0' || *phone_number == ',') {
BUFF[i] = '\0';
strcpy(g_ConfigDevice.cfgSMSNumbers[j++].cfgReportSMS,BUFF);
goto finish;
		}
	}
BUFF[i] = '\0'; phone_number++;
strcpy(g_ConfigDevice.cfgSMSNumbers[j].cfgReportSMS,BUFF);
	}

finish:
while (j < 5) {
g_ConfigDevice.cfgSMSNumbers[j].cfgReportSMS[0] = '\0';
j++;
  }
return 0;
}

int8_t parse_time_zone(const char *value) {
float tz = atof(value);
int8_t negateTz = 0;
int d1;
float d2;

if(tz < 0) {
tz = tz * -1;
negateTz = 1;
	}
d1 = (int)tz;
d2 = (tz - d1)*100;
d1 = d1 *4;
if(d2 > 74) {
d1 += 3;
	} else if(d2 > 49) {
d1 += 2;
	} else if(d2 > 24) {
d1 += 1;
	}

if(negateTz) {
d1 = d1 * -1;
	}

if(d1 > 68 || d1 < -48) {
return -1;
	}
g_ConfigDevice.cfgTimeZone = d1;

return 0;
}

int8_t parse_battery_alarm_variable(const char *value) {

int16_t val;
value = parse_value(value,&val);
g_ConfigDevice.stBattPowerAlertParam.minutesPower = val;
value = parse_value(value,&val);
g_ConfigDevice.stBattPowerAlertParam.battThreshold = val;
return 0;
}

int8_t parse_alarm_repeat_variable(const char *value) {

int16_t val;
value = parse_value(value,&val);
g_ConfigDevice.stAlarmRepeatParam.repeat_interval_secs = MINUTES_(val);
value = parse_value(value,&val);
g_ConfigDevice.stAlarmRepeatParam.repeat_count_max = val;
return 0;
}
*/
const char *g_HTTPheader = "http://";
const char *g_HTTPSheader = "https://";
#define PULL_URL_BUF_SIZE 50 //just for buffer checking
int8_t parse_config_pull(const char *value, int type) {
    char szTemp[PULL_URL_BUF_SIZE]; //for holding temp gen'd null-term strings
    int szTemp_c = 0;
    uint8_t address_end = 0;

    //prepare string buffer
    szTemp[0] = '\0';
    szTemp_c = PULL_URL_BUF_SIZE;

    //match and remove http header
    if(strstr(value, g_HTTPheader)) {
	value += strlen(g_HTTPheader);
    } else if(strstr(value, g_HTTPSheader)) {
	value += strlen(g_HTTPSheader);
    }

    address_end = strcspn(value, "/"); //get end pos of address
    szTemp_c -= address_end; //account for space of copy
    if(szTemp_c < 1) { //check space before strncat
	config_url_error_show();
	return 1; //error - stop parsing
    }
    strncat(szTemp, value, address_end); //append only server address to empty string
    value += address_end; //move to path portion of URL

    //reconfigure for this endpoint and continue pull if successfull
    uart_txf("AT#HTTPCFG=1,\"%s\",80,0,,,0\r\n", szTemp);
    if (uart_getTransactionState() == UART_SUCCESS) {
	szTemp[0] = '\0';
	szTemp_c = PULL_URL_BUF_SIZE;
	address_end = strcspn(value, " \r\n"); //get end pos of path
	if(address_end > szTemp_c - 1) { //check space before strncat
	    config_url_error_show();
	    return 1; //error - stop parsing
	}
	strncat(szTemp, value, address_end); //copy path
	config_pull_configuration(szTemp, type);
    }

    return 1; //always break kvp parsing with this command
}
#undef PULL_URL_BUF_SIZE

#define SECTION_SYSTEM "SYSTEM"
#define SECTION_SERVER "SERVER"
#define SECTION_INTERVALS "INTERVALS"
#define SECTION_LOGS "LOGS"

#ifdef USE_MININI

// This functionality doesnt work yet
#ifdef CONFIG_SAVE_IN_PROGRESS
void config_save_ini() {
    long n;
    INTERVAL_PARAM *intervals;

    //f_rename(CONFIG_INI_FILE, "thermal.old");
    lcd_printf(LINEC, get_string(LCD_MSG, SAVING_CONFIG));

    n = ini_puts(SECTION_SERVER, "GatewaySMS", g_pDevCfg->cfgGatewaySMS, CONFIG_INI_FILE);
    n = ini_puts(SECTION_SERVER, "GatewayIP", g_pDevCfg->cfgGatewayIP, CONFIG_INI_FILE);
    n = ini_puts(SECTION_SERVER, "Config_URL", g_pDevCfg->cfgConfig_URL, CONFIG_INI_FILE);
    n = ini_puts(SECTION_SERVER, "Upload_URL", g_pDevCfg->cfgUpload_URL, CONFIG_INI_FILE);

    n = ini_puts("SIM1", "APN", g_pDevCfg->SIM[0].cfgAPN, CONFIG_INI_FILE);
    n = ini_puts("SIM2", "APN", g_pDevCfg->SIM[1].cfgAPN, CONFIG_INI_FILE);

    intervals = &g_pDevCfg->sIntervalsMins;
    n = ini_putl(SECTION_INTERVALS, "Sampling", intervals->sampling, CONFIG_INI_FILE);
    if (n == 0)
	return;

    n = ini_putl(SECTION_INTERVALS, "Upload", intervals->upload, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "Reboot", intervals->systemReboot, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "Configuration", intervals->configurationFetch, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "SMS_Check", intervals->smsCheck, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "Network_Check", intervals->networkCheck, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "LCD_off", intervals->lcdOff, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "Alarms", intervals->alarmsCheck, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "ModemPullTime", intervals->modemPullTime, CONFIG_INI_FILE);
    n = ini_putl(SECTION_INTERVALS, "BatteryCheck", intervals->batteryCheck, CONFIG_INI_FILE);
}
#endif

FRESULT config_read_ini_file() {
    if(g_bServiceMode == true) {
	config_load_profile(g_ServiceModeConfigProfile);
	//indicate success in service mode
	lcd_printl(LINEC, get_string(LCD_MSG, SDCARD_SUCCESS));
	delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
    } else {
	config_load_profile(g_DefaultConfigProfile);
    }

    return FR_OK;
}

#endif

/* get APN based on MCC and MNC from apn.ini file*/
uint8_t config_get_apn(const char *buf, int8_t SIM_slot)
{
    uint8_t i;
    char mcc[4],mnc[4];

    while(*buf++ != ' ');

    for(i=0;i<3;i++){
	mcc[i] = *buf;
	buf++;
	if(*buf == ','){
	    mcc[i+1] = '\0';
	    buf++;
	    break;
	}
    }

    for(i=0;i<3;i++){
	mnc[i] = *buf;
	buf++;
	if(*buf == ','){
	    mnc[i+1] = '\0';
	    break;
	}
    }

    i = ini_gets(mcc, mnc, g_pDevCfg->SIM[SIM_slot].cfgAPN, g_pDevCfg->SIM[SIM_slot].cfgAPN,
		 sizeof(g_pDevCfg->SIM[SIM_slot].cfgAPN), APN_INI_FILE);
    return i;
}

int config_default_configuration() {
    int i = 0;
    TEMP_ALERT_PARAM *alert;
    BATT_POWER_ALERT_PARAM *power;
    ALARM_REPEAT_PARAM *repeat;
    for (i = 0; i < SYSTEM_NUM_SENSORS; i++) {
	alert = &g_pDevCfg->stTempAlertParams[i];

	alert->maxSecondsCold = MINUTES_(ALARM_LOW_TEMP_PERIOD);
	alert->maxSecondsHot = MINUTES_(ALARM_HIGH_TEMP_PERIOD);

	alert->threshCold = LOW_TEMP_THRESHOLD;
	alert->threshHot = i > 2 ? HIGH_TEMP_THRESHOLD_OUTSIDE : HIGH_TEMP_THRESHOLD;
    }

    power = &g_pDevCfg->stBattPowerAlertParam;
    power->flags.bits.enablePowerAlert = POWER_ENABLE_ALERT;
    power->minutesPower = ALARM_POWER_PERIOD;
    power->battThreshold = BATTERY_ALARM_THRESHOLD;

    repeat = &g_pDevCfg->stAlarmRepeatParam;
    repeat->repeat_interval_secs = MINUTES_(ALARM_REPEAT_INTERVAL);
    repeat->repeat_count_max = ALARM_REPEAT_COUNT;

    //log defaults
    g_pDevCfg->cfg.logs.system_log = LOG_DEFAULT_SYSTEM;
    g_pDevCfg->cfg.logs.web_csv =	LOG_DEFAULT_WEB;
    g_pDevCfg->cfg.logs.server_config = LOG_DEFAULT_CONFIG;
    g_pDevCfg->cfg.logs.modem_transactions = LOG_DEFAULT_MODEM;

    //alert default
    g_pDevCfg->cfg.logs.sms_alerts = ALERTS_SMS;

    return 1;
}
