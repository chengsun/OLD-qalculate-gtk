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

#ifdef HAVE_GIAC
#  undef PACKAGE
#  undef VERSION
#  include <giac/giac.h>
#  undef PACKAGE
#  undef VERSION
#  ifdef HAVE_CONFIG_H
#    include <config.h>
#  endif
#endif

class Calculator;
class Manager;
class Unit;
class Variable;
class DynamicVariable;
class ExpressionItem;
class Number;
class Prefix;
class Error;
class CompositeUnit;
class AliasUnit;
class AliasUnit_Composite;
class Function;
class Matrix;
class Vector;
class UserFunction;
class EqItem;
class EqNumber;
class EqContainer;
class Argument;

enum {
	TYPE_VARIABLE,
	TYPE_FUNCTION,
	TYPE_UNIT
};

#define BASE_ROMAN_NUMERALS	-1

typedef enum {
	PLOT_LEGEND_NONE,
	PLOT_LEGEND_TOP_LEFT,
	PLOT_LEGEND_TOP_RIGHT,
	PLOT_LEGEND_BOTTOM_LEFT,
	PLOT_LEGEND_BOTTOM_RIGHT,
	PLOT_LEGEND_BELOW,
	PLOT_LEGEND_OUTSIDE
} PlotLegendPlacement;

typedef enum {
	PLOT_STYLE_LINES,
	PLOT_STYLE_POINTS,
	PLOT_STYLE_POINTS_LINES,
	PLOT_STYLE_BOXES,
	PLOT_STYLE_HISTOGRAM,
	PLOT_STYLE_STEPS,
	PLOT_STYLE_CANDLESTICKS,
	PLOT_STYLE_DOTS
} PlotStyle;

typedef enum {
	PLOT_SMOOTHING_NONE,
	PLOT_SMOOTHING_UNIQUE,
	PLOT_SMOOTHING_CSPLINES,
	PLOT_SMOOTHING_BEZIER,
	PLOT_SMOOTHING_SBEZIER
} PlotSmoothing;

typedef enum {
	NUMBER_FORMAT_NORMAL,
	NUMBER_FORMAT_DECIMALS,
	NUMBER_FORMAT_EXP,
	NUMBER_FORMAT_EXP_PURE,
	NUMBER_FORMAT_HEX,
	NUMBER_FORMAT_OCTAL,
	NUMBER_FORMAT_BIN,
	NUMBER_FORMAT_ROMAN
} NumberFormat;

typedef enum {
	PLOT_FILETYPE_AUTO,
	PLOT_FILETYPE_PNG,
	PLOT_FILETYPE_PS,
	PLOT_FILETYPE_EPS,
	PLOT_FILETYPE_LATEX,
	PLOT_FILETYPE_SVG,
	PLOT_FILETYPE_FIG
} PlotFileType;

typedef enum {
	OPERATION_MULTIPLY,
	OPERATION_DIVIDE,
	OPERATION_ADD,
	OPERATION_SUBTRACT,
	OPERATION_RAISE,
	OPERATION_EXP10,
	OPERATION_AND,
	OPERATION_OR,
	OPERATION_LESS,
	OPERATION_GREATER,
	OPERATION_EQUALS_LESS,
	OPERATION_EQUALS_GREATER,
	OPERATION_EQUALS,
	OPERATION_NOT_EQUALS
} MathOperation;

typedef enum {
	COMPARISON_LESS,
	COMPARISON_GREATER,
	COMPARISON_EQUALS_LESS,
	COMPARISON_EQUALS_GREATER,
	COMPARISON_EQUALS,
	COMPARISON_NOT_EQUALS
} ComparisonType;

