/*
 * jsonAPIs.c
 *
 *  Created on: Oct 2, 2015
 *      Author: Ben
 */

#include "thermalcanyon.h"
#include "jsonAPIs.h"
#include "alarms.h"

//top level json structures
void json_print_api_header(json_printer *printer, uint8_t include_simslot) {
        
	json_print_raw(printer, JSON_OBJECT_BEGIN, NULL, 0);
	json_print_raw(printer, 	JSON_KEY, "vId", 3);
	json_print_raw(printer,		 JSON_STRING, "Nexleaf", 7);
        if (include_simslot) {
          	json_print_asm_pair(printer);
        }
	json_print_raw(printer,		JSON_KEY, "data", 4);
	json_print_raw(printer,			JSON_ARRAY_BEGIN, NULL, 0);

}

void json_print_api_footer(json_printer *printer) {

	json_print_raw(printer,			JSON_ARRAY_END, NULL, 0);
	json_print_raw(printer,	JSON_OBJECT_END, NULL, 0);

}

void json_print_data_header(json_printer *printer) {

	json_print_raw(printer,	JSON_OBJECT_BEGIN, NULL, 0);
	json_print_raw(printer,		 JSON_KEY, "dId", 3);
	json_print_raw(printer,		JSON_STRING, g_pDevCfg->cfgIMEI, strlen(g_pDevCfg->cfgIMEI));

}

void json_print_data_footer(json_printer *printer) {

	json_print_raw(printer, JSON_OBJECT_END, NULL, 0);

}

void json_print_asm_pair(json_printer *printer) {
    uint8_t slot = config_getSelectedSIM(); //current sim
    
    json_print_raw(printer, JSON_KEY, "asm", 3);
    json_print_raw(printer,	 JSON_INT, itoa_nopadding(slot), 1);
                
}

void json_print_sId_pair(json_printer *printer, int sensor) {

	json_print_raw(printer, JSON_KEY, "sId", 3);
	json_print_raw(printer,	 JSON_STRING, SensorName[sensor], 1);

}

void json_print_stat_pair(json_printer *printer, int status) {

	json_print_raw(printer, JSON_KEY, "stat", 4);
	json_print_raw(printer,	 JSON_INT, itoa_nopadding(status), 1);

}

void json_print_time_pair(json_printer *printer, time_t timeUTC) {
	char *utcString = itoa_pad(timeUTC);

	json_print_raw(printer,	JSON_KEY, "time", 4);
	json_print_raw(printer,	 JSON_INT, utcString, strlen(utcString));

}

void json_print_pwrAvail_pair(json_printer *printer) {
	int pwrStatus = g_pSysState->system.switches.power_connected;
	
  	json_print_raw(printer,	JSON_KEY, "pwa", 3);
  	json_print_raw(printer,	JSON_OBJECT_BEGIN, NULL, 0);

	json_print_stat_pair(printer, pwrStatus);
	json_print_time_pair(printer, g_pSysState->powerEventTimestamp );

	json_print_raw(printer,	JSON_OBJECT_END, NULL, 0);
	
}

//temperature upload json structures
void json_print_temp_header(json_printer *printer) {

	json_print_raw(printer,	JSON_KEY, "tmps", 4);
	json_print_raw(printer,		JSON_ARRAY_BEGIN, NULL, 0);

}

void json_print_temp_obj(json_printer *printer, int sensor, time_t timeUTC) {
	char *tempString = temperature_getString(sensor);
	SENSOR_STATUS *sensorState = getAlarmsSensor(sensor);
	int type = JSON_TEMP_TYPE_RAW;
	
	if (sensorState->state.incursion) {
		type = JSON_TEMP_TYPE_INC;
		sensorState->state.incursion = false;
		g_pSysState->system.switches.tempBoundaryCrossed = true;
	} else if(sensorState->state.excursion) {
		type = JSON_TEMP_TYPE_EXC;
		sensorState->state.excursion = false;
		g_pSysState->system.switches.tempBoundaryCrossed = true;
	} 
#ifndef _EVIN_BUILD_
        else if(g_pSysState->excursion_alarm_set[sensor]) {
          //lcd_printl(LINEC, "__JSON__");
        //delay(2000);
		type = JSON_TEMP_TYPE_ALR;
        g_pSysState->excursion_alarm_set[sensor] = false;
        }
#endif

	json_print_raw(printer,	JSON_OBJECT_BEGIN, NULL, 0);

	json_print_sId_pair(printer, sensor); //char const reuse

	json_print_time_pair(printer, timeUTC);

#ifdef _EVIN_BUILD_       
	json_print_raw(printer,		JSON_KEY, "typ", 3);
	json_print_raw(printer,		 JSON_INT, itoa_nopadding(type), 1);
#else
        if(type != 0 ) {
                json_print_raw(printer,		JSON_KEY, "typ", 3);
                json_print_raw(printer,		 JSON_INT, itoa_nopadding(type), 1);
        }
#endif   
        json_print_raw(printer,		JSON_KEY, "tmp", 3);
	json_print_raw(printer,		 JSON_FLOAT, tempString, strlen(tempString));

	json_print_raw(printer,	JSON_OBJECT_END, NULL, 0);

}

