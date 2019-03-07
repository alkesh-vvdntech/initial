/*
* temperature.c
*
*  Created on: 2 Jun 2015
*      Author: dancat
*/

#include "msp430.h"
#include "config.h"
#include "common.h"
#include "stdlib.h"
#include "globals.h"
#include "main_system.h"
#include "temperature.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "events.h"
#include "alarms.h"
#include "stringutils.h"
#include "device_alarms.h"
#include "watchdog.h"
#include "stdbool.h"
#include "lcd.h"

int charger_con = 0;
int charger_con2 = 0;
uint8_t slaveadr = 0x40;
uint8_t temper[2];
uint8_t tempert[10];
float temp_data;
char temp_disconnt[4]= {'0','0','0'};
uint16_t temp = 0;
float A0tempdegC = 0.0;

#pragma pack(1)
d_values dvalue;

void ADC_setupIO()
{
#ifndef _FISM_
    P1SELC |= BIT0 | BIT1;                    // Enable VEREF-,+
    P1SELC |= BIT2;                           // Enable A/D channel A2
#ifdef SEQUENCE
    P1SELC |= BIT3 | BIT4 | BIT5;          // Enable A/D channel A3-A5
#if defined(SYSTEM_NUM_SENSORS) && SYSTEM_NUM_SENSORS == 5
    P4SELC |= BIT2;          				// Enable A/D channel A10
#endif
#endif

#ifdef POWER_SAVING_ENABLED
    P3DIR |= BIT3;							// Modem DTR
    P3OUT &= ~BIT3;// DTR ON (low)
#endif
    //P1SEL0 |= BIT2;

    // Configure ADC12
    ADC12CTL0 = ADC12ON | ADC12SHT0_2;       // Turn on ADC12, set sampling time
    ADC12CTL1 = ADC12SHP;                     // Use pulse mode
    ADC12CTL2 = ADC12RES_2;                   // 12bit resolution
    ADC12CTL3 |= ADC12CSTARTADD_2;			// A2 start channel
    ADC12MCTL2 = ADC12VRSEL_4 | ADC12INCH_2; // Vr+ = VeREF+ (ext) and Vr-=AVss, 12bit resolution, channel 2
#ifdef SEQUENCE
    ADC12MCTL3 = ADC12VRSEL_4 | ADC12INCH_3; // Vr+ = VeREF+ (ext) and Vr-=AVss, 12bit resolution, channel 3
    ADC12MCTL4 = ADC12VRSEL_4 | ADC12INCH_4; // Vr+ = VeREF+ (ext) and Vr-=AVss, 12bit resolution, channel 4
#if defined(SYSTEM_NUM_SENSORS) && SYSTEM_NUM_SENSORS == 5
    ADC12MCTL5 = ADC12VRSEL_4 | ADC12INCH_5; // Vr+ = VeREF+ (ext) and Vr-=AVss,12bit resolution,channel 5,EOS
    ADC12MCTL6 = ADC12VRSEL_4 | ADC12INCH_10 | ADC12EOS; // Vr+ = VeREF+ (ext) and Vr-=AVss,12bit resolution,channel 5,EOS
#else
    ADC12MCTL5 = ADC12VRSEL_4 | ADC12INCH_5 | ADC12EOS; // Vr+ = VeREF+ (ext) and Vr-=AVss,12bit resolution,channel 5,EOS
#endif
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_1;                   // sample a sequence
    ADC12CTL0 |= ADC12MSC; //first sample by trigger and rest automatic trigger by prior conversion
#endif

    //ADC interrupt logic
#if defined(SYSTEM_NUM_SENSORS) && SYSTEM_NUM_SENSORS == 5
    ADC12IER0 |= ADC12IE2 | ADC12IE3 | ADC12IE4 | ADC12IE5 | ADC12IE6; // Enable ADC conv complete interrupt
#else
    ADC12IER0 |= ADC12IE2 | ADC12IE3 | ADC12IE4 | ADC12IE5; // Enable ADC conv complete interrupt
#endif
#else

    ADC12CTL0 = ADC12ON | ADC12SHT0_2| ADC12MSC;
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_1;
    ADC12CTL2 = ADC12RES_2;
    ADC12CTL3 |= ADC12CSTARTADD_10;
    //ADC12MCTL7 = ADC12VRSEL_1 | ADC12INCH_8;        // rachit change
    ADC12MCTL10 = ADC12VRSEL_1 | ADC12INCH_10;
    ADC12MCTL11 = ADC12VRSEL_1 | ADC12INCH_11;
    ADC12MCTL15 = ADC12VRSEL_1 | ADC12INCH_15;

    REFCTL0 |= REFVSEL_3 + REFON + REFOUT;
#endif
}

