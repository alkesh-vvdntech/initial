/*
 * sms.h
 *
 *  Created on: Feb 25, 2015
 *      Author: rajeevnx
 */

#ifndef TEMPSENSOR_V1_SMS_H_
#define TEMPSENSOR_V1_SMS_H_

#include "stdint.h"

#define TRUE 1
#define FALSE 0
#define SMS_PASSWORD  "1234"

extern char *config_commands[];
extern char user_num[15];
void ota_sms(char *buffer);
unsigned char send_sms(char *, char *);
extern void sms_call_init(void);
void delete_sms(void);
void fill_config_buffer(char *, char* );
uint8_t check_sms_command(char *);
uint8_t validate_number(char *);

void sms_send_data_request(char *number);
int8_t sms_process_msg(char* pSMSmsg);
//*****************************************************************************
//! \brief send sms msg
//! \param pointer to sms text contents
//! \return UART_SUCCESS or UART_ERROR
//*****************************************************************************
uint8_t sms_send_message(char* pData);
uint8_t sms_send_message_number(char *szPhoneNumber, char* pData);

#define MAX_SMS_SIZE 160

// + MODEM EXTRA DATA FOR SENDING + END
#define MAX_SMS_SIZE_FULL MAX_SMS_SIZE + 4

#define SMS_MSG "SMS"
typedef enum{
HMN_RDBL_RESP_CONFIG=0,
HMN_RDBL_RESP_EVENTS,
HMN_RDBL_RESP_RESET,
HMN_RDBL_RESP_NAK,
HMN_RDBL_LOW_BATT,
HMN_RDBL_PWR_OUT,
HMN_RDBL_TAKE_ACTION
}SMS_MESSAGE;

//*****************************************************************************
//! \brief delete the sms msg(s) read and sent successfully
//! \param none
//! \return none
//*****************************************************************************
void delreadmsg();
unsigned char parse_sms_parameters();
void delallmsg();
void delallmsgs();
void delmsg(int8_t iMsgIdx);

void sms_send_dataready();
//void sms_send_heart_beat();
int8_t sms_process_messages();

#endif /* TEMPSENSOR_V1_SMS_H_ */
