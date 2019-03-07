/*
* 	jSON_api.c
*
*  Created  On: 31/3/2017
   Modified On: 7/4/2017
*  Author: Ambrish Gautam

*/

#include <msp430.h>
#include "spi_flash.h"
#include <string.h>

char Packet_Buff[256];
char json_t[26];
char json_did[11];
char json_tm[9];
char json_sid[17];
char json[11];
char json_id[11];
char json_close[3];
char seprator[3];
char type[8];
char power_cap = 1;
char ss[3];
char power[7];
char btr[12];
//{ "vId": "Nexleaf","data":
void json_start(void){

 char json[] = {'{', '"','v','I','d','"',':', '"','N','e','x','l','e','a','f','"',',','"','d','a','t','a','"',':'};
 strcpy(json_t,json);
 
  //Store_IN_TEMP_SECTOR(json_t);
}
//[{"dId": "358072045351407",
void json_dId(void){
  
  char json[11] = {'[','{','"','d','I','d','"',':','"',','};
  strcpy(json_did,json);
  
  //Store_IN_TEMP_SECTOR(json_dId);
}

//"tmps": [
void json_tempshow(void){
  
  char json[9] = {'\"','t','m','p','s','\"',':','['};
  strcpy(json_tm,json);
  //Store_IN_TEMP_SECTOR(json_tm);
}
// {"sId": "B", "time": 1491305307, "tmp": 28.5},
void json_sID(void){
  
   char json[17] = {'{','"','s','I','d','"',':','\0'};
  strcpy(json_sid,json);
   //Store_IN_TEMP_SECTOR(json);
  
  
}
void json_sensorname(int i){
  
  
  json_sID();
  json[0] = '"';
  switch (i){
  case 1:
    json[1] = 'A';
    break;
  case 2:
    json[1] = 'B';
    break;
  case 3:
    json[1] = 'C';
    break;
  default:
    break;
  }
  json[2] = '"';
  json[3] = ',';
  json[4] = '"';
  json[5] = 't';
  json[6] = 'm';
  json[7] = 'p';
  json[8] = '"';
  json[9] = ':';
  json[10] = '\0';
  // json_tempshow();
  strcpy(json_id,json);
}

void close_json(void){
  
   char json[3] = {'}','\0'};
   strcpy(json_close,json);
}

void seprator_json(void){
  
  char json[3] = {',','\0'};
  strcpy(seprator,json);
  //Store_IN_TEMP_SECTOR(seprator);
}

void type_json(char types){

  char  json[8] = {'"','t','y','p','"',':','"','\0'};
  //char for_type[3] = {'1','"','\0'};
  strcpy(type,json);
 // Store_IN_TEMP_SECTOR(type);
  __delay_cycles(10);
 // Store_IN_TEMP_SECTOR(for_type);

}

void json_btr(void){
  char json[12] = {'"','P','"',',','"','b','a','t','t','"',':','\0'};
  strcpy(btr,json);
}
void json_power(void){
  
  char json[7] = {'"','p','w','r','"',':','\0'};
  strcpy(power,json);
}
void json_signalS(void){
  
  char json[6] = {'"','s','s','"',':','\0'};
  strcpy(ss,json);
}
void capture_batry(void){

  char json_on[3] = {'6','4','\0'};
   Store_IN_TEMP_SECTOR(json_on);
  
  }
void capture_power(void){
  
  char json_on[3] = {'1','\0'};
  char json_off[2] = {'0','\0'};
  
  if(power_cap == 1){
      Store_IN_TEMP_SECTOR(json_on);
  }
  else
      Store_IN_TEMP_SECTOR(json_off);

}

void capture_sS(void){
 
  char json_on[3] = {'3','5','\0'};
   Store_IN_TEMP_SECTOR(json_on);
  
}

void packet_closing(void){

  char json[5] = {']','}',']','}'};
  strcpy(json_close,json);
  
}

void forming_packet(void){
  //json_start();
  //json_dId();
  //json_tempshow();
  //json_sID();
  //close_json();
  //packet_closing();
  
//    strcat(Packet_Buff,"{\"vid\": \"NEXLEAF\", \"data\": 20}");

  strcat(Packet_Buff,"{\"vId\": \"Nexleaf\", \"data\": [{\"dId\": \"1234\"} , {\"tmps\": \"20\"} , {\"sId\": \"1234\"} ]}");
  /*
  strcat(Packet_Buff,json_did);
  strcat(Packet_Buff,json_tm);
  strcat(Packet_Buff,json_sid);
  strcat(Packet_Buff,json_close);
 // strcat(Packet_Buff,json_close);
  */
      //UART_write(Packet_Buff,strlen(Packet_Buff));
debug_print(Packet_Buff);
  
}
