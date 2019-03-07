#include "thermalcanyon.h"
#include "fatdata.h"
#include "main_system.h"
#include "alarms.h"
#include "modem.h"
#include "rtc.h"
#include "spi_flash.h"
#include "device_alarms.h"
#include "apn_config.h"
#include "config.h"
#include "fridge_uart.h"
#include "http.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "debug.h"

#include "stdint.h"
#include "modem_uart.h"
//#include "modem.h"
#include "battery.h"
#include "main_system.h"
#include "temperature.h"
#include "watchdog.h"
#include "time.h"
#include "timer.h"
#include "config.h"
#include "events.h"
#include "rtc.h"
#include "device_alarms.h"


bool fatdone = false;

//char* fridge_new;        

#ifndef _FISM_
const char ctrlZ[2] = { 0x1A, 0 };
const char ESC[2] = { 0x1B, 0 };

#else
extern d_values dvalue;
extern void add_sec_in_date_time(char *ptr_data_time_str,signed int second);
extern unsigned g_uch_status;
//extern uint32_t local_time;
char device_imei[20];
extern unsigned int rec_buffer_index;
extern unsigned char fw_version[];
char ccid[22];
char gsm_fw[20];
char device_apn[32] = {"www"};
unsigned char wake_gsm = 0;

#define ACTUAL_EPOCH    1512046471
#endif
uint8_t g_lastSIMErrors_c;
SIM_ERROR_ENTRY g_lastSIMErrors[ERRORS_CACHE_SLOTS];

/*
* AT Commands Reference Guide 80000ST10025a Rev. 9 2010-10-04
*
* Read command reports the <mode> and <stat> parameter values in the format:
* +CREG: <mode>,<stat>[,<Lac>,<Ci>]
*
* <mode>
* 0 - disable network registration unsolicited result code (factory default)
* 1 - enable network registration unsolicited result code
* 2 - enable network registration unsolicited result code with network Cell identification data
*
* If <mode>=1, network registration result code reports:
* +CREG: <stat> where <stat>
*  0 - not registered, ME is not currently searching a new operator to register to
*  1 - registered, home network
*  2 - not registered, but ME is currently searching a new operator to register to
*  3 - registration denied
*  4 -unknown
*  5 - registered, roaming
*
*  If <mode>=2, network registration result code reports:
*  +CREG: <stat>[,<Lac>,<Ci>]
*  where:
*  <Lac> - Local Area Code for the currently registered on cell
*  <Ci> - Cell Id for the currently registered on cell
*
*
*  Note: <Lac> and <Ci> are reported only if <mode>=2 and the mobile is registered on some network cell.
*/

//const char NETWORK_MODE_0[] = "Net disabled";
const char * const NETWORK_STATUS[8] = { "initializing", "connected",
"searching net", "reg denied", "unknown state", "roaming net", "Net disabled", "Unknown" };
const char NETWORK_MODE_2[] = "Registered";
//const char NETWORK_UNKNOWN_STATUS[] = "Unknown";
const char * const NETWORK_SERVICE_TEXT[2] = { "GSM", "GPRS" };

/*
CME ERROR: 10	SIM not inserted
CME ERROR: 11	SIM PIN required
CME ERROR: 12	SIM PUK required
CME ERROR: 13	SIM failure
CME ERROR: 15	SIM wrong
CME ERROR: 30	No network service
*/

uint16_t CME_fatalErrors[] = { 10, 11, 12, 13, 15, 30 };

/*
CMS ERROR: 30	Unknown subscriber
CMS ERROR: 38	Network out of order
*/
uint16_t CMS_fatalErrors[] = { 10, 30, 38, 310 };

// Check against all the errors that could kill the SIM card operation
uint8_t modem_check_working_SIM() 	// Not used in FISM --> NI for Demo
{
    int t = 0;

    if (!state_isSimOperational())
	return false;

    char token = '\0';
    uint16_t lastError = config_getSimLastError(&token);
    if (token == 'S')
	for (t = 0; t < sizeof(CMS_fatalErrors) / sizeof(uint16_t); t++)
	    if (CMS_fatalErrors[t] == lastError) {
		state_SIM_not_operational();
		return false;
	    }

    if (token == 'E')
	for (t = 0; t < sizeof(CME_fatalErrors) / sizeof(uint16_t); t++)
	    if (CME_fatalErrors[t] == lastError) {
		state_SIM_not_operational();
		return false;
	    }

    state_SIM_operational();
    return true;
}

const char *modem_getNetworkStatusText(int mode, int status) {	// Need to check

    if (mode == 0) {
	//	state_setNetworkStatus(NETWORK_UNKNOWN_STATUS);
	state_setNetworkStatus(NETWORK_STATUS[7]);
	//sachin	return NETWORK_MODE_0;
	return NETWORK_STATUS[6];
    }

    if (status >= 0 && status < 6) {
	state_setNetworkStatus(NETWORK_STATUS[status]);
	return NETWORK_STATUS[status];
    }

    //state_setNetworkStatus(NETWORK_UNKNOWN_STATUS);
    state_setNetworkStatus(NETWORK_STATUS[7]);
    return NETWORK_STATUS[7];
    //        return NETWORK_UNKNOWN_STATUS;
}

int modem_getNetworkStatus(int *mode, int *status) {	 	// Not used in FISM

    char *pToken1 = NULL;
    int uart_state = 0;
    int verbose;
    char command_result[16];

    SIM_CARD_CONFIG *sim = config_getSIM();

    *mode = -1;
    *status = -1;

    verbose = lcd_getVerboseMode();
    lcd_disable_verbose();

    sprintf(command_result, "+%s: ", modem_getNetworkServiceCommand());

    uart_txf("+%s?", modem_getNetworkServiceCommand());

    lcd_setVerboseMode(verbose);

    uart_state = uart_getTransactionState();
    if (uart_state == UART_SUCCESS) {

	PARSE_FINDSTR_RET(pToken1, command_result, UART_FAILED);

	PARSE_FIRSTVALUE(pToken1, mode, ",\n", UART_FAILED);
	PARSE_NEXTVALUE(pToken1, status, ",\n", UART_FAILED);

	// Store the current network status to know if we are connected or not.
	sim->networkMode = *mode;
	sim->networkStatus = *status;

	return UART_SUCCESS;
    }
    return UART_FAILED;
}

/**********************************************************************************************************************/
/* Network cellular service */
/**********************************************************************************************************************/

const char NETWORK_GPRS_COMMAND[] = "CGREG";
const char NETWORK_GSM_COMMAND[] = "CREG";

const char inline *modem_getNetworkServiceCommand() {	// Need to check
    if (g_pSysState->network_mode == NETWORK_GPRS)
	return NETWORK_GPRS_COMMAND;

    return NETWORK_GSM_COMMAND;
}

const char inline *modem_getNetworkServiceText() {	// Need to check
    switch (g_pSysState->network_mode) {
      case NETWORK_GSM:
	return NETWORK_SERVICE_TEXT[NETWORK_GSM];
      case NETWORK_GPRS:
	return NETWORK_SERVICE_TEXT[NETWORK_GPRS];
    }

    //	return NETWORK_UNKNOWN_STATUS;
    return NETWORK_STATUS[7];
}

int modem_getNetworkService() {		//Need to check
    if (g_pSysState->network_mode < 0 || g_pSysState->network_mode > 1)
	return NETWORK_GSM;

    return g_pSysState->network_mode;
}

void modem_setNetworkService(int service) {	//Need to check
    if (g_pSysState->network_mode == service)
	return;

    g_pSysState->network_mode = service;
    //	config_setLastCommand(COMMAND_SET_NETWORK_SERVICE);

    if (modem_connect_network(NETWORK_CONNECTION_ATTEMPTS) == UART_FAILED) {
#ifndef _FISM_
	log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_NETFAIL), modem_getNetworkServiceText());
#endif
    }
}

/*void modem_network_sequence() {
//uint8_t networkSwapped = 0;

// Checks if the current sim is the selected one.
//modem_check_sim_active();


//check network
//	if(modem_check_network() != UART_SUCCESS){
//		//failed, leave
//		return;
//	}

//check http while we're here
//	if (http_enable() != UART_SUCCESS){
//		//store that we have failed gprs
//		state_failed_gprs(config_getSelectedSIM());
//	}



//config_setLastCommand(COMMAND_FAILOVER);
if (modem_check_network() != UART_SUCCESS) {
modem_swap_SIM();
networkSwapped = 1;
if (modem_check_network() != UART_SUCCESS) {
return;
		}
	}

if (g_pDevCfg->cfgUploadMode != MODE_GSM && state_isGPRS()
&& http_enable() != UART_SUCCESS) {
//	config_setLastCommand(COMMAND_FAILOVER_HTTP_FAILED);

// This means we already checked the network
if (networkSwapped) {
http_deactivate();
return;
		}

modem_swap_SIM();
if (modem_check_network() == UART_SUCCESS
&& http_enable() != UART_SUCCESS) {
state_failed_gprs(config_getSelectedSIM());
		}
	}
*/

