/*
 * \file   TI_MSPBoot_CI_NWK_APP_BSL.c
 *
 * \brief  Implementation of Network and App layer of BSL-based protocol 
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
#if (MCLK==1000000)
#   warning "It's recommended to use MCLK>=4Mhz with BSL-type protocol"
#endif
#ifndef MSPBoot_CI_I2C
#   warning "This file was tested on I2C interface"
#endif


//
// Macros and definitions - Network Layer
//
#ifndef __IAR_SYSTEMS_ICC__
// This keyword doesn't exist in CCS and is only used with IAR.
#   define __no_init
#endif
/*! Maximum size of data payload =  16 data + 1 CMD + 2 ADDR */
#define PAYLOAD_MAX_SIZE            (16+1+2)
/*! MSPBoot Packet header */
#define HEADER_CHAR                 (0x80)

// MSPBoot Network layer responses 
#define RESPONSE_NWK_HEADER_ERROR       (0x51)  /*! Incorrect header        */
#define RESPONSE_NWK_CHECKSUM_ERROR     (0x52)  /*! Packet checksum error   */
#define RESPONSE_NWK_PACKETZERO_ERROR   (0x53)  /*! Packet size is zero     */
#define RESPONSE_NWK_PACKETSIZE_ERROR   (0x54)  /*! Packet size is invalid  */
#define RESPONSE_NWK_UNKNOWN_ERROR      (0x55)  /*! Error in protocol       */

/*! MSPBoot version sent as response of COMMAND_TX_VERSION command  */
#define MSPBoot_VERSION                 (0xA0)

//
// Macros and definitions - Application layer
//
// Supported commands in BSL-based protocol
/*! Erase a segment :
    0x80    LEN=0x03    0x12    ADDRL   ADDRH   CHK_L   CHK_H   */
#define COMMAND_ERASE_SEGMENT           (0x12)
/*! Erase application area:
    0x80    LEN=0x01    0x15    CHK_L   CHK_H   */
#define COMMAND_ERASE_APP               (0x15)
/*! Receive data block :
    0x80    LEN=3+datapayload   0x10    ADDRL   ADDRH   DATA0...DATAn   CHK_L   CHK_H   */
#define COMMAND_RX_DATA_BLOCK           (0x10)
/*! Transmit MSPBoot version :
    0x80    LEN=0x01    0x19    CHK_L   CHK_H   */
#define COMMAND_TX_VERSION              (0x19)
/*! Jump to application:
    0x80    LEN=0x01    0x1C    CHK_L   CHK_H   */
#define COMMAND_JUMP2APP                (0x1C)

//  MSPBoot Application layer responses
#define RESPONSE_APP_OK                 (0x00)  /*! Command processed OK    */
#define RESPONSE_APP_INVALID_PARAMS     (0xC5)  /*! Invalid parameters      */
#define RESPONSE_APP_INCORRECT_COMMAND  (0xC6)  /*! Invalid command         */


//
//  Global variables
//
/*! Communication Status byte:
 *  BIT1 = COMM_PACKET_RX = Packet received
 *  BIT3 = COMM_ERROR = Error in communication protocol
 */
__no_init uint8_t CommStatus;
__no_init uint8_t TxByte;                       /*! Byte sent as response to Master */
__no_init uint8_t RxPacket[PAYLOAD_MAX_SIZE];   /*! Data received from Master */
__no_init static uint8_t counter;               /*! Data counter */
__no_init static uint8_t Len;                   /*! Data lenght */

//
//  Local function prototypes
//
extern uint8_t CI_CMD_Intepreter(uint8_t *RxData, uint8_t RxLen, uint8_t *TxData);
static void CI_NWK_Start_Callback(void);
static void CI_NWK_Rx_Callback(uint8_t data);
static void CI_NWK_Tx_Callback(uint8_t * data);
static uint8_t CI_CMD_Rx_Data_Block(uint16_t addr, uint8_t *data, uint8_t len);


/*! Callback structure used for I2C BSL-based 
 *   RX, TX and Start callback functions are implemented 
 */
static const t_CI_Callback CI_Callback_s =
{
    CI_NWK_Rx_Callback,
    CI_NWK_Tx_Callback,
    NULL,                   // Error callback not implemented in this protocol
    CI_NWK_Start_Callback,
    NULL                    // Stop callback not implemented in this protocol
};

/******************************************************************************
 *
 * @brief   Initialize the Communication Interface
*  Lower layers will also be initialized
 *
 * @return none
 *****************************************************************************/
void TI_MSPBoot_CI_Init(void)
{
    CommStatus = 0;
    TxByte = RESPONSE_NWK_UNKNOWN_ERROR;
    TI_MSPBoot_CI_PHYDL_Init((t_CI_Callback *) &CI_Callback_s);
}

/******************************************************************************
 *
 * @brief   On packet reception, process the data
 *  BSL-based protocol expects:
 *  HEADER  = 0x80
 *  Lenght  = lenght of CMD + [ADDR] + [DATA]
 *  CMD     = 1 byte with the corresponding command
 *  ADDR    = optional address depending on command
 *  DATA    = optional data depending on command
 *  CHKSUM  = 2 bytes (L:H) with CRC checksum of CMD + [ADDR] + [DATA]
 *
 * @return RET_OK: Communication protocol in progress
 *         RET_JUMP_TO_APP: Last byte received, request jump to application
 *         RET_PARAM_ERROR: Incorrect command
 *****************************************************************************/
uint8_t TI_MSPBoot_CI_Process(void)
{
    uint8_t ret = RET_OK;

    if (CommStatus & COMM_PACKET_RX)    // On complete packet reception
    {
        ret = CI_CMD_Intepreter(RxPacket, Len, &TxByte);
    }
    // Clear packet reception
    CommStatus &= ~COMM_PACKET_RX;
    return ret;
}


