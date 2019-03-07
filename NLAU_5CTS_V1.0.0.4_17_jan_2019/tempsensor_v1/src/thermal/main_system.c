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
#include "lcd.h"
#include "json.h"
#include "data_transmit.h"

#include "thermalcanyon.h"
#include "buzzer.h"
#include "json.h"
#include "rtc.h"
#include "jsonAPIs.h"
#include "watchdog.h"

uint16_t  counter_sample = 0;
uint16_t      counter_boot=0;
uint16_t      counter_lcd=0;
uint16_t      counter_sms=0;
uint16_t      counter_upload=0;
uint16_t      counter_heath=0;

int g_iRunning = 0;
extern char device_imei[20];
#ifdef _FISM_
struct tm rtime1;
s_flags flags;
unsigned g_uch_status = SEND_PACKET_OK;
#endif

//reset the board by issuing a SW BOR
void system_reboot(const char *message)
{
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

static void setup_IO()
{
  //***********************//
  // clock, lcd, i2c, SPI
  // IO set up in Bootloader
  //***********************//
  switchers_setupIO();
  P2DIR |= BIT2;  // Buzzer
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;
  uart_setupIO();  
  uart_setupIO_clock();

  /* ADC Temperature*/
  P4SEL0 |= (BIT2 | BIT3);
  P4SEL1 &= ~(BIT2 | BIT3);  
  ADC_setupIO();
  
  //sd card test
  
    // Configure GPIO for SPI
  P7SEL1 &= ~BIT3;     
  P7SEL0 &= ~BIT3;
  P7DIR |= BIT3;						// SPI CS
  P7OUT |= BIT3;					    // drive SPI CS high to deactive the chip
  P7SEL1 &= ~(BIT0 | BIT1 | BIT2);       // enable SPI CLK, SIMO, SOMI
  P7SEL0 |= (BIT0 | BIT1 | BIT2);       // enable SPI CLK, SIMO, SOMI

  /* I2C SDA/SCL Selection */
  P6SEL0 |= (BIT4| BIT5);
  P6SEL1 &= ~(BIT4| BIT5);
  P6DIR &= ~(BIT5);   //i2c clock as input
  P6DIR |= (BIT4);    //i2c data as output

  SPI_Init();
  PM5CTL0 &= ~LOCKLPM5;

  //wifi
  
  P5SEL1 &= ~BIT6;
  P5SEL0 &= ~BIT6;
  P5DIR |= BIT6;
  P5OUT |= BIT6;

  __bis_SR_register(GIE);  //enable interrupt globally
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
    //delay(300); //200ms minimum to properly disable modem
    __delay_cycles(1000);
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

void system_boot()
{
  UINT bytes_written = 0;
  setup_IO();
  modem_reset();
  debug_print("\n\r/***********BATT INIT************/\n\r");
  batt_init(); // aditya
  debug_print("\n\r/***********BATT INIT SUCCESS************/\n\r");
  batt_getlevel();  //aditya
  //config_init();    //test
  lcd_setVerboseMode(VERBOSE_BOOTING);         // Booting is not completed
  lcd_progress_wait(2000);

  debug_print("\n\r/***********MODEM INIT************/\n\r");
  g_pSysCfg->modemFirstInit = 1;
  modem_init();
  g_pSysCfg->modemFirstInit = 0;
  g_iDisplayId = 0;
  lcd_disable_verbose();

  

  //gsm_init();
  debug_print("fat init");
  
    SPI_CS_H;
  __delay_cycles(1000);
  SPI_FLASH_CS_SD_L;
  __delay_cycles(1000);
  
  fat_init_drive();
  
  if(fatdone == true)
  {
    fatdone = false;
    SPI_CS_H;
    __delay_cycles(1000);
                    SPI_FLASH_CS_SD_L;
 
                    UINT tbw = 0;
		    fat_create_dev_ready(&tbw);
  }

   
  temp_check();
  //debug_print("TEMP CHECK SUCCESSFUL");
  //log_sample_to_disk(&bytes_written);
  //debug_print("SAMPLE TO DISK SUCCESSFUL");
 
  //log_sample_web_format(&bytes_written);
  //debug_print("WEB FORMAT SUCCESSFUL");
 
  lcd_printl(LINEC,get_string(LCD_MSG,FINISHED_BOOT));
  //aditya delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
  g_iRunning = 1;	// System finished booting and we are going to run
  g_iSystemSetup = 0;	// Reset system setup button state
}

#ifdef _FISM_
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
char buff[50];
//uint8_t stackClear = EMPTY_STACK_VALUE;
#ifdef ___CHECK_STACK___
// Used to check the stack for leaks
extern char __STACK_END;
extern char __STACK_SIZE;

void checkStack()
{
    char *pStack = (void*) (&__STACK_END - &__STACK_SIZE);
    size_t stack_empty = 0;
    char *current_SP;

    current_SP = (char *) _get_SP_register();

    while (current_SP > pStack)
    {
	current_SP --;
	*current_SP = stackClear;
	stack_empty++;
    }

    if (g_pSysCfg->stackLeft==0 || stack_empty < g_pSysCfg->stackLeft)
	g_pSysCfg->stackLeft = stack_empty;

    if (stack_empty == 0)
    {
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
extern __interrupt void Port_1(void);
extern __interrupt void Port_3(void);
extern __interrupt void Timer2_B0_ISR(void);
extern __interrupt void USCI_A3_ISR(void);



extern uint16_t _App_Proxy_Vector_Start[];  /* App proxy table address */
#endif


int main(void)
{
  // Disable for init since we are not going to be able to respond to it.
  watchdog_disable();

#pragma diag_suppress=Pe767
  *((uint16_t *)(0xC3DC)) = (uint16_t)ADC12_ISR;
  *((uint16_t *)(0xC388)) = (uint16_t)USCI_A2_ISR;
  *((uint16_t *)(0xC3A0)) = (uint16_t)RTC_C_ISR;
  *((uint16_t *)(0xC3E4)) = (uint16_t)USCI_A0_ISR;
  //*((uint16_t *)(0xC3D0)) = (uint16_t)USCI_A1_ISR;
  *((uint16_t *)(0xC3C0)) = (uint16_t)Port_1;
  *((uint16_t *)(0xC3A8)) = (uint16_t)Port_3;
  *((uint16_t *)(0xC3BC)) = (uint16_t)Timer2_B0_ISR;
  *((uint16_t *)(0xC384)) = (uint16_t)USCI_A3_ISR;

#pragma diag_default=Pe767

  debug_init();
  debug_print("in main\n\r");
  
  memcpy(g_pDevCfg->cfgSMSNumbers[0].cfgReportSMS, "+919879581848", 13);

  state_init();  // Clean the state machine
  
  system_boot();
  config_init();    //test

  debug_print("\n\rSYSTEM_BOOT_SUCCESS");
  UINT bytes_written = 0;

  temp_check();
  //log_sample_to_disk(&bytes_written);
  //log_sample_web_format(&bytes_written);
  events_init();
  enable_diagnosis();
  flags.booted_device = 1;
  Extrn_UART_init();
  flags.device_init = 1;
  event_force_sample();
  debug_print("**Initialization is Complete,you can press ? with enter key for help**\r\n");
  //lcd_show();

  while(1)
  {
    lcd_show();
    debug_print("loop");
     hardware_actions();
    // added temp code
    //rtc_update_time();
    
   
    
    if(!(GSM_STATUS_ACTIVE))
    {
      flags.internet_flag = 0;
      gsm_init();
      //dev_sms_cofig();
      //check_for_OTA();
    }

    //debug_print("diagnosis");
    /*check for diagnosis*/
    if(flags.check_diagnosis)	
    {
      flags.check_diagnosis = FALSE;
      check_diagnosis_command();
      parse_diag_command();
    }
    // START_WDT_14_MIN;
#if 1

    for(uint8_t i=0; i<60; i++)
    {
      delay_sec(1);
      if (g_iHardware_actions==HWD_NOTHING)
      {
        break;
      }
    }
    rtc_update_time();
    events_run();
    
    
#endif
  }
}
