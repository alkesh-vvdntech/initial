/*
 * battery.c
 *
 *  Created on: Feb 11, 2015
 *      Author: rajeevnx
 */

//#include "thermalcanyon.h"
#include "config.h"
#include "battery.h"
#include "i2c.h"
#include "timer.h"

#ifndef _CT5_BOOTLOADER_
  
#include "modem_uart.h"
#include "watchdog.h"
#endif

#include "main_system.h"
#include "spi_flash.h"
//#include "state_machine.h"
#include <stdio.h>

//extern SYSTEM_STATE *g_pSysState;

#ifndef _FISM_
#include "lcd.h"
#define  TRANS_DELAY 		10

int16_t g_iFullRecharge = 0;

/*uint8_t batt_check_level() {
	uint8_t iBatteryLevel = 0;
	// Battery checks
//	lcd_printl(LINEC, get_string(LCD_MSG, BATTERY_STATUS));

#ifndef BATTERY_DISABLED
	iBatteryLevel = batt_getlevel();
#endif

	if (iBatteryLevel == 0)
		lcd_printl(LINEE, get_string(LCD_MSG, FAILED));
	else if (iBatteryLevel > 100)
		lcd_printl(LINEE, get_string(LCD_MSG, UNKNOWN));
	else if (iBatteryLevel > 99)
		lcd_printl(LINE2, get_string(LCD_MSG, FULL));
	else if (iBatteryLevel > 15)
		lcd_printl(LINE2, get_string(LCD_MSG, OK));
	else if (iBatteryLevel)
		lcd_printl(LINEE, get_string(LCD_MSG, LOW));

	return iBatteryLevel;
}*/

void batt_init() {
   uint8_t Cflag = 0;
   uint8_t retries = 10;
   uint8_t block_cmd = 0;
   uint8_t old_dc[2] = {0,0};
   uint8_t old_de[2] = {0,0};
   uint8_t old_tv[2] = {0,0};
   uint8_t old_tr[2] = {0,0};
   uint8_t old_tpv[2] = {0,0};
   uint8_t old_crc = 0;
   uint8_t tmp_chksum = 0;
   uint8_t new_chksum = 0;
   uint8_t new_tpv[2] = {0,0};
   uint8_t new_dc[2] = {0,0};
   uint8_t new_de[2] = {0,0};
   uint8_t new_tv[2] = {0,0};
   uint8_t new_tr[2] = {0,0};
   uint8_t data_cmd[2] = {0x00,0x80};

   //unsealed
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   delay(20);
   //set cfg update
   data_cmd[0] = 0x13;
   data_cmd[1] = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   delay(20);
   
    while(!(flag & 0x10) && --retries)
    {
      //i2c_read(SLAVE_ADDR_BATTERY, BATT_FLAGS, 2, &flags);
      lcd_clear();
      i2c_read(SLAVE_ADDR_BATTERY, BATT_FLAGS, 1, (uint8_t *) &flag);
      delay(100);
    }

    //enable block data
   block_cmd = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_BLOCK_DATA_CONTROL, 1, &block_cmd);
   delay(20);
   block_cmd = 0x52;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_DATA_BLOCK_CLASS, 1, &block_cmd);
   delay(20);
   block_cmd = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_DATA_BLOCK, 1, &block_cmd);
   delay(20);

   //old_checksum
   lcd_clear();
   i2c_read(SLAVE_ADDR_BATTERY, BATT_BLOCK_DATA_CHECKSUM, 1, &old_crc);
   //old design capacity
   lcd_clear();
   i2c_read(SLAVE_ADDR_BATTERY, BATT_DESIGN_CAPACITY, 2, old_dc);
   //old design engine
   lcd_clear();
   i2c_read(SLAVE_ADDR_BATTERY, BATT_DESIGN_ENERGY, 2, old_de);
   //old terminal voltage
   lcd_clear();
   i2c_read(SLAVE_ADDR_BATTERY, BATT_TERM_VOLT, 2, old_tv);
   //old taper rate
   lcd_clear();
   i2c_read(SLAVE_ADDR_BATTERY, BATT_TAPER_RATE, 2, old_tr);
   //old taper volt
   lcd_clear();
   i2c_read(SLAVE_ADDR_BATTERY, BATT_TAPER_VOLT, 2, old_tpv);       

   //write new design capacity
   new_dc[0] = (BATTERY_CAPACITY >> 8) & 0xFF;
   new_dc[1] = BATTERY_CAPACITY & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_DESIGN_CAPACITY, 2, new_dc);
   delay(20);
   //write new design engine
   new_de[0] = (DESIGN_ENERGY >> 8) & 0xFF;
   new_de[1] = DESIGN_ENERGY & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_DESIGN_ENERGY, 2, new_de);
   delay(20);
   //write new terminal voltage
   new_tv[0] = (TERMINAL_VOLTAGE >> 8) & 0xFF;
   new_tv[1] = TERMINAL_VOLTAGE & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_TERM_VOLT, 2, new_tv);
   delay(20);
   //write new taper rate
   new_tr[0] = (TAPER_RATE >> 8) & 0xFF;
   new_tr[1] = TAPER_RATE & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_TAPER_RATE, 2, new_tr);
   delay(20);
   //write new taper volt
   new_tpv[0] = (TAPER_VOLT >> 8) & 0xFF;
   new_tpv[1] = TAPER_VOLT & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_TAPER_VOLT, 2, new_tpv);
   delay(20);

   tmp_chksum = ((255 - old_crc - old_dc[0] - old_dc[1] - old_de[0] - \
                old_de[1] - old_tv[0] - old_tv[1] - old_tpv[0] - \
                old_tpv[1] - old_tr[0] - old_tr[1]) % 256); 
   //new checksum
   new_chksum = 255 - ((tmp_chksum + new_dc[0] + new_dc[1] + new_de[0] + \
                new_de[1] + new_tv[0] + new_tv[1] + new_tpv[0] + \
                new_tpv[1] + new_tr[0] + new_tr[1]) % 256);
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_BLOCK_DATA_CHECKSUM, 1, &new_chksum);

   data_cmd[0] = 0x42;
   data_cmd[1] = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   delay(20);
   //check flag
   while(flag && --retries)
   {
     lcd_clear();
    i2c_read(SLAVE_ADDR_BATTERY, BATT_FLAGS, 1, &flag);
    flag = flag & 0x10;
    delay(100);
   }
   //sealed again
   data_cmd[0] = 0x20;
   data_cmd[1] = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   delay(20);
}