//--------------- TEMPERATURES ---------------------

float inline temperature_getValue(uint8_t id) {
    return g_pSysState->temp.sensors[id].fTemperature;
}

char *temperature_getString(uint8_t id) {
    // Check if the value is valid
    return g_pSysState->temp.sensors[id].temperature;
}

TEMPERATURE_SENSOR inline *sensor_get(uint8_t id) {
    return &g_pSysState->temp.sensors[id];
}

void temperature_sensor_init(uint8_t id) {
    TEMPERATURE_SENSOR *sensor = sensor_get(id);
    sensor->name[0] = SensorName[id][0];
    sensor->name[1] = 0;
}

void sensor_clear(uint8_t id) {
    TEMPERATURE_SENSOR *sensor;
    sensor = sensor_get(id);
    sensor->iADC = 0;
    sensor->fTemperature = 0;
    sensor->temperature[0] = 0;
}


void temperature_trigger_init()
{
    USE_TEMPERATURE
	int c;

    tem->iCapturing = 0;
    tem->iSamplesRequired = 0;
    tem->iSamplesRead = 0;

    //initialze ADCvar
    for (c = 0; c < SYSTEM_NUM_SENSORS; c++)
    {
	sensor_get(c)->iADC = 0;
	sensor_get(c)->iSamplesRead = 0;
    }
}

// Implementing basic POW to avoid the 3.5kb overhead of the
// required libmath.a to enable POW function
double pow_(double base, int exp)
{
    int t=0;
    double ret = base;
    --exp;
    for (t=0; t<exp; t++) {
	ret *= base;
    }
    return ret;
}

float resistance_to_temperature(float R)
{
    // Thermistor Vishay NTCASRFE3C90406
    //float A1 = 0.00335, B1 = 0.0002565, C1 = 0.0000026059, D1 = 0.00000006329,
    //		tempdegC;
    //float R25 = 9965.0;

    // Thermistor USSensor KS103J2
    //float A1 = 0.003353704, B1 = 0.000256595, C1 = 0.000z002561571, D1 = 0.00000004108811,
    //                 tempdegC;
    //float R25 = 10000.0;
    float tempdegC = 0;

    if(cfgGetAppMode() == APP_MODE_ST) { //Thermistor USSensor PN 254JG1K
	float y0 = 1001.035823, a = -178.800543, b = 11.715844, c = -0.293475;

	tempdegC = y0 + (a * log(R)) + (b * pow_((log(R)), 2))
	    + (c * pow_((log(R)), 3));
    } else {
	// Thermistor USSensor USX3315 REV NONE
	float A1 = 0.003354, B1 = 0.0002564253, C1 = 0.000002404009, D1 = 0.00000008733894;
	float R25 = 10000.0;

	tempdegC = 1
	    / (A1 + (B1 * log(R / R25)) + (C1 * pow_((log(R / R25)), 2))
	       + (D1 * pow_((log(R / R25)), 3)));

	tempdegC = tempdegC - 273.15;
    }

    return tempdegC;
}

