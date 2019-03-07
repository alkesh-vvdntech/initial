#include "main_system.h"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "modem.h"
#include "stringutils.h"
#include "spi_flash.h"
#include "config.h"
#include "timer.h" 
#include "fridge_data.h"
#include "fridge_uart.h"

/************* Global Variables ******************/


/************** Extern Variables**********************/

extern char device_imei[20];
extern unsigned g_uch_status;
extern void get_local_time();

/****************************************************/

/**********************************************************************************************
 * Function Name : void ack_to_fridge(void)
 * Parameters :  void
 * Return : void
 * Description : Send ack to fridge
 **********************************************************************************************/
void ack_to_fridge( char* time)
{
  char ack[100];
  uint16_t crc = 0x1D0F;
  char crc_buff[10];
  char padded_crc[4] ={"0000"} ;

  memset(ack,'\0',sizeof(ack));
  memset(crc_buff,'\0',sizeof(crc_buff));
  
  sprintf(ack,"{\"UTC\":%ld,\"ACK\":%s}",epoch_seconds,time);
  crc = crc_get(ack,strlen(ack),crc);
  itoa(crc,crc_buff,16);
  strcpy(&padded_crc[sizeof(padded_crc)-strlen(crc_buff)],crc_buff);
  strcat(ack,padded_crc); 
  strcat(ack,"#\r\n");
  ext_uart_tx(ack);
  
#ifdef _FISM_
  debug_print("\r\n");
  debug_print(ack);
  debug_print("\r\n");
#endif
}
/**********************************************************************************************
 * Function Name : void check_data_crc()
 * Parameters :  void
 * Return : void
 * Description : Calaculate fridge data's CRC and store the data in SPI flash
 **********************************************************************************************/
void check_data_crc()
{
   uint16_t calc_crc = 0x1D0F;
   char padded_crc[4] ={"0000"} ;  
   static char crc[5] = {'\0'};
   char fridge_crc[5] = {'\0'};
   char *ret_ptr=0;
   char time_stamp[20];
   char debug_buff[50];
   char buff_cpy[250];
   uint8_t count = 0;
    
   debug_print(fridge_buffer);
    
   ret_ptr = strchr(fridge_buffer,'}');
   
   if(ret_ptr)
   {
     for(count = 0; count<=6;count++)
     {
       if((ret_ptr[count+1]== '#') || (ret_ptr[count+2]== '\r') || (ret_ptr[count+2]== '\n') || (ret_ptr[count+2]== '\0'))
       {
         break;
       }
       crc[count] = ret_ptr[count+1];
     }
   }
   
 /*  ret_ptr = strstr(fridge_buffer,"0X");
   
   if(!ret_ptr)
   {
     ret_ptr = strstr(fridge_buffer,"0x");
   }
   
   if(ret_ptr)
   {
     for(count = 0; count<=6;count++)
     {
       if((ret_ptr[count+2]== '#') || (ret_ptr[count+2]== '\r') || (ret_ptr[count+2]== '\n') || (ret_ptr[count+2]== '\0'))
       {
         break;
       }
       crc[count] = ret_ptr[count+2];
     }
   }*/
    
   debug_print("\r\nParsed CRC ");
   debug_print(crc);
   debug_print("\r\n");
    
   strtok(fridge_buffer,"}");
   strcat(fridge_buffer,"}");
   debug_print("\r\nnew buff\r\n");
   debug_print(fridge_buffer);
    
   memset(buff_cpy,'\0',sizeof(buff_cpy));
   memset(time_stamp,'\0',sizeof(time_stamp));
   
   strcpy(buff_cpy,&fridge_buffer[W25Q16_PAGE_SIZE-1]);
   calc_crc = crc_get(fridge_buffer,strlen(fridge_buffer),calc_crc);
   
   itoa(calc_crc,fridge_crc,16);
   strcpy(&padded_crc[sizeof(padded_crc)-strlen(fridge_crc)],fridge_crc);
    
   memset(debug_buff,'\0',sizeof(debug_buff));
   sprintf(debug_buff,"\r\n fridge_crc is %s and calc. crc is %s\r\n",crc,padded_crc);
   debug_print(debug_buff);
    
   if(strcmp(padded_crc,crc) == 0)
   {
      ret_ptr = strstr(fridge_buffer,"\"TS\":");
      if(ret_ptr)
      {
         for(count = 0; count<=12;count++)
         {
           if(ret_ptr[count+5]== ',')
           {
             break;
           }
           time_stamp[count] = ret_ptr[count+5];
         }
      }
      debug_print("\r\ntime");
      debug_print(time_stamp);
      debug_print("\r\n");
      debug_print("\r\nFridge Data CRC Matched\r\n");
      fridge_buffer[W25Q16_PAGE_SIZE-1] = '\0';
      save_packet(fridge_buffer,flash_memory.put_at_cmd_idx,2);  
      save_packet(buff_cpy,flash_memory.put_at_cmd_idx,2);  
      ack_to_fridge(time_stamp);
   }
}
/**********************************************************************************************
 * Function Name : void fridge_sampled_data()
 * Parameters :  void
 * Return : void
 * Description : Read fridge sampled data from SPI flash
 **********************************************************************************************/
