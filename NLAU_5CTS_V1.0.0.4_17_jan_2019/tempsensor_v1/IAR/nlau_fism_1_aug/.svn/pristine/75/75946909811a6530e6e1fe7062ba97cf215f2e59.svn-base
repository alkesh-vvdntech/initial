/*
 * \file   TI_MSPBoot_MI_FRAM.c
 *
 * \brief  Driver for memory interface using FRAM in FR57xx
 */
/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

//
// Include files
//
#include "msp430.h"
#include "TI_MSPBoot_Common.h"
#include "TI_MSPBoot_MI.h"


//
//  Functions declarations
//
/******************************************************************************
 *
 * @brief   Erase the application area (address obtained from linker file)
 *          FRAM doesn't have an "erased" state but this function is added
 *          for compatibility with Flash and in order to calculate CRC
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_EraseApp(void)
{
    uint16_t addr;
    uint32_t addrHigh;

    for (addr = 0xC3FF; addr >0x5400; addr--)
    {
       __data20_write_char(addr, 0xFF);
    }
    
    for (addrHigh = 0x42FFF; addrHigh >=0x10000; addrHigh--)
    {
        __data20_write_char(addr, 0xFF);
    }
}

/******************************************************************************
 *
 * @brief   Write a Byte to FRAM memory
 *          The bootloader is protected using MPU but all interrupts (except for
 *          Vector) can be reprogrammed 
 *
 * @param  addr     Address of the Byte being written (must be 32b to allow for 20bit addressing in 430X)
 * @param  data     Byte being written
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_WriteByte(uint32_t addr, uint8_t data)
{

#define VECTOR_REDIRECTION
#ifndef VECTOR_REDIRECTION
    if ((addr >= BOOT_VECTOR_TABLE) && (addr < BOOT_RESET_VECTOR_ADDR))
    {      
         MPUCTL0 = MPUPW | MPUENA;   // Enable access to MPU registers
        MPUSAM |= MPUSEG2WE;        // Enable Write access
        *(uint8_t *) ((uint16_t)addr) = data;   // Write the vector
        MPUSAM &= ~MPUSEG2WE;       // Disable Write access
        MPUCTL0_H = 0x00;           // Disable access to MPU registers      
        
        return RET_OK;
    }
#endif
    
#ifdef CONFIG_MI_MEMORY_RANGE_CHECK
    if ((addr < APP_MAIN_START_ADDR) ||
        ((addr > APP_MAIN_END_ADDR) && (addr < APP_HIGH_START_ADDR)) ||
        (addr > APP_HIGH_END_ADDR)
       )
        return RET_PARAM_ERROR;
#endif
    
    //extra check for validation check parameters which we don't want to overwrite
    if(addr == APP_UPDATE_PWD_ADDR ||
       addr == APP_UPDATE_PWD_ADDR + 1 ||
       addr == APP_UPDATE_STAT_ADDR
       )
        return RET_OK;
    
    if(addr & 0xFFFF0000) {
        *(__data20 uint8_t *) addr = data;
    } else {
        *(uint8_t *) ((uint16_t)addr) = data;
    }

    return RET_OK;
}
