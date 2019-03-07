/*
 * \file   TI_MSPBoot_CI_NWK_APP_SMBus.c
 *
 * \brief  Implementation of Network layers of SMBus protocol
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
#   warning "It's recommended to use MCLK>=4Mhz with SMBus-type protocol"
#endif
#ifndef MSPBoot_CI_I2C
#   warning "This file was tested on I2C interface"
#endif

//
// Macros and definitions
//
#ifndef __IAR_SYSTEMS_ICC__
// This keyword doesn't exist in CCS and is only used with IAR.
#   define __no_init
#endif
/*! MSPBoot version sent as response of COMMAND_TX_VERSION command  */
#define MSPBoot_VERSION             0xC1
/*! Maximum size of data payload =  1 Command byte + 32 data + 1 Byte Count + 1 PEC*/
#define RX_PAYLOAD_MAX_SIZE         (32+1+1+1)
/*! Define if SMBus Packet Error Check is supported */
#define SMB_PEC_SUPPORTED
/*! NACK packets when MSP430 is busy processing command */
//#define SMB_NACK_WHILE_BUSY


//  MSPBoot SMbus Commands , check description of each command below
/*! Commands with BIT7 set will have PEC enabled */
#define     PEC_CMD                 BIT7
/*! Supported commands */
typedef enum
{
    CMD_TX_VER=1,       /*! Request MSPBoot version */
    CMD_ERASE_APP,      /*! Erase application area */
    CMD_ERASE_SEGMENT,  /*! Erase a segment */
    CMD_RX_DATA_BLOCK,  /*! Program a block of data */
    CMD_JUMP2APP,       /*! Jump to Application */
} tSMBUS_CMD_LIST;

// Description of all commands
/*! Request MSPBoot Version -
    SEND BYTE
    ADDR(W)     0x01
    SEND BYTE w/PEC
    ADDR(W)     0x81    PEC
  */

/*! Erase application area -
    SEND BYTE
    ADDR(W)     0x02
    SEND BYTE w/PEC
    ADDR(W)     0x82    PEC
  */

/*! Erase a segment
    WRITE WORD
    ADDR(W)     0x03    ADDR_L  ADDR_H
    WRITE WORD w/PEC
    ADDR(W)     0x83    ADDR_L  ADDR_H  PEC
  */

/*! Program a block of data
    BLOCK WRITE
    ADDR(W)     0x04    BYTE_COUNT  DATA1   DATA2 .. DATAN
    BLOCK WRITE w/PEC
    ADDR(W)     0x04    BYTE_COUNT  DATA1   DATA2 .. DATAN  PEC
  */

/*! Jump to Application
    SEND BYTE
    ADDR(W)     0x05
    SEND BYTE w/PEC
    ADDR(W)     0x85    PEC
  */

/*! Request response
    RECEIVE BYTE
    ADDR(R)     Response (OK, Error, Version)
  */

/*! Structure with SmBus packet info */
typedef struct  {
    uint8_t RxCounter;                      /* number of received bytes*/
    uint8_t RxPacket[RX_PAYLOAD_MAX_SIZE];  /* RX buffer */
    uint8_t Pec;                            /* CRC8/PEC */
    uint8_t TxByte;                         /* Byte to be tx on request */
}t_SMB_packet;

//
//  MSPBoot SMBus-based responses
//
#define RESPONSE_OK                     0x00    /*! Command processed OK */
#define RESPONSE_NTR                    0x50    /*! Nothing to respond */
#define RESPONSE_PEC_ERROR              0x52    /*! Packet checksum error */
#define RESPONSE_PACKETSIZE_ERROR       0x53    /*! Packet size is invalid */
#define RESPONSE_UNKNOWN_ERROR          0x54    /*! Error in protocol */
#define RESPONSE_INCORRECT_FORMAT       0x55    /*! Incorrect format in protocol */
#define RESPONSE_INCORRECT_COMMAND      0x56    /*! Invalid command */
#define RESPONSE_MEM_ERROR              0x57    /*! Memory Area error */

//
//
//  Global variables
//
/*! Communication Status byte:
 *   BIT1 = COMM_PACKET_RX = Packet received
 *   BIT3 = COMM_ERROR = Error in communication protocol
 */
__no_init static uint8_t CommStatus;
/*! SMbus packet with RX and TX data */
t_SMB_packet SMB_Packet;

