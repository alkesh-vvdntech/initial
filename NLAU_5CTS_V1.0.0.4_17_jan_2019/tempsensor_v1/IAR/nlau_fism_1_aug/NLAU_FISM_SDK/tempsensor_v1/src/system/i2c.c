/*
 * i2c.c
 *
 *  Created on: July 24, 2017
 *      Author: aditya
 */
//macros for cMode
#define I2C_WRITE				0x00
#define I2C_READ				0x01
#define I2C_DATA_RECEIVED 	             	0x02

#include "config.h"
#include "i2c.h"
#include "driverlib.h"
#include "timer.h"
#include "string.h"

#define I2C_DELAY 150
#define I2C_DELAY_WRITE 200

#ifdef _CT5_BOOTLOADER_
#if defined(__TI_COMPILER_VERSION__)
#pragma SET_DATA_SECTION(".aggregate_vars")
volatile int8_t 	I2CRX[I2C_RX_LEN];
volatile int8_t 	I2CTX[I2C_TX_LEN];
volatile int8_t  	iI2CRxIdx = 0;
volatile int8_t    iI2CTxIdx = 0;
volatile int8_t    iI2CTxLen = 0;
volatile int8_t    iI2CRxLen = 0;
volatile int8_t     cMode  = 0;
#pragma SET_DATA_SECTION()
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma location="AGGREGATE" 
__no_init volatile int8_t 	I2CRX[I2C_RX_LEN];
#pragma location="AGGREGATE" 
__no_init volatile int8_t 	I2CTX[I2C_TX_LEN];
#pragma location="AGGREGATE" 
__no_init volatile int8_t  	iI2CRxIdx;
#pragma location="AGGREGATE" 
__no_init volatile int8_t    iI2CTxIdx ;
#pragma location="AGGREGATE" 
__no_init volatile int8_t    iI2CTxLen ;
#pragma location="AGGREGATE" 
__no_init volatile int8_t    iI2CRxLen ;
#pragma location="AGGREGATE" 
__no_init volatile int8_t     cMode ;
#else
#error Compiler not supported!
#endif

#ifndef _FISM_
void i2c_init(uint32_t uiSpeed)
{
    UCB0CTLW0 |= UCSWRST;                     	            // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCSSEL__SMCLK; // I2C mode, Master mode, sync, SMCLK src
    UCB0BRW 	= (uint16_t) (CS_getSMCLK() / uiSpeed);
    UCB0CTLW0 &= ~UCSWRST;                     		    // enable I2C by releasing the reset
    UCB0IE |= UCRXIE0 | UCTXIE0 | UCSTTIE | UCNACKIE;	    // enable Rx and Tx interrupt
}
#else
void i2c_init()
{
    UCB3CTLW0 |= UCSWRST;                     		      // Software reset enabled
    UCB3CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, SMCLK src
    UCB3BRW = 0x64;
    UCB3CTLW0 &= ~UCSWRST;                     		      // enable I2C by releasing the reset
    UCB3IE |= UCRXIE0 | UCTXIE0 | UCSTTIE | UCNACKIE;	      // enable Rx and Tx interrupt
}
#endif


void i2c_read(uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData)
{
     int8_t      iRetry = MAX_I2CRX_RETRY;

#ifdef _FISM_
     uint8_t i2c_busy_count = 10;

     while((UCB3STATW & UCBBUSY) && --i2c_busy_count)   {  // checking I2C busy flag
         __delay_cycles(100000);
         UCB3CTLW0 |= UCSWRST;      // Software reset enabled
         UCB3CTLW0 = 0;
         UCB3CTLW0 &= ~UCSWRST;     // enable I2C by releasing the reset
         i2c_init();
     }
#endif

#ifndef _FISM_
     UCB0I2CSA = ucSlaveAddr;               // Slave address
#else
     UCB3I2CSA = ucSlaveAddr;               // Slave address
#endif
     cMode = I2C_READ;
     I2CTX[0] = ucCmd;
     iI2CTxIdx = iI2CRxIdx =0;
     iI2CTxLen = 1;
     iI2CRxLen = ucLen;
#ifndef _FISM_
     UCB0CTL1 |= UCTR;
     UCB0CTL1 |= UCTXSTT;                   // I2C start condition
#else
     UCB3CTL1 |= UCTR;
     UCB3CTL1 |= UCTXSTT;                   // I2C start condition     
#endif

     while(!(cMode & I2C_DATA_RECEIVED) && iRetry)  {
        iRetry--;
#ifndef _FISM_
        delay(I2C_DELAY);
#else
        __delay_cycles(10000);
#endif
        pucData[0] = 0;
     };
     memcpy((void *) pucData,(const void *) I2CRX,ucLen);
}


