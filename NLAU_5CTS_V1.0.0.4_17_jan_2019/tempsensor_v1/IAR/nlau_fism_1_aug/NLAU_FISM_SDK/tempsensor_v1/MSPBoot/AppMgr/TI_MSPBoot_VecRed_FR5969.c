/*
 * \file   TI_MSPBoot_VecRed_FR5969.c
 *
 * \brief  Vector redirection table for FR5969
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

#ifdef _CT5_BOOTLOADER_
//
// Include files
//
#include "msp430.h"
#include "TI_MSPBoot_Common.h"
#include "TI_MSPBoot_AppMgr.h"

//
//  External variables from linker file
//
extern uint16_t _App_Proxy_Vector_Start[];  /* App proxy table address */
//
//  Macros and definitions
//
/* Value written to unused vectors */
#define RESERVED                (0x3FFF)  
/*! Macro used to calculate address of vector in Application Proxy Table */
#define APP_PROXY_VECTOR(x)     ((uint16_t)&_App_Proxy_Vector_Start[x*2])

//
//  Constant table
//
/*! MSPBoot Vector Table: It's fixed since it can't be erased and modified.
 *  Points to a proxy vector table in Application area*/
#   ifdef __IAR_SYSTEMS_ICC__
#       pragma location="BOOT_VECTOR_TABLE"
__root const uint16_t Vector_Table[] =
#   elif defined (__TI_COMPILER_VERSION__)
#       pragma DATA_SECTION(Vector_Table, ".BOOT_VECTOR_TABLE")
#       pragma RETAIN(Vector_Table)
const uint16_t Vector_Table[] =
#   endif
{
#ifndef _FISM_
    APP_PROXY_VECTOR(0),                        // FFCC = AES256
    APP_PROXY_VECTOR(1),                        // FFCE = RTC
    APP_PROXY_VECTOR(2),                        // FFD0 = P4
    APP_PROXY_VECTOR(3),                        // FFD2 = P3
    APP_PROXY_VECTOR(4),                        // FFD4 = Timer3_A2 CC1, TA
    APP_PROXY_VECTOR(5),                        // FFD6 = Timer3_A2 CC0
    APP_PROXY_VECTOR(6),                        // FFD8 = P2
    APP_PROXY_VECTOR(7),                        // FFDA = Timer2_A2 CC1, TA
    APP_PROXY_VECTOR(8),                        // FFDC = Timer2_A2 CC0
    APP_PROXY_VECTOR(9),                        // FFDE = P1
    APP_PROXY_VECTOR(10),                       // FFE0 = Timer1_A3 CC1-2, TA 
    APP_PROXY_VECTOR(11),                       // FFE2 = Timer1_A3 CC0
    APP_PROXY_VECTOR(12),                       // FFE4 = DMA
    APP_PROXY_VECTOR(13),                       // FFE6 = USCI A1
    APP_PROXY_VECTOR(14),                       // FFE8 = Timer0_A3 CC1-2, TA
    RESERVED,                                   // FFEA = Timer0_A3 CC0
    APP_PROXY_VECTOR(16),                       // FFEC = ADC
    RESERVED,                                   // FFEE = USCI B0
    APP_PROXY_VECTOR(18),                       // FFF0 = USCI A0
    APP_PROXY_VECTOR(19),                       // FFF2 = WDT
    APP_PROXY_VECTOR(20),                       // FFF4 = Timer0_B7 CC1-6, TB
    APP_PROXY_VECTOR(21),                       // FFF6 = Timer0_B7 CC0
    APP_PROXY_VECTOR(22),                       // FFF8 = COMP_E
    APP_PROXY_VECTOR(23),                       // FFFA = User NMI
    APP_PROXY_VECTOR(24)                        // FFFC = Sys NMI
#else
    APP_PROXY_VECTOR(0),                        // FFB4 = LEA_VECTOR
    APP_PROXY_VECTOR(1),                        // FFB6 = PORT8_VECTOR
    APP_PROXY_VECTOR(2),                        // FFB8 = PORT7_VECTOR     
    APP_PROXY_VECTOR(3),                        // FFBA = EUSCI_B3_VECTOR
    APP_PROXY_VECTOR(4),                        // FFBC = EUSCI_B2_VECTOR
    APP_PROXY_VECTOR(5),                        // FFBE = EUSCI_B1_VECTOR
    APP_PROXY_VECTOR(6),                        // FFC0 = EUSCI_A3_VECTOR
    APP_PROXY_VECTOR(7),                        // FFC2 = EUSCI_A2_VECTOR
    APP_PROXY_VECTOR(8),                        // FFC4 = PORT6_VECTOR
    APP_PROXY_VECTOR(9),                        // FFC6 = PORT5_VECTOR
    APP_PROXY_VECTOR(10),                       // FFC8 = TIMER4_A1_VECTOR
    APP_PROXY_VECTOR(11),                       // FFCA = TIMER4_A0_VECTOR
    APP_PROXY_VECTOR(12),                       // FFCC = AES256_VECTOR
    APP_PROXY_VECTOR(13),                       // FFCE = RTC_C_VECTOR
    APP_PROXY_VECTOR(14),                       // FFD0 = PORT4_VECTOR
    APP_PROXY_VECTOR(15),                       // FFD2 = PORT3_VECTOR
    APP_PROXY_VECTOR(16),                       // FFD4 = Timer3_A2 CC1, TA
    APP_PROXY_VECTOR(17),                       // FFD6 = Timer3_A2 CC0
    APP_PROXY_VECTOR(18),                       // FFD8 = P2
    APP_PROXY_VECTOR(19),                       // FFDA = TIMER2_A1_VECTOR
    APP_PROXY_VECTOR(20),                       // FFDC = TIMER2_A0_VECTOR
    APP_PROXY_VECTOR(21),                       // FFDE = PORT1_VECTOR
    APP_PROXY_VECTOR(22),                       // FFE0 = TIMER1_A1_VECTOR
    APP_PROXY_VECTOR(23),                       // FFE2 = TIMER1_A0_VECTOR
    APP_PROXY_VECTOR(24),                       // FFE4 = DMA_VECTOR
    APP_PROXY_VECTOR(25),                       // FFE6 = EUSCI_A1_VECTOR
    APP_PROXY_VECTOR(26),                       // FFE8 = TIMER0_A1_VECTOR
    APP_PROXY_VECTOR(27),                       // FFEA = TIMER0_A0_VECTOR
    APP_PROXY_VECTOR(28),                       // FFEC = ADC12_B_VECTOR
    APP_PROXY_VECTOR(29),                       // FFEE = EUSCI_B0_VECTOR
    APP_PROXY_VECTOR(30),                       // FFF0 = EUSCI_A0_VECTOR
    APP_PROXY_VECTOR(31),                       // FFF2 = WDT_VECTOR
    APP_PROXY_VECTOR(32),                       // FFF4 = TIMER0_B1_VECTOR
    APP_PROXY_VECTOR(33),                       // FFF6 = TIMER0_B0_VECTOR
    APP_PROXY_VECTOR(34),                       // FFF8 = COMP_E_VECTOR
    APP_PROXY_VECTOR(35),                       // FFFA = UNMI_VECTOR
    APP_PROXY_VECTOR(36)                        // FFFC = SYSNMI_VECTOR
#endif
};
#else
#include "stdint.h"
#include "intrinsics.h"

