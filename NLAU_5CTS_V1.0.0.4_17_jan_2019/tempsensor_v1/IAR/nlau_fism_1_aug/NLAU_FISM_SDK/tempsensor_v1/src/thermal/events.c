/*
 *
 *  Created on: Jun 9, 2015
 *      Author: sergioam
 */

#include "thermalcanyon.h"
#include "main_system.h"
#include "hardware_buttons.h"
#include "data_transmit.h"
#include "sms.h"
#include "fatdata.h"
#include "modem.h"
#include "spi_flash.h"
#include "state_machine.h"
#include "alarms.h"
#include "timer.h"
#include "stringutils.h"


extern unsigned char ota_start_flag;
extern unsigned g_uch_status;
extern char ccid[22];

#ifdef USE_MININI
#include "minIni.h"
#endif

#if defined(__TI_COMPILER_VERSION__)
#pragma SET_DATA_SECTION(".xbigdata_vars")
EVENT_MANAGER g_sEvents;
#pragma SET_DATA_SECTION()
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma location="XBIGDATA"
__no_init EVENT_MANAGER g_sEvents;
#else
#error Compiler not supported!
#endif

/*******************************************************************************************************/
/* Event based system */
/*******************************************************************************************************/

// Commands postponed by their senders.
// Some commands we don't want to run them just when they happen;
void event_run_deferred_commands() {
	SIM_CARD_CONFIG *sim = config_getSIM();
	char error[16];
	if (g_sEvents.defer.status == 0)
		return;

//	config_setLastCommand(COMMAND_EVENT_DEFERRED);

	if (g_sEvents.defer.command.display_config) {
		g_sEvents.defer.command.display_config = 0;
		//config_display_config();
		return;
	}

	if (g_sEvents.defer.command.swap_sim) {
		g_sEvents.defer.command.swap_sim = 0;
		return;
	}

	if (g_sEvents.defer.command.display_http_error) {
		zeroString(error);
		g_sEvents.defer.command.display_http_error = 0;
		if (sim->http_last_status_code != 200 && sim->http_last_status_code != 201
				&& sim->http_last_status_code > 0) {
#ifdef USE_MININI
			ini_gets("STATUS", itoa_nopadding(sim->http_last_status_code),
					"unassigned", error, sizeof(error), "http.ini");
			lcd_printl(LINEC, get_string(LCD_MSG, HTTP_ERR));
	//sachin		lcd_printl(LINEH, error);
#endif
		}
		return;
	}
}

time_t inline event_getIntervalSeconds(EVENT *pEvent) {
	if (pEvent == NULL)
		return PERIOD_UNDEFINED;

	return pEvent->interval_secs;
}

time_t event_getIntervalMinutes(EVENT *pEvent) {
	return (event_getIntervalSeconds(pEvent) / 60);
}

time_t event_getInterval_by_id_secs(EVENT_IDS id) {
	EVENT *pEvent = events_find(id);
	if (pEvent == NULL)
		return PERIOD_UNDEFINED;

	return event_getIntervalSeconds(pEvent);
}

void inline event_setIntervalSeconds(EVENT *pEvent, time_t time_seconds) {
	if (pEvent == NULL)
		return;

	// Intervals more than reset time will not be accepted.
	if (time_seconds > event_getInterval_by_id_secs(EVT_PERIODIC_REBOOT))
		return;

	pEvent->interval_secs = time_seconds;
}

void event_setInterval_by_id_secs(EVENT_IDS id, time_t time_seconds) {
	EVENT *pEvent = events_find(id);
	if (pEvent == NULL)
		return;

	event_setIntervalSeconds(pEvent, time_seconds);
}

time_t events_getTick() {
#ifdef USING_REALTIME_CLOCK
	static time_t oldTime = 0;
	static time_t frameDiff = 0;
	static time_t elapsedTime = 0;

	time_t newTime = rtc_get_second_tick();
	if (oldTime == 0) {
		oldTime = newTime;
		return 0;
	}

	frameDiff = (newTime - oldTime);

	if (frameDiff < 10000) {
		elapsedTime += frameDiff;
	} else {
		_NOP();
	}

	oldTime = newTime;
	return elapsedTime;
#else
	return rtc_get_second_tick();
#endif
}

