/*
 * globals.c
 *
 *  Created on: May 15, 2015
 *      Author: sergioam
 *
 *  Global variables refactorization
 */

#include "stdint.h"
#include "i2c.h"
#include "config.h"
#include "lcd.h"
#include "time.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
#include "globals.h"

/*****************************************************************************************************/
/* Legacy names */
/*****************************************************************************************************/
const char SensorName[SYSTEM_NUM_SENSORS+1][NAME_LEN] = { "C", "D"};
//const char SensorName[SYSTEM_NUM_SENSORS + 1][NAME_LEN] = { "A", "B", "C","D","E" };
//const char SensorName[SYSTEM_NUM_SENSORS][NAME_LEN] = { "A"};
//char Sensor_Name[SYSTEM_NUM_SENSORS] = {'A','B'};

/*****************************************************************************************************/
/* Variables to revisit */

volatile uint8_t g_iDebug = 0;

// Current display view
volatile uint8_t g_iDisplayId = 0;

struct tm g_tmCurrTime;



