/*
 * sms.c
 *
 *  Created on: Feb 25, 2015
 *      Author: rajeevnx
 */


#define SMS_C_

#ifndef _FISM_


#include "thermalcanyon.h"
#include "alarms.h"

// Send back data after an SMS request
void sms_send_data_request(char *number)
{
	uint32_t iOffset;
	char *data = getSMSBufferHelper();

	if (number == NULL || strlen(number) == 0)
		return;

	//get temperature values
//sachin	rtc_update_time();
	//strcat(data, get_simplified_date_string(NULL));
        rtc_getlocal(&g_tmCurrTime);
	strcat(data, get_date_string(&g_tmCurrTime, "-", " ", ":", 1));
	strcat(data, " ");
	for (iOffset = 0; iOffset < SYSTEM_NUM_SENSORS; iOffset++) {
		strcat(data, SensorName[iOffset]);
		strcat(data, ":");
		strcat(data, temperature_getString(iOffset));
		strcat(data, "C, ");
	}

	// added for show msg//
        sprintf(data, "%sBATT:%d%",data, batt_getlevel());
//	strcat(data, "BATT:");
//	strcat(data, itoa_nopadding(batt_getlevel()));
//	strcat(data, "%, ");
//sachin	strcat(data, state_get_batt_string());

	iOffset = strlen(data);
	sms_send_message_number(number, data);
}

// Gets the message
const char COMMAND_RESULT_CMGR[] = "+CMGR: ";

// +CMGR: <stat>,<length><CR><LF><pdu>
// <stat> - status of the message
// 0 - new message
// 1 - read message
// 2 - stored message not yet sent
// 3 - stored message already sent

uint8_t parse_config_sms = 0;

int8_t sms_process_memory_message(int8_t index) {
//sachin	TEMP_ALERT_PARAM *alert;
	char *token;
	char *msg = getSMSBufferHelper();
	char state[16];
	char ok[8];
	char phoneNumber[32];
	char *phone;
        int t = 0, len = 0;

	uart_txf("+CMGR=%d", index);
        g_pDevCfg->msg_index = index;

	int8_t uart_state = uart_getTransactionState();
	if (uart_state != UART_SUCCESS)
		return uart_state;

	PARSE_FINDSTR_RET(token, COMMAND_RESULT_CMGR, UART_FAILED);
	PARSE_FIRSTSTRING(token, state, sizeof(state), ",\n", UART_FAILED);
	PARSE_NEXTSTRING(token, phoneNumber, sizeof(phoneNumber), ",\n",
			UART_FAILED);

	len = strlen(phoneNumber);
	for (t = 0; t < len; t++)
		if (phoneNumber[t] == '\"')
			phoneNumber[t] = 0;

	PARSE_SKIP(token, "\n", UART_FAILED);

	PARSE_NEXTSTRING(token, msg, MAX_SMS_SIZE, "\n", UART_FAILED);

	// Jump first \n to get the OK
	PARSE_SKIP(token, "\n", UART_FAILED);
	ok[0] = 0;
	PARSE_NEXTSTRING(token, ok, sizeof(ok), "\n", UART_FAILED);
	if (ok[0] != 'O' || ok[1] != 'K')
		return UART_FAILED;

	phone = &phoneNumber[1];

	trim_sms(msg); //remove sms 0x0D end char

	token = strtok(msg," ,"); //get command from msg if there is any other params

	if (strcmp(token,"STATUS") == 0)
        {
#ifndef _FISM_
		sms_send_data_request(phone); //send status to phone
		delallmsg();
#endif
	}
	else if(!strcmp(token,g_pDevCfg->cfgMsgSenderId)) {
#ifndef _FISM_
                parse_config_sms = 1;
		config_parse_configuration_sms(msg);
#endif
	}
	/*else if (strcmp(token,"CONFIG") == 0) {
		sms_send_message_number(phone, get_string(SMS_MSG, HMN_RDBL_RESP_CONFIG));

		int setting = 0; //keep track of which config params
		while (token != NULL && setting < 5) //while still more configs in msg
		{
			int i; //loop var
			//token = strtok(NULL, ","); //get new config
			for (i = 0; i < 4; i++) { //loop 4 settings as a time
                                token = strtok(NULL,",");
                                if (token == NULL) {
                                   break;
                                }
                                alarm_param = atoi(token);

				alert = &g_pDevCfg->stTempAlertParams[setting];

				switch (i) {
				case 0:
					alert->threshCold = alarm_param;
					break;
				case 1:
					alert->maxSecondsCold = MINUTES_(alarm_param);
					break;
				case 2:
					alert->threshHot = alarm_param;
					break;
				case 3:
					alert->maxSecondsHot = MINUTES_(alarm_param);
					break;
				default:
					_NOP(); //nothing
					break;
				}
			}//end forloop

			setting++;
		}
                state_alarm_reset_all();
		delallmsg();
	}*//*
	else if (strcmp(token,"EVENTS") == 0) {
		sms_send_message_number(phone, get_string(SMS_MSG, HMN_RDBL_RESP_EVENTS));

		int setting = 0; //keep track of which config params
		while (token != NULL) //while still more configs in msg
		{
			token = strtok(NULL, " ,"); //get new config

				switch (setting) {
				case 0:
					event_force_event_by_id(EVT_SUBSAMPLE_TEMP, strtol(token,NULL,10));
					break;
				case 1:
					event_force_event_by_id(EVT_UPLOAD_SAMPLES, strtol(token,NULL,10));
					break;
				default:
					_NOP(); //nothing
					break;
				}

			setting++;
		}
		delallmsg();
	}*/
	else if (strcmp(token, "RESET") == 0) {
#ifndef _FISM_
		sms_send_message_number(phone, get_string(SMS_MSG, HMN_RDBL_RESP_RESET));
		delallmsg();

		//rebooting
		//lcd_print(get_string(LCD_MSG, REBOOTING_NOW));
		//lcd_progress_wait(1000);

		//reset the board by issuing a SW BOR
#ifndef _FISM_
                log_reboot("RESET");
#endif
		system_reboot(get_string(RBT_MSG, RBT_NET_COMMAND));
#endif
	}
	else {
//		sms_send_message_number(phone, get_string(SMS_MSG, HMN_RDBL_RESP_NAK));
		delallmsg();
	}
	return UART_SUCCESS;
}

