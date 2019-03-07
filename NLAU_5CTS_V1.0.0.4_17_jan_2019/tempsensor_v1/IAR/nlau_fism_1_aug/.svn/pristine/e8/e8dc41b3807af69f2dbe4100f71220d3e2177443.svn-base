/*
 * \file   TI_MSPBoot_AppMgr.c
 *
 * \brief  Application Manager. Handles App validation, decides if device
 *         should jump to App or stay in bootloader
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
#include "TI_TXT_File_Parser.h"
#include "TI_MSPBoot_MI.h"
#include "FAT_Update.h"
#include "crc.h"
#include "lcd.h"
#include "image_write.h"
//
//  Global variables
//
/*! Password sent by Application to force boot mode. This variable is in a fixed
    location and should keep same functionality and location in Boot and App */
#ifdef __IAR_SYSTEMS_ICC__
#           pragma location="FRAM_PASSWORD"
    __no_init uint16_t  PassWd;
#       elif defined (__TI_COMPILER_VERSION__)
extern uint16_t  PassWd;
#endif

/*! Status and Control byte. This variable is in a fixed
 location and should keep same functionality and location in Boot and App */
#ifdef __IAR_SYSTEMS_ICC__
#           pragma location="FRAM_STATCTRL"
    __no_init uint8_t  StatCtrl;
#       elif defined (__TI_COMPILER_VERSION__)
extern uint8_t  StatCtrl;
#endif

//
//  Local function prototypes
//
static void TI_MSPBoot_AppMgr_ForceValidation(void);
static tBOOL TI_MSPBoot_AppMgr_ValidationIsForced(void);

/******************************************************************************
 *
 * @brief   Checks if an Application is valid
 *  Depending on CONFIG_APPMGR_APP_VALIDATE, this function can validate app by:
 *  CONFIG_APPMGR_APP_VALIDATE  |    Function
 *          1                   | Check if reset vector is different from 0xFFFF
 *          2                   | Check CRC_CCITT of application and compare vs
 *                              |  an expected value
 *          3                   | Check CRC8 of application and compare vs
 *                              |  an expected value
 *          other               | Application is expected to be valid
 *
 *
 * @return  TRUE_t if application is valid,
 *          FALSE_t if applicaiton is invalid
 *****************************************************************************/
#if (CONFIG_APPMGR_APP_VALIDATE == 1)
static tBOOL TI_MSPBoot_AppMgr_AppisValid(void)
{
    // Check if Application Reset vector exists
    if (*(volatile uint16_t *)(&_App_Reset_Vector) != 0xFFFF)
    {
        return TRUE_t;
    }
    else
    {
        return FALSE_t;
    }
}
#elif (CONFIG_APPMGR_APP_VALIDATE == 2)
// Check the Applications's Checksum
static tBOOL TI_MSPBoot_AppMgr_AppisValid(void)
{
    extern uint16_t _AppChecksum;   // Address of Application checksum
    extern uint16_t _App_Main_Start;     // Address of main Application start
    extern uint16_t _CRC_Main_Size;      // Address of main App area being calculated
    extern uint16_t _FRAM_VECTORS_START;        /*! Bootloader Vector Table Start */
    extern uint16_t _FRAM_RESET_VECTOR;         /*! Bootloader Vector Table Start */
    extern __data20 uint32_t _App_High_Start;     // Address of high Application start
#ifndef _FISM_
    extern uint16_t _CRC_High_Size;      // Address of high App area being calculated
#else
    extern uint32_t _CRC_High_Size;      // Address of high App area being calculated
#endif
    uint16_t checksumResult;
    
    checksumResult = crc16MakeBitwise((uint8_t __data20 *) & _App_Main_Start, (uint16_t) &_CRC_Main_Size);
    //checksumResult = crc16MakeBitwiseUsingSeed((uint8_t __data20 *) & _FRAM_VECTORS_START, (uint16_t) (&_FRAM_RESET_VECTOR - &_FRAM_VECTORS_START), checksumResult);
#ifndef _FISM_
    checksumResult = crc16MakeBitwiseUsingSeed((uint8_t __data20 *) & _App_High_Start, (uint16_t) &_CRC_High_Size, checksumResult);
#else
    checksumResult = crc16MakeBitwiseUsingSeed((uint8_t __data20 *) & _App_High_Start, _CRC_High_Size, checksumResult);
#endif

    // calculate CRC and compare vs expected value in reserved location
    if (checksumResult != _AppChecksum)
    {
        return FALSE_t;
    }
    else
    {
        return TRUE_t;
    }
//sachin    return TRUE_t;
}
#elif (CONFIG_APPMGR_APP_VALIDATE == 3)
// Check the Applications's Checksum
static tBOOL TI_MSPBoot_AppMgr_AppisValid(void)
{
    extern uint8_t _AppChecksum_8;   // Address of Application checksum
    extern uint16_t _App_Start;     // Address of Application start
    extern uint16_t _CRC_Size;      // Address of App area being calculated

    // calculate CRC and compare vs expected value in reserved location
    if (crc8MakeBitwise((uint8_t *) & _App_Main_Start, (uint16_t) &_CRC_Size) != 
        _AppChecksum_8)
    {
        return FALSE_t;
    }
    else
    {
        return TRUE_t;
    }
}
#else
// Always assume that Application is valid
#warning "Application is not validated"
#define TI_MSPBoot_AppMgr_AppisValid()   TRUE_t
#endif