//
//  Local function prototypes
//
static void Start_Callback(void);
static void Rx_Callback(uint8_t data);
static void Tx_Callback(uint8_t * data);
static void Stop_Callback(void);
static void Error_Callback(uint8_t data);

static uint8_t CI_CMD_Intepreter(t_SMB_packet * SMB_pkt);
static uint8_t CI_CMD_Rx_Data_Block(uint16_t addr, uint8_t *data, uint8_t len);

/*! Callback structure used for I2C SMbus-based protocol
 *   RX, TX, Error, Start and Stop callback functions are implemented 
 */
static const t_CI_Callback CI_Callback_s =
{
    Rx_Callback,
    Tx_Callback,
    Error_Callback,
    Start_Callback,
    Stop_Callback,
};

/******************************************************************************
 *
 * @brief   Initialize the Communication Interface
 *  All layers will be initialized
 *
 * @return none
 *****************************************************************************/
void TI_MSPBoot_CI_Init(void)
{
    CommStatus = 0;
    SMB_Packet.TxByte = RESPONSE_UNKNOWN_ERROR;
    TI_MSPBoot_CI_PHYDL_Init((t_CI_Callback *) &CI_Callback_s);
}

/******************************************************************************
 *
 * @brief   On packet reception, process the data
 *  SMBus-based protocol expects:
 *  CMD         = 1 byte with the corresponding command
 *  ByteCount   = 1 optional byte with size of Data
 *  DATA        = optional data depending on command (1-32 bytes)
 *  PEC         = 1 byte with CRC-8 checksum 
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
        ret = CI_CMD_Intepreter(&SMB_Packet);
#ifdef SMB_NACK_WHILE_BUSY        
        TI_MSPBoot_CI_PHYDL_reenable();
#endif
        CommStatus &= ~COMM_PACKET_RX;
    }
    return ret;
}


/******************************************************************************
 *
 * @brief   Process a packet checking the command and sending a response
 *  New commands can be added in this switch statement
 *
 * @param SMB_pkt Pointer to SMBus packet containing RX/TX info
 *
 * @return RET_OK: Communication protocol in progress
 *         RET_JUMP_TO_APP: Last byte received, request jump to application
 *         RET_PARAM_ERROR: Incorrect command
 *****************************************************************************/
static uint8_t CI_CMD_Intepreter(t_SMB_packet * SMB_pkt)
{
    uint8_t ret = RET_OK;

    switch ((SMB_pkt->RxPacket[0])& 0x7F)   // Check command without PEC bit
    {
        case CMD_ERASE_APP:
            // Erase the application area
            TI_MSPBoot_MI_EraseApp();
            SMB_Packet.TxByte = RESPONSE_OK;
        break;
        case CMD_RX_DATA_BLOCK:
            // Receive and program a data block specified by an address
            SMB_Packet.TxByte = CI_CMD_Rx_Data_Block(SMB_pkt->RxPacket[2]+(SMB_pkt->RxPacket[3]<<8),
                                                        &SMB_pkt->RxPacket[4], SMB_pkt->RxPacket[1]-2);
        break;
        case CMD_ERASE_SEGMENT:
            // Erase an application area sector as defined by the address
            if (TI_MSPBoot_MI_EraseSector(SMB_pkt->RxPacket[2]+(SMB_pkt->RxPacket[3]<<8)) == RET_OK)
                 SMB_Packet.TxByte = RESPONSE_OK;
            else
                SMB_Packet.TxByte = RESPONSE_MEM_ERROR;
        break;
        case CMD_TX_VER:
            // Transmit MSPBoot version
            SMB_Packet.TxByte = MSPBoot_VERSION;
        break;
        case CMD_JUMP2APP:
            // Jump to Application
            ret = RET_JUMP_TO_APP;
        break;
        default:
            SMB_Packet.TxByte = RESPONSE_INCORRECT_COMMAND;
            ret = RET_PARAM_ERROR;
        break;
    }
    return ret;
}

/******************************************************************************
 *
 * @brief   Programs a block of data to memory
 *
 * @param addr  Start address (16-bit) or area being programmed
 * @param data  Pointer to data being written
 * @param len   Lenght of data being programmed
 *
 * @return  RESPONSE_OK: Result OK
 *          RESPONSE_MEM_ERROR: Error writing the data
 *****************************************************************************/
