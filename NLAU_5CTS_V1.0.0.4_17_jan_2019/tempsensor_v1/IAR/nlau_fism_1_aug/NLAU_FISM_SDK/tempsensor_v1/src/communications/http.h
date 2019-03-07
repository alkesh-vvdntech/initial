/*
 * http.h
 *
 *  Created on: May 22, 2015
 *      Author: sergioam
 */

#ifndef TEMPSENSOR_V1_HTTP_H_
#define TEMPSENSOR_V1_HTTP_H_

#define HTTP_STATUS_PULL_CONFIG 303

int8_t http_setup();
uint8_t http_deactivate();
uint8_t http_enable();
//int http_get_configuration();
int http_check_error(int *retry);
int http_open_connection_upload(uint32_t data_length, char *);
//int http_get(char *URL, void (*process_callback)(void), uint8_t type);
int http_get(char *URL, int (*process_callback)(void), uint8_t type);
//int backend_get_configuration();
//int8_t http_post(char* postdata);
//int16_t formatfield(char* pcSrc, char* fieldstr, int lastoffset,
//		char* seperator, int8_t iFlagVal, char* pcExtSrc, int8_t iFieldSize);
int8_t http_send_file(FIL *file, char *URL);

//extern const char HTTP_INCOMING_DATA[];

#endif /* TEMPSENSOR_V1_HTTP_H_ */
