/*
 * main_system.h
 *
 *  Created on: Jun 2, 2015
 *      Author: sergioam
 */

#ifndef MAIN_SYSTEM_H_
#define MAIN_SYSTEM_H_

#include "stdint.h"
#define RBT_MSG "RBT"

typedef enum {
	RBT_SCHEDULED,
	RBT_NET_COMMAND,
	RBT_GPRS_FAILS,
	RBT_MAIN_DEATH,
	RBT_RECOVERY
} REBOOT_MESSAGE;

#define TRUE 1
#define FALSE 0

#define GREEN_LED_OFF           P2OUT &= ~BIT7;
#define GREEN_LED_ON            P2OUT |= BIT7;
#define GREEN_LED_TOGGLE        P2OUT ^= BIT7;

#define RED_LED_ON              P4OUT |=  BIT7;
#define RED_LED_OFF             P4OUT &=  ~BIT7;
#define RED_LED_TOGGLE          P4OUT ^=  BIT7;

#define GSM_STATUS_ACTIVE       P6IN & BIT6
#define POWER_CONNECTED         P7IN & BIT6

#define GSM_DTR                     P3IN & BIT2
#pragma pack(1)
typedef struct
{
  uint8_t  check_diagnosis :1;
  uint8_t  check_ota : 1;
  uint8_t  internet_flag :1;
  uint8_t  check_internet : 1;
  uint8_t  sim_ready : 1;
  uint8_t  booted_device : 1;
  uint8_t  at_cmd : 1;
  uint8_t  factory_reset : 1;
  uint8_t  batt_connected : 1;
  uint8_t  sensors_altered : 1;
  uint8_t  device_init : 1;
  uint8_t  fridge_uart_start : 1;
  uint8_t  fridge_uart_complete : 1;
  uint8_t  power_saving :1;
  uint8_t  time_fetched : 1;
  uint8_t  sms_config : 1;
  uint8_t  param_updated : 1;
  uint8_t  low_battery : 1;
  uint8_t  configure_http : 1;
  uint8_t  spi_flash_active : 1;
  uint8_t  ng_time_zone : 1;
}s_flags;
extern s_flags flags;

extern int g_iRunning;
char system_isRunning();
void system_reboot(const char *message);


#endif /* MAIN_SYST EM_H_ */
