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
#include <errno.h>
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

struct ExpressionName;
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
class MathFunction;
class Matrix;
class Vector;
class UserFunction;
class EqItem;
class EqNumber;
class EqContainer;
class Argument;
class DataSet;
class DataProperty;
class DataObject;

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

#define NR_OF_PRIMES 174

static const int PRIMES[] = {
2, 3, 5, 7, 11, 13, 17, 19, 21, 23, 29, 31, 37, 41, 31, 37, 
41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 
107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 
173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 
239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 
311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 
383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 
457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523,
541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 
613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 
683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 
769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 
857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 
941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013
};

#define SQP_LT_1000 11
#define SQP_LT_2000 17
#define SQP_LT_10000 28
#define SQP_LT_25000 40
#define SQP_LT_100000 68

static const int SQUARE_PRIMES[] = {
4, 9, 25, 49, 121, 169, 289, 361, 441, 529, 
841, 961, 1369, 1681, 961, 1369, 1681, 1849, 2209, 2809, 
3481, 3721, 4489, 5041, 5329, 6241, 6889, 7921, 9409, 10201, 
10609, 11449, 11881, 12769, 16129, 17161, 18769, 19321, 22201, 22801, 
24649, 26569, 27889, 29929, 32041, 32761, 36481, 37249, 38809, 39601, 
44521, 49729, 51529, 52441, 54289, 57121, 58081, 63001, 66049, 69169, 
72361, 73441, 76729, 78961, 80089, 85849, 94249, 96721, 97969, 100489, 
109561, 113569, 120409, 121801, 124609, 128881, 134689, 139129, 143641, 146689, 
151321, 157609, 160801, 167281, 175561, 177241, 185761, 187489, 192721, 196249, 
201601, 208849, 212521, 214369, 218089, 229441, 237169, 241081, 249001, 253009, 
259081, 271441, 273529, 292681, 299209, 310249, 316969, 323761, 326041, 332929, 
344569, 351649, 358801, 361201, 368449, 375769, 380689, 383161, 398161, 410881, 
413449, 418609, 426409, 434281, 436921, 452929, 458329, 466489, 477481, 491401, 
502681, 516961, 528529, 537289, 546121, 552049, 564001, 573049, 579121, 591361, 
597529, 619369, 635209, 654481, 657721, 674041, 677329, 683929, 687241, 703921, 
727609, 734449, 737881, 744769, 769129, 776161, 779689, 786769, 822649, 829921, 
844561, 863041, 877969, 885481, 896809, 908209, 935089, 942841, 954529, 966289, 
982081, 994009, 1018081, 1026169
};

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
	bool indicate_infinite_series, show_ending_zeroes;
	bool abbreviate_names, use_reference_names;
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
	bool round_halfway_to_even;
	bool improve_division_multipliers;
	Prefix *prefix;
	bool *is_approximate;
	SortOptions sort_options;
	string comma_sign, decimalpoint_sign;
	PrintOptions() : min_exp(EXP_PRECISION), base(BASE_DECIMAL), lower_case_numbers(false), number_fraction_format(FRACTION_DECIMAL), indicate_infinite_series(false), show_ending_zeroes(false), abbreviate_names(true), use_reference_names(false), place_units_separately(true), use_unit_prefixes(true), use_prefixes_for_currencies(false), use_all_prefixes(false), use_denominator_prefix(true), negative_exponents(false), short_multiplication(true), allow_non_usable(false), use_unicode_signs(false), spacious(true), excessive_parenthesis(false), halfexp_to_sqrt(true), min_decimals(0), max_decimals(-1), use_min_decimals(true), use_max_decimals(true), round_halfway_to_even(false), improve_division_multipliers(true), prefix(NULL), is_approximate(NULL) {}
	const string &comma() const;
	const string &decimalpoint() const;
} default_print_options;

static const struct InternalPrintStruct {
	int depth, power_depth, division_depth;
	bool wrap;
	string *num, *den, *re, *im, *exp;
	bool *minus, *exp_minus;
	bool parent_approximate;
	int parent_precision;
	InternalPrintStruct() : depth(0), power_depth(0), division_depth(0), wrap(false), num(NULL), den(NULL), re(NULL), im(NULL), exp(NULL), minus(NULL), exp_minus(NULL), parent_approximate(false), parent_precision(-1) {}
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

typedef enum {
	POST_CONVERSION_NONE,
	POST_CONVERSION_BEST,
	POST_CONVERSION_BASE
} AutoPostConversion;

typedef enum {
	DONT_READ_PRECISION,
	ALWAYS_READ_PRECISION,
	READ_PRECISION_WHEN_DECIMALS
} ReadPrecisionMode;

static const struct ParseOptions {
	bool variables_enabled, functions_enabled, unknowns_enabled, units_enabled;
	bool rpn;
	int base;
	ReadPrecisionMode read_precision;
	ParseOptions() : variables_enabled(true), functions_enabled(true), unknowns_enabled(true), units_enabled(true), rpn(false), base(BASE_DECIMAL), read_precision(DONT_READ_PRECISION) {}
} default_parse_options;

typedef enum {
	RADIANS,
	DEGREES,
	GRADIANS
} AngleUnit;

static const struct EvaluationOptions {
	ApproximationMode approximation;
	bool sync_units, sync_complex_unit_relations, keep_prefixes;
	bool calculate_variables, calculate_functions, test_comparisons, isolate_x;
	bool simplify_addition_powers, reduce_divisions, do_polynomial_division;
	bool allow_complex, allow_infinite;
	bool assume_denominators_nonzero;
	bool split_squares;
	AutoPostConversion auto_post_conversion;
	StructuringMode structuring;
	ParseOptions parse_options;
	AngleUnit angle_unit;
	const MathStructure *isolate_var;
	EvaluationOptions() : approximation(APPROXIMATION_TRY_EXACT), sync_units(true), sync_complex_unit_relations(true), keep_prefixes(false), calculate_variables(true), calculate_functions(true), test_comparisons(true), isolate_x(true), simplify_addition_powers(true), reduce_divisions(true), do_polynomial_division(true), allow_complex(true), allow_infinite(true), assume_denominators_nonzero(false), split_squares(true), auto_post_conversion(POST_CONVERSION_NONE), structuring(STRUCTURING_SIMPLIFY), angle_unit(RADIANS), isolate_var(NULL) {}
} default_evaluation_options;

extern MathStructure m_undefined, m_empty_vector, m_empty_matrix, m_zero, m_one, m_minus_one;
extern EvaluationOptions no_evaluation;
extern ExpressionName empty_expression_name;

extern Calculator *calculator;

#define CALCULATOR	calculator

#define DEFAULT_PRECISION	8
#define PRECISION		CALCULATOR->getPrecision()

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