// Gets how many messages and the max number of messages on the card / memory
// +CPMS: <memr>,<usedr>,<totalr>,<memw>,<usedw>,<totalw>,
// <mems>,<useds>,<totals>

const char COMMAND_RESULT_CPMS[] = "+CPMS: ";

int8_t sms_process_messages() {
	char *token;
	char SM_ME[5]; // Internal memory or sim card used
	uint32_t iIdx;
	SIM_CARD_CONFIG *sim = config_getSIM();
	uint8_t usedr = 0; // Reading memory
	uint8_t totalr = 0;

//	config_setLastCommand(COMMAND_SMS_PROCESS);

	memset(SM_ME, 0, sizeof(SM_ME));
#ifndef _FISM_
	lcd_printf(LINEC, get_string(LCD_MSG, FETCHING_SMS));
	lcd_printf(LINE2, "SIM %d", g_pDevCfg->cfgSIM_slot + 1);
#endif
	//check if messages are available
	uart_tx("+CPMS?");

	// Failed to get parameters
	int8_t uart_state = uart_getTransactionState();
	if (uart_state != UART_SUCCESS)
		return uart_state;

	PARSE_FINDSTR_RET(token, COMMAND_RESULT_CPMS, UART_FAILED);
	PARSE_FIRSTSTRING(token, SM_ME, sizeof(SM_ME) - 1, ", \n", UART_FAILED);
	PARSE_NEXTVALUE(token, &usedr, ", \n", UART_FAILED);
	PARSE_NEXTVALUE(token, &totalr, ", \n", UART_FAILED);

	if (check_address_empty(sim->iMaxMessages) || sim->iMaxMessages != totalr) {
		sim->iMaxMessages = totalr;
	}

	if (usedr == 0) {
	//	lcd_printf(LINEC, "No new messages", usedr);
                return UART_SUCCESS;
	}
#ifndef _FISM_
	lcd_printf(LINEC, "%d SMS Fetch", usedr);
	lcd_printl(LINE2, get_string(LCD_MSG, MSG_PROCESSING));
#endif
	uart_tx("+CSDH=0"); // Disable extended output

	//uart_tx("AT+CSCS=\"PCCP437\"\r\n");		// Select PC character set to suppot pipes in config SMS
	for (iIdx = 1; iIdx <= totalr; iIdx++) {
		if (sms_process_memory_message(iIdx) == UART_SUCCESS) {
                  //SMS get deleted if other config SMS processed,
                  //trials crossed in case of network failure, crc mismatched
               if (g_pSysState->type == 1 || g_pSysState->type == 3
                      || g_pSysState->fw_flag == 1) {
                        usedr--;
			delmsg(iIdx);
                } else
                    return 0;

                        if(usedr == 0)
                          break;
                }
//sachin		} else {
//			_NOP();
//                        delmsg(iIdx);
//                }
	}

        if( parse_config_sms == 1 && g_pSysState->type == 1) {
           parse_config_sms = 0;
           http_setup();
//           lcd_printl(LINE2,"Sending ConfAck");
//           delay(1000);
//sachin           lcd_printl(LINE1, "DRY");
           if (data_send_api_packet(API_PACKET_DEV_READY) != 0 && g_pDevCfg->cfgSMSmode.bits.APIFailover) {
              sms_send_dataready();
           }
           state_alarm_reset_all();
        }

	delallmsg();
	uart_tx("+CSDH=1"); // Restore extended output
	return UART_SUCCESS;
}

