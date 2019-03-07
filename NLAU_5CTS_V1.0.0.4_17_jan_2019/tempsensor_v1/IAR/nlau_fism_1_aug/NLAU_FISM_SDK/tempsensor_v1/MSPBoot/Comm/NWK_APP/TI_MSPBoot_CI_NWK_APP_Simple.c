/*
 * \file   TI_MSPBoot_CI_NWK_APP_Simple.c
 *
 * \brief  Implementation of Network layers of Simple protocol 
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
#include "TI_MSPBoot_MI.h"
#include "crc.h"

//
//  Configuration checks
//
#ifndef MSPBoot_CI_I2C
#   warning "This file was tested on I2C interface"
#endif


//
// Macros and definitions
//
/*! Response character will be sent upon Master RX request. It's hardcoded */
#define RESPONSE_CHAR           0xB1
/*! SYNC character, starts the transfer, triggering an application area erase and
    should be followed by the receiption of BYTE0-BYTE1-...-BYTEn */
#define SYNC_CHAR               0x55

#ifndef __IAR_SYSTEMS_ICC__
// This keyword doesn't exist in CCS and is only used with IAR.
#   define __no_init
#endif

//
//  Global variables
//
/*! Received byte */
__no_init static uint8_t RxByte;           

/*! Communication Status byte:
    BIT1 = COMM_PACKET_RX = Packet received
    BIT3 = COMM_ERROR = Error in communication protocol
*/
__no_init static uint8_t CommStatus;

/*! Indicates if SYNC byte has been received */
static uint8_t sync_received;               

//
//  Local function prototypes
//
static void Rx_Callback(uint8_t data);
static void Tx_Callback(uint8_t * data);

/*! Callback structure used for Simple protocol
 *   RX, TX callback functions are implemented 
 */
static const t_CI_Callback CI_Callback_s =
{
    Rx_Callback,
    Tx_Callback,
    // ErrorCallback not enabled 
    // StartCallback not enabled
    // StopCallback not enabled
};

/******************************************************************************
 *
 * @brief   Initialize the Communication Interface
 *  Lower layers will be initialized
 *
 * @return none
 *****************************************************************************/
void TI_MSPBoot_CI_Init(void)
{
    sync_received = 0;
    CommStatus = 0;
    TI_MSPBoot_CI_PHYDL_Init((t_CI_Callback *)&CI_Callback_s);
}

/******************************************************************************
 *
 * @brief   On packet reception, process the data
 *  Simple protocol doesn't have a Network layer and it expects:
 *  SYNC_CHAR: triggers app erase
 *  Application image from Byte0 to ByteN (i.e. if Application resides in E000-FDFF
 *      the first byte corresponds to E000, and the last byte (7680) is FDFF
 *  After the last byte, the MCU will reset and go out to Application (if valid)
 *
 *  No response is expected for each command. The response is hardcoded 
 *
 * @note   This routine will request exit to application after the last byte 
 *         reception
 *
 * @return RET_OK: Communication protocol in progress
 *         RET_JUMP_TO_APP: Last byte received, request jump to application
 *****************************************************************************/
volatile uint16_t address;
uint8_t TI_MSPBoot_CI_Process(void)
{
    if (CommStatus & COMM_PACKET_RX)       // Check for packet reception
    {
        if (sync_received == 0)
       {   // If SYNC char hasn't been received check for this byte
            if (RxByte == SYNC_CHAR)
            {   // If the received byte is the SYNC_CHAR, wait for data
                sync_received = 1;
                TI_MSPBoot_MI_EraseApp();  // Erase Application area
                address = APP_START_ADDR;  // First received byte goes to App Start address
            }
        }
        else
        {
            // If SYNC_CHAR was received, we are expecting to receive data from App image
            TI_MSPBoot_MI_WriteByte(address++, RxByte); // Write one byte at a time and
                                                        //  increase the address pointer
            if (address > APP_END_ADDR)
            {
                // If last byte was written, jump to App
                return RET_JUMP_TO_APP;
            }
        }
    }
    // Clear packet reception
    CommStatus &= ~COMM_PACKET_RX;

    return RET_OK;

}



/******************************************************************************
 *
 * @brief   RX Callback for Simple protocol
 *  Simple protocol is byte-based and each byte is processed as received
 *
 * @param data  Byte received by Peripheral interface
 *
 * @return none
 *
 *****************************************************************************/
void Rx_Callback(uint8_t data)
{
    // In Simple Protocol, each byte is processed
    CommStatus |= COMM_PACKET_RX;
    RxByte = data;
}

/******************************************************************************
 *
 * @brief   TX Callback for Simple protocol
 *  Simple protocol sends the version when requested 
 *  Then it sends the version+1 after mass erase
 *  And version+2 before jumping to Application
 *
* @param data  Written with Byte being sent to master on RX Request
 *
 * @return none
 *
 *****************************************************************************/
void Tx_Callback(uint8_t *data)
{
    // A single char is sent as a response
    *data = RESPONSE_CHAR;
}