//http_deactivate();
//}

#define MODEM_ERROR_SIM_NOT_INSERTED 10

int modem_connect_network(uint8_t attempts) {	// Need to implement for demo
    char token;
    int net_status = 0;
    int net_mode = 0;
    int tests = 0;
    int nsim = config_getSelectedSIM();

    /* PIN TO CHECK IF THE SIM IS INSERTED IS NOT CONNECTED, CPIN WILL NOT RETURN SIM NOT INSERTED */
    // Check if the SIM is inserted
    uart_tx("+CPBW=?");
    int uart_state = uart_getTransactionState();
    if (uart_state == UART_ERROR) {
	if (config_getSimLastError(&token) == MODEM_ERROR_SIM_NOT_INSERTED)
	    return UART_ERROR;
    }

    if (!state_isSimOperational())
	return UART_ERROR;

    // enable network registration and location information unsolicited result code;
    // if there is a change of the network cell. +CGREG: <stat>[,<lac>,<ci>]

    //	config_setLastCommand(COMMAND_NETWORK_CONNECT);

    uart_txf("+%s=2", modem_getNetworkServiceCommand());
#ifndef _FISM_
    //aditya delay(1000);
#endif
    do {
	if (modem_getNetworkStatus(&net_mode, &net_status) == UART_SUCCESS) {
#ifndef _FISM_
	    if (!g_iRunning || net_status != 1) {
		lcd_printf(LINEC, "SIM %d %s", nsim + 1,
			   modem_getNetworkServiceText());
		lcd_printl(LINEH,
			   (char *) modem_getNetworkStatusText(net_mode,
							       net_status));

	    }
#endif
	    state_network_status(net_mode, net_status);

	    // If we are looking for a network
	    if ((net_mode == 1 || net_mode == 2)
		&& (net_status == NETWORK_STATUS_REGISTERED_HOME_NETWORK
		    || net_status == NETWORK_STATUS_REGISTERED_ROAMING)) {

			// TODO: If network is roaming don't connect
			//				state_network_success(nsim);

			// We tested more than once, lets show a nice we are connected message
			if (tests > 4)
#ifndef _FISM_
			    delay(HUMAN_DISPLAY_INFO_DELAY);
#endif
			return UART_SUCCESS;
		    } else {
			//				if (g_pSysState->network_mode == NETWORK_GPRS) {
			//					state_failed_gprs(g_pDevCfg->cfgSIM_slot);
			//				} else {
			//					state_failed_gsm(g_pDevCfg->cfgSIM_slot);
			//				}
		    }

	    tests++;
	}
#ifndef _FISM_
	lcd_progress_wait(NETWORK_CONNECTION_DELAY);
#endif
    } while (--attempts > 0);

    //config_incLastCmd();
    return UART_FAILED;
}
;

int modem_disableNetworkRegistration() {
    return uart_txf("+%s=0\r\n", modem_getNetworkServiceCommand());
}

uint8_t g_lastSIMErrors_h = 0;
uint8_t g_lastSIMErrors_t = 0;

void modem_setLastErrorsCache(char errorToken, uint16_t errorCode) {
    char *curSlot = g_lastSIMErrors[g_lastSIMErrors_h].errStr;
    sprintf(curSlot, "CM%c%s", errorToken, itoa_nopadding(errorCode));
    g_lastSIMErrors[g_lastSIMErrors_h].simId = g_pDevCfg->cfgSIM_slot;
    g_lastSIMErrors[g_lastSIMErrors_h].timeStamp = rtc_getUTCsecs();
    g_lastSIMErrors_h = (g_lastSIMErrors_h+1) % ERRORS_CACHE_SLOTS;
    memset(&g_lastSIMErrors[g_lastSIMErrors_h], 0, sizeof(SIM_ERROR_ENTRY)); //clear head
    if(g_lastSIMErrors_t == g_lastSIMErrors_h) { //update tail if overflow
	g_lastSIMErrors_t = (g_lastSIMErrors_t+1) % ERRORS_CACHE_SLOTS;
    }
}

const SIM_ERROR_ENTRY * modem_getLastErrorsCacheEntry() {
    SIM_ERROR_ENTRY *curSlot;
    curSlot = &g_lastSIMErrors[g_lastSIMErrors_t];
    if(g_lastSIMErrors_t != g_lastSIMErrors_h) { //only move tail up to head
	g_lastSIMErrors_t = (g_lastSIMErrors_t+1) % ERRORS_CACHE_SLOTS;
    }

    return curSlot;
}

void modem_clearLastErrorsCache() {
    memset(&g_lastSIMErrors, 0, sizeof(g_lastSIMErrors));
    g_lastSIMErrors_h = 0;
    g_lastSIMErrors_t = 0;
}

void modem_logSIMErrorCodes() {
    const SIM_ERROR_ENTRY *curSlot;
    while(g_lastSIMErrors_t != g_lastSIMErrors_h) {
	curSlot = modem_getLastErrorsCacheEntry();
	log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_ERROR), curSlot->errStr);
    }
}

void modem_setNumericError(char errorToken, int16_t errorCode) {
    char szCode[16];

    char token;
    SIM_CARD_CONFIG *sim = config_getSIM();

    //	config_setLastCommand(COMMAND_SIM_ERROR);

    if (config_getSimLastError(&token) == errorCode)
	return;

    // Sim busy (we just have to try again, sim is busy... you know?)
    if (errorCode==14)
	return;

    // Activation failed
    if (errorCode==555)
	return;

    sprintf(szCode, "ERROR %d", errorCode);
    config_setSIMError(sim, errorToken, errorCode, szCode);

    // Check the error codes to figure out if the SIM is still functional
    modem_check_working_SIM();
    return;
}

static const char AT_ERROR[] = " ERROR: ";

// Used to ignore previsible errors like the SIM not supporting a command.
void modem_ignore_next_errors(int error) {
    uart.switches.b.ignoreError = error;
}

void modem_check_uart_error() {
    char *pToken1;
    char errorToken;

    int uart_state = uart_getTransactionState();
    if (uart_state != UART_ERROR)
	return;

    //	config_setLastCommand(COMMAND_UART_ERROR);

    pToken1 = strstr(uart_getRXHead(), (const char *) AT_ERROR);
    if (pToken1 != NULL) { // ERROR FOUND;
	char *error = (char *) (pToken1 + strlen(AT_ERROR));
	errorToken = *(pToken1 - 1);

	if (uart.switches.b.ignoreError == 0) {
#ifndef _FISM_
#ifdef _DEBUG_OUTPUT
	    if (errorToken=='S') {
		lcd_printl(LINEC, "SERVICE ERROR");
	    } else {
		lcd_printl(LINEC, "MODEM ERROR");
	    }
#endif
#endif
	    modem_setNumericError(errorToken, atoi(error));
	    modem_setLastErrorsCache(errorToken, atoi(error));
	}
    }
}

// Makes sure that there is a network working
int8_t modem_check_network() {	// Used in FISM
    int res = UART_FAILED;
    //sachin	int iSignal = 0;
    //int service;

    //	config_setLastCommand(COMMAND_CHECK_NETWORK);

    // Check signal quality,
    // if it is too low we check if we
    // are actually connected to the network and fine
    //service = modem_getNetworkService();

    modem_getSignal();
    res = uart_getTransactionState();
    /*sachin if (res == UART_SUCCESS) {
    state_setSignalLevel(iSignal);
} else {
    state_setSignalLevel(0);
    //g_pSysState->net_service[service].network_failures++;
    //	sachin	state_network_fail(config_getSelectedSIM(),
    //		NETWORK_ERROR_SIGNAL_FAILURE);
    //log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_NETDOWN), config_getSelectedSIM(),
    //		g_pSysState->net_service[service].network_failures);
}*/

    return res;
}


