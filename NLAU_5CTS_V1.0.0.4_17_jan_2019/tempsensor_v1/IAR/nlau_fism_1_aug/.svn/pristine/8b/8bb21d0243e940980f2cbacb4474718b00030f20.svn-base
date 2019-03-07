/*
 * state_machine.h
 *
 *  Created on: Jun 12, 2015
 *      Author: sergioam
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include "config.h"

extern SYSTEM_STATE *g_pSysState;

#define STATE_CONNECTION 1
#define STATE_OFF 0
#define STATE_ON 1

void state_clear_alarm_state();

void state_setNetworkStatus(const char *status);
void state_SD_card_problem(FRESULT fr, const char *szError);
void state_SD_card_OK();
void state_sim_failure(SIM_CARD_CONFIG *sim);
//void state_setSMS_notSupported();
uint8_t state_getSignalPercentage();

SYSTEM_ALARMS *state_getAlarms();
SYSTEM_SWITCHES *state_getSwitches();
int8_t state_isSignalInRange();
int8_t state_isNetworkRegistered();
void state_setSignalLevel(uint8_t iSignal);
uint8_t state_getSignalLevel();
char *state_getNetworkState();

void state_alarm_turnon_buzzer();
void state_alarm_on(char *alarm_msg) ;
void state_alarm_reset_sensors();
void state_alarm_reset_all();
void state_init();
uint8_t state_isGPRS();
uint8_t state_isGSM();
void state_power_on();
void state_power_out();
uint8_t state_isBuzzerOn();
void state_alarm_disable_buzzer_override();
void state_alarm_enable_buzzer_override();

//void state_SMS_lastMessageACK(SIM_CARD_CONFIG *sim, int8_t msgNumber);
void state_reset_network_errors();
void state_network_status(int net_mode, int net_status);
void state_network_fail(uint8_t sim, uint16_t error);
//void state_network_success(uint8_t sim);

void state_failed_sdcard(uint16_t error);

//void state_failed_gprs(uint8_t sim);
//void state_failed_gsm(uint8_t sim);

void state_transmission_failed_gprs(uint8_t sim);
//void state_transmission_failed_gsm(uint8_t sim);

void state_process();
void state_sensor_temperature(uint8_t sensor, float temp);
uint8_t state_get_batt_status(void);
void state_battery_level(uint8_t battery_level);
char *state_get_batt_string(void);
void state_check_power();

#endif /* STATE_MACHINE_H_ */
