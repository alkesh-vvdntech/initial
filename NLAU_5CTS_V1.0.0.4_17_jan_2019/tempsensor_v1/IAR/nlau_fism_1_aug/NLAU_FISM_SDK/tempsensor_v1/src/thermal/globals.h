/*
 * globals.h
 *
 *  Created on: May 15, 2015
 *      Author: sergioam
 */

#ifndef TEMPSENSOR_V1_GLOBALS_H_
#define TEMPSENSOR_V1_GLOBALS_H_


extern volatile uint8_t g_iDebug;
extern volatile uint8_t g_iDisplayId;

extern CONFIG_DEVICE *g_pDevCfg;
extern CONFIG_CALIBRATION *g_pCalibrationCfg;
extern CONFIG_SYSTEM *g_pSysCfg; // System configuration
extern SYSTEM_STATE *g_pSysState;
extern const char SensorName[SYSTEM_NUM_SENSORS + 1][NAME_LEN];
extern struct tm g_tmCurrTime;
extern struct tm g_lastSampleTime;
#endif /* TEMPSENSOR_V1_GLOBALS_H_ */