void sms_send_dataready() {

    	SENSOR_STATUS *sensorState;
        uint8_t slot;
	int i = 0;
        char sensors[12];
        char gwkey[MAX_SI_LEN +2];
  	char *msg = getSMSBufferHelper();

        gwkey[0] = 0;
        if (strlen(g_pDevCfg->cfgGatewayKey) > 0) {
          sprintf(gwkey, "%s ", g_pDevCfg->cfgGatewayKey);
        }
	sensors[0] = 0;
	for (i = 0; i < SYSTEM_NUM_SENSORS; i++) {
          sensorState = getAlarmsSensor(i);
          if (!sensorState->state.disabled) {
            if (i != 0) {
              strcat(sensors, ":");
            }
            strcat(sensors, SensorName[i]);
          } else {
            if (i != 0) {
              strcat(sensors, ":");
            }
          }
	}

	slot = config_getSelectedSIM();

        /* v=1,t=2,i=123456789012345,s=123456789012345,p=+911234512345,s2=123456789012345,
            p2=+911234512345,im=123456789012345
            d=10.2.3,m=1.0.1,a=CT5,as=A:B:C:D */
        sprintf(msg, "%sv=%d,t=%d,i=%s,s=%s,p=%s,s2=%s,p2=%s,im=%s,d=%s,a=%s,as=%s,l=%d",
                          gwkey, SMS_MSG_VERS, SMS_MSG_TYPE_DATAREADY,
                          g_pDevCfg->cfgIMEI,
                          g_pDevCfg->SIM[0].cfgSimId, g_pDevCfg->SIM[0].cfgPhoneNum,
                          g_pDevCfg->SIM[1].cfgSimId, g_pDevCfg->SIM[1].cfgPhoneNum,
                          g_pDevCfg->cfgIMEI,fw_version, g_pDevCfg->cfgVersion, sensors, slot);

	sms_send_message(msg);
/*	if (g_bServiceMode == true)
	{
		if(ret)
			lcd_printl(LINE1, get_string(LCD_MSG, HEARTBEAT_NOT_SENT));
		else
			lcd_printl(LINE2, get_string(LCD_MSG, HEARTBEAT_SENT));
		delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
	}
        */
}

