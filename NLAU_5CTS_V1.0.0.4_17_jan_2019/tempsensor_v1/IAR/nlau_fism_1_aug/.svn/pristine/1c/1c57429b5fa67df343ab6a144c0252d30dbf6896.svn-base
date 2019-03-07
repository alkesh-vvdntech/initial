/*
 * hardware_buttons.h
 *
 *  Created on: Jun 1, 2015
 *      Author: sergioam
 */

#ifndef HARDWARE_BUTTONS_H_
#define HARDWARE_BUTTONS_H_


#define MODEM_ON    !(P2IN & BIT7)

#ifdef _FISM_

#define POWER_OFF   P4IN & BIT4
#define POWER_ON    !(POWER_OFF)
#define NOT_CHARGING P4IN & BIT6
#define IS_CHARGING !(NOT_CHARGING)

#else

#define POWER_OFF       P7IN & BIT6
#define POWER_ON        !(POWER_OFF)
#define NOT_CHARGING    P7IN & BIT6
#define IS_CHARGING     !(NOT_CHARGING)


#endif

void switchers_setupIO();
uint8_t switch_check_service_pressed();

typedef enum {
        HWD_NOTHING = 0,
        HWD_POWER_CHANGE,
        HWD_BUZZER_FEEDBACK,
        HWD_TURN_SCREEN,
        HWD_THERMAL_SYSTEM,

} HARDWARE_ACTIONS;

extern HARDWARE_ACTIONS g_iHardware_actions;

void hardware_disable_buttons();
void hardware_enable_buttons();
void hardware_actions();

#endif /* HARDWARE_BUTTONS_H_ */