static uint8_t CI_CMD_Rx_Data_Block(uint16_t addr, uint8_t *data, uint8_t len)
{
    uint8_t i;
    for (i=0; i < len; i++)
    {
        if ( TI_MSPBoot_MI_WriteByte(addr++, data[i]) != RET_OK)
        {
            return RESPONSE_MEM_ERROR;
        }
    }
    return RESPONSE_OK;
}

/******************************************************************************
 *
 * @brief   Start callback for SMBus-based protocol
 *  Initializes the counter and Status
 *
 * @return  none
 *****************************************************************************/
#ifndef CONFIG_CI_PHYDL_START_CALLBACK
    #error "This Comm Interface Network/App layer requires the Start Callback"
#endif
static void Start_Callback(void)
{
    SMB_Packet.RxCounter = 0;
    CommStatus &= ~COMM_ERROR;  // Clear error flag
}

/******************************************************************************
 *
 * @brief   RX Callback for SMBus-based protocol
 *
 * @param data  Byte received from Master
 *
 * @return  none
 *****************************************************************************/
static void Rx_Callback(uint8_t data)
{
    SMB_Packet.TxByte = RESPONSE_INCORRECT_FORMAT;

    // If there's no error in current packet
    if (SMB_Packet.RxCounter < RX_PAYLOAD_MAX_SIZE)
    {
        SMB_Packet.RxPacket[SMB_Packet.RxCounter] = data;
        SMB_Packet.RxCounter++;
    }
    else
    {
        SMB_Packet.TxByte = RESPONSE_PACKETSIZE_ERROR;
        CommStatus |= COMM_ERROR;
    }
}

/******************************************************************************
 *
 * @brief   TX Callback for SMBus-based protocol
 *
 * @param data  Written with Byte being sent to master on RX Request
 *
 * @return  none
 *****************************************************************************/
static void Tx_Callback(uint8_t * data)
{
    // A single char is sent as a response
    *data = SMB_Packet.TxByte;
    SMB_Packet.TxByte = RESPONSE_NTR;
}

/******************************************************************************
 *
 * @brief   Stop callback for SMBus-based protocol
 *  Initializes the counter and Status
 *
 * @return  none
 *****************************************************************************/
#ifndef CONFIG_CI_PHYDL_STOP_CALLBACK
    #error "This Comm Interface Network/App layer requires the Stop Callback"
#endif
static void Stop_Callback(void)
{
    uint8_t i;

    // If there was a communication error, return and don't process packet
    if (CommStatus & COMM_ERROR)
        return; 

    if (SMB_Packet.RxCounter != 0x00)
    {
#ifdef SMB_NACK_WHILE_BUSY
        TI_MSPBoot_CI_PHYDL_disable();       // NACK until we have processed the command
#endif
        #ifdef SMB_PEC_SUPPORTED
        // Packet with PEC should have at least more than 1 byte (CMD + PEC)
        if ((SMB_Packet.RxCounter > 1) && (SMB_Packet.RxPacket[0] & PEC_CMD))
        {
            // Calculate PEC
            SMB_Packet.Pec = 0x00;
            // Add the address +R/W byte
            crc8_add(&SMB_Packet.Pec, I2C_SLAVE_ADDR_W);
            for (i=0; i < (SMB_Packet.RxCounter-1); i++)
            {
              // Add all data to PEC
               crc8_add(&SMB_Packet.Pec, SMB_Packet.RxPacket[i]);
            }
            // Compare PEC
            if (SMB_Packet.Pec != SMB_Packet.RxPacket[SMB_Packet.RxCounter -1])
            {
                SMB_Packet.TxByte = RESPONSE_PEC_ERROR;
#ifdef SMB_NACK_WHILE_BUSY
                TI_MSPBoot_CI_PHYDL_reenable();
#endif
                return;
            }
        }
        #endif

        CommStatus |= COMM_PACKET_RX;   // Packet was received
    }
}

/******************************************************************************
 *
 * @brief   Error Callback for SMBus protocol
 *
 * @param data  PHYDL layer error code:
 *              1: Timeout
 *
 * @return  none
 *****************************************************************************/
#ifndef CONFIG_CI_PHYDL_ERROR_CALLBACK
    #error "This Comm Interface Network/App layer requires the Error Callback"
#endif
static void Error_Callback(uint8_t error)
{
    if (error == 1)
    {
        TI_MSPBoot_CI_Init();
    }

}