uint8_t sms_send_message_number(char *szPhoneNumber, char* pData) {
	char szCmd[64];
	uint16_t msgNumber = 0; // Validation number from the network returned by the CMGS command
	int res = UART_ERROR;
#ifndef _FISM_
	int verbose = g_iLCDVerbose;
#endif
	int phonecode = 129;
	char *token;
	SIM_CARD_CONFIG *sim = config_getSIM();
#ifndef _FISM_
	log_appendf(LOG_TYPE_MODEM, "sms_send_message_number: Phone Number:%s, Msg:%s \n", szPhoneNumber, pData);
#endif
//	state_SMS_lastMessageACK(sim, -1);
//sachin        delay(1000);

	if (szPhoneNumber == NULL || strlen(szPhoneNumber) == 0) {
// sachin               log_append_(LOG_TYPE_MODEM, "Return due to phone number empty \n");
		return UART_FAILED;
        }

	if (pData == NULL || strlen(pData) == 0) {
//sachin                log_append_(LOG_TYPE_MODEM, "Return due to msg empty \n");
		return UART_FAILED;
        }

#ifndef _FISM_
	if (g_iLCDVerbose == VERBOSE_BOOTING) {
//sachin		lcd_clear();
		lcd_printf(LINEC, "SYNC SMS %d", g_pDevCfg->cfgSIM_slot + 1);
		lcd_printl(LINE2, szPhoneNumber);
		lcd_disable_verbose();
	}
#endif
	strcat(pData, ctrlZ);

	// 129 - number in national format
	// 145 - number in international format (contains the "+")

	if (szPhoneNumber[0] == '+')
		phonecode = 145;

	http_deactivate();
	sprintf(szCmd, "AT+CMGS=\"%s\",%d\r\n", szPhoneNumber, phonecode);

	uart_setSMSPromptMode();
	if (!uart_tx_waitForPrompt(szCmd, TIMEOUT_CMGS_PROMPT)) {
		uart_tx_data(pData, TIMEOUT_CMGS, 1);

		token = strstr(uart_getRXHead(), "ERROR");
		if (token == NULL) {
			token = strstr(uart_getRXHead(), "+CMGS:");
			if (token != NULL) {
				msgNumber = atoi(token + 6);
//				state_SMS_lastMessageACK(sim, msgNumber);
			}
			if (msgNumber != 0)
				res = UART_SUCCESS;
			else
				res = UART_ERROR;
		} else {
			res = UART_ERROR;
		}
	}
#ifndef _FISM_
	if (verbose == VERBOSE_BOOTING)
		lcd_enable_verbose();
#endif
	//TODO: this should be debug
	/*
	if (res == UART_SUCCESS) {
		lcd_clear();
		lcd_printl(LINE1, "SMS Confirm");
		lcd_printf(LINE2, "MSG %d", msgNumber);
		delay(HUMAN_DISPLAY_INFO_DELAY);
		_NOP();
	} else {
		lcd_print("TIMEOUT");
		delay(HUMAN_DISPLAY_ERROR_DELAY);
	}
	*/

	_NOP();
	return res;
}

uint8_t sms_send_message(char* pData) {
	return sms_send_message_number(g_pDevCfg->cfgGatewaySMS, pData);
}

void delallmsg() {
	uart_tx("+CMGD=1,2");	//delete all the sms msgs read or sent successfully
}

void delallmsgs() {
	uart_tx("+CMGD=10,4");	//delete all the sms msgs read , unread, sent successfully
}

void delmsg(int8_t iMsgIdx) {
	uart_txf("+CMGD=%d,0", iMsgIdx);
#ifndef _FISM_
	lcd_progress_wait(3000);
#endif
}
#else


/***********************************HEADER FILES************************************************/
#include "ota.h"
#include "msp430.h"
#include "string.h"
#include "sms.h"
#include "events.h"
#include "stdio.h"
#include "stdint.h"
#include "config.h"
#include "modem.h"
#include "main_system.h"
#include "spi_flash.h"
#include "modem_uart.h"
#include "watchdog.h"

extern CONFIG_DEVICE *g_pDevCfg;
extern unsigned char fw_version[];
char user_num[15];
char ota_url[75];
char image_crc[10];

char number[15];
    int16_t dur_hot_delay=0, dur_cold_delay=0;
    float temp_cold;
    float temp_hot;

/*********************************FUNCTION DEFINITIONS*****************************************/


