#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "debug.h"
#include "spi_flash.h"
#include "stdint.h"
#include "modem_uart.h"
#include "modem.h"
#include "battery.h"
#include "main_system.h"
#include "temperature.h"
#include "watchdog.h"
#include "time.h"
#include "timer.h"
#include "config.h"
#include "events.h"
#include "rtc.h"
#include "device_alarms.h"
#include "fridge_uart.h"


char* fridge_new;        
struct tm 		date_time_int = { 0 };

void add_sec_in_date_time(char *ptr_data_time_str,signed int second);
void convert_utc_time(char *date_time_str, signed int gmt_time_zone);
void get_local_time();
unsigned char break_data(char *buffer, unsigned char gprmc_index);
unsigned char send_data_to_server_wifi_gprs_gsm(char *packet);
void set_rtc_val(char *buff);
extern char device_imei[20];
extern CONFIG_DEVICE *g_pDevCfg;
extern struct tm g_tmCurrTime;
extern unsigned char fw_version[];
extern char ccid[22];
extern char gsm_fw[20];

void append_data_in_buffer()
{
    char temp_data[250];
    unsigned char batt_percent= 0;
    char sensor[3] = {"AB"};
    unsigned char sensor_count = 0;
    char sampled_data[300];

    memset(sampled_data,'\0',sizeof(sampled_data));
    batt_percent = batt_getlevel();

    get_local_time();

    for(sensor_count = 0;sensor_count<2;sensor_count++)
    {
	if(temp_disconnt[sensor_count]== '0')
	{
	    memset(temp_data,'\0',sizeof(temp_data));
	    sprintf(temp_data,"{\"sId\":\"%c\",\"time\":%ld,\"typ\":%d,\"tmp\":%s}",sensor[sensor_count],rtc_getUTCsecs(),0,dvalue.sensor[sensor_count].value); //shubham
	    strcat(sampled_data,temp_data);
	    sampled_data[strlen(sampled_data)] = ',';
	    if((dvalue.sensor[sensor_count].value != '\0') && sensor_count==1)
	    {
		sensor_count = 0;
	    }
	}
	memset(temp_data,'\0',sizeof(temp_data));
	sprintf(temp_data,"{\"sId\":\"pwa\",\"stat\":%d,\"time\":%ld}",flags.batt_connected,rtc_getUTCsecs());
	strcat(sampled_data,temp_data);
	save_packet(sampled_data,flash_memory.put_temp_idx,1);
    }
}

unsigned char send_data_to_server(char *packet)
{
    char *token;
    int value =0;

    if(flags.internet_flag == 1)
    {
      char http_data[150];
      unsigned int len =0;
      unsigned char ret_status = 0;
      unsigned char retry = 0;

    /*  debug_print("\r\n***");
    debug_print(packet);
    debug_print("****\r\n");
	return 0;*/

      memset(http_data,'\0',sizeof(http_data));

      while(retry++ < 3)
      {
        len = strlen(packet);
        sprintf(http_data,"AT+HTTPDATA=%d,12000\r\n",len);
        if(send_cmd_wait(http_data,"DOWNLOAD\r\n",20,1))
        {
          if(send_cmd_wait(packet, "OK",10,1))
          {
            delay_sec(1);
            if (send_cmd_wait("AT+HTTPACTION=1\r\n", ",",150,1))
            {
              if(strstr(GSM_buffer,"+HTTPACTION: 1,303"))
              {
                debug_print("configure device http\n\r");
                event_force_event_by_id(EVT_ALARMS_CHECK, 0);
                flags.configure_http = 1;
                ret_status = 1;
                flags.internet_flag = 1;
                return ret_status;
              }
              else if(strstr(GSM_buffer,"+HTTPACTION: 1,20"))
              {
                ret_status = 1;
                flags.internet_flag = 1;
                return ret_status;
              }
            }
          }
        }
      }
      return ret_status;
    }
    else
    {
     return 0;
    }
}

