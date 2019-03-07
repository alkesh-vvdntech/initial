#include <msp430.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "spi_flash.h"
#include "config.h"
#include "temperature.h"
#include "main_system.h"
#include "watchdog.h"
#include "events.h"

char diagnosis_buff[DIAG_BUFF_LEN];
char diag_command[DIAG_BUFF_LEN];
unsigned char diag_buff_len;

extern CONFIG_SYSTEM *g_pSysCfg;
extern char ota_url[75];
extern char image_crc[10];

void debug_init()
{
  // Configure USCI_A0 for UART mode
  UCA0CTLW0 = UCSWRST;           // Put eUSCI in reset
  UCA0CTLW0 |= UCSSEL__SMCLK;    // CLK = SMCLK
  UCA0BRW = 4;
  UCA0MCTLW |= UCOS16 | UCBRF_5 |0x5500;
  UCA0CTLW0 &= ~UCSWRST;         // release from reset
}

void debug_print(char *cmd)
{
  while(*cmd!='\0')
  {
    UCA0TXBUF=*cmd++;
    while(UCA0STATW&=UCBUSY);
  }

  UCA0TXBUF='\n';
  UCA0TXBUF='\n';
  UCA0TXBUF='\r';
}       
   
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  uint8_t ch;
  flags.power_saving = 0;
  __bic_SR_register_on_exit(LPM0_bits); // Resume functionality.
  
  switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_UART_UCRXIFG:
      ch=UCA0RXBUF;
      diagnosis_buff[diag_buff_len]=ch;
      diag_buff_len=(diag_buff_len+1)%DIAG_BUFF_LEN;
      if((ch=='\r'|| ch=='\n'))
      {
        flags.check_diagnosis=TRUE;
      }
      break;

    default:   
      break;
  }
}

void enable_diagnosis()
{
  UCA0IE |= UCRXIE;
}

void disable_diagnosis()
{
  UCA0IE&= ~UCRXIE;
}

/**********************************************************************************************
 * Function Name :uint8_t check_diagnosis_command()
 * Parameters : void
 * Return : uint8_t
 * Description : Check '?' entered by user
 **********************************************************************************************/
uint8_t check_diagnosis_command()
{
  uint8_t index;
  for(index=0; index<diag_buff_len; index++)
  {
    if(diagnosis_buff[index]=='\r' || diagnosis_buff[index]=='\n')
    {
      break;
    }
    else
    {
      diag_command[index]=diagnosis_buff[index];
    }
  }
  
  diag_buff_len=0;
  memset(diagnosis_buff,'\0',DIAG_BUFF_LEN);
  return 1;
}

/**********************************************************************************************
 * Function Name :void parse_diag_command()
 * Parameters : void
 * Return : void
 * Description : Parse User choice & Diagnose
 **********************************************************************************************/