#ifndef _EVIN_BUILD_
void json_print_pwrinfo_obj(json_printer *printer, time_t timeUTC) {
    int battLevel = batt_getlevel(), pwrStatus = g_pSysState->system.switches.power_connected;
    int sigstr = g_pSysState->signal_level;

    if (g_pSysState->power_alarm_set) {
      g_pSysState->power_alarm_set = false;
      pwrStatus = 3;
    }
        battLevel = battLevel > 99 ? 99 : battLevel; //always only two digits here
    
	json_print_raw(printer,	JSON_OBJECT_BEGIN, NULL, 0);

	json_print_raw(printer, JSON_KEY, "sId", 3);
	json_print_raw(printer,	 JSON_STRING, "P", 1);

	json_print_time_pair(printer, timeUTC);

	json_print_raw(printer,		JSON_KEY, "pwr", 3);
	json_print_raw(printer,		 JSON_INT, itoa_nopadding(pwrStatus), 1);
	json_print_raw(printer,	    JSON_KEY, "batt", 4);
	json_print_raw(printer,		 JSON_INT, itoa_pad(battLevel), 2);
	json_print_raw(printer,		JSON_KEY, "ss", 2);
	json_print_raw(printer,		 JSON_INT, itoa_pad(sigstr), 2);
        
	json_print_raw(printer,	JSON_OBJECT_END, NULL, 0);
}
#endif

//device ready json structures
void json_print_simDevRdy_obj(json_printer *printer, int slot) {
	char *simId, *simPhoneNum;
	slot = slot ? 1 : 0; //the only allowed non-zero value is 1, ***dual-sim hardware
	simId = g_pDevCfg->SIM[slot].cfgSimId;
	simPhoneNum = g_pDevCfg->SIM[slot].cfgPhoneNum;

	if (slot == 0)
		json_print_raw(printer,	JSON_KEY, "sim", 3);
	else
		json_print_raw(printer,	JSON_KEY, "altSim", 6);
	json_print_raw(printer,		JSON_OBJECT_BEGIN, NULL, 0);
	json_print_raw(printer,			JSON_KEY, "sid", 3);
	json_print_raw(printer,			 JSON_STRING, simId, strlen(simId));
	json_print_raw(printer,			JSON_KEY, "phn", 3);
	json_print_raw(printer,		 	 JSON_STRING, simPhoneNum, strlen(simPhoneNum));
	json_print_raw(printer,		JSON_OBJECT_END, NULL, 0);

}

void json_print_devDevRdy_obj(json_printer *printer) {
	char fw_vers[13] = "\0";	

    sprintf(fw_vers, "v%d.%d.%d", FW_VER_MAJ, FW_VER_MIN, FW_VER_MINMIN);

	json_print_raw(printer,	JSON_KEY, "dev", 3);
	json_print_raw(printer,		JSON_OBJECT_BEGIN, NULL, 0);
	json_print_raw(printer,			JSON_KEY, "imei", 4);
	json_print_raw(printer,			 JSON_STRING, g_pDevCfg->cfgIMEI, strlen(g_pDevCfg->cfgIMEI));
	json_print_raw(printer,			JSON_KEY, "dVr", 3);
	json_print_raw(printer,			 JSON_STRING, fw_vers, strlen(fw_vers));
	json_print_raw(printer,			JSON_KEY, "mVr", 3);
	json_print_raw(printer,			 JSON_STRING, g_pDevCfg->ModemFirmwareVersion, strlen(g_pDevCfg->ModemFirmwareVersion));
	json_print_raw(printer,			JSON_KEY, "mdl", 3);
	json_print_raw(printer,			 JSON_STRING, g_pDevCfg->cfgVersion, strlen(g_pDevCfg->cfgVersion));
	json_print_raw(printer,		JSON_OBJECT_END, NULL, 0);

}