void events_send_data(char *phone) {
#ifdef _DEBUG_OUTPUT
	char *msg=getSMSBufferHelper();
	EVENT *pEvent;
	int t;
	size_t length;

	if (g_sEvents.registeredEvents == 0)
	return;

	size_t currentTime = events_getTick();
	sprintf(msg, "[%s] Events start",
			get_date_string(&g_tmCurrTime, "-", " ", ":", 1));
	sms_send_message_number(phone, msg);

	for (t = 0; t < g_sEvents.registeredEvents; t++) {
		pEvent = &g_sEvents.events[t];
		length = (pEvent->nextEventRun - currentTime);

		sprintf(msg, "%s : left %d next %d", pEvent->name, (uint16_t) length,
				(uint16_t) event_getIntervalMinutes(pEvent));
		sms_send_message_number(phone, msg);
	}

	//sms_send_data_request(phone);

	sprintf(msg, "[%s] Events end",
			get_date_string(&g_tmCurrTime, "-", " ", ":", 1));
	sms_send_message_number(phone, msg);
#endif
}

/*
 void event_sanity_check(EVENT *pEvent, time_t currentTime) {

 time_t maxRunTime = currentTime + pEvent->offset_secs
 + event_getIntervalSeconds(pEvent);

 if (pEvent->nextEventRun <= maxRunTime)
 return;

 event_init(pEvent, currentTime);
 _NOP();
 }

 void events_sanity(time_t currentTime) {
 int t;
 EVENT *pEvent = NULL;

 for (t = 0; t < g_sEvents.registeredEvents; t++) {
 pEvent = &g_sEvents.events[t];
 event_sanity_check(pEvent, currentTime);
 }
 }
 */

// Populates the next event index depending on event times
void events_find_next_event() {
	int t;
	EVENT *pEvent = NULL;
	time_t nextEventTime = UINT32_MAX;
	uint8_t nextEvent = 0;

	if (g_sEvents.registeredEvents == 0)
		return;

	nextEvent = 0;

	// Search for the event that is closer in time
	for (t = 0; t < g_sEvents.registeredEvents; t++) {
		pEvent = &g_sEvents.events[t];

		// 0 seconds events are disabled
		if (pEvent->interval_secs != 0
				&& pEvent->nextEventRun < nextEventTime) {
			nextEventTime = pEvent->nextEventRun;
			nextEvent = t;
		}
	}

	g_sEvents.nextEvent = nextEvent;

	_NOP();
}

// The interval can be dynamic
// We offset the events to allow events to not run simultaneously.
// For example: Subsampling always have to ocurr before Sampling and saving.

void events_register(EVENT_IDS id, char *name,
					void (*functionCall)(void *, time_t),
					time_t intervalDefault, time_t intervalMinutes,
					time_t offset_time_secs) {
	EVENT *pEvent;
	uint8_t nextEvent;

	// Not enough space to register an event.
	if (g_sEvents.registeredEvents >= MAX_EVENTS)
		return;

	nextEvent = g_sEvents.registeredEvents;

	pEvent = &g_sEvents.events[nextEvent];
	pEvent->id = id;
	strncpy(pEvent->name, name, sizeof(pEvent->name));

//#ifdef _DEBUG
//	intervalDefault /= 2;
//	offset_time_secs /= 2;
//	intervalMinutes /= 2;
//#endif
//***changed aditya/
        if (intervalMinutes != 0)  {
                pEvent->interval_secs = MINUTES_(intervalMinutes);
        }
	else if (intervalDefault != 0)  {
		pEvent->interval_secs = intervalDefault;
        }

	pEvent->lastEventRun = 0;
	pEvent->nextEventRun = 0;
	pEvent->offset_secs = offset_time_secs;
	pEvent->callback = functionCall;
	g_sEvents.registeredEvents++;
	event_init(pEvent, 0);
}

void event_init(EVENT *pEvent, time_t currentTime) {
	pEvent->nextEventRun = currentTime + pEvent->offset_secs
			+ event_getIntervalSeconds(pEvent);
}

//this can be tricky
//we want each event to fire exactly intervalSecs from lastEventRun
//another event may prevent ours from being on time - so we have a trade off:
//1 - we could just set the next event time based when the last event actually run (previous method)
//2 - we can set the next method based on the intended time event so we could theoretically MISS
//    events and have a gap where that event did not fire, but timing will at least be consistent (currently trying this)
void event_next(EVENT *pEvent, time_t currentTime) {
	pEvent->lastEventRun = pEvent->nextEventRun;

	time_t eventInterval = (time_t)event_getIntervalSeconds(pEvent);

	time_t elapsedInterval = currentTime - pEvent->lastEventRun;
	if(elapsedInterval < 0) //not sure how we could have run the event in the future - revert to old method
		pEvent->nextEventRun = currentTime + eventInterval;
	else
		pEvent->nextEventRun = pEvent->lastEventRun								//using integer division to add intervals if the
								+ eventInterval 								//difference between the current time and the time
								+ eventInterval*(elapsedInterval/eventInterval);//that the event should have fired is larger than the interval
}