unsigned char send_data_to_server_wifi_gprs_gsm(char *packet)
{
    char *token;
    int value =0;

    if(flags.internet_flag == 1)
    {
      char http_data[150];
      unsigned int len =0;
      unsigned char ret_status = 0;
      unsigned char retry = 0;

      memset(http_data,'\0',sizeof(http_data));
      while(retry++ < 3)
      {
        ///lcd_print
	  lcd_printl(0, "GPRS Trial");
	  len = strlen(packet);
	  sprintf(http_data,"AT+HTTPDATA=%d,12000\r\n",len);
	  if(send_cmd_wait(http_data,"DOWNLOAD\r\n",20,1))
	  {
	      if(send_cmd_wait(packet, "OK",10,1))
	      {
		  delay_sec(1);
		  if (send_cmd_wait("AT+HTTPACTION=1\r\n", ",",150,1))
		  {
		      if(strstr(GSM_buffer,"+HTTPACTION: 1,303"))
		      {
			  debug_print("configure device http\n\r");
			  event_force_event_by_id(EVT_ALARMS_CHECK, 0);
			  flags.configure_http = 1;
			  ret_status = 1;
			  flags.internet_flag = 1;
			  return ret_status;
		      }
		      else if(strstr(GSM_buffer,"+HTTPACTION: 1,20"))
		      {
			  ret_status = 1;
			  flags.internet_flag = 1;
			  return ret_status;
		      }
		      else if(strstr(GSM_buffer,"+HTTPACTION: 1,603"))
		      {
			  debug_print("--URL Error--\n\r");
			  ret_status = 0;
			  flags.internet_flag = 1;
			  return ret_status;
		      }
		  }
	      }
	  }
      }
      return ret_status;
    }
    else
    {
     return 0;
    }
}

uint8_t device_ready_http()
{
  if(flags.internet_flag == 1)
  {
    char http_url[150];
    char http_ready[300];

    memset(http_url,'\0',sizeof(http_url));
    memset(http_ready,'\0',sizeof(http_ready));

    sprintf(http_ready,"{\"data\":[{\"dId\":\"%s\",\"asm\":0,\"actSns\":[\"A\",\"B\",\"C\"],\"dev\":{\"imei\":\"%s\",\"mVr\":\"103.230.39.231\",\"mdl\":\"FISM\",\"dVr\":\"%s\"},\"sim\":{\"phn\":\"\",\"sid\":\"%s\"}}],\"vId\":\"Nexleaf\"}",device_imei,device_imei,/*gsm_fw,*/fw_version,ccid);
    sprintf(http_url,"AT+HTTPPARA=\"URL\",\"http://%s/coldtrace/intel/config/\"\r\n",flash_memory.server_url);

    send_cmd_wait(http_url,"OK\r\n",20,2);

    debug_print("http_ready = ");
    
    debug_print(http_ready);

    if(send_data_to_server(http_ready))
    {
      return 1;
    }
  }
   return 0;
}
unsigned char break_data(char *buffer, unsigned char gprmc_index)
{
    unsigned char i=0,count=0;

    for(i=0; i < gprmc_index; i++)
    {
        while(buffer[count++] != ',')
        {
            if(count >= 300)
            {
            	break;
            }
        }
    }
    return count;
}

