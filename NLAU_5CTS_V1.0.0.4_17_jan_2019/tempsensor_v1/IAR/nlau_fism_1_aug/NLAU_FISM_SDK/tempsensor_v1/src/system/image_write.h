/*
 * image.h
 *
 *
 *      Author: rach
 */
#include "stdint.h"
#include "string.h"
#ifndef IMAGE_H_
#define IMAGE_H_

#define GSM_BUFFER_CLEAR (memset(rxBuffer_gsm_g,'\0',sizeof(rxBuffer_gsm_g)))
#define WORD_SIZE 300
#define DATA_COMMANDS ' '

unsigned char  write_image_to_flash(int32_t addr,uint8_t data);
unsigned char* hex_decode(const char *in, size_t len,uint8_t *out);
char image_store_mcu (void);
extern char rxBuffer_gsm_g[300];


enum Result {	Pass,
		Failed,
		Success,
		ERROR,
		TRUE,
		NO_IMAGE,
		FALSE
};
enum output {	NO=0,
		YES,
		IMAGE_PRESENT,
		Check_Image,
		Low,
		HIGH
	    };
#endif /* IMAGE_H_ */