/******************************************************************************
 *
 * @brief   Process a packet checking the command and sending a response
 *  New commands can be added in this switch statement
 *
 * @param RxData Pointer to buffer with received data including command
 * @param RxLen  Lenght of Received data
 * @param TxData Pointer to buffer which will be updated with a response
 *
 * @return RET_OK: Communication protocol in progress
 *         RET_JUMP_TO_APP: Last byte received, request jump to application
 *         RET_PARAM_ERROR: Incorrect command
 *****************************************************************************/
uint8_t CI_CMD_Intepreter(uint8_t *RxData, uint8_t RxLen, uint8_t *TxData)
{
    switch (RxData[0])
    {
        case COMMAND_ERASE_APP:
            // Erase the application area
            TI_MSPBoot_MI_EraseApp();
            *TxData = RESPONSE_APP_OK;
        break;
        case COMMAND_RX_DATA_BLOCK:
            // Receive and program a data block specified by an address
            *TxData = CI_CMD_Rx_Data_Block(RxData[1]+(RxData[2]<<8), &RxData[3], RxLen-3);
        break;
        case COMMAND_ERASE_SEGMENT:
            // Erase an application area sector as defined by the address
            if (TI_MSPBoot_MI_EraseSector(RxData[1]+(RxData[2]<<8)) == RET_OK)
            {
                *TxData = RESPONSE_APP_OK;
            }
            else
            {
                *TxData = RESPONSE_APP_INVALID_PARAMS;
            }
        break;
        case COMMAND_TX_VERSION:
            // Transmit MSPBoot version
            *TxData = MSPBoot_VERSION;
        break;
        case COMMAND_JUMP2APP:
            // Jump to Application
            return RET_JUMP_TO_APP;
        //break;
        default:
            *TxData = RESPONSE_APP_INCORRECT_COMMAND;
            return RET_PARAM_ERROR;
        //break;
    }

    return RET_OK;
}


/******************************************************************************
 *
 * @brief   Programs a block of data to memory
 *
 * @param addr  Start address (16-bit) or area being programmed
 * @param data  Pointer to data being written
 * @param len   Lenght of data being programmed
 *
 * @return  RESPONSE_APP_OK: Result OK
 *          RESPONSE_APP_INVALID_PARAMS: Error writing the data
 *****************************************************************************/
static uint8_t CI_CMD_Rx_Data_Block(uint16_t addr, uint8_t *data, uint8_t len)
{
    uint8_t i;
    for (i=0; i < len; i++)
    {
        if ( TI_MSPBoot_MI_WriteByte(addr++, data[i]) != RET_OK)
        {
            return RESPONSE_APP_INVALID_PARAMS;
        }
    }

    return RESPONSE_APP_OK;
}

/******************************************************************************
 *
 * @brief   Start callback for BSL-based protocol
 *  Initializes the counter and Status
 *
 * @return  none
 *****************************************************************************/
#ifndef CONFIG_CI_PHYDL_START_CALLBACK
    #error "This Comm Interface Network/App layer requires the Start Callback"
#endif
void CI_NWK_Start_Callback(void)
{
    counter = 0;
    CommStatus &= ~COMM_ERROR;
}

/******************************************************************************
 *
 * @brief   RX Callback for BSL-based protocol
 *
 * @param data  Byte received from Master
 *
 * @return  none
 *****************************************************************************/
void CI_NWK_Rx_Callback(uint8_t data)
{
    __no_init static uint16_t Checksum;

    if ((CommStatus & COMM_ERROR) == 0)     // If there's no error in current packet
    {
        if (counter == 0)
        {
            // Byte 0 = Header
            TxByte = RESPONSE_NWK_UNKNOWN_ERROR;    // Initial response if packet is incomplete
            // Check header
            if (data !=HEADER_CHAR)
            {
                // Incorrect header
                CommStatus |= COMM_ERROR;
                TxByte =  RESPONSE_NWK_HEADER_ERROR;    // Send as response to Master
            }
        }
        else if (counter == 1)
        {
            // Byte 1 = Len (max 255)
            Len = data;
            if (data == 0)
            {
                // Size = 0
                CommStatus |= COMM_ERROR;
                TxByte =  RESPONSE_NWK_PACKETZERO_ERROR;    // Send as response to Master
            }
            else if(data > PAYLOAD_MAX_SIZE)
            {
                // Size too big
                CommStatus |= COMM_ERROR;
                TxByte =  RESPONSE_NWK_PACKETSIZE_ERROR;    // Send as response to Master
            }
        }
        else if (counter < (Len+2))
        {
            // Payload (optional address + data)
            RxPacket[counter-2] = data;
        }
        else if (counter == (Len+2))
        {
            // Byte n-1 = Expected checksum_Low
            Checksum = data;
        }
        else if (counter == (Len+3))
        {
            // Byte n = Expected checksum_High
            Checksum += (data<<8);
            // Calculate checksum and compare with received
            if (crc16MakeBitwise(RxPacket, Len) != Checksum)
            {
                // Incorrect checksum
                CommStatus |= COMM_ERROR;
                TxByte =  RESPONSE_NWK_CHECKSUM_ERROR;  // Send as response to Master
            }
            else
            {
                CommStatus |= COMM_PACKET_RX;
            }
        }
        counter++;
    }
}

/******************************************************************************
 *
 * @brief   TX Callback for BSL-based protocol
 *
 * @param data  Written with Byte being sent to master on RX Request
 *
 * @return  none
 *****************************************************************************/
void CI_NWK_Tx_Callback(uint8_t * data)
{
    // A single char is sent as a response
    *data = TxByte;
}


