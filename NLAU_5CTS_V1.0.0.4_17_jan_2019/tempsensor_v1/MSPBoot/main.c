/*
* \file   main.c
*
* \brief  Main routine for the bootloader for FR5739
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
/******************************************************************************
*
*  Include files
*
*****************************************************************************/
#include "msp430.h"
#include "TI_MSPBoot_Common.h"
#include "TI_MSPBoot_CI.h"
#include "TI_MSPBoot_MI.h"
#include "TI_MSPBoot_AppMgr.h"
#include "FatFs_Lib.h"
#include "i2c.h"
#include "lcd.h"
#include "debug.h"


#define SPI_CS_L					(P5OUT &= (~BIT7))
#define SPI_CS_H 					(P5OUT |= BIT7)
#define SPI_FLASH_CS_SD_L                               (P7OUT &= ~BIT3)
#define SPI_FLASH_CS_SD_H                               (P7OUT |= BIT3)



/******************************************************************************
*
*  Local function prototypes
*
*****************************************************************************/
static void clock_init(void);
static void HW_init(void);
static void MPU_init(void);

/******************************************************************************
*
*  External function
*
*****************************************************************************/
extern __interrupt void Timer0_A0_ISR(void);
extern __interrupt void USCI_B3_ISR(void);

/******************************************************************************
*
*  Global variable
*
*****************************************************************************/
unsigned char check_boot = 0;

/******************************************************************************
*
* @brief   Main function
*  - Initializes the MCU
*  - Selects whether or not to update via SD card
*  - If update:
*      - Erases program memory
*      - Loads data from UPDATE.TXT
*      - Validates image CRC
*  - If application is valid or validation is not forced:
*      - Jump to application
*  - Else:
*      - Spin on error state
*
* @return  none
*****************************************************************************/
int main_boot( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW + WDTHOLD;

#pragma diag_suppress=Pe767
    //this is a hack
    *((uint16_t *)(0xFF90 + TIMER0_A0_VECTOR)) = (uint16_t)Timer0_A0_ISR;
    *((uint16_t *)(0xFF90 + EUSCI_B3_VECTOR)) = (uint16_t)USCI_B3_ISR;
#pragma diag_default=Pe767

    // Initialize MCU and required peripherals
    HW_init();
    clock_init();
    SPI_Init();
    i2c_init();

    debug_init();
    debug_print("\n\r/**********In Bootloader************************/\n\r");
    __bis_SR_register(GIE);

    lcd_init();
    lcd_printl(LINEC, "CT5Wifi");
    lcd_printl(LINE2, "LOADING");

    /* CHECKING FOR OTA */
    check_boot = *(uint16_t*)0x1980;
    if(check_boot == 0x03)
    {
      TI_MSPBoot_AppMgr_ProgramApp();
    }
    else
    {
      TI_MSPBoot_APPMGR_JUMPTOAPP();
    }
}

/******************************************************************************
*
* @brief   Initializes the MSP430 Clock
*
* @return  none
*****************************************************************************/
static void clock_init(void)
{
  CSCTL0_H = CSKEY_H;                     // Unlock CS registers
  CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
  CSCTL0_H = 0;                           // Lock CS registers
}

/******************************************************************************
*
* @brief   Initializes the basic MCU HW
*
* @return  none
*****************************************************************************/
static void HW_init(void)
{
  // Configure GPIO
  P2OUT &= ~BIT7;                         // Clear P1.0 output latch for a defined power-on state
  P2DIR |= BIT7;                          // Set P1.0 to output direction
  P4OUT &= ~BIT7;                         // Clear P1.0 output latch for a defined power-on state
  P4DIR |= BIT7;
  P2OUT &= ~BIT3;                         // Clear P1.0 output latch for a defined power-on state
  P2DIR |= BIT3;                          // Set P1.0 to output direction
  P4OUT &= ~BIT6;                         // Clear P1.0 output latch for a defined power-on state
  P4DIR |= BIT6;
  //buzzer
  P2OUT &= ~BIT2;                         // Clear P1.0 output latch for a defined power-on state
  P2DIR |= BIT2;                          // Set P1.0 to output direction
  //button
  P2DIR = 0;
  P7DIR = 0;
  P2OUT = 0;
  P7OUT = 0;

  // Configure GPIO for SPI
  P7SEL1 &= ~BIT3;     
  P7SEL0 &= ~BIT3;
  P7DIR |= BIT3;						// SPI CS
  P7OUT |= BIT3;					    // drive SPI CS high to deactive the chip
  P7SEL1 &= ~(BIT0 | BIT1 | BIT2);       // enable SPI CLK, SIMO, SOMI
  P7SEL0 |= (BIT0 | BIT1 | BIT2);       // enable SPI CLK, SIMO, SOMI

  // SMCLK
  P3SEL1 |= BIT4;
  P3SEL0 &= ~BIT4;
  P3DIR |= BIT4;

  // I2C SDA/SCL Selection
  P6SEL0 |= (BIT4| BIT5);
  P6SEL1 &= ~(BIT4| BIT5);
  P6DIR &= ~(BIT5);   //i2c clock as input
  P6DIR |= (BIT4);    //i2c data as output

  //configure LCD backlight
  PJDIR |= BIT6;		// Backlight disable
  PJDIR |= BIT7;
  PJOUT |= BIT6;
  PJOUT &= ~BIT7;
  
   
    
  //wifi
  P4SEL1 &= ~BIT4;
  P4SEL0 &= ~BIT4;
  P4DIR |= BIT4;
  P4OUT |= BIT4;
  __delay_cycles(1000);
    
  P5SEL1 &= ~BIT6;
  P5SEL0 &= ~BIT6;
  P5DIR |= BIT6;
  P5OUT &= ~BIT6;
  
 

  //UART DEBUG
  P2SEL1 |= (BIT0 | BIT1);
  P2SEL0 &= ~(BIT0 | BIT1);
  P2DIR &= ~BIT1;
  
  

  P2SEL1 |= (BIT5 | BIT6);
  P2SEL0 &= ~(BIT5 | BIT6);
  P2DIR &= ~BIT5;
#if 0 
   P6SEL1 &= ~(BIT0 | BIT1);
    P6SEL0 |= (BIT0 | BIT1);
    P6DIR &= ~BIT0;
#endif
  SPI_CS_H;
  __delay_cycles(1000);
  SPI_FLASH_CS_SD_L;

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;
}

/******************************************************************************
*
* @brief   Initializes the Memory Protection Unit of FR5969
*          This allows for HW protection of Bootloader area
*
* @return  none
*****************************************************************************/
static void MPU_init(void)
{
    // These calculations work for FR5739 (check user guide for MPUSEG values)
#define MPUB1 (((APP_MAIN_END_ADDR+1)&0x00FFF0)>>4) //lower memory mask
#define MPUB2 (((APP_HIGH_START_ADDR)&0x013FF0)>>4) //up memory mask

    // Enable access to MPU registers
    MPUCTL0 = MPUPW;
    // Seg1 = 0x0000 - App , Seg2 = App - Boot, Seg3 = Boot - 0xFFFF
    MPUSEGB1 = MPUB1;
    MPUSEGB2 = MPUB2;
    MPUSAM |= MPUSEG1WE;        // Segment 1 Enable Write access
    MPUSAM &= ~MPUSEG2WE;       // Segment 2 is protected from write
    MPUSAM &= ~MPUSEG2VS;       // Violation select on write access
    MPUSAM |= MPUSEG3WE;        // Segment 3 Enable Write access
    // Enable MPU protection
    MPUCTL0 = MPUPW | MPUENA | MPUSEGIE;    // MPU registers locked until BOR
}