int8_t modem_first_init()
{	// Need to check
    g_pDevCfg->print_en = 0;
    int t = 0;
    int attempts = MODEM_CHECK_RETRY;
    int iSIM_Error = 0;

    modem_clearLastErrorsCache();

#ifndef _FISM_
    //check Modem is powered on. Otherwise wait for a second
    while (!MODEM_ON && --attempts>0)
	delay(100);

    if (!MODEM_ON) {
	lcd_printl(LINEC, get_string(LCD_MSG, MODEM_FAILED));
	//lcd_printl(LINEE, "Power on");
    }
#endif

    //uart_setOKMode();

    lcd_disable_verbose();
    //uart_tx_nowait(ESC); // Cancel any previous command in case we were reseted (not used anymore)
   // uart_tx_timeout("AT", TIMEOUT_DEFAULT, 10); // Loop for OK until modem is ready

    lcd_enable_verbose();

    //sachin	uint8_t nsims = SYSTEM_NUM_SIM_CARDS;

    /*
#ifdef _DEBUG
    nsims = 1;
#endif
    */


    // One or more of the sims had a catastrofic failure on init, set the device
    switch (iSIM_Error) {
      case 1:
	/*
	for (t = 0; t < SYSTEM_NUM_SIM_CARDS; t++)
	if (config_getSIMError(t) == NO_ERROR) {
	if (config_getSelectedSIM() != t) {
	g_pDevCfg->cfgSIM_slot = t;
	g_pDevCfg->cfgSelectedSIM_slot = t;
	modem_init();
    }
    }
	*/

	if (!state_isSimOperational()) {
#ifndef _FISM_
	    log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_SIM_SWP), config_getSelectedSIM());
#endif
	    g_pDevCfg->cfgSIM_slot = !g_pDevCfg->cfgSIM_slot;
	    g_pDevCfg->cfgSelectedSIM_slot = g_pDevCfg->cfgSIM_slot;
#ifndef _FISM_
	    lcd_printf(LINEC, "SIM %d", g_pDevCfg->cfgSIM_slot + 1);
#endif
	    modem_init();
	}
	break;
#ifndef _DEBUG
      case 2:
#ifndef _FISM_
	lcd_printf(LINEE, "SIMS FAILED");
	if (g_bServiceMode == true)
	    delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
#endif
	break;
#endif
    }

    //	data_send_api_packet(API_PACKET_ALARM_CONN);
    //	data_send_api_packet(API_PACKET_ALARM_SENS);
    //	data_send_api_packet(API_PACKET_ALARM_BATT);

    return MODEM_ON;
}

void modem_check_sim_active() {
    if (g_pDevCfg->cfgSIM_slot != g_pDevCfg->cfgSelectedSIM_slot) {
	modem_swap_SIM();
    }
}

/*
int modem_swap_to_SIM(int sim) {
if (g_pDevCfg->cfgSIM_slot != sim) {
return modem_swap_SIM();
	}

return UART_SUCCESS;
}
*/

int modem_swap_SIM() {	// NA
#ifndef _FISM_
    log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_SIM_SWP), config_getSelectedSIM());
#endif
    int res = UART_FAILED;

    g_pDevCfg->cfgSIM_slot = !g_pDevCfg->cfgSIM_slot;
    g_pDevCfg->cfgSelectedSIM_slot = g_pDevCfg->cfgSIM_slot;
#ifndef _FISM_
    lcd_printf(LINEC, "SIM %d", g_pDevCfg->cfgSIM_slot + 1);
#endif
    modem_init();

#ifndef _FISM_
    // Wait for the modem to be ready to send messages
#ifndef _DEBUG
    lcd_progress_wait(1000);
#endif
#endif

    // Just send the message if we dont have errors.
    if (state_isSimOperational()) {
	if (g_pSysCfg->numberConfigurationRuns < 3 && g_pSysCfg->modemFirstInit ||
	    state_SimIdChanged()) {
		SIM_CARD_CONFIG *sim = config_getSIM();
		g_pDevCfg->cfgSIM_slot = !g_pDevCfg->cfgSIM_slot;
		uart_tx_timeout("AT",5000,10);
		uart_tx_timeout("ATZ",10000,3);
		g_pDevCfg->print_en = 1;
		modem_sim_select();
		clear_mem(sim->cfgSimId);
		g_pDevCfg->print_en = 0;
		g_pDevCfg->cfgSIM_slot = !g_pDevCfg->cfgSIM_slot;

		data_send_api_packet(API_PACKET_DEV_READY) ;
		//              lcd_printl(LINEC, "HBT SENT");
		//              delay(1000);
		//               if(g_pDevCfg->cfgSMSmode.bits.BootDeviceReady) sms_send_dataready();
		//                 state_clearSimIdChanged();
	    }

	/*
	if (data_send_api_packet(API_PACKET_DEV_READY) != 0)
	//heartbeat to server
	sms_send_heart_beat();
	//get configsync as per spec
	//backend_get_configuration();
	*/
	res = UART_SUCCESS;
    }
    //        else {
    //		res = UART_FAILED;
    //	}

    return res;
}

const char COMMAND_RESULT_CSQ[] = "CSQ: ";

int modem_getSignal() {
    char *token;
    int iSignalLevel = 0;

    if (uart_tx_timeout("AT+CSQ\r\n", TIMEOUT_CSQ, 1) != UART_SUCCESS)
	return 0;

    PARSE_FINDSTR_RET(token, COMMAND_RESULT_CSQ, UART_FAILED);
    PARSE_FIRSTVALUE(token, &iSignalLevel, ",\n", UART_FAILED);

    //pToken2 = strtok(&pToken1[5], ",");
    g_pSysState->signal_level = iSignalLevel;
    //sachin	state_setSignalLevel(iSignalLevel);
    return iSignalLevel;
}

int8_t modem_parse_string(char *cmd, char *response, char *destination,
			  uint16_t size) {	// Need to check
			      char *token;
			      int8_t uart_state;

			      uart_tx_timeout(cmd, MODEM_TX_DELAY1, 2);

			      uart_state = uart_getTransactionState();
			      if (uart_state == UART_SUCCESS) {
				  PARSE_FINDSTR_RET(token, response, UART_FAILED);
				  token++;
				  PARSE_FIRSTSTRING(token, destination, size, "\" \r\n\0", UART_FAILED);
			      }

			      return uart_state;
			  }

// Reading the Service Center Address to use as message gateway
// http://www.developershome.com/sms/cscaCommand.asp
// Get service center address; format "+CSCA: address,address_type"

/*sachin int8_t modem_getSMSCenter() {
SIM_CARD_CONFIG *sim = config_getSIM();
//return modem_parse_string("AT+CSCA?\r\n", "CSCA: \"", sim->cfgSMSCenter,
//GW_MAX_LEN + 1);

// added for SMS Message center number to be sent in the heart beat
return UART_SUCCESS;
}*/

int8_t modem_getOwnNumber()
{
    SIM_CARD_CONFIG *sim = config_getSIM();
    int8_t state;
    modem_ignore_next_errors(1); // Ignore 1 error since we know our sim cards dont support this command
    state = modem_parse_string("AT+CNUM", "CNUM: \"\",\"", g_pDevCfg->SIM[0].cfgPhoneNum,GW_MAX_LEN + 1);
    //if no phone number is found, it will copy the next comma and number format type - clear in that case
    if(g_pDevCfg->SIM[0].cfgPhoneNum[0] == ',') memset(g_pDevCfg->SIM[0].cfgPhoneNum, 0, sizeof(g_pDevCfg->SIM[0].cfgPhoneNum));
    modem_ignore_next_errors(0);
    return state;
}

int8_t modem_getFirmwareVersion() {
    int8_t state;
    modem_ignore_next_errors(1); // Ignore 1 error since we know our sim cards dont support this command
    state = modem_parse_string("AT#CGMR", "#CGMR: ", g_pDevCfg->ModemFirmwareVersion,
			       FWV_MAX_LEN + 1);
    modem_ignore_next_errors(0);
    return state;
}

int8_t modem_get_Sim_Id()
{
    char lastSimId[SID_MAX_LEN+1];
    SIM_CARD_CONFIG *sim = config_getSIM();
    int8_t state;
    lastSimId[0] = '\0';
    strncat(lastSimId, sim->cfgSimId, sizeof(lastSimId));
    modem_ignore_next_errors(1); // Ignore 1 error since we know our sim cards dont support this command
    state = modem_parse_string("AT+CCID?", "+CCID: ", sim->cfgSimId,SID_MAX_LEN + 1);
    modem_ignore_next_errors(0);
    if(strcmp(lastSimId, sim->cfgSimId)) {
	sim->fSimIdChanged = 1;
    }
    return state;
}

uint8_t validateIMEI(char *IMEI) {
    int t;
    uint8_t len = strlen(IMEI);

    for (t = 0; t < len; t++) {
	if (IMEI[t] == '\r' || IMEI[t] == '\n') {
	    IMEI[t] = 0;
	    break;
	}
	if (IMEI[t] < '0' || IMEI[t] > '9')
	    return 0;
    }

    len = strlen(IMEI);
    if (len < IMEI_MIN_LEN)
	return 0;

    return 1;
}

void modem_getIMEI() {
    // added for IMEI number//
    char IMEI[IMEI_MAX_LEN + 1];
    char *token = NULL;

    uart_tx("+CGSN");
    memset(IMEI, 0, sizeof(IMEI));

    token = strstr(uart_getRXHead(), "OK");
    if (token == NULL)
	return;

    strncpy(IMEI, uart_getRXHead() + 2, IMEI_MAX_LEN);
    if (!validateIMEI(IMEI))
    {
	return;
    }

    if (check_address_empty(g_pDevCfg->cfgIMEI[0]))
    {
	strcpy(g_pDevCfg->cfgIMEI, IMEI);
    }

    // Lets check if we have the right IMEI from the modem, otherwise we flash it again into config.
    if (memcmp(IMEI, g_pDevCfg->cfgIMEI, IMEI_MAX_LEN) != 0)
    {
	strcpy(g_pDevCfg->cfgIMEI, IMEI);
    }

}