void temperature_subsampling_calculate(int8_t iSensorIdx)
{
    float ADCval;
    float A0V2V, A0R2;
    float tempfloat = 0.0;

    char* TemperatureVal = temperature_getString(iSensorIdx);

    float cold = g_pDevCfg->stTempAlertParams[iSensorIdx].threshCold;
    float hot = g_pDevCfg->stTempAlertParams[iSensorIdx].threshHot;

    TEMPERATURE_SENSOR *sensor = sensor_get(iSensorIdx);
    SENSOR_STATUS *sensorState = getAlarmsSensor(iSensorIdx);

    if (sensor->iSamplesRead == 0)
	return;

    ADCval = (float) sensor->iADC / (float) sensor->iSamplesRead;
    if(cfgGetAppMode() != APP_MODE_ST)
    {
	if(ADCval < 67 && iSensorIdx == 4)
	{
	    g_pSysState->temp.state[iSensorIdx].state.disabled = true;
	    ADCval=(float)4095;
	}
	else
	{
	    g_pSysState->temp.state[iSensorIdx].state.disabled = false;
	}
    }
    A0V2V = (float)0.00061 * ADCval;		//Converting to voltage. 2.5/4096 = 0.00061
    A0R2 = (A0V2V * (float)10000.0) / ((float)2.5 - A0V2V);			//R2= (V2*R1)/(V1-V2)

    //Convert resistance to temperature using Steinhart-Hart algorithm
    A0tempdegC = resistance_to_temperature(A0R2);

    /*
    calibration = &g_pCalibrationCfg->calibration[iSensorIdx][0];

    if ((calibration[0] > -2.0) && (calibration[0] < 2.0)
    && (calibration[1] > -2.0) && (calibration[1] < 2.0)) {
    A0tempdegC = A0tempdegC * calibration[1] + calibration[0];
}
    */
    if ((float)A0tempdegC < 0.0)
    {
	tempfloat = ((float)-0.0062 * A0tempdegC)  + (float)0.1493;
	A0tempdegC = (float)A0tempdegC - (float)tempfloat;
    }
    if ((float)A0tempdegC > TEMP_CUTOFF)
    {
	if(cfgGetAppMode() == APP_MODE_ST)
	{
	    sensorState->state.disconnected = false;
            RED_LED2_OFF;
	}
	if((float)sensor->fTemperature != TEMP_BOOT_MAGIC_VAL)
	{
	    if((float)A0tempdegC > (float)hot && (float)sensor->fTemperature < (float)sensor->lastThreshHot)	//Hot Excursion
	    {
		sensorState->state.excursion = true;
		sensorState->state.incursion = false;
                RED_LED2_ON;
		//		sensorState->state.typ = 2;
	    }
	    else if((float)A0tempdegC < (float)cold && (float)sensor->fTemperature > (float)sensor->lastThreshCold)	//Cold Excursion
	    {
		sensorState->state.excursion = true;
		sensorState->state.incursion = false;
                RED_LED2_ON;
		//		sensorState->state.typ = 2;
	    }
	    else if((float)A0tempdegC < (float)hot && (float)sensor->fTemperature > (float)sensor->lastThreshHot)	//Hot Incursion
	    {
		sensorState->state.incursion = true;
		sensorState->state.excursion = false;
                RED_LED2_ON;
		//		sensorState->state.typ = 1;
	    }
	    else if((float)A0tempdegC > (float)cold && (float)sensor->fTemperature < (float)sensor->lastThreshCold)	//Cold Incursion
	    {
		sensorState->state.incursion = true;
		sensorState->state.excursion = false;
                RED_LED2_ON;
		//		sensorState->state.typ = 1;
	    }
	    else
	    {
		sensorState->state.excursion = false;
		sensorState->state.incursion = false;
                RED_LED2_OFF;
		//		sensorState->state.typ = 0;
	    }
	}
	else
	{
	    if(A0tempdegC >= hot || A0tempdegC <= cold)
	    {		//Hot Excursion
		sensorState->state.excursion = true;
                RED_LED2_ON;
	    }
	    else
	    {
		sensorState->state.incursion = true;
                RED_LED2_ON;
	    }
	}
    }
    else
    {   
      if(iSensorIdx == 1){
        lcd_printl(LINE2,"SENSOR C DISC.");
      }
      else if(iSensorIdx == 2){
      lcd_printl(LINE2,"SENSOR D DISC.");
      }  
      debug_print("\n\rSensor Disconnected");
       
#ifdef _FISM_
	A0tempdegC = -100;
#else
	sensor->fTemperature = -100;
#endif
	if(cfgGetAppMode() == APP_MODE_ST)
	{
	    sensorState->state.disconnected = true;
	}
    }
    sensor->fTemperature = A0tempdegC;
    sensor->lastThreshCold = cold;
    sensor->lastThreshHot = hot;
    getFloatNumber2Text(A0tempdegC, TemperatureVal);
}

