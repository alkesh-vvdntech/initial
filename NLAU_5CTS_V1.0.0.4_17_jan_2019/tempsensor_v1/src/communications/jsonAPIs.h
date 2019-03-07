/*
 * jsonAPIs.h
 *
 *  Created on: Oct 2, 2015
 *      Author: Ben
 */

#ifndef SRC_COMMUNICATIONS_JSONAPIS_H_
#define SRC_COMMUNICATIONS_JSONAPIS_H_

#include "json.h"

typedef enum {
    JSON_TEMP_TYPE_RAW = 0,
    JSON_TEMP_TYPE_INC,
    JSON_TEMP_TYPE_EXC,
    JSON_TEMP_TYPE_ALR
} JSON_TEMP_TYPE;

//top level json structures
void json_print_api_header(json_printer *printer, uint8_t include_simslot);
void json_print_api_footer(json_printer *printer);
void json_print_data_header(json_printer *printer);
void json_print_data_footer(json_printer *printer);
void json_print_asm_pair(json_printer *printer);
void json_print_sId_pair(json_printer *printer, int sensor);
void json_print_stat_pair(json_printer *printer, int status);
void json_print_time_pair(json_printer *printer, time_t timeUTC);
void json_print_pwrAvail_pair(json_printer *printer);
void json_print_pwa(json_printer *printer, time_t timeUTC);

//temperature upload json structures
void json_print_temp_header(json_printer *printer);
void json_print_temp_obj(json_printer *printer, int sensor, time_t timeUTC);
#ifndef _EVIN_BUILD_
void json_print_pwrinfo_obj(json_printer *printer, time_t timeUTC);
#endif

//device ready json structures
void json_print_simDevRdy_obj(json_printer *printer, int slot);
void json_print_devDevRdy_obj(json_printer *printer);
void json_print_actSnsDevRdy_obj(json_printer *printer);

//device alarm json structures
void json_print_dvcAlarm_header(json_printer *printer);
void json_print_dvcAlarm_footer(json_printer *printer);
//void json_print_connAlarm_obj(json_printer *printer, int sensor, time_t timeUTC);
void json_print_extSensAlarm_obj(json_printer *printer, int sensor, time_t timeUTC);
void json_print_battAlarm_obj(json_printer *printer, time_t timeUTC);
void json_print_fwErrorAlarm_arr(json_printer *printer, time_t timeUTC);

#endif /* SRC_COMMUNICATIONS_JSONAPIS_H_ */
