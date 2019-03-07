#include "thermalcanyon.h"
#include "data_transmit.h"
#include "hardware_buttons.h"
#include "main_system.h"
#include "fatdata.h"
#include "main_system.h"
#include "alarms.h"
#include "state_machine.h"
#include "wdt_a.h"
#include "watchdog.h"

void thermal_low_battery_message() {
	lcd_turn_on();
	lcd_printl(LINEC, get_string(LCD_MSG, LOW_BATTERY));
	lcd_printl(LINE2, get_string(LCD_MSG, HIBERNATING));
	delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
	lcd_turn_off();
	delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
}

void modem_halt() {
  uart_tx("#SYSHALT");
}

void thermal_low_battery_hibernate() {
//start_again:
        thermal_low_battery_message();

//	//kill the modem
//	P4OUT |= BIT0;
//
//	// leave running
//	// watchdog_disable();
//
//	//stop sleep wake
//	SYSTEM_ALARMS *s = &g_pSysState->state; //pointer to alarm states
//	s->alarms.hibernate = 1;
//
//	// is power plugged in yet?
//	while (!g_pSysState->system.switches.power_connected) {
//
//                watchdog_timer_touch();
//
//		//sleep device
//		event_main_sleep();
//
//		//check power state while sleeping
//		state_check_power();
//                
//	}
//
//	//reset hibernate flag
//	s->alarms.hibernate = 0;
//
//	//recover now
//        lcd_turn_on();
//        
//        while(batt_getlevel() < 10) {
//          lcd_printl(LINEC,get_string(LCD_MSG, RECOVERING));
//     //   lcd_printf(LINE2,"%s%% Charging",itoa_pad(batt_getlevel()));
//          state_check_power();
//          if(!g_pSysState->system.switches.power_connected) {
//            goto start_again;
//          }
//          delay(60000);
//        }

modem_halt();  
lcd_turn_on();
lcd_bldisable();

        while(batt_getlevel() < 10) {
  lcd_bldisable();
  if(!(P4IN & BIT4)) {
  lcd_printf(LINEC,"Charging % of 10%%", batt_getlevel());
  } else { 
    lcd_printl(LINEC,"BattLow,Hibrnt");
  }
  rtc_dead_mans_switch();
  watchdog_timer_touch();
  delay(5000);
}        
	//soft reboot without state preservation
    log_enable(); //just in case we hung during an upload
	log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_REBOOT), get_string(RBT_MSG, RBT_RECOVERY));
	PMM_trigBOR();
	while (1);	//code should not come here

}