void temperature_analog_to_digital_conversion()
{
    USE_TEMPERATURE
	uint8_t iIdx;
    ADC12IER0 |= ADC12IE10 | ADC12IE11;    //Enable ADC interrupt

    if (tem->iSamplesRequired == 0)
	return;

    // convert the current sensor ADC value to temperature
    for (iIdx = 0; iIdx < SYSTEM_NUM_SENSORS; iIdx++)
    {
	temperature_subsampling_calculate(iIdx);
	memset(dvalue.sensor[iIdx].value,'\0',sizeof(dvalue.sensor[iIdx].value));
	getFloatNumber2Text(A0tempdegC,dvalue.sensor[iIdx].value);
/*	if(iIdx==0)
	{
	    memset(dvalue.sensor[0].value,'\0',sizeof(dvalue.sensor[0].value));
	    getFloatNumber2Text(A0tempdegC,dvalue.sensor[0].value);
	    debug_print("\n\rsensor A = ");
	    debug_print(dvalue.sensor[0].value);
	    debug_print("\n\r");
	}
	else
	{
	    memset(dvalue.sensor[1].value,'\0',sizeof(dvalue.sensor[1].value));
	    getFloatNumber2Text(A0tempdegC,dvalue.sensor[1].value);
	    debug_print("\n\rsensor B = ");
	    debug_print(dvalue.sensor[1].value);
	    debug_print("\n\r");
	}*/
    }
}

void temp_check()
{
  uint8_t avg = 0;
  float avg_temp[3]={0.0,0.0,0.0};
  char blank[]={"----"};
  uint8_t sensor_count = 0;
  
  for(avg= 1; avg <10 ;avg++)
  {
    temperature_subsamples(1);
    temperature_single_capture();//turn on the adc
    __delay_cycles(10000);
    temperature_analog_to_digital_conversion();
    for(sensor_count = 0; sensor_count < 2; sensor_count++)
    {
      avg_temp[sensor_count] = (atof(dvalue.sensor[sensor_count].value) + avg_temp[sensor_count]);
    }
  }

  debug_print("temperature_data = ");
  debug_print((char *)dvalue.sensor[0].value);
  debug_print((char *)dvalue.sensor[1].value);

  for(sensor_count = 0; sensor_count < 2; sensor_count++)
  {
    avg_temp[sensor_count] = avg_temp[sensor_count]/(avg-1);
    if(strcmp(blank,dvalue.sensor[sensor_count].value) == 0)
    {
      if(temp_disconnt[sensor_count] != '1')
      {
        temp_disconnt[sensor_count] = '1';
        flags.sensors_altered = 1;
      }
      memset(dvalue.sensor[sensor_count].value,'\0',sizeof(dvalue.sensor[sensor_count].value));
    }
    else
    {
      memset(dvalue.sensor[sensor_count].value,'\0',sizeof(dvalue.sensor[sensor_count].value));
      getFloatNumber2Text(avg_temp[sensor_count],dvalue.sensor[sensor_count].value);
      if(temp_disconnt[sensor_count] != '0')
      {
        temp_disconnt[sensor_count] = '0';
        flags.sensors_altered = 1;
      }
    }
  }
}

void temperature_subsamples(uint8_t samples)
{
    USE_TEMPERATURE

	if (tem->iSamplesRequired!=0)
	    return;

    temperature_trigger_init();
    tem->iSamplesRequired = samples;
}

void temperature_trigger_capture()
{
    USE_TEMPERATURE

	// SUBSAMPLING TOO CLOSE FOR THE TRIGGER TO WORK

	// We have to capture all the sensors to relaunch the sampling interruption
	// iCapturing will be incremented per sensor captured
	if (tem->iCapturing > 0 && tem->iCapturing <= SYSTEM_NUM_SENSORS)
	    return;

    if (tem->iCapturing == 0)
    {
	tem->iCapturing = 1;
    }

    // Turn on ADC conversion
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 |= ADC12ENC | ADC12SC;
}

