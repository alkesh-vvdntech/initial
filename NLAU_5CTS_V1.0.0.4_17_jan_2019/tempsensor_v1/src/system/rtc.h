/*
 * rtc.h
 *
 *  Created on: Feb 5, 2015
 *      Author: rajeevnx
 */

#ifndef TEMPSENSOR_V1_RTC_H_
#define TEMPSENSOR_V1_RTC_H_


#include "time.h"

#ifdef __cplusplus
extern "C"
{
#endif
  
#define MINUTES_BEFORE_REBOOT 60
void rtc_dead_mans_switch();
void rtc_setTimeZoneOffset(time_t tz);
//*****************************************************************************
//
//! \brief Initialize RTC.
//!
//! \param inittime is the initial calendar time.
//!
//! \return None
//
//*****************************************************************************
void rtc_init(struct tm* pTime);

//*****************************************************************************
//
//! \brief Get current RTC time in UTC format.
//!
//! \param stores the current time read from RTC.
//!
//! \return time_t (also current time in UTC)
//
//*****************************************************************************
time_t rtc_getUTC(struct tm* pTime, struct tm* tempDate);
time_t rtc_getUTCsecs(); //helper function if you only want time_t

//*****************************************************************************
//
//! \brief Get current RTC time.
//!
//! \param stores the current time read from RTC.
//!
//! \return none
//
//*****************************************************************************
void rtc_getlocal(struct tm* pTime);
#ifdef __cplusplus
}
#endif

#endif /* TEMPSENSOR_V1_RTC_H_ */
