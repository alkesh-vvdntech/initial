/*
 * monitor_alarm.c
 *
 *  Created on: Jun 2, 2015
 *      Author: sergioam
 */

#include "thermalcanyon.h"
#include "sms.h"
#include "rtc.h"
#include "globals.h"
#include "temperature.h"
#include "events.h"
#include "alarms.h"
#include "debug.h"

#define SMS_ALERT

//sachin   char error[2][11] = {"SD FAILURE", "BAD TIME"};
/*************************************************************************************************************/
/* Monitor alarm */
/*************************************************************************************************************/

/*************************************************************************************************************/
/*void alarm_sms_battery_level() {
	char *msg = getSMSBufferHelper();
	int i;

	if (!g_pDevCfg->cfg.logs.sms_alerts)
		return;
        
        lcd_printl(LINEC, "Sending Batt");
	lcd_printl(LINE2, "Alarm SMS");
        
        // Alert: Battery percentage below 20%. Please refer to SOP
        
        sprintf(msg, "Alert! Battery precentage below %d%%. ", g_pDevCfg->stBattPowerAlertParam.battThreshold);
	strcat(msg, get_string(SMS_MSG, HMN_RDBL_TAKE_ACTION));

	// Send to all SMS numbers
	for (i = 0; i < MAX_SMS_NUMBERS; i++){
		sms_send_message_number(g_pDevCfg->cfgSMSNumbers[i].cfgReportSMS, msg);
	}

}*/

/*void alarm_sms_sensor(uint8_t sensorId, time_t elapsed) {
  	USE_TEMPERATURE
	TEMPERATURE_SENSOR *sensor = &tem->sensors[sensorId];
	char *msg = getSMSBufferHelper();
        char totaltime[11];
	int i;
        totaltime[0] = '\0';

	if (!g_pDevCfg->cfg.logs.sms_alerts)
		return;
        
        lcd_printl(LINEC, "Sending Temp");
	lcd_printl(LINE2, "Alarm SMS");
        
        // Alert: Temp to high for X mins on sensor C. Currently 21.3C. Please refer to SOP
        
        if ((elapsed / 60) < 60) {
          sprintf(totaltime, "%d mins", elapsed / 60);
        } else if ((elapsed / 60) < 1440) {
          sprintf(totaltime, "%d hrs", elapsed / (3600));
        } else {
          sprintf(totaltime, "%d days", elapsed / (86400));  
        }
        
        if (sensor->fTemperature >= g_pDevCfg->stTempAlertParams[sensorId].threshHot) {
              sprintf(msg, "Alert! Temp too high for %s on Sensor %s. Currently %sC. ", totaltime,
                            SensorName[sensorId], temperature_getString(sensorId));
        }
        if (sensor->fTemperature <= g_pDevCfg->stTempAlertParams[sensorId].threshCold) {
              sprintf(msg, "Alert! Temp too low for %s on Sensor %s. Currently %sC. ", totaltime,
                            SensorName[sensorId], temperature_getString(sensorId));
        }
        
	strcat(msg, get_string(SMS_MSG, HMN_RDBL_TAKE_ACTION));

	// Send to all SMS numbers
	for (i = 0; i < MAX_SMS_NUMBERS; i++){
		sms_send_message_number(g_pDevCfg->cfgSMSNumbers[i].cfgReportSMS, msg);
	}
}

void alarm_sms_power_outage(time_t elapsed) {
  	char *msg = getSMSBufferHelper();
        char totaltime[14];
	int i;
        totaltime[0] = '\0';
        
	if (!g_pDevCfg->cfg.logs.sms_alerts)
		return;
        
        lcd_printl(LINEC, "Sending Power");
	lcd_printl(LINE2, "Alarm SMS");
        
        // Alert: Power outage for X mins. Please refer to SOP
        
        if ((elapsed / 60) < 60) {
          sprintf(totaltime, "%d mins", elapsed / 60);
        } else if ((elapsed / 60) < 1440) {
          sprintf(totaltime, "%d hrs", elapsed / 3600);
        } else {
            sprintf(totaltime, "%d days", elapsed / 86400);  
        }
        
	sprintf(msg, "Alert! Power Outage for %s. ", totaltime);
        strcat(msg, get_string(SMS_MSG, HMN_RDBL_TAKE_ACTION));
        
	// Send to all SMS numbers
	for (i = 0; i < MAX_SMS_NUMBERS; i++){
		sms_send_message_number(g_pDevCfg->cfgSMSNumbers[i].cfgReportSMS, msg);
	}
}
*/
        char error[7][4] = {"Sen","SD","TIM","PWR","BAT","FWU","\0"};
