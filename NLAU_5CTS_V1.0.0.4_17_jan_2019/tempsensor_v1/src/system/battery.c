/*
 * battery.c
 *
 *  Created on: Feb 11, 2015
 *      Author: rajeevnx
 */

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
#include <stdio.h>
#include "lcd.h"
#define  TRANS_DELAY 		10

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
   uint8_t flag = 0;

   //unsealed
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   //delay(20);
   //set cfg update
   data_cmd[0] = 0x13;
   data_cmd[1] = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   //delay(20);

   while(!(flag & 0x10) && --retries)
   {
     lcd_clear();
     i2c_read(SLAVE_ADDR_BATTERY, BATT_FLAGS, 1, (uint8_t *) &flag);
     //delay(100);
   }

   //enable block data
   block_cmd = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_BLOCK_DATA_CONTROL, 1, &block_cmd);
   //delay(20);
   block_cmd = 0x52;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_DATA_BLOCK_CLASS, 1, &block_cmd);
   //delay(20);
   block_cmd = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_DATA_BLOCK, 1, &block_cmd);
   //delay(20);

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
   //delay(20);
   //write new design engine
   new_de[0] = (DESIGN_ENERGY >> 8) & 0xFF;
   new_de[1] = DESIGN_ENERGY & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_DESIGN_ENERGY, 2, new_de);
   //delay(20);
   //write new terminal voltage
   new_tv[0] = (TERMINAL_VOLTAGE >> 8) & 0xFF;
   new_tv[1] = TERMINAL_VOLTAGE & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_TERM_VOLT, 2, new_tv);
   //delay(20);
   //write new taper rate
   new_tr[0] = (TAPER_RATE >> 8) & 0xFF;
   new_tr[1] = TAPER_RATE & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_TAPER_RATE, 2, new_tr);
   //delay(20);
   //write new taper volt
   new_tpv[0] = (TAPER_VOLT >> 8) & 0xFF;
   new_tpv[1] = TAPER_VOLT & 0xFF;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_TAPER_VOLT, 2, new_tpv);
   //delay(20);

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
   //delay(20);
   
   //check flag
   while(flag && --retries)
   {
     lcd_clear();
     i2c_read(SLAVE_ADDR_BATTERY, BATT_FLAGS, 1, &flag);
     flag = flag & 0x10;
     //delay(100);
   }
   
   //sealed again
   data_cmd[0] = 0x20;
   data_cmd[1] = 0x00;
   lcd_clear();
   i2c_write(SLAVE_ADDR_BATTERY, BATT_CONTROL, 2, data_cmd);
   //delay(20);
}

uint8_t batt_getlevel()
{
  //soc
  uint8_t soc = 0;
  lcd_clear();
  i2c_read(SLAVE_ADDR_BATTERY, BATT_STATE_OF_CHARGE, 1, &soc);
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

    return soc;
}

void check_batt_state()
{
  uint8_t batt_thrsd_lvl = 5;
#ifndef _CT5_BOOTLOADER_
  //watchdog_disable();
#endif
//  P2OUT &= ~BIT7;
//  P4OUT &= ~BIT7;
#if 0 //aditya
  while(batt_getlevel() < batt_thrsd_lvl)
  {
#ifndef _CT5_BOOTLOADER_
     flags.low_battery = 0;
#endif
    debug_print("Hibernation Active\n\n\r");
    batt_thrsd_lvl = 10;
    //delay(1000);
    __delay_cycles(1000);
  }
#endif

#ifndef _CT5_BOOTLOADER_
  //START_WDT_14_MIN;
#endif
  debug_print("Battery sufficient\n\n\r");
}

// aditya
//void power_conn_status()
//{
//  //Enable ADC interrupt
//  ADC12IER0 |= ADC12IE7 | ADC12IE15;
//}
