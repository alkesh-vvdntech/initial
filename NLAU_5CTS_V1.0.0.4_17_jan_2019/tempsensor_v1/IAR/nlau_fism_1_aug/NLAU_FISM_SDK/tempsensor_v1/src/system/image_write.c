#include "stdlib.h"
#include "msp430.h"
#include "stdint.h"
#include "string.h"
#include "image_write.h"
#include "spi_flash.h"


uint8_t* hex_decode(const char *in, size_t len,uint8_t *out)
{
        unsigned int i, t, hn, ln;

        for (t = 0,i = 0; i < len; i+=2,++t)
        {

                hn = in[i] > '9' ? in[i] - 'A' + 10 : in[i] - '0';
                ln = in[i+1] > '9' ? in[i+1] - 'A' + 10 : in[i+1] - '0';

                out[t] = (hn << 4 ) | ln;
        }

        return out;
}

unsigned char  write_image_to_flash(int32_t addr,uint8_t data)
{
        uint8_t dummy_var;
	dummy_var=*(uint8_t*)addr;
  
        MPUCTL0 = MPUPW | MPUENA;   // Enable access to MPU registers
        MPUSAM |= MPUSEG2WE;        // Enable Write access
        *(uint8_t *) ((uint32_t)addr) = data;   // Write the vector
        MPUSAM &= ~MPUSEG2WE;       // Disable Write access
        MPUCTL0_H = 0x00;           // Disable access to MPU registers      
        
        dummy_var=*(uint8_t*)addr;

	return (dummy_var == data) ? Success : Failed;
       // return RET_OK;
}
char image_store_mcu (void)
{
	//static unsigned int crc =0;
	char address_available = 0;
	volatile uint16_t word_count=0;
	uint8_t code_memory[2]={'\0'};
	uint8_t code_hexmemory[2]={'\0'};
	volatile uint32_t flash_address = 0;
	uint32_t address = 0;
	uint8_t temp = 0;
	uint8_t	count_byte = 1;
	uint8_t	status = 0;
	char * ptr;
	char asc[10]={'\0'};
	char asc_temp[6]={'\0'};
	unsigned char first_check =  0;
        static unsigned char high_address=0;

        memset(asc,'\0',sizeof(asc));

        flash_memory.get_default_idx = 3096;
        memset(local_storage_buff,'\0',sizeof(local_storage_buff));
        flash_memory_read_ota_image();
        //crc = CRC_check(local_storage_buff);
        for(word_count =0 ; (local_storage_buff[word_count] != 'q') /*&& (rxBuffer_gsm_g[word_count + 1] != 'K'))*/ ;word_count++)
        {

          /* below 0x20 or ' ' lies the commnad signals..
           * GSM sometimes sends the command siagnal
           * 	ignoring this signals
           * 	*/

          if(first_check == 1)
          {
                  word_count = 0;
                  first_check = 0;
          }
          if(local_storage_buff[word_count] > DATA_COMMANDS)
          {
              /* in the text file '@' means start of the address . data after this address
               * will be written there
               * */

              if(local_storage_buff[word_count] == '@')
              {
                      address_available = YES;
                      memset(asc,'\0',sizeof(asc));
                      address = 0;
                      if(strstr(local_storage_buff,"@10000") !='\0')
                      {
                        high_address = 1;
                      }
                      continue;
              }
              if(address_available == YES)
              {
                  /* after receving the first '@' it will store the next 4 bytes of data as
                   * the next for bytes are the address. and then will continue with normal operation
                   * */
                  if(high_address==0)
                  {
                      if(temp<4)//|| ((local_storage_buff[word_count] != 0x0D)))
                      {
                              asc_temp[temp++]=local_storage_buff[word_count];
                      }
                      else
                      {
                              /* address is in ascii code convert to hex */
                              address_available = NO;
                              flash_address = strtol(asc_temp,&ptr,16);
                              temp = 0;
                      }
                  }
                  else if(high_address==1)
                  {
                      if(temp<5) //|| ((local_storage_buff[word_count] != 0x0D)))
                      {
                              asc_temp[temp++]=local_storage_buff[word_count];
                      }
                      else
                      {
                              /* address is in ascii code convert to hex */
                              address_available = NO;
                              flash_address = strtoul(asc_temp,&ptr,16);
                              temp = 0;
                      }
                  }
              }
              if(address_available == NO)
              {
                  /* all the data comming are in ASCII format needs to converted into hex format
                     2 bytes OF ASCII will converted to single hex for mat and then stored to flash*/

                  if(count_byte == 1)
                          code_hexmemory[count_byte - 1] = local_storage_buff[ word_count ];
                  else
                          code_hexmemory[count_byte - 1] = local_storage_buff[word_count];

                  if(count_byte++ == 2)
                  {
                    count_byte=1;
                    /* converting ascii to hex*/
                    
                    hex_decode((const char *)code_hexmemory,2,code_memory);
                    
                    /* flash write operation*/
                    
                    __data20_write_char(flash_address+address++,code_memory[0]);
                    /*clearing memory sections*/
                    memset(code_hexmemory,'\0',sizeof(code_hexmemory));
                    memset(code_hexmemory,'\0',sizeof(code_memory));
                    /*image wriitng operation failed*/
                    if(status == Failed)
                            return Failed;
                  }
              }
          }
          if(word_count == 249)
          {
                  //word_count = 0;
                  first_check = 1;
                  memset(local_storage_buff,'\0',sizeof(local_storage_buff));
                  flash_memory_read_ota_image();

          }
        }
        return Success;
}
