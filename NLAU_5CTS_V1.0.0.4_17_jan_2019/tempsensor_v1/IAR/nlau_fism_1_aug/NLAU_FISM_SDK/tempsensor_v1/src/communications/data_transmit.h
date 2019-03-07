/*
 * data_transmit.h
 *
 *  Created on: Jun 1, 2015
 *      Author: sergioam
 */

#ifndef DATA_TRANSMIT_H_
#define DATA_TRANSMIT_H_

typedef enum {
	API_PACKET_DEV_READY,
	API_PACKET_ALARM_CONN,
	API_PACKET_ALARM_SENS,
	API_PACKET_ALARM_BATT,
	API_PACKET_ALARM_FWERR
} API_PACKET_TYPE;

typedef enum {
	SD_FAILURE = 7,
	TIME_FAILURE,
          POWER_FAILURE,
          BATT_FAILURE,
          FWU_FAILURE
} ALARM_TYPE;

int8_t data_send_api_packet(API_PACKET_TYPE apiType);
//sachin int8_t data_send_sd_alarm();
int8_t data_send_alarm();
int8_t process_batch();
#endif /* DATA_TRANSMIT_H_ */
