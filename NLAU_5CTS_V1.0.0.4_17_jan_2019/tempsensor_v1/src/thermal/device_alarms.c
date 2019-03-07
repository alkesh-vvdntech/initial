#include "msp430.h"
#include "config.h"
#include "stdlib.h"
#include "globals.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "temperature.h"
#include "modem.h"
#include "spi_flash.h"
#include "rtc.h"
#include "timer.h"
#include "watchdog.h"
#include "device_alarms.h"
#include "main_system.h"

/************************ Extern Variables ***************************************/
extern char device_imei[20];
//extern uint32_t local_time;
extern unsigned g_uch_status;
/********************************************************************************/


/**********************************************************************************************
 * Function Name : char* battery_alarm()
 * Parameters : void
 * Return : void
 * Description :  send battery alarm with battery %
 **********************************************************************************************/
void check_battery_alarm()
{
  /*
  In the device alarm, the 'stat' field is
  0: ok (change to above 95%)
  1: warning ( change to below 80% )
  2: alarm ( change to below 20%)
  3: charging (not implemented)
  */
  char batt_alarm[250];
  char http_url[150];
  uint8_t batter_percent = 0;
  static uint8_t batt_health_check = 3;
  uint8_t stat_value = 3;
  uint8_t num = 0;
  uint32_t batt_val = 0;

  memset(http_url,'\0',sizeof(http_url));
  memset(batt_alarm,'\0',sizeof(batt_alarm));

  batter_percent = batt_getlevel();
  memset(batt_alarm,'\0',sizeof(batt_alarm));
  if(batter_percent <= 20)
  {
    stat_value = 2;
  }
  else if((batter_percent>20) &&(batter_percent <= 80))
  {
    stat_value = 1;  
  }
  else
  {
    stat_value = 0;
  }
  if((batt_health_check!=stat_value))
  {
    get_local_time();
    memset(batt_alarm,'\0',sizeof(batt_alarm));
    sprintf(batt_alarm,"{\"dId\":\"%s\",\"dvc\":{\"batt\":{\"avl\":%d,\"stat\":%d,\"time\":%ld}}}",device_imei,batter_percent,stat_value,rtc_getUTCsecs());
    save_packet(batt_alarm,flash_memory.put_error_idx,3);
    batt_health_check = stat_value;
  }
}

/**********************************************************************************************
 * Function Name : char* sensor_alarm()
 * Parameters : void
 * Return : void
 * Description :  send sensor connection status alarm
 **********************************************************************************************/
void check_sensor_alarm()
{
  if(flags.sensors_altered == 1)
  {
    char sensor_data[400];
    char sensor_status[250];
    char sensor[3] = {"CD"};
    unsigned char sensor_count = 0;

    memset(sensor_data,'\0',sizeof(sensor_data));
    memset(sensor_status,'\0',sizeof(sensor_status));

    get_local_time();
    for(sensor_count = 0;sensor_count<2;sensor_count++)
    {
      sprintf(sensor_status,"{\"dId\":\"%s\",\"dvc\":{\"xSns\":{\"stat\":%c,\"time\": %ld}},\"sId\":\"%c\"}",device_imei,temp_disconnt[sensor_count],rtc_getUTCsecs(),sensor[sensor_count]);
      strcat(sensor_data,sensor_status);
      sensor_data[strlen(sensor_data)] = ',';
      memset(sensor_status,'\0',sizeof(sensor_status));
    }
    sensor_data[strlen(sensor_data)-1] = '\0';
    save_packet(sensor_data,flash_memory.put_error_idx,3);
    flags.sensors_altered = 0;
  }
}

/**********************************************************************************************
   * Function Name	: void alarm_packets(char *alarm_type)
   * Parameters 	: char *alarm_type
   * Return 		: void
   * Description 	: save alarm packet in SPI flash
   **********************************************************************************************/
void alarm_packet(char *alarm_type)
{
  char alarm[250];

  memset(alarm,'\0',sizeof(alarm));

  sprintf(alarm,"{\"dId\":\"%s\",\"errs\":[{\"code\":\"%s\",\"time\":%ld}]}",device_imei,alarm_type,rtc_getUTCsecs());
  save_packet(alarm,flash_memory.put_error_idx,3);
}
/**********************************************************************************************
   * Function Name	: void read_ota_alarm_packet(
   * Parameters 	: void
   * Return 		: void
   * Description 	: read alrm packet if OTA has failed (CRCF/NETF)
   **********************************************************************************************/
void read_alarm_packets()
{
 if((flags.internet_flag)&&(flash_memory.get_error_idx !=flash_memory.put_error_idx))
 {
   char http_url[150];
   uint16_t error_idx_cpy = 0;
   memset(http_url,'\0',sizeof(http_url));
   sprintf(http_url,"AT+HTTPPARA=\"URL\",\"http://%s/coldtrace/intel/config/\"\r\n",flash_memory.server_url);
   if(send_cmd_wait(http_url,"OK\r\n",20,2))
   {
     char sensor_buff[300];
     while(flash_memory.get_error_idx !=flash_memory.put_error_idx)
     {
       error_idx_cpy = flash_memory.get_error_idx;
       g_uch_status = read_packets(flash_memory.put_error_idx,flash_memory.get_error_idx,ERROR_TOTAL_PACKETS);
       flash_memory_increment_get_page_idx(flash_memory.get_error_idx,ERROR_TOTAL_PACKETS,3);
       if(FLASH_MEM_READ_OK == g_uch_status)
       {
          if((strstr(local_storage_buff,"errs")!=0) ||(strstr(local_storage_buff,"batt")!=0)||(strstr(local_storage_buff,"xSns")!=0))
          {
            memset(sensor_buff,'\0',sizeof(sensor_buff));
            strcat(sensor_buff,"{\"data\":[");
            strcat(sensor_buff,local_storage_buff);
            strcat(sensor_buff,"],\"vId\":\"Nexleaf\"}");
            if(!(send_data_to_server(sensor_buff)))
            {
              flash_memory.get_error_idx = error_idx_cpy;
              update_put_get_idx();
              return;
            }
            update_put_get_idx();   // data uploaded successfully
          }
          else
          {
            flash_memory.put_error_idx = DEFAULT_PUT_ERROR_IDX;
            flash_memory.get_error_idx = DEFAULT_GET_ERROR_IDX;
            update_put_get_idx();
            return;
          }
       }
       else
       {
          flash_memory.put_error_idx = DEFAULT_PUT_ERROR_IDX;
          flash_memory.get_error_idx = DEFAULT_GET_ERROR_IDX;
          update_put_get_idx();
          return;
       }
     }
   }
 }
}