//
//  External variables from linker file
//
extern uint16_t _App_Reset_Vector[];  /* App proxy table address */

//
//  Constant tables
//
/*! This is a "proxy" interrupt table which is used by the bootloader to jump to
    each interrupt routine.
    It always resides in the same location.
    It contains a BRA instruction (0x4030) followed by the address of each
    vector routine.
    Note that the number and location of each vector should match the declaration
    in Boot's Vector_Table
    Unimplemented vectors are removed (and unused vectors can be removed too)
    to save flash space
*/
#   ifdef __IAR_SYSTEMS_ICC__
#       pragma location="APP_PROXY_VECTORS"
__root const uint16_t ProxyVectorTable[] =
#   elif defined (__TI_COMPILER_VERSION__)
#       pragma DATA_SECTION(ProxyVectorTable, ".APP_PROXY_VECTORS")
#       pragma RETAIN(ProxyVectorTable)
const uint16_t ProxyVectorTable[] =
#   endif
{ 
#ifndef _FISM_
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(0)     // FFCC = AES256
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(1)     // FFCE = RTC
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(2)     // FFD0 = P4
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(3)     // FFD2 = P3
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(4)     // FFD4 = Timer3_A2 CC1, TA
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(5)     // FFD6 = Timer3_A2 CC0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(6)     // FFD8 = P2
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(7)     // FFDA = Timer2_A2 CC1, TA
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(8)     // FFDC = Timer2_A2 CC0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(9)     // FFDE = P1
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(10)    // FFE0 = Timer1_A3 CC1-2, TA 
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(11)    // FFE2 = Timer1_A3 CC0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(12)    // FFE4 = DMA
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(13)    // FFE6 = USCI A1
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(14)    // FFE8 = Timer0_A3 CC1-2, TA
    0x4030, (uint16_t)&_App_Reset_Vector[0], // RESERVED FOR BOOTLOADER // FFEA = Timer0_A3 CC0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(16)    // FFEC = ADC
    0x4030, (uint16_t)&_App_Reset_Vector[0], // RESERVED FOR BOOTLOADER // FFEE = USCI B0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(18)    // FFF0 = USCI A0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(19)    // FFF2 = WDT
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(20)    // FFF4 = Timer0_B7 CC1-6, TB
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(21)    // FFF6 = Timer0_B7 CC0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(22)    // FFF8 = COMP_E
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(23)    // FFFA = User NMI
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(24)    // FFFC = Sys NMI
#else
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(0)      // FFB4 = LEA_VECTOR              //C36C
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(1)      // FFB6 = PORT8_VECTOR            //C370
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(2)      // FFB8 = PORT7_VECTOR            //C374
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(3)      // FFBA = EUSCI_B3_VECTOR         //C378
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(4)      // FFBC = EUSCI_B2_VECTOR         //C37C
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(5)      // FFBE = EUSCI_B1_VECTOR         //C380
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(6)      // FFC0 = EUSCI_A3_VECTOR         //C384
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(7)      // FFC2 = EUSCI_A2_VECTOR         //C388
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(8)      // FFC4 = PORT6_VECTOR            //C38C
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(9)      // FFC6 = PORT5_VECTOR            //C390
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(10)     // FFC8 = TIMER4_A1_VECTOR        //C394
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(11)     // FFCA = TIMER4_A0_VECTOR        //C398
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(12)     // FFCC = AES256                  //C39C
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(13)     // FFCE = RTC                     //C3A0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(14)     // FFD0 = P4                      //C3A4
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(15)     // FFD2 = P3                      //C3A8
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(16)     // FFD4 = Timer3_A2 CC1, TA       //C3AC
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(17)     // FFD6 = Timer3_A2 CC0           //C3B0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(18)     // FFD8 = P2                      //C3B4
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(19)     // FFDA = Timer2_A2 CC1, TA       //C3B8
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(20)     // FFDC = Timer2_A2 CC0           //C3BC
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(21)     // FFDE = P1                      //C3C0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(22)     // FFE0 = Timer1_A3 CC1-2, TA     //C3C4
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(23)     // FFE2 = Timer1_A3 CC0           //C3C8
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(24)     // FFE4 = DMA                     //C3CC
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(25)     // FFE6 = USCI A1                 //C3D0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(26)     // FFE8 = Timer0_A1               //C3D4
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(27)     // FFEA = Timer0_A0               //C3D8
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(28)     // FFEC = ADC                     //C3DC
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(29)     // FFEE = USCI B0                 //C3E0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(30)     // FFF0 = USCI A0                 //C3E4
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(31)     // FFF2 = WDT                   ` //C3E8
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(32)     // FFF4 = Timer0_B7 CC1-6, TB     //C3EC
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(33)     // FFF6 = Timer0_B7 CC0           //C3F0
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(34)     // FFF8 = COMP_E                  //C3F4
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(35)     // FFFA = User NMI                //C3F8
    0x4030, (uint16_t)&_App_Reset_Vector[0], // APP_PROXY_VECTOR(36)     // FFFC = Sys NMI                 //C3FC 
#endif
};


#endif