void event_next_by_id(EVENT_IDS id) {
    time_t currentTime = events_getTick();
    EVENT *pEvent = events_find(id);
	if (pEvent == NULL)
		return;

    event_next(pEvent, currentTime);
}

void events_sync_rtc() {
	// Events dependent on time
}

void events_sync() {
	EVENT *pEvent;
	int t;

	time_t currentTime = events_getTick();

	for (t = 0; t < g_sEvents.registeredEvents; t++) {
		pEvent = &g_sEvents.events[t];
		event_init(pEvent, currentTime);
	}
	events_find_next_event();
}

void events_debug() {
	/*
#ifdef _DEBUG
	if (!g_iDebug)
		return;

	time_t currentTime = events_getTick();

	EVENT *pEvent = &g_sEvents.events[g_sEvents.nextEvent];
	//if (pEvent->id == EVT_DISPLAY)
	//	return;

	int nextEventTime = pEvent->nextEventRun - currentTime;
	int test = nextEventTime % 10;

	if (nextEventTime< 0)
		nextEventTime = 0;
	if (test == 0 || nextEventTime < 10 || 1)
		lcd_printf(LINE1, "[%s %d]   ", pEvent->name, nextEventTime);
#endif
*/
}

void event_run_now(EVENT *pEvent) {

	time_t currentTime = events_getTick();

#ifndef _FISM_
	// Disable hardware interruptions before running the actions
	hardware_disable_buttons();
#endif
	// Store in the log file
//	config_setLastCommand(pEvent->id);

	// Move the interval to the next time
	event_next(pEvent, currentTime);
	pEvent->callback(pEvent, currentTime);
	events_find_next_event();
#ifndef _FISM_
	hardware_enable_buttons();
#endif
}

void event_run_now_by_id(EVENT_IDS id) {

	EVENT *pEvent = events_find(id);
	if (pEvent == NULL)
		return;

	event_run_now(pEvent);
}

void event_force_event_by_id(EVENT_IDS id, time_t offset) {

	time_t currentTime;
	EVENT *pEvent = events_find(id);
	if (pEvent == NULL)
		return;

	currentTime = events_getTick();
	pEvent->nextEventRun = currentTime + offset;
	events_find_next_event();
}

EVENT inline *events_getNextEvent() {
	return &g_sEvents.events[g_sEvents.nextEvent];
}

void events_run() {

	EVENT *pEvent = NULL;
	EVENT *pOldEvent = NULL;

//	event_run_deferred_commands();
#ifndef _FISM_
	state_check_power();
#endif
	pEvent = &g_sEvents.events[g_sEvents.nextEvent];
	while (events_getTick() >= pEvent->nextEventRun && pEvent != NULL) {
		/*
		if (g_iDebug)
			buzzer_feedback_value(50);
		*/

		// We don't want to run the same event twice in a row
		// lets return control over the system for a round of CPU before repeating the process
		if (pOldEvent == pEvent) {
			return;
		}

		pOldEvent = pEvent;
		event_run_now(pEvent);
		pEvent = &g_sEvents.events[g_sEvents.nextEvent];
	}

	//events_sanity(currentTime);
}

/*******************************************************************************************************/
/* Event functions */
/*******************************************************************************************************/

/*
void event_sms_test(void *event, time_t currentTime) {
	if (!g_pDevCfg->cfg.logs.sms_reports)
		return;
}
*/

void event_SIM_check_incoming_msgs(void *event, time_t currentTime) {

#ifndef _FISM_
      sms_process_messages();
      modem_check_network();
 //      lcd_show();
//sachin    event_force_event_by_id(EVT_CHECK_NETWORK, 0); //update signal strength (also display)
#else
      disable_diagnosis();
      dev_sms_cofig();
      enable_diagnosis();
//      gprs_config_dev();
#endif
}

/*sachin void event_pull_time(void *event, time_t currentTime) {
	modem_pull_time();
	// We update all the timers according to the new time
	events_sync_rtc();
    event_force_event_by_id(EVT_CHECK_NETWORK, 0); //update signal strength (also display)
}*/

/*void event_update_display(void *event, time_t currentTime) {
	// xxx Does system time do this?
	config_update_system_time();
	// Invalidate display
	if (lcd_show() == 1) {
		lcd_show();
	}
}*/