void modem_getExtraInfo() {
    modem_getIMEI();
    delay(50);
}

// 2 minutes timeout by the TELIT documentation
#define MAX_CSURV_TIME 120000

#ifdef _DEBUG
#define NETWORK_WAITING_TIME 5000
#define NET_ATTEMPTS 10
#else
#define NETWORK_WAITING_TIME 10000
#define NET_ATTEMPTS 10
#endif

#if defined(CAPTURE_MCC_MNC) && defined(_DEBUG)
void modem_survey_network() {	//  --> NA for demo

    char *pToken1;
    int attempts = NET_ATTEMPTS;
    int uart_state;

    SIM_CARD_CONFIG *sim = config_getSIM();

    if (sim->iCfgMCC != 0 && sim->iCfgMNC != 0)
	return;

    lcd_disable_verbose();

    uart_setDelayIntervalDivider(64);  // 120000 / 64

    do {
	if (attempts != NET_ATTEMPTS)
	    lcd_printf(LINEC, "MCC RETRY %d   ", NET_ATTEMPTS - attempts);
	else
	    lcd_printl(LINEC, "MCC DISCOVER");

	// Displays info for the only serving cell
	uart_tx("#CSURVEXT=0");
	uart_state = uart_getTransactionState();

	if (uart_state == UART_SUCCESS)
	{
	    // We just want only one full buffer. The data is on the first few characters of the stream
	    uart_setNumberOfPages(1);
	    uart_tx_timeout("AT#CSURV", TIMEOUT_CSURV, 10);// #CSURV - Network Survey
	    // Maximum timeout is 2 minutes

	    //Execution command allows to perform a quick survey through channels
	    //belonging to the band selected by last #BND command issue, starting from
	    //channel <s> to channel <e>. If parameters are omitted, a full band scan is
	    //performed.

	    uart_state = uart_getTransactionState();
	    if (uart_state != UART_SUCCESS) {
		lcd_printl(LINE2, "NETWORK BUSY");
		delay(NETWORK_WAITING_TIME);
	    } else {

		pToken1 = strstr(uart_getRXHead(), "ERROR");
		if (pToken1 != NULL) {
		    uart_state = UART_ERROR;
		    //		sachin			lcd_printl(LINE2, "FAILED");
		    //					delay(1000);
		} else {
		    pToken1 = strstr(uart_getRXHead(), "mcc:");
		    if (pToken1 != NULL)
			sim->iCfgMCC = atoi(pToken1 + 5);

		    pToken1 = strstr((const char *) pToken1, "mnc:");
		    if (pToken1 != NULL)
			sim->iCfgMNC = atoi(pToken1 + 5);

		    if (sim->iCfgMCC > 0 && sim->iCfgMNC > 0) {
			lcd_printl(LINEC, "SUCCESS");
			lcd_printf(LINE2, "MCC %d MNC %d", sim->iCfgMCC,
				   sim->iCfgMNC);
		    }
		}
	    }
	}

	attempts--;
    }while (uart_state != UART_SUCCESS && attempts > 0);

    uart_setDefaultIntervalDivider();

    lcd_enable_verbose();
    lcd_progress_wait(10000); // Wait 10 seconds to make sure the modem finished transfering.
    // It should be clear already but next transaction
}
#endif

const char COMMAND_RESULT_CCLK[] = "+CCLK: \"";

int modem_parse_time(struct tm* pTime) {	// Need to check --> in modem_pull_time
    struct tm tmTime;
    char* pToken1 = NULL;
    char* delimiter = "/,:-\"+";
    time_t timeZoneOffset = 0;
    uint8_t negateTz = 0;

    if (!pTime)
    {
	return UART_FAILED;
    }

    PARSE_FINDSTR_RET(pToken1, COMMAND_RESULT_CCLK, UART_FAILED);

    // Find the character before parsing as it will be replaced by \0
    if (strchr(pToken1, '-') == NULL) //time zone offset reported must be flipped in sign in order to produce UTC with gmtime()
	negateTz = 1;

    memset(&tmTime, 0, sizeof(tmTime));
    //string format "yy/MM/dd,hh:mm:ss+-zz"
    PARSE_FIRSTVALUE(pToken1, &tmTime.tm_year, delimiter, UART_FAILED);
    tmTime.tm_year += 2000;
    PARSE_NEXTVALUE(pToken1, &tmTime.tm_mon, delimiter, UART_FAILED);
    PARSE_NEXTVALUE(pToken1, &tmTime.tm_mday, delimiter, UART_FAILED);
    PARSE_NEXTVALUE(pToken1, &tmTime.tm_hour, delimiter, UART_FAILED);
    PARSE_NEXTVALUE(pToken1, &tmTime.tm_min, delimiter, UART_FAILED);
    PARSE_NEXTVALUE(pToken1, &tmTime.tm_sec, delimiter, UART_FAILED);
    PARSE_NEXTVALUE(pToken1, &timeZoneOffset, delimiter, UART_FAILED);

    timeZoneOffset = negateTz == 1 ? -1 * timeZoneOffset * 900 : timeZoneOffset * 900; //quarter hour increments

    if(timeZoneOffset == 0) {
	timeZoneOffset = g_ConfigDevice.cfgTimeZone * -900;
	tmTime.tm_year -= 1900;
	tmTime.tm_mon -= 1;
	tmTime.tm_sec += g_ConfigDevice.cfgTimeZone * 900;
	mktime(&tmTime);
	tmTime.tm_year += 1900;
	tmTime.tm_mon += 1;
    }

    rtc_setTimeZoneOffset(timeZoneOffset);

    if (tmTime.tm_year < 2015 || tmTime.tm_year > 2200)
	return UART_FAILED;

    memcpy(pTime, &tmTime, sizeof(tmTime));
    return UART_SUCCESS;
}

int8_t modem_set_max_messages() {	// Need to check
    int8_t uart_state;
    char *token;
    char memory[16];
    int storedmessages = 0;
    //check if messages are available
    uart_tx("+CPMS?");
    uart_state = uart_getTransactionState();
    if (uart_state == UART_SUCCESS) {
	PARSE_FINDSTR_RET(token, "CPMS: \"", UART_FAILED);
	PARSE_FIRSTSTRING(token, memory, 16, "\"", UART_FAILED);
	PARSE_NEXTVALUE(token, &storedmessages, ",\n", UART_FAILED);
	PARSE_NEXTVALUE(token, &config_getSIM()->iMaxMessages, ",\n",
			UART_FAILED);
    }

    if (storedmessages != 0)
	_NOP();
    return uart_state;
}

void modem_pull_time()	// Need to check -->
{
    int i;
    int res = UART_FAILED;
    uint8_t try_ntp=3;

    //    if (!state_isSimOperational())
    //	return;
    for (i = 0; i < NETWORK_PULLTIME_ATTEMPTS; i++)
    {
	res = uart_tx("AT+CCLK?");
	if (res == UART_SUCCESS)
	    res = modem_parse_time(&g_tmCurrTime);

	if (res == UART_SUCCESS) {
	    rtc_init(&g_tmCurrTime);
	    config_update_system_time();
	} else {
	    /*#ifdef DEBUG_OUTPUT
	    lcd_printf(LINEC, "WRONG DATE", config_getSelectedSIM());
	    lcd_printf(LINEH, get_YMD_String(&g_tmCurrTime));
#endif

	    lcd_printf(LINEC, get_string(LCD_MSG, LAST_DATE));
	    lcd_printf(LINEH, get_YMD_String(&g_tmCurrTime));*/

	    rtc_init(&g_pDevCfg->lastSystemTime);
	    rtc_getlocal(&g_tmCurrTime);

	    if(i==2 && try_ntp>0)
	    {
		try_ntp--;i--;
		http_enable();
		delay(500);
		uart_tx_timeout("AT#NTP=\"in.pool.ntp.org\",123,1,10\r\n",15000,2);
	    }
	}
    }
}

// Read command returns the selected PLMN selector <list> from the SIM/USIM
void modem_getPreferredOperatorList() {
    // List of networks for roaming
    uart_tx("+CPOL?");
}

void modem_init()	// Complete
{
  g_pSysState->system.switches.timestamp_on = 1;
  gsm_init();
}

/**********************************************************************************************
* Function Name : send_sms(char * sms, char * mobile_number)
* Parameters : void
* Return : void
* Description :  send SMS
**********************************************************************************************/

