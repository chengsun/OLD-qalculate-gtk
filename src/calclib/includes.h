/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef INCLUDES_H
#define INCLUDES_H

typedef enum {
	NUMBER_FORMAT_NORMAL,
	NUMBER_FORMAT_DECIMALS,
	NUMBER_FORMAT_EXP,
	NUMBER_FORMAT_EXP_PURE,
	NUMBER_FORMAT_PREFIX,
	NUMBER_FORMAT_HEX,
	NUMBER_FORMAT_OCTAL,
	NUMBER_FORMAT_BIN
} NumberFormat;

typedef enum {
	UNIT_FORMAT_DEFAULT		= 1 << 0,
	UNIT_FORMAT_SCIENTIFIC		= 1 << 1,
	UNIT_FORMAT_BEAUTIFY		= 1 << 3,
	UNIT_FORMAT_NONASCII		= 1 << 4,
	UNIT_FORMAT_TAGS		= 1 << 5,
	UNIT_FORMAT_ALLOW_NOT_USABLE	= 1 << 6,
	UNIT_FORMAT_SHORT		= 1 << 7,
	UNIT_FORMAT_LONG		= 1 << 8,	
	UNIT_FORMAT_HIDE		= 1 << 9	
} UnitFormatFlags;

#define PRECISION	12
#define PI_VALUE	3.1415926535897932384626433832795029L
#define E_VALUE		2.7182818284590452353602874713526625L
#define RADIANS		1
#define DEGREES		2
#define GRADIANS	3

#define SIGN_POWER_0		"°"
#define SIGN_POWER_1		"¹"
#define SIGN_POWER_2		"²"
#define SIGN_POWER_3		"³"
#define SIGN_EURO		"€"
#define SIGN_MICRO		"µ"

#define ID_WRAP_LEFT_CH		'{'
#define ID_WRAP_RIGHT_CH	'}'

extern char *ID_WRAP_LEFT_STR;
extern char *ID_WRAP_RIGHT_STR;
extern char *ID_WRAP_S;
extern char *NUMBERS_S;
extern char *SIGNS_S;
extern char *OPERATORS_S;
extern char *BRACKETS_S;
extern char *LEFT_BRACKET_S;
extern char *LEFT_BRACKET_STR;
extern char *RIGHT_BRACKET_S;
extern char *RIGHT_BRACKET_STR;
extern char *DOT_STR;
extern char *DOT_S;
extern char *SPACE_S;
extern char *SPACE_STR;
extern char *RESERVED_S;
extern char *PLUS_S;
extern char *PLUS_STR;
extern char *MINUS_S;
extern char *MINUS_STR;
extern char *MULTIPLICATION_S;
extern char *MULTIPLICATION_STR;
extern char *DIVISION_S;
extern char *DIVISION_STR;
extern char *EXP_S;
extern char *EXP_STR;
extern char *POWER_STR;
extern char *POWER_S;
extern char *INF_STR;
extern char *NAN_STR;
extern char *COMMA_S;
extern char *COMMA_STR;
extern char *UNDERSCORE_STR;
extern char *UNDERSCORE_S;
extern char *NAME_NUMBER_PRE_S;
extern char *NAME_NUMBER_PRE_STR;
extern char *FUNCTION_VAR_PRE_STR;
extern char *FUNCTION_VAR_X;
extern char *FUNCTION_VAR_Y;
extern char *ZERO_STR;
extern char *ONE_STR;
extern char *ILLEGAL_IN_NAMES;
extern char *ILLEGAL_IN_UNITNAMES;
extern char *ILLEGAL_IN_NAMES_MINUS_SPACE_STR;
/*extern char *ILLEGAL_IN_NAMES_MINUS_SPACE_STR_PLUS_NUMBERS_S;
extern char *BRACKETS_S_AND_OPERATORS_S;
extern char *NUMBERS_S_AND_OPERATORS_S_AND_BRACKETS_S_AND_SPACE_S;
extern char *PLUS_S_AND_SPACE_S;
extern char *NUMBERS_S_AND_MINUS_S_AND_DOT_S;*/

#define DOT_CH			'.'
#define ZERO_CH			'0'
#define ONE_CH			'1'
#define TWO_CH			'2'
#define THREE_CH		'3'
#define FOUR_CH			'4'
#define FIVE_CH			'5'
#define SIX_CH			'6'
#define SEVEN_CH		'7'
#define EIGHT_CH		'8'
#define NINE_CH			'9'
#define PLUS_CH			'+'
#define MINUS_CH		'-'
#define MULTIPLICATION_CH	'*'
#define MULTIPLICATION_2_CH	' '
#define DIVISION_CH		'/'
#define EXP_CH			'E'
#define POWER_CH		'^'
#define SPACE_CH		' '
#define LEFT_BRACKET_CH		'('
#define RIGHT_BRACKET_CH	')'
#define FUNCTION_VAR_PRE_CH	'\\'
#define COMMA_CH		','
#define NAME_NUMBER_PRE_CH	'_'
#define UNIT_DIVISION_CH	'/'

#define ID_WRAPS	"{}"	
#define DOT		"."
#define COMMA		","
#define NUMBERS		"0123456789"
#define SIGNS		"+-*/^E"
#define OPERATORS	"+-*/^E"
#define	BRACKETS	"()"
#define LEFT_BRACKET	"("
#define	RIGHT_BRACKET	")"
#define	SPACES		" \t\n"
#define SPACE		" "
#define RESERVED	"@?!\\{}&':<>|\""
#define PLUS		"+"
#define MINUS		"-"
#define MULTIPLICATION	"*"
#define DIVISION	"/"
#define EXP		"E"
#define	POWER		"^"
#define SINF		"INF"
#define SNAN		"NAN"
#define UNDERSCORE	"_"

#define NOT_IN_NAMES 	RESERVED OPERATORS SPACES DOT BRACKETS

using namespace std;

#include <vector>
#include <string>
#include <stack>
#include <list>
#include <ext/hash_map>
#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

using namespace __gnu_cxx;

#endif
