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

#ifndef _FISM_
#include "lcd.h"
#endif

#ifdef _FDEBUG_
#include "debug.h"
#endif

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
#ifndef _FISM_
extern __interrupt void USCI_B0_ISR(void);
#else
extern __interrupt void Timer0_A0_ISR(void);
extern __interrupt void USCI_B3_ISR(void);
#endif

/******************************************************************************
 *
 *  Global variable
 *
*****************************************************************************/
#ifdef _FISM_
unsigned char check_boot = 0;
#endif

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
#ifndef _FISM_
    *((uint16_t *)(0xFF90 + USCI_B0_VECTOR)) = (uint16_t)USCI_B0_ISR;
#else
    *((uint16_t *)(0xFF90 + TIMER0_A0_VECTOR)) = (uint16_t)Timer0_A0_ISR;
    *((uint16_t *)(0xFF90 + EUSCI_B3_VECTOR)) = (uint16_t)USCI_B3_ISR;
#endif
#pragma diag_default=Pe767

    // Initialize MPU
#ifndef _FISM_
  MPU_init();
#endif
    

    // Initialize MCU and required peripherals
    HW_init();
    clock_init();
    SPI_Init();

#ifndef _FISM_
    i2c_init(I2C_INIT_SPEED);
#else
    i2c_init();
#endif

#ifdef _FDEBUG_
    debug_init();
    debug_print("\n\r/**********In Bootloader************************/\n\r");
#endif

    __bis_SR_register(GIE);	//enable interrupt globally
    
    /*check I2C Busy*/
    
    while(UCB3STATW & UCBBUSY)
    {
#ifdef _FDEBUG_
        debug_print("\n\rI2C busy\n\r");
#endif
        UCB3CTLW0 |= UCSWRST;                                     // Software reset enabled
        UCB3CTLW0 = 0;
        UCB3CTLW0 &= ~UCSWRST;                                    // enable I2C by releasing the reset

        i2c_init();
    }
#ifdef _FISM_
    check_batt_state();
    check_boot = *(uint16_t*)0x1980;
#endif
   
#ifndef _FISM_
    lcd_reset();
    lcd_blenable();
    batt_init();
    delay(2000);
    lcd_init();

    while(batt_getlevel() < 10) 
    {
        lcd_bldisable();
        if(!(P4IN & BIT4)) {
            lcd_printl(LINEC,"Chrging");
        } else {
            lcd_printl(LINEC,"Hibrnt");
        }
        delay(2000);
    }

#ifndef NDEBUG // Used for debugging purposes to show entry to MSPBoot  
    lcd_printl(LINEC, "CT5 BOOTLOADER");
    lcd_printl(LINE2, "LOADING");
    lcd_print_progress();
    __delay_cycles(50000000);
#endif
#endif

    // Check for valid update image and update if needed
#ifdef _FISM_
    if ((TI_MSPBoot_AppMgr_CheckForUpdate() == TRUE_t) || (check_boot == 3)) {
#else
    //if (TI_MSPBoot_AppMgr_CheckForUpdate() == TRUE_t) {
        lcd_printl(LINEC, "CT5 BOOTLOADER");
        lcd_printl(LINE2, "LOADING FW");
#endif

#ifdef _FDEBUG_
        debug_print("programming App\n\r");
#endif
        TI_MSPBoot_AppMgr_ProgramApp();
    }

    // Validate the application and jump if needed
    if (TI_MSPBoot_AppMgr_ValidateApp() == TRUE_t) {
#ifndef _FISM_
        lcd_printl(LINEC, "JUMP TO APP");
#endif

#ifdef _FDEBUG_
        debug_print("jumping to app\n\n\r");
#endif
        TI_MSPBoot_APPMGR_JUMPTOAPP();
    }

    while(1)
    {
#ifndef _FISM_
        //Error Updating need manual intervention
        //ALERT MSG!
        lcd_printl(LINEC, "LOAD VALID IMAGE");
        lcd_printl(LINE2, "AND REBOOT");
        __delay_cycles(0xFFFFFF);
#endif

#ifdef _FDEBUG_
        debug_print("invalid image\n\r");
        delay(5000);
#endif
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
#ifndef _FISM_
    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
    CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
    //CSCTL3 = DIVA__1 | DIVS__8 | DIVM__1;   // Set all dividers TODO - divider
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers TODO - divider
    CSCTL4 &= ~LFXTOFF;
    do {
            CSCTL5 &= ~LFXTOFFG;              // Clear XT1 fault flag
            SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);                // Test oscillator fault flag
    CSCTL0_H = 0;                             // Lock CS registers
#else

    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
    CSCTL0_H = 0;                           // Lock CS registers
#endif
}

/******************************************************************************
 *
 * @brief   Initializes the basic MCU HW
 *
 * @return  none
 *****************************************************************************/
static void HW_init(void)
{
#ifndef _FISM_
    // Set all GPIO pins to input to avoid any line contention
    P1DIR = 0;
    P2DIR = 0;
    P3DIR = 0;
    P4DIR = 0;
    PJDIR = 0;

    P1OUT = 0;
    P2OUT = 0;
    P3OUT = 0;
    P4OUT = 0;
    PJOUT = 0;

    // Configure GPIO for SPI
    P2DIR |= BIT3;			// SPI CS
    P2OUT |= BIT3;		        // drive SPI CS high to deactive the chip
    P2SEL1 |= BIT4 | BIT5 | BIT6;       // enable SPI CLK, SIMO, SOMI
    PJSEL0 |= BIT4 | BIT5;              // For XT1
    P1SEL1 |= BIT6 | BIT7;		// Enable I2C SDA and CLK

    //configure LCD backlight
    PJDIR |= BIT6 | BIT7;    	        // set LCD reset and Backlight enable
    PJOUT |= BIT6;			// LCD reset pulled high
    PJOUT &= ~BIT7;			// Backlight disable
#else

    //led
    P2DIR |= BIT7;
    P2OUT &= ~BIT7;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    //charger gpio
    P2DIR &= ~BIT4;  //chrg_imon
    P3DIR &= ~BIT7;  //batt_chrg_curr
    P7DIR &= ~BIT6;  //batt_chrg_status
    
    //SPI Flash
    P7SEL1 &= ~BIT2;
    P7SEL0 |= BIT2;
    P7SEL1 &= ~(BIT0 | BIT1); //MISO, MOSI pin select
    P7SEL0 |= BIT0 | BIT1;

    //Chip Select
    P7DIR |= BIT3;
    P7OUT |= BIT3;

    //SMCLK
    P3SEL1 |= BIT4;
    P3SEL0 &= ~BIT4;
    P3DIR |= BIT4;
    
    // I2C SDA/SCL Selection
    P6SEL0 |= (BIT4| BIT5);
    P6SEL1 &= ~(BIT4| BIT5);
    P6DIR &= ~(BIT5);   //i2c clock as input
    P6DIR |= (BIT4);    //i2c data as output

#ifdef _FDEBUG_
    //UART DEBUG
    P2SEL1 |= (BIT0 | BIT1);
    P2SEL0 &= ~(BIT0 | BIT1);
    P2DIR &= ~BIT1;
#endif
#endif

    P2SEL1 |= (BIT5 | BIT6);
    P2SEL0 &= ~(BIT5 | BIT6);
    P2DIR &= ~BIT5;

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
