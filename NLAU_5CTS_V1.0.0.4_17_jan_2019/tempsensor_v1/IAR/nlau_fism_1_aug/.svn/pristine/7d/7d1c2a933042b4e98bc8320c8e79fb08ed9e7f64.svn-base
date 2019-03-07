#include <msp430.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "spi_flash.h"
#include "config.h"
#include "temperature.h"
#include "main_system.h"
#include "fridge_uart.h"
#include "watchdog.h"

uint8_t fridge_buffer[FRIDGE_MAX_SIZE] = {"\0"};
uint16_t fridge_index=0;

void ext_uart_enable()
{
    UCA1IE |= UCRXIE;
}


void ext_uart_disable()
{
    UCA1IE&= ~UCRXIE;
}

void ext_uart_tx(char *cmd)
{
   while(*cmd!='\0')
    {
        UCA1TXBUF=*cmd++;
        while(UCA1STATW&=UCBUSY);
    }
}

void Extrn_UART_init()
{
    /* Baud Rate 115200 */
   // Configure USCI_A1 for UART mode
    UCA1CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA1CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    UCA1BRW = 4;                            
    UCA1MCTLW |= UCOS16 | UCBRF_5 |0x5500;  
    UCA1CTLW0 &= ~UCSWRST;                  // release from reset
    UCA1IE |= UCRXIE;                    // Enable USCI_A1 RX interrupt
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=EUSCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    uint8_t ch;
    flags.power_saving = 0;
   __bic_SR_register_on_exit(LPM0_bits); // Resume functionality.
    switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG))
    {
      case USCI_NONE: break;
      case USCI_UART_UCRXIFG:
        
        ch = UCA1RXBUF;
        if((ch == '{') ||(flags.fridge_uart_start == 1))
        {
           flags.fridge_uart_start = 1;
           fridge_buffer[fridge_index] = UCA1RXBUF;
           fridge_buffer[fridge_index+1] = '\0';
           fridge_index=((fridge_index+1)%FRIDGE_MAX_SIZE);
           if(ch == '#')
           {
             check_data_crc();
             fridge_index = 0;
             memset(fridge_buffer,'\0',sizeof(fridge_buffer));
             flags.fridge_uart_start = 0;
           }               
        }
            
         break;

        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;

        default: break;

    }
}