void temperature_single_capture()
{
    USE_TEMPERATURE
	//	config_setLastCommand(COMMAND_TEMPERATURE_SAMPLE);
	if (tem->iSamplesRequired==0)
	    return;

    if (tem->iSamplesRead == tem->iSamplesRequired)
    {
	temperature_analog_to_digital_conversion();
	tem->iSamplesRequired = 0;
	return;
    }
    temperature_trigger_capture();
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector = ADC12_VECTOR
__interrupt
void ADC12_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12_ISR (void)
#else
#error Compiler not supported!
#endif
{
    USE_TEMPERATURE
	switch (__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG)) {

	  case ADC12IV_NONE:
	    break;        // Vector  0:  No interrupt
	  case ADC12IV_ADC12OVIFG:
	    break;        // Vector  2:  ADC12MEMx Overflow
	  case ADC12IV_ADC12TOVIFG:
	    break;        // Vector  4:  Conversion time overflow
	  case ADC12IV_ADC12HIIFG:
	    break;        // Vector  6:  ADC12BHI
	  case ADC12IV_ADC12LOIFG:
	    break;        // Vector  8:  ADC12BLO
	  case ADC12IV_ADC12INIFG:
	    break;        // Vector 10:  ADC12BIN
#ifndef _FISM_
	  case ADC12IV_ADC12IFG0:           // Vector 12:  ADC12MEM0 Interrupt
	    if (ADC12MEM0 >= 0x7ff) { // ADC12MEM0 = A1 > 0.5AVcc?
		P1OUT |= BIT0;      // P1.0 = 1
	    } else {
		P1OUT &= ~BIT0;     // P1.0 = 0
	    }
	    __bic_SR_register_on_exit(LPM0_bits); // Exit active CPU
	    break;                    // Clear CPUOFF bit from 0(SR)

	  case ADC12IV_ADC12IFG1:
	    break;                    // Vector 14:  ADC12MEM1
	  case ADC12IV_ADC12IFG2:   	  // Vector 16:  ADC12MEM2
	    tem->sensors[0].iADC += ADC12MEM2;
	    tem->sensors[0].iSamplesRead++;
	    break;

	  case ADC12IV_ADC12IFG3:           // Vector 18:  ADC12MEM3
	    tem->sensors[1].iADC += ADC12MEM3;
	    tem->sensors[1].iSamplesRead++;
	    break;

	  case ADC12IV_ADC12IFG4:   	  // Vector 20:  ADC12MEM4
	    tem->sensors[2].iADC += ADC12MEM4;
	    tem->sensors[2].iSamplesRead++;
	    break;

	  case ADC12IV_ADC12IFG5:   	  // Vector 22:  ADC12MEM5
	    tem->sensors[3].iADC += ADC12MEM5;
	    tem->sensors[3].iSamplesRead++;
	    break;

	  case ADC12IV_ADC12IFG6:           // Vector 24:  ADC12MEM6
	    tem->sensors[4].iADC += ADC12MEM6;
	    tem->sensors[4].iSamplesRead++;
	    break;

	  case ADC12IV_ADC12RDYIFG:
	    break;        // Vector 76:  ADC12RDY
#else
	  case ADC12IV_ADC12IFG10:
	    tem->sensors[0].iADC += ADC12MEM10;
	    tem->sensors[0].iSamplesRead++;
	    ADC12IER0 &= ~(ADC12IE10);
	    break;

	  case ADC12IV_ADC12IFG11:
	    tem->sensors[1].iADC += ADC12MEM11;
	    tem->sensors[1].iSamplesRead++;
	    ADC12IER0 &= ~(ADC12IE11);
	    break;

	  case ADC12IV_ADC12IFG7:
	    charger_con2 = ADC12MEM7;
	    ADC12IER0 &= ~(ADC12IE7);
#ifndef _FDEBUG_
	    debug_print("imon=");
	    debug_print(itoa_nopadding(charger_con2));
	    debug_print("\n\r");
#endif
	    break;

	  case ADC12IV_ADC12IFG15:
	    charger_con = ADC12MEM15;
	    ADC12IER0 &= ~(ADC12IE15);
#ifdef _FDEBUG_
	    //                debug_print("charger_ts=");
	    //              debug_print(itoa_nopadding(charger_con));
	    //            debug_print("\n\r");

	    if(charger_con < 600)
	    {
		debug_print("charger out\n\r");
	    }
	    else
	    {
		debug_print("charger connected\n\r");
	    }
#endif
	    break;

#endif
	  default:
	    break;
	}

    tem->iCapturing++;
    if (tem->iCapturing == SYSTEM_NUM_SENSORS + 1) {
	tem->iSamplesRead++;
	tem->iCapturing = 0;
	// Instantly run this event after the capture is finished
	if (tem->iSamplesRequired>1 && tem->iSamplesRead<=tem->iSamplesRequired)
	    event_force_event_by_id(EVT_SUBSAMPLE_TEMP, 0);
	WAKEUP_MAIN
    }
}