typedef enum {
	DISPLAY_FORMAT_DEFAULT			= 1 << 0,
	DISPLAY_FORMAT_SCIENTIFIC		= 1 << 1,
	DISPLAY_FORMAT_FRACTION			= 1 << 2,
	DISPLAY_FORMAT_FRACTIONAL_ONLY		= 1 << 3,	
	DISPLAY_FORMAT_DECIMAL_ONLY		= 1 << 4,	
	DISPLAY_FORMAT_BEAUTIFY			= 1 << 5,
	DISPLAY_FORMAT_DEFINITE			= 1 << 6,
	DISPLAY_FORMAT_NONASCII			= 1 << 7,
	DISPLAY_FORMAT_TAGS			= 1 << 8,
	DISPLAY_FORMAT_ALLOW_NOT_USABLE		= 1 << 9,
	DISPLAY_FORMAT_USE_PREFIXES		= 1 << 10,
	DISPLAY_FORMAT_SHORT_UNITS		= 1 << 11,
	DISPLAY_FORMAT_LONG_UNITS		= 1 << 12,	
	DISPLAY_FORMAT_HIDE_UNITS		= 1 << 13,
	DISPLAY_FORMAT_INDICATE_INFINITE_SERIES	= 1 << 14,
	DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT	= 1 << 15	
} DisplayFormatFlags;

typedef enum {
	SORT_DEFAULT				= 1 << 0,
	SORT_SCIENTIFIC				= 1 << 1
} SortFlags;

#define DEFAULT_PRECISION	8
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
#define SIGN_POUND			"£"
#define SIGN_CENT			"¢"
#define SIGN_YEN			"¥"
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
#define	SIGN_LESS_OR_EQUAL		"≤"
#define	SIGN_GREATER_OR_EQUAL		"≥"
#define	SIGN_NOT_EQUAL			"≠"
#define SIGN_CAPITAL_SIGMA		"Σ"
#define SIGN_CAPITAL_PI			"Π"
#define SIGN_CAPITAL_OMEGA		"Ω"

#define ID_WRAP_LEFT_CH		'{'
#define ID_WRAP_RIGHT_CH	'}'

using namespace std;

#include <vector>
#include <string>
#include <stack>
#include <list>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef __GNUC__
#   if __GNUC__ < 3
#      include <hash_map.h>
       namespace Sgi { using ::hash_map; }; // inherit globals
#   else
#      include <ext/hash_map>
#      if __GNUC_MINOR__ == 0
          namespace Sgi = std;               // GCC 3.0
#      else
          namespace Sgi = ::__gnu_cxx;       // GCC 3.1 and later
#      endif
#   endif
#else      // ...  there are other compilers, right?
    namespace Sgi = std;
#endif

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
#define LEFT_PARENTHESIS_CH	'('
#define RIGHT_PARENTHESIS_CH	')'
#define LEFT_VECTOR_WRAP_CH	'['
#define RIGHT_VECTOR_WRAP_CH	']'
#define FUNCTION_VAR_PRE_CH	'\\'
#define COMMA_CH		','
#define NAME_NUMBER_PRE_CH	'_'
#define UNIT_DIVISION_CH	'/'
#define	AND_CH			'&'
#define	OR_CH			'|'
#define	LESS_CH			'<'
#define	GREATER_CH		'>'
#define	NOT_CH			'!'
#define EQUALS_CH		'='

#define ID_WRAP_LEFT		"{"	
#define ID_WRAP_RIGHT		"}"	
#define ID_WRAPS		"{}"	
#define DOT			"."
#define COMMA			","
#define COMMAS			",;"
#define NUMBERS			"0123456789"
#define SIGNS			"+-*/^"
#define OPERATORS		"+-*/^&|!<>="
#define	PARENTHESISS		"()"
#define LEFT_PARENTHESIS	"("
#define	RIGHT_PARENTHESIS	")"
#define	VECTOR_WRAPS		"[]"
#define LEFT_VECTOR_WRAP	"["
#define	RIGHT_VECTOR_WRAP	"]"
#define	SPACES			" \t\n"
#define SPACE			" "
#define RESERVED		"\'@?\\{}:\"$"
#define PLUS			"+"
#define MINUS			"-"
#define MULTIPLICATION		"*"
#define MULTIPLICATION_2	" "
#define DIVISION		"/"
#define EXP			"E"
#define	POWER			"^"
#define	AND			"&"
#define	OR			"|"
#define	LESS			"<"
#define	GREATER			">"
#define	NOT			"!"
#define	EQUALS			"="
#define SINF			"INF"
#define SNAN			"NAN"
#define UNDERSCORE		"_"

#define NOT_IN_NAMES 	RESERVED OPERATORS SPACES DOT VECTOR_WRAPS PARENTHESISS COMMAS

#endif