void alarm_sms(uint8_t Id, time_t elapsed){
        USE_TEMPERATURE
	TEMPERATURE_SENSOR *sensor = &tem->sensors[Id];
        char *msg = getSMSBufferHelper();

        char totaltime[14];
	int i;
        totaltime[0] = '\0';

#ifndef _FISM_
	if (!g_pDevCfg->cfg.logs.sms_alerts)
		return;
#endif

        if (Id >=0 && Id <= 6)
          i = 0;
        else if(Id > 6 && Id <=11)
          i = Id-6;
#ifndef _FISM_
        lcd_printf(LINEC, "Sending %s", error[i]);
	lcd_printl(LINE2, "Alarm SMS");
        delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
#endif

        // Alert: Power outage for X mins. Please refer to SOP
        
        if ((elapsed / 60) < 60) {
          sprintf(totaltime, "%d mins", elapsed / 60);
        } else if ((elapsed / 60) < 1440) {
          sprintf(totaltime, "%d hrs", elapsed / 3600);
        } else {
            sprintf(totaltime, "%d days", elapsed / 86400);
        }
        
        if (Id == POWER_FAILURE) {
                sprintf(msg, "Alert! Power Out for %s.", totaltime);
        }
        else

          if(Id >=0 && Id <=6) {
                if (sensor->fTemperature >= g_pDevCfg->stTempAlertParams[Id].threshHot) {
                        sprintf(msg, "Alert! Temp too high for %s on Sen %s. Currently %sC.", totaltime,
                            SensorName[Id], temperature_getString(Id));
                }
                if (sensor->fTemperature <= g_pDevCfg->stTempAlertParams[Id].threshCold) {
                sprintf(msg, "Alert! Temp too low for %s on Sen %s. Currently %sC.", totaltime,
                            SensorName[Id], temperature_getString(Id));
                }
          } 
#ifndef _FISM_
          else if(Id == SD_FAILURE){
          strcpy(msg, "Alert:SD ERR:");
           if (g_szLastSD_CardError != NULL && strlen(g_szLastSD_CardError) ) {
		strcat(msg, g_szLastSD_CardError);
          }
        } 
#endif
          else if (Id == TIME_FAILURE) {
              strcpy(msg, "Alert:TIME ERR.");
        }

        else if (Id == BATT_FAILURE) {
          sprintf(msg, "Alert! Batt below %d%%.", g_pDevCfg->stBattPowerAlertParam.battThreshold);
        }
        else if (Id == FWU_FAILURE) {
          strcpy(msg, "Alert:FWU ERR.");
        }

        //strcat(msg, get_string(SMS_MSG, HMN_RDBL_TAKE_ACTION));

    for (i = 0; i < MAX_SMS_NUMBERS; i++)
    {
       send_sms(msg, g_pDevCfg->cfgSMSNumbers[i].cfgReportSMS);
    }
}

/*void alarm_case_failure(ALARM_TYPE type) {
	char *msg;

	data_send_alarm();
	
}*/
 /*sachin 	if (!g_pDevCfg->cfg.logs.sms_alerts)
		return;
        
        lcd_printl(LINEC, "Sending");
	lcd_printl(LINE2, "Alarm SMS");
	
         // Alert: SD CARD ERROR: <card error text>
        msg = getSMSBufferHelper();
       if(g_pSysState->state.alarms.SD_card_failure) { 
	strcpy(msg, "Alert: SD CARD ERROR");
       } else if(g_pSysState->state.alarms.time_failure) { 
	strcpy(msg, "Alert: TIME ERROR");
        }
        
        if (g_szLastSD_CardError != NULL && strlen(g_szLastSD_CardError) ) {
		strcat(msg, " : ");
		strcat(msg, g_szLastSD_CardError);
	}
	// Send to all SMS numbers
	int i;
	for (i = 0; i < MAX_SMS_NUMBERS; i++){
		sms_send_message_number(g_pDevCfg->cfgSMSNumbers[i].cfgReportSMS, msg);
	}	
}*/