void event_display_off(void *event, time_t currentTime) {
	lcd_turn_off();
}

void event_save_samples(void *event, time_t currentTime) {

	UINT bytes_written = 0;
g_pSysState->time_flag = 0;
	log_sample_to_disk(&bytes_written);
	log_sample_web_format(&bytes_written);
        g_pSysState->time_flag = 1;
//	config_setLastCommand(COMMAND_MONITOR_ALARM);

	//monitor for temperature alarms
	//alarm_monitor();
        lcd_show();
//sachin	event_force_event_by_id(EVT_DISPLAY, 0);
}

void event_subsample_temperature(void *event, time_t currentTime) {
#ifndef _FISM_
  lcd_print_progress();
#endif
        disable_diagnosis();
//	temperature_subsamples(NUM_SAMPLES_CAPTURE);
        temp_check();
        append_data_in_buffer();
        enable_diagnosis();
}


/*void event_network_check(void *event, time_t currentTime) {
//	config_setLastCommand(COMMAND_EVENT_CHECK_NETWORK);

//	if( g_pSysState->simState[0].failedTransmissionsGPRS > NETWORK_MAX_TRANSMIT_FAILS)
//		system_reboot(get_string(RBT_MSG, (uint8_t)2));

	if(modem_check_network() != UART_SUCCESS){
		//failed, leave
		return;
	}
	event_force_event_by_id(EVT_DISPLAY, 0); //display now that we have updated the signal quality
}*/

typedef struct {
	uint8_t sim;
	uint8_t upload_mode;
	int8_t network_mode;
} FAILOVER_STEP;

const FAILOVER_STEP failoverSequence[] = {
		{0, MODE_GPRS, NETWORK_GPRS},
		{1, MODE_GPRS, NETWORK_GPRS},
		{0, MODE_GSM, NETWORK_GSM},
		{1, MODE_GSM, NETWORK_GSM}
};

void event_upload_samples(void *event, time_t currentTime)
{
  debug_print("\r\nUpload EVENT\r\n");
  disable_diagnosis();
   
  flags.sim_ready = 0;
  if(send_cmd_wait("AT+CPIN?\r\n", "READY\r\n",5,1))
  {
    flags.sim_ready = 1;
  }
  if((g_ConfigDevice.cfgUploadMode == 2) || (g_ConfigDevice.cfgUploadMode == 1))
  {
    if(check_internet() == 0)
    {
      flags.internet_flag = 0;
      gsm_init();
    }
  }
  upload_packet_sms();
  enable_diagnosis();
  
#ifndef _FISM_

  char message[13];
	int failoverSteps = sizeof(failoverSequence)/sizeof(failoverSequence[0]), netMode, netStatus;

	int i, prev_i, retry=0;

	for(i = 0; i < failoverSteps; i++) {


		if(	(g_pDevCfg->cfgUploadMode & failoverSequence[i].upload_mode) &&
			(!g_pDevCfg->cfgSIM_force || g_pDevCfg->cfgSIM_force - 1 == failoverSequence[i].sim)
		)
		{
                  if (failoverSequence[i].upload_mode == MODE_GPRS ){
                   prev_i = i;
                    retry++;
                  }

			if(config_getSelectedSIM() != failoverSequence[i].sim){ //if not on sim
                          modem_swap_SIM(); //swap to other
			}

			//check that we are on the right network
			if(modem_getNetworkService() != failoverSequence[i].network_mode) {
				modem_setNetworkService(failoverSequence[i].network_mode);
			} else { //if we are make sure we are still registered
				modem_getNetworkStatus(&netMode, &netStatus);
				if(netStatus != NETWORK_STATUS_REGISTERED_HOME_NETWORK &&
				   netStatus != NETWORK_STATUS_REGISTERED_ROAMING)
				{
					modem_connect_network(NETWORK_CONNECTION_ATTEMPTS);
				}
			}

			//modem_clearLastErrorsCache();
			//try batch upload with forced settings
                        modem_pull_time();
                rtc_getlocal(&g_tmCurrTime);
                if( g_tmCurrTime.tm_year <= 2015 || g_tmCurrTime.tm_year >= 2025) {
                g_pSysState->state.alarms.time_failure = 1;
                }

			if (process_batch() == 0) { //TRANS_SUCCESS
				//success
				return;
			} else if (retry < 3 && (failoverSequence[i].upload_mode == MODE_GPRS)) {
                          i = prev_i - 1;
                          lcd_printf(LINEC, "GPRS_RETRY:%d", retry);
                          delay(1000);
                    strcpy(message, "GPRS_RETRY:");
                    log_enable();
                  log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_BATCH_TX), strcat(message, itoa_pad(retry)));
                        } else if (retry == 3) {
                            retry = 0;//resetting the trials for next sim.
                        }
			//modem_logSIMErrorCodes();
		}
	}
  //sachin event_force_event_by_id(EVT_CHECK_NETWORK, 0); //update signal strength
