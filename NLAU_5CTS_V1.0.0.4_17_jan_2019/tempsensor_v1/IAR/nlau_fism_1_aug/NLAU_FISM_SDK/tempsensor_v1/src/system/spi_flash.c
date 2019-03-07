/*
* 	spi_flash.h
*
*  Created  On: 1/2/2017
Modified On: 31/3/2017
*  Author: Ambrish Gautam

*/

/* #### HEADER FILES #### */
#include <string.h>
#include "spi_flash.h"
#include <msp430.h>
#include <string.h>
#include <stdbool.h>
# include "stdint.h"
#include "config.h"
#include "main_system.h"
#include "watchdog.h"

char debug_tmp_buff[16] = { 0 };
extern char temperatureAc[5];
/* #### GLOBLE VARIABLES #### */
char local_storage_buff[W25Q16_PAGE_SIZE] = { 0 };

s_flash_memory flash_memory;


int spacefilled = 0;
char type1;


/* #### EXTERNS #### */
extern char debug_tmp_buff[16];


/* #### LOCAL VARIABLES #### */
static unsigned int spi_rx_data = 0;
static unsigned int sys_time_out = 0;

extern char temperatureA[5];
extern char temperatureB[5];
extern char temperatureC[5];
extern float temp_dataA;
extern float temp_data[3];



/* #### FUNCTION DEFINITIONS #### */

/******************************************************************************
* File Name : spi_flash.c
* Function Name : __interrupt void USCI_B2_ISR(void)
* Parameters : None
* Return : None
* Description : spi isr
*****************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=EUSCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_B2_VECTOR))) USCI_B2_ISR (void)
#else
#error Compiler not supported!
#endif
{  volatile unsigned int i;

switch(__even_in_range(UCB2IV,4))
{
case 0:
break;                          // Vector 0 - no interrupt

case 2:                                 // Vector 2 - RXIFG
  // TX buffer ready?
  while ((UCB2IFG & UCTXIFG) != UCTXIFG);

  spi_rx_data = UCB2RXBUF;
  break;

default:
break;

}
}
/******************************************************************************
* File Name : spi_flash.c
* Function Name : void SPI_Init(void)
* Parameters : None
* Return : None
* Description :
*****************************************************************************/
void SPI_Init(void)
{
  P7SEL1 &= ~BIT2;                        // USCI_A0 SCLK pin
  P7SEL0 |= BIT2;
  P7SEL1 &= ~(BIT0 | BIT1);               // USCI_A0 MOSI and MISO pin
  P7SEL0 |= BIT0 | BIT1;
  // PJSEL0 |= BIT4 | BIT5;                  // For XT1
  P7OUT |= BIT3;                         // Clear P1.0 output latch for a defined power-on state
  P7DIR |= BIT3;
  P2SEL0 &= ~(BIT0 | BIT1);
  P2SEL1 |= BIT0 | BIT1;

  // Configure USCI_A0 for SPI operation
  UCB2CTLW0 = UCSWRST;                    // **Put state machine in reset**
  UCB2CTLW0 |= UCMST | UCSYNC | UCCKPH | UCMSB; // 3-pin, 8-bit SPI master
  // Clock polarity high, MSB
  UCB2CTLW0 |= UCSSEL__SMCLK;              // SMCLK
  UCB2BRW = 0x08;                        
  UCB2CTLW0 &= ~UCSWRST;                  // **Initialize USCI state machine**
  return;
}

/******************************************************************************
 * File Name : spi_flash.c
 * Function Name : void SPI_Write (unsigned char data)
 * Parameters : None
 * Return : None
 * Description : send 8-bit data with SPI
 *****************************************************************************/
