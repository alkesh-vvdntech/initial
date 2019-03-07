#include "TestFWtxt.h"
#include <string.h>

const char* g_pcStartFW =
"@4400\r\n\
70 AC \r\n\
@4405\r\n\
FF 31 40 00 24 B0 13 12 44 B0 13 9A 44 B2 40 80\r\n\
5A 5C 01 F2 40 A5 00 61 01 B2 40 46 00 62 01 B2\r\n\
40 33 01 64 01 82 43 66 01 C2 43 61 01 C2 43 04\r\n\
02 C2 43 05 02 C2 43 24 02 C2 43 25 02 82 43 24\r\n\
03 C2 43 02 02 C2 43 03 02 C2 43 22 02 C2 43 23\r\n\
02 82 43 22 03 92 C3 30 01 F2 D0 20 00 25 02 F2\r\n\
D0 20 00 23 02 F2 E0 20 00 23 02 3F 40 46 E8 03\r\n\
43 1E 43 3F 53 3E 63 FD 2F F5 3F 03 43 F1 03 0A\r\n\
4C 81 4A 00 00 0D 41 5C 43 B0 13 9E 44 F9 3F 03\r\n\
43 80 00 82 44 80 00 96 44 10 01 \r\n\
@F3FE\r\n\
06 44 \r\n\
@10000\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
AA BB CC DD EE AA DD 11 AA BB CC DD EE AA DD 11\r\n\
q\
";

char* g_pcCurLine;
void resetLine(void) {
	g_pcCurLine = (char*)g_pcStartFW;
}

uint8_t getLine(uint8_t *buffer, uint16_t buffer_n){
	char* EOL = 0;
	
	if(g_pcCurLine >= g_pcStartFW + strlen(g_pcStartFW) - 1) return 0;
	
	EOL = strstr(g_pcCurLine, "\r\n");
	if(EOL) {
        int cpy_n = EOL - g_pcCurLine > buffer_n ? buffer_n : EOL - g_pcCurLine;
        memcpy(buffer, g_pcCurLine, cpy_n);
        buffer[cpy_n] = '\0';
        g_pcCurLine = EOL + 2; //skip the \r\n for beginning of next line
        return 1;
	} else {
        return 0;
    }
}
