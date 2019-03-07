/*
 * \file   TI_MSPBoot_AppMgr.c
 *
 * \brief  Application Manager. Handles App validation, decides if device
 *         should jump to App or stay in bootloader
 *         This file supports dual image
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
#include "TI_MSPBoot_AppMgr.h"
#include "crc.h"

//
//  Configuration checks
//
#if ((CONFIG_APPMGR_APP_VALIDATE != 2) && (CONFIG_APPMGR_APP_VALIDATE != 3))
#error CRC is required in order to validate application
#endif

//
//  Global variables
//
/*! Password sent by Application to force boot mode. This variable is in a fixed
    location and should keep same functionality and location in Boot and App */
#ifdef __IAR_SYSTEMS_ICC__
#           pragma location="RAM_PASSWORD"
    __no_init uint16_t  PassWd;
#       elif defined (__TI_COMPILER_VERSION__)
extern uint16_t  PassWd;
#endif

/*! Status and Control byte. This variable is in a fixed
 location and should keep same functionality and location in Boot and App */
#ifdef __IAR_SYSTEMS_ICC__
#           pragma location="RAM_STATCTRL"
    __no_init uint8_t  StatCtrl;
#       elif defined (__TI_COMPILER_VERSION__)
extern uint8_t  StatCtrl;
#endif

//
//  Local function prototypes
//
static tBOOL TI_MSPBoot_AppMgr_BootisForced(void);

/******************************************************************************
 *
 * @brief   Checks if an Application is valid
 *  Depending on CONFIG_APPMGR_APP_VALIDATE, this function can validate app by:
 *  CONFIG_APPMGR_APP_VALIDATE  |    Function
 *          2                   | Check CRC_CCITT of application and compare vs
 *                              |  an expected value
 *          3                   | Check CRC8 of application and compare vs
 *                              |  an expected value
 *          other               | Application is expected to be valid
 *
* @param Downarea 0:Valdate App Area, 1:Validate Download Area
 *
 * @return  TRUE_t if application is valid,
 *          FALSE_t if applicaiton is invalid
 *****************************************************************************/
#if (CONFIG_APPMGR_APP_VALIDATE == 2)
// Check the Applications's Checksum
static tBOOL TI_MSPBoot_AppMgr_AppisValid(uint8_t Downarea)
{
    extern uint16_t _AppChecksum;   // Address of Application checksum
    extern uint16_t _App_Start;     // Address of Application start
    extern uint16_t _CRC_Size;      // Address of App area being calculated
    extern uint16_t _AppChecksumDown;   // Address of Download area checksum
    extern uint16_t _App_StartDown;     // Address of Download area start
    
    if (Downarea == 0)
    {
        // Validate Application Area
        // calculate CRC and compare vs expected value in reserved location
        if (crc16MakeBitwise((uint8_t *) & _App_Start, (uint16_t) &_CRC_Size) != 
            _AppChecksum)
        {
            return FALSE_t;
        }
        else
        {
            return TRUE_t;
        }
    }
    else
    {
        // Validate application in Download Area
        // calculate CRC and compare vs expected value in reserved location
        if (crc16MakeBitwise((uint8_t *) & _App_StartDown, (uint16_t) &_CRC_Size) != 
            _AppChecksumDown)
        {
            return FALSE_t;
        }
        else
        {
            return TRUE_t;
        }
    }
}
#elif (CONFIG_APPMGR_APP_VALIDATE == 3)
// Check the Applications's Checksum
static tBOOL TI_MSPBoot_AppMgr_AppisValid(uint8_t Downarea)
{
    extern uint8_t _AppChecksum_8;   // Address of Application checksum
    extern uint16_t _App_Start;     // Address of Application start
    extern uint16_t _CRC_Size;      // Address of App area being calculated
    extern uint8_t _AppChecksum_8Down;   // Address of Download Area checksum
    extern uint16_t _App_StartDown;     // Address of Download Area start

    if (Downarea == 0)
    {
        // Validate Application Area
        // calculate CRC and compare vs expected value in reserved location
        if (crc8MakeBitwise((uint8_t *) & _App_Start, (uint16_t) &_CRC_Size) != 
            _AppChecksum_8)
        {
            return FALSE_t;
        }
        else
        {
            return TRUE_t;
        }
    }
    else
    {
        // Validate Image in Download Area
        // calculate CRC and compare vs expected value in reserved location
        if (crc8MakeBitwise((uint8_t *) & _App_StartDown, (uint16_t) &_CRC_Size) != 
            _AppChecksum_8Down)
        {
            return FALSE_t;
        }
        else
        {
            return TRUE_t;
        }
    }

}
#else
// Always assume that Application is valid
#error "Application is not validated"
#endif