#ifdef _FISM_ //aditya

int totalsms_count()
{
    char *token;
    int totalsms=0;
    int totsmscap=0;
    send_cmd_wait("AT+CPMS?\r\n","OK\r\n",5,1);

    PARSE_FINDSTR_RET(token, "+CPMS: ", UART_FAILED);
    PARSE_FIRSTSKIP(token, ",", UART_FAILED);
    PARSE_NEXTVALUE(token, &totalsms, ",", UART_FAILED);
    PARSE_NEXTVALUE(token, &totsmscap, ",", UART_FAILED);

#ifdef _FDEBUG_
    debug_print("total sms = ");
    debug_print(itoa_nopadding(totalsms));
    debug_print("\n\r");
    debug_print("total sms storage = ");
    debug_print(itoa_nopadding(totsmscap));
    debug_print("\n\r");
#endif

    if(totalsms == NULL)
    {
        return 0;
    }

    return totsmscap;
}

uint8_t alert_num_reg(char *token)
{
      uint8_t status = UART_FAILED;
      PARSE_NEXTSTRING(token, number, sizeof(number), ":", UART_FAILED);

      if(strchr(number, '+') != NULL) {
        debug_print("number=");
        debug_print(number);
          status = UART_SUCCESS;
      }
      return status;
}

int sensor_sel(char *buffer)
{
    int sensor_num = 0;
    char temp_val[10];

    //check Sensor ID in buffer data
    buffer = strstr(buffer, "sId");
    PARSE_FIRSTSKIP(buffer, "=", UART_FAILED);
    PARSE_NEXTSTRING(buffer, temp_val, sizeof(temp_val), "\r\n", UART_FAILED);

    if(strcmp(temp_val, "a") == 0)
    {
      sensor_num = 0;
    }
    else if(strcmp(temp_val, "b") == 0)
    {
      sensor_num = 1;
    }
     else if(strcmp(temp_val, "c") == 0)
    {
      sensor_num = 2;
    }
    else
    {
      sensor_num = -1;
    }

    return sensor_num;
}


void sensor_config(int index)
{
      char value[20];
      TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[index];
      pAlertParams->maxSecondsCold = MINUTES_(dur_cold_delay);
      pAlertParams->threshCold = temp_cold;
      pAlertParams->maxSecondsHot = MINUTES_(dur_hot_delay);
      pAlertParams->threshHot = temp_hot;

#ifdef _FDEBUG_
      debug_print("pAlertParams->threshHot=");
      getFloatNumber2Text(pAlertParams->threshHot, value);
      debug_print(value);
      debug_print("\n\r");
      debug_print("pAlertParams->maxSecondsHot=");
      debug_print(itoa_pad(pAlertParams->maxSecondsHot));
      debug_print("\n\r");
      debug_print("pAlertParams->threshCold=");
      getFloatNumber2Text(pAlertParams->threshCold, value);
      debug_print(value);
      debug_print("\n\r");
      debug_print("pAlertParams->maxSecondsCold=");
      debug_print(itoa_pad(pAlertParams->maxSecondsCold));
      debug_print("\n\r");
#endif
}


uint8_t dev_sms_parsing(uint8_t index)
{
    char *token;

    flags.sms_config = 1;
    token = (char *)uart_getRXHead();
    config_parse_configuration_sms(token);
    return UART_SUCCESS;
}

