/*
 * i2c.h
 *
 *  Created on: Feb 10, 2015
 *      Author: rajeevnx
 */

#ifndef TEMPSENSOR_V1_I2C_H_
#define TEMPSENSOR_V1_I2C_H_

#include <msp430.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
#ifdef _CT5_BOOTLOADER_
/* bootloader held I2C vector table */
extern const uint32_t I2C_Vectors[];
#endif

//I2C configuration
#define   I2C_TX_LEN			32
#define   I2C_RX_LEN			32

#define I2C_INIT_SPEED  380000
#define MAX_I2CRX_RETRY  10
//*****************************************************************************
//
//! \brief Initialize I2C.
//!
//! \param speed
//!
//! \return None
//
//*****************************************************************************
extern void i2c_init(void);

//*****************************************************************************
//
//! \brief Perform I2C read.
//!
//! \param slave address
//! \param command
//! \param data length
//! \param pointer to store the data read from the slave
//!
//! \return None
//
//*****************************************************************************
extern void i2c_read(uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData);

//*****************************************************************************
//
//! \brief Perform I2C write.
//!
//! \param slave address
//! \param command
//! \param data length
//! \param pointer to store the data to be written to slave
//!
//! \return None
//
//*****************************************************************************
extern void i2c_write(uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData);

#ifdef __cplusplus
}
#endif

#endif /* TEMPSENSOR_V1_I2C_H_ */
