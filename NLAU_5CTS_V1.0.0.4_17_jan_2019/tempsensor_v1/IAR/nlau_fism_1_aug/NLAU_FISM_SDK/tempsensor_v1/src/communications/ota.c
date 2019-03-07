/***********************************HEADER FILES************************************************/
#include "modem.h"
#include "modem_uart.h"
#include "main_system.h"
#include "ota.h"
#include "debug.h"
#include "sms.h"
#include "crc.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "watchdog.h"
#include "string.h"
#include "config.h"
#include "events.h"
#include "data_transmit.h"
#include "spi_flash.h"

#define DEBUG

/************ Global Variables *******************/

unsigned char fw_version[] = {"V2.0.5"};

/*********** Extern variables *******************/

extern void get_local_time();
extern char ota_url[75];
extern char image_crc[10];
extern char device_imei[20];
extern unsigned g_uch_status;
extern CONFIG_SYSTEM *g_pSysCfg;
/************************************************/
/**********************************************************************************************
   * File Name		: ota.c
   * Function Name	: check_for_OTA(void)
   * Parameters 	: None
   * Return 		: None
   * Description 	: Check for OTA
   **********************************************************************************************/
void check_for_OTA(void)
{
    if((flags.check_ota) &&(flags.internet_flag))
    {
      if(batt_getlevel() >= 10)
      {
          disable_diagnosis();
          unsigned char to_boot = 0;
          if(ota_process())
          {
              flash_memory.put_error_idx = DEFAULT_PUT_ERROR_IDX;
              flash_memory.put_default_idx = DEFAULT_PUT_DEFAULT_IDX;

              flash_memory.get_error_idx = DEFAULT_GET_ERROR_IDX;
              flash_memory.get_default_idx = DEFAULT_GET_DEFAULT_IDX;
             
              update_put_get_idx();
              
              g_pSysCfg->memoryInitialized = 0xFF;    
              to_boot = 3;
              fram_write(0x1980,(char *)&to_boot,1);
#ifdef _WDT_
              watchdog_disable();
#endif
              debug_print("\r\njumping to bootloader\r\n");
              __bic_SR_register(GIE);
              delay_sec(1);
              TI_MSPBoot_JumpToBoot();
          } 
          else
          {
              enable_diagnosis();
              delete_sms();
              alarm_sms(FWU_FAILURE, 0);
              flags.check_ota = 0;
          }
      }
    }
}
 /**********************************************************************************************
   * File Name		: ota.c
   * Function Name	: unsigned int CRC_check(char * buffer)
   * Parameters 	: char * buffer
   * Return 		: crc
   * Description 	: calculate the CRC of the buffer
   **********************************************************************************************/
unsigned int CRC_check(char * buffer)
{
    unsigned int l_index = 0, temp;
    static unsigned int CRC = 0;

    for (l_index = 0;l_index<(strlen(buffer)); l_index++)
    {
      if (buffer[l_index] > ' ')
	  {
        temp = buffer[l_index];
        CRC = (CRC + temp) % 256;   // 8 bit CRC
      }
    }
    return CRC;
  }
