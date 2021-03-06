#include "thermalcanyon.h"
#include "data_transmit.h"
#include "hardware_buttons.h"
#include "fatdata.h"
#include "battery.h"
#include "events.h"
#include "debug.h"
#include "spi_flash.h"
#include "state_machine.h"
#include "main_system.h"
#include "temperature.h"
#include "watchdog.h"
#include "timer.h"
#include "fridge_data.h"
#include "fridge_uart.h"

int g_iRunning = 0;
extern char device_imei[20];
#ifdef _FISM_
struct tm rtime;
s_flags flags;
unsigned g_uch_status = SEND_PACKET_OK;
#endif

//reset the board by issuing a SW BOR
void system_reboot(const char *message) {
#ifndef _FISM_
	log_enable(); //just in case we hung during an upload
	log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_REBOOT), message);
#endif

        g_pSysState->safeboot.disable.state_clear_on_boot = 1;
	PMM_trigBOR();
	while (1);	//code should not come here
}

/****************************************************************************/
/*  IO SETUP                                                                */
/****************************************************************************/

// Setup Pinout for I2C, and SPI transactions.
// http://www.ti.com/lit/ug/slau535a/slau535a.pdf
/*sachin void system_setupIO_clock() {
	// Startup clock system with max DCO setting ~8MHz
	CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
	CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
	CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
	//CSCTL3 = DIVA__1 | DIVS__8 | DIVM__1;     // Set all dividers TODO - divider
	CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers TODO - divider
	CSCTL4 &= ~LFXTOFF;
	do {
		CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
		SFRIFG1 &= ~OFIFG;
	} while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag
	CSCTL0_H = 0;                             // Lock CS registers
}*/

static void setup_IO() {
//***********************//
// clock, lcd, i2c, SPI
// IO set up in Bootloader
//***********************//

#ifndef _FISM_
        switchers_setupIO();

        P3DIR |= BIT4;    		// Set P3.4 buzzer output
        P2DIR &= ~BIT7;   		// Set P2.7 MODEM_ON# input

//Already unlocked in bootloader
//PM5CTL0 &= ~LOCKLPM5;
//sachin	lcd_setupIO();
#endif
	// Disable the GPIO power-on default high-impedance mode to activate
	// previously configured port settings
        PM5CTL0 &= ~LOCKLPM5;

#ifdef _FISM_
        uart_setupIO();
	uart_setupIO_clock();
        
        /* DTR*/
        
        P3DIR |=  BIT2;          // DTR
        P3OUT &= ~BIT2;
        
        /* ADC Charger Pins */
        P3SEL0 |= (BIT3);
        P3SEL1 &= ~(BIT3);
        P2SEL0 |= (BIT4);
        P2SEL1 |= (BIT4);

        /* ADC Temperature*/
        P4SEL0 |= (BIT2 | BIT3);
        P4SEL1 &= ~(BIT2 | BIT3);
        ADC_setupIO();

        /* I2C SDA/SCL Selection */
        P6SEL0 |= (BIT4| BIT5);
        P6SEL1 &= ~(BIT4| BIT5);
        P6DIR &= ~(BIT5);   //i2c clock as input
        P6DIR |= (BIT4);    //i2c data as output

        SPI_Init();
        
        
         PM5CTL0 &= ~LOCKLPM5;
       // i2c_init();
#endif

//also redundant in bootloader - not sure if this should be changed
/*sachin #ifndef I2C_DISABLED
        i2c_init(EUSCI_B_I2C_SET_DATA_RATE_400KBPS);
        i2c_init(380000);
#endif*/
	__bis_SR_register(GIE);		//enable interrupt globally
}

/****************************************************************************/
/*  SYSTEM START AND CONFIGURATION                                          */
/****************************************************************************/


void modem_turn_on() {
#ifndef _FISM_
	// Turn modem ON
	P4OUT &= ~BIT0;
#else
        P4OUT |= BIT5;
#endif
}

void modem_turn_off() {
#ifndef _FISM_
	// Turn modem OFF
	P4OUT |= BIT0;
#else
        P4OUT &= ~BIT5;
#endif
}

#ifdef _FCLOCK_
void clock_init()
{
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
    CSCTL0_H = 0;
}
#endif


void modem_reset() {
        modem_turn_off();
	delay(300); //200ms minimum to properly disable modem
	modem_turn_on();
}

