/*
 * battery.h
 *
 *  Created on: Feb 11, 2015
 *      Author: rajeevnx
 */

#ifndef TEMPSENSOR_V1_BATTERY_H_
#define TEMPSENSOR_V1_BATTERY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SLAVE_ADDR_BATTERY	        0x55

#define BATT_CONTROL			0x00
#define BATT_FLAGS			0x06
#define BATT_FULL_AVAIL_CAPACITY	0x0a
#define BATT_REMAINING_CAPACITY		0x0c
#define BATT_FULL_CHG_CAPACITY		0x0e
#define BATT_STATE_OF_CHARGE		0x1c
#define BATT_BLOCK_DATA_CHECKSUM	0x60
#define BATT_BLOCK_DATA_CONTROL		0x61
#define BATT_DATA_BLOCK_CLASS		0x3E
#define BATT_DATA_BLOCK			0x3F
#define BATT_DESIGN_CAPACITY		0x4A
#define BATT_DESIGN_ENERGY		0x4C
#define BATT_TERM_VOLT			0x50
#define BATT_TAPER_RATE			0x5B
#define BATT_TAPER_VOLT			0x5D
#define BATT_FULLCHRGCAP		0x0E

//*****************************************************************************
//! \brief Initialize Battery fuel guage.
//! \param speed
//! \return None
//*****************************************************************************
//void batt_init();

// Checks level, displays a message with the result
// aditya uint8_t batt_check_level();
void check_batt_state();

//#define BATT_STATE_OF_CHARGE		0x2C
//#define BATT_VOLATGE                  0x08
//#define BATT_CURRENT                  0x0C

//*****************************************************************************
//! \brief Get the battery remaining capacity
//! \return battery remaining capacity in percentage
//*****************************************************************************
uint8_t batt_getlevel();

//int batt_current();
//uint16_t batt_voltage();


#ifdef __cplusplus
}
#endif

#endif