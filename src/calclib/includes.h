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
	NUMBER_FORMAT_OCTAL
} NumberFormat;

typedef enum {
	UNIT_FORMAT_DEFAULT		= 1 << 0,
	UNIT_FORMAT_SCIENTIFIC		= 1 << 1,
	UNIT_FORMAT_BEAUTIFY		= 1 << 3,
	UNIT_FORMAT_NONASCII		= 1 << 4,
	UNIT_FORMAT_TAGS		= 1 << 5,
	UNIT_FORMAT_ALLOW_NOT_USABLE	= 1 << 6,
	UNIT_FORMAT_SHORT		= 1 << 7,
	UNIT_FORMAT_HIDE		= 1 << 8	
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

#define ID_WRAP_LEFT_STR	"{"
#define ID_WRAP_RIGHT_STR	"}"
#define ID_WRAP_S		"{}"

#define NUMBERS_S		"0123456789"
#define SIGNS_S			"+-*/^E"
#define OPERATORS_S		"+-*/^E"
#define BRACKETS_S		"()[]"
#define LEFT_BRACKET_S		"(["
#define LEFT_BRACKET_STR	"("
#define RIGHT_BRACKET_S		"])"
#define RIGHT_BRACKET_STR	")"
#define DOT_STR			"."
#define DOT_S			"."
#define SPACE_S			" \t\n"
#define SPACE_STR		" "
#define RESERVED_S		"?!\\{}&':<>|"
#define PLUS_S			"+"
#define PLUS_STR		"+"
#define MINUS_S			"-"
#define MINUS_STR		"-"
#define MULTIPLICATION_S	"*"
#define MULTIPLICATION_STR	"*"
#define DIVISION_S		"/"
#define DIVISION_STR		"/"
#define EXP_S			"E"
#define EXP_STR			"E"
#define POWER_STR		"^"
#define POWER_S			"^"
#define INF_STR			"INF"
#define NAN_STR			"NAN"
#define COMMA_S			",;"
#define COMMA_STR		","
#define ILLEGAL_IN_NAMES			"?!\\{}&':<>|+-*/^E()[] \t."
#define ILLEGAL_IN_NAMES_MINUS_SPACE_STR	"?!\\{}&':<>|+-*/^E()[]\t."
#define UNDERSCORE_STR		"_"
#define UNDERSCORE_S		"_"
#define NAME_NUMBER_PRE_S	"_~#"
#define NAME_NUMBER_PRE_STR	"_"
#define FUNCTION_VAR_PRE_STR	"\\"
#define ZERO_STR		"0"
#define ONE_STR			"1"

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
