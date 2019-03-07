/*
* http.c
*
*  Created on: May 22, 2015
*      Author: sergioam
*/

//uart_tx("AT+CGDCONT=1,\"IP\",\"www\",\"0.0.0.0\",0,0\r\n");
//#include "TI_MSPBoot_Mgr_Vectors.h"
#include "thermalcanyon.h"
#include "stringutils.h"
#include "lcd.h"
#include "config.h"
#ifdef USE_MININI
#include "minIni.h"
#endif

#define HTTP_RESPONSE_RETRY	10

#define TRANS_FAILED		   -1
#define TRANS_SUCCESS			0

//const char HTTP_INCOMING_DATA[] = { 0x0D, 0x0A, '<', '<', '<', 0 };
const char HTTP_ERROR[] = { 0x0D, 0x0A, 'E', 'R', 'R', 'O', 'R', 0x0D, 0x0A,
0x0D, 0x0A, 0 };
const char HTTP_OK[] = { 0x0D, 0x0A, 'O', 'K', 0x0D, 0x0A, 0 };
const char HTTP_RING[] = "#HTTPRING: ";

// <prof_id>,<http_status_code>,<content_type>,<data_size>
int http_check_error(int *retry) {

    char *token = NULL;
    char error[16];
    int prof_id = 0;
    int data_size = 0;
    int http_status_code = 0;

    SIM_CARD_CONFIG *sim = config_getSIM();

    sim->http_last_status_code = -1;
    // Parse HTTPRING
    PARSE_FINDSTR_RET(token, HTTP_RING, UART_FAILED);

    PARSE_FIRSTVALUE(token, &prof_id, ",\n", UART_FAILED);
    PARSE_NEXTVALUE(token, &http_status_code, ",\n", UART_FAILED);
    PARSE_SKIP(token, ",\n", UART_FAILED); 	// Skip content_type string.
    PARSE_NEXTVALUE(token, &data_size, ",\n", UART_FAILED);

    if (http_status_code != 200 && http_status_code != 201 &&
	http_status_code != 400 && http_status_code != 206 && http_status_code != HTTP_STATUS_PULL_CONFIG) {
	    *retry = 2;
	    //		g_sEvents.defer.command.display_http_error=1;
	    ini_gets("STATUS", itoa_nopadding(sim->http_last_status_code),
		     "unassigned", error, sizeof(error), "http.ini");
	    lcd_printl(LINEC, error);
	}

    // Check if we got the right prof_id
    if (prof_id == 0) {
	_NOP();
    }

    /*
#ifdef _DEBUG
    log_appendf("HTTP %i[%d] %d", prof_id, http_status_code, data_size);
#endif
    */

    sim->http_last_status_code = http_status_code;

    // Check for recoverable errors
    // Server didnt return any data
    if ((http_status_code == 200 || http_status_code == 201 || http_status_code == HTTP_STATUS_PULL_CONFIG
	 || http_status_code == 400 || http_status_code == 206) && data_size == 0) {
	     *retry = 1;
#ifdef _DEBUG_OUTPUT
	     lcd_printl(LINEC, "HTTP Server");
	     lcd_printl(LINEH, "Empty response");
#endif
	 }

    // TODO Find non recoverable errors
    return UART_SUCCESS;
}

int http_open_connection_upload(uint32_t data_length, char *URL)
{	// For SIM800 --> Converted for Demo use
//    char *cmd = getSMSBufferHelper();
    char http_url[150];
    sprintf(http_url,"AT+HTTPPARA=\"URL\",\"%s\"\r\n",URL);	// For FISM board
    send_cmd_wait(http_url,"OK\r\n",20,2);
    return 1;
}

int8_t http_send_file(FIL *file, char *URL)	// For Telit	--> Converterd in SIM for demo
{
    int uart_state;
    uint16_t lineSize = 0;
    char *line = getStringBufferHelper(&lineSize);
    int retry = 0;
    uint32_t length = 0;
    char upload_buffer[2000],upload_success=0;
    int res = TRANS_FAILED;

    memset(upload_buffer,'\0',sizeof(upload_buffer));
    length = file->fsize;
    f_lseek(file, 0);
    lcd_print_progress();
    while (f_gets(line, lineSize, file) != 0)
    {
	if (file->fptr != file->fsize)
	{
	    strcat(upload_buffer,line);
	    lcd_print_progress();
	}
	else
	{
	    upload_success = send_data_wifi_gprs_sms(upload_buffer,URL);
	    if(upload_success == 1)
	    {
		goto release;
	    }
	    res = TRANS_SUCCESS;
	}
    }

  release:
    //release_buf();
    releaseStringBufferHelper();
    return res;
}