bool fridge_sampled_data()
{
    uint8_t packet_count = 0;
    char upload_buffer[W25Q16_PAGE_SIZE*2*4];
    char packet_start[100];
    uint16_t get_idx_cpy = 0;
       
    memset(upload_buffer,'\0',sizeof(upload_buffer));
    memset(packet_start,'\0',sizeof(packet_start));
    
    if(flash_memory.get_at_cmd_idx !=flash_memory.put_at_cmd_idx)
    {
      get_idx_cpy = flash_memory.get_at_cmd_idx;
      sprintf(packet_start,"{\"vId\":\"Nexleaf\",\"asm\":0,\"data\":[{\"dId\":\"%s\",\"raw\":[",device_imei);
      strcat(upload_buffer,packet_start);
      while(flash_memory.get_at_cmd_idx !=flash_memory.put_at_cmd_idx)
      {
          memset(local_storage_buff,'\0', sizeof(local_storage_buff));
          g_uch_status = read_packets(flash_memory.put_at_cmd_idx,flash_memory.get_at_cmd_idx,DEFAULT_TOTAL_PACKETS);
          if(FLASH_MEM_READ_OK == g_uch_status)
          {
            strcat(upload_buffer,local_storage_buff);
            memset(local_storage_buff,'\0',sizeof(local_storage_buff));
            flash_memory_increment_get_page_idx(flash_memory.get_at_cmd_idx,TEMP_TOTAL_PACKETS,2);
            g_uch_status = read_packets(flash_memory.put_at_cmd_idx,flash_memory.get_at_cmd_idx,DEFAULT_TOTAL_PACKETS);
            if(FLASH_MEM_READ_OK == g_uch_status)
            {
              strcat(upload_buffer,local_storage_buff);
              flash_memory_increment_get_page_idx(flash_memory.get_at_cmd_idx,TEMP_TOTAL_PACKETS,2);
            }
            else
            {
              flash_memory.get_at_cmd_idx = DEFAULT_GET_AT_CMD_IDX;
              flash_memory.put_at_cmd_idx = DEFAULT_PUT_AT_CMD_IDX;
              update_put_get_idx();
              return false;           
            }

          }
          else
          {
            flash_memory.get_at_cmd_idx = DEFAULT_GET_AT_CMD_IDX;
            flash_memory.put_at_cmd_idx = DEFAULT_PUT_AT_CMD_IDX;
            update_put_get_idx();
            return false;           
          }
          upload_buffer[strlen(upload_buffer)] = ',';
          if(++packet_count >=((unsigned char)((float)PERIOD_UPLOAD/PERIOD_SAMPLING)-2))
          {
            break;
          }
      }
      upload_buffer[strlen(upload_buffer)-1] = '\0';
      strcat(upload_buffer,"]}]}");
      
      send_data(upload_buffer);

      if(SEND_PACKET_ERROR == g_uch_status)
      {
        flash_memory.get_at_cmd_idx = get_idx_cpy;
        update_put_get_idx();
        return false;
      }
      else                // Data sent successfully through HTTP
      {
        update_put_get_idx();
        return true;
      }
    }
    else         
    {
      return false;
    }
}