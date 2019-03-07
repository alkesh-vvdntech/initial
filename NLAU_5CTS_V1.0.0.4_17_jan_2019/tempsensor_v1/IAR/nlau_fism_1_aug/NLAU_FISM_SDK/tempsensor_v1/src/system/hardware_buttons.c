/*
 * buttons.c
 *
 *  Created on: Jun 1, 2015
 *      Author: sergioam
 */
#include "thermalcanyon.h"
#include "state_machine.h"
#include "hardware_buttons.h"
#include "state_machine.h"

extern void buzzer_feedback();

// Interrupt services

uint8_t switch_check_service_pressed() {
	return !(P3IN&BIT5);
}

void switchers_setupIO() {
	// Setup buttons
	P4IE |= BIT1;							// enable interrupt for button input
	P4IE |= BIT4;							// enable interrupt for Power input
	P2IE |= BIT2;							// enable interrupt for buzzer off
	P3IE |= BIT5;							// enable interrupt for extra button setup
}

void hardware_disable_buttons() {
	P4IE &= ~BIT1;				// disable interrupt of button input
	P3IE &= ~BIT5;				// enable interrupt for extra button setup
}

void hardware_enable_buttons() {
	switchers_setupIO();
}

HARDWARE_ACTIONS g_iHardware_actions = HWD_NOTHING;

void hardware_actions() {
	uint8_t lcd_on = false;
	if (g_iHardware_actions==HWD_NOTHING)
		return;

	// TODO Change into flags to run several actions at the same time
	switch (g_iHardware_actions) {
		case HWD_POWER_CHANGE:
			event_force_event_by_id(EVT_ALARMS_CHECK, 0);
			lcd_on = 0;
			break;
		case HWD_THERMAL_SYSTEM:
			if (g_bServiceMode || (P2IN & 0x4) == 0) {
		  		event_force_sample_and_upload();
			} else {
				event_force_sample();
			}
			lcd_on = 1;
			break;
		case HWD_BUZZER_FEEDBACK:
			/*
			if (g_pSysState->system.switches.button_buzzer_override) {
				lcd_printf(LINE1, get_string(LCD_MSG, BUZZER_OFF));
			} else {
				lcd_printf(LINE1, get_string(LCD_MSG, BUZZER_ONN));
			}
			*/
			g_iSystemSetup = 0;
			lcd_on = 1;
			break;
		case HWD_TURN_SCREEN:
			lcd_on = 1;
			break;
	}

	if (lcd_on) {
		//event_LCD_turn_on();
//		/event_force_event_by_id(EVT_DISPLAY, 0);
                lcd_turn_on();
                lcd_show();
	}

	//event_force_event_by_id(EVT_SUBSAMPLE_TEMP, 0);
	buzzer_feedback();
	g_iHardware_actions=HWD_NOTHING;
}

// Port 2 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) Port_2 (void)
#else
#error Compiler not supported!
#endif
{
	switch (__even_in_range(P2IV, P2IV_P2IFG7)) {
	case P2IV_NONE:
		break;
	case P2IV_P2IFG0:
		break;
	case P2IV_P2IFG1:
		break;
	case P2IV_P2IFG2:
		SYSTEM_RUNNING_CHECK
#ifdef _DEBUG
		g_iDebug = !g_iDebug;
#endif
		g_iHardware_actions = HWD_TURN_SCREEN;
g_pSysState->system.switches.button_buzzer_override = true;
                //		state_alarm_enable_buzzer_override();
    	// Resume execution if the device is in deep sleep mode
		WAKEUP_MAIN
		break;
	default:
		break;
	}
}

// Port 3 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=PORT3_VECTOR
__interrupt void Port_3(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT3_VECTOR))) Port_3 (void)
#else
#error Compiler not supported!
#endif
{

	switch (__even_in_range(P3IV, P3IV_P3IFG5)) {
	case P3IV_NONE:
		break;
	case P3IV_P3IFG5:
		SYSTEM_RUNNING_CHECK
		g_iSystemSetup ++;
		g_iHardware_actions = HWD_THERMAL_SYSTEM;
    	// Resume execution if the device is in deep sleep mode
		WAKEUP_MAIN
		break;
	default:
		break;
	}
}

// Port 4 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT4_VECTOR))) Port_4 (void)
#else
#error Compiler not supported!
#endif
{
	switch (__even_in_range(P4IV, P4IV_P4IFG7)) {
	case P4IV_NONE:
		break;
	case P4IV_P4IFG0:
		break;
	case P4IV_P4IFG4:
		SYSTEM_RUNNING_CHECK
		g_iHardware_actions = HWD_POWER_CHANGE;
    	// Resume execution if the device is in deep sleep mode
		WAKEUP_MAIN
		break;
	case P4IV_P4IFG1:
		SYSTEM_RUNNING_CHECK
		g_iDisplayId++;
		g_iDisplayId %= MAX_DISPLAY_ID;
		g_iHardware_actions = HWD_TURN_SCREEN;

    	// Resume execution if the device is in deep sleep mode
		WAKEUP_MAIN
		break;
	default:
		break;
	}
}

