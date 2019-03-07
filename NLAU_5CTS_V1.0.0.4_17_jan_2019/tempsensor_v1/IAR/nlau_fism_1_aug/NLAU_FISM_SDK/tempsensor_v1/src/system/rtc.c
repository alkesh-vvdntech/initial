/*
 * rtc.c
 *
 *  Created on: Feb 5, 2015
 *      Author: rajeevnx
 */

#include "rtc.h"
#include "driverlib.h"
#include "stdlib.h"
#include "events.h"
#include "main_system.h"
#include "string.h"
#include "time.h"
#include "config.h"
#include "globals.h"
#include "debug.h"

#define UTC_1_YEAR	(time_t)31556926
#define UTC_70_YEARS	UTC_1_YEAR * 70

extern uint32_t epoch_seconds;   

Calendar g_rtcCalendarTime;
time_t g_lTimeZoneOffset = 0;

volatile uint32_t iMinuteTick = 0;
volatile time_t iSecondTick = 0;
volatile time_t iDeadCountdown = MINUTES_BEFORE_REBOOT;

const int8_t daysinMon[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//local functions
void converttoUTC(struct tm* pTime);
void multiply16(int16_t op1, int16_t op2, uint32_t* pResult);
void multiply32(int16_t op1, int32_t op2, uint32_t* pResult);

time_t rtc_get_second_tick() 
{
  return iSecondTick;
}

// Returns tick in minute granularity
uint32_t rtc_get_minute_tick() {
	return iMinuteTick;
}

extern struct tm g_tmCurrTime;

void rtc_setTimeZoneOffset(time_t tz) {
	g_lTimeZoneOffset = tz;
#if defined(__TI_COMPILER_VERSION__)
	_tz.timezone = tz;
#endif
}

void rtc_dead_mans_switch() {
	iDeadCountdown = MINUTES_BEFORE_REBOOT;
}

void rtc_update_time(void) {
	rtc_dead_mans_switch();
	rtc_getlocal(&g_tmCurrTime);
}

void rtc_init(struct tm* pTime) {
	if (!pTime)
		return;

	g_rtcCalendarTime.Seconds = pTime->tm_sec;
	g_rtcCalendarTime.Minutes = pTime->tm_min;
	g_rtcCalendarTime.Hours = pTime->tm_hour;
	g_rtcCalendarTime.DayOfWeek = pTime->tm_wday;
	g_rtcCalendarTime.DayOfMonth = pTime->tm_mday;
	g_rtcCalendarTime.Month = pTime->tm_mon;
	g_rtcCalendarTime.Year = pTime->tm_year;

	//Initialize Calendar Mode of RTC
	/*
	 * Base Address of the RTC_B
	 * Pass in current time, intialized above
	 * Use BCD as Calendar Register Format
	 */
#ifndef _FISM_
	RTC_B_initCalendar(RTC_B_BASE, &g_rtcCalendarTime,
	RTC_B_FORMAT_BINARY);

	//Specify an interrupt to assert every minute
	RTC_B_setCalendarEvent(RTC_B_BASE,
	RTC_B_CALENDAREVENT_MINUTECHANGE);

	//Enable interrupt for RTC Ready Status, which asserts when the RTC
	//Calendar registers are ready to read.
	//Also, enable interrupts for the Calendar alarm and Calendar event.

	RTC_B_clearInterrupt(RTC_B_BASE,
	RTCRDYIE + RTCTEVIE + RTCAIE);

	//RTC_B_enableInterrupt(RTC_B_BASE,
	//                      RTCRDYIE + RTCTEVIE + RTCAIE);

	RTC_B_enableInterrupt(RTC_B_BASE,
	RTCRDYIE + RTCTEVIE + RTCAIE);

	//Start RTC Clock
	RTC_B_startClock(RTC_B_BASE);
#else
	RTC_C_initCalendar(RTC_C_BASE, &g_rtcCalendarTime,
	RTC_C_FORMAT_BINARY);

	//Specify an interrupt to assert every minute
	RTC_C_setCalendarEvent(RTC_C_BASE,
	RTC_C_CALENDAREVENT_MINUTECHANGE);
        
	RTC_C_clearInterrupt(RTC_C_BASE,
	RTCRDYIE + RTCTEVIE + RTCAIE);

	RTC_C_enableInterrupt(RTC_C_BASE,
	RTCRDYIE + RTCTEVIE + RTCAIE);

	//Start RTC Clock
	RTC_C_startClock(RTC_C_BASE);        
#endif
}

void rtc_getlocal(struct tm* pTime) {
#ifndef _FISM_
	g_rtcCalendarTime = RTC_B_getCalendarTime(RTC_B_BASE);
#else
        g_rtcCalendarTime = RTC_C_getCalendarTime(RTC_C_BASE);
#endif        
	if (!pTime)
		return;

	pTime->tm_sec = g_rtcCalendarTime.Seconds;
	pTime->tm_min = g_rtcCalendarTime.Minutes;
	pTime->tm_hour = g_rtcCalendarTime.Hours;
	pTime->tm_wday = g_rtcCalendarTime.DayOfWeek;
	pTime->tm_mon = g_rtcCalendarTime.Month;
	pTime->tm_mday = g_rtcCalendarTime.DayOfMonth;
	pTime->tm_year = g_rtcCalendarTime.Year;        
}

time_t rtc_getUTC(struct tm* pTime, struct tm* tempDate) {
	rtc_getlocal(pTime);
	time_t timeSec = 0;

	memcpy(tempDate, pTime, sizeof(struct tm));
	tempDate->tm_year -= 1900;
	tempDate->tm_mon -= 1;

#if defined(__TI_COMPILER_VERSION__)
	timeSec = mktime(tempDate);
	memcpy(tempDate, gmtime(&timeSec), sizeof(struct tm));
	return mktime(tempDate) - UTC_70_YEARS;
#elif defined(__IAR_SYSTEMS_ICC__)
	timeSec = mktime(tempDate) + g_lTimeZoneOffset;
	memcpy(tempDate, gmtime(&timeSec), sizeof(struct tm));
	return timeSec;
#else
	return 	mktime(tempDate);
#endif

}
inline void led_blink()
{
  if((flags.low_battery)&&(!(POWER_CONNECTED)))
  {
     GREEN_LED_OFF;
     if((iSecondTick%5)==0)
     {
        RED_LED_ON;
     }
     else
     {
       RED_LED_OFF;
     }
  }
  else
  {
     if(flags.internet_flag)
    {
      RED_LED_OFF;
      GREEN_LED_TOGGLE;
    }
    else if((flags.sim_ready)&&(!flags.internet_flag))
    {
      GREEN_LED_OFF;
      RED_LED_TOGGLE;
    }
    else
    {
      GREEN_LED_OFF;
      RED_LED_ON;
    }
  }
}
time_t rtc_getUTCsecs() {
#ifndef _FISM_
	struct tm tempDate;
	return rtc_getUTC(&g_tmCurrTime, &tempDate);
#else
        return epoch_seconds;
#endif
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=RTC_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(RTC_VECTOR)))
#endif
#ifndef _FISM_
void RTC_B_ISR(void) {
#else
void RTC_C_ISR(void) {
#endif
	SYSTEM_ALARMS *s = &g_pSysState->state; //pointer to alarm states

	switch (__even_in_range(RTCIV, 16)) {
	case RTCIV_NONE:
		break;      //No interrupts
	case RTCIV_RTCRDYIFG:             //RTCRDYIFG
		//every second
		iSecondTick++;                
                epoch_seconds++;
		// Resume execution if the device is in deep sleep mode
		WAKEUP_EVENT
                led_blink();
		break;
	case RTCIV_RTCTEVIFG:             //RTCEVIFG
		//Interrupts every minute
		iMinuteTick++;

		// if we are NOT hibernating reboot
		if (--iDeadCountdown<=0 && !(s->alarms.hibernate)) {
#ifndef _FISM_
                  log_reboot("DEATH");
			system_reboot("MAIN_DEATH");
#endif
		}


		break;
	case RTCIV_RTCAIFG:             //RTCAIFG
		//Interrupts 5:00pm on 5th day of week
		__no_operation();
		break;
	case RTCIV_RT0PSIFG:
		break;      //RT0PSIFG
	case RTCIV_RT1PSIFG:
		break;     //RT1PSIFG
	case RTCIV_RTCOFIFG:
		break;     //Reserved
	default:
		break;
	}
}