void modem_reboot() {
#ifndef _FISM_
	log_appendf(LOG_TYPE_SYSTEM, get_string(LOG_MSG, LOG_REBOOT), "MODEM");
//	lcd_printl(LINE1,get_string(LCD_MSG, UARTFAILED));
//	lcd_printl(LINE2,get_string(LCD_MSG, REBOOTING_MODEM));
	modem_reset();
//	delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
	modem_first_init();
#endif
}

void system_boot() {
#ifndef _FISM_
        UINT bytes_written = 0;
#endif
	setup_IO();
	modem_reset();

#ifndef _FISM_
//sachin	lcd_reset();
// For some reason, lcd has to be enabled before battery init.
// Probably something with the i2c, still checking what is the problem here.
//     	lcd_blenable();

//#ifndef BATTERY_DISABLED
//	batt_init();
//#endif

//sachin	hardware_enable_buttons();
//sachin	lcd_init();

	fat_init_drive();
#endif
        // Checks if this system has been initialized. Reflashes config
        //and runs calibration in case of being first flashed.
	config_init();

#ifndef _FISM_
	lcd_setVerboseMode(VERBOSE_BOOTING);         // Booting is not completed
#endif
	// Initial trigger of temperature capture. Just get one sample
          temp_check();

#ifndef _FISM_
	// to allow conversion to get over and prevent any side-effects to other interface like modem
	// TODO is this delay to help on the following bug from texas instruments ? (http://www.ti.com/lit/er/slaz627b/slaz627b.pdf)
	lcd_progress_wait(2000);
#endif

#ifdef _FDEBUG_
        debug_print("\n\r/***********MODEM INIT************/\n\r");
#endif

        g_pSysCfg->modemFirstInit = 1;
#ifndef _FISM_
	modem_first_init();
#else
        gsm_init();
#endif
        g_pSysCfg->modemFirstInit = 0;

#ifndef _FISM_
//sachin	batt_check_level();

// Init finished, we disabled the debugging display
//batt_init();
//batt_getlevel();

        g_iDisplayId = 0;
	lcd_disable_verbose();

        log_sample_to_disk(&bytes_written);
	log_sample_web_format(&bytes_written);
	sms_process_messages();

	lcd_printl(LINEC,get_string(LCD_MSG,FINISHED_BOOT));
	delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
#endif

#ifdef _FDEBUG_
        debug_print("finished boot\n\r");
#endif
	g_iRunning = 1;		// System finished booting and we are going to run
	g_iSystemSetup = 0;	// Reset system setup button state
}

#ifndef _FISM_
_Sigfun * signal(int i, _Sigfun *proc) {
	__no_operation();
	return NULL;
}
#endif
/****************************************************************************/
/*                         MAIN                                             */
/****************************************************************************/

// Paints the stack in a value to check for anomalies
//#define EMPTY_STACK_VALUE 0x69

//uint8_t stackClear = EMPTY_STACK_VALUE;
#ifdef ___CHECK_STACK___
// Used to check the stack for leaks
extern char __STACK_END;
extern char __STACK_SIZE;

void checkStack() {
	char *pStack = (void*) (&__STACK_END - &__STACK_SIZE);
	size_t stack_empty = 0;
	char *current_SP;

	current_SP = (char *) _get_SP_register();

	while (current_SP > pStack) {
		current_SP --;
		*current_SP = stackClear;
		stack_empty++;
	}

	if (g_pSysCfg->stackLeft==0 || stack_empty < g_pSysCfg->stackLeft)
		g_pSysCfg->stackLeft = stack_empty;

	if (stack_empty == 0) {
		*current_SP = 0;
	}

	stackClear++;
}
#endif

//Manual interrupt vector assignment with extern defs
#ifndef _FISM_
extern __interrupt void RTC_B_ISR(void);
extern __interrupt void Port_4(void);
extern __interrupt void Port_3(void);
extern __interrupt void Port_2(void);
extern __interrupt void Timer2_A0_ISR(void);
extern __interrupt void ADC12_ISR(void);
extern __interrupt void USCI_A0_ISR(void);

extern uint16_t _App_Proxy_Vector_Start[];  /* App proxy table address */
#else
extern __interrupt void ADC12_ISR(void);
extern __interrupt void USCI_A2_ISR(void);
extern __interrupt void RTC_C_ISR(void);
extern __interrupt void USCI_B3_ISR(void);
extern __interrupt void USCI_A0_ISR(void);
extern __interrupt void port3_ISR(void);
extern __interrupt void TIMERB0_ISR(void);
extern __interrupt void USCI_A1_ISR(void);

