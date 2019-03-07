/***********************************HEADER FILES************************************************/
#include "modem.h"
#include "modem_uart.h"
#include "main_system.h"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "file_system.h"



/**********************************************************************************************
 * Function Name :void file_system(void)
 * Parameters :  void
 * Return : void
 * Description : Get the local storage drive of GSM
 **********************************************************************************************/
void file_system(void)
{
    send_cmd_wait(GSM_FLASH_GET_DRIVE,"OK\r\n",60,2);
    send_cmd_wait(GSM_FLASH_LIST_DIR,"OK\r\n",20,1);
}


/**********************************************************************************************
 * Function Name :void write_apn_config(char *apn_code_name,char *apn_codes,uint16_t apn_len)
 * Parameters :  void
 * Return : void
 * Description : write apn config on GSM flash
 **********************************************************************************************/
unsigned char  write_apn_config(char *apn_code_name,char *apn_codes,uint16_t apn_len)
{
        char temp_buffer[50];
        char *response;
        unsigned  char ret = 0;

        memset(temp_buffer,'\0',sizeof(temp_buffer));
        sprintf(temp_buffer,GSM_FLASH_DEL,apn_code_name);
        send_cmd_wait(temp_buffer,"OK\r\n",7,1);
        memset(temp_buffer,'\0',sizeof(temp_buffer));
        sprintf(temp_buffer,GSM_FLASH_CREATE_FILE,apn_code_name);
	if(send_cmd_wait(temp_buffer,"OK\r\n",60,1))
	{
		memset(temp_buffer,'\0',sizeof(temp_buffer));
		sprintf(temp_buffer,GSM_FLASH_WRITE,apn_code_name,apn_len);
		if(send_cmd_wait(temp_buffer,">",60,1))
		{
			if(send_cmd_wait(apn_codes,"OK\r\n",60,1))
			{
                            ret = 1;
                            send_cmd_wait(GSM_FLASH_LIST_DIR,"OK\r\n",10,1);
                        }
                }
        }
        return ret;
}

/**********************************************************************************************
 * Function Name :void read_apn_config(char *apn_code_name,char *apn_codes,uint16_t apn_len)
 * Parameters :  void
 * Return : void
 * Description : write apn config on GSM flash
 **********************************************************************************************/

void read_apn_config(char *apn_name,uint16_t apn_len)
{
    char temp_buffer[50];
    char *response;
    unsigned  char ret = 0;

    memset(temp_buffer,'\0',sizeof(temp_buffer));
    sprintf(temp_buffer,GSM_FLASH_READ,apn_name,BUFFER_MAX_SIZE-10,apn_len);
    response=send_cmd_wait(temp_buffer,"OK\r\n",60,1);
    if(response!=NULL)
    {
       strtok(GSM_buffer,"OK");
    }

}
/**********************************************************************************************
 * Function Name : unsigned int read_file_size(char *apn_name)
 * Parameters :  void
 * Return : void
 * Description : Get the length of the file
 **********************************************************************************************/
unsigned int read_file_size(char *apn_name)
{
    char temp_buffer[50];
    char *response;
    char file_size[10];
    unsigned char count= 0;

    memset(temp_buffer,'\0',sizeof(temp_buffer));
    sprintf(temp_buffer,GET_FILE_SIZE,apn_name);
    response = send_cmd_wait(temp_buffer,"+FSFLSIZE:",10,2);
    if(response)
    {
      for(count = 0; count<=10;count++)
      {
          if((response[count + 11] == '\r') ||(response[count + 11] == '\n') ||(response[count + 11] == '\0'))
          {
            break;
          }
          file_size[count] = response[count + 11];
      }
      if((atoi(file_size)!=0) && (atoi(file_size)<3000))
      {
        return atoi(file_size);
      }
    }
}