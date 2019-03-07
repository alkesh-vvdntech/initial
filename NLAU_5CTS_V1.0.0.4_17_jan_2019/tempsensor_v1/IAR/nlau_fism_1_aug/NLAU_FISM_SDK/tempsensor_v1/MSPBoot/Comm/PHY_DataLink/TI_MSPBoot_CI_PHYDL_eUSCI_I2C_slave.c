/*
 * \file   TI_MSPBoot_CI_PHYDL_eUSCI_I2C_slave.c
 *
 * \brief  Driver for I2C Slave Physical-DataLink layers using eUSCI
 *
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
#include "TI_MSPBoot_CI.h"
#include "TI_MSPBoot_AppMgr.h"

//
//  Configuration checks
//
#ifndef MSPBoot_CI_I2C
#   error "This file uses the I2C interface"
#endif
/*! Define to enable workaround for USCI37 bug */
#define eUSCI_BUG_WORKAROUND

//
//  Type Definitions
//
/*! State machine used by this communication interface 
 *   eUSCI doesn't require too much control, so we only check for an idle state
 *   and when receiving a packet 
 */
typedef enum {
    USCI_STATE_IDLE=0,          /*! Initialized state waiting for start */
    USCI_STATE_RECEIVING,       /*! Receiving packet */
}USCI_State_Machine_e;

//
//  Global variables
//
//    Note that these variables are assigned to a specific location in RAM
//    since they can also be used during application.
//    If not used by application, the location doesn't matter.
//
/*! Pointer to the Communication Interface callback structure 
 *   The NWK layer will define its callback structure and pass the pointer to
 *   this layer. An application can also declare its own structure and pass it
 *   to this layer in order to use the same interface.
 *   Note that the declaration for IAR and CCS is different.
 */
#ifdef __IAR_SYSTEMS_ICC__
#pragma location="RAM_CICALLBACK"
__no_init t_CI_Callback  * CI_Callback_ptr;
#elif defined (__TI_COMPILER_VERSION__)
extern t_CI_Callback  * CI_Callback_ptr;
#endif


/*! State machine used by this interface. 
 *   Note that the declaration for IAR and CCS is different.
 */
#ifdef __IAR_SYSTEMS_ICC__
#pragma location="RAM_CISM"
__no_init USCI_State_Machine_e CI_State_Machine;
#elif defined (__TI_COMPILER_VERSION__)
extern USCI_State_Machine_e CI_State_Machine;
#endif