unsigned char parse_input_value_for_sms()
{
    char no_error = 0;
    char *ret;

    char encoded_tempA[8];
    char encoded_tempB[8];
    char encoded_tempC[8];
    char epoch_time[12];
    char encoded_batt[4];
    char encoded_batt_percent[5];
    char local_batt[4];
    char local_batt_percent[5];

    unsigned char temp=0;
    unsigned char len = 0;

    memset(encoded_tempA,'\0',sizeof(encoded_tempA));
    memset(encoded_tempB,'\0',sizeof(encoded_tempB));
    memset(encoded_tempC,'\0',sizeof(encoded_tempC));
    memset(encoded_batt,'\0',sizeof(encoded_batt));
    memset(encoded_batt_percent,'\0',sizeof(encoded_batt_percent));
    memset(epoch_time,'\0',sizeof(epoch_time));

    memset(dvalue.sensor[0].value,'\0',sizeof(dvalue.sensor[0].value));
    memset(dvalue.sensor[1].value,'\0',sizeof(dvalue.sensor[1].value));
    memset(dvalue.sensor[2].value,'\0',sizeof(dvalue.sensor[2].value));
    memset(local_batt,'\0',sizeof(local_batt));
    memset(local_batt_percent,'\0',sizeof(local_batt_percent));

    strcpy(dvalue.sensor[0].value,"--");
    strcpy(dvalue.sensor[1].value,"--");

    ret = strstr(local_storage_buff,"sId");
    if(ret)
    {
        ret = strstr(local_storage_buff,"\"sId\":\"A\"");
        if(ret)
        {
          memset(dvalue.sensor[0].value,'\0',sizeof(dvalue.sensor[0].value));

          temp = break_data(ret,2);

          for(len=0; len<6; len++)
          {
              if((ret[temp+len+6] == ',') || (ret[temp+len+6] == '}') || (ret[temp+len+6] == '{'))
              {
                  break;
              }
              dvalue.sensor[0].value[len] = ret[temp+len+6];
          }
        }
        ret = strstr(local_storage_buff,"\"sId\":\"B\"");
        if(ret)
        {
          memset(dvalue.sensor[1].value,'\0',sizeof(dvalue.sensor[1].value));

          temp = break_data(ret,2);

          for(len=0; len<6; len++)
          {
              if((ret[temp+len+6] == ',') || (ret[temp+len+6] == '}') || (ret[temp+len+6] == '{'))
              {
                  break;
              }
              dvalue.sensor[1].value[len] = ret[temp+len+6];
          }
        }

        ret = strstr(local_storage_buff,"\"sId\":\"C\"");
        if(ret)
        {

          temp = break_data(ret,2);

          for(len=0; len<6; len++)
          {
              if((ret[temp+len+6] == ',') || (ret[temp+len+6] == '}') || (ret[temp+len+6] == '{'))
              {
                  break;
              }
              dvalue.sensor[2].value[len] = ret[temp+len+6];
          }
        }

        ret = strstr(local_storage_buff,"batt");
        if(ret)
        {
          temp = break_data(ret,1);
          memset(local_batt,'\0',sizeof(local_batt));

          for(len=0; len<6; len++)
          {
              if((ret[temp+len+6] == ',') || (ret[temp+len+6] == '}') || (ret[temp+len+6] == '{'))
              {
                  break;
              }
              local_batt[len] = ret[temp+len+6];
          }

          temp = break_data(ret,0);
          memset(local_batt_percent,'\0',sizeof(local_batt_percent));

          for(len=0; len<6; len++)
          {
              if((ret[temp+len+6] == ',') || (ret[temp+len+6] == '}') || (ret[temp+len+6] == '{'))
              {
                  break;
              }
              local_batt_percent[len] = ret[temp+len+6];
          }

          temp = break_data(ret,3);

          for(len=0; len<16; len++)
          {
              if((ret[temp+len+7] == ',') || (ret[temp+len+7] == '}') || (ret[temp+len+7] == '{'))
              {
                  break;
              }
              epoch_time[len] = ret[temp+len+7];
          }
        }

#if 0
        debug_print("\r\ntime");
        debug_print(epoch_time);
        debug_print("\r\n");
        char bufff[100];
        memset(bufff,'\0',sizeof(bufff));
        sprintf(bufff,"Sensor A %s Sensor B %s Sensor C %s",dvalue.sensor[0].value,dvalue.sensor[1].value,dvalue.sensor[2].value);
        debug_print(bufff);
        debug_print("\r\nbatt %");
        debug_print(local_batt_percent);
        debug_print("\r\n batt status");
        debug_print(local_batt);
        debug_print("\r\n");
#endif
        encode_value(dvalue.sensor[0].value,encoded_tempA);
        encode_value(dvalue.sensor[1].value,encoded_tempB);
        encode_value(dvalue.sensor[2].value,encoded_tempC);
        encode_value(local_batt,encoded_batt);
        encode_value(local_batt_percent,encoded_batt_percent);

        sprintf(local_storage_buff,"cltr i=%s,v=1,t=0,ty=0,tm=%s",device_imei,epoch_time);

        strcat(local_storage_buff,":");
        if(strcmp(dvalue.sensor[0].value,"--")!=0)
        {
          strcat(local_storage_buff,encoded_tempA);
        }
        strcat(local_storage_buff,":");
        if(strcmp(dvalue.sensor[1].value,"--")!=0)
        {
          strcat(local_storage_buff,encoded_tempB);
        }
        strcat(local_storage_buff,":");
        strcat(local_storage_buff,encoded_tempC);
        strcat(local_storage_buff,":");
        strcat(local_storage_buff,encoded_batt);
        strcat(local_storage_buff,":");
        strcat(local_storage_buff,encoded_batt_percent);
        strcat(local_storage_buff,";");
        no_error = 1;
    }
    return no_error;
#if 1
    debug_print("\r\n");
    debug_print(local_storage_buff);
    debug_print("\r\n");
#endif
}
/**********************************************************************************************
 * Function Name : void get_local_time()
 * Parameters : void
 * Return : void
 * Description :Get time from GSM rtc
 **********************************************************************************************/

