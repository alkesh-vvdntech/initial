#include <msp430.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void debug_init();

void debug_print(char *cmd);

#define DIAG_BUFF_LEN	300
extern char diagnosis_flag;
extern char diagnosis_buff[DIAG_BUFF_LEN];
extern char diag_command[DIAG_BUFF_LEN];
extern unsigned char diag_buff_len;

#ifdef __cplusplusb
}
#endif