//
//  Function declarations
//
/******************************************************************************
*
 * @brief   Initializes the eUSCI I2C Slave interface
 *  - Sets corresponding GPIOs for I2C functionality
 *  - Resets and then configures the I2C
 *  - Initializes the I2C state machine
 *  - Initializes the I2C callbacks
 *  The NWK layer will define its callback structure and pass the pointer to
 *   this function. An application can also declare its own structure and call
 *   this function in order to use the same interface.
 *
 *  @param  CI_Callback     Pointer to Communication interface callbacks
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_Init(t_CI_Callback * CI_Callback)
{
    P3SEL1 |= (BIT5 | BIT6);                 // Enable P3[5:6] for eUSCI_B0 I2C mode
    P3SEL0 &= ~(BIT5 | BIT6);                //

    UCB0CTLW0 |= UCSWRST ;	                 //Software reset enabled
    UCB0CTLW0 |= UCMODE_3  + UCSYNC;	     //I2C mode, sync mode
#ifdef CONFIG_CI_PHYDL_I2C_TIMEOUT
    UCB0CTLW1 |= UCCLTO_2;                   // Enable Clock timeout 
#endif
    UCB0I2COA0 = CONFIG_CI_PHYDL_I2C_SLAVE_ADDR +UCOAEN; // set own (slave) address
    UCB0CTLW0 &=~UCSWRST;	                 //clear reset register

    // Initialize all callbacks
    CI_Callback_ptr = CI_Callback;
    // Init state machine
    CI_State_Machine = USCI_STATE_IDLE;
}

/******************************************************************************
 *
 * @brief   Disables the eUSCI module
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_disable(void)
{
    UCB0CTLW0 |= UCSWRST;
}

/******************************************************************************
 *
 * @brief   Enables the eUSCI module
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_reenable(void)
{
    UCB0CTLW0 &= ~UCSWRST;
}

/******************************************************************************
*
 * @brief   Polls for eUSCI flags
 *  - Checks the Start, RX, TX, Stop flags and Timeout timer
 *  - This function calls the corresponding Callback functions and informs 
 *    higher level layers, so they can handle data properly
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_Poll(void)
{
    uint8_t temp;
    uint16_t flag_stat;
    
    // Read flags at the beggining of function in order to avoid race conditions
    // New detected flags will be handled in the next poll
    flag_stat = UCB0IFG;

    if (flag_stat & UCSTTIFG)                // Check for Start bit flag
    {
#ifdef CONFIG_CI_PHYDL_STOP_CALLBACK
        if ((CI_State_Machine != USCI_STATE_IDLE) && 
            (CI_Callback_ptr->StopCallback != NULL))
        {
            // Special case when a stop was cleared by a new Start
            UCB0IFG &= ~UCSTPIFG;               // Clear stop flag
            CI_Callback_ptr->StopCallback();  // Call Stop callback (if valid)
            CI_State_Machine = USCI_STATE_IDLE; // Reset state machine
            return;
        }
#endif            
        // If no stop was expected/used, process the start flag
#ifdef CONFIG_CI_PHYDL_START_CALLBACK
        if (CI_Callback_ptr->StartCallback != NULL)  
        {
            CI_Callback_ptr->StartCallback(); // Call Start callback (if valid)
        }
#endif
        CI_State_Machine = USCI_STATE_RECEIVING; // Update state machine
        UCB0IFG &= ~UCSTTIFG;                    // Clear start flag
    }
    else if (flag_stat & UCRXIFG0)  // Check the RX flag
    {
        temp = UCB0RXBUF;                       // Get received byte
#ifdef eUSCI_BUG_WORKAROUND
        temp = UCB0RXBUF;
#endif
        if (CI_Callback_ptr->RxCallback != NULL)
        {
            // Call RX Callback (if valid) and send byte to upper layer
            CI_Callback_ptr->RxCallback(temp);      
        }
    }
    else if (flag_stat & UCTXIFG0)      // Check TX flag
    {
        // Send ACK after byte reception
        if (CI_Callback_ptr->TxCallback != NULL)
        {
            // Call TXCallback (if valid) and get byte to send from upper layer
            CI_Callback_ptr->TxCallback(&temp);     
            UCB0TXBUF = temp;                       // Send byte 
        }
    }
    else if (flag_stat & UCSTPIFG)                // Check for Stop bit flag
    {
        UCB0IFG &= ~UCSTPIFG;              // Clear start flag
#ifdef CONFIG_CI_PHYDL_STOP_CALLBACK
        if (CI_Callback_ptr->StopCallback != NULL)
        {
            CI_Callback_ptr->StopCallback();      // Call End callback (if valid)
        }
#endif
        CI_State_Machine = USCI_STATE_IDLE;         // Reset the state machine
    }
#ifdef CONFIG_CI_PHYDL_TIMEOUT
    else if (flag_stat & UCCLTOIFG )        // Check timeout flag
    {
        UCB0IFG &= ~UCCLTOIFG; 
        //PJDIR |= BIT2;                                // Show timeout detection if desired
        //PJOUT ^= BIT2;
#ifdef CONFIG_CI_PHYDL_ERROR_CALLBACK
        if (CI_Callback_ptr->ErrorCallback != NULL)
        {
            // Call Error callback (if valid) and inform the upper layer
            CI_Callback_ptr->ErrorCallback(1);          
        }
#endif
    }
#endif
}   // TI_MSPBoot_CI_PHYDL_Poll


//
//  Constant table
//
/*! Peripheral Interface vectors for Application:
 *   These vectors are shared with application and can be used by the App to
 *   initialize and use the Peripheral interface
 *   Note that FR57xx has MSP430X, so the pointers used are 32-bit for compatibility
 *   with small/large memory models
 *   Note that the declaration for IAR and CCS is different.
 */
#   ifdef CONFIG_CI_PHYDL_COMM_SHARED
#       ifdef __IAR_SYSTEMS_ICC__
#           pragma location="BOOT_APP_VECTORS"
__root const uint32_t Boot2App_Vector_Table[] =
#       elif defined (__TI_COMPILER_VERSION__)
#           pragma DATA_SECTION(Boot2App_Vector_Table, ".BOOT_APP_VECTORS")
#           pragma RETAIN(Boot2App_Vector_Table)
const uint32_t Boot2App_Vector_Table[] =
#       endif
{
    (uint32_t) &TI_MSPBoot_CI_PHYDL_Init,   /*! Initialization routine */
    (uint32_t) &TI_MSPBoot_CI_PHYDL_Poll    /*! Poll routine */
};
#endif  //CONFIG_CI_PHYDL_COMM_SHARED