void get_local_time()
{
  char time[20];                     //ap
  char gsm_time[20];
  char buff[2];
  char *resp;
  char time_index=0,g_time_index=0;
  char time_zone[3];
  int8_t timeZoneOffset = 0;
  int time_val[6] = {0};

  memset(time,'\0',sizeof(time));
  memset(time_zone,'\0',sizeof(time_zone));
  memset(gsm_time,'\0',sizeof(gsm_time));

  if((!flags.time_fetched) || (!flags.device_init))
  {
    resp = send_cmd_wait("AT+CCLK?\r\n","OK\r\n",10,2);
    if(resp)
    {
      if(send_cmd_wait("AT+CCLK?\r\n","OK\r\n",10,2))
      {
          resp=strstr(GSM_buffer,"\"");
          memset(dvalue.date_time,'\0',sizeof(dvalue.date_time));
          strncpy(dvalue.date_time,resp+1,20);
          strncpy(time,resp+1,20);
      }
      while(time[time_index])
      {
          if((time[time_index]=='/') ||(time[time_index]==':') ||(time[time_index]==','))
          {

          }
          else if((time[time_index]=='+') || (time[time_index]=='-'))
          {
            if(time[time_index]=='-')
            {
              flags.ng_time_zone = 1;
            }
            time_zone[0] = time[time_index+1];
            time_zone[1] = time[time_index+2];
            break;
          }
          else
          {
            gsm_time[g_time_index++]=time[time_index];
          }
          time_index++;
      }
      gsm_time[g_time_index]='\0';
      debug_print("gsm_time_check\n\r");
      debug_print(gsm_time);
      set_rtc_val(gsm_time);
      timeZoneOffset = atoi(time_zone);
      timeZoneOffset = flags.ng_time_zone == 0 ? (-1 * timeZoneOffset) : timeZoneOffset ; //quarter hour increments
      memset(time,'\0',sizeof(time));
      sprintf(time,"offset %d\r\n",(timeZoneOffset));
      debug_print("\n\rtimecheck\n\r");
      debug_print(time);
      convert_utc_time(gsm_time,timeZoneOffset);
    }
  }
  memset(time,'\0',sizeof(time));
  sprintf(time,"sec  %ld\r\n",rtc_getUTCsecs());
  debug_print(time);
}

/************************************************************************************
 * File Name 		: utc_time_converter.c
 * Function Name 	: void convert_utc_time(char * date_time_str, int gmt_time_zone)
 * Parameters 		: *date_time -> data and time information in ASCII as YYMMDDHHMMSS
 *            		: gmt_time_zone -> time zone
 * Return 			: none
 * Description 		: convert UTC-time according to current GMT time zone.
 ***********************************************************************************/
/*
 * strftime Explanation:-
 * Refer : http://www.epochconverter.com/programming/c
 * length = strftime(string,maxlen,format,timestruct);
 * Where:
 * char *string; : points to an area of memory that is big enough to hold the data produced by "strftime". Information will be written into this area as a string.
 * size_t maxlen; : is the maximum number of characters that "strftime" should write into "*string". This includes the '\0' on the end of the string.
 * const char *format; : is a format string indicating what pieces of information "strftime" should write into "*string". More details are given below.
 * const struct tm *timestruct; : points to a time structure as created by "gmtime" or "localtime". "strftime" uses this time structure as the source of information to be written into "*string".
 * size_t length; : is the number of characters written into "*string" (not counting the '\0' character that ends the string). If "strftime" fails (e.g. if the requested string would be longer than "maxlen" characters), the return value will be zero, and the contents of "*string" will be undefined.
*/
void convert_utc_time(char *date_time_str, signed int gmt_time_zone)
{
	signed int seconds_in_gmt_time_zone = 0;

	// convert gmt time zone value into seconds
	seconds_in_gmt_time_zone = (gmt_time_zone * 15 * 60);
	add_sec_in_date_time(date_time_str, seconds_in_gmt_time_zone);
}
/**********************************************************************************************
 * File Name 		: utc_time_converter.c
 * Function Name 	: void add_sec_in_date_time(char *ptr_data_time_str)
 * Parameters 		: *ptr_data_time_str - pointer to "packet_format.Date_time" parameter
 * 					  data and time information in ASCII as YYMMDDHHMMSS
 * 					: seconds -
 * Return 			: None
 * Description 		: increment data and time in "packet_format.Date_time" parameter according to
 * 					  "second" argument
 **********************************************************************************************/
