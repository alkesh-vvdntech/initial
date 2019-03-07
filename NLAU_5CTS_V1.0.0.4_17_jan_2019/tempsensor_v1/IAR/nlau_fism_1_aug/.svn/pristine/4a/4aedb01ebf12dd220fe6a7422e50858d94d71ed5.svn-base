/*
 * whatchdog.h
 *
 *  Created on: Jun 17, 2015
 *      Author: sergioam
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_


#define STOP_WDT                        WDTCTL = WDTPW | WDTHOLD

/*Periodically clear an active watchdog*/
   
/*Clearing the watchdog timer by writing a '1' to the WDCNTCL bit in the WDTCTL register and.
when we write to this register, we must write all 16 bits including the password for the register which is
there in MACRO START_WDT and timer clock source selected for watchdog is VLO_CLK
*/
#define START_WDT_14_MIN	        WDTCTL = WDTPW +  WDTCNTCL+ WDTSSEL_2 + WDTIS_2
 

void watchdog_init();
void watchdog_disable();
void watchdog_timer_touch();

#endif /* WATCHDOG_H_ */