//uint8_t batt_getlevel()
//{
//	static uint32_t lastTick = UINT32_MAX;
//	uint8_t level = 0;
//	double adjustedlevel = 0.0;
//        
//	// Read battery levels only in minutes
//	if (g_pSysState->battery_level!=0 && lastTick==rtc_get_minute_tick())
//		return g_pSysState->battery_level;
//
//	lastTick = rtc_get_minute_tick();        
//
//        i2c_read(SLAVE_ADDR_BATTERY, BATT_STATE_OF_CHARGE, 1, (uint8_t *) &level);
//        
////sachin	if(level > 100)
////		return g_pSysState->battery_level;
//        
//	adjustedlevel = level*1.15;
//	if(adjustedlevel > 100.0)
//		level = 100;
//	else
//		level = (uint8_t)adjustedlevel;
//        
////        if(level > 100)
////            level = g_pSysState->battery_level;
//
//	g_pSysState->battery_level = level;
//	return level;
//}

#else

uint8_t batt_getlevel()
{
    //soc
    uint8_t soc = 0;    
    
#ifndef _FISM_
    lcd_clear();
    i2c_read(SLAVE_ADDR_BATTERY, BATT_STATE_OF_CHARGE, 1, &soc);
#else
    i2c_read(SLAVE_ADDR_BATTERY, BATT_STATE_OF_CHARGE, 1, &soc);
#endif
    if(soc > 100 || soc < 0 ) 
    {
        soc=100;
    }
#ifndef _CT5_BOOTLOADER_
    if(soc<=20)
      flags.low_battery = 1;
    else
      flags.low_battery = 0;
#endif
    batt_current();
    
    char local_buff[6];
    memset(local_buff,'\0',sizeof(local_buff));
    debug_print("\r\nbatt");
    itoa(soc,local_buff,10);
    debug_print(local_buff);
    debug_print("\r\n");
    
    return soc;
}

void check_batt_state()
{
  uint8_t batt_thrsd_lvl = 5;

#ifndef _CT5_BOOTLOADER_
  watchdog_disable();
#endif
  P2OUT &= ~BIT7;
  P4OUT &= ~BIT7;
  while((batt_current() < 0) && (batt_getlevel() < batt_thrsd_lvl)) 
  {
#ifndef _CT5_BOOTLOADER_
      flags.low_battery = 0;
#endif
    debug_print("Hibernation Active\n\n\r");
    batt_current();
    batt_thrsd_lvl = 10;
    delay(1000);
  }
#ifndef _CT5_BOOTLOADER_
  START_WDT_14_MIN;
#endif
  debug_print("Battery sufficient\n\n\r");  
}

void power_conn_status()
{
  
    //Enable ADC interrupt
    ADC12IER0 |= ADC12IE7 | ADC12IE15;
}

void charger_status()
{
#if 1           ////////////// rachit comment 
    if(P7IN & BIT6)  {
#ifdef _FDEBUG_
        debug_print("\r\nDischarging\r\n");
#endif
    }  else  {
#ifdef _FDEBUG_
        debug_print("\r\nCharging\r\n");
#endif
    }
#endif
}

uint16_t batt_voltage()
{
#if 1          ///////// rachit comment 
    uint8_t bvolt[3] = {0,0,'\0'};
    unsigned int  voltage = 0;
    i2c_read(SLAVE_ADDR_BATTERY, BATT_VOLATGE, 2, bvolt);

    voltage = (bvolt[1] << 8) | (bvolt[0] & 0xff);
#ifndef _FDEBUG_
    debug_print("\n\rvoltage = ");
    debug_print(itoa_nopadding(voltage)); 
    debug_print("\n\r");
#endif
    return voltage;
#endif
}


int batt_current()
{
    uint8_t bcurr[2] = {0,0};
    signed int curr = 0;
    i2c_read(SLAVE_ADDR_BATTERY, BATT_CURRENT, 2, bcurr);

    curr = (bcurr[1] << 8) | (bcurr[0] & 0xff);
    if(curr < 0)  
    {
#ifndef _CT5_BOOTLOADER_
       flags.batt_connected = 0;
#endif
       debug_print("power not connected\r\n");        
    } 
    else 
    {
#ifndef _CT5_BOOTLOADER_
      flags.batt_connected = 1;
#endif
      debug_print("power connected\r\n");
    }
    return curr;
}

#endif