/******************************************************************************
 *
 * @brief   Decides whether to stay in MSPBoot or if it should jump to App
 *  MSPBoot:  Boot mode is forced by a call from App, OR
 *          Boot mode is forced by an external event (button pressed), OR
 *          Application is invalid
 *  App:    Boot mode is not forced, AND
 *          Application is valid
 *
 * @return  TRUE_t if application is valid and should be executed
 *          FALSE_t if we must stay in Boot mode
 *****************************************************************************/
tBOOL TI_MSPBoot_AppMgr_ValidateApp(void)
{
    extern void TI_MSPBoot_MI_EraseAppDirect(uint8_t DownArea);
    extern void TI_MSPBoot_MI_ReplaceApp(void);

    if (TI_MSPBoot_AppMgr_BootisForced() == FALSE_t)
    {
        // If Bootloader is not forced, check the current image
        if (TI_MSPBoot_AppMgr_AppisValid(0) == TRUE_t)
        {
            // Image valid, jump to App
            return TRUE_t;
        }
        else
        {
            // Image invalid, check Download Area
            if (TI_MSPBoot_AppMgr_AppisValid(1) == TRUE_t)
            {
                // Erase App Area
                TI_MSPBoot_MI_EraseAppDirect(0);
                // Replace the Image in App Area by Download Area
                TI_MSPBoot_MI_ReplaceApp();
                // Validate image in App Area
                if (TI_MSPBoot_AppMgr_AppisValid(0) == TRUE_t)
                {
                    // Erase the image in Download Area
                    TI_MSPBoot_MI_EraseAppDirect(1);
                    return TRUE_t;
                }
            }
        }
    }
    
    return FALSE_t; // Boot is forced or App is valid
}


/******************************************************************************
 *
 * @brief   Jump to Appplication 
 *          The image in download area is validated and copied to App Area if valid
 *          A Reset is forced in order to validate Application after Reset
 *
 * @return  None
 *****************************************************************************/
void TI_MSPBoot_AppMgr_JumpToApp(void)
{
    extern void TI_MSPBoot_MI_EraseAppDirect(uint8_t DownArea);
    extern void TI_MSPBoot_MI_ReplaceApp(void);

    // Check image in download area 
    WATCHDOG_FEED();
    if (TI_MSPBoot_AppMgr_AppisValid(1) == TRUE_t)
    {
        // If Image is valid, erase current App
        TI_MSPBoot_MI_EraseAppDirect(0);
        // Replace current App by Image in Download Area
        TI_MSPBoot_MI_ReplaceApp();
        #ifdef NDEBUG
        // Feed the dog 
        WATCHDOG_FEED();
        #endif

        // Validate image in App Area
        if (TI_MSPBoot_AppMgr_AppisValid(0) == TRUE_t)
        {
            // Erase the image in Download Area
             TI_MSPBoot_MI_EraseAppDirect(1);
        }
    }
#if defined (HW_RESET_BOR)
    // Force a Software BOR
    PMMCTL0 = PMMPW | PMMSWBOR;
#else
    // Force a PUC reset by writing incorrect Watchdog password
    WDTCTL = 0xDEAD;
#endif
}


/******************************************************************************
 *
 * @brief   Checks if Boot mode is forced
 *          Boot mode is forced by an application call sending a request and password
 *          Or by an external event such as a button press
 *
 * @return  TRUE_t Boot mode is forced
 *          FALSE_t Boot mode is not forced
 *****************************************************************************/
static tBOOL TI_MSPBoot_AppMgr_BootisForced(void)
{
    tBOOL ret = FALSE_t;

    // Check if application is requesting Boot mode and password is correct
    if (((StatCtrl & BOOT_APP_REQ) != 0) && (PassWd == BSL_PASSWORD))
    {
        ret = TRUE_t;
    }

    // Check for an external event such as S3 (P1.3) button in Launchpad
    __delay_cycles(10000);   // Wait for pull-up to go high
    //If S2 button (P1.3) is pressed, force BSL
    if (HW_ENTRY_CONDITION)
    {
        ret = TRUE_t;
    }

    // Clear Status and Control byte and password
    PassWd = 0;
    StatCtrl = 0;
    return ret;
}