void dev_sms_cofig()
{
  uint8_t index=0;
  char sms_val[50];
  int total_sms=0;

  flags.sim_ready = 0;
  if(send_cmd_wait("AT+CPIN?\r\n", "READY\r\n",5,1))
  {
    flags.sim_ready = 1;
    total_sms = totalsms_count();
    if(total_sms == NULL)
    {
      debug_print("no sms found\n\r");
      return;
    }

    memset(sms_val,'\0', sizeof(sms_val));
    while(index++ < total_sms)
    {
      //Read SMS
      sprintf(sms_val,"AT+CMGR=%d\r\n",index);
      send_cmd_wait(sms_val,"OK\r\n",10,2);
      debug_print("GSM_buffer=");
      debug_print(GSM_buffer);
      ota_sms(GSM_buffer);

      if(flags.check_ota == 0)
      {
        dev_sms_parsing(index);
  #ifndef _FISM_
        memset(sms_val,0, sizeof(sms_val));
  #endif
        }
    }
        if(flags.param_updated)
        {
            event_force_event_by_id(EVT_UPLOAD_SAMPLES, 0);
            send_deviceready();
            flags.param_updated = 0;
            state_alarm_reset_all();
            event_force_event_by_id(EVT_UPLOAD_SAMPLES, 0);
            event_force_event_by_id(EVT_SUBSAMPLE_TEMP, 0);
        }
        flags.sms_config = 0;

        if(flags.check_ota == 0)
        {
#ifdef _FISM_
          send_cmd_wait("AT+CMGD=1,3\r\n","OK\r\n",10,3);           //Delete Readed SMS
          memset(sms_val,0, sizeof(sms_val));
#endif
        }
      }
}



uint8_t gprs_config_dev()
{
  char *token;
  char msg[200];

  http_open_connection_upload(0,CONFIGURATION_URL_PATH);
  if(send_cmd_wait("AT+HTTPACTION=0\r\n", ",200", 120,3))
  {
    if(send_cmd_wait("AT+HTTPREAD\r\n", "+HTTPREAD", 60, 2))
    {
      config_parse_configuration_http();
      flags.param_updated = 0;
      return UART_SUCCESS;
    }
  }
  return UART_FAILED;
}
#endif
/**********************************************************************************************
   * File Name : sms.c
   * Function Name : void parse_sms(void)
   * Parameters : None
   * Return : None
   * Description : parse sms contents
   * 		Check if sms received is from a valid mobile number or not
    check if sms received is from a valid mobile number or through internet sms applications and if valid,
     *  store the reveived mobile number to send acknowledgement back on same number.
     *	Sample Response:
     *	+CMGR: "REC UNREAD","+917834895318","","16/05/05,15:51:32+22"
     *	0000,A10,30
     *	OK
     *	Invalid Response:
     *	+CMGR: "REC UNREAD","44*5w275149535w435","","16/05/05,15:52:14+22"
     */
 /**********************************************************************************************/
void ota_sms(char *buffer)
{
    char *command;
    uint8_t sms_command_code;
    char *ret_ptr=0;
    char *resp=0;
    char *ret_val=0;
    char cloud_fw[12];
    unsigned char count = 0;


    char local_buffer[200];

    memset(local_buffer,'\0',sizeof(local_buffer));
    memset(cloud_fw,'\0',sizeof(cloud_fw));

      ret_ptr=strstr(buffer,"fwu");
      if(ret_ptr)
      {
        resp=strstr(buffer,";");
        if(resp)
        {
          for(count = 0;count < 12; count++)
          {
            if((resp[count+1]== '\0') || (resp[count+1]== '\r') || (resp[count+1]== '\n'))
            {
              break;
            }
            cloud_fw[count] = resp[count+1];
          }
        }
        if(strcmp(cloud_fw,fw_version) != 0)
        {
          resp=strstr(buffer,"crc=");
          ret_ptr = strtok(ret_ptr+4,",");
          resp = strtok(resp+4,";");
          memset(ota_url,'\0',sizeof(ota_url));
          memset(image_crc,'\0',sizeof(image_crc));
          strcpy(ota_url,ret_ptr);
          strcpy(image_crc,resp);
          if(atoi(image_crc)<= 65535);
          {
              flags.check_ota = 1;
              debug_print("\r\nOTA_Will_Start\r\n");
          }
        }
        else
        {
           return;
        }
      }
}
/**********************************************************************************************
 * File Name : sms.c
 * Function Name : void delete_sms(void)
 * Parameters : None
 * Return : None
 * Description : send the sms
 **********************************************************************************************/
void delete_sms(void)
{
  // to check this AT+CNMI
  send_cmd_wait(NEW_SMS_INDICATION, "OK\r\n", 10, 1); // set new SMS indication
  send_cmd_wait(DELETE_SMS, "OK\r\n", 25, 1); // deleting all SMS
}
#endif
