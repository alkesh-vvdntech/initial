/*
 * \file   TI_MSPBoot_MI.h
 *
 * \brief  Header file for the Memory Interface 
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

#ifndef __TI_MSPBoot_MI_H__
#define __TI_MSPBoot_MI_H__

//
// Include files
//

//
//  External variables from linker file. 
//  Note that this file gets constants from linker file in order to be able
//  to check where the Application resides, avoid corrupting boot area, etc
//
extern uint16_t _App_Main_Start;            /*! Application Start Address */
extern uint16_t _App_Main_End;              /*! Application End Address */
extern __data20 uint32_t _App_High_Start;   /*! Application Start Address */
extern __data20 uint32_t _App_High_End;     /*! Application End Address */
extern uint16_t _AppChecksum;               /*! Application Checksum Address */
extern uint16_t _AppUpdate_Password_Addr;   /*! AppUpdate Password Address */
extern uint16_t _AppUpdate_Status_Addr;     /*! AppUpdate Status Address */
extern uint16_t _App_Reset_Vector;          /*! Application Reset vector */
extern uint16_t _FRAM_VECTORS_START;        /*! Bootloader Vector Table Start */
extern uint16_t _FRAM_RESET_VECTOR;         /*! Bootloader Vector Table Start */

//
//  Macros and definitions
//
/*! Application Checksum address (from linker file) */
#define APP_CHECKSUM_ADDR           ((uint16_t )&_AppChecksum)
/*! Application start address (from linker file) */
#define APP_UPDATE_PWD_ADDR         ((uint16_t )&_AppUpdate_Password_Addr)
/*! Application start address (from linker file) */
#define APP_UPDATE_STAT_ADDR        ((uint16_t )&_AppUpdate_Status_Addr)
/*! Application start address (from linker file) */
#define APP_MAIN_START_ADDR         ((uint16_t )&_AppChecksum)
/*! Application end address (from linker file) */
#define APP_MAIN_END_ADDR           ((uint16_t )&_App_Main_End)
/*! Application Reset Vector */
#define APP_RESET_VECTOR_ADDR       ((uint16_t) &_App_Reset_Vector)
/*! Application Interrupt Table (from linker file) */
#define BOOT_VECTOR_TABLE           ((uint16_t) &_FRAM_VECTORS_START)
 /*! Boot Reset Vector (from linker file) */
#define BOOT_RESET_VECTOR_ADDR       ((uint16_t) &_FRAM_RESET_VECTOR)
/*! Application high start address (from linker file) */
#define APP_HIGH_START_ADDR          ((uint32_t )&_App_High_Start)
/*! Application high end address (from linker file) */
#define APP_HIGH_END_ADDR            ((uint32_t )&_App_High_End)

//
//  Functions prototypes
//
extern void TI_MSPBoot_MI_EraseApp(void);
extern uint8_t TI_MSPBoot_MI_WriteByte(uint32_t addr, uint8_t data);

#endif //__TI_MSPBoot_MI_H__