#endif

}

//only use in alarms check routine!! keeps timing base on being scheduled right after the alarms check
void event_force_upload(time_t offset) {
	event_force_event_by_id(EVT_UPLOAD_SAMPLES, offset);
}

//Event forcing resets the interval alignment. We want these to stay aligned
//due to the interrupt driven nature of sampling routing. Use this helper function
//to service immediate sample and upload requests while retaining their predetermined offsets
void event_force_sample() 
{
#ifndef _FISM_
	event_force_event_by_id(EVT_SUBSAMPLE_TEMP, OFFSET_EVT_SUBSAMPLE_TEMP);
	event_force_event_by_id(EVT_SAVE_SAMPLE_TEMP, OFFSET_EVT_SAVE_SAMPLE_TEMP);
	event_force_event_by_id(EVT_ALARMS_CHECK, OFFSET_EVT_ALARMS_CHECK);
#else
        event_force_event_by_id(EVT_SUBSAMPLE_TEMP, OFFSET_EVT_SUBSAMPLE_TEMP);
	event_force_event_by_id(EVT_ALARMS_CHECK, OFFSET_EVT_ALARMS_CHECK);
#endif
}

//sometimes we want both of these to happen
uint8_t g_forceSampleUpload_c = 0;
void event_force_sample_and_upload() {
	g_forceSampleUpload_c++;
	event_force_sample();
	event_force_upload(OFFSET_EVT_UPLOAD_SAMPLES);
}

//global RAM variable for consistent boot sequence behavior - DO NOT PLACE IN FRAM!!
uint8_t g_fEventQueueInitialized = 0;
#define CHECK_EVENTS_INITED (g_fEventQueueInitialized)

EVENT *events_find(EVENT_IDS id) {
	int t;

        if (!CHECK_EVENTS_INITED) {
           return NULL;
        }

	for (t = 0; t < g_sEvents.registeredEvents; t++) {
		EVENT *pEvent = &g_sEvents.events[t];
		if (pEvent->id == id)
			return pEvent;
	}

	return NULL;
}

// turns on the screen, resets the timeout to turn it off
/*void event_LCD_turn_on() {
	EVENT *event = events_find(EVT_LCD_OFF);
	lcd_turn_on();
        if (event == NULL) {
          return;
        }
	event_init(event, events_getTick());
}*/

/*sachin void events_check_battery(void *event, time_t currentTime) {
	state_battery_level(batt_getlevel());
}*/

void events_health_check(void *event, time_t currentTime)
{
#ifdef _FISM_
  
   disable_diagnosis();   
   debug_print("\r\nEvent Health Check\r\n");
   temp_check();
   check_sensor_alarm();
   check_battery_alarm();
   read_alarm_packets();
   check_batt_state();
   alarm_monitor();
   if(flags.configure_http)
   {
      if(gprs_config_dev() == UART_SUCCESS)
      {
        event_force_event_by_id(EVT_UPLOAD_SAMPLES,0);
        if(device_ready_http())
        {
          flags.configure_http = 0;
          event_force_event_by_id(EVT_SUBSAMPLE_TEMP,0);
          state_alarm_reset_all();
        }
      }
   }
   state_battery_level(batt_getlevel());
   state_check_disconn_flags();
   check_for_OTA();
   
#else
 static int8_t i;

 state_process();

 if((i%PERIOD_BATTERY_CHECK) == 0) {
   state_battery_level(batt_getlevel());
 }
  i++;

  lcd_show();
#endif
  enable_diagnosis();
}

void event_reboot_system(void *event, time_t currentTime) {
#ifndef _FISM_
      log_reboot("EVENT");
      system_reboot(get_string(RBT_MSG, RBT_SCHEDULED));
#else

#ifdef _FDEBUG_
      debug_print("\n\n***Scheduled Reboot***\n\n");
      PMM_trigBOR();
#endif
//      system_reboot("REBOOT");
#endif
}

/*sachin void event_fetch_configuration(void *event, time_t currentTime) {
	config_pull_configuration(g_pDevCfg->cfgConfig_URL, 1); //no failover logic here!
//sachin    event_force_event_by_id(EVT_CHECK_NETWORK, 0); //update signal strength
}*/