/******************************************************************************
 *
 * @brief   Decides whether to stay in MSPBoot or if device should jump to App
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
    if ((TI_MSPBoot_AppMgr_ValidationIsForced() == TRUE_t) && 
        (TI_MSPBoot_AppMgr_AppisValid() == FALSE_t))
    {
        TI_MSPBoot_AppMgr_ForceValidation(); //this resets validation check
        *(uint16_t *) (APP_CHECKSUM_ADDR) = 0x0000; // Reset Application checksum
        return FALSE_t;  // Validation forced and App is not valid
    }
    else
    {
        return TRUE_t; // App is valid or validation not forced
    }
}


/******************************************************************************
 *
 * @brief   Jump to Application 
 *          A Reset is forced in order to validate Application after Reset
 *          Software BOR is used on devices supporting this feature,
 *          other devices use a PUC by an invalid watchdog write
 *          Check HW_RESET_BOR 
 *
 * @return  None
 *****************************************************************************/
void TI_MSPBoot_AppMgr_JumpToApp(void)
{
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
 * @brief   Jump to Application with CRC
 *          A Reset is forced in order to validate Application after Reset
 *          Explicitly calling out a CRC check using a bit in Status and 
 *          Control byte to request Boot mode, writes a password  in PassWd 
 *
 * @return  None
 *****************************************************************************/
void TI_MSPBoot_AppMgr_JumpToApp_WithCRC(void)
{
    PassWd = BSL_PASSWORD;      // Send password
    StatCtrl |= BOOT_CRC_CHECK;   // Set flag to request CRC check
    TI_MSPBoot_AppMgr_JumpToApp();
}


/******************************************************************************
 *
 * @brief   Checks if Boot mode is forced
 *          Boot mode is forced by an application call sending a request and 
 *          password, or by an external event such as a button press
 *
 * @return  TRUE_t Boot mode is forced
 *          FALSE_t Boot mode is not forced
 *****************************************************************************/
#if 0 //not using for our bootloader
static tBOOL TI_MSPBoot_AppMgr_BootisForced(void)
{
    tBOOL ret = FALSE_t;

    // Check if application is requesting Boot mode and password is correct
    if (((StatCtrl & BOOT_APP_REQ) != 0) && (PassWd == BSL_PASSWORD))
    {
        ret = TRUE_t;
    }

    // Check for an external event such as S2 (P1.1) button in Launchpad
    __delay_cycles(10000);   // Wait for pull-up to go high
    //If S2 button (P1.1) is pressed, force BSL
    if (HW_ENTRY_CONDITION)
    {
        ret = TRUE_t;
    }

    // Clear Status and Control byte and password
    PassWd = 0;
    StatCtrl = 0;
    return ret;
}
#endif

/******************************************************************************
 *
 * @brief   Will set Status and Control byte and password for app validation
 *          which cause TI_MSPBoot_AppMgr_ValidationIsForced to return true
 *
 * @return  none
 *****************************************************************************/
static void TI_MSPBoot_AppMgr_ForceValidation()
{
    // Set Status and Control byte and password
    PassWd = 0xC0DE;
    StatCtrl |= BOOT_CRC_CHECK;
}

/******************************************************************************
 *
 * @brief   Checks if CRC validation is forced
 *          CRC validation is forced by the bootloader setting a flag 
 *          and password
 *
 * @return  TRUE_t CRC check is forced
 *          FALSE_t CRC check is not forced
 *****************************************************************************/
static tBOOL TI_MSPBoot_AppMgr_ValidationIsForced(void)
{
    tBOOL ret = FALSE_t;

    // Check if bootloader set flag and password correct for CRC check
    if (((StatCtrl & BOOT_CRC_CHECK) != 0) && (PassWd == BSL_PASSWORD))
    {
        ret = TRUE_t;
    }
    
    //force if reset vector is invalid
    if (*(volatile uint16_t *)(&_App_Reset_Vector) == 0xFFFF)
    {
        ret = TRUE_t;
    }

    // Clear Status and Control byte and password
    PassWd = 0;
    StatCtrl = 0;
    return ret;
}

/******************************************************************************
 *
 * @brief   Checks for update image
 *          CRC validation is forced by the bootloader setting a flag 
 *          and password
 *
 * @return  TRUE_t CRC image found and does not match current app
 *          FALSE_t CRC no image found or crc matches current app
 *****************************************************************************/
tBOOL TI_MSPBoot_AppMgr_CheckForUpdate(void)
{
    uint16_t imageCRC;
    
    extern uint16_t _AppChecksum;   // Address of Application checksum
    
//    if(FAT_Update_ValidImageFound() != TRUE_t) {
//      return FALSE_t;
//    }
    
    //check update image crc against current app crc
    if(FAT_Update_ValidImageFound()== FR_OK) {
   if( (TI_TXT_GetShortAtAddress((uint16_t) &_AppChecksum, &imageCRC) == RET_OK) &&
        (imageCRC != _AppChecksum) )
    {
            return TRUE_t; //
    }
    else
    {
            return FALSE_t; //image did not have CRC or App CRC matched image CRC
    }
    } else
      return FALSE_t;
}

/******************************************************************************
 *
 * @brief   Program App memory from update image
 *          This will automatically set the app validation flag
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_AppMgr_ProgramApp(void)
{
    TI_MSPBoot_MI_EraseApp();  // Erase Application area

#ifdef _FISM_
   if(image_store_mcu() == Success)
    {
       write_image_to_flash(0x1940,0xFF);
       write_image_to_flash(0x1930,0xFF); 
       write_image_to_flash(0x1980,0xFF); 
       P4DIR &= ~BIT7;
    }
  
#else

    TI_MSPBoot_AppMgr_ForceValidation();
    TI_TXT_Program();
#endif
}
