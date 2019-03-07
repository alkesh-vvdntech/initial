/*
 * stringutils.c
 *
 *  Created on: May 18, 2015
 *      Author: sergioam
 */

#include "thermalcanyon.h"

char g_iStringCaptured = 0;

#if defined(__TI_COMPILER_VERSION__)
#pragma SET_DATA_SECTION(".aggregate_vars")
char g_szStringTemp[STRING_BUFFER_HELPER_SIZE];
char g_smsMsg[MAX_SMS_SIZE_FULL];
char g_szItoa[16];
#pragma SET_DATA_SECTION()
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma location="AGGREGATE"
__no_init char g_szStringTemp[STRING_BUFFER_HELPER_SIZE];
#pragma location="AGGREGATE"
__no_init char g_smsMsg[MAX_SMS_SIZE_FULL];
#pragma location="AGGREGATE"
__no_init char g_szItoa[16];
#else
#error Compiler not supported!
#endif

// Always release this buffer so you are sure to not use it more than once!!
char *getStringBufferHelper(uint16_t *size) {
	if (size!=NULL)
		*size = sizeof(g_szStringTemp);

	if (g_iStringCaptured) {
		__no_operation();
	}
	g_iStringCaptured = 1;

	memset(g_szStringTemp, 0x0, sizeof(g_szStringTemp));

	return g_szStringTemp;
}

void releaseStringBufferHelper() {
	if (!g_iStringCaptured) {
		__no_operation();
	}
	g_iStringCaptured = 0;
}

char *getSMSBufferHelper() {
#if 1                 // rachit debug
	memset(g_smsMsg, 0x0, sizeof(g_smsMsg));
#endif
	g_smsMsg[0] = 0;
	return g_smsMsg;
}

#ifdef USEOLDFLOAT
char *getFloatNumber2Text(float number, char *ret) {
	int i = 0;
	int8_t count = 0;

	memset(ret, 0, 6);

	//Round to one digit after decimal point
	int32_t fixedPoint = (int32_t) (number * 10);

	if (number < TEMP_CUTOFF) {
		for (i = 0; i < 4; i++)
			ret[i] = '-';
		return ret;
	}

	if (number < 0) {
		count = 1;
		ret[0] = '-';
		fixedPoint = abs(fixedPoint);
	}

	for (i = 2; i >= 0; i--) {
		ret[i + count] = fixedPoint % 10 + 0x30;
		fixedPoint = fixedPoint / 10;
	}

	ret[3 + count] = ret[2 + count];
	ret[2 + count] = (char) '.';
	return ret;
}
#else
char *getFloatNumber2Text(float number, char *ret) {
        int d1 =(int) number;
        float f2 = (number -  d1)*10;
        int d2 = abs((int) (f2));
        int i = 0;

	f2 = f2 - ((int)f2);
	if(f2 < 0)
		f2 = f2*-1;

	if(d1 < 0)
		d1 = d1*-1;

	if(f2 >= 0.5) {
		d2 += 1;
		if(d2==10) {
			d1 += 1;
			d2=0;
		}
	}

	memset(ret, 0, 6);
        if (number < -80) {
                for (i = 0; i < 4; i++)
                        ret[i] = '-';
                return ret;
        }
        if (number < 0 && d1 >= 0) {
                sprintf(ret, "-%d.%d", d1, d2);
        } else {
                sprintf(ret, "%d.%d", d1, d2);
        }
        return ret;
}
#endif

/*****************************************************************************/
/* _OUTC -  Put a character in a string                                      */
/*****************************************************************************/
int _outc(char c, void *_op)
{
    return *(*((char **)_op))++ = c;
}

/*****************************************************************************/
/* _OUTS -  Append a string to another string                                */
/*****************************************************************************/
int _outs(char *s, void *_op, int len)
{
    memcpy(*((char **)_op), s, len);
    *((char **)_op) += len;
    return len;
}


char* itoa_pad(uint32_t num) {
	uint8_t digit = 0;
	uint8_t iIdx = 0;
	uint8_t iCnt = 0;
	char str[15];

	memset(g_szItoa, 0, sizeof(g_szItoa));

	if (num == 0) {
		g_szItoa[0] = 0x30;
		g_szItoa[1] = 0x30;
	} else {
		while (num != 0) {
			digit = (num % 10) + 0x30;
			str[iIdx] = digit;
			iIdx++;
			num = num / 10;
		}

		//pad with zero if single digit
		if (iIdx == 1)
                {
			g_szItoa[0] = 0x30; //leading 0
			g_szItoa[1] = str[0];
		} else
                {
			while (iIdx) {
				iIdx = iIdx - 1;
				g_szItoa[iCnt] = str[iIdx];
				iCnt = iCnt + 1;
			}
		}
	}

	return g_szItoa;
}

char* itoa_nopadding(uint32_t num) {
	uint8_t digit = 0;
	uint8_t iIdx = 0;
	uint8_t iCnt = 0;
	char str[15];

	memset(g_szItoa, 0, sizeof(g_szItoa));

	if (num == 0) {
		g_szItoa[0] = 0x30;
		//tmpstr[1] = 0x30;
	} else {
		while (num != 0) {
			digit = (num % 10) + 0x30;
			str[iIdx] = digit;
			iIdx++;
			num = num / 10;
		}

		//pad with zero if single digit
		if (iIdx == 1) {
			//tmpstr[0] = 0x30; //leading 0
			g_szItoa[0] = str[0];
		} else {
			while (iIdx) {
				iIdx = iIdx - 1;
				g_szItoa[iCnt] = str[iIdx];
				iCnt = iCnt + 1;
			}
		}
	}

	return g_szItoa;
}

char* replace_character(char* string, char charToFind, char charToReplace) {
	int i = 0;

	while (string[i] != '\0') {
		if (string[i] == charToFind)
			string[i] = charToReplace;
		i++;
	}
	return string;
}

// remove sigma corruption from states being pushed to the LCD
char* trim_sigma(char *str)
{
	int i = 0; //loop variable

	// if empty string
	if(strlen(str) == 0){
		*str = 0; //force to zero
		return str;
	}

	// check whole array
	for (i = 0; i < strlen(str); i++)
	{
		if(str[i] == (char)0x1A) //if simga char
		{
			str[i] = 0; //set 0
		}
	}

  return str;
}

char* trim_sms(char *str)
{
	int i = 0; //loop variable

	// check whole array
	for (i = 0; i < strlen(str); i++)
	{
		if(str[i] == (char)0x0D) //if 0D char
		{
			str[i] = 0; //set 0
		}
	}

  return str;
}


