/*
 * alarm.h
 *
 *  Created on: Mar 19, 2015
 *      Author: rajeevnx
 */

#ifndef TEMPSENSOR_V1_ALARM_H_
#define TEMPSENSOR_V1_ALARM_H_

#ifdef __cplusplus
extern "C" {
#endif

//dev alarm API
typedef enum {
	DEV_ALARM_CONN = 1,
	DEV_ALARM_SENSOR = 2,
	DEV_ALARM_BATTERY = 4,
	DEV_ALARM_FW_ERR = 8,
} DEV_ALARM_TYPE;


void alarm_low_memory();
/*void alarm_sms_battery_level();
void alarm_sms_power_outage(time_t elapsed);
void alarm_SD_card_failure();*/
SENSOR_STATUS *getAlarmsSensor(int id);

void alarm_sms(uint8_t Id, time_t elapsed);

//*****************************************************************************
//! \brief Check for threshold, trigger buzzer and SMS on alarm condition.
//! \param none
//! \return None
//*****************************************************************************
void alarm_monitor();

//*****************************************************************************
//! \brief Check for valid thresholds and set default thresholds in case of
//! invalid values.
//! \param none
//! \return None
//*****************************************************************************
void validatealarmthreshold();

#ifdef __cplusplus
}
#endif

#endif /* TEMPSENSOR_V1_ALARM_H_ */
