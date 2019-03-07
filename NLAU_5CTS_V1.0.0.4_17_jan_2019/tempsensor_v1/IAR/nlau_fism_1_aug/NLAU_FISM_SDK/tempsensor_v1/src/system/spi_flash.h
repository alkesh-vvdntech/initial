/*
 * 	spi_flash.h
 *
 *  Created  On: 1/2/2017
    Modified On: 10/2/2017
 *  Author: Ambrish Gautam

 */
#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include "stdint.h"
/* #### HEADER FILES #### */

/* #### MACROS #### */
#define SPI_CS_L							(P7OUT &= (~BIT3))
#define SPI_CS_H 							(P7OUT |= BIT3)



/* #### W25Q16 registers #### */
#define W25Q16_CMD_WRITE_ENABLE				(0x06)
#define W25Q16_CMD_DISABLE_ENABLE			(0x04)
#define W25Q16_CMD_READ_STATUS_REG1			(0x05)
#define W25Q16_CMD_PAGE_PROGRAM				(0x02)
#define W25Q16_CMD_READ_DATA				(0x03)
#define W25Q16_CMD_CHIP_ERASE				(0x60)
#define W25Q16_CMD_SECTOR_ERASE				(0x20)
#define BLS_CODE_DP               0xB9 /**< Power down */
#define BLS_CODE_RDP              0xAB /**< Power standby */

/* #### size information #### */
#define W25Q16_PAGE_SIZE					((unsigned int)256)
#define W25Q16_SECTOR_SIZE					((unsigned int)4096)
#define W25Q16_TTL_PAGES					((unsigned int)7936)
#define W25Q16_PKT_PER_PAGE					((unsigned int)2)
#define W25Q16_PKT_PER_SECTOR				((W25Q16_SECTOR_SIZE / W25Q16_PAGE_SIZE) * W25Q16_PKT_PER_PAGE)
#define W25Q16_TTL_PKTS						(W25Q16_TTL_PAGES * W25Q16_PKT_PER_PAGE)



/* flash read */
#define FLASH_MEM_READ_OK				(0)
#define FLASH_MEM_READ_EMPTY				(1)

#define IS_SECTOR_ADDRESS(add)				((0 == (add & 0x0FFF)) ? TRUE : FALSE)
#define Sector1_startingadd                             (0x00001)
#define Sector1_endadd                                  (0x0FFF)
#define FALSE 0
#define TRUE 1
#define MAX_BUFF_SIZE        256        //equals to one sector

/******for temp sample*************************/
#define IS_SECTOR_ADDRESS(add)				((0 == (add & 0x0FFF)) ? TRUE : FALSE)
#define START_ADD_TEMPSEC       0xA000 // 160
#define END_ADD_TEMPSEC         0x471FF
#define TEMP_TOTAL_PACKETS 978
//1958 fridge 
/******for OTA packets*************************/
#define START_ADD_OTA_SEC        0xC1800
#define END_ADD_OTA_SEC          0xFFFFF  // 1000 packets
#define OTA_TOTAL_PACKETS 1000

/**************for default values**************/
#define Sector1_startingadd                             (0x00000) //0
#define Sector1_endadd                                  (0x0FFF) // 16 packets
#define DEFAULT_TOTAL_PACKETS 16
/***************for error logs****************/
//We need 2 sector for error logs as discussed with martin
#define START_ADD_ERRORSEC                            (0x02000) // 32
#define END_ADD_ERRORSEC                            (0x3FFF) // 32 packets
#define ERROR_TOTAL_PACKETS 32

/**********for At commands storage****************/
//We need 1 sector for it
#define START_ADD_AtcommandSEC                            (0x47200) //1138
#define END_ADD_AtcommandSEC                            (0xC17FF)       // 1958 packets
#define AT_TOTAL_PACKETS 1958

#define DEFAULT_PUT_TEMP_IDX            160
#define DEFAULT_PUT_AT_CMD_IDX          1138
#define DEFAULT_PUT_ERROR_IDX           32
#define DEFAULT_PUT_DEFAULT_IDX         0

#define DEFAULT_GET_TEMP_IDX            160
#define DEFAULT_GET_AT_CMD_IDX          1138
#define DEFAULT_GET_ERROR_IDX           32
#define DEFAULT_GET_DEFAULT_IDX         0

//#define SYSTEM_NUM_SENSORS		4

/* #### stucture #### */
typedef struct
{
	unsigned int put_temp_idx;
        unsigned int put_at_cmd_idx;
        unsigned int put_error_idx;
        unsigned int put_default_idx;

        unsigned int get_temp_idx;
	unsigned int get_at_cmd_idx;
	unsigned int get_error_idx;
	unsigned int get_default_idx;
        unsigned char server_url[24];

}s_flash_memory;

extern s_flash_memory flash_memory;
extern char local_storage_buff[W25Q16_PAGE_SIZE];
typedef struct
{
	char buffer;
	char pkt_count;
}s_local_storage_t;

/* #### FUNTION PROTOTYPE #### */

/* spi low level */
void SPI_Init(void);
void GPIO_init(void);
void SPI_Write (unsigned char data);
unsigned int SPI_Read (void);

/* W25Q16 */
void Write_Enable (void);
void Write_Disable (void);
unsigned char W25Q16_BUSY (void);
void W25Q16_Write (unsigned long address, char *data, unsigned int len);
void W25Q16_Read (unsigned long address, char *datar, unsigned int len);
void W25q16_Erasure (void);
void W25Q16_Sector_Erase (unsigned long address);
unsigned char flash_memory_save_OTA_image(char *pkt);//,unsigned int page);
unsigned char flash_memory_read_ota_image();//,unsigned int page);
void flash_memory_save_pkt_to_ram(char *pkt);
/* application */
void save_packet(char *pkt, unsigned int saving_idx,unsigned char data_type);
void save_into_sector1(char ptr[]);
void flash_memory_increment_get_page_idx(unsigned int get_idx,unsigned int total_packets,unsigned char data_type);
void Store_IN_TEMP_SECTOR(char pkt[]);
void read_from_flash(void);
unsigned char read_packets(unsigned int put_page_idx,unsigned int get_page_idx, unsigned int total_packets);
unsigned int flash_memory_get_count_of_saved_packets(unsigned int put_index,unsigned int get_index,unsigned int total_packets);
//void log_sample_to_disk(void);
unsigned char fram_write(uint32_t addr,char* data,uint16_t size);
void update_put_get_idx();
void new_test_idx();
void read_indexes(uint32_t address,char *data,uint16_t size);
void read_mem_segment(uint32_t address,char *data,uint16_t size);
void READ_TEMP_SECTOR(void);
void Store_IN_TEMP_SECTOR(char pkt[]);
void packet_closing(void);
void capture_sS(void);
void strrev (char *str);
void itoa(unsigned long num, char* arr, int base);
void capture_power(void);
void capture_batry(void);
void json_signalS(void);
void json_power(void);
void json_btr(void);
void type_json(char types);
void seprator_json(void);
void close_json(void);
void json_sensorname(int i);
void json_sID(void);
void json_tempshow(void);
void json_dId(void);
void json_start(void);




#endif