unsigned char send_sms(char *sms, char * mobile_number)	// Used in FISM
{
    if((flags.sim_ready) &&(strlen(mobile_number) > 4) &&(mobile_number[0] == '+'))
    {
	char sms_command[27];
	char* ret_val = NULL;

	memset(sms_command,'\0',sizeof(sms_command));
	strcpy(sms_command,"AT+CMGS=\"");
	strcat(sms_command,mobile_number);
	strcat(sms_command,"\"\r\n");

	if(send_cmd_wait(sms_command,">",30,1))
	{
	    strcat(sms,"\x1a");
	    send_cmd_wait(sms,"\r\n",45,1);
	    if(strstr(GSM_buffer,"+CMGS:"))
	    {
		return 1;
	    }
	    return 0;
	}
	return 0;
    }
}
/**********************************************************************************************
* Function Name : void check_radio_signal()
* Parameters : void
* Return : void
* Description :  Check and parse Signal Strength
**********************************************************************************************/
void check_radio_signal()	// Used in FISM
{
    unsigned char i=0,j=0;
    char *ret_val;
    unsigned char signal_st[8];

    memset(signal_st,'\0',sizeof(signal_st));
    flags.sim_ready = 0;
    ret_val = send_cmd_wait("AT+CPIN?\r\n", "READY\r\n",5,1);
    if(ret_val)
    {
	flags.sim_ready = 1;
	ret_val = send_cmd_wait("AT+CSQ\r\n", "+CSQ:",3,1);
	if(ret_val != '\0')
	{
	    for(i=0; i<10; i++)
	    {
		if((ret_val[i] == ',') || (ret_val[i] == '\r') || (ret_val[i] == '\n') || (ret_val[i] == '\0'))
		{
		    break;
		}

		if ( (ret_val[i] >= '0' && ret_val[i] <='9'))
		{
		    signal_st[j++] = ret_val[i];
		}
	    }
	}
	dvalue.signal_st =((unsigned char)((float)atoi(signal_st)/31*100));
    }
    else
    {
	dvalue.signal_st = 0;
    }
}

/**********************************************************************************************
* Function Name : char get_sim_num()
* Parameters : void
* Return : void
* Description :  Return SIM number
**********************************************************************************************/
char get_sim_num()	// Used in FISM
{
    char *token;
    char simnum[20];

    memset(simnum,'\0',sizeof(simnum));
    send_cmd_wait("AT+CNUM\r\n","OK",20,1);
    send_cmd_wait("AT+CNUM\r\n","OK\r\n",20,2);
    PARSE_FINDSTR_RET(token, "+CNUM:", UART_FAILED);
    PARSE_FIRSTSKIP(token, ",", UART_FAILED);
    //   PARSE_FIRSTSKIP(token, "\"", UART_FAILED);
    PARSE_NEXTSTRING(token, simnum, 20, "\"", UART_FAILED);
    memcpy(g_pDevCfg->SIM[0].cfgPhoneNum, simnum, 20);
    return 1;
}
/**********************************************************************************************
* Function Name : void send_deviceready()
* Parameters : void
* Return : void
* Description :  send device ready SMS
**********************************************************************************************/
void send_deviceready()	// Used in FISM
{
    SENSOR_STATUS *sensorState;
    uint8_t slot;
    int i = 0;

    char sensors[12];
    char gwkey[MAX_SI_LEN +2];
    //        char *msg = getSMSBufferHelper();
    char msg_ad[200];                                    //  adi change
    memset(msg_ad,'\0',sizeof(msg_ad));
    memset(sensors,'\0',sizeof(sensors));

    gwkey[0] = 0;
    if (strlen(g_pDevCfg->cfgGatewayKey) > 0) {
	sprintf(gwkey, "%s ", g_pDevCfg->cfgGatewayKey);
    }
#if 1
    debug_print("gwkey = ");
    debug_print(gwkey);
    debug_print("\n\r");
#endif
    strcat(sensors,"A:B:C");

    slot = 0;
    memcpy(g_pDevCfg->cfgVersion, "FISM",4);
    /* v=1,t=2,i=123456789012345,s=123456789012345,p=+911234512345,s2=123456789012345,
    p2=+911234512345,im=123456789012345
    d=10.2.3,m=1.0.1,a=CT5,as=A:B:C:D */
    sprintf(msg_ad, "%sv=%d,t=%d,i=%s,s=%s,p=%s,s2=%s,p2=%s,im=%s,d=%s,a=%s,as=%s,l=%d",
	    gwkey, SMS_MSG_VERS, SMS_MSG_TYPE_DATAREADY,
	    device_imei, ccid,g_pDevCfg->SIM[0].cfgPhoneNum,
	    g_pDevCfg->SIM[1].cfgSimId, g_pDevCfg->SIM[1].cfgPhoneNum,
	    device_imei,fw_version,
	    g_pDevCfg->cfgVersion, sensors, slot);
    // #ifndef _FDEBUG_
#if 1                            // rachit debug
    debug_print("msg = ");
    debug_print(msg_ad);
    debug_print("\n\r");
#endif
    // memcpy(g_pDevCfg->cfgGatewaySMS,"+919212594827",13);
    send_sms(msg_ad,g_pDevCfg->cfgGatewaySMS);

}
/**********************************************************************************************
* Function Name : void sms_call_init(void)
* Parameters : void
* Return : void
* Description :  Init. SMS AT commands
**********************************************************************************************/

void sms_call_init(void)	// Used in FISM
{
    send_cmd_wait("AT+CMGF=1\r\n", "OK\r\n", 10, 2); // select sms message format as text mode
    send_cmd_wait("AT+CSCS=\"GSM\"\r\n", "OK\r\n", 10, 2);
    send_cmd_wait("AT+CSMP=17,167,0,2\r\n", "OK\r\n", 10, 2);
}

