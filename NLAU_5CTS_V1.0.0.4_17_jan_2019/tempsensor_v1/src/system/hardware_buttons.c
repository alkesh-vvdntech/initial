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
#include "config.h"

extern void buzzer_feedback();

// Interrupt services
uint8_t switch_check_service_pressed()
{
  return !(P1IN&BIT5);       
}

void switchers_setupIO()
{
 // Setup buttons
 P1IE |= BIT6; 
 P1IE |= BIT4;							
 P1IE |= BIT5;
 P3IE |= BIT7; //power good
}

void hardware_disable_buttons()
{
  P1IE &= ~BIT4;	
  P1IE &= ~BIT5;
  P1IE &= ~BIT6;
}

void hardware_enable_buttons()
{
  switchers_setupIO();
}

HARDWARE_ACTIONS g_iHardware_actions = HWD_NOTHING;

void hardware_actions() {
  static uint8_t lcd_on = false;
  if (g_iHardware_actions==HWD_NOTHING)
  {
    return;
  }

  switch (g_iHardware_actions) 
  {
    case HWD_POWER_CHANGE:
         event_force_event_by_id(EVT_ALARMS_CHECK, 0);           
         lcd_on = 0;
         break;

    case HWD_THERMAL_SYSTEM:
         if (g_bServiceMode || (P1IN & 0x4) == 0) {
            event_force_sample_and_upload();
         } else {
            event_force_sample();
         }
         lcd_on = 1;
         break;

    case HWD_BUZZER_FEEDBACK:
         g_iSystemSetup = 0;
         lcd_on = 1;
         break;

    case HWD_TURN_SCREEN:
         lcd_on = 1;
	 break;

    default:
         break;
  }

  if (lcd_on)
  {
    lcd_turn_on();
    lcd_show();
  }
  g_iHardware_actions=HWD_NOTHING;
}

// Port 1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
  switch (__even_in_range(P1IV, P1IV_P1IFG7))
  {
    case P1IV_P1IFG4:
      flags.lcd_update = 1;
      SYSTEM_RUNNING_CHECK
      g_iSystemSetup ++;
      g_iHardware_actions = HWD_TURN_SCREEN;  
      g_pSysState->system.switches.buzzer_sound = STATE_OFF;
      WAKEUP_MAIN
      break;

    case P1IV_P1IFG5:
      flags.lcd_update = 1;
      SYSTEM_RUNNING_CHECK
      g_iHardware_actions = HWD_TURN_SCREEN;          
      g_pSysState->system.switches.button_buzzer_override = true;
      WAKEUP_MAIN
      break;

    case P1IV_P1IFG6:
      flags.lcd_update = 1;
      SYSTEM_RUNNING_CHECK
      g_iDisplayId++;
      g_iDisplayId %= MAX_DISPLAY_ID;
      g_iHardware_actions = HWD_TURN_SCREEN;
      flags.button_int_val++;;
      WAKEUP_MAIN
      break;

    default:
      break;
  }
}

// Port 3 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void Port_3(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT3_VECTOR))) Port_3 (void)
#else
#error Compiler not supported!
#endif
{
  switch (__even_in_range(P3IV, P3IV_P3IFG7))
  {
    case P3IV_P3IFG7:
         debug_print("POWER INTERRUPT");
         SYSTEM_RUNNING_CHECK
         g_iHardware_actions = HWD_POWER_CHANGE;
         WAKEUP_MAIN
         break;

    default:
         break;	
  }
}