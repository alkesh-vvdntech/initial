/*
 * \file   TI_MSPBoot_MI_FRAMDualImg.c
 *
 * \brief  Driver for memory interface using FRAM in FR57xx
 *         This file supports Dual Image
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
// Local Function prototypes
//
static uint8_t TI_MSPBoot_MI_EraseSectorDirect(uint16_t addr);
static uint8_t TI_MSPBoot_MI_WriteByteDirect(uint16_t addr, uint8_t data);
// Function prototypes
void TI_MSPBoot_MI_EraseAppDirect(uint8_t DownArea);
void TI_MSPBoot_MI_ReplaceApp(void);

//
//  Functions declarations
//
/******************************************************************************
*
 * @brief   Erase a FRAM Sector
 *          FRAM doesn't have sectors or an "erase" state but this function 
 *          is added for compatibility with Flash and erases as 0xFF
 *          in order to calculate CRC
 *
 * @param  addr    Address in the sector being erased (sector is 512B)
 *                  The actual address will be in Download area
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_EraseSector(uint16_t addr)
{
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
    
#ifdef CONFIG_MI_MEMORY_RANGE_CHECK
    // Check address to be within Application range
    if ((addr < APP_START_ADDR) || (addr > APP_END_ADDR))
        return RET_PARAM_ERROR;
#endif
    // Erase address will be in Download area
    addr -= (APP_START_ADDR- ((uint16_t )&_FLASHDOWN_START));
    // Erase the corresponding area
    TI_MSPBoot_MI_EraseSectorDirect(addr);

    return RET_OK;
}

/******************************************************************************
*
 * @brief   Erase a FRAM Sector directly
 *
 * @param  addr    Address in the sector being erased (sector is 512B)
 *          FRAM doesn't have sectors or an "erase" state but this function 
 *          is added for compatibility with Flash and erases as 0xFF
 *          in order to calculate CRC
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_EraseSectorDirect(uint16_t addr)
{
    uint16_t i;
    uint8_t * ptr;
    // Erase is not necessary for FRAM but we do it in order to calculate CRC
    // properly
    ptr = (uint8_t *) (addr & ~0x1FF);
    for (i = 0; i < 512; i++)
        *ptr++ = 0xFF;


    return RET_OK;
}
        
/******************************************************************************
 *
 * @brief   Erase the application area (address obtained from linker file)
 *          Erases the application in Download Area
 *          FRAM doesn't have an "erased" state but this function is added
 *          for compatibility with Flash and in order to calculate CRC
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_EraseApp(void)
{
    // Erase Download Area
    TI_MSPBoot_MI_EraseAppDirect(1);
 
}

/******************************************************************************
 *
 * @brief   Erase the application area (address obtained from linker file)
 *          It can erase application in Download or App Area
 *          FRAM doesn't have an "erased" state but this function is added
 *          for compatibility with Flash and in order to calculate CRC
 *
 * @param DownArea 1:Erase Download area, 0: Erase App Area
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_EraseAppDirect(uint8_t DownArea)
{
    uint16_t addr;
    uint16_t Start, End;
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
    extern uint16_t _App_EndDown; // Define Address of Download area

    if (DownArea==0)
    {
        Start = APP_START_ADDR;
        End = APP_END_ADDR;
    }
    else
    {
        Start = ((uint16_t )&_FLASHDOWN_START);
        End = ((uint16_t )&_App_EndDown);
    }

    for (addr = End; addr >=Start; addr--)
    {
        *(uint8_t *) (addr) = 0xFF;
    }

}

/******************************************************************************
 *
 * @brief   Write a Byte to FRAM memory
 *      This function writes the byte to Download area 
 *
 * @param  addr     Address of the Byte being written in Application area
 *                  The actual address will be in Download area
 * @param  data     Byte being written
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_WriteByte(uint16_t addr, uint8_t data)
{
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
#ifdef CONFIG_MI_MEMORY_RANGE_CHECK
    if ((addr < APP_START_ADDR) || (addr > APP_END_ADDR))
        return RET_PARAM_ERROR;
#endif
    // Write address will be in Download area
    addr -= (APP_START_ADDR- ((uint16_t )&_FLASHDOWN_START));
    // Write the byte
    TI_MSPBoot_MI_WriteByteDirect(addr, data);
    
    return RET_OK;
}

/******************************************************************************
 *
 * @brief   Write a Byte Directly to Flash memory
 *          The bootloader is protected using MPU but all interrupts (except for
 *          Vector) can be reprogrammed 
 *
 * @param  addr     Address of the Byte being written in Flash
 * @param  data     Byte being written
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_WriteByteDirect(uint16_t addr, uint8_t data)
{
    *(uint8_t *) addr = data;
    if ((addr >= APP_VECTOR_TABLE) && (addr < APP_RESET_VECTOR_ADDR))
    {
        // If address is an interrupt vector, copy directly to interrupt table
        addr = (addr - APP_VECTOR_TABLE) + BOOT_VECTOR_TABLE;
        MPUCTL0 = MPUPW | MPUENA;   // Enable access to MPU registers
        MPUSAM |= MPUSEG3WE;        // Enable Write access
        *(uint8_t *) addr = data;   // Write the vector
        MPUSAM &= ~MPUSEG3WE;       // Disable Write access
        MPUCTL0_H = 0x00;           // Disable access to MPU registers
    }
    return RET_OK;
}


/******************************************************************************
 *
 * @brief   Replaces the application area with the contents of download area
 *          This function should only be called after validating the download
 *          area.
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_ReplaceApp(void)
{
    uint16_t addr;
    uint8_t *data;
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
    extern uint16_t _App_EndDown;   // Define End Address of Download area

    data = (uint8_t *) (&_FLASHDOWN_START);
    for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr++)
    {
        TI_MSPBoot_MI_WriteByteDirect(addr, *data++);
    }
}