/**********************************************************************************************
* Function Name : void gsm_init()
* Parameters : void
* Return : void
* Description :  Initialize GSM module
**********************************************************************************************/
void gsm_init()		// Used in FISM --> Complete
{
    DISABLE_GSM
	delay_sec(1);
    ENABLE_GSM
	if(GSM_STATUS_ACTIVE)
	{
	    GREEN_LED2_ON
            lcd_printl(LINEC, "CT5wifi");
	    char *ret_val;
	    char temp_buffer[100];

	    debug_print("\r\nDevice Firmware Version is ");
	    debug_print(fw_version);
	    debug_print("\r\n");

	    modem_clearLastErrorsCache();

	    send_cmd_wait("AT+CSCLK=0\r\n","OK\r\n",5,10);
	    send_cmd_wait("AT\r\n","OK\r\n",20,20);
	    send_cmd_wait("ATE0\r\n","OK\r\n",10,3);

	    parse_imei();
	    send_cmd_wait("AT+GSMBUSY = 0\r\n","OK\r\n",10,3);
	    send_cmd_wait("AT+CMEE=2\r\n","OK\r\n",1,3);
	    send_cmd_wait("AT+CFUN=1\r\n","OK\r\n",1,3);

	    /* it will prevent the device from factory reset if device rebooted using button */
	    g_pSysCfg->numberConfigurationRuns = 0;
	    read_indexes(0x1900,(char *)&flash_memory,sizeof(flash_memory));
	    file_system();
	    if(flags.factory_reset == 1)
	    {
		write_apn_file();
	    }
	    send_cmd_wait("AT&W\r\n", "OK\r\n",20,2);


	    if(send_cmd_wait("AT+CPIN?\r\n", "READY\r\n",5,1))
	    {   
		lcd_printl(LINEC,"SIM INSERTED");       //LCD PRINT
                send_cmd_wait("AT+CREG?\r\n","+CREG",30,3);
		send_cmd_wait("AT+CSCS=\"GSM\"\r\n","OK\r\n",30,1);
		send_cmd_wait("AT+CENG=3\r\n", "OK",10,2);  // get base station info
		sms_call_init();
		send_cmd_wait("AT+CLTS=1\r\n","OK\r\n",30,2);
		send_cmd_wait("AT+COPS=2\r\n","OK",30,1);
		delay_sec(1);
		ret_val = send_cmd_wait("AT+COPS=0\r\n","*PSUTTZ:",45,3);
		send_cmd_wait("AT\r\n","OK\r\n",10,20);
		send_cmd_wait("AT+CGATT?\r\n","+CGATT",30,2);
		send_cmd_wait("AT+CGATT=1\r\n","OK",10,10);
		parse_base_station_info();
		get_sim_num();
		check_ccid();
		check_gsm_fw();
		debug_print("\n\r RS 1 \n\r");
		check_radio_signal();
		get_local_time();
		if(epoch_seconds < ACTUAL_EPOCH)
		{
		    alarm_packet("TIME ERR");
		    alarm_sms(TIME_FAILURE, 0);
		}
		send_cmd_wait("AT&W\r\n", "OK\r\n",20,2);
		if(send_cmd_wait("AT+CGATT?\r\n","+CGATT: 1",10,1))
		{
		    ret_val = send_cmd_wait("AT+CCLK?\r\n","OK\r\n",10,2);
		    send_cmd_wait("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n","OK\r\n",60,2);
		    memset(temp_buffer,'\0',sizeof(temp_buffer));

		    sprintf(temp_buffer,"AT+SAPBR=3,1,\"APN\",\"%s\"\r\n",device_apn);
		    send_cmd_wait(temp_buffer,"OK\r\n",30,1);
		    send_cmd_wait("AT+SAPBR=1,1\r\n", "OK\r\n",60,2);
		    send_cmd_wait("AT&W\r\n", "OK\r\n",20,2);
		    if(ret_val == '\0')
		    {
			char ntp[150];
			memset(ntp,'\0',sizeof(ntp));
			sprintf(ntp,"AT+CNTP=\"in.pool.ntp.org\",%d\r\n",IST_TO_GMT);
			send_cmd_wait("AT+CNTPCID=1\r\n", "OK\r\n",20,2);
			send_cmd_wait(ntp,"OK\r\n",20,2); //AT#NTP=\"in.pool.ntp.org\",123,1,10\r\n",15000,2);
			send_cmd_wait("AT+CNTP\r\n", "OK\r\n",20,2);
			send_cmd_wait("AT+CCLK?\r\n","OK\r\n",10,2);
		    }

		    check_internet();
		    http_init();
                    debug_print("sending device ready");
                    //device_ready_http();  // adi
//                     SPI_CS_H;
//                    __delay_cycles(1000);
//                    SPI_FLASH_CS_SD_L;
// 
//                    UINT tbw = 0;
//		    fat_create_dev_ready(&tbw);
                    fatdone = true;
		}
	    }
	    else
	    {
               debug_print("sim not inserted");
               lcd_printl(LINEC,"SIM NOT INSERTED");       //LCD PRINT
               //alarm_packet("TIME ERR");
               //debug_print("tme alarm");
	       //alarm_sms(TIME_FAILURE, 0);
               //debug_print("alarm fail sms");
	    }
	    if(flags.factory_reset == 1)
	    {
		//		send_deviceready();
		flags.factory_reset = 0;
	    }
	}
      
      GREEN_LED2_OFF
}
/**********************************************************************************************
* Function Name : unsigned char send_data(char *packet)
* Parameters : void
* Return : void
* Description :  Send data
**********************************************************************************************/
unsigned char send_data(char *packet)	// Used in FISM --> Need to modify
{
    if((flags.sim_ready) &&(flags.internet_flag))
    {
	if(send_data_to_server(packet)==0)
	{
	    return SEND_PACKET_ERROR;
	}
	return SEND_PACKET_OK;
    }
    return SEND_PACKET_ERROR;
}
unsigned char send_data_wifi_gprs_sms(char *packet, char *URL)	// Used in FISM --> Need to modify
{
    unsigned char data_upload_success = 0;
    unsigned char skip_queue = 0;

    g_pDevCfg->cfgUploadMode = 0;
    if(g_pDevCfg->cfgUploadMode == 0)	// For GPRS --> Test
    {
	data_upload_success = 0;
	check_internet();
	http_init();
	if((flags.sim_ready) &&(flags.internet_flag))
	{
            //debug_print(" url empty");
	    http_open_connection_upload(0, URL);
	    data_upload_success = send_data_to_server_wifi_gprs_gsm(packet);
	}
	if(data_upload_success == 1)
	{
	    skip_queue = 1;
	    lcd_printl(2, "GPRS SUCCESS");
	    __delay_cycles(10000);
	}
	else
	{
	    g_pDevCfg->cfgUploadMode = 1;
	    lcd_printl(LINE2, "GPRS FAIL");
	    __delay_cycles(10000);
	}
    }
    if(g_pDevCfg->cfgUploadMode == 1 && !skip_queue)	// For wifi
    {
      
      
	data_upload_success = 1;
        /*
        P5OUT &= ~BIT6;
        
        __delay_cycles(100);
        P4OUT &= ~BIT4;
        __delay_cycles(1000);
        P4OUT |= BIT4;
        __delay_cycles(100);
        P5OUT |= BIT6;
        */
        
	//	send_data_to_wifi();
            //debug_print("wifi success");
	    //lcd_printl(2, "Wifi SUCCESS");
	    g_pDevCfg->cfgUploadMode = 2;
	    //lcd_printl(LINE2, "Wifi upload fail");
	    //delay_sec(15);
            //__delay_cycles(15000);
    }
    if(g_pDevCfg->cfgUploadMode == 2 && !skip_queue)	// For GSM
    {

	//sms_send_dataready();	// For device ready sms

	data_upload_success = 0;
	if(data_upload_success == 1)
	{
	    skip_queue = 1;
	    lcd_printl(2, "GSM upload-Yes");
	    delay_sec(3);
	}
	else
	{
	    g_pDevCfg->cfgUploadMode = 0;
	    lcd_printl(LINE2, "GSM upload fail");
	    delay_sec(3);
	}
    }
    return data_upload_success;
}
/**********************************************************************************************
* Function Name : void http_init()
* Parameters : void
* Return : void
* Description :  Init. HTTP on GSM module
**********************************************************************************************/
void http_init()		// Used in FISM	--> Complete
{
    send_cmd_wait("AT+HTTPTERM\r\n", "OK\r\n",10,1);
    send_cmd_wait("AT+HTTPINIT\r\n", "OK\r\n",10,1);
    send_cmd_wait("AT+HTTPPARA=\"CID\",1\r\n","OK\r\n",10,1);
    send_cmd_wait("AT+HTTPPARA=\"TIMEOUT\",1000\r\n","OK\r\n",10,1);
    send_cmd_wait("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n", "OK\r\n",10,1);
}
/**********************************************************************************************
* Function Name : void check_internet ()
* Parameters : void
* Return : void
* Description :  Check Internet Status
**********************************************************************************************/
unsigned char check_internet ()		// Used in FISM	--> Complete
{
    static unsigned char retry_count = 0;

    flags.internet_flag = 0;
    flags.sim_ready = 0;
    if(send_cmd_wait("AT+CPIN?\r\n", "READY\r\n",5,1))
    {
	flags.sim_ready = 1;
	send_cmd_wait("AT+CIPGSMLOC=1,1\r\n", "+CIPGSMLOC",45,1);
	if(strstr(GSM_buffer,"+CIPGSMLOC: 0")!=0)
	{
	    flags.internet_flag = 1;
            lcd_printl(LINE2,"INTERNET ACTIVE");
	}
	else
	{
          lcd_printl(LINE2,"INTERNET INACTIVE");
	    if(++retry_count>3)
	    {
		retry_count = 0;
		return 0 ;
	    }
	}
    }
    return 1 ;
}

