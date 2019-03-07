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
 char buff0[15]; 
 int i;
 //char j_resp[40];
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
#if 0
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
#endif
/**********************************************************************************************
 * Function Name : void check_data_crc()
 * Parameters :  void
 * Return : void
 * Description : Calaculate fridge data's CRC and store the data in SPI flash
 **********************************************************************************************/
#if 0
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
      //ack_to_fridge(time_stamp);
   }
}
#endif
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
      //
      //sprintf(j_resp,"{\"main_s\":%d,\"sv\":%f,\"pcb\":%f,\"cs\":%d,\"bco\":%f,\"bci\":%f}",resp_json.ms,resp_json.sv,resp_json.pcb,resp_json.cs,resp_json.co,resp_json.ci);
     // sprintf(j_resp,"{\"main_s\":%d,\"sv\":%f}",resp_json.ms,resp_json.sv);
      //debug_print(j_resp);
      
           
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
/**********************************************************************************************************
*
*    Fridge testing through switch cases.
*    addition: Shubham
*
*
*********************************************************************************************************/

// sprintf(temp_data,"{\"sId\":\"P\",\"batt\":%d,\"pwr\":%d,\"ss\":%d,\"time\":%ld}",batt_percent,flags.batt_connected,dvalue.signal_st,rtc_getUTCsecs());


void Fridge_test(uint8_t *buff_new)
   



// char buff0[50];
        {     
          
//sprintf(j_resp,"{\"main_s\":%d,\"sv\":%f,\"pcb\":%f,\"cs\":%d,\"bco\":%f,\"bci\":%f}",resp_json.ms,resp_json.sv,resp_json.pcb,resp_json.cs,resp_json.co,resp_json.ci);
             
              
              //memset(fridge_buffer, '\0', sizeof(fridge_buffer));
              //uint8_t buff[10]  = {0x01,0x03,0x8C,0x3E,0x00,0x01,0xCF,0x56,0x23};
//              uint8_t buff[]  = "0x01,0x03,0x8C,0x3E,0x00,0x01,0xCF,0x56,0x23";
 //             uint8_t buff1[10]={0x01,0x03,0x8C,0x3E,0x00,0x01,0xCF,0x56,0x23};
                // uint8_t buff3[10]={0x01,0x03,0x89,0xE3,0x00,0x01,0x5F,0xA0,0x23};    //FAN SPEED
               //uint8_t buff3[]  = {0x01,0x03,0x89,0x1A,0x00,0x01,0x8F,0x91,0x23};
               //uint8_t buff3[]  = {0x01,0x03,0x8A,0x48,0x00,0x01,0x2E,0x04,0x23};   //main switch
               fridge_index = 0;
               
             // P2OUT &= ~(BIT5);     
              //P2OUT |=(BIT6);
              
              ext_uart_tx(buff_new);
              __delay_cycles(8000000);
              
              //sprintf(buff,"%x-%x-%x-%x-%x-%x-%x-%x-%x",buff1[0],buff1[1],buff1[2],buff1[3],buff1[4],buff1[5],buff1[6],buff1[7],buff1[8]);
              sprintf(buff0,"%x-%x-%x-%x-%x-%x-%x-%x-%x",buff_new[0],buff_new[1],buff_new[2],buff_new[3],buff_new[4],buff_new[5],buff_new[6],buff_new[7],buff_new[8]);
            
              debug_print(buff0);
              __delay_cycles(8000000);
              print_response();
                
             if(fridge_index >=7)
        {
          memset(fridge_buffer,'\0',sizeof(fridge_buffer));
          fridge_index = 0;
        }
        }
/*****************************************************************************************************
****************************************************************************************************
Description: Samples of Fridge response
1.MAIN SWITCH
2.PCB TEMPERATURE
3.SUPPLY VOLTAGE                       
4.COMPRESSOR SPEED
////////////////////////////////Addition shubham
***************************************************************************************************/

void sampled_fridge_response()
          
