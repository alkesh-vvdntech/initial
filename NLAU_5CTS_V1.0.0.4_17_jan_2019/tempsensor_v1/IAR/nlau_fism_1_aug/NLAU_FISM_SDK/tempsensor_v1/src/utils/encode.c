#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"

void encode(float input_val, char output_chars[]) {
  char SIX_BIT_ENCODER[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

  char ENCODED_STR_MAX[] = "V/";
  char ENCODED_STR_MIN[] = "/X";
  //char ENCODED_STR_NAN[] = "/W";
  
  //int LOWER6_BITMASK = 0x3f;
  //int HIGHER2_BITMASK = 0xc0;
  //int SIGNBITMASK = 0x800;
  int SIGN_BIT = 0x20;
  int WHOLE_PART_LOW2_MASK = 0x3;
  int WHOLE_PART_HIGH5_MASK = 0x7c;
  int MASK_6BITS = 0x3f;

  if (input_val > 127.0) {
    output_chars[0] = ENCODED_STR_MAX[0];
    output_chars[1] = ENCODED_STR_MAX[1];
  } else if (input_val < -127.0) {
    output_chars[0] = ENCODED_STR_MIN[0];
    output_chars[1] = ENCODED_STR_MIN[1];    
  } else {
    int sign = 0;

    // whole part
    int whole_part = (int) input_val;
    
    // decimal part to two decimal points
    int decimal_part = 0;
    if (input_val > 0) {
      decimal_part = (int) ((input_val - whole_part) * 100.0);
    } else {
      sign = 1; // number is negative
      decimal_part = (int) ((input_val - whole_part) * -100.0);
    }
    
    decimal_part = ((int) (decimal_part * 0.16)) & 0xf;
    
    int low_byte = (whole_part & WHOLE_PART_HIGH5_MASK) >> 2;
    if (sign) {
      low_byte = (~low_byte) & MASK_6BITS;
      low_byte |= SIGN_BIT;
    }
    
    int high_byte = ((whole_part & WHOLE_PART_LOW2_MASK) << 4) | decimal_part;
    if (sign) {
      high_byte = (~high_byte) & MASK_6BITS;
    }
    
    output_chars[0] = SIX_BIT_ENCODER[low_byte];
    output_chars[1] = SIX_BIT_ENCODER[high_byte];
  }
}

void encode_string(char* inputString, char* outputString, char* delimiter) {
	char *token;
	char encoded[3];
	float result;
	encoded[2] = outputString[0] = '\0';
	token = strtok(inputString, delimiter);

	while (token != NULL) {
		result = (float) atof(token);
		encode(result, encoded);
		strcat(outputString, encoded);
		token = strtok(NULL, delimiter);
	}
}

void encode_value(char* inputString, char* outputString) {
	char encoded[3];
	float result;
	encoded[2] = outputString[0] = '\0';

	result = (float) atof(inputString);
	encode(result, encoded);
        encoded[strlen(encoded)] = '\0';
	strcat(outputString, encoded);
}

