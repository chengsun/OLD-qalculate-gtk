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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

class Calculator;
class Manager;
class Unit;
class Variable;
class ExpressionItem;
class Function;
class Prefix;
class Integer;
class Error;
class CompositeUnit;
class AliasUnit;
class AliasUnit_Composite;
class Fraction;
class Matrix;
class Vector;
class UserFunction;
class EqItem;
class EqNumber;
class EqContainer;

enum {
	TYPE_VARIABLE,
	TYPE_FUNCTION,
	TYPE_UNIT
};

typedef enum {
	NUMBER_FORMAT_NORMAL,
	NUMBER_FORMAT_DECIMALS,
	NUMBER_FORMAT_EXP,
	NUMBER_FORMAT_EXP_PURE,
	NUMBER_FORMAT_HEX,
	NUMBER_FORMAT_OCTAL,
	NUMBER_FORMAT_BIN
} NumberFormat;

typedef enum {
	MULTIPLY,
	DIVIDE,
	ADD,
	SUBTRACT,
	RAISE,
	EXP10
} MathOperation;

typedef enum {
	DISPLAY_FORMAT_DEFAULT			= 1 << 0,
	DISPLAY_FORMAT_SCIENTIFIC		= 1 << 1,
	DISPLAY_FORMAT_FRACTION			= 1 << 2,
	DISPLAY_FORMAT_FRACTIONAL_ONLY		= 1 << 3,	
	DISPLAY_FORMAT_DECIMAL_ONLY		= 1 << 4,	
	DISPLAY_FORMAT_BEAUTIFY			= 1 << 5,
	DISPLAY_FORMAT_NONASCII			= 1 << 6,
	DISPLAY_FORMAT_TAGS			= 1 << 7,
	DISPLAY_FORMAT_ALLOW_NOT_USABLE		= 1 << 8,
	DISPLAY_FORMAT_USE_PREFIXES		= 1 << 9,
	DISPLAY_FORMAT_SHORT_UNITS		= 1 << 10,
	DISPLAY_FORMAT_LONG_UNITS		= 1 << 11,	
	DISPLAY_FORMAT_HIDE_UNITS		= 1 << 12,
	DISPLAY_FORMAT_INDICATE_INFINITE_SERIES	= 1 << 13,
	DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT	= 1 << 14	
} DisplayFormatFlags;

typedef enum {
	SORT_DEFAULT				= 1 << 0,
	SORT_SCIENTIFIC				= 1 << 1
} SortFlags;

#define DEFAULT_PRECISION	12
#define PRECISION		CALCULATOR->getPrecision()

#define PI_VALUE		3.1415926535897932384626433832795029L
#define E_VALUE			2.7182818284590452353602874713526625L
#define E_STRING		"2.718281828459045235360287471352662497757247093699959574"
#define PI_STRING		"3.1415926535897932384626433832795028841971693993751"
#define PYTHAGORAS_STRING	"1.41421356237309504880168872420969807856967187537694"
#define EULER_STRING		"0.57721566490153286060651209008240243104215933593992"
#define	APERY_STRING		"1.20205690315959428539973816151144999076498629234049"
#define GOLDEN_STRING		"1.6180339887498948482045868343656381177203091798057628621"

#define RADIANS		1
#define DEGREES		2
#define GRADIANS	3

#define SIGN_POWER_0			"°"
#define SIGN_POWER_1			"¹"
#define SIGN_POWER_2			"²"
#define SIGN_POWER_3			"³"
#define SIGN_EURO			"€"
#define SIGN_MICRO			"µ"
#define SIGN_PI				"π"
#define SIGN_MULTIPLICATION		"×"
#define SIGN_MULTIDOT			"⋅"
#define SIGN_DIVISION			"∕"
#define SIGN_MINUS			"−"
#define SIGN_PLUS			"＋"
#define SIGN_SQRT			"√"
#define SIGN_ALMOST_EQUAL		"≈"
#define SIGN_APPROXIMATELY_EQUAL	"≅"
#define	SIGN_ZETA			"ζ"
#define SIGN_GAMMA			"γ"
#define SIGN_PHI			"φ"

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

#define ID_WRAP_LEFT	"{"	
#define ID_WRAP_RIGHT	"}"	
#define ID_WRAPS	"{}"	
#define DOT		"."
#define COMMA		","
#define NUMBERS		"0123456789"
#define SIGNS		"+-*/^"
#define OPERATORS	"+-*/^"
#define	BRACKETS	"()"
#define LEFT_BRACKET	"("
#define	RIGHT_BRACKET	")"
#define	SPACES		" \t\n"
#define SPACE		" "
#define RESERVED	"@?!\\{}&:<>|\""
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