/**********************************************************************************************
* File Name     : ota.c
* Function Name	: unsigned char ota_process(void)
* Parameters 	: None
* Return        : download_done
* Description 	: start the OTA
**********************************************************************************************/
unsigned char ota_process(void)
{
  char * ptr_at_responce,*rsp,*ota_check;
  uint32_t pkt_length;
  uint16_t byte_to_read;
  char debug_buff[100];
  //char tmp_str[16];
  unsigned char ota_status = 0;
  char local_buff[BUFFER_MAX_SIZE];
  char at_cmd_str[32];
  uint32_t http_break = 0;
  char fw_version_cloud[10];
  char download_done = 0, first_check = 1;
  char http_url[200];
  unsigned int page=0;;
  char cloud_crc[10] = {0};
  char file_size_buff[50];
  uint32_t file_size = 0;
  uint32_t check_image_size = 0;
  uint8_t state,len=0;
  char * ptr;
  unsigned char ota_failure = 0;

  uint16_t crc = 0xFFFF,downloaded_crc = 0;

  send_cmd_wait(GSM_BUSY_1, "OK\r\n",20,2);
  send_cmd_wait(DETACH_BEARER, "OK\r\n", 60, 3);
  send_cmd_wait(CONTYPE, "OK\r\n", 60, 3);
  sprintf(local_buff,"AT+SAPBR=3,1,\"APN\",\"%s\"\r\n",device_apn);
  send_cmd_wait(local_buff, "OK\r\n",20,2);

  send_cmd_wait(SET_BEARER, "OK\r\n", 60, 1);
  send_cmd_wait(CHECK_SAP_IP, "OK\r\n", 60,1);


  flash_memory.put_default_idx = 3096;

  memset(local_buff,'\0',sizeof(local_buff));
  /* setup- http parameter */
  http_init();
  memset(http_url,'\0',sizeof(http_url));
  sprintf(http_url,"AT+HTTPPARA=\"URL\",\"%s\"\r\n",ota_url);
  send_cmd_wait(http_url, "OK\r\n",20,2);


  //send_cmd_wait("AT+HTTPPARA=\"URL\",\"http://in.coldtrace.org/fism_uploads/uploads/FISM_APP.txt\"\r\n", "OK\r\n",20,2);
  // Image on cloud is as
  //v1.0.2*070$@44048013068F0C43B013C669B013EEA86A14B1001A00B240805A
  /*
  FW version--- v1.0.2*
  CRC------- 070$
  */
#ifdef _WDT_
              START_WDT_14_MIN;
#endif
	while (0 == download_done)
	{          
           if(send_cmd_wait("AT+HTTPACTION=0\r\n", ",", 150,3))
           {
               if(ota_check=strstr(GSM_buffer,",200,"))
               {
                   ptr_at_responce=strtok(ota_check,",");
                  // ptr_at_responce=strtok(NULL,",");
                   ptr_at_responce=strtok(NULL,"\r\n");
                   file_size = atol(ptr_at_responce);

                   #ifdef DEBUG
                   debug_print(ptr_at_responce);
                   debug_print("\r\n");
                   #endif
               }
               else
               {
                 alarm_packet("NETF");
                 return 0;
               }
             }
             else
             {
               alarm_packet("NETF");
               return 0;
             }
             while (file_size>0)
             {
                 memset(at_cmd_str,'\0',sizeof(at_cmd_str));
                 sprintf(at_cmd_str,"AT+HTTPREAD=%ld,250\r\n",http_break);
                 if(send_cmd_wait(at_cmd_str, "+HTTPREAD", 60, 2))
                 {
                 ptr_at_responce=strtok(GSM_buffer,"\n");
                 ptr_at_responce=strtok(NULL,"\n");
                 ptr_at_responce=strtok(NULL,"\r");
                 memset(local_buff,'\0',sizeof(local_buff));
                 file_size = file_size - 250;
                 http_break = http_break + 250;

                  #ifdef DEBUG
                  memset(file_size_buff,'\0',sizeof(file_size_buff));
                  sprintf(file_size_buff,"Download remain :%ld bytes\r\n",file_size);
                  debug_print(file_size_buff);				// new firmware is available
                  #endif

                 if(first_check)
                 {
                     first_check=0;
                     memset(fw_version_cloud,'\0',sizeof(fw_version_cloud));
                     memset(cloud_crc,'\0',sizeof(cloud_crc));
                     rsp=strtok(ptr_at_responce,"*");
                     strcpy(fw_version_cloud,rsp);
                     rsp=strtok(NULL,"$");
                     strcpy(cloud_crc,rsp);
                     if(strcmp(cloud_crc,image_crc)!=0)
                     {
                        alarm_packet("CRCF");
                        return 0;
                     }
                     rsp=strtok(NULL,"\r");
                     strcpy(local_buff,rsp);
                     #ifdef DEBUG
                     debug_print(fw_version_cloud);
                     debug_print("\r\n");
                     debug_print(cloud_crc);
                     debug_print("\r\n");
                     #endif
                     if(strcmp(fw_version,fw_version_cloud))
                     {
                        #ifdef DEBUG
                        debug_print("OTA Started\r\n");				// new firmware is available
                        #endif
                     }
                     else
                     {
                        delete_sms();
                        return 0;
                     }
                   }
                   else
                   {
                      strcpy(local_buff,ptr_at_responce);
                   }
                   crc = crc_get(local_buff,strlen(local_buff),crc);
                   #ifdef DEBUG
                   sprintf(at_cmd_str,"CRC of packet:%x\r\n",crc);
                   debug_print(at_cmd_str);
                   #endif
                  ota_status =  flash_memory_save_OTA_image(local_buff);
                  if(ota_status == 0)
                  {
                    break;
                  }
               }
               else
               {
                  alarm_packet("NETF");
                  return 0;
               }
             }

            downloaded_crc = strtol(cloud_crc,&ptr,16);
            if ((downloaded_crc == crc) && (crc != 0) && (downloaded_crc) != 0)
            {
                #ifdef DEBUG
                sprintf(at_cmd_str,"CRC:%x\r\n",crc);
                debug_print(at_cmd_str);
                debug_print(cloud_crc);
                debug_print("\r\n");
                debug_print("CRC Pass\r\n");				// new firmware is available
                #endif
                download_done = 1;     // CRC is matched
                delete_sms();
            }
            else
            {
                #ifdef DEBUG
                sprintf(at_cmd_str,"CRC:%x\r\n",crc);
                debug_print(at_cmd_str);
                debug_print(cloud_crc);
                debug_print("\r\n");
                debug_print("CRC Fail\r\n");				// new firmware is available
                #endif
                alarm_packet("CRCF");
                download_done = 0;     // CRC match failed
            }
            return download_done;
	}
  return download_done;
}
