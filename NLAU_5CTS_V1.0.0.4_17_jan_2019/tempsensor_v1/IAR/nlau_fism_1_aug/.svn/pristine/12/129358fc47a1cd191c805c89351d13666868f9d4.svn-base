
/*
 * Copyright (c) 2006-2012 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef SD_RAW_CONFIG_H
#define SD_RAW_CONFIG_H

#include <stdint.h>
//use on MSP340FR5969
#include <msp430.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup sd_raw
 *
 * @{
 */
/**
 * \file
 * MMC/SD support configuration (license: GPLv2 or LGPLv2.1)
 */

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write support.
 *
 * Set to 1 to enable MMC/SD write support, set to 0 to disable it.
 */
#define SD_RAW_WRITE_SUPPORT 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write buffering.
 *
 * Set to 1 to buffer write accesses, set to 0 to disable it.
 *
 * \note This option has no effect when SD_RAW_WRITE_SUPPORT is 0.
 */
#define SD_RAW_WRITE_BUFFERING 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD access buffering.
 * 
 * Set to 1 to save static RAM, but be aware that you will
 * lose performance.
 *
 * \note When SD_RAW_WRITE_SUPPORT is 1, SD_RAW_SAVE_RAM will
 *       be reset to 0.
 */
#define SD_RAW_SAVE_RAM 1

/**
 * \ingroup sd_raw_config
 * Controls support for SDHC cards.
 *
 * Set to 1 to support so-called SDHC memory cards, i.e. SD
 * cards with more than 2 gigabytes of memory.
 */
#define SD_RAW_SDHC 1

/**
 * @}
 */

/* defines for customisation of sd/mmc port access */

//msp430
// SPI port definitions
#define SPI_PxSEL         P2SEL1
#define SPI_PxDIR         P2DIR
#define SPI_PxIN          P2IN
#define SPI_PxOUT         P2OUT
#define SPI_SIMO          BIT5
#define SPI_SOMI          BIT6
#define SPI_UCLK          BIT4

//original
//#define configure_pin_mosi() DDRB |= (1 << DDB3)
//#define configure_pin_sck() DDRB |= (1 << DDB5)
//#define configure_pin_ss() DDRB |= (1 << DDB2)
//#define configure_pin_miso() DDRB &= ~(1 << DDB4)

//msp430
// Chip Select define
#define CS_PxOUT      P2OUT
#define CS_PxDIR      P2DIR
#define CS            BIT3

#define select_card() CS_PxOUT &= ~CS               // Card Select
#define unselect_card_preinit() CS_PxOUT |= CS  // Card Deselect - pre init
#define unselect_card() CS_PxOUT |= CS  // Card Deselect
//#define select_card() PORTB &= ~(1 << PORTB2)
//#define unselect_card() PORTB |= (1 << PORTB2)


#define configure_pin_available() /* nothing */
#define configure_pin_locked() /* nothing */

#define get_pin_available() 0
#define get_pin_locked() 1


#if SD_RAW_SDHC
    typedef uint64_t offset_t;
#else
    typedef uint32_t offset_t;
#endif

/* configuration checks */
#if SD_RAW_WRITE_SUPPORT
#undef SD_RAW_SAVE_RAM
#define SD_RAW_SAVE_RAM 0
#else
#undef SD_RAW_WRITE_BUFFERING
#define SD_RAW_WRITE_BUFFERING 0
#endif

#ifdef __cplusplus
}
#endif

#endif