void json_print_actSnsDevRdy_obj(json_printer *printer) {
	SENSOR_STATUS *sensorState;
	int i;

	json_print_raw(printer,	JSON_KEY, "actSns", 6);
	json_print_raw(printer,		JSON_ARRAY_BEGIN, NULL, 0);

	for(i = 0; i < SYSTEM_NUM_SENSORS; i++) {
		sensorState = getAlarmsSensor(i);
		//if(!sensorState->state.disconnected && !sensorState->state.disabled)
                if(!sensorState->state.disabled) {
                  // only include disabled sensors... device alarm will communicate disconenctions
			json_print_raw(printer,	JSON_STRING, SensorName[i], 1);
                }
	}

	json_print_raw(printer,		JSON_ARRAY_END, NULL, 0);

}

//device alarm json structures
void json_print_dvcAlarm_header(json_printer *printer) {

	json_print_raw(printer,	JSON_KEY, "dvc", 3);
	json_print_raw(printer,		JSON_OBJECT_BEGIN, NULL, 0);

}

void json_print_dvcAlarm_footer(json_printer *printer) {

	json_print_raw(printer, 	JSON_OBJECT_END, NULL, 0);

}
/*
void json_print_connAlarm_obj(json_printer *printer, int sensor, time_t timeUTC) {
	int connStatus = g_pSysState->temp.state[sensor].state.disconnected;

	json_print_raw(printer,	JSON_KEY, "dCon", 4);
	json_print_raw(printer,		JSON_OBJECT_BEGIN, NULL, 0);
	json_print_stat_pair(printer, connStatus); //char const reuse
	json_print_time_pair(printer, timeUTC); //char const reuse
	json_print_raw(printer,		JSON_OBJECT_END, NULL, 0);

}
*/

void json_print_extSensAlarm_obj(json_printer *printer, int sensor, time_t timeUTC) {
	int sensStatus = g_pSysState->temp.state[sensor].state.disconnected;

	json_print_raw(printer,	JSON_KEY, "xSns", 4);
	json_print_raw(printer,		JSON_OBJECT_BEGIN, NULL, 0);
	json_print_stat_pair(printer, sensStatus); //char const reuse
	json_print_time_pair(printer, timeUTC); //char const reuse
	json_print_raw(printer,		JSON_OBJECT_END, NULL, 0);

}

void json_print_battAlarm_obj(json_printer *printer, time_t timeUTC) {
	int battStatus = state_get_batt_status(), battLevel = batt_getlevel();

	json_print_raw(printer, JSON_KEY, "batt", 4);
	json_print_raw(printer,		JSON_OBJECT_BEGIN, NULL, 0);
	json_print_stat_pair(printer, battStatus); //char const reuse
	json_print_time_pair(printer, timeUTC);  //char const reuse
	json_print_raw(printer,			JSON_KEY, "avl", 3);
	json_print_raw(printer,			 JSON_INT, itoa_nopadding(battLevel), strlen(itoa_nopadding(battLevel)));
	json_print_raw(printer,		JSON_OBJECT_END, NULL, 0);

}

void json_print_fwErrorAlarm_arr(json_printer *printer, time_t timeUTC) {
  
	json_print_raw(printer, JSON_KEY, "errs", 4);
	json_print_raw(printer, 	JSON_ARRAY_BEGIN, NULL, 0);
	
//	if(g_pSysState->state.alarms.SD_card_failure) {
	  json_print_raw(printer,		JSON_OBJECT_BEGIN, NULL, 0);
	  json_print_raw(printer, 		JSON_KEY, "code", 4);

          if(g_pSysState->state.alarms.SD_card_failure)
            json_print_raw(printer,	 	 JSON_STRING, "SD ERR", 6);
           
          if(g_pSysState->state.alarms.time_failure)
            json_print_raw(printer,	 	 JSON_STRING, "TIME ERR", 8);
          
          if(g_pSysState->fw_flag == 1 )
            json_print_raw(printer,	 	 JSON_STRING, "CRCF", 4);
          
            if(g_pSysState->fw_flag == 2)
            json_print_raw(printer,	 	 JSON_STRING, "NETF", 4);
            
	  //json_print_raw(printer, 		JSON_KEY, "msg", 3);	//optional
	  //json_print_raw(printer,		 JSON_STRING, , ); 	//message
	  
	  json_print_time_pair(printer, timeUTC);
	  
	  json_print_raw(printer,		JSON_OBJECT_END, NULL, 0);
//	}

	json_print_raw(printer, 	JSON_ARRAY_END, NULL, 0);
	
}