/**********************************************************************************************
* Function Name : void upload_packet_sms(void)
* Parameters : void
* Return : void
* Description : Upload the packet via GPRS/SMS
**********************************************************************************************/
void upload_packet_sms()	// Used in FISM --> Need to modify --> Wifi --> GPRS --> SMS
{   debug_print("\n\r in upload packet sms");
    temp_sampled_data();
    if((flags.check_ota==0)&&(flags.sim_ready))
    {
	if((flags.internet_flag == 1) &&((g_ConfigDevice.cfgUploadMode == 2) || (g_ConfigDevice.cfgUploadMode == 1)))
	{
	    http_init();
            debug_print(" url empty 3");
	}
	while(temp_sampled_data() == 1);	// Data to be uploaded need to write here
	//	while(fridge_sampled_data() == 1);
    }
}
/**********************************************************************************************
* Function Name : void check_ccid(void)
* Parameters : void
* Return : void
* Description : Check ICCID (Integrated Circuit Card Identifier)
**********************************************************************************************/
void check_ccid()	// Used in FISM	--> complete
{
    //    char ccidval[22];
    char *ret;
    ret = send_cmd_wait("AT+CCID\r\n","OK\r\n",60,1);
    if(ret)
    {
	memset(g_pDevCfg->SIM[0].cfgSimId,'\0',sizeof(ccid));
	snprintf(g_pDevCfg->SIM[0].cfgSimId,21,ret-24);        // changed the value of ccid
    }
}
/**********************************************************************************************
* Function Name : void parse_imei(void)
* Parameters : void
* Return : void
* Description : Check ICCID (Integrated Circuit Card Identifier)
**********************************************************************************************/
void parse_imei(void)	// Used in FISM 	--> Complete
{
    char *ret_val = 0;
    ret_val = send_cmd_wait("AT+GSN\r\n","OK\r\n",30,1);
    if(ret_val)
    {
	unsigned char count=0,l_v=0;
	memset(device_imei,'\0',sizeof(device_imei));
	for(l_v=0; l_v<20; l_v++)
	{
	    if(GSM_buffer[l_v] >= '0' && GSM_buffer[l_v] <= '9')
	    {
		if(count < 16)   // till -1 to keep NULL at the end
		{
		    device_imei[count++] = GSM_buffer[l_v];
		}
		else
		{
		    break;
		}
	    }
	}
	memcpy(g_pDevCfg->cfgIMEI,device_imei, 20);
    }
}
/**********************************************************************************************
* Function Name : void check_ccid(void)
* Parameters : void
* Return : void
* Description : Check GSM FW
**********************************************************************************************/
void check_gsm_fw()	// Used in FISM
{
    char *ret;
    ret= send_cmd_wait("AT+CGMR\r\n","OK\r\n",60,1);
    if(ret)
    {
	memset(g_pDevCfg->ModemFirmwareVersion,'\0',sizeof(g_pDevCfg->ModemFirmwareVersion));
	snprintf(g_pDevCfg->ModemFirmwareVersion,17,ret-20);
    }
}
/**********************************************************************************************
* Function Name : void gsm_exit_sleep(void)
* Parameters : void
* Return : void
* Description :  Disable GSM Power Saving
**********************************************************************************************/
void gsm_exit_sleep(void)	// Used in FISM
{
    if(GSM_DTR)
    {
	debug_print("\n\r gsm exit sleep enter\n\r");  //ad
	uint8_t count = 0;
	char end_at_cmd = 0;

	P3OUT &= ~BIT2;// DTR ON (low)
	__delay_cycles(1000);
	while(count <= 10)
	{
	    send_cmd_wait("AT+CSCLK=0\r\n","\r\n",5,1);
	    __delay_cycles(300);
	    if(strstr(GSM_buffer,"OK")!= NULL)
	    {
		flags.power_saving = 1;
		debug_print("\r\npower save disabled\r\n");
		break;
	    }
	    else
	    {
		count++;
		if(count >=9)
		{
		    debug_print("\r\nreboot the device\r\n");//PMM_trigBOR();
		}
	    }
	}
	debug_print("gsm exit sleep");
    }
}
/**********************************************************************************************
* Function Name : void gsm_enter_sleep(void)
* Parameters : void
* Return : void
* Description :  Disable GSM Power Saving
**********************************************************************************************/
void enter_sleep(void)	// Used in FISM
{
    if(!(GSM_DTR))
    {
	flags.power_saving = 0;
	P3OUT |= BIT2;// DTR ON (High)
	__delay_cycles(100);

	unsigned char ret = 1;
	char *ret_ptr;
	unsigned char delay_count=0, retry_count=0;

	flags.at_cmd = 1;
	while((ret == 1) && (retry_count++ < 3))
	{
	    flags.at_cmd = 0;
	    delay_count = 0;
	    rec_buffer_index = 0;
#ifdef _FDEBUG_
	    debug_print("AT+CSCLK=1\r\n");      // debug print
#endif
	    memset(GSM_buffer,'\0',BUFFER_MAX_SIZE);
	    uartA2_xmit("AT+CSCLK=1\r\n");       // xmit data to GPS/GSM module
	    flags.at_cmd = 1;

	    while((ret == 1) && (delay_count++ < 10))
	    {
		ret_ptr = strstr(GSM_buffer,"OK\r\n");
		if(ret_ptr != '\0')
		{
		    ret = 0;
		    debug_print("\r\npower save enabled\r\n");
		    break;
		}
		delay_sec(1);
	    }
#ifdef _FDEBUG_
	    debug_print(GSM_buffer);     // debug print
	    debug_print("\r\n");
#endif
	}
    }
    if(flags.spi_flash_active)
    {
	extFlashPowerDown();
    }
    if(!flags.power_saving)
    {
	flags.power_saving = 1;
	__bis_SR_register(LPM0_bits + GIE);      // Disable CPU, keep functional master clock and slaves.
    }
}
/**********************************************************************************************
 * Function Name : void temp_data()
 * Parameters : void
 * Return : void
 * Description :Send temperature data
 **********************************************************************************************/
bool temp_sampled_data()
{

      uint16_t get_idx_cpy = 0;
      uint16_t total_pckt = 0;
      uint8_t packet_count = 0;
      char upload_buffer[2000];
      char packet_start[75];

      if(flash_memory.put_temp_idx !=flash_memory.get_temp_idx)
      {
        memset(upload_buffer,'\0',sizeof(upload_buffer));
        memset(packet_start,'\0',sizeof(packet_start));

        sprintf(packet_start,"{\"vId\":\"Nexleaf\",\"asm\":0,\"data\":[{\"dId\":\"%s\",\"tmps\":[",device_imei);
        strcat(upload_buffer,packet_start);

        get_idx_cpy = flash_memory.get_temp_idx;

        debug_print("Uploading tx mode");
        debug_print(itoa_pad(g_pDevCfg->cfgUploadMode));
        debug_print("\r\n");

        while(flash_memory.put_temp_idx != flash_memory.get_temp_idx)
        {
          g_uch_status = read_packets(flash_memory.put_temp_idx,flash_memory.get_temp_idx,TEMP_TOTAL_PACKETS);

          if(FLASH_MEM_READ_OK == g_uch_status)
          {
            strcat(upload_buffer,local_storage_buff);
            upload_buffer[strlen(upload_buffer)] = ',';
            memset(local_storage_buff,'\0',sizeof(local_storage_buff));
            flash_memory_increment_get_page_idx(flash_memory.get_temp_idx,TEMP_TOTAL_PACKETS,1);

            if(++packet_count >=((unsigned char)((float)PERIOD_UPLOAD/PERIOD_SAMPLING)))
            {
              packet_count = 0;
              break;
            }
          }
        }
        upload_buffer[strlen(upload_buffer)-1] = '\0';
        strcat(upload_buffer,"]}]}");

        if(g_ConfigDevice.cfgUploadMode == 2 || g_ConfigDevice.cfgUploadMode == 1)
        {
            debug_print("\r\nuploading Via GPRS \r\n");
            g_uch_status = send_data(upload_buffer);

            if(SEND_PACKET_ERROR == g_uch_status)
            {
              flash_memory.get_temp_idx = get_idx_cpy;
              update_put_get_idx();
            }
            else                // Data sent successfully through HTTP
            {
              update_put_get_idx();
              return true;
            }

        }
        if(((g_ConfigDevice.cfgUploadMode == 2) && (SEND_PACKET_ERROR == g_uch_status)) || (g_ConfigDevice.cfgUploadMode == 0))
        {
            debug_print("\r\nuploading Via SMS \r\n");

            flash_memory.get_temp_idx = get_idx_cpy;
            char sms_buffer[200];

            while(flash_memory.get_temp_idx != flash_memory.put_temp_idx)
            {
                get_idx_cpy = flash_memory.get_temp_idx;

                memset(sms_buffer,'\0',sizeof(sms_buffer));
                memset(local_storage_buff,'\0',sizeof(local_storage_buff));

                g_uch_status = read_packets(flash_memory.put_temp_idx,flash_memory.get_temp_idx,TEMP_TOTAL_PACKETS);

                if(FLASH_MEM_READ_OK == g_uch_status)
                {
                  if(parse_input_value_for_sms())
                  {
                      strcat(sms_buffer,local_storage_buff);
                  }
                  else
                  {
                      return false;
                  }
                }
                else
                {
                  return false;
                }
                if(sms_buffer != '\0')
                {
                    if(send_sms(sms_buffer,g_pDevCfg->cfgGatewaySMS))
                    {
                      memset(sms_buffer,'\0',sizeof(sms_buffer));
                      flash_memory_increment_get_page_idx(flash_memory.get_temp_idx,TEMP_TOTAL_PACKETS,1);
                      update_put_get_idx();
                    }
                    else
                    {
                      memset(local_storage_buff,'\0',sizeof(local_storage_buff));
                      memset(sms_buffer,'\0',sizeof(sms_buffer));
                      flash_memory.get_temp_idx =  get_idx_cpy;
                      update_put_get_idx();
                      return  false ;
                    }
                }
                else
                {
                   memset(local_storage_buff,'\0',sizeof(local_storage_buff));
                   memset(sms_buffer,'\0',sizeof(sms_buffer));
                   flash_memory.get_temp_idx =  get_idx_cpy;
                   update_put_get_idx();
                   return  false ;
                }
            }
            return false;
        }
        else
        {
          flash_memory.get_temp_idx =  get_idx_cpy;
          update_put_get_idx();
          return false;
        }
        return false;
      }
      else
      {
        return false;
      }
}

