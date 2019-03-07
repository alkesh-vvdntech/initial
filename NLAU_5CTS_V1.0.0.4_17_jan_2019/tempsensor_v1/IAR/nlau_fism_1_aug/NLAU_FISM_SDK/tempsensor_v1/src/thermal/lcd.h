/*
 * lcd.h
 *
 *  Created on: May 15, 2015
 *      Author: sergioam
 */

#ifndef TEMPSENSOR_V1_LCD_H_
#define TEMPSENSOR_V1_LCD_H_
     
#include <stdint.h>

#define   SLAVE_ADDR_DISPLAY	0x3e

//LCD lines
#define LINE1					1
#define LINE2					2
// Line 1 and clear screen
#define LINEC					0

// Line 2 wait for user to read
#define LINEH					3

// Line 2 wait for user to read ERROR
#define LINEE					4

#define LCD_MSG "LCD"
#define VERBOSE_BOOTING 0
#define VERBOSE_DISABLED -1

// Time to display an error to a human
#ifdef _DEBUG
#define HUMAN_DISPLAY_ERROR_DELAY 3000
#define HUMAN_DISPLAY_INFO_DELAY 500
#define HUMAN_DISPLAY_LONG_INFO_DELAY 1000
#else
#define HUMAN_DISPLAY_ERROR_DELAY 10000
#define HUMAN_DISPLAY_INFO_DELAY 3000
#define HUMAN_DISPLAY_LONG_INFO_DELAY 6000
#endif

#define SDCARD_FAILED "SDcard Failure"
typedef enum{
SERVICE_MODE=0,
LCD_INIT_1,
LCD_INIT_2,
BUZZER_TEST,
PRESS_STOP_ALARM,
SDCARD_FAILURE,
FAT_NOT_INITIALIZED,
NO_INI_FILE,
SDCARD_SUCCESS,
FINISHED_BOOT,
MODEM_FAILED,  //10
SIMS_FAILED,
PARSING_CONFIG,
SAVING_CONFIG,
SMS_GATEWAY,
GATEWAY_IP,
CONFIG_URL,
UPLOAD_URL,
CONFIG,
SIM_ERROR,
POWER_CABLE, //20
DISCONNECTED,
POWER_RESUMED,
HTTP_ERR,
ALARM,
PRESS_AGAIN,
TO_SWAP_SIM,
TO_RE_CALIBRATE,
LOW_BATTERY,
HIBERNATING,
RECOVERING, //30
PING,
FAILED,
SUCCESS,
HTTP_SERVER,
EMPTY_RESPONSE,
TXERR,
TIMEOUT,
REBOOTING_NOW,
FETCHING_SMS,
NO_NEW_MSG, //40
MSG_PROCESSING,
HEARTBEAT_NOT_SENT,
HEARTBEAT_SENT,
SMS_CONFIRM,
SERVICE_ERROR,
MODEM_ERROR,
POWER_ON,
MCC_RETRY,
MCC_DISCOVER,
NETWORK_BUSY, //50
WRONG_DATE,
LAST_DATE,
SIM_NOT_INSERTED,
TRANSMITTING,
TRANSMISSION,
COMPLETED,
SD_WRITE_FAILED,
SAVING_SAMPLE,
BUZZER_OFF,
BUZZER_ONN, //60
BATTERY_STATUS,
UNKNOWN,
FULL,
OK,
LOW,
UARTFAILED,
REBOOTING_MODEM
}LCD_MESSAGE;


extern int g_iLCDVerbose;
extern char g_bLCD_state;

#ifdef _CT5_BOOTLOADER_
//
//  External variables from linker file. 
//  Note that this file gets constants from linker file
//
extern const uint16_t _LCD_VECTOR;            /*! I2C Vector Start Address */

/* bootloader held I2C vector table */
extern const uint32_t LCD_Vectors[];
#endif

// Encapsulation, this should go into a class
void lcd_setVerboseMode(int v);
int lcd_getVerboseMode();
void lcd_disable_verbose();
void lcd_enable_verbose();

void lcd_setupIO();
void lcd_reset();
void lcd_blenable();
void lcd_init();
void lcd_display_config();
void lcd_display_config_sensor(int id);
void lcd_on();
void lcd_off();
void lcd_clear();
void lcd_setaddr(int8_t addr);
uint8_t lcd_show();
void lcd_turn_on();
void lcd_turn_off();
void lcd_print(char* pcData);
void lcd_printf(int line, const char *_format, ...);
void lcd_printl(int8_t iLine, const char* pcData);
void lcd_print_boot(const char* pcData, int line);
void lcd_bldisable();
void lcd_progress_wait(uint16_t delayTime);
void lcd_print_progress();
char* get_string(char *section, uint8_t Key);
char getLCD_state(void);
#endif /* TEMPSENSOR_V1_LCD_H_ */