void SPI_Write (unsigned char data)
{
	//SPI_CS_L;

   // UCB2IE |= UCRXIE;						// Enable USCI_B2 RX interrupt

    // USCI_B2 TX buffer ready?
    //sys_time_out = 1;
	while ((UCB2IFG & UCTXIFG) != UCTXIFG); //&& (sys_time_out != 0) );

//	if((UCB2IFG & UCTXIFG) != UCTXIFG)
//	{
//		SPI_Init();
//		return;
//	}

	//spi_wait_rx_complite = FALSE;

	UCB2TXBUF = data;                   	// Transmit first character

	 while ((UCB2IFG & UCRXIFG) != UCRXIFG);  // Rx received?
	spi_rx_data = UCB2RXBUF;
	// wait till rx operation complite
	//sys_time_out = 1;
	//while(spi_wait_rx_complite != TRUE) //&& (sys_time_out != 0));

//	if((spi_wait_rx_complite != TRUE))
//	{
//		SPI_Init();
//		return;
//	}

	//UCB2IE &= (~UCRXIE);					// disable USCI_B2 RX interrupt
	//SPI_CS_H;
	return;
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description :  SPI accepts 8-bit data
 *****************************************************************************/
unsigned int SPI_Read (void)
{
	//    sys_time_out = 1;
	 while (((UCB2IFG & UCTXIFG) != UCTXIFG));    // USCI_B2 TX buffer ready?

	/* if((UCB2IFG & UCTXIFG) != UCTXIFG)
	 {
	  SPI_Init();
	  return 0xFF;
	 }*/

	// spi_wait_rx_complite = FALSE;

	// while (UCBUSY & UCB2STAT);
	 UCB2TXBUF = 0xFF;                    // Transmit first character
	// while (UCBUSY & UCB2STAT);

	// spi_rx_data = 0xFF;

	 // wait till rx operation complite
	// sys_time_out = 1;

	 while ((UCB2IFG & UCRXIFG) != UCRXIFG);  // Rx received?
	 spi_rx_data = UCB2RXBUF;
	/* while((spi_wait_rx_complite != TRUE) && (sys_time_out != 0));

	 if((spi_wait_rx_complite != TRUE))
	 {
	  SPI_Init();
	  return 0xFF;
	 }*/

	 //UCB2IE &= (~UCRXIE);     // disable USCI_B2 RX interrupt




//    UCB2IE |= UCRXIE;						// Enable USCI_B2 RX interrupt
//
//    // USCI_B2 TX buffer ready?
//    sys_time_out = 1;
//	while (((UCB2IFG & UCTXIFG) != UCTXIFG) && (sys_time_out != 0) );
//
//	if((UCB2IFG & UCTXIFG) != UCTXIFG)
//	{
//		SPI_Init();
//		return 0xFF;
//	}
//
//	spi_wait_rx_complite = FALSE;
//
//	UCB2TXBUF = 0xFF;                   	// Transmit first character
//
//	spi_rx_data = 0xFF;
//
//	// wait till rx operation complite
//	sys_time_out = 1;
//	while((spi_wait_rx_complite != TRUE) && (sys_time_out != 0));
//
//	if((spi_wait_rx_complite != TRUE))
//	{
//		SPI_Init();
//		return 0xFF;
//	}
//
//	//UCB2IE &= (~UCRXIE);					// disable USCI_B2 RX interrupt
//
	// read_count++;
//	return spi_rx_data;						// disable USCI_B2 RX interrupt
	 return spi_rx_data;      // disable USCI_B2 RX interrupt

}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description : enable write function on W25Q16
 *****************************************************************************/
void Write_Enable (void)
{
        SPI_CS_L;

	SPI_Write (W25Q16_CMD_WRITE_ENABLE);

	SPI_CS_H;

	return;
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description : write prohibit function
 *****************************************************************************/
void Write_Disable (void)
{
	SPI_CS_L;

	SPI_Write (W25Q16_CMD_DISABLE_ENABLE);

	SPI_CS_H;

	return;
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description : determine whether W25Q16 busy busy function returns 1
 *****************************************************************************/
unsigned char W25Q16_BUSY (void)
{
    unsigned char flag;

	SPI_CS_L;

	SPI_Write (W25Q16_CMD_READ_STATUS_REG1);

    flag = SPI_Read();

	SPI_CS_H;

    flag &= 0x01;

	return flag;
}

/*******************************************************************************
* @fn       extFlashPowerDown
*
* @brief    Put the device in power save mode. No access to data; only
*           the status register is accessible.
*
* @param    none
*
* @return   none
*******************************************************************************/
void extFlashPowerDown(void)
{
#ifdef  _SPI_FLASH_
  
    SPI_CS_L;
    SPI_Write(BLS_CODE_DP);
    SPI_CS_H;  
    flags.spi_flash_active = 0;
    
#endif
}

/*******************************************************************************
* @fn       extFlashPowerStandby
*
* @brief    Take device out of power save mode and prepare it for normal operation
*
* @param    none
*
* @return   true if successful
*******************************************************************************/
bool extFlashPowerStandby(void)
{
#ifdef _SPI_FLASH_
   
   SPI_CS_L;
   SPI_Write(BLS_CODE_RDP);
   SPI_CS_H;
   while (W25Q16_BUSY() == 1);

   flags.spi_flash_active = 1;

   return true;
   
#endif
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description : write "len" bytes from "data" to "address" on W25Q16
 *****************************************************************************/
void W25Q16_Write (unsigned long address, char * data, unsigned int len)
{
    unsigned int i = 0;

    extFlashPowerStandby();
    
    // If the chip is busy on the other here
    while (W25Q16_BUSY() == 1);


    Write_Enable (); // first write enable command
    
    // If the chip is busy on the other here
    while (W25Q16_BUSY() == 1);

    
    SPI_CS_L;

    SPI_Write (W25Q16_CMD_PAGE_PROGRAM);

    SPI_Write (address >> 16);

    SPI_Write (address >> 8);

    SPI_Write (address);

    for (i = 0; i < len; i++)
    {
          SPI_Write ((*(data + i)));
    }

    SPI_CS_H;
    return;
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description : read "len" bytes from "address" to "data" from W25Q16
 *****************************************************************************/
void W25Q16_Read (unsigned long address, char * data, unsigned int len)
{
    unsigned int i = 0;
    
    extFlashPowerStandby();

    while (W25Q16_BUSY() == 1); 

    SPI_CS_L;

    SPI_Write (W25Q16_CMD_READ_DATA);

    SPI_Write (address >> 16);

    SPI_Write (address >> 8);

    SPI_Write (address);

    for (i = 0; i < len; i ++)
    {
      *(data + i) = SPI_Read ();
    }

    SPI_CS_H;
    return;
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description : erase the entire chip
 *****************************************************************************/
void W25q16_Erasure (void)
{
    extFlashPowerStandby();
    
    Write_Enable ();

    sys_time_out = 2;

    // If the chip is busy on the other here
    while ((W25Q16_BUSY() == 1) && (sys_time_out != 0));

    if(W25Q16_BUSY() == 1)
    {
            return;
    }

    SPI_CS_L;

    SPI_Write (W25Q16_CMD_CHIP_ERASE);

    SPI_CS_H;

    sys_time_out = 2;

    // If the chip is busy on the other here
    while ((W25Q16_BUSY() == 1) && (sys_time_out != 0));

    if(W25Q16_BUSY() == 1)
    {
        return;
    }

    return;
}
/******************************************************************************
* File Name : spi_flash.c
* Function Name :
* Parameters :
* Return :
* Description : erase sector on W25Q16
*****************************************************************************/
void W25Q16_Sector_Erase (unsigned long address)
{        
    extFlashPowerStandby();
    
    sys_time_out = 2;

    while ((W25Q16_BUSY() == 1) && (sys_time_out != 0));

    if(W25Q16_BUSY() == 1)
    {
        return;
    }

    Write_Enable ();

    sys_time_out = 2;

    while ((W25Q16_BUSY() == 1) && (sys_time_out != 0));

    if(W25Q16_BUSY() == 1)
    {
            return;
    }

    SPI_CS_L;

    SPI_Write (W25Q16_CMD_SECTOR_ERASE);

    SPI_Write (address >> 16);

    SPI_Write (address >> 8);

    SPI_Write (address);

    SPI_CS_H;

    sys_time_out = 2;

    while ((W25Q16_BUSY() == 1) && (sys_time_out != 0));

    if(W25Q16_BUSY() == 1)
    {
            return;
    }

    return;
}

/******************************************************************************
 * File Name 		: spi_flash.c
 * Function Name 	: flash_memory_save_pkt_to_ram
 * Parameters 		: pkt -
 * 					: pos -
 * Return 			:
 * Description 		: store pkt into 'local_storage_buff'
 *****************************************************************************/
void flash_memory_save_pkt_to_ram(char *pkt)
{
  memset(local_storage_buff,'\0',sizeof(local_storage_buff));
  strcpy(local_storage_buff,pkt);
  return;
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :void flash_memory_save_OTA_image(char *pkt)
 * Parameters :
 * Return :
 * Description :
 *****************************************************************************/
unsigned char flash_memory_save_OTA_image(char *pkt)//,unsigned int page)
{
	 unsigned long address = 0;

#ifndef _FISM_
	debug_print("\r\n\r\n<Before write>\r\nput_idx :");
	memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
        itoa(flash_memory.put_default_idx, debug_tmp_buff, 16);
	debug_print(debug_tmp_buff);
	debug_print("\r\nwrite @ ");
#endif

	flash_memory_save_pkt_to_ram(pkt);

	address = (unsigned long) flash_memory.put_default_idx * W25Q16_PAGE_SIZE;

#ifndef _FISM_
        debug_print("0x");
        memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
        itoa(address,debug_tmp_buff, 16);
        debug_print(debug_tmp_buff);
#endif

        if(IS_SECTOR_ADDRESS(address) == TRUE)
        {
  #ifndef _FISM_
                debug_print("\r\nerase @ 0x");
                debug_print(debug_tmp_buff);
  #endif
                W25Q16_Sector_Erase(address);
        }

        W25Q16_Write(address, local_storage_buff, W25Q16_PAGE_SIZE);
        if(strchr(local_storage_buff,'q')!= '\0')
        {
          return 0;
        }
        memset(local_storage_buff,'\0', W25Q16_PAGE_SIZE);
        flash_memory.put_default_idx++;

#ifndef _FISM_
	debug_print("\r\npkt :");
	debug_print(pkt);
	debug_print("\r\n");
#endif

	return 1;
}

/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description :
 *****************************************************************************/
unsigned char flash_memory_read_ota_image()//,unsigned int page)
{
	unsigned long address = 0;
	//unsigned int count = 0;
	unsigned char status = FLASH_MEM_READ_OK;


        address = (unsigned long) flash_memory.get_default_idx * W25Q16_PAGE_SIZE;
        //address = 0x100;
        W25Q16_Read(address,&local_storage_buff[0], (W25Q16_PAGE_SIZE));
        flash_memory.get_default_idx++;

#ifndef _FISM_
        debug_print("0x");
        memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
        itoa(address,debug_tmp_buff, 16);
        debug_print(debug_tmp_buff);
#endif

#ifndef _FISM_
	debug_print("\r\npkt :");
	debug_print(local_storage_buff);
	debug_print("\r\n");
#endif

	return status;
}
/******************************************************************************
* File Name : spi_flash.c
* Function Name :
* Parameters :
* Return :
* Description :
*****************************************************************************/
unsigned int W25Q16_ReadStatusReg(void)
{
  unsigned int flag;

  SPI_CS_L;

  SPI_Write (W25Q16_CMD_READ_STATUS_REG1);

  flag = SPI_Read();

  SPI_CS_H;

  flag &= 0x01;

  return flag;

}
/******************************************************************************
* File Name : spi_flash.c
* Function Name :
* Parameters :
* Return :
* Description :
*****************************************************************************/
unsigned char  fram_write (uint32_t addr,char* data,uint16_t size)
{
    uint8_t dummy_var;

    unsigned int i;
    char * Flash_ptr;                         // Initialize Flash pointer
    Flash_ptr = (char *)addr;

    MPUCTL0 = MPUPW | MPUENA;   // Enable access to MPU registers
    MPUSAM |= MPUSEG2WE;        // Enable Write access

    for (i = 0; i < size; i++)
    {
      *(Flash_ptr+i) = *(data+i);                   // Write value to flash
    }
    MPUSAM &= ~MPUSEG2WE;       // Disable Write access
    MPUCTL0_H = 0x00;           // Disable access to MPU registers
}
/******************************************************************************
* File Name : spi_flash.c
* Function Name :
* Parameters :
* Return :
* Description :
*****************************************************************************/
void default_idx_values()
{
  unsigned char reboot = 0x05;

  flash_memory.put_temp_idx = DEFAULT_PUT_TEMP_IDX ;
  flash_memory.put_at_cmd_idx = DEFAULT_PUT_AT_CMD_IDX;
  flash_memory.put_error_idx = DEFAULT_PUT_ERROR_IDX;
  flash_memory.put_default_idx = DEFAULT_PUT_DEFAULT_IDX;

  flash_memory.get_temp_idx = DEFAULT_GET_TEMP_IDX ;
  flash_memory.get_at_cmd_idx = DEFAULT_GET_AT_CMD_IDX;
  flash_memory.get_error_idx = DEFAULT_GET_ERROR_IDX;
  flash_memory.get_default_idx = DEFAULT_GET_DEFAULT_IDX;
  
  memset(flash_memory.server_url,'\0',sizeof(flash_memory.server_url));
  strcpy(flash_memory.server_url,"in.coldtrace.org");
  
  fram_write(0x1800,(char *)&reboot,1);
}
/******************************************************************************
* File Name : spi_flash.c
* Function Name :
* Parameters :
* Return :
* Description :
*****************************************************************************/
void update_put_get_idx()
{
  fram_write(0x1900,(char *)&flash_memory,sizeof(flash_memory));  
}
/**********************************************************************************************
 * File Name : spi_flash.c
 * Function Name : void read_indexes(void)
 * Parameters : None
 * Return : None
 * Description : read memory locations
 **********************************************************************************************/
void read_indexes(uint32_t address,char *data,uint16_t size)
{
    unsigned char first_boot=0;

    first_boot = *(uint16_t*)0x1800;

    debug_print("\r\n\r\nfirst boot value :");
    memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
    itoa(first_boot, debug_tmp_buff,10);
    debug_print(debug_tmp_buff);
    debug_print("\r\n\r\n");

    if(first_boot == 0x05)
    {
      flags.booted_device = 1;
      read_mem_segment(address,data,size);
    }
    else
    {
      flags.booted_device = 0;
      first_boot = 0xff;
      W25q16_Erasure();
      default_idx_values();
    }
    update_put_get_idx();
}
/**********************************************************************************************
 * File Name : spi_flash.c
 * Function Name : read_mem_segment(uint32_t address,char *data,uint16_t size);
 * Parameters : None
 * Return : None
 * Description : read memory locations
 **********************************************************************************************/
void read_mem_segment(uint32_t address,char *data,uint16_t size)
{
    uint16_t index=0;
    char *seg_address;
    seg_address = (char*)address;
    for(index=0;index<size;index++)
    {
            *(data+index)=*(seg_address+index);
    }
}
/**********************************************************************************************
 * File Name : gps_gsm_parse.c
 * Function Name : void itoa(unsigned long num, char* str, int base)
 * Parameters : num : numeric value that needs to be converted to ascii array
 * 				arr : array in which converted value is stored
 * Return : None
 * Description : convert integer value into ASCII
 **********************************************************************************************/
void itoa(unsigned long num, char* arr, int base)
{
     unsigned char i=0,rem=0;
     char buff_cpy[20];
     
     memset(buff_cpy,'\0',sizeof(buff_cpy));
    /* Handle 0 explicitly, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        arr[i++] = '0';
        arr[i] = '\0';
    }

    // Process individual digits
    while (num != 0)
    {
        rem = num % base;
        arr[i++] = (rem > 9)? (rem-10) + 'A' : rem + '0';
        num = num/base;
    }
    arr[i] = '\0'; 		// Append string terminator
    strrev(arr);        // Reverses the string
}
/**********************************************************************************************
 * File Name : gps_gsm_parse.c
 * Function Name : void strrev(char*)
 * Parameters : *arr : array in which converted value is stored
 * Return : None
 * Description : reveerses the string
 **********************************************************************************************/
void strrev (char *str)
{
    char temp, *end_ptr;

    if( str == '\0' || !(*str) )    // If str is NULL or empty, do nothing
    return;

    end_ptr = str + strlen(str) - 1;
    // Swap the characters
    while( end_ptr > str )
    {
          temp = *str;
          *str = *end_ptr;
          *end_ptr = temp;
          str++;
          end_ptr--;
    }
}
/**********************************************************************************************
 * File Name : spi_flash.c
 * Function Name : void save_packet(char *pkt, unsigned int saving_idx,unsigned int total_packets)
 * Parameters :
 * Return : None
 * Description : reveerses the string
 **********************************************************************************************/
void save_packet(char *pkt, unsigned int saving_idx,unsigned char data_type)
{
   unsigned long address = 0;

#if 1 ///       rachit debug
	debug_print("\r\n\r\n<Before write>\r\nput_idx :");
	memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
        itoa(saving_idx, debug_tmp_buff, 16);
	debug_print(debug_tmp_buff);
	debug_print("\r\nwrite @ ");
#endif

	flash_memory_save_pkt_to_ram(pkt);

	address = (unsigned long) saving_idx * W25Q16_PAGE_SIZE;

#ifdef _FISM_
          debug_print("0x");
          memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
          itoa(address,debug_tmp_buff, 16);
          debug_print(debug_tmp_buff);
#endif

          if(IS_SECTOR_ADDRESS(address) == TRUE)
          {
#ifdef _FISM_
                  debug_print("\r\nerase @ 0x");
                  debug_print(debug_tmp_buff);
#endif
                  W25Q16_Sector_Erase(address);
          }

          W25Q16_Write(address, local_storage_buff, W25Q16_PAGE_SIZE);

#if 1      // rachit debug 
	debug_print("\r\npkt :");
	debug_print(local_storage_buff);
	debug_print("\r\n");
#endif
        extFlashPowerDown();


        memset(local_storage_buff, 0, W25Q16_PAGE_SIZE);

        switch(data_type)
        {

          case 1:   flash_memory.put_temp_idx++;
                    if(flash_memory.put_temp_idx >= (TEMP_TOTAL_PACKETS+DEFAULT_PUT_TEMP_IDX))
                    {
                          flash_memory.put_temp_idx = DEFAULT_PUT_TEMP_IDX;
                    }
                    break;

          case 2 :  flash_memory.put_at_cmd_idx++;
                    if(flash_memory.put_at_cmd_idx >= (AT_TOTAL_PACKETS+DEFAULT_PUT_AT_CMD_IDX))
                    {
                          flash_memory.put_at_cmd_idx = DEFAULT_PUT_AT_CMD_IDX;
                    }
                    break;
          case 3 :  flash_memory.put_error_idx++;
                    if(flash_memory.put_error_idx >= (ERROR_TOTAL_PACKETS+DEFAULT_PUT_ERROR_IDX))
                    {
                          flash_memory.put_error_idx = DEFAULT_PUT_ERROR_IDX;
                    }
                    break;
          case 4 :  flash_memory.put_default_idx++;
                    if(flash_memory.put_default_idx >= (DEFAULT_TOTAL_PACKETS))
                    {
                          flash_memory.put_default_idx = DEFAULT_PUT_DEFAULT_IDX;
                    }
                    break;

          default: break;
        }
        update_put_get_idx();
	return;
}

/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description :
 *****************************************************************************/
void flash_memory_increment_get_page_idx(unsigned int get_idx,unsigned int total_packets,unsigned char data_type)
{
    switch(data_type)
    {
        case 1: flash_memory.get_temp_idx++;
                if(flash_memory.get_temp_idx >= (TEMP_TOTAL_PACKETS+DEFAULT_GET_TEMP_IDX))
                {
                        flash_memory.get_temp_idx = DEFAULT_GET_TEMP_IDX;
                }
                break;

        case 2 : flash_memory.get_at_cmd_idx++;
                if(flash_memory.get_at_cmd_idx >= (AT_TOTAL_PACKETS+DEFAULT_GET_AT_CMD_IDX))
                {
                        flash_memory.get_at_cmd_idx = DEFAULT_GET_AT_CMD_IDX;
                }
                 break;
        case 3 : flash_memory.get_error_idx++;
                if(flash_memory.get_error_idx >= (ERROR_TOTAL_PACKETS+DEFAULT_GET_ERROR_IDX))
                {
                        flash_memory.get_error_idx = DEFAULT_GET_ERROR_IDX;
                }
                 break;
        case 4 : flash_memory.get_default_idx++;
                if(flash_memory.get_default_idx >= (DEFAULT_TOTAL_PACKETS))
                {
                        flash_memory.get_default_idx = DEFAULT_GET_DEFAULT_IDX;
                }
                 break;

        default: break;
    }
}
/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description :
 *****************************************************************************/
unsigned int flash_memory_get_count_of_saved_packets(unsigned int put_index,unsigned int get_index,unsigned int total_packets)
{
    unsigned int count = 0;

    if(put_index >= get_index)
    {
        count = put_index - get_index;
    }
    else
    {
        count = (total_packets - get_index) + put_index;
    }

    return count;
}

/******************************************************************************
 * File Name : spi_flash.c
 * Function Name :
 * Parameters :
 * Return :
 * Description :
 *****************************************************************************/
unsigned char read_packets(unsigned int put_page_idx,unsigned int get_page_idx, unsigned int total_packets)
{
      unsigned long address = 0;
      unsigned int count = 0;
      unsigned char status = FLASH_MEM_READ_OK;

      count = flash_memory_get_count_of_saved_packets(put_page_idx,get_page_idx,total_packets);

      if (0 == count)
      {
              return FLASH_MEM_READ_EMPTY;
      }

#ifdef _FISM_
      debug_print("\r\n\r\ncount of packets =: ");
      memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
      itoa(count, debug_tmp_buff,10);
      debug_print(debug_tmp_buff);
      debug_print("\r\n\r\n<Before read>\r\nput_idx :");
      memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
      itoa(put_page_idx, debug_tmp_buff, 16);
      debug_print(debug_tmp_buff);
      debug_print("\r\nget_idx :");
      memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
      itoa(get_page_idx, debug_tmp_buff, 16);
      debug_print(debug_tmp_buff);
      debug_print("\r\nread @ ");
#endif

      /* Check if it is only packet which is stored in 'local_storage_buff'
       * then read packet from 'local_storage_buff'
       * othewise read packet form spi flash memory
       */
        memset(local_storage_buff,'\0', sizeof(local_storage_buff));

        address = (unsigned long) get_page_idx * W25Q16_PAGE_SIZE;
        W25Q16_Read(address,local_storage_buff,W25Q16_PAGE_SIZE);

#ifdef _FISM_
        debug_print("0x");
        memset(debug_tmp_buff, 0, sizeof(debug_tmp_buff));
        itoa(address, debug_tmp_buff, 16);
        debug_print(debug_tmp_buff);
#endif

#ifdef _FISM_
      debug_print("\r\npkt :");
      debug_print(local_storage_buff);
      debug_print("\r\n");
#endif

      return status;
}
/*EOF*/