extern uint16_t _App_Proxy_Vector_Start[];  /* App proxy table address */
#endif


int main(void) {

	// Disable for init since we are not going to be able to respond to it.
       watchdog_disable();
       
#pragma diag_suppress=Pe767
#ifndef _FISM_
    //this is a hack
    *((uint16_t *)(_App_Proxy_Vector_Start + RTC_VECTOR       - 59)) = (uint16_t)RTC_B_ISR;
    *((uint16_t *)(_App_Proxy_Vector_Start + PORT4_VECTOR     - 59)) = (uint16_t)Port_4;
    *((uint16_t *)(_App_Proxy_Vector_Start + PORT3_VECTOR     - 59)) = (uint16_t)Port_3;
    *((uint16_t *)(_App_Proxy_Vector_Start + PORT2_VECTOR     - 59)) = (uint16_t)Port_2;
    *((uint16_t *)(_App_Proxy_Vector_Start + TIMER2_A0_VECTOR - 59)) = (uint16_t)Timer2_A0_ISR;
    *((uint16_t *)(_App_Proxy_Vector_Start + ADC12_VECTOR     - 59)) = (uint16_t)ADC12_ISR;
    *((uint16_t *)(_App_Proxy_Vector_Start + USCI_A0_VECTOR   - 59)) = (uint16_t)USCI_A0_ISR;
#else
    *((uint16_t *)(0xC3DC)) = (uint16_t)ADC12_ISR;
    *((uint16_t *)(0xC388)) = (uint16_t)USCI_A2_ISR;
    *((uint16_t *)(0xC3A0)) = (uint16_t)RTC_C_ISR;
   // *((uint16_t *)(0xC378)) = (uint16_t)USCI_B3_ISR;
    *((uint16_t *)(0xC3E4)) = (uint16_t)USCI_A0_ISR;
  //  *((uint16_t *)(0xC3F0)) = (uint16_t)TIMERB0_ISR;
    *((uint16_t *)(0xC3D0)) = (uint16_t)USCI_A1_ISR;
    
#endif

#pragma diag_default=Pe767

#ifdef _FCLOCK_
        clock_init();
#endif
        debug_init();
#ifdef _FDEBUG_
        debug_print("in main\n\r");
#endif

        state_init();  // Clean the state machine

#ifndef _FISM_
        watchdog_init();
        watchdog_timer_touch();
#else

#ifdef _WDT_
        START_WDT_14_MIN;
#endif

#endif
        system_boot();
	events_init();
        
#ifdef _FISM_
        rtc_init(&rtime);
        enable_diagnosis();
      //  timerB_init();
        flags.booted_device = 1;        
        Extrn_UART_init();
        flags.device_init = 1;
        debug_print("debug UART OK\r\n");
    //    ext_uart_tx("Ext UART OK\r\n");
     //   uint8_t buff[] = {0x01,0x20,0x2B,0x20,0x0E,0x20,0x03,0x20,0x80,0x20,0x70,0x20,0xB7};
      //  event_force_sample();
        debug_print("/**Initialization is Complete,you can press ? with enter key for help**/\r\n");       
        while(1)
        {      
             // ext_uart_tx(buff);
              delay_sec(5);
              rtc_update_time();
              events_run();
#ifdef _FISM_
              if(!(GSM_STATUS_ACTIVE))
              {
                 flags.internet_flag = 0;
                 gsm_init();
              }
             
              /*check for diagnosis*/
              if(flags.check_diagnosis)
              {
                  debug_print("\r\n");
                  flags.check_diagnosis = FALSE;
                  check_diagnosis_command();
                  parse_diag_command();
              }
             // enter_sleep();
#ifdef _WDT_
              START_WDT_14_MIN;
#endif
              
#endif
        }
#endif

#ifndef _FISM_
	events_init();
	state_process();
        lcd_show();

//      watchdog_init();
//      watchdog_timer_touch();

	while (1) {
#ifdef ___CHECK_STACK___
            checkStack();
#endif
/*
#ifdef _DEBUG
            //events_debug(rtc_get_second_tick());
#endif
*/
            hardware_actions();
            // Checks all the events that we have and runs the right one.
            rtc_update_time();
            events_run();
            watchdog_timer_touch();

#ifdef POWER_SAVING_ENABLED
            if(!MODEM_POWERSAVE_ACTIVE) modem_enter_powersave_mode();
#endif
		// Wait here behind the interruption to check for a change on display.
		// If a hardware button is pressed it will resume CPU
		event_main_sleep();
	}
#endif
}