SENSOR_STATUS *getAlarmsSensor(int id) {
	USE_TEMPERATURE
	return &tem->state[id];
}

void alarm_test_sensor(int id) {
  char tmp[TEMP_DATA_LEN+1];
  USE_TEMPERATURE
  SENSOR_STATUS *s = getAlarmsSensor(id);
  TEMPERATURE_SENSOR *sensor = &tem->sensors[id];
  TEMP_ALARM_STATUS *alarmCounts = &tem->alarms[id];
  TEMP_ALERT_PARAM *pAlertParams = &g_pDevCfg->stTempAlertParams[id];
  ALARM_REPEAT_PARAM * pRepeatParams = &g_pDevCfg->stAlarmRepeatParam;

  UINT bytes_written;
  time_t elapsed = 0;
  time_t offset;

  memset(tmp,0,sizeof(tmp));
  float temperature = sensor->fTemperature;

  if (temperature < TEMP_CUTOFF) {
    if(s->state.disconnected == false) {
        s->state.disconnected = true;
        s->state.connStateChange = true;
        if (s->state.alarm) {
            s->status &= STATUS_NO_ALARM;
        }
        alarmCounts->alarm_time = 0;
        alarmCounts->alarm_count = 0;
    }
    return;
  } else if (s->state.disconnected == true) {
    s->state.disconnected = false;
    s->state.connStateChange = true;
  }

  if (temperature > pAlertParams->threshCold && temperature < pAlertParams->threshHot) {
      if (s->state.alarm) {
          s->status &= STATUS_NO_ALARM;
      }
      alarmCounts->alarm_time = 0;
      alarmCounts->alarm_count = 0;
      return;
  }

  if (alarmCounts->alarm_time == 0) {
      alarmCounts->alarm_time = rtc_getUTCsecs();
  } else {
      elapsed = rtc_getUTCsecs() - alarmCounts->alarm_time;
  }

  offset = temperature <= pAlertParams->threshCold ?  pAlertParams->maxSecondsCold :  pAlertParams->maxSecondsHot;
  
  if(s->state.alarm == true) {
      if (elapsed > ((time_t)pRepeatParams->repeat_interval_secs * alarmCounts->alarm_count + offset) &&
          alarmCounts->alarm_count <= pRepeatParams->repeat_count_max) {
            goto alarm_error;
      }
      return;
  } else if (elapsed <= offset) {
      return;
  }

   alarm_error:
	s->state.alarm = true; //set alarm flag
#ifndef _FISM_
	state_alarm_turnon_buzzer();
	state_alarm_on("TEMP ALARM");
//	alarm_sms_sensor(id, elapsed);
#ifndef _EVIN_BUILD_
        //lcd_printl(LINEC, "__EXCUR__");
        //delay(2000);
          g_pSysState->time_flag = 1;
          g_pSysState->system.switches.timestamp_on = 1;
          g_pSysState->excursion_alarm_set[id] = true; 
#ifndef _FISM_
          log_sample_to_disk(&bytes_written);
#endif
          g_pSysState->time_flag = 0;
#endif

#endif
        alarm_sms(id, elapsed);
	(alarmCounts->alarm_count)++;
#ifndef _FISM_
#ifndef _EVIN_BUILD_
        event_force_upload(0);
#endif
#endif
        
}

void alarm_monitor() 
{
    int8_t c;
    for (c = 0; c < SYSTEM_NUM_SENSORS + 1; c++) {
            alarm_test_sensor(c);
    }

}