void i2c_write(uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData)
{
#ifndef _FISM_
      uint8_t i2c_busy_count=0;

      if(ucSlaveAddr == 0x3e)  {
          if(!ucCmd)  {
              P4OUT &= ~BIT5;
          } else {
              P4OUT |= BIT5;
          }
          delay(15);
      }

     while(UCB0STATW & UCBBUSY)  {     // checking I2C busy flag   
         delay(50);
         i2c_busy_count++;
         if(i2c_busy_count > 10)  {
             // I2C controller is stuck for some reason. Re-Initializing the I2C controller and returning from here
             i2c_init(I2C_INIT_SPEED);
             return;
         }
     }
     UCB0I2CSA = ucSlaveAddr;          // Slave address
#else

     uint8_t i2c_busy_count = 10;

     while((UCB3STATW & UCBBUSY) && --i2c_busy_count)   {  // checking I2C busy flag
         __delay_cycles(100000);
         UCB3CTLW0 |= UCSWRST;      // Software reset enabled
         UCB3CTLW0 = 0;
         UCB3CTLW0 &= ~UCSWRST;     // enable I2C by releasing the reset
         i2c_init();
     }
     UCB3I2CSA = ucSlaveAddr;       // Slave address    
#endif

     cMode = I2C_WRITE;
     I2CTX[0] = ucCmd;
     memcpy((char *) &I2CTX[1],(const char *) pucData,ucLen);
     iI2CTxIdx = 0;
     iI2CTxLen = ucLen+1;
#ifndef _FISM_
     UCB0CTL1 |= UCTR;
     UCB0CTL1 |= UCTXSTT;       // I2C start condition
     delay(I2C_DELAY_WRITE);
#else
     UCB3CTL1 |= UCTR;
     UCB3CTL1 |= UCTXSTT;       // I2C start condition
#endif
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#ifndef _FISM_
__interrupt void USCI_B0_ISR(void)
#else
__interrupt void USCI_B3_ISR(void)
#endif
#else
#error Compiler not supported!
#endif
{
#ifndef _FISM_
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
#else
    switch(__even_in_range(UCB3IV, USCI_I2C_UCBIT9IFG))
#endif
    {
      case USCI_NONE:          break;         // Vector 0: No interrupts
      case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
      case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG
#ifndef _FISM_
            UCB0CTL1 |= UCTXSTT;              // I2C start condition
#else
            UCB3CTL1 |= UCTXSTT;
#endif
            break;
      case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
      case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
      case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
      case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
      case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
      case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
      case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
      case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
      case USCI_I2C_UCRXIFG0:                 // Vector 22: RXIFG0
#ifndef _FISM_
            I2CRX[iI2CRxIdx] = UCB0RXBUF;     // Get RX data
            if ((cMode & I2C_READ) && (iI2CRxIdx == iI2CRxLen))  {
    		  //trigger a stop condition
    		  UCB0CTL1 |= UCTXSTP;
    		  cMode |= I2C_DATA_RECEIVED;
            }
#else
            I2CRX[iI2CRxIdx] = UCB3RXBUF;     // Get RX data
            if ((cMode & I2C_READ) && (iI2CRxIdx == iI2CRxLen))  {
    		  //trigger a stop condition
    		  UCB3CTL1 |= UCTXSTP;
    		  cMode |= I2C_DATA_RECEIVED;
            }
#endif
            //update the RX index
            iI2CRxIdx = (iI2CRxIdx + 1);
            if(iI2CRxIdx >= I2C_RX_LEN)  {
                iI2CRxIdx = 0;
            }
            //__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
            break;

    case USCI_I2C_UCTXIFG0:  		      // Vector 24: TXIFG0
#ifndef _FISM_
            if(iI2CTxIdx < iI2CTxLen)  {
                UCB0TXBUF = I2CTX[iI2CTxIdx];
                iI2CTxIdx = (iI2CTxIdx + 1);
            }  else  {
                if(cMode & I2C_READ)  {
                    //trigger a repeat start with receiver mode
                    UCB0CTL1 &= ~UCTR;
                    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
                }  else  {
                    //trigger a stop condition
                    UCB0CTL1 |= UCTXSTP;
                    __bic_SR_register_on_exit(LPM0_bits); // Transfer finished restore CPU so we can interrupt the i2c write delay
                }
            }
#else
            if(iI2CTxIdx < iI2CTxLen)  {
                UCB3TXBUF = I2CTX[iI2CTxIdx];
                iI2CTxIdx = (iI2CTxIdx + 1);
            }  else  {
                if(cMode & I2C_READ)  {
                    //trigger a repeat start with receiver mode
                    UCB3CTL1 &= ~UCTR;
                    UCB3CTL1 |= UCTXSTT;                    // I2C start condition
                }  else  {
                    //trigger a stop condition
                    UCB3CTL1 |= UCTXSTP;
                    __bic_SR_register_on_exit(LPM0_bits); // Transfer finished restore CPU so we can interrupt the i2c write delay
                }
            }            
#endif
            break;

      case USCI_I2C_UCBCNTIFG: break;         // Vector 26: BCNTIFG
      case USCI_I2C_UCCLTOIFG: break;         // Vector 28: clock low timeout
      case USCI_I2C_UCBIT9IFG: break;         // Vector 30: 9th bit
      default: break;
    }
}

#if defined(__IAR_SYSTEMS_ICC__)
    /* static data buffer for acceleration */
    #pragma location = "I2C_VTABLE"
__root const uint32_t I2C_Vectors[] = {
    (uint32_t)i2c_read,
    (uint32_t)i2c_write
};
#else
#error "Compiler not supported!"
#endif


#else
extern const uint32_t _I2C_VECTOR[2];

static void (**pf_i2c_read) (uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData) = (void (**) (uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData)) (_I2C_VECTOR + 0);

static void (**pf_i2c_write) (uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData) = (void (**) (uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData)) (_I2C_VECTOR + 1);

void i2c_read(uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData) {
    (**pf_i2c_read)(ucSlaveAddr, ucCmd, ucLen, pucData);
}

void i2c_write(uint8_t ucSlaveAddr, uint8_t ucCmd, uint8_t ucLen, uint8_t* pucData) {
    (**pf_i2c_write)(ucSlaveAddr, ucCmd, ucLen, pucData);
}

#endif
