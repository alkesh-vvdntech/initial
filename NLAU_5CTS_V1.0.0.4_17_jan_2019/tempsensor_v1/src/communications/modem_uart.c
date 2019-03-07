#include "thermalcanyon.h"
#include "msp430.h"
#include "config.h"
#include "common.h"
#include "modem_uart.h"
#include "stdlib.h"
#include "string.h"
#include "lcd.h"
#include "globals.h"
#include "stdio.h"
#include "stringutils.h"
#include "modem.h"
#include "fatdata.h"
#include "timer.h"
#include "spi_flash.h"



//extern resp resp_json;
#ifndef _FISM_
// AT Messages returns to check.
const char AT_MSG_OK[] = { 0x0D, 0x0A, 'O', 'K', 0x0D, 0x0A, 0 };
const char AT_MSG_PROMPT[] = { 0x0D, 0x0A, '>', 0 }; // Used for SMS message sending
const char AT_MSG_LONG_OUT_PROMPT[] = { 0x0D, 0x0A, '>', '>', '>', 0 }; // Used for http post transactions
const char AT_MSG_LONG_IN_PROMPT[]  = { 0x0D, 0x0A, '<', '<', '<', 0 }; // Used for http qry transactions
const char AT_MSG_HTTPRING[] = "#HTTPRING: ";
const char AT_CME_ERROR[] = "+CME ERROR:";
const char AT_CMS_ERROR[] = "+CMS ERROR:"; // Sim error
const char AT_ERROR[] = " ERROR:";
char RXBuffer[RX_LEN];
#else
char GSM_buffer[BUFFER_MAX_SIZE];
unsigned int rec_buffer_index;
#endif

#if defined(__TI_COMPILER_VERSION__)
#pragma SET_DATA_SECTION(".aggregate_vars")
volatile char RXBuffer[RX_LEN];
char TXBuffer[TX_LEN] = "";
#pragma SET_DATA_SECTION()
#elif defined(__IAR_SYSTEMS_ICC__)
//#pragma location="AGGREGATE"
//__no_init volatile char RXBuffer[RX_LEN];
char RXBuffer[RX_LEN];
#pragma location="AGGREGATE"
__no_init char TXBuffer[TX_LEN];
#else
#error Compiler not supported!
#endif

volatile int uart_numDataPages = -1; // Number of pages of data to retrieve. -1 is circular buffer
volatile UART_TRANSFER uart;

// Carriage return
#define ATCR 10

int count = 0;

/*************************** ERROR AND OK  *****************************/
// Function to check end of msg
void uart_setCheckMsg(const char *szOK, const char *szError) {
	uart.switches.b.active = 1;
	uart.switches.b.transmissionEnd = 0;
	uart.iUartState = UART_INPROCESS;

	if (szError != NULL) {
		uart.ErrorIdx = 0;
		uart.ErrorLength = strlen(szError) - 1; // We don't check for 0 terminated string
		strncpy((char *) uart.szError, szError, sizeof(uart.szError));
	} else
		uart.ErrorLength = -1;

	if (szOK != NULL) {
		uart.OKIdx = 0;
		uart.OKLength = strlen(szOK) - 1; // We don't check for 0 terminated string
		strncpy((char *) uart.szOK, szOK, sizeof(uart.szOK));
	} else
		uart.OKLength = -1;

	// Wait for a return after OK
	uart.switches.b.RXWaitForReturn = 0;
}

