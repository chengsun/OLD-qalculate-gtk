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
#include <unistd.h>

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

static string empty_string;


class Calculator;
class MathStructure;
class Manager;
class Unit;
class Variable;
class KnownVariable;
class UnknownVariable;
class Assumptions;
class DynamicVariable;
class ExpressionItem;
class Number;
class Prefix;
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

#define COMPARISON_MIGHT_BE_LESS_OR_GREATER(i)	(i == COMPARISON_RESULT_UNKNOWN || i == COMPARISON_RESULT_NOT_EQUAL)
#define COMPARISON_NOT_FULLY_KNOWN(i)		(i == COMPARISON_RESULT_UNKNOWN || i == COMPARISON_RESULT_NOT_EQUAL || i == COMPARISON_RESULT_EQUAL_OR_LESS || i == COMPARISON_RESULT_EQUAL_OR_GREATER)
#define COMPARISON_IS_EQUAL_OR_GREATER(i)	(i == COMPARISON_RESULT_EQUAL || i == COMPARISON_RESULT_GREATER || i == COMPARISON_RESULT_EQUAL_OR_GREATER)
#define COMPARISON_IS_EQUAL_OR_LESS(i)		(i == COMPARISON_RESULT_EQUAL || i == COMPARISON_RESULT_LESS || i == COMPARISON_RESULT_EQUAL_OR_LESS)
#define COMPARISON_IS_NOT_EQUAL(i)		(i == COMPARISON_RESULT_NOT_EQUAL || i == COMPARISON_RESULT_LESS || i == COMPARISON_RESULT_GREATER)
#define COMPARISON_MIGHT_BE_EQUAL(i)		(i == COMPARISON_RESULT_UNKNOWN || i == COMPARISON_RESULT_EQUAL_OR_LESS || i == COMPARISON_RESULT_EQUAL_OR_GREATER)
#define COMPARISON_MIGHT_BE_NOT_EQUAL(i)	(i == COMPARISON_RESULT_UNKNOWN || i == COMPARISON_RESULT_EQUAL_OR_LESS || i == COMPARISON_RESULT_EQUAL_OR_GREATER)

typedef enum {
	COMPARISON_RESULT_EQUAL,
	COMPARISON_RESULT_GREATER,
	COMPARISON_RESULT_LESS,
	COMPARISON_RESULT_EQUAL_OR_GREATER,
	COMPARISON_RESULT_EQUAL_OR_LESS,
	COMPARISON_RESULT_NOT_EQUAL,
	COMPARISON_RESULT_UNKNOWN
} ComparisonResult;

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
	OPERATION_XOR,
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
	SORT_DEFAULT				= 1 << 0,
	SORT_SCIENTIFIC				= 1 << 1
} SortFlags;

#define BASE_ROMAN_NUMERALS	-1
#define BASE_TIME		-2
#define BASE_BINARY		2
#define BASE_OCTAL		8
#define BASE_DECIMAL		10
#define BASE_HEXADECIMAL	16
#define BASE_SEXAGESIMAL	60

#define EXP_PRECISION		-1
#define EXP_NONE		0
#define EXP_PURE		1
#define EXP_SCIENTIFIC		3

typedef enum {
	FRACTION_DECIMAL,
	FRACTION_DECIMAL_EXACT,
	FRACTION_FRACTIONAL,
	FRACTION_COMBINED
} NumberFractionFormat;

static const struct SortOptions {
	bool prefix_currencies;
	bool minus_last;
	SortOptions() : prefix_currencies(true), minus_last(true) {}
} default_sort_options;

static const struct PrintOptions {
	int min_exp;
	int base;
	bool lower_case_numbers;
	NumberFractionFormat number_fraction_format;
	bool indicate_infinite_series;
	bool abbreviate_units;
	bool place_units_separately;
	bool use_unit_prefixes;
	bool use_prefixes_for_currencies;
	bool use_all_prefixes;
	bool use_denominator_prefix;
	bool negative_exponents;
	bool short_multiplication;
	bool allow_non_usable;
	bool use_unicode_signs;
	bool spacious;
	bool excessive_parenthesis;
	bool halfexp_to_sqrt;
	int min_decimals;
	int max_decimals;
	bool use_min_decimals;
	bool use_max_decimals;
	Prefix *prefix;
	bool *is_approximate;
	SortOptions sort_options;
	string comma_sign, decimalpoint_sign;
	PrintOptions() : min_exp(EXP_PRECISION), base(BASE_DECIMAL), lower_case_numbers(false), number_fraction_format(FRACTION_DECIMAL), indicate_infinite_series(false), abbreviate_units(true), place_units_separately(true), use_unit_prefixes(true), use_prefixes_for_currencies(false), use_all_prefixes(false), use_denominator_prefix(true), negative_exponents(false), short_multiplication(true), allow_non_usable(false), use_unicode_signs(false), spacious(true), excessive_parenthesis(false), halfexp_to_sqrt(true), min_decimals(0), max_decimals(-1), use_min_decimals(true), use_max_decimals(true), prefix(NULL), is_approximate(NULL) {}
	const string &comma() const;
	const string &decimalpoint() const;
} default_print_options;

static const struct InternalPrintStruct {
	int depth, power_depth, division_depth;
	bool wrap;
	string *num, *den, *re, *im, *exp;
	bool *minus, *exp_minus;
	InternalPrintStruct() : depth(0), power_depth(0), division_depth(0), wrap(false), num(NULL), den(NULL), re(NULL), im(NULL), exp(NULL), minus(NULL), exp_minus(NULL) {}
} top_ips;

typedef enum {
	APPROXIMATION_EXACT,
	APPROXIMATION_TRY_EXACT,
	APPROXIMATION_APPROXIMATE
} ApproximationMode;

typedef enum {
	STRUCTURING_NONE,
	STRUCTURING_SIMPLIFY,
	STRUCTURING_FACTORIZE
} StructuringMode;

static const struct ParseOptions {
	bool variables_enabled, functions_enabled, unknowns_enabled, units_enabled;
	bool rpn;
	ParseOptions() : variables_enabled(true), functions_enabled(true), unknowns_enabled(true), units_enabled(true), rpn(false) {}
} default_parse_options;

static const struct EvaluationOptions {
	ApproximationMode approximation;
	bool sync_units, sync_complex_unit_relations, keep_prefixes;
	bool calculate_variables, calculate_functions, test_comparisons, isolate_x;
	bool simplify_addition_powers;
	StructuringMode structuring;
	ParseOptions parse_options;
	EvaluationOptions() : approximation(APPROXIMATION_TRY_EXACT), sync_units(true), sync_complex_unit_relations(true), keep_prefixes(false), calculate_variables(true), calculate_functions(true), test_comparisons(true), isolate_x(true), simplify_addition_powers(true), structuring(STRUCTURING_SIMPLIFY) {}
} default_evaluation_options;

#define DEFAULT_PRECISION	8
#define PRECISION		CALCULATOR->getPrecision()

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
#define SIGN_CAPITAL_GAMMA		"Γ"
#define SIGN_CAPITAL_BETA		"Β"
#define SIGN_INFINITY			"∞"

#define ID_WRAP_LEFT_CH		'{'
#define ID_WRAP_RIGHT_CH	'}'

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
#define SEXADOT			":"
#define COMMA			","
#define COMMAS			",;"
#define NUMBERS			"0123456789"
#define NUMBER_ELEMENTS		"0123456789.:"
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
#define RESERVED		"\'@?\\{}:\""
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
