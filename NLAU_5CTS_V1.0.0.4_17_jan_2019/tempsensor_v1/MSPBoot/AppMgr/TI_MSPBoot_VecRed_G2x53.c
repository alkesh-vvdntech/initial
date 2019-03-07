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
#include "TI_MSPBoot_AppMgr.h"

//
//  External variables from linker file
//
extern uint16_t _App_Proxy_Vector_Start[];  /* Proxy table address */

//
//  Macros and definitions
//
#define UNUSED 0x3FFF       /* Value written to unused vectors */
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

    UNUSED,                                     // FFE0 = unused
    UNUSED,                                     // FFE2 = unused
    APP_PROXY_VECTOR(0),                        // FFE4 = P1
    APP_PROXY_VECTOR(1),                        // FFE6 = P2
    APP_PROXY_VECTOR(2),                        // FFE8 = USI
    APP_PROXY_VECTOR(3),                        // FFEA = ADC10
    UNUSED,                                     // FFEC = unused
    UNUSED,                                     // FFEE = unused
    APP_PROXY_VECTOR(4),                        // FFF0 = TA0_1
    APP_PROXY_VECTOR(5),                        // FFF2 = TA0_0
    APP_PROXY_VECTOR(6),                        // FFF4 = WDT
    APP_PROXY_VECTOR(7),                        // FFF6 = COMP_A
    UNUSED,                                     // FFF8 = unused
    UNUSED,                                     // FFFA = unused
    APP_PROXY_VECTOR(8),                        // FFFC = NMI
};