//All comment are referenced to AT Commands Reference Guide 80000ST10025a Rev. 22 - 2015-08-05
int http_get(char *URL, int (*process_callback)(void), uint8_t type) 	// Telit
{
    int uart_state = UART_FAILED;
    int retry = 0;
    FRESULT fr;
    FIL *fobj;
    uint16_t crc = 0xFFFF;
    //   const char *buf = "\0";
    int finish = 1;
    int byw;
    static int trial;
    char *cmd = getSMSBufferHelper();
    char fn[15];
    const char *szLog = NULL;
    char *szLog_1, crc_buf[5], flag = 1;

    SIM_CARD_CONFIG *sim = config_getSIM();

    if (!state_isSimOperational()) {
	goto release;
    }

    if (http_enable() != UART_SUCCESS) {
	goto release;
    }

    // #HTTPQRY send HTTP GET, HEAD or DELETE request
    // Execution command performs a GET, HEAD or DELETE request to HTTP server.
    // Parameters:
    // <prof_id> - Numeric parameter indicating the profile identifier. Range: 0-2
    // <command>: Numeric parameter indicating the command requested to HTTP server:
    // 0 GET 1 HEAD 2 DELETE
    //<resource>: String parameter indicating the HTTP resource (uri)

    //If sending ends successfully, the response is OK; otherwise an error code
    //is reported.

    /*sachin  sprintf(cmd, "AT#HTTPQRY=1,0,\"%s\"\r\n", URL);
    uart_tx_timeout(cmd, TIMEOUT_HTTPQRY, 1);
    if (uart_getTransactionState() != UART_SUCCESS) {
    goto release;
}*/

    sprintf(cmd, "AT#HTTPQRY=1,0,\"%s\"\r\n", URL);

    uart_tx_timeout(cmd, TIMEOUT_HTTPQRY, 1);
    if(uart_getTransactionState() != UART_SUCCESS) {
	goto release;
    }
    //When the HTTP server answer is received, then the following URC is put
    //on the serial port:
    //#HTTPRING:
    //<prof_id>,<http_status_code>,<content_type>,<data_size>
    //Where:
    //<prof_id> is defined as above
    //<http_status_code> is the numeric status code, as received from the
    //server (see RFC 2616)
    //<content_type> is a string reporting the "Content-Type" header line, as
    //received from the server (see RFC 2616)
    //<data_size> is the byte amount of data received from the server. If the
    //server does not report the "Content-Length:" header line, the parameter value is 0.
    //Note: if there are no data from server or the server does not answer within
    //the time interval specified in <timeout> parameter of #HTTPCFG
    //command, then the URC #HTTPRING <http_status_code> parameter has value 0.

    // This command also disables the appending of AT and \r\n on the data
    /*sachin   uart_setHTTPDataMode();
    uart_state = uart_tx_waitForPrompt("", TIMEOUT_HTTPQRY);
    http_check_error(&retry);
    if (sim->http_last_status_code != 200 || uart_state == UART_FAILED) {
    goto release;
}*/

    uart_setHTTPDataMode();
    uart_state = uart_tx_waitForPrompt("", TIMEOUT_HTTPQRY);

    http_check_error(&retry);

    if (uart_state == UART_FAILED) {
	goto release;
    }
    //AT#HTTPRCV=<prof_id>[,<maxByte>]
    //Execution command permits the user to read data from HTTP server in
    //response to a previous HTTP module request. The module is notified of
    //these data by the #HTTPRING URC.
    //The device shall prompt a three character sequence
    //<less_than><less_than><less_than>
    //(IRA 60, 60, 60)
    //followed by the data.
    //If reading ends successfully, the response is OK; otherwise an error code
    //is reported.
    //Parameters:
    //<prof_id> - Numeric parameter indicating the profile identifier.
    //Range: 0-2
    //< maxByte > - Max number of bytes to read at a time
    //Range: 0,64-1500 (default is 0 which means infinite size)
    //Note: if <maxByte> is unspecified, server data will be transferred all in once.
    //Note: If the data are not present or the #HTTPRING <http_status_code>
    //parameter has value 0, an error code is reported.

    // Wait for prompt
    /*sachin	uart_setHTTPRCVPromptMode();
    if(uart_tx_waitForPrompt("AT#HTTPRCV=1,500\r\n", TIMEOUT_HTTPSND_PROMPT) != UART_SUCCESS) {
    goto release;
}

    //wait for OK or Error here
    if(uart_tx_waitForPrompt("", TIMEOUT_HTTPRCV) != UART_SUCCESS) { //Gathering data here
    goto release;
}

    (*process_callback)();
    uart_state = UART_SUCCESS;
    */
    //remove the files before starting


    while(finish) {

	uart_setHTTPRCVPromptMode();
	if(uart_tx_waitForPrompt("AT#HTTPRCV=1,400\r\n", TIMEOUT_HTTPSND_PROMPT) != UART_SUCCESS) {
	    goto release;
	}
	//wait for OK or Error here
	if(uart_tx_waitForPrompt("", TIMEOUT_HTTPRCV) != UART_SUCCESS) { //Gathering data here
	    goto release;
	}
	if (type == 1) { //To read the configuration
	    uart_state = (*process_callback)();
	    return uart_state;
	}

	// buf = uart_getRXHead();
	//    szLog = getStringBufferHelper(NULL);
	szLog = uart_getRXHead();
	//   szLog = getStringBufferHelper(NULL);
	//  sprintf(szLog, "%s", buf);

	szLog_1 = strchr(szLog, 'O');
	*szLog_1 = '\0';

	if(flag == 1) {
	    sprintf(crc_buf, "%d.%d", FW_VER_MIN,FW_VER_MINMIN);
	    crc_buf[4] = '\0';
	    if(!strcmp(crc_buf, g_ConfigDevice.ver)) {
		g_pSysState->type = 1;
		return UART_FAILED;
	    }
	    if (type == 2) {
		strcpy(fn,"/UPDATE.TXT" );
		f_unlink(fn);
		strcpy(fn,"/fw.txt" );
		f_unlink(fn);
	    }
	    flag = 0;
	}

	lcd_printl(LINEC,"Downloading..");
	fr = fat_open(&fobj, fn, FA_READ | FA_WRITE | FA_OPEN_ALWAYS );//currently only supports 8.3 OEM character filenames
	if (fr == FR_OK) {
	    if (fobj->fsize) {
		f_lseek(fobj, (fobj->fsize-2));
	    }
	}

	f_write(fobj, szLog, strlen(szLog), (UINT *) &byw);
	f_close(fobj);


	crc = crc_get(szLog, (strlen(szLog) - 2), crc);
	if(strchr(szLog, 'q')) {
	    fat_open(&fobj, fn, FA_READ | FA_WRITE);
	    f_lseek(fobj, (fobj->fsize-2));
	    f_write(fobj, szLog, strlen(szLog), (UINT *) &byw);
	    f_close(fobj);
	    break;
	}
    }

    sprintf(crc_buf,"%x", crc);
    if (!strcmp(crc_buf, g_ConfigDevice.crc)) {
	//    memset(&g_pSysCfg->numberConfigurationRuns, 0, sizeof(g_pSysCfg->numberConfigurationRuns));
	memset(&g_pSysCfg->memoryInitialized, 0, sizeof(g_pSysCfg->memoryInitialized));
	f_rename(fn, "/UPDATE.TXT");
	delmsg(g_ConfigDevice.msg_index);
	f_rename(fn, "/UPDATE.TXT");
	lcd_printl(LINEC,"JMP TO BOOT");
	delallmsgs();
	//aditya delay(3000);
	TI_MSPBoot_JumpToBoot();
    } else {
	g_pSysState->fw_flag = 1;
	lcd_printl(LINEC,"CRC MISMATCH");
	//       uart_state = UART_FAILED;
	goto release1;
    }

  release:
    if (type == 2) {
	lcd_printl(LINE2,"FAILED");
	g_pSysState->fw_flag = 2;
	trial++;
	if (trial%3 == 0) {
	    g_pSysState->type = 3;
	}
	// uart_state = UART_FAILED;
    }

  release1:
    uart_state = UART_FAILED;
    //aditya delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
    //  releaseStringBufferHelper();
    //    //disconnect http if needed
    //    if (g_pSysState->system.switches.http_enabled) {
    //        releaseStringBufferHelper();
    //        http_deactivate();
    //    }
    //release_buf();
    return uart_state;
}