//void event_reset_trans(void *event, time_t currentTime) {
//	state_reset_network_errors();
//}

// Sleeping state
uint8_t iMainSleep = 0;

// Resume execution if the device is in deep sleep mode
// Triggered by the interruption
uint8_t event_wake_up_main() {
	if (iMainSleep == 0)
		return 0;

	EVENT *pNext = events_getNextEvent();

	if (events_getTick() > pNext->nextEventRun)
		return 1;

	return 0;
}

// Main sleep, if there are not events happening it will just sleep

void event_main_sleep() {
	iMainSleep = 1;

	// Screen on, check every second
	if (getLCD_state())
		delay(MAIN_SLEEP_TIME);
	else
	// If we are disconnected, lets check every 5 seconds for the power to be back.
	if (g_pSysState->system.switches.power_connected)
		delay(MAIN_SLEEP_POWER_OUTAGE);
	else
		// Deep sleep
		delay(MAIN_LCD_OFF_SLEEP_TIME);

	iMainSleep = 0;
}

/*oid events_display_alarm(void *event, time_t currentTime) {
        uint8_t t = 0;
        uint8_t fndtemp = 0;
   //     SYSTEM_ALARMS *s = state_getAlarms();
	SENSOR_STATUS *sensor = NULL;

//	if (s->alarms.globalAlarm == STATE_OFF)
//		return;
        if (g_pSysState->state.alarms.globalAlarm == STATE_OFF)
		return;

        lcd_printl(LINEC, get_string(LCD_MSG, ALARM));

//	if (s->alarms.battery ) {
          if (g_pSysState->state.alarms.battery ) {
         	lcd_printl(LINEE, "LOW BATTERY");
                iMainSleep = 1;
                delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
                iMainSleep = 0;
        }

//sachin        if (s->alarms.poweroutage && g_pDevCfg->stBattPowerAlertParam.flags.bits.StateForcedUpload) {
           if (g_pSysState->state.alarms.poweroutage && g_pDevCfg->stBattPowerAlertParam.flags.bits.StateForcedUpload) {
         	lcd_printl(LINEE, "POWER OUT  ");
                iMainSleep = 1;
                delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
                iMainSleep = 0;
        }

// sachin       if (s->alarms.SD_card_failure) {
            if (g_pSysState->state.alarms.SD_card_failure) {
          	lcd_printl(LINEE, "SD FAILURE ");
                iMainSleep = 1;
                delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
                iMainSleep = 0;
        }
//sachin        if (s->alarms.time_failure) {
           if (g_pSysState->state.alarms.time_failure) {
          	lcd_printl(LINEE, "TIME FAILURE ");
                iMainSleep = 1;
                delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
                iMainSleep = 0;
        }

	for (t = 0; t < SYSTEM_NUM_SENSORS; t++) {
		sensor = getAlarmsSensor(t);
		if (sensor->state.lowAlarm || sensor->state.highAlarm
                    || sensor->state.alarm) {
			fndtemp = 1;
                    }
	}
        if (fndtemp) {
          	lcd_printl(LINEE, "TEMP ALARM ");
                iMainSleep = 1;
                delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
                iMainSleep = 0;
        }

}*/

