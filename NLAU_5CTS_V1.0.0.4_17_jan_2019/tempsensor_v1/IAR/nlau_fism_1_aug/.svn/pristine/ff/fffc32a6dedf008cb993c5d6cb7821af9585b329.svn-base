#include <msp430.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define FRIDGE_MAX_SIZE         512  
extern uint8_t fridge_buffer[FRIDGE_MAX_SIZE];
extern uint16_t fridge_index;

void Extrn_UART_init();
void ext_uart_enable();
void ext_uart_disable();
void ext_uart_tx(char *cmd);

#ifdef __cplusplusb
}
#endif