/*
int http_get_configuration() {
int uart_state = UART_FAILED;

char szTemp[100];
int uart_state = UART_FAILED;
int retry = 1;
int attempts = HTTP_COMMAND_ATTEMPTS;

if (!state_isSimOperational())
return UART_ERROR;

if(http_enable() != UART_SUCCESS)	//make sure we are enabled
return uart_state;

config_setLastCommand(COMMAND_GET_CONFIGURATION);

// #HTTPQRY send HTTP GET, HEAD or DELETE request
// Execution command performs a GET, HEAD or DELETE request to HTTP server.
// Parameters:
// <prof_id> - Numeric parameter indicating the profile identifier. Range: 0-2
// <command>: Numeric parameter indicating the command requested to HTTP server:
// 0 GET 1 HEAD 2 DELETE

sprintf(szTemp, "AT#HTTPQRY=1,0,\"%s%s/%d/\"\r\n", g_pDevCfg->cfgConfig_URL,
g_pDevCfg->cfgIMEI,
g_pDevCfg->cfgSIM_slot);
uart_tx_timeout(szTemp, 5000, 1);
if (uart_getTransactionState() != UART_SUCCESS) {
(void)http_deactivate();
return UART_FAILED;
	}

// CME ERROR: 558 cannot resolve DN ?

// Get the configration from the server
// #HTTPRCV � receive HTTP server data

uart_setCheckMsg(HTTP_OK, HTTP_ERROR);

while (retry == 1 && attempts > 0) {
if (uart_tx_timeout("#HTTPRCV=1", TIMEOUT_HTTPRCV, 1) == UART_SUCCESS) {
uart_state = uart_getTransactionState();
if (uart_state == UART_SUCCESS) {
retry = 0; 	// Found a configuration, lets parse it.6
config_process_configuration();
			}

if (uart_state == UART_ERROR)
http_check_error(&retry);  // Tries again
		}
attempts--;
	};

(void)http_deactivate();

return uart_state;
}*/