unsigned char parse_diag_command()
{
  unsigned char static ota_check = 0,modem_command = 0,config_command=0;
  char debug_buffer[150];
  if(!strcmp(diag_command,"?"))
  {
    debug_print("*****Please Enter your choice*****\r\n\r\n");
    debug_print("   1.    Check for internet\r\n\r\n");
    debug_print("   2.    Start OTA\r\n\r\n");
    debug_print("   3.    See Logs\r\n\r\n");
    debug_print("   4.    Check Signal Strenth\r\n\r\n");
    debug_print("   5.    Check Number Samples queued for Upload\r\n\r\n");
    debug_print("   6.    Check Battery Percentage \r\n\r\n");
    debug_print("   7.    Send GSM Modem Command \r\n\r\n");
    debug_print("   8.    Enter Device Configuration String\r\n\r\n");
    debug_print("   9.    Factory Reset\r\n\r\n");
    debug_print("   10.   Reboot the device\r\n\r\n");
    debug_print("   11.   Force temperature sample\r\n\r\n");
    debug_print("   12.   Force samples upload\r\n\r\n");
    debug_print("   13.   Clear Entire Storage Memory and do factory reset\r\n\r\n");
  }
  else if(!strcmp(diag_command,"1"))
  {
    debug_print("Checking for intenet, please wait...\r\n");
    check_internet();
    if(flags.internet_flag == 1)
    {
      debug_print("\r\nInternet is Active\r\n\n");
    }
    else
    {
      debug_print("\r\nInternet is Inactive\r\n\n");
    }
  }
  else if(((!strcmp(diag_command,"2"))&& ota_check == 0 ) ||(ota_check == 2))
  {
    if(ota_check == 0)
    {
      debug_print("\r\nPlease enter OTA URL with CRC...\r\n\n");
      ota_check = 2 ;
    }
    else
    {
      ota_sms(diag_command);
      ota_check = 0; 
    }
  }    
  else if(!strcmp(diag_command,"3"))
  {
    debug_print("I Am in 3\r\n");
  }

  else if(!strcmp(diag_command,"4"))
  {
    debug_print("\n\r RS 3 \n\r");
    check_radio_signal();
    memset(debug_buffer,'\0',sizeof(debug_buffer));
    sprintf(debug_buffer,"Signal Strenth is %d%\r\n",dvalue.signal_st);
    debug_print(debug_buffer);
    debug_print("\r\n");
  }
  else if(!strcmp(diag_command,"5"))
  {
    unsigned int total_packets_remain = 0;
    unsigned int remianing_memory = 0;
    total_packets_remain = flash_memory_get_count_of_saved_packets(flash_memory.put_temp_idx,flash_memory.get_temp_idx,TEMP_TOTAL_PACKETS);
    remianing_memory = TEMP_TOTAL_PACKETS - total_packets_remain;
    memset(debug_buffer,'\0',sizeof(debug_buffer));
    sprintf(debug_buffer,"Total number of samples queued for upload is %d and memory remaining is for %d packets\r\n",total_packets_remain,remianing_memory);
    debug_print(debug_buffer);
  }
  else if(!strcmp(diag_command,"6"))
  {
    batt_getlevel();	
  }

  else if(((!strcmp(diag_command,"7"))&& modem_command == 0 ) ||(modem_command == 2))
  {
    if(modem_command== 0)
    {
      debug_print("\r\nPlease Enter GSM Modem Command...\r\n\n");
      modem_command = 2 ;
    }
    else
    {
      memset(debug_buffer,'\0',sizeof(debug_buffer));
      if((strstr(diag_command,"AT+")==0) ||(strstr(diag_command,"at+")==0))
      {
        sprintf(debug_buffer,"%s\r\n",diag_command);
        send_cmd_wait(debug_buffer,"\r\n",30,2);
      }     
      modem_command = 0;
    }
  }

  else if(((!strcmp(diag_command,"8"))&& config_command == 0 ) ||(config_command == 2))
  {
    if(config_command == 0)
    {
      debug_print("\r\nPlease enter String for configuration...\r\n\n");
      config_command = 2 ;  
    }
    else
    {
      if(config_parse_configuration_sms(diag_command) == 0)
      {
        event_force_event_by_id(EVT_UPLOAD_SAMPLES, 0);
        device_ready_http();
        state_alarm_reset_all();
        event_force_event_by_id(EVT_SUBSAMPLE_TEMP, 0);
      }
      config_command = 0;
      flags.param_updated = 0;
    }
  }
  else if(!strcmp(diag_command,"9"))
  {
     g_pSysCfg->memoryInitialized = 0xFF;           
     PMM_trigBOR();
  }
  else if(!strcmp(diag_command,"10"))
  {
    PMM_trigBOR();
  }
  else if(!strcmp(diag_command,"11"))
  {
    event_force_event_by_id(EVT_SUBSAMPLE_TEMP, 0);
  }
  else if(!strcmp(diag_command,"12"))
  {
    event_force_upload(0);
  }
  else if(!strcmp(diag_command,"13"))
  {
    uint8_t erase_chip = 0xff;
    fram_write(0x1800,(char *)&erase_chip,1);
    g_pSysCfg->memoryInitialized = 0xFF;
    config_default_configuration();
    PMM_trigBOR();
  }
  else
  {
    debug_print("ERROR\r\n");
    debug_print("Please Enter ? for Help.\r\n");	
  }
  enable_diagnosis();
  memset(diag_command,'\0',DIAG_BUFF_LEN);
}