void add_sec_in_date_time(char *ptr_data_time_str,signed int second)
{
	time_t 			epoch_time = 0;
	struct tm 		*ptr_date_time_int = { 0 };

	// Convert year string (i.e."YY") into integer
	date_time_int.tm_year = (ptr_data_time_str[0] - '0') * 10;
	date_time_int.tm_year += (ptr_data_time_str[1] - '0');

	//tm_year variable accept year value as years since 1900
	date_time_int.tm_year += 2000;
	date_time_int.tm_year -= 1900;

	// Convert month string (i.e."MM") into integer
	date_time_int.tm_mon = (ptr_data_time_str[2] - '0') * 10;
	date_time_int.tm_mon += (ptr_data_time_str[3] - '0');

	//tm_mon variable accept month value in range- [00:11]
	date_time_int.tm_mon = date_time_int.tm_mon - 1;

	// Convert day string (i.e."DD") into integer
	//tm_mday -> range- [01:31]
	date_time_int.tm_mday = (ptr_data_time_str[4] - '0') * 10;
	date_time_int.tm_mday += (ptr_data_time_str[5] - '0');

	// Convert hour string (i.e."HH") into integer
	//tm_hour -> range- [00:23]
	date_time_int.tm_hour = (ptr_data_time_str[6] - '0') * 10;
	date_time_int.tm_hour += (ptr_data_time_str[7] - '0');

	// Convert minute string (i.e."MM") into integer
	//tm_hour -> range- [00:59]
	date_time_int.tm_min = (ptr_data_time_str[8] - '0') * 10;
	date_time_int.tm_min += (ptr_data_time_str[9] - '0');

	// Convert second string (i.e."SS") into integer
	date_time_int.tm_sec = (ptr_data_time_str[10] - '0') * 10;
	date_time_int.tm_sec += (ptr_data_time_str[11] - '0');
        char check[20];
        //sprintf(check,"%d-%d-%d",date_time_int.tm_mday,date_time_int.tm_mon,date_time_int.tm_year);
        sprintf(check,"%d",date_time_int.tm_mon);
        //check[]=itoa_pad((uint32_t)date_time_int.tm_mon)
        //debug_print(itoa_pad((uint32_t)date_time_int.tm_mon));
         debug_print(check);
//          memcpy((void *)&RTCclk, (void *)&RTCclkBuffert, sizeof RTCclk);
	// get epoch-time
        epoch_time = 0;
	epoch_time = mktime(&date_time_int);

	// update epoch time according to gmt time zone
	epoch_time = epoch_time + second;
       // local_time = epoch_time;

        epoch_seconds = epoch_time;
        flags.time_fetched = 1;
	// convert epoch time into human readable date & time
	//ptr_date_time_int = localtime(&epoch_time);

	// convert data & time into string
	//strftime(ptr_data_time_str, 12, "%y%m%d%H%M%S", ptr_date_time_int);
}

void set_rtc_val(char *buff)
{
int rtc_val[6] = {NULL};
char val[3] = {NULL};
char index1=0,index2=0,count=0;

memset(val,'\0',sizeof(val));

for(count = 0;count<strlen(buff);count++)
{
val[index1++] = buff[count];
if(index1 > 1)
{
debug_print("updated vals\r\n");
debug_print(val);
debug_print("\r\n");
rtc_val[index2++] = atoi(val);
memset(val,'\0',sizeof(val));
index1 = 0;
}
}
g_tmCurrTime.tm_year = rtc_val[0];
g_tmCurrTime.tm_mon = rtc_val[1];
g_tmCurrTime.tm_mday = rtc_val[2];
g_tmCurrTime.tm_hour = rtc_val[3];
g_tmCurrTime.tm_min = rtc_val[4];
g_tmCurrTime.tm_sec = rtc_val[5];

rtc_init(&g_tmCurrTime); //ad rachb
}