// We define the exit condition for the wait for ready function
inline void uart_checkERROR() {

	if (uart.ErrorLength == -1)
		return;

	if (uart.iUartState == UART_ERROR) {
#ifndef _FISM_
          if (UCA0RXBUF == '\n')
#else
          if (UCA2RXBUF == '\n')
#endif
            uart.switches.b.transmissionEnd = 1;
		return;
	}

#ifndef _FISM_
	if (UCA0RXBUF != uart.szError[++uart.ErrorIdx]) {
#else
        if (UCA2RXBUF != uart.szError[++uart.ErrorIdx]) {
#endif
		uart.ErrorIdx = 0;
		return;
	}

	if (uart.ErrorIdx == uart.ErrorLength) {
		uart.iUartState = UART_ERROR;
		uart.ErrorIdx = 0;
	}
}

// Cancels data transaction after a number of pages has been obtained
void uart_setNumberOfPages(int numPages) {
	uart_numDataPages = numPages;
}

void uart_setRingBuffer() {
	uart_numDataPages = -1;
}

// We define the exit condition for the wait for ready function
inline void uart_checkOK() {

	if (uart.OKLength == -1)
		return;

	if (uart.switches.b.RXWaitForReturn && uart.iUartState == UART_SUCCESS) {
#ifndef _FISM_
		if (UCA0RXBUF == 0x0A)
#else
                if (UCA2RXBUF == 0x0A)
#endif
			uart.switches.b.transmissionEnd = 1;
		return;
	}

#ifndef _FISM_
	if (UCA0RXBUF != uart.szOK[++uart.OKIdx]) {
#else
        if (UCA2RXBUF != uart.szOK[++uart.OKIdx]) {
#endif
		uart.OKIdx = 0;
		return;
	}

	// Wait for carriage return
	if (uart.OKIdx == uart.OKLength) {
		if (!uart.switches.b.RXWaitForReturn)
			uart.switches.b.transmissionEnd = 1;
		else
			_NOP();
		uart.iUartState = UART_SUCCESS;
		uart.OKIdx = 0;
	}
}

void uart_setupIO() {
  // GSM UART
  P5SEL0 |= (BIT4 | BIT5);
  P5SEL1 &= ~(BIT4 | BIT5);

  // GSM POWER ON
  P5DIR |= BIT2;	// FISM --> P1, CT5Wifi --> P5
  P5OUT |= BIT2;	// FISM --> P1, CT5Wifi --> P5

  // GSM SLEEP
  P4DIR |= BIT5;
  P4REN = 0x00;
  P6DIR |= BIT7;    // Set GSM RESET pin (P6.7)
  P6REN = 0x00;
  P6OUT |= BIT7;  // set GSM_RESET to high to turn sim808 ON (internally pull up)

  // Ext UART
  P2SEL0 &= ~(BIT5 | BIT6);
  P2SEL1 |= BIT5 | BIT6;
}

void uart_init() {
	// Configure USCI_A0 for UART mode
	UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
	UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
	// Baud Rate calculation
	// 8000000/(16*115200) = 4.340	//4.340277777777778
	// Fractional portion = 0.340
	// User's Guide Table 21-4: UCBRSx = 0x49
	// UCBRFx = int ( (4.340-4)*16) = 5
	UCA0BRW = 4;                             // 8000000/16/115200
	UCA0MCTLW |= UCOS16 | UCBRF_5 | 0x4900;

#ifdef LOOPBACK
	UCA0STATW |= UCLISTEN;
#endif
	UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
	uart.switches.b.active = 1;
}

int8_t uart_getTransactionState() {
	return uart.iUartState;
}
/*
const char *uart_getRXHead() {
	if (uart.iRXHeadIdx >= sizeof(RXBuffer)) {
		uart.iRXHeadIdx = 0;
		_NOP();
	}
	return (const char *) GSM_buffer;
}
*/
const char *uart_getRXHead() {
	if (uart.iRXHeadIdx >= sizeof(local_storage_buff)) {
		uart.iRXHeadIdx = 0;
		_NOP();
	}
	return (const char *)&local_storage_buff[0];
}

// Reset all buffers, headers, and counters for transmission
void uart_reset_headers() {
#ifdef _DEBUG
	memset((char *) RXBuffer, 0, sizeof(RXBuffer));
#else
	RXBuffer[0] = 0;
#endif

	uart.iTXIdx = 0;
	uart.iRXHeadIdx = uart.iRXTailIdx = 0;
	uart.switches.b.transmissionEnd = 0;

	uart.OKIdx = 0;     // Indexes for checking for end of command
	uart.ErrorIdx = 0;
	uart.iUartState = UART_INPROCESS; // Setup in process, we are actually going to generate a command

	uart.iRxCountBytes = 0; // Stats to know how much data are we getting on this buffer.
}

void modem_send_command(const char *cmd) {
#ifndef _FISM_
#ifdef POWER_SAVING_ENABLED
    //make sure the modem is not in power saving mode
//    if(MODEM_POWERSAVE_ACTIVE)
 modem_exit_powersave_mode();
#endif
#endif
	// Clear reset
	uart_reset_headers();

	// Only copy the buffer if we are not actually using TXBuffer,
	// some places of the old code use the TXBuffer directly :-/

	uart.iTxLen = strlen(cmd);

#ifndef _FISM_
	if (!g_bLogDisabled && g_pDevCfg->cfg.logs.modem_transactions) {
		log_appendf(LOG_TYPE_MODEM, "---%s---\r\n", get_simplified_date_string(NULL));
		log_modem(cmd);
	}
#endif

	// Store the maximum size used from this buffer
#ifdef _DEBUG_COUNT_BUFFERS
	if (uart.iTxLen > g_pSysCfg->maxTXBuffer)
	g_pSysCfg->maxTXBuffer = uart.iTxLen;
#endif

	// This problem should not ocurr, if this happens means that you are not zero terminating the string
	// or you are trying to transmit too much data in one go so you have to split your commands.
	if (uart.iTxLen > sizeof(TXBuffer)) {
		/*
		lcd_print("TXERR");
		*/
#ifndef _FISM_
		delay(HUMAN_DISPLAY_ERROR_DELAY);
#endif
		uart.iTxLen = sizeof(TXBuffer) -1;
	}

	if (cmd != TXBuffer) {
#ifdef _DEBUG
		memset((char *) TXBuffer, 0, sizeof(TXBuffer));
#else
		TXBuffer[0] = 0;
#endif
		memcpy((char *) TXBuffer, cmd, uart.iTxLen);
	}

#ifndef _FISM_
	UCA0IE |= UCRXIE;   // Enable USCI_A0 RX interrupt
	UCA0IE |= UCTXIE;
#else
        UCA2IE |= UCRXIE;   // Enable USCI_A0 RX interrupt
	UCA2IE |= UCTXIE;
#endif
	_NOP();
	return;
}

#define DEFAULT_INTERVAL_DELAY 2
uint8_t delayDivider = DEFAULT_INTERVAL_DELAY;
void uart_setDefaultIntervalDivider() {
	delayDivider = DEFAULT_INTERVAL_DELAY;
}

void uart_setDelayIntervalDivider(uint8_t divider) {
	delayDivider = divider;
}

int waitForReady(uint32_t timeoutTimeMs) {
	uint32_t delayTime = timeoutTimeMs / delayDivider;
	int count = delayDivider;

	if (delayTime <= 100) {
		delayTime = 100;
		count = 1;
	}

	while (count > 0) {
#ifndef _FISM_
		delay(delayTime);
#else
                __delay_cycles(100000);
#endif
		if (uart.switches.b.transmissionEnd == 1) {
#ifndef _FISM_
			delay(100);  // Documentation specifies 30 ms delay between commands
			if (!g_bLogDisabled && g_pDevCfg->cfg.logs.modem_transactions)
				log_modem(uart_getRXHead());
#endif
			return UART_SUCCESS; // There was a transaction, you have to check the state of the uart transaction to check if it was successful
		}
		count--;
	}
#ifndef _FISM_
	if (!g_bLogDisabled && g_pDevCfg->cfg.logs.modem_transactions) {
		log_modem("FAILED\r\n");
		log_modem(uart_getRXHead());
	}
#endif

#ifdef _DEBUG_COUNT_BUFFERS
	// Store the maximum size used from this buffer
	if (uart.iRxCountBytes > g_pSysCfg->maxRXBuffer)
	g_pSysCfg->maxRXBuffer = uart.iRxCountBytes;
#endif

#ifndef _FISM_
	delay(100);  // Documentation specifies 30 ms delay between commands
#else
       __delay_cycles(100000);
#endif

	if (uart.iUartState != UART_SUCCESS) {
		uart.iUartState = UART_TIMEOUT;
		return UART_FAILED;
	}

	return UART_SUCCESS;
}

uint8_t g_iModemErrors = 0;
char modem_lastCommand[16];

// Try a command until success with timeout and number of attempts to be made at running it
uint8_t uart_tx_data(const char *cmd, uint32_t timeout,	uint8_t attempts) {
#ifndef _FISM_
	lcd_print_progress();
#endif

	if (!state_isSimOperational())
		return UART_FAILED;

	while (attempts > 0) {
		modem_send_command(cmd);
                if (!waitForReady(timeout)) {

			// Transaction could be sucessful but the modem could have return an error.
			modem_check_uart_error();
			uart_setRingBuffer();
			g_iModemErrors = 0;
			return UART_SUCCESS;
		}
		attempts--;
		/*
		if (g_iLCDVerbose == VERBOSE_BOOTING) {
			lcd_printl(LINEC, modem_lastCommand);
			lcd_print_boot("TIMEOUT", LINE2);
		}
		*/
	}

	// If there was an error we have to wait a bit to retrieve everything
        // that is left from the transaction, like the error message
	modem_check_uart_error();

	g_iModemErrors++;
	if(g_iModemErrors > 5) {
            g_pSysState->state.alarms.modemFailure = 1;
            g_iModemErrors = 0;
        }
	uart_setRingBuffer(); // Restore the ring buffer if it was changed.
	return UART_FAILED;
}

// Try a command until success with timeout and number of attempts to be made at running it
// Appends AT command
uint8_t uart_tx_timeout(const char *cmdInput, uint32_t timeout,
		uint8_t attempts) {
	char *cmd = modem_lastCommand;
	char idx = 0;

	if (!state_isSimOperational())
		return UART_FAILED;

	int len = strlen(cmdInput);

	// Append AT and
	if (!uart.switches.b.RXWaitForReturn &&
		len < sizeof(modem_lastCommand) + 1) {
		if (cmdInput[0] != 'A') {
			modem_lastCommand[0] = 'A';
			modem_lastCommand[1] = 'T';
			idx = 2;
		}
		strcpy(&modem_lastCommand[idx], cmdInput);

		if (cmdInput[len - 1] != '\n') {
			strcat(modem_lastCommand, "\r\n");
			_NOP();
		} else {
			_NOP();
		}
	} else {
		zeroTerminateCopy(modem_lastCommand, cmdInput);
		cmd = (char *) cmdInput;
	}

	return uart_tx_data(cmd, timeout, attempts);
}

void uart_tx_nowait(const char *cmd) {
	uint16_t attempts = 60;
	uart.switches.b.waitForTXEnd = 1;
	modem_send_command(cmd);

	while (uart.switches.b.waitForTXEnd == 1 && attempts>0) {
		delay(1000);
		attempts--;
	}

	modem_check_uart_error();

	if (attempts==0)
		uart.iUartState = UART_FAILED;
	else
		uart.iUartState = UART_SUCCESS;
}

int uart_tx_waitForPrompt(const char *cmd, uint32_t promptTime) {
	/*
#ifdef _DEBUG
	checkStack();
#endif
*/
	int res = UART_ERROR;

	modem_send_command(cmd);
	if (!waitForReady(promptTime)) {
		res = uart_getTransactionState();
		if(res != UART_SUCCESS) goto error;
		uart_setOKMode();
		return res; // We found a prompt
	}
   	res = uart_getTransactionState();

   error:
	modem_check_uart_error();
	uart_setOKMode();
	return res;
}

uint32_t g_iModemMaxWait = MODEM_TX_DELAY1;

void uart_setDelay(uint32_t delay) {
	g_iModemMaxWait = delay;
}

uint8_t isTransactionOK() {
	return uart.ErrorIdx;
}


uint8_t uart_txf(const char *_format, ...) {
	va_list _ap;

	uint16_t res = 0;
	uint16_t size = 0;

	/*
#ifdef _DEBUG
	checkStack();
#endif
*/

	char *szTemp = getStringBufferHelper(&size);
	va_start(_ap, _format);
	vsnprintf(szTemp, size, _format, _ap);
	va_end(_ap);

	res = uart_tx(szTemp);
	releaseStringBufferHelper();
	return res;
}

uint8_t uart_tx(const char *cmd) {
#ifdef _DEBUG_OUTPUT
	char* pToken1;
#endif
	int uart_state;
	int transaction_completed;

	if (!state_isSimOperational())
		return UART_FAILED;

	transaction_completed = uart_tx_timeout(cmd, g_iModemMaxWait, 5);
	if (uart.iRXHeadIdx > uart.iRXTailIdx)
		return transaction_completed;

	uart_state = uart_getTransactionState();
#ifdef _DEBUG_OUTPUT
	if (transaction_completed == UART_SUCCESS && uart_state != UART_ERROR) {
		pToken1 = strstr((const char *) &RXBuffer[RXHeadIdx], ": \""); // Display the command returned
#ifndef _FISM_
		if (pToken1 != NULL) {
			lcd_print_boot((char *) pToken1 + 3, LINE2);
		} else {
			lcd_print_progress((char *) (const char *) &RXBuffer[RXHeadIdx + 2], LINE2); // Display the OK message
		}
#endif
	}
#endif
	return uart_state;
}

#ifndef _FISM_
void uart_setHTTPSNDPromptMode() {
	uart_setCheckMsg(AT_MSG_LONG_OUT_PROMPT, AT_ERROR);
	uart.switches.b.RXWaitForReturn = 0;
}

void uart_setHTTPRCVPromptMode() {
	uart_setCheckMsg(AT_MSG_LONG_IN_PROMPT, AT_ERROR);
	uart.switches.b.RXWaitForReturn = 0;
}

void uart_setHTTPDataMode() {
	uart_setCheckMsg(AT_MSG_HTTPRING, AT_ERROR);
	uart.switches.b.RXWaitForReturn = 1;
}

void uart_setSMSPromptMode() {
	uart_setCheckMsg(AT_MSG_PROMPT, AT_CMS_ERROR);
	uart.switches.b.RXWaitForReturn = 0;
}
#endif

#ifndef _FISM_
void uart_setOKMode() {
	uart_setCheckMsg(AT_MSG_OK, AT_ERROR);
	uart.switches.b.RXWaitForReturn = 0;
}
#endif

#ifndef _FISM_
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=USCI_A0_VECTOR
#ifndef _FISM_
__interrupt void USCI_A0_ISR(void)
#else
__interrupt void USCI_A2_ISR(void)
#endif
#elif defined(__GNUC__)
#ifndef _FISM_
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
void __attribute__ ((interrupt(USCI_A2_VECTOR))) USCI_A2_ISR (void)
#endif
#else
#error Compiler not supported!
#endif
{
#ifndef _FISM_
	switch (__even_in_range(UCA0IV, USCI_UART_UCTXIFG)) {
#else
        switch (__even_in_range(UCA2IV, USCI_UART_UCTXIFG)) {
#endif
	case USCI_NONE:
		break;

	case USCI_UART_UCRXIFG:
		if (!uart.switches.b.active)
			return;

		if (uart.iRXTailIdx >= sizeof(RXBuffer)) {
			uart.iRXTailIdx = 0;
			if (uart_numDataPages > 0) {
				uart_numDataPages--;
				if (uart_numDataPages == 0) {
					uart.iUartState = UART_SUCCESS;
				}
			}
		}

		uart_checkOK();
		uart_checkERROR();

		if (uart_numDataPages != 0) {  // -1 or >1 will be capturing data
#ifndef _FISM_
			RXBuffer[uart.iRXTailIdx++] = UCA0RXBUF;
#else
                        RXBuffer[uart.iRXTailIdx++] = UCA2RXBUF;
#endif
			uart.iRxCountBytes++;  // Data transfer check
		}

#ifndef _FISM_
		if (UCA0STATW & UCRXERR) {
#else
                if (UCA2STATW & UCRXERR) {
#endif
			uart.iError++;
		}

		if (uart.switches.b.transmissionEnd) {
			RXBuffer[uart.iRXTailIdx] = 0;
			__bic_SR_register_on_exit(LPM0_bits); // Resume execution
		}
		break;

	case USCI_UART_UCTXIFG:
		if (!uart.switches.b.active)
			return;

#ifndef _FISM_
		UCA0TXBUF = TXBuffer[uart.iTXIdx];       // Transmit characters
#else
                UCA2TXBUF = TXBuffer[uart.iTXIdx];       // Transmit characters
                debug_print(&UCA2TXBUF);
#endif
		uart.iTXIdx++;
		if (uart.iTXIdx >= uart.iTxLen) {
			uart.iTXIdx = 0;
#ifndef _FISM_
			UCA0IE &= ~UCTXIE; 	// Finished transmision
#else
                        UCA2IE &= ~UCTXIE; 	// Finished transmision
#endif
			if (uart.switches.b.waitForTXEnd == 1) {
				uart.switches.b.waitForTXEnd = 0;
				__bic_SR_register_on_exit(LPM0_bits);
			}
		}

		break;

	default:
		break;
	}
}
#endif

void uart_setupIO_clock() {
#ifndef _FISM_
	// Configure USCI_A0 for UART mode
	UCA0CTLW0 = UCSWRST;               // Put eUSCI in reset
	UCA0CTLW0 |= UCSSEL__SMCLK;        // CLK = SMCLK
	// Baud Rate calculation
	// 8000000/(16*115200) = 4.340	   //4.340277777777778
	// Fractional portion = 0.340
	// User's Guide Table 21-4: UCBRSx = 0x49
	// UCBRFx = int ( (4.340-4)*16) = 5
	UCA0BRW = 4;                       // 8000000/16/115200
	UCA0MCTLW |= UCOS16 | UCBRF_5 | 0x4900;

#ifdef LOOPBACK
	UCA0STATW |= UCLISTEN;
#endif
	UCA0CTLW0 &= ~UCSWRST;          // Initialize eUSCI
	//UCA0IE |= UCRXIE;             // Enable USCI_A0 RX interrupt
#else

        UCA2CTLW0 = UCSWRST;            // Put eUSCI in reset
        UCA2CTLW0 |= UCSSEL__SMCLK;     // CLK = SMCLK

#if 0
        UCA2BRW = 52;                   // 8000000/16/9600
        UCA2MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
#endif
        UCA2BRW = 4;
        UCA2MCTLW |= UCOS16 | UCBRF_5 |0x5500;

        UCA2CTLW0 &= ~UCSWRST;          // release from reset
        UCA2IE |= UCRXIE;               // Enable RX interrupt
#endif
}


#ifdef _FISM_
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void USCI_A2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A2_VECTOR))) USCI_A2_ISR (void)
#else
#error Compiler not supported!
#endif
{
#ifndef _FISM_
	switch (__even_in_range(UCA0IV, 4)) {
#else
        switch (__even_in_range(UCA2IV, 4)) {
#endif
	case USCI_NONE:
		break;

	case USCI_UART_UCRXIFG:
                if(flags.at_cmd)
                {
                     GSM_buffer[rec_buffer_index] = UCA2RXBUF;
                     GSM_buffer[rec_buffer_index+1] = '\0';
                     rec_buffer_index=((rec_buffer_index+1)%BUFFER_MAX_SIZE);
                }
                break;
        case USCI_UART_UCTXIFG:
                break;

	default:
		break;
	}
}

char* send_cmd_wait(char* command, char *response, unsigned char timeout, unsigned char retry)
{
      unsigned char ret = 1;
      char *ret_ptr;
      unsigned char delay_count=0, retry_count=0;

      gsm_exit_sleep();
      flags.at_cmd = 1;
      while((ret == 1) && (retry_count++ < retry))
      {
         flags.at_cmd = 0;
         delay_count = 0;
        debug_print(command);      // debug print
        //AD log_modem(command);         //log to SD card
         rec_buffer_index = 0;
         memset(GSM_buffer,'\0',BUFFER_MAX_SIZE);
         uartA2_xmit(command);       // xmit data to GPS/GSM module
         flags.at_cmd = 1;
         while((ret == 1) && (delay_count++ < timeout))
         {
              ret_ptr = strstr(GSM_buffer, response);
              if(ret_ptr != '\0')
              {
                  ret = 0;
                  break;
              }
              delay_sec(1);
        }
        debug_print(GSM_buffer);     // debug print
//        debug_print("\r\n");
      return ret_ptr;
     }
     return ret_ptr;
}

void uartA2_xmit(char *string)
{
  while(*string!='\0')
    {
        UCA2TXBUF=*string++;
        while(UCA2STATW&=UCBUSY);
    }
}

void delay_sec(unsigned char delay_time)
{
unsigned int count=0;
for(count=0;count<=delay_time;count++)
  {
    __delay_cycles(5005000);
  }
}

#endif