{
for(i=1;i<=9;i++)
         {   
         switch(i)
      
         {
         case 1: //voltage scaling
           
           debug_print("VOLTAGE SCALING\r\n");
           uint8_t buff4[10]={0x01,0x03,0x89,0xE9,0x00,0x01,0x7F,0xA2,0x23};    //voltage scaling
	   Fridge_test(buff4);
           debug_print("\n\r");
           voltage_scaling();
                          
           //debug_print("value of Main switch\r\n");
           //main_switch();
           //parsing_response();
           break;
        
         
         case 2: //PCB TEMPERATURE 
           
         debug_print("PCB TEMPERATURE\r\n");
         uint8_t buff5[10]={0x01,0x03,0x89,0x1A,0x00,0x01,0x8F,0x91,0x23};    //PCB TEMPERATURE
	 Fridge_test(buff5);
         debug_print("\n\r");
         pcb_temp();
         //debug_print("value of PCB temperature\r\n");
         //parsing_response();
         break;
         
         
         case 3: // SUPPLY VOLTAGE
           
         debug_print("SUPPLY VOLTAGE\r\n");
         uint8_t buff6[10]={0x01,0x03,0x8A,0x47,0x00,0x01,0x1E,0x07,0x23};     //Supply voltage 
	 Fridge_test(buff6);
         debug_print("\n\r");
         debug_print("value of Supply Voltage");
         supply_voltage();
         //parsing_response();
         break;
         
         case 4: // COMPRESSOR SPEED
	 debug_print("COMPRESSSOR SPEED\r\n");
         uint8_t buff7[10]={0x01,0x03,0x89,0x19,0x00,0x01,0x7F,0x91,0x23};    //compressor speed
	 Fridge_test(buff7);
         debug_print("\n\r");
         debug_print("value of COMPRESSOR SPEED\r\n");
         compressor_speed();
         //parsing_response();
         break;
        
         case 5:  //BATTERY CUT-OUT LEVEL
           
         debug_print("BATTERY CUT-OUT LEVEL\r\n");  
         uint8_t buff8[10]={0x01,0x03,0x89,0xE7,0x00,0x01,0x1E,0x61,0x23};  
         Fridge_test(buff8); 
         debug_print("\n\r");
         debug_print("value of cutout level\n\r");
         cutout_level();
         break;
         
         case 6:  //BATTERY CUT-OUT LEVEL  
         debug_print("BATTERY CUT-IN LEVEL\r\n");  
         uint8_t buff9[10]={0x01,0x03,0x89,0xE8,0x00,0x01,0x2E,0x62,0x23};  
         Fridge_test(buff9); 
         debug_print("\n\r");
         debug_print("value of cut-in level\n\r");
         cut_in_level();
         break;
         
         case 7: //Actual Error
         debug_print("Actual Error\r\n");  
         uint8_t buff10[10]={0x01,0x03,0x89,0x1B,0x00,0x01,0xDE,0x51,0x23};  
         Fridge_test(buff10); 
         debug_print("\n\r");
         debug_print("Actual Error\n\r");
         actual_error();
         debug_print("\r\n");
         break; 
         
         case 8: // fan speed 
         debug_print("FAN SPEED\n\r");
         uint8_t buff11[10]= {0x01,0x03,0x89,0xE3,0x00,0x01,0x5F,0xA0,0x23};
         Fridge_test(buff11); 
         debug_print("\n\r");
         debug_print("Fan speed\n\r");
         fan_speed();
         debug_print("\r\n");
         break;  
         
         
         case 9: // compressor runtime 
         debug_print("COMPRESSOR RUNNTIME\n\r");
         uint8_t buff12[10]= {0x01,0x03,0x89,0x28,0x00,0x01,0x2E,0x5E,0x23};
         Fridge_test(buff12); 
         debug_print("\n\r");
         debug_print("commpressor runtime\n\r");
         comp_runtime();
         debug_print("\r\n");
         break;  
         
         default :  debug_print("no response of fridge\r\n");
        
         }
      
         }        
}
        
        