void events_init() 
{
        int i;
	memset(&g_sEvents, 0, sizeof(g_sEvents));

#ifndef _FISM_
        struct event_or {
        int id;
        time_t mins;
        time_t interval;
        time_t offset;
        char id_name[4];
         void (*functionCall)(void *, time_t);
        } events [7]= {
                        {EVT_SUBSAMPLE_TEMP, MINUTES_(PERIOD_SAMPLING), g_pDevCfg->sIntervalsMins.sampling,
                          OFFSET_EVT_SUBSAMPLE_TEMP, "SUB", &event_subsample_temperature },

                        {EVT_ALARMS_CHECK,  MINUTES_(PERIOD_ALARMS_CHECK), g_pDevCfg->sIntervalsMins.alarmsCheck,
                          OFFSET_EVT_ALARMS_CHECK, "ALR", &events_health_check},

                        {EVT_UPLOAD_SAMPLES, MINUTES_(PERIOD_UPLOAD), g_pDevCfg->sIntervalsMins.upload,
                          OFFSET_EVT_UPLOAD_SAMPLES, "UPL", &event_upload_samples},

                        {EVT_SMSCHECK,  MINUTES_(PERIOD_SMS_CHECK), g_pDevCfg->sIntervalsMins.smsCheck,
                          OFFSET_EVT_SMSCHECK, "SMS", &event_SIM_check_incoming_msgs},

                        {EVT_LCD_OFF, MINUTES_(PERIOD_LCD_OFF), g_pDevCfg->sIntervalsMins.lcdOff,
                          OFFSET_EVT_LCD_OFF,  "LOF", &event_display_off},

                        {EVT_PERIODIC_REBOOT, MINUTES_(PERIOD_REBOOT), g_pDevCfg->sIntervalsMins.systemReboot,
                          OFFSET_EVT_PERIODIC_REBOOT, "RBT", &event_reboot_system}
        };

         for (i=0; i<=6; i++) {
         events_register(events[i].id, events[i].id_name, events[i].functionCall, events[i].mins,
                         events[i].interval, events[i].offset);
         }
#else
        struct event_or {
        int id;
        time_t mins;
        time_t interval;
        time_t offset;
        char id_name[4];
         void (*functionCall)(void *, time_t);
        } events []= {
          
                    {EVT_UPLOAD_SAMPLES,MINUTES_(PERIOD_UPLOAD), g_pDevCfg->sIntervalsMins.upload,
                     OFFSET_EVT_UPLOAD_SAMPLES, "UPL", &event_upload_samples},
                     
                     {EVT_SUBSAMPLE_TEMP,MINUTES_(PERIOD_SAMPLING), g_pDevCfg->sIntervalsMins.sampling,
                     OFFSET_EVT_SUBSAMPLE_TEMP, "SUB", &event_subsample_temperature},

                    {EVT_PERIODIC_REBOOT, MINUTES_(PERIOD_REBOOT), g_pDevCfg->sIntervalsMins.systemReboot,
                     OFFSET_EVT_PERIODIC_REBOOT, "RBT", &event_reboot_system},

                    {EVT_SMSCHECK,  MINUTES_(PERIOD_SMS_CHECK), g_pDevCfg->sIntervalsMins.smsCheck,
                     OFFSET_EVT_SMSCHECK, "SMS", &event_SIM_check_incoming_msgs},

                    {EVT_ALARMS_CHECK,  MINUTES_(PERIOD_ALARMS_CHECK),g_pDevCfg->sIntervalsMins.alarmsCheck,
                    OFFSET_EVT_ALARMS_CHECK, "ALR", &events_health_check}
        };

         for (i=0; i<5; i++) 
         {
              events_register(events[i].id, events[i].id_name, events[i].functionCall, events[i].mins,
                         events[i].interval, events[i].offset);
         }


#endif

        //        void (*functionCall[6])(void *, time_t) = {&event_subsample_temperature, &event_save_samples, &events_health_check,
//        &event_upload_samples, &event_SIM_check_incoming_msgs, &event_reboot_system};
//
//        char id_name[6][4] = {"SUB", "SAV", "ALR", "UPL", "SMS", "RBT"};


//        int id[6] ={EVT_SUBSAMPLE_TEMP, EVT_SAVE_SAMPLE_TEMP, EVT_ALARMS_CHECK, EVT_UPLOAD_SAMPLES, EVT_SMSCHECK, EVT_PERIODIC_REBOOT};
//
//        int mins[6] = {MINUTES_(PERIOD_SAMPLING), MINUTES_(PERIOD_SAMPLING), MINUTES_(PERIOD_ALARMS_CHECK), MINUTES_(PERIOD_UPLOAD),
//        MINUTES_(PERIOD_SMS_CHECK), MINUTES_(PERIOD_REBOOT)};
//
//        int interval[6] = {g_pDevCfg->sIntervalsMins.sampling, g_pDevCfg->sIntervalsMins.sampling, g_pDevCfg->sIntervalsMins.alarmsCheck,
//        g_pDevCfg->sIntervalsMins.upload, g_pDevCfg->sIntervalsMins.smsCheck, g_pDevCfg->sIntervalsMins.systemReboot,};
//
//        int offset[6] = { OFFSET_EVT_SUBSAMPLE_TEMP, OFFSET_EVT_SAVE_SAMPLE_TEMP, OFFSET_EVT_ALARMS_CHECK,
//        OFFSET_EVT_UPLOAD_SAMPLES, OFFSET_EVT_SMSCHECK, OFFSET_EVT_PERIODIC_REBOOT};
//
//
//        for (i=0; i<=5; i++) {
//
//	if (g_sEvents.registeredEvents >= MAX_EVENTS)
//		return;
//
//	nextEvent = g_sEvents.registeredEvents;
//
//	pEvent = &g_sEvents.events[nextEvent];
//	pEvent->id = events[i].id;
//	strncpy(pEvent->name, events[i].id_name, sizeof(pEvent->name));
//
//	if (events[i].mins != 0)
//		pEvent->interval_secs = events[i].mins;
//
//	if (events[i].mins != 0)
//		pEvent->interval_secs = events[i].interval;
//
//	pEvent->lastEventRun = 0;
//	pEvent->nextEventRun = 0;
//	pEvent->offset_secs = events[i].offset;
//	pEvent->callback = events[i].functionCall;
//	g_sEvents.registeredEvents++;
//	event_init(pEvent, 0);
//        }


//	events_register(EVT_SUBSAMPLE_TEMP, "SUB",
//					&event_subsample_temperature,
//					MINUTES_(PERIOD_SAMPLING), g_pDevCfg->sIntervalsMins.sampling,
//					OFFSET_EVT_SUBSAMPLE_TEMP);
//
//	// Offset the save N seconds from the subsample taking
//	events_register(EVT_SAVE_SAMPLE_TEMP, "SAV",
//					&event_save_samples,
//					MINUTES_(PERIOD_SAMPLING), g_pDevCfg->sIntervalsMins.sampling,
//					OFFSET_EVT_SAVE_SAMPLE_TEMP);
//
//	//we want to do state check before upload since an alarm may be forcing an upload
//	events_register(EVT_ALARMS_CHECK, "ALR",
//					&events_health_check,
//					MINUTES_(PERIOD_ALARMS_CHECK),
//					g_pDevCfg->sIntervalsMins.alarmsCheck,
//					OFFSET_EVT_ALARMS_CHECK);
//
//	events_register(EVT_UPLOAD_SAMPLES, "UPL",
//					&event_upload_samples,
//					MINUTES_(PERIOD_UPLOAD), g_pDevCfg->sIntervalsMins.upload,
//					OFFSET_EVT_UPLOAD_SAMPLES);
//
///*sachin        events_register(EVT_BATTERY_CHECK, "BAT",
//					&events_check_battery,
//					MINUTES_(PERIOD_BATTERY_CHECK),
//					g_pDevCfg->sIntervalsMins.batteryCheck,
//					OFFSET_EVT_BATTERY_CHECK);
//
//	events_register(EVT_CHECK_NETWORK, "NET",
//					&event_network_check,
//					MINUTES_(PERIOD_NETWORK_CHECK), g_pDevCfg->sIntervalsMins.networkCheck,
//					OFFSET_EVT_CHECK_NETWORK);
//*/
///*	events_register(EVT_PULLTIME, "TME",
//					&event_pull_time,
//					MINUTES_(PERIOD_PULLTIME), g_pDevCfg->sIntervalsMins.modemPullTime,
//					OFFSET_EVT_PULLTIME);
//*/
//	// Check every 30 seconds until we get the configuration message from server;
//	events_register(EVT_SMSCHECK, "SMS",
//					&event_SIM_check_incoming_msgs,
//					MINUTES_(PERIOD_SMS_CHECK), g_pDevCfg->sIntervalsMins.smsCheck,
//					OFFSET_EVT_SMSCHECK);
//
///*	events_register(EVT_DISPLAY, "DIS",
//					&event_update_display,
//					MINUTES_(PERIOD_LCD_REFRESH), NULL,
//					OFFSET_EVT_DISPLAY);
//*/
///*sachin	events_register(EVT_LCD_OFF, "LOF",
//					&event_display_off,
//					MINUTES_(PERIOD_LCD_OFF), g_pDevCfg->sIntervalsMins.lcdOff,
//					OFFSET_EVT_LCD_OFF);
//*/
///*sachin	events_register(EVT_DISPLAY_ALARM, "LAR",
//					&events_display_alarm,
//					MINUTES_(PERIOD_LCD_REFRESH*2), NULL,
//					PERIOD_LCD_REFRESH / 2);
//*/
//	events_register(EVT_PERIODIC_REBOOT, "RBT",
//					&event_reboot_system,
//					MINUTES_(PERIOD_REBOOT),
//					g_pDevCfg->sIntervalsMins.systemReboot,
//					OFFSET_EVT_PERIODIC_REBOOT);
//
// /*   events_register(EVT_CONFIG_FETCH, "CFG",
//					&event_fetch_configuration,
//					MINUTES_(PERIOD_CONFIGURATION_FETCH),
//					g_pDevCfg->sIntervalsMins.configurationFetch,
//					OFFSET_EVT_CONFIG_FETCH);
//   */
	events_sync();

    g_fEventQueueInitialized = 1;
}

