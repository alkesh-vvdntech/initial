#include "TI_TXT_File_Parser.h"
#include <string.h>
#include <stdlib.h>
#include <intrinsics.h>
#include "FAT_Update.h"
#include "TI_MSPBoot_MI.h"

#ifndef _FISM_
//lcd extern declare
extern void lcd_print_progress();
#endif

//utility from SO by Orwellophile Jun 17 '12 at 4:08
static const int8_t hextable[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//0-15
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//16-31
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//32-47
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,//48-63
    
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,//64-79
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//80-95
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,//96-111
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//112-127
    
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 //all the way to 255
};

/** 
 * @brief convert a hexidecimal string to a signed long
 * will not produce or process negative numbers except 
 * to signal error.
 * 
 * @param hex without decoration, case insensative. 
 * 
 * @return -1 on error, or result (max sizeof(long)-1 bits)
 */
int32_t hexdec(unsigned const char *hex) {
   int32_t ret = 0; 
   while (*hex && ret >= 0) {
      ret = (ret << 4) | (int32_t)hextable[*hex++];
   }
   return ret; 
}

uint32_t TI_TXT_Parse_Addr(uint8_t *pcAddrString) {
    int32_t addr = hexdec(pcAddrString + 1); //table based hex to int32_t above, returns -1 if parse failed
    return addr < 0 ? 0 : addr; //if the parse failed return null, returnee must check
}

uint8_t TI_TXT_Parse_Data(uint8_t *buffer, uint8_t buffer_n, uint8_t *pcByteString) {
    pcByteString = (uint8_t*)strtok((char *)pcByteString, " ");
    
    int count = 0;
    while(pcByteString != 0 && count < buffer_n) {
        int32_t data = hexdec(pcByteString);
        if(data < 0) return count; //parse failed so we must exit
        buffer[count] = (uint8_t)data;
        pcByteString = (uint8_t *)strtok(0, " ");
        count++;
    }
    
    return count;
}

extern void __no_operation(void);

//user library function pointers
const TI_TXT_InitLineFeed g_fpInitLineFeed = &FAT_Update_GetUpdateFile;
const TI_TXT_GetLine g_fpGetLine = &FAT_Update_GetLine;
const TI_TXT_ReleaseLineFeed g_fpReleaseLineFeed = &FAT_Update_CloseUpdateFile;

uint8_t data[16];
uint8_t line[60];
uint8_t TI_TXT_Program(void)
{
    uint8_t ret = 1;
    int updateCounter = 0;
    
    memset(data, 0, sizeof(data));
    memset(line, 0, sizeof(line));
    
    //here we make sure we are at the beginning of the TI TXT file
    (*g_fpInitLineFeed)();
  
    uint32_t address = 0; //start with invalid address - assumes every data sections starts with @ADDR
    
    while((*g_fpGetLine)((uint8_t*)line, sizeof(line))) //line from image without returns of any kind
    {
        
        switch (*line) {
           case 'q':
            {
               debug_print("\n\r CASE Q \n\r");
                ret = 0;
                goto quit;
            }

           case '@':
            {
               debug_print("\n\r CASE @ \n\r");
                address = TI_TXT_Parse_Addr(line);
                 debug_print("\n\r CASE @ DOWN \n\r");
                if(address == NULL) goto quit; //error parsing address
            } break;

           case '0':
           case '1':
           case '2':
           case '3':
           case '4':
           case '5':
           case '6':
           case '7':
           case '8':
           case '9':
           case 'A':
           case 'B':
           case 'C':
           case 'D':
           case 'E':
           case 'F':
            {
                 debug_print("\n\r MAMA \n\r");
                int i, bytesParsed = TI_TXT_Parse_Data(data, sizeof(data), line);
                for(i = 0; i < bytesParsed; i++) {
                  debug_print("\n\r ADITYA \n\r");
                    if(TI_MSPBoot_MI_WriteByte(address++, data[i])) goto quit; // Write one byte at a time and
                                                                //  increase the address pointer
                }
#ifndef _FISM_
                if(updateCounter % 1000 == 0) lcd_print_progress();
#endif
            } break;
            
           default:
            {
                goto quit;
            }
        }
        updateCounter++;
    }
     debug_print("\n\r UPDATE COUNTER \n\r");

   quit:
      debug_print("\n\r QUIT \n\r");
    (*g_fpReleaseLineFeed)();
    return ret;

}

uint8_t TI_TXT_GetShortAtAddress(uint16_t usAddress, uint16_t *p_usData)
{
    uint8_t ret = 1, lowerFound = 0, upperFound = 0;
    
    memset(data, 0, sizeof(data));
    memset(line, 0, sizeof(line));
    
    //here we make sure we are at the beginning of the TI TXT file
    (*g_fpInitLineFeed)();
  
    uint32_t address = 0; //start with invalid address - assumes every data sections starts with @ADDR
    
    while((*g_fpGetLine)((uint8_t*)line, sizeof(line)))
    {
        switch (*line) {
           case 'q':
            {
                goto quit;
            }

           case '@':
            {
                address = TI_TXT_Parse_Addr(line);
                if(address == NULL) goto quit; //error parsing address
            } break;

           case '0':
           case '1':
           case '2':
           case '3':
           case '4':
           case '5':
           case '6':
           case '7':
           case '8':
           case '9':
           case 'A':
           case 'B':
           case 'C':
           case 'D':
           case 'E':
           case 'F':
            {
                int i, bytesParsed = TI_TXT_Parse_Data(data, sizeof(data), line);
                for(i = 0; i < bytesParsed; i++) {
                    if(address == usAddress) {
                        *((uint8_t *)p_usData) = data[i];
                        lowerFound = 1; //found the LSB
                    }
                    if(address == usAddress + 1) {
                        *(((uint8_t *)p_usData) + 1) = data[i];
                        upperFound = 1; //found the MSB
                    }
                    if(lowerFound && upperFound) {
                        ret = 0;
                        goto quit;
                    }
                    address++;
                }
            } break;
            
           default:
            {
                goto quit;
            }
        }

    }

   quit: 
    (*g_fpReleaseLineFeed)();
    return ret;
}