/**********************************************************************************************
* Function Name : void temp_data()
* Parameters : void
* Return : void
* Description :Send temperature data
**********************************************************************************************/
/*bool temp_sampled_data()	// For FISM project
{
    uint16_t get_idx_cpy = 0;
    uint16_t total_pckt = 0;
    uint8_t packet_count = 0;
    char upload_buffer[2000];
    //char upload_buffer[1000];       
    char packet_start[75];

    if(g_ConfigDevice.cfgUploadMode == 2 || g_ConfigDevice.cfgUploadMode == 1)	// Send data via GSM
    {
	debug_print("\r\nuploading Via GPRS \r\n");

	FRESULT fr;
	FIL *fobj;
	char* fn;
	uint16_t lineSize = 0;
	char *line = getStringBufferHelper(&lineSize);

	memset(upload_buffer,'\0',sizeof(upload_buffer));
	sprintf(upload_buffer,"{\"vId\":\"Nexleaf\",\"data\":[{\"dId\":\"%s\",\"tmps\":[",g_pDevCfg->cfgIMEI);
	fn = get_current_fileName(&g_tmCurrTime, FOLDER_TEXT, EXTENSION_TEXT);
	fr = fat_open(&fobj, fn, FA_READ | FA_OPEN_EXISTING); //currently only supports 8.3 OEM character filenames
	f_lseek(fobj, 0);
	lcd_print_progress();
	while(f_gets(line,800,fobj) != NULL)
	{
	    strcat(upload_buffer,line);
	}
	replace_character(line, '\n', ',');
	strcat(upload_buffer,line);
	strcat(upload_buffer,"]");
	if(g_pSysState->system.switches.powerAvailChanged == true)
	{
	    char temp_demo[30];
	    memset(temp_demo,'\0',sizeof(temp_demo));
	    char *utcString = itoa_pad(g_pSysState->last_sample_time);
	    strcat(upload_buffer,",");
	    strcat(upload_buffer,"\"pwa\":{");
	    sprintf(temp_demo,"\"stat\":\"%s\"",itoa_nopadding(g_pSysState->system.switches.power_connected));
	    strcat(upload_buffer,temp_demo);
	    strcat(upload_buffer,",");
	    memset(temp_demo,'\0',sizeof(temp_demo));
	    sprintf(temp_demo,"\"time\":\"%s\"",utcString);
	    strcat(upload_buffer,temp_demo);
	    strcat(upload_buffer,"}");
	}
	strcat(upload_buffer,"}]}");
	http_open_connection_upload(0,DATA_UPLOAD_URL_PATH);
	debug_print(upload_buffer);
	send_data(upload_buffer);
	releaseStringBufferHelper();

	return fat_close();

	g_uch_status = send_data(upload_buffer);

	if(SEND_PACKET_ERROR == g_uch_status)
	{
	    // Data sent via GPRS fail
	}
	else                // Data sent successfully through HTTP
	{
	    // Data sent via GPRS success
	}
    }
    if(((g_ConfigDevice.cfgUploadMode == 2) && (SEND_PACKET_ERROR == g_uch_status)) || (g_ConfigDevice.cfgUploadMode == 0))	// Send data via SMS
    {
	debug_print("\r\nuploading Via SMS \r\n");

	flash_memory.get_temp_idx = get_idx_cpy;
	char sms_buffer[200];

	while(flash_memory.get_temp_idx != flash_memory.put_temp_idx)
	{
	    get_idx_cpy = flash_memory.get_temp_idx;

	    memset(sms_buffer,'\0',sizeof(sms_buffer));
	    memset(local_storage_buff,'\0',sizeof(local_storage_buff));

	    g_uch_status = read_packets(flash_memory.put_temp_idx,flash_memory.get_temp_idx,TEMP_TOTAL_PACKETS);

	    if(FLASH_MEM_READ_OK == g_uch_status)
	    {
		if(parse_input_value_for_sms())
		{
		    strcat(sms_buffer,local_storage_buff);
		}
		else
		{
		    return false;
		}
	    }
	    else
	    {
		return false;
	    }
	    if(sms_buffer != '\0')
	    {
		if(send_sms(sms_buffer,g_pDevCfg->cfgGatewaySMS))
		{
		    memset(sms_buffer,'\0',sizeof(sms_buffer));
		    flash_memory_increment_get_page_idx(flash_memory.get_temp_idx,TEMP_TOTAL_PACKETS,1);
		    update_put_get_idx();
		}
		else
		{
		    memset(local_storage_buff,'\0',sizeof(local_storage_buff));
		    memset(sms_buffer,'\0',sizeof(sms_buffer));
		    flash_memory.get_temp_idx =  get_idx_cpy;
		    update_put_get_idx();
		    return  false ;
		}
	    }
	    else
	    {
		memset(local_storage_buff,'\0',sizeof(local_storage_buff));
		memset(sms_buffer,'\0',sizeof(sms_buffer));
		flash_memory.get_temp_idx =  get_idx_cpy;
		update_put_get_idx();
		return  false ;
	    }
	}
	return false;
    }
    else
    {
	flash_memory.get_temp_idx =  get_idx_cpy;
	update_put_get_idx();
	return false;
    }
    return false;
}
*/
/**********************************************************************************************
* Function Name : void parse_base_station_info(void)
* Parameters : None
* Return : None
* Description : get base station info from GSM module and parse it
**********************************************************************************************/
void parse_base_station_info(void)	// Used in FISM
{
    char mcc[8];
    char mnc[8];

    /* AT_CENG? resonse :
    *  +CENG: 3,0
    *
    * 	+CENG: 0,"404,04,1b3,d914,03,17"
    +CENG: 1,"404,04,138,d252,38,13"
    +CENG: 2,"404,04,7d,8ab9,19,12"
    +CENG: 3,"404,04,1b3,d915,48,12"
    +CENG: 4,"404,04,1b3,e0da,24,10"
    +CENG: 5,"404,04,7d,dc49,21,08"
    +CENG: 6,"000,00,0,0,00,00"

    OK

    In the above, only first string will be taken
    0 : serving cell
    404 : Mobile country code (MCC) in decimal
    04 : Mobile Network Code (MNC) in decimal
    1b3 : Location area code (LAC) in hex
    d914 : Cell ID (CI) in hex
    */
    char *ret_val;
    unsigned char temp=0, count=0;

    ret_val = send_cmd_wait("AT+CENG?\r\n", "+CENG: 0,\"",10,2);  // get base station info

    if(ret_val != '\0')
    {
	memset(mcc,'\0',sizeof(mcc));
	memset(mnc,'\0',sizeof(mnc));
	for(count=0;count<=6;count++)
	{
	    if(ret_val[count+10]==',')
	    {
		break;
	    }
	    mcc[count] =  ret_val[count+10];
	}
	for(temp=0;temp<=6;temp++)
	{
	    if(ret_val[count+11+temp]==',')
	    {
		break;
	    }
	    mnc[temp] =  ret_val[count+11+temp];

	}
	char debug_buff[50];
	memset(debug_buff,'\0',sizeof(debug_buff));
	sprintf(debug_buff,"\r\n\r\n MCC is %s and MNC is %s\r\n\r\n",mcc,mnc);
	debug_print(debug_buff);
	read_apn(mcc,mnc);
    }
}

uint8_t modem_sim_select()	//  NA (Not used in FISM)
{
    const char *buf;
    uint8_t sim_slot = config_getSelectedSIM();
    uart_txf("#GPIO=3,%d,1",sim_slot); // Sim select

    uart_tx("#GPIO=1,0,0");
    uart_tx("#GPIO=1,2");
    buf = uart_getRXHead();
    while(*buf++ != ',');
#ifdef OLD_HW
    if(*buf == '1'){
#else
	if(*buf == '0'){
#endif
#ifndef _FISM_
	    if(g_pDevCfg->print_en != 1) {
		lcd_printl(LINE2,get_string(LCD_MSG, SIM_NOT_INSERTED));
		delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
	    }
#endif
	    state_SIM_not_operational();
	    return 1;
	}
	return 0;
    }

#ifdef POWER_SAVING_ENABLED	// NI --> Demo
    int8_t modem_enter_powersave_mode()
    {
	//	if(!MODEM_POWERSAVE_IS_SET) {
	//	    delay(15000);		//to allow the last transmit to do successfully
	//	    //set DTR OFF (high)
	//	    P3OUT |= BIT3;//DTR
	//	    delay(100);//opt
	//	}
	gsm_sleep();
	//get CTS to confirm the modem entered power saving mode
	return MODEM_POWERSAVE_ACTIVE ? 1 : 0;
    }

    int8_t modem_exit_powersave_mode()
    {
	/*	if(MODEM_POWERSAVE_IS_SET)
	{
	//set DTR ON (low)
	P3OUT &= ~BIT3;//DTR
	delay(100);//opt
    }*/
	gsm_exit_sleep();
	//return true if the modem has successfully exited power saving mode
	return MODEM_POWERSAVE_ACTIVE ? 0 : 1;
    }

#endif
