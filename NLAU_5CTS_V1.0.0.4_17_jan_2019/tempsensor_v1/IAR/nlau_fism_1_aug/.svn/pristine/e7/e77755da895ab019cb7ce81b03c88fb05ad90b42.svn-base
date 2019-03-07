
#include "driverlib.h"
#include "thermalcanyon.h"
#include "state_machine.h"

#define TIMER_PERIOD 0x2000
#define DUTY_CYCLE   0x1000

typedef struct {
    uint16_t match;
    uint16_t togglePeriod;
} BuzzerPattern;

const BuzzerPattern g_BuzzPatt = {0x01D0, 0x13DD}; //pure -7.5 peak@~4.3kHz on,off

#include "timer_b.h"

void buzzer_start() {

    GPIO_setAsOutputPin(
        GPIO_PORT_P3,
        GPIO_PIN4
        );

	TA2CCTL0 = CCIE;

	if (!state_isBuzzerOn() && g_pSysState->buzzerFeedback>0) {
		TA2CCR0 = 8000;						  // 0.5khz normal op
	}
	else {
		TA2CCR0 = g_BuzzPatt.match;	// 4khz alarm op ~= 575
	}
	
	TA2CTL = TASSEL__SMCLK | MC__UP | ID__2;  // SMCLK/2 (4MHz), UP mode

}

void buzzer_feedback_value(uint16_t value) {
	//disable buzzer if system switch is set, else play sound
	if(g_pDevCfg->cfg.logs.buzzer_disable != 1){
		g_pSysState->buzzerFeedback = value;
		buzzer_start();
	}
}

void buzzer_feedback() {
	buzzer_feedback_value(50);
}

extern void lcd_blenable();
extern void lcd_bldisable();
// Timer1_A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector = TIMER2_A0_VECTOR
__interrupt void Timer2_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER2_A0_VECTOR))) Timer2_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	static uint16_t count = 0;

	if (g_pSysState->buzzerFeedback>0) {
	    GPIO_toggleOutputOnPin(
	        GPIO_PORT_P3,
	        GPIO_PIN4);
		g_pSysState->buzzerFeedback--;
		return;
	}

    if(g_pDevCfg->cfg.logs.buzzer_toggle && count / g_BuzzPatt.togglePeriod) {
        if(count % g_BuzzPatt.togglePeriod == 0) {
            lcd_bldisable();
            P3OUT &= ~BIT4; //disable buzzer when not sounding
        }
    } else {
        //here we check the P3.4 output and toggle if not equal to count mod 2 (simple sqaure wave)
        if (!(P3OUT & BIT4) != !(count & 1)) P3OUT ^= BIT4;
    }

	count++;
    
    if(g_pDevCfg->cfg.logs.buzzer_toggle && count / (g_BuzzPatt.togglePeriod * 2)) {
        lcd_blenable();
        count = 0;
    }

	if (!state_isBuzzerOn()) {
		TA2CTL = MC__STOP;
        P3OUT &= ~BIT4;  //disable buzzer when not sounding
        count = 0;
        lcd_blenable();
		return;
	}
}

