/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Calculator.h"
#include "BuiltinFunctions.h"
#include "util.h"
#include "Manager.h"
#include "Unit.h"
#include "Variable.h"
#include "Function.h"
#include "ExpressionItem.h"
#include "EqItem.h"
#include "Error.h"
#include "Prefix.h"
#include "Integer.h"
#include "Matrix.h"
#include "Fraction.h"

#include <fenv.h>
#include <locale.h>

#ifdef HAVE_LIBCLN
#define WANT_OBFUSCATING_OPERATORS
#include <cln/cln.h>
using namespace cln;
#endif

#define FIRST_READ_TAB_DELIMITED_SET_0(str)	if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) { \
							str = stmp.substr(i, i2 - i); \
							if(str == "0") { \
								str = ""; \
							}

#define FIRST_READ_TAB_DELIMITED_SET_BOOL(b)	if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) { \
							if(stmp.substr(i, i2 - i) == "1") { \
								b = true; \
							} else { \
								b = false; \
							}
							
#define FIRST_READ_TAB_DELIMITED(str)		if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) { \
							str = stmp.substr(i, i2 - i);

#define READ_TAB_DELIMITED_SET_0(str)		if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) { \
							str = stmp.substr(i, i2 - i); \
							remove_blank_ends(str); \
							if(str == "0") { \
								str = ""; \
							}

#define READ_TAB_DELIMITED(str)			if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) { \
							str = stmp.substr(i, i2 - i);
						
#define READ_TAB_DELIMITED_SET_BOOL(b)		if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) { \
							if(stmp.substr(i, i2 - i) == "1") { \
								b = true; \
							} else { \
								b = false; \
							}

#define TEST_TAB_DELIMITED			if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {


 char * ID_WRAP_LEFT_STR;
 char * ID_WRAP_RIGHT_STR;
 char * ID_WRAP_S;
 char * NUMBERS_S;
 char * SIGNS_S;
 char * OPERATORS_S;
 char * BRACKETS_S;
 char * LEFT_BRACKET_S;
 char * LEFT_BRACKET_STR;
 char * RIGHT_BRACKET_S;
 char * RIGHT_BRACKET_STR;
 char * DOT_STR;
 char * DOT_S;
 char * SPACE_S;
 char * SPACE_STR;
 char * RESERVED_S;
 char * PLUS_S;
 char * PLUS_STR;
 char * MINUS_S;
 char * MINUS_STR;
 char * MULTIPLICATION_S;
 char * MULTIPLICATION_STR;
 char * DIVISION_S;
 char * DIVISION_STR;
 char * EXP_S;
 char * EXP_STR;
 char * POWER_STR;
 char * POWER_S;
 char * INF_STR;
 char * NAN_STR;
 char * COMMA_S;
 char * COMMA_STR;
 char * UNDERSCORE_STR;
 char * UNDERSCORE_S;
 char * NAME_NUMBER_PRE_S;
 char * NAME_NUMBER_PRE_STR;
 char * FUNCTION_VAR_PRE_STR;
 char * FUNCTION_VAR_X;
 char * FUNCTION_VAR_Y;
 char * ZERO_STR;
 char * ONE_STR;
 char * ILLEGAL_IN_NAMES;
 char * ILLEGAL_IN_UNITNAMES;
 char * ILLEGAL_IN_NAMES_MINUS_SPACE_STR;

void Calculator::addStringAlternative(string replacement, string standard) {
	signs.push_back(replacement);
	real_signs.push_back(standard);
}
void Calculator::addDefauktStringAlternative(string replacement, string standard) {
	default_signs.push_back(replacement);
	default_real_signs.push_back(standard);
}

Calculator *calculator;

Calculator::Calculator() {

	setPrecision(DEFAULT_PRECISION);

	setLocale();
	addStringAlternative(SIGN_POWER_0, "o");
	addStringAlternative(SIGN_POWER_1, "^1");
	addStringAlternative(SIGN_POWER_2, "^2");
	addStringAlternative(SIGN_POWER_3, "^3");
	addStringAlternative(SIGN_EURO, "euro");
	addStringAlternative(SIGN_MICRO, "micro");
	addStringAlternative(SIGN_PI, "pi");	
	addStringAlternative(SIGN_SQRT, "sqrt ");	
	addStringAlternative(SIGN_PHI, "golden");
	addStringAlternative(SIGN_ZETA, "zeta ");		
	addStringAlternative(SIGN_GAMMA, "euler");
	addStringAlternative(SIGN_DIVISION, DIVISION);	
	addStringAlternative(SIGN_MULTIPLICATION, MULTIPLICATION);		
	addStringAlternative(SIGN_MULTIDOT, MULTIPLICATION);			
	addStringAlternative(SIGN_MINUS, MINUS);		
	addStringAlternative(SIGN_PLUS, PLUS);		
	addStringAlternative("[", LEFT_BRACKET);	
	addStringAlternative("]", RIGHT_BRACKET);	
	addStringAlternative(";", COMMA);	
	addStringAlternative("\t", SPACE);	
	addStringAlternative("\n", SPACE);	


	ID_WRAP_LEFT_STR = "{";
	ID_WRAP_RIGHT_STR = "}";	
	ID_WRAP_S = "{}";	
//	DOT_STR = ".";
//	COMMA_STR = ",";
//	DOT_S = ".";
//	COMMA_S = ",;";
	NUMBERS_S = "0123456789";
	SIGNS_S = "+-*/^";
	OPERATORS_S = "+-*/^";
	BRACKETS_S = "()[]";
	LEFT_BRACKET_S = "([";
	LEFT_BRACKET_STR = "(";
	RIGHT_BRACKET_S = ")]";
	RIGHT_BRACKET_STR = ")";
	SPACE_S = " \t\n";
	SPACE_STR = " ";
	RESERVED_S = "@?!\\{}&:<>|\",;";
	PLUS_S = "+";
	PLUS_STR = "+";
	MINUS_S = "-";
	MINUS_STR = "-";
	MULTIPLICATION_S = "*";
	MULTIPLICATION_STR = "*";
	DIVISION_S = "/";
	DIVISION_STR = "/";
	EXP_S = "E";
	EXP_STR = "E";
	POWER_S = "^";
	POWER_STR = "^";
	INF_STR = "INF";
	NAN_STR = "NAN";
	UNDERSCORE_STR = "_";
	UNDERSCORE_S = "_";
	NAME_NUMBER_PRE_S = "_~#";
	NAME_NUMBER_PRE_STR = "_";
	FUNCTION_VAR_PRE_STR = "\\";
	FUNCTION_VAR_X = "\\x";	
	FUNCTION_VAR_Y = "\\y";		
	ZERO_STR = "0";
	ONE_STR = "1";  
	saved_locale = strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	
	ILLEGAL_IN_NAMES = (char*) malloc(sizeof(char) * (strlen(RESERVED_S) + strlen(OPERATORS_S) + strlen(SPACE_S) + strlen(DOT_S) + strlen(BRACKETS_S) + 1));
	sprintf(ILLEGAL_IN_NAMES, "%s%s%s%s%s", RESERVED_S, OPERATORS_S, SPACE_S, DOT_S, BRACKETS_S);
	ILLEGAL_IN_NAMES_MINUS_SPACE_STR = (char*) malloc(sizeof(char) * (strlen(RESERVED_S) + strlen(OPERATORS_S) + strlen(DOT_S) + strlen(BRACKETS_S) + 1));
	sprintf(ILLEGAL_IN_NAMES_MINUS_SPACE_STR, "%s%s%s%s\t\n", RESERVED_S, OPERATORS_S, DOT_S, BRACKETS_S);	
	ILLEGAL_IN_UNITNAMES = (char*) malloc(sizeof(char) * (strlen(ILLEGAL_IN_NAMES) + strlen(NUMBERS_S) + 1));
	sprintf(ILLEGAL_IN_UNITNAMES, "%s%s", ILLEGAL_IN_NAMES, NUMBERS_S);			

	b_rpn = false;
	calculator = this;
	srand48(time(0));
	angleMode(RADIANS);
	addBuiltinVariables();
	addBuiltinFunctions();
	addBuiltinUnits();
	b_functions = true;
	b_variables = true;
	b_units = true;
	b_unknown = true;
	b_calcvars = true;
	b_always_exact = false;
	disable_errors_ref = 0;
	
}
Calculator::~Calculator() {}

void Calculator::setRPNMode(bool enable) {
	b_rpn = enable;
}
bool Calculator::inRPNMode() const {
	return b_rpn;
}
bool Calculator::alwaysExact() const {
	return b_always_exact;
}
void Calculator::setAlwaysExact(bool always_exact) {
	b_always_exact = always_exact;
}
void Calculator::beginTemporaryStopErrors() {
	disable_errors_ref++;
}
void Calculator::endTemporaryStopErrors() {
	disable_errors_ref--;
}
Variable *Calculator::getVariable(int index) const {
	if(index >= 0 && index < variables.size()) {
		return variables[index];
	}
	return NULL;
}
ExpressionItem *Calculator::getActiveExpressionItem(ExpressionItem *item) {
	string name;
	int nindex = 1;
	while(true) {
		switch(item->type()) {
			case TYPE_UNIT: {
				if(!((Unit*) item)->unitType() == COMPOSITE_UNIT) {
					if(nindex == 1) {
						name = item->name();
					}
					if(nindex == 2) {
						name = ((Unit*) item)->plural(false);
						if(name.empty()) {
							nindex++;
						}
					}
					if(nindex == 3) {
						name = ((Unit*) item)->shortName(false);
						if(name.empty()) {
							nindex++;
						}
					}					
					if(nindex > 3) {
						return NULL;
					}
					break;
				}
			}
			default: {
				if(nindex > 1) {
					return NULL;
				}
				name = item->referenceName();
			}
		}
		ExpressionItem *item2 = getActiveExpressionItem(name, item);
		if(item2) {
			return item2;
		}
		nindex++;	
	}
	return NULL;
}
ExpressionItem *Calculator::getActiveExpressionItem(string name, ExpressionItem *item) {
	for(int index = 0; index < variables.size(); index++) {
		if(variables[index] != item && variables[index]->isActive() && variables[index]->referenceName() == name) {
			return variables[index];
		}
	}
	for(int index = 0; index < functions.size(); index++) {
		if(functions[index] != item && functions[index]->isActive() && functions[index]->referenceName() == name) {
			return functions[index];
		}
	}
	for(int i = 0; i < units.size(); i++) {
		if(units[i] != item && units[i]->isActive()) {
			if(units[i]->unitType() == COMPOSITE_UNIT) {
				if(name == units[i]->referenceName()) {
					return units[i];
				}
			} else {
				if(units[i]->name() == name || units[i]->shortName(false) == name || units[i]->plural(false) == name) {
					return units[i];
				}
			}
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getInactiveExpressionItem(string name, ExpressionItem *item) {
	for(int index = 0; index < variables.size(); index++) {
		if(variables[index] != item && !variables[index]->isActive() && variables[index]->referenceName() == name) {
			return variables[index];
		}
	}
	for(int index = 0; index < functions.size(); index++) {
		if(functions[index] != item && !functions[index]->isActive() && functions[index]->referenceName() == name) {
			return functions[index];
		}
	}
	for(int i = 0; i < units.size(); i++) {
		if(units[i] != item && !units[i]->isActive()) {
			if(units[i]->unitType() == COMPOSITE_UNIT) {
				if(name == units[i]->referenceName()) {
					return units[i];
				}
			} else {
				if(units[i]->name() == name || units[i]->shortName(false) == name || units[i]->plural(false) == name) {
					return units[i];
				}
			}
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getExpressionItem(string name, ExpressionItem *item) {
	Variable *v = getVariable(name);
	if(v && v != item) return v;
	Function *f = getFunction(name);
	if(f && f != item) return f;
	Unit *u = getUnit(name);
	if(u && u != item) return u;
	u = getCompositeUnit(name);
	if(u && u != item) return u;
	return NULL;
}
Unit *Calculator::getUnit(int index) const {
	if(index >= 0 && index < units.size()) {
		return units[index];
	}
	return NULL;
}
Function *Calculator::getFunction(int index) const {
	if(index >= 0 && index < functions.size()) {
		return functions[index];
	}
	return NULL;
}
Prefix *Calculator::getPrefix(int index) const {
	if(index >= 0 && index < prefixes.size()) {
		return prefixes[index];
	}
	return NULL;
}
Prefix *Calculator::getPrefix(string name_) const {
	for(int i = 0; i < prefixes.size(); i++) {
		if(prefixes[i]->shortName(false) == name_ || prefixes[i]->longName(false) == name_) {
			return prefixes[i];
		}
	}
	return NULL;
}
Prefix *Calculator::getExactPrefix(long int exp10, long int exp) const {
	for(int i = 0; i < prefixes.size(); i++) {
		if(prefixes[i]->exponent(exp) == exp10) {
			return prefixes[i];
		} else if(prefixes[i]->exponent(exp) > exp10) {
			break;
		}
	}
	return NULL;
}
Prefix *Calculator::getExactPrefix(const Fraction *fr, long int exp) const {
	Fraction tmp_exp;
	for(int i = 0; i < prefixes.size(); i++) {
		if(fr->equals(prefixes[i]->value(exp, &tmp_exp))) {
			return prefixes[i];
		} else if(fr->isLessThan(prefixes[i]->value(exp, &tmp_exp))) {
			break;
		}
	}
	return NULL;
}
Prefix *Calculator::getNearestPrefix(long int exp10, long int exp) const {
	if(prefixes.size() <= 0) return NULL;
	for(int i = 0; i < prefixes.size(); i++) {
		if(prefixes[i]->exponent(exp) == exp10) {
			return prefixes[i];
		} else if(prefixes[i]->exponent(exp) > exp10) {
			if(i == 0) {
				return prefixes[i];
			} else if(exp10 - prefixes[i - 1]->exponent(exp) < prefixes[i]->exponent(exp) - exp10) {
				return prefixes[i - 1];
			} else {
				return prefixes[i];
			}
		}
	}
	return prefixes[prefixes.size() - 1];
}
Prefix *Calculator::getBestPrefix(long int exp10, long int exp) const {
	if(prefixes.size() <= 0) return NULL;
	for(int i = 0; i < prefixes.size(); i++) {
		if(prefixes[i]->exponent(exp) == exp10) {
			return prefixes[i];
		} else if(prefixes[i]->exponent(exp) > exp10) {
			if(i == 0) {
				return prefixes[i];
			} else if(exp10 - prefixes[i - 1]->exponent(exp) < (prefixes[i]->exponent(exp) - exp10) * 2 + 2) {
				return prefixes[i - 1];
			} else {
				return prefixes[i];
			}
		}
	}
	return prefixes[prefixes.size() - 1];	
}
Prefix *Calculator::getBestPrefix(const Integer *exp10, const Integer *exp) const {
	if(prefixes.size() <= 0) return NULL;
	Integer tmp_exp;
	for(int i = 0; i < prefixes.size(); i++) {
		if(exp10->equals(prefixes[i]->exponent(exp, &tmp_exp))) {
			return prefixes[i];
		} else if(exp10->compare(prefixes[i]->exponent(exp, &tmp_exp)) == 1) {
			if(i == 0) {
				return prefixes[i];
			}
			Integer exp10_1(exp10);
			exp10_1.subtract(prefixes[i - 1]->exponent(exp, &tmp_exp));
			Integer exp10_2(prefixes[i]->exponent(exp, &tmp_exp));
			exp10_2.subtract(exp10);
			exp10_2.multiply(2);
			exp10_2.add(2);
			if(exp10_1.isLessThan(&exp10_2)) {
				return prefixes[i - 1];
			} else {
				return prefixes[i];
			}
		}
	}
	return prefixes[prefixes.size() - 1];	
}
Prefix *Calculator::addPrefix(Prefix *p) {
	prefixes.push_back(p);
	prefixNameChanged(p);
	return p;	
}
void Calculator::prefixNameChanged(Prefix *p) {
	int l, i = 0;
	delUFV((void*) p);
	if(!p->longName(false).empty()) {
		for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
			l = 0;
			if(it != ufv.end()) {
				if(ufv_t[i] == 'v')
					l = ((Variable*) (*it))->name().length();
				else if(ufv_t[i] == 'f')
					l = ((Function*) (*it))->name().length();
				else if(ufv_t[i] == 'U')
					l = ((Unit*) (*it))->name().length();
				else if(ufv_t[i] == 'Y')
					l = ((Unit*) (*it))->plural(false).length();
				else if(ufv_t[i] == 'u')
					l = ((Unit*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) p);
				ufv_t.push_back('P');
				break;
			} else if(l < p->longName(false).length() || (l == p->longName(false).length() && ufv_t[i] != 'u' && ufv_t[i] != 'U' && ufv_t[i] != 'Y')) {
				ufv.insert(it, (void*) p);
				ufv_t.insert(ufv_t.begin() + i, 'P');
				break;
			}
			i++;
		}
	}
	i = 0;
	if(!p->shortName(false).empty()) {
		for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
			l = 0;
			if(it != ufv.end()) {
				if(ufv_t[i] == 'v')
					l = ((Variable*) (*it))->name().length();
				else if(ufv_t[i] == 'f')
					l = ((Function*) (*it))->name().length();
				else if(ufv_t[i] == 'U')
					l = ((Unit*) (*it))->name().length();
				else if(ufv_t[i] == 'Y')
					l = ((Unit*) (*it))->plural(false).length();
				else if(ufv_t[i] == 'u')
					l = ((Unit*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) p);
				ufv_t.push_back('p');
				break;
			} else if(l < p->shortName(false).length() || (l == p->shortName(false).length() && ufv_t[i] != 'u' && ufv_t[i] != 'U' && ufv_t[i] != 'Y')) {
				ufv.insert(it, (void*) p);
				ufv_t.insert(ufv_t.begin() + i, 'p');
				break;
			}
			i++;
		}
	}
}

void Calculator::setPrecision(int precision) {
	if(precision <= 0) precision = DEFAULT_PRECISION;
#ifdef HAVE_LIBCLN
	cln::default_float_format = float_format(precision + 5);	
#endif
	i_precision = precision;
}
int Calculator::getPrecision() const {
	return i_precision;
}

const char *Calculator::getDecimalPoint() const {return DOT_STR;}
const char *Calculator::getComma() const {return COMMA_STR;}	
void Calculator::setLocale() {
	setlocale(LC_NUMERIC, saved_locale);
	lconv *locale = localeconv();
	if(strcmp(locale->decimal_point, ",") == 0) {
		DOT_STR = ",";
		DOT_S = ".,";	
		COMMA_STR = ";";
		COMMA_S = ";";		
		addStringAlternative(",", DOT);			
	} else {
		DOT_STR = ".";	
		DOT_S = ".";	
		COMMA_STR = ",";
		COMMA_S = ",;";			
	}
	setlocale(LC_NUMERIC, "C");
}
void Calculator::unsetLocale() {
	COMMA_STR = ",";
	COMMA_S = ",;";	
	DOT_STR = ".";
	DOT_S = ".";
}

int Calculator::addId(Manager *mngr, bool persistent) {
	for(int i = 0; ; i++) {
		if(!ids.count(i)) {
			ids[i] = mngr;
			ids_p[i] = persistent;
			mngr->ref();
			return i;
		}
	}
	return 0;
}
Manager *Calculator::getId(int id) {
	if(ids.count(id)) {
		return ids[id];
	} 
	return NULL;
}
void Calculator::delId(int id, bool force) {
	if(ids.count(id) && (!ids_p[id] || force)) {
		ids[id]->unref();
		ids.erase(id);
		ids_p.erase(id);		
	}
}
bool Calculator::functionsEnabled() {
	return b_functions;
}
void Calculator::setFunctionsEnabled(bool enable) {
	b_functions = enable;
}
bool Calculator::variablesEnabled() {
	return b_variables;
}
void Calculator::setVariablesEnabled(bool enable) {
	b_variables = enable;
}
bool Calculator::unknownVariablesEnabled() {
	return b_unknown;
}
void Calculator::setUnknownVariablesEnabled(bool enable) {
	b_unknown = enable;
}
bool Calculator::donotCalculateVariables() {
	return !b_calcvars;
}
void Calculator::setDonotCalculateVariables(bool enable) {
	b_calcvars = !enable;
}
bool Calculator::unitsEnabled() {
	return b_units;
}
void Calculator::setUnitsEnabled(bool enable) {
	b_units = enable;
}
int Calculator::angleMode() {
	return ianglemode;
}
void Calculator::angleMode(int mode_) {
	if(mode_ >= RADIANS && mode_ <= GRADIANS)
		ianglemode = mode_;
}
void Calculator::resetVariables() {
	variables.clear();
	addBuiltinVariables();
}
void Calculator::resetFunctions() {
	functions.clear();
	addBuiltinFunctions();
}
void Calculator::resetUnits() {
	units.clear();
	addBuiltinUnits();
}
void Calculator::reset() {
	resetVariables();
	resetFunctions();
	resetUnits();
}
void Calculator::addBuiltinVariables() {
}
void Calculator::addBuiltinFunctions() {
	addFunction(new EFunction());
	addFunction(new PiFunction());	
	addFunction(new EulerFunction());
	addFunction(new AperyFunction());	
	addFunction(new CatalanFunction());
	addFunction(new PythagorasFunction());
	addFunction(new GoldenFunction());
#ifdef HAVE_LIBCLN
	addFunction(new ZetaFunction());
#endif	
	addFunction(new ProcessFunction());
	addFunction(new CustomSumFunction());
	addFunction(new FunctionFunction());
	addFunction(new MatrixFunction());
	addFunction(new VectorFunction());	
	addFunction(new ElementsFunction());	
	addFunction(new SortFunction());	
	addFunction(new RankFunction());
	addFunction(new MatrixToVectorFunction());	
	addFunction(new RowFunction());
	addFunction(new ColumnFunction());	
	addFunction(new RowsFunction());
	addFunction(new ColumnsFunction());
	addFunction(new ElementFunction());
	addFunction(new ComponentsFunction());	
	addFunction(new ComponentFunction());	
	addFunction(new LimitsFunction());	
	addFunction(new TransposeFunction());
	addFunction(new IdentityFunction());
	addFunction(new DeterminantFunction());
	addFunction(new CofactorFunction());
	addFunction(new AdjointFunction());
	addFunction(new InverseFunction());
	addFunction(new IFFunction());
	addFunction(new DifferentiateFunction());	
	addFunction(new DaysFunction());		
	addFunction(new DaysBetweenDatesFunction());		
	addFunction(new YearsBetweenDatesFunction());		
	addFunction(new GCDFunction());	
	addFunction(new FactorialFunction());		
	addFunction(new AbsFunction());
	addFunction(new CeilFunction());
	addFunction(new FloorFunction());
	addFunction(new TruncFunction());
	addFunction(new RoundFunction());
	addFunction(new FracFunction());
	addFunction(new IntFunction());	
	addFunction(new ModFunction());
	addFunction(new RemFunction());
	addFunction(new SinFunction());
	addFunction(new CosFunction());
	addFunction(new TanFunction());
	addFunction(new SinhFunction());
	addFunction(new CoshFunction());
	addFunction(new TanhFunction());
	addFunction(new AsinFunction());
	addFunction(new AcosFunction());
	addFunction(new AtanFunction());
	addFunction(new AsinhFunction());
	addFunction(new AcoshFunction());
	addFunction(new AtanhFunction());
	addFunction(new LogFunction());
	addFunction(new Log2Function());
	addFunction(new Log10Function());
	addFunction(new ExpFunction());
	addFunction(new Exp2Function());
	addFunction(new Exp10Function());
	addFunction(new SqrtFunction());
	addFunction(new CbrtFunction());
	addFunction(new RootFunction());	
	addFunction(new PowFunction());	
	addFunction(new HypotFunction());
	addFunction(new PercentileFunction());
	addFunction(new MinFunction());
	addFunction(new MaxFunction());
	addFunction(new ModeFunction());
	addFunction(new RandomFunction());
	addFunction(new BASEFunction());
	addFunction(new BINFunction());
	addFunction(new OCTFunction());
	addFunction(new HEXFunction());
	addFunction(new TitleFunction());
}
void Calculator::addBuiltinUnits() {
}
void Calculator::error(bool critical, const char *TEMPLATE, ...) {
	if(disable_errors_ref) return;
	string error_str = TEMPLATE;
	va_list ap;
	va_start(ap, TEMPLATE);
	const char *str;
	while(true) {
		int i = error_str.find("%s");
		if(i == string::npos) break;	
		str = va_arg(ap, const char*);
		if(!str) break;
		error_str.replace(i, 2, str);
	}
	va_end(ap);
	errors.push(new Error(error_str, critical));
}
Error* Calculator::error() {
	if(!errors.empty()) {
		return errors.top();
	}
	return NULL;
}
Error* Calculator::nextError() {
	if(!errors.empty()) {
		errors.pop();
		if(!errors.empty()) {
			return errors.top();
		}
	}
	return NULL;
}
void Calculator::deleteName(string name_, ExpressionItem *object) {
	Variable *v2 = getVariable(name_);
	if(v2 == object) {
		return;
	}
	if(v2 != NULL) {
		v2->destroy();
	} else {
		Function *f2 = getFunction(name_);
		if(f2 == object)
			return;
		if(f2 != NULL) {
			f2->destroy();
		}
	}
}
void Calculator::deleteUnitName(string name_, Unit *object) {
	Unit *u2 = getUnit(name_);
	if(u2) {
		if(u2 != object) {
			u2->destroy();
		}
		return;
	} 
	u2 = getCompositeUnit(name_);	
	if(u2) {
		if(u2 != object) {
			u2->destroy();
		}
	}
}
Manager *Calculator::calculate(string str) {
	int i = 0; 
	string str2 = "";
	if(unitsEnabled() && (i = str.find(_(" to "))) != string::npos) {
		int l = strlen(_(" to "));
		str2 = str.substr(i + l, str.length() - i - l);		
		remove_blank_ends(str2);
		if(!str2.empty()) {
			str = str.substr(0, i);
		}
	}
	setFunctionsAndVariables(str);
	EqContainer *e = new EqContainer(str, ADD);
	Manager *mngr = e->calculate();
	mngr->finalize();
	if(!str2.empty()) {
		convert(mngr, str2);
	}
	mngr->ref();
/*	int vtype = fpclassify(value);
	if(vtype == FP_NAN)
		error(true, _("Math error: not a number"), NULL);
	else if(vtype == FP_INFINITE)
		error(true, _("Math error: infinite"), NULL);*/	
	checkFPExceptions();
	delete e;
	return mngr;
}
void Calculator::checkFPExceptions() {
	int raised = fetestexcept(FE_ALL_EXCEPT);
#ifdef FE_INEXACT
	//		if(raised & FE_INEXACT) error(true, _("Floating point error: inexact rounded result"), NULL);
#endif
#ifdef FE_DIVBYZERO

	if(raised & FE_DIVBYZERO)
		error(true, _("Floating point error: division by zero."), NULL);
#endif
#ifdef FE_UNDERFLOW
	//		if(raised & FE_UNDERFLOW) error(true, _("Floating point error: underflow"), NULL);
#endif
#ifdef FE_OVERFLOW
	//		if(raised & FE_OVERFLOW) error(true, _("Floating point error: overflow"), NULL);
#endif
#ifdef FE_INVALID

	if(raised & FE_INVALID)
		error(true, _("Floating point error: invalid operation."), NULL);
#endif

	feclearexcept(FE_ALL_EXCEPT);
}
void Calculator::checkFPExceptions(const char *str) {
	int raised = fetestexcept(FE_ALL_EXCEPT);
#ifdef FE_INEXACT
	//		if(raised & FE_INEXACT) error(true, "Floating point error: inexact rounded result in \"%s\"", str, NULL);
#endif
#ifdef FE_DIVBYZERO

	if(raised & FE_DIVBYZERO)
		error(true, _("Floating point error: division by zero in \"%s\"."), str, NULL);
#endif
#ifdef FE_UNDERFLOW
	//		if(raised & FE_UNDERFLOW) error(true, _("Floating point error: underflow in \"%s\"."), str, NULL);
#endif
#ifdef FE_OVERFLOW
	//		if(raised & FE_OVERFLOW) error(true, _("Floating point error: overflow in \"%s\"."), str, NULL);
#endif
#ifdef FE_INVALID

	if(raised & FE_INVALID)
		error(true, _("Floating point error: invalid operation in \"%s\n."), str, NULL);
#endif

	feclearexcept(FE_ALL_EXCEPT);
}

Manager *Calculator::convert(long double value, Unit *from_unit, Unit *to_unit) {
	string str = d2s(value);
	return convert(str, from_unit, to_unit);
}
Manager *Calculator::convert(string str, Unit *from_unit, Unit *to_unit) {
	Manager *mngr = calculate(str);
	mngr->addUnit(from_unit, MULTIPLY);
	from_unit->hasComplexRelationTo(to_unit);
	mngr->convert(to_unit);
	mngr->finalize();	
//	mngr->convert(to_unit);
	mngr->addUnit(to_unit, DIVIDE);
	mngr->finalize();	
	return mngr;
}
Manager *Calculator::convert(Manager *mngr, Unit *to_unit, bool always_convert) {
	if(to_unit->unitType() == COMPOSITE_UNIT) return convertToCompositeUnit(mngr, (CompositeUnit*) to_unit, always_convert);
	if(to_unit->unitType() != ALIAS_UNIT || (((AliasUnit*) to_unit)->baseUnit()->unitType() != COMPOSITE_UNIT && ((AliasUnit*) to_unit)->baseExp() == 1.0L)) {
		Manager *mngr_old = new Manager(mngr);
		mngr->finalize();
		if(!mngr->convert(to_unit)) {
			mngr->moveto(mngr_old);
		} else {
			mngr->finalize();
			mngr_old->unref();
			return mngr;
		}
	}
	mngr->finalize();
	if(mngr->type() == ADDITION_MANAGER) {
		for(int i = 0; i < mngr->mngrs.size(); i++) {
			convert(mngr->mngrs[i], to_unit, false);
		}
		mngr->sort();
	} else {
		bool b = false;
		if(mngr->convert(to_unit) || always_convert) {	
			b = true;
		} else if(to_unit->unitType() == ALIAS_UNIT && ((AliasUnit*) to_unit)->baseUnit()->unitType() == COMPOSITE_UNIT) {
			CompositeUnit *cu = (CompositeUnit*) ((AliasUnit*) to_unit)->baseUnit();
			switch(mngr->type()) {
				case UNIT_MANAGER: {
					if(cu->containsRelativeTo(mngr->o_unit)) {
						b = true;
					}
					break;
				} 
				case MULTIPLICATION_MANAGER: {
					for(int i = 0; i < mngr->mngrs.size(); i++) {
						if(mngr->mngrs[i]->type() == UNIT_MANAGER && cu->containsRelativeTo(mngr->mngrs[i]->o_unit)) {
							b = true;
						}
						if(mngr->mngrs[i]->type() == POWER_MANAGER && mngr->mngrs[i]->mngrs[0]->type() == UNIT_MANAGER && cu->containsRelativeTo(mngr->mngrs[i]->mngrs[0]->o_unit)) {
							b = true;
						}
					}
					break;
				}
				case POWER_MANAGER: {
					if(mngr->mngrs[0]->type() == UNIT_MANAGER && cu->containsRelativeTo(mngr->mngrs[0]->o_unit)) {
						b = true;
					}
					break;				
				}
			}
		}
		if(b) {			
			mngr->addUnit(to_unit, DIVIDE);
			mngr->finalize();			
			Manager *mngr2 = new Manager(to_unit);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				mngr->mngrs.push_back(mngr2);
			} else {
				mngr->transform(mngr2, MULTIPLICATION_MANAGER, MULTIPLY);
				mngr2->unref();
			}
			mngr->sort();
		}
	}
	return mngr;
}
Manager *Calculator::convertToCompositeUnit(Manager *mngr, CompositeUnit *cu, bool always_convert) {
	mngr->finalize();
	Manager *mngr3 = cu->generateManager(true);
	if(mngr->type() == ADDITION_MANAGER) {
		for(int i = 0; i < mngr->mngrs.size(); i++) {
			convertToCompositeUnit(mngr->mngrs[i], cu, false);
		}
		mngr->sort();
	} else {
		bool b = false;
		if(mngr->convert(cu) || always_convert) {	
			b = true;
		} else {
			switch(mngr->type()) {
				case UNIT_MANAGER: {
					if(cu->containsRelativeTo(mngr->o_unit)) {
						b = true;
					}
					break;
				} 
				case MULTIPLICATION_MANAGER: {
					for(int i = 0; i < mngr->mngrs.size(); i++) {
						if(mngr->mngrs[i]->type() == UNIT_MANAGER && cu->containsRelativeTo(mngr->mngrs[i]->o_unit)) {
							b = true;
						}
						if(mngr->mngrs[i]->type() == POWER_MANAGER && mngr->mngrs[i]->mngrs[0]->type() == UNIT_MANAGER && cu->containsRelativeTo(mngr->mngrs[i]->mngrs[0]->o_unit)) {
							b = true;
						}
					}
					break;
				}
				case POWER_MANAGER: {
					if(mngr->mngrs[0]->type() == UNIT_MANAGER && cu->containsRelativeTo(mngr->mngrs[0]->o_unit)) {
						b = true;
					}
					break;				
				}
			}
		}
		if(b) {	
			mngr->add(mngr3, DIVIDE);
			mngr->finalize();			
			Manager *mngr2 = new Manager(cu);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				mngr->mngrs.push_back(mngr2);
			} else {
				mngr->transform(mngr2, MULTIPLICATION_MANAGER, MULTIPLY);
				mngr2->unref();		
			}
			mngr->sort();
		}
	}
	mngr3->unref();
	return mngr;
}
Manager *Calculator::convert(Manager *mngr, string composite_) {
	remove_blank_ends(composite_);
	if(composite_.empty()) return mngr;
	Unit *u = getUnit(composite_);
	if(u) return convert(mngr, u);
	CompositeUnit *cu = new CompositeUnit("", "temporary_composite_convert", "", composite_);
	convertToCompositeUnit(mngr, cu);
	return mngr;			
}
Unit* Calculator::addUnit(Unit *u, bool force) {
	if(u->unitType() == COMPOSITE_UNIT) {
		u->setName(getUnitName(u->referenceName(), u, force));		
	} else {
		u->setName(getUnitName(u->name(), u, force));
		if(!u->plural(false).empty()) {
			u->setPlural(getUnitName(u->plural(false), u, force));
		}
		if(!u->shortName(false).empty()) {
			u->setShortName(getUnitName(u->shortName(false), u, force));
		}
	}
	if(!u->isLocal() && units.size() > 0 && units[units.size() - 1]->isLocal()) {
		units.insert(units.begin(), u);
	} else {	
		units.push_back(u);
	}
	u->setChanged(false);
	return u;
}
void Calculator::delUFV(void *object) {
	int i = 0;
	bool halt = false;
	for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
		if(halt) {
			--it;
			halt = false;
		}
		if(it == ufv.end())
			break;
		if(*it == object) {
			if(it == ufv.begin()) {
				ufv.erase(it);
				ufv_t.erase(ufv_t.begin() + i);
				it = ufv.begin();
				halt = true;
				i = -1;
			} else {
				vector<void*>::iterator it_tmp = it;
				--it_tmp;
				ufv.erase(it);
				ufv_t.erase(ufv_t.begin() + i);
				it = it_tmp;
				i--;
			}
		}
		i++;
	}
}
Unit* Calculator::getUnit(string name_) {
	for(int i = 0; i < (int) units.size(); i++) {
		if(units[i]->unitType() != COMPOSITE_UNIT && (units[i]->name() == name_ || units[i]->shortName(false) == name_ || units[i]->plural(false) == name_)) {
			return units[i];
		}
	}
	return NULL;
}
Unit* Calculator::getCompositeUnit(string internal_name_) {
	for(int i = 0; i < (int) units.size(); i++) {
		if(units[i]->unitType() == COMPOSITE_UNIT && units[i]->referenceName() == internal_name_) {
			return units[i];
		}
	}
	return NULL;
}

Variable* Calculator::addVariable(Variable *v, bool force) {
	if(!v->isLocal() && variables.size() > 0 && variables[variables.size() - 1]->isLocal()) {
		variables.insert(variables.begin(), v);
	} else {
		variables.push_back(v);
	}
	v->setName(getName(v->name(), v, force));
	v->setChanged(false);
	return v;
}
void Calculator::expressionItemDeactivated(ExpressionItem *item) {
	delUFV((void*) item);
}
void Calculator::expressionItemActivated(ExpressionItem *item) {
	ExpressionItem *item2 = getActiveExpressionItem(item);
	if(item2) {
		item2->setActive(false);
	}
	nameChanged(item);
}
void Calculator::expressionItemDeleted(ExpressionItem *item) {
	switch(item->type()) {
		case TYPE_VARIABLE: {
			for(vector<Variable*>::iterator it = variables.begin(); it != variables.end(); ++it) {
				if(*it == item) {
					variables.erase(it);
					break;
				}
			}
			break;
		}
		case TYPE_FUNCTION: {
			for(vector<Function*>::iterator it = functions.begin(); it != functions.end(); ++it) {
				if(*it == item) {
					functions.erase(it);
					break;
				}
			}
			break;
		}		
		case TYPE_UNIT: {
			for(vector<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
				if(*it == item) {
					units.erase(it);
					break;
				}
			}		
			break;
		}		
	}
	delUFV((void*) item);
}
void Calculator::nameChanged(ExpressionItem *item) {
	switch(item->type()) {
		case TYPE_VARIABLE: {
			variableNameChanged((Variable*) item);
			break;
		}
		case TYPE_FUNCTION: {
			functionNameChanged((Function*) item);
			break;
		}		
		case TYPE_UNIT: {
			unitNameChanged((Unit*) item);
			break;
		}		
	}
}
void Calculator::variableNameChanged(Variable *v) {
	if(!v->isActive()) return;
	int l, i = 0;
	delUFV((void*) v);
	for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
		l = 0;
		if(it != ufv.end()) {
			if(ufv_t[i] == 'v')
				l = ((Variable*) (*it))->name().length();
			else if(ufv_t[i] == 'f')
				l = ((Function*) (*it))->name().length();
			else if(ufv_t[i] == 'U')
				l = ((Unit*) (*it))->name().length();
			else if(ufv_t[i] == 'Y')
				l = ((Unit*) (*it))->plural(false).length();
			else if(ufv_t[i] == 'u')
				l = ((Unit*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'p')
				l = ((Prefix*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'P')
				l = ((Prefix*) (*it))->longName(false).length();				
		}
		if(it == ufv.end()) {
			ufv.push_back((void*) v);
			ufv_t.push_back('v');
			break;
		} else if(l < v->name().length() || (l == v->name().length() && ufv_t[i] == 'v')) {
			ufv.insert(it, (void*) v);
			ufv_t.insert(ufv_t.begin() + i, 'v');
			break;
		}
		i++;
	}
}
void Calculator::functionNameChanged(Function *f, bool priviliged) {
	if(!f->isActive()) return;
	int l, i = 0;
	delUFV((void*) f);
	if(priviliged) {
		ufv.insert(ufv.begin(), (void*) f);
		ufv_t.insert(ufv_t.begin(), 'f');
		return;
	}
	for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
		l = 0;
		if(it != ufv.end()) {
			if(ufv_t[i] == 'v')
				l = ((Variable*) (*it))->name().length();
			else if(ufv_t[i] == 'f')
				l = ((Function*) (*it))->name().length();
			else if(ufv_t[i] == 'U')
				l = ((Unit*) (*it))->name().length();
			else if(ufv_t[i] == 'Y')
				l = ((Unit*) (*it))->plural(false).length();
			else if(ufv_t[i] == 'u')
				l = ((Unit*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'p')
				l = ((Prefix*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'P')
				l = ((Prefix*) (*it))->longName(false).length();				
		}
		if(it == ufv.end()) {
			ufv.push_back((void*) f);
			ufv_t.push_back('f');
			break;
		} else if(l < f->name().length() || (l == f->name().length() && (ufv_t[i] == 'v' || ufv_t[i] == 'f'))) {
			ufv.insert(it, (void*) f);
			ufv_t.insert(ufv_t.begin() + i, 'f');
			break;
		}
		i++;
	}
}
void Calculator::unitNameChanged(Unit *u) {
	if(!u->isActive()) return;
	if(u->unitType() == COMPOSITE_UNIT) {
		return;
	}
	int l, i = 0;
	delUFV((void*) u);
	for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
		l = 0;
		if(it != ufv.end()) {
			if(ufv_t[i] == 'v')
				l = ((Variable*) (*it))->name().length();
			else if(ufv_t[i] == 'f')
				l = ((Function*) (*it))->name().length();
			else if(ufv_t[i] == 'U')
				l = ((Unit*) (*it))->name().length();
			else if(ufv_t[i] == 'Y')
				l = ((Unit*) (*it))->plural(false).length();
			else if(ufv_t[i] == 'u')
				l = ((Unit*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'p')
				l = ((Prefix*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'P')
				l = ((Prefix*) (*it))->longName(false).length();
		}
		if(it == ufv.end()) {
			ufv.push_back((void*) u);
			ufv_t.push_back('U');
			break;
		} else if(l <= u->name().length()) {
			ufv.insert(it, (void*) u);
			ufv_t.insert(ufv_t.begin() + i, 'U');
			break;
		}
		i++;
	}
	if(!u->plural(false).empty()) {
		i = 0;
		for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
			l = 0;
			if(it != ufv.end()) {
				if(ufv_t[i] == 'v')
					l = ((Variable*) (*it))->name().length();
				else if(ufv_t[i] == 'f')
					l = ((Function*) (*it))->name().length();
				else if(ufv_t[i] == 'U')
					l = ((Unit*) (*it))->name().length();
				else if(ufv_t[i] == 'Y')
					l = ((Unit*) (*it))->plural(false).length();
				else if(ufv_t[i] == 'u')
					l = ((Unit*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('Y');
				break;
			} else if(l <= u->plural(false).length()) {
				ufv.insert(it, (void*) u);
				ufv_t.insert(ufv_t.begin() + i, 'Y');
				break;
			}
			i++;
		}
	}
	if(!u->shortName(false).empty()) {
		i = 0;
		for(vector<void*>::iterator it = ufv.begin(); ; ++it) {
			l = 0;
			if(it != ufv.end()) {
				if(ufv_t[i] == 'v')
					l = ((Variable*) (*it))->name().length();
				else if(ufv_t[i] == 'f')
					l = ((Function*) (*it))->name().length();
				else if(ufv_t[i] == 'U')
					l = ((Unit*) (*it))->name().length();
				else if(ufv_t[i] == 'Y')
					l = ((Unit*) (*it))->plural(false).length();
				else if(ufv_t[i] == 'u')
					l = ((Unit*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('u');
				break;
			} else if(l <= u->shortName(false).length()) {
				ufv.insert(it, (void*) u);
				ufv_t.insert(ufv_t.begin() + i, 'u');
				break;
			}
			i++;
		}
	}

}
void Calculator::unitShortNameChanged(Unit *u) {
	unitNameChanged(u);
}
void Calculator::unitPluralChanged(Unit *u) {
	unitNameChanged(u);
}

Variable* Calculator::getVariable(string name_) {
	for(int i = 0; i < (int) variables.size(); i++) {
		if(variables[i]->name() == name_) {
			return variables[i];
		}
	}
	return NULL;
}
ExpressionItem* Calculator::addExpressionItem(ExpressionItem *item, bool force) {
	switch(item->type()) {
		case TYPE_VARIABLE: {
			return addVariable((Variable*) item, force);
		}
		case TYPE_FUNCTION: {
			return addFunction((Function*) item, force);
		}		
		case TYPE_UNIT: {
			return addUnit((Unit*) item, force);
		}		
	}
	return NULL;
}
Function* Calculator::addFunction(Function *f, bool force) {
	f->setName(getName(f->name(), f, force));
	if(!f->isLocal() && functions.size() > 0 && functions[functions.size() - 1]->isLocal()) {
		functions.insert(functions.begin(), f);
	} else {
		functions.push_back(f);
	}
	f->setChanged(false);
	return f;
}
Function* Calculator::getFunction(string name_) {
	for(int i = 0; i < (int) functions.size(); i++) {
		if(functions[i]->name() == name_) {
			return functions[i];
		}
	}
	return NULL;
}
bool Calculator::variableNameIsValid(string name_) {
	return name_.find_first_of(ILLEGAL_IN_NAMES) == string::npos && !is_in(NUMBERS_S, name_[0]);
}
bool Calculator::functionNameIsValid(string name_) {
	return variableNameIsValid(name_);
}
bool Calculator::unitNameIsValid(string name_) {
	return name_.find_first_of(ILLEGAL_IN_UNITNAMES) == string::npos;
}
string Calculator::convertToValidVariableName(string name_) {
	int i = 0;
	while(1) {
		i = name_.find_first_of(ILLEGAL_IN_NAMES_MINUS_SPACE_STR, i);
		if(i == string::npos)
			break;
		name_.erase(name_.begin() + i);
	}
	gsub(SPACE_STR, UNDERSCORE_STR, name_);
	while(is_in(NUMBERS_S, name_[0])) {
		name_.erase(name_.begin());
	}
	return name_;
}
string Calculator::convertToValidFunctionName(string name_) {
	return convertToValidVariableName(name_);
}
string Calculator::convertToValidUnitName(string name_) {
	int i = 0;
	while(1) {
		i = find_first_of(name_, i, ILLEGAL_IN_NAMES_MINUS_SPACE_STR, NUMBERS_S, NULL);
		if(i == string::npos)
			break;
		name_.erase(name_.begin() + i);
	}
	gsub(SPACE_STR, UNDERSCORE_STR, name_);
	return name_;
}
bool Calculator::nameTaken(string name_, ExpressionItem *object) {
	ExpressionItem *item = getExpressionItem(name_, object);
	if(item) {
		return true;
	}
	return false;
}
bool Calculator::unitIsUsedByOtherUnits(const Unit *u) const {
	const Unit *u2;
	for(int i = 0; i < units.size(); i++) {
		if(units[i] != u) {
			u2 = units[i];
			while(u2->unitType() == ALIAS_UNIT) {
				u2 = ((AliasUnit*) u2)->firstBaseUnit();
				if(u2 == u) {
					return true;
				}
			}
		}
	}
	return false;
}
void Calculator::setFunctionsAndVariables(string &str) {
	int i = 0, i3 = 0, i4, i5, i6, i7, i8, i9;
	bool b;
	Variable *v;
	Function *f;
	Prefix *p;
	Unit *u;
	string ch;
	vector<int> uss;
	vector<int> ues;
	vector<char> ut;
	string stmp, stmp2;
	long int value;
	for(int i = 0; i < signs.size(); i++) gsub(signs[i], real_signs[i], str);
	Manager *mngr;
	b = false;
	while(!b) {
		i = str.find("\"", i);
		if(i == string::npos) break;
		i3 = str.find("\"", i + 1);
		if(i3 == string::npos) {
			i3 = str.length();
			b = true;
		}
		stmp = str.substr(i + 1, i3 - i - 1);
		remove_blank_ends(stmp);
		if(stmp.empty()) {
			i = i3;
		} else {
			mngr = new Manager(stmp);
			stmp = LEFT_BRACKET_CH;
			stmp += ID_WRAP_LEFT_CH;
			stmp += i2s(addId(mngr));
			stmp += ID_WRAP_RIGHT_CH;
			stmp += RIGHT_BRACKET_CH;
			if(b) str.replace(i, str.length() - i, stmp);
			else str.replace(i, i3 + 1 - i, stmp);
			mngr->unref();		
		}
	}	
	gsub("\"", "", str);	
	i = -1; i3 = 0; b = false;
	for(int i2 = 0; i2 < (int) ufv.size(); i2++) {
		i = 0, i3 = 0;
		b = false;
		if(ufv_t[i2] == 'v' && b_variables) {
			v = (Variable*) ufv[i2];
			while(1) {
				if((i = str.find(v->name(), i3)) != (int) string::npos) {
					stmp = LEFT_BRACKET_CH;
					stmp += ID_WRAP_LEFT_CH;
					if(b_calcvars && (!b_always_exact || v->isPrecise())) {
						mngr = new Manager(v->get());
						mngr->recalculateFunctions();
						if(!v->isPrecise()) {
							mngr->setPrecise(false);
						}
						stmp += i2s(addId(mngr));
						mngr->unref();
					} else {
						mngr = new Manager(v->name());
						stmp += i2s(addId(mngr));
						mngr->unref();
					}
					stmp += ID_WRAP_RIGHT_CH;
					stmp += RIGHT_BRACKET_CH;
					str.replace(i, v->name().length(), stmp);
				} else {
					break;
				}
			}
		} else if(ufv_t[i2] == 'f' && b_functions) {
			f = (Function*) ufv[i2];
			while(1) {
				b = false;
				if((i = str.find(f->name(), i3)) != (int) string::npos) {
					i4 = -1;
					if(f->args() == 0) {
						i5 = str.find_first_not_of(SPACES, i + f->name().length());
						if(i5 != string::npos && str[i5] == LEFT_BRACKET_CH) {
							i5 = str.find_first_not_of(SPACES, i5 + 1);							
							if(i5 != string::npos && str[i5] == RIGHT_BRACKET_CH) {
								i4 = i5 - i + 1;
							}
						}
						mngr =  f->calculate("");
						if(mngr) {
							stmp = LEFT_BRACKET_CH;
							stmp += ID_WRAP_LEFT_CH;
							stmp += i2s(addId(mngr));
							mngr->unref();
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_BRACKET_CH;
						} else {
							stmp = "";
						}
						if(i4 < 0) i4 = f->name().length();
					} else {
						b = false;
						i5 = 1;
						i6 = 0;
						while(i5 > 0 && !b) {
							if(i6 + i + (int) f->name().length() >= (int) str.length()) {
								b = true;
								i5 = 2;
								i6++;
								break;
							} else {
								char c = str[i + (int) f->name().length() + i6];
								if(c == LEFT_BRACKET_CH && i5 != 2) {
									b = true;
								} else if(c == ' ') {
									if(i5 == 2) {
										b = true;
									}
								} else if(i5 == 2 && is_in(OPERATORS, str[i + (int) f->name().length() + i6])) {
									b = true;
								} else {
									if(i6 > 0) {
										i5 = 2;
									} else {
										i5 = -1;
									}		
								}
							}
							i6++;
						}
						if(b && i5 == 2) {

							stmp2 = str.substr(i + f->name().length(), i6 - 1);

							mngr =  f->calculate(stmp2);
							if(mngr) {
								stmp = LEFT_BRACKET_CH;
								stmp += ID_WRAP_LEFT_CH;
								stmp += i2s(addId(mngr));
								mngr->unref();
								stmp += ID_WRAP_RIGHT_CH;
								stmp += RIGHT_BRACKET_CH;
							} else {
								stmp = "";
							}
							
							i4 = i6 + 1 + f->name().length() - 2;
							b = false;
						}
						i9 = i6;
						if(b) {
							b = false;
							i6 = i6 + 1 + i + (int) f->name().length();
							i7 = i6 - 1;
							i8 = i7;
/*							i8 = 0;
							while(1) {
								i5 = str.find_first_of(RIGHT_BRACKET LEFT_BRACKET, i7);
								if(i5 == string::npos) {
									for(int index = 0; index < i8; index++) {
										str.append(1, RIGHT_BRACKET_CH);
									}
								}
							}*/
							while(1) {
								i5 = str.find(RIGHT_BRACKET_CH, i7);
								if(i5 == string::npos) {
									str.append(1, RIGHT_BRACKET_CH);
									i5 = str.length() - 1;
								}
								if(i5 < (i6 = str.find(LEFT_BRACKET_CH, i8)) || i6 == string::npos) {
									i6 = i5;
									b = true;
									break;
								}
								i7 = i5 + 1;
								i8 = i6 + 1;
							}
						}
						if(b) {
							stmp2 = str.substr(i + f->name().length() + i9, i6 - (i + f->name().length() + i9));
							mngr =  f->calculate(stmp2);
							if(mngr) {
								stmp = LEFT_BRACKET_CH;
								stmp += ID_WRAP_LEFT_CH;
								stmp += i2s(addId(mngr));
								mngr->unref();
								stmp += ID_WRAP_RIGHT_CH;
								stmp += RIGHT_BRACKET_CH;
							} else {
								stmp = "";
							}							

							i4 = i6 + 1 - i;
						}
					}
					if(i4 > 0) {
						str.replace(i, i4, stmp);
					} else {
						i3 = i + 1;
					}
				} else {
					break;
				}
			}
		} else if(b_units && (ufv_t[i2] == 'p' || ufv_t[i2] == 'P')) {
			p = (Prefix*) ufv[i2];
			if(ufv_t[i2] == 'p') {
				ch = p->shortName();
			} else {
				ch = p->longName();
			}
			while(1) {
				if((i = str.find(ch, i3)) != (int) string::npos) {
					i4 = i + ch.length() - 1;
					b = true;
					if(b_units) {
						i5 = str.find_first_of(NUMBERS OPERATORS BRACKETS SPACES, i4);
						if(i5 == string::npos)
							i5 = str.length() - 1;
						else
							i5--;
						if(i5 != i) {
							for(i6 = 0; i6 < (int) ufv.size(); i6++) {
								if(ufv_t[i6] == 'u') {
									i7 = ((Unit*) ufv[i6])->shortName(false).length();
								} else if(ufv_t[i6] == 'U') {
									i7 = ((Unit*) ufv[i6])->name().length();
								} else if(ufv_t[i6] == 'Y') {
									i7 = ((Unit*) ufv[i6])->plural(false).length();
								} else {
									i7 = -1;
								}
								if(i7 > 0 && i7 <= i5 - i4) {
									b = false;
									for(i8 = 1; i8 <= i7; i8++) {
										if((ufv_t[i6] == 'u' && str[i4 + i8] != ((Unit*) ufv[i6])->shortName(false)[i8 - 1]) || (ufv_t[i6] == 'U' && str[i4 + i8] != ((Unit*) ufv[i6])->name()[i8 - 1]) || (ufv_t[i6] == 'Y' && str[i4 + i8] != ((Unit*) ufv[i6])->plural(false)[i8 - 1])) {
											b = true;
											break;
										}
									}
									if(!b) {
										u = (Unit*) ufv[i6];
										if(str.length() > i4 + 1 && is_in(NUMBERS, str[i4 + 1])) {
											str.insert(i4 + 1, 1, POWER_CH);
										}
										i4 += i7;
										break;
									}
								}
							}
						}
					}
					if(!b) {
						stmp = LEFT_BRACKET_CH;
						stmp += ID_WRAP_LEFT_CH;
						if(b) mngr = new Manager(1, 1, p->exponent());
						else mngr = new Manager(u, p->exponent());
						stmp += i2s(addId(mngr));
						mngr->unref();
						stmp += ID_WRAP_RIGHT_CH;
						stmp += RIGHT_BRACKET_CH;
						if(!b) str.replace(i, ch.length() + i7, stmp);
						else str.replace(i, ch.length(), stmp);						
					} else {
						i3 = i + 1;
					}
				} else {
					break;
				}
			}
		} else if(b_units && (ufv_t[i2] == 'u' || ufv_t[i2] == 'U' || ufv_t[i2] == 'Y')) {
			u = (Unit*) ufv[i2];
			while(u->unitType() != COMPOSITE_UNIT) {
				find_unit:
				value = 0;
				if((ufv_t[i2] == 'u' && (i = str.find(u->shortName(), i3)) != (int) string::npos) || (ufv_t[i2] == 'U' && (i = str.find(u->name(), i3)) != (int) string::npos) || (ufv_t[i2] == 'Y' && (i = str.find(u->plural(), i3)) != (int) string::npos)) {
					if(ufv_t[i2] == 'u')
						i4 = i + u->shortName().length() - 1;
					else if(ufv_t[i2] == 'Y')
						i4 = i + u->plural().length() - 1;
					else
						i4 = i + u->name().length() - 1;
					if(i4 != str.length() - 1 && is_not_in(SPACES NUMBERS OPERATORS BRACKETS DOT, str[i4 + 1])) {
						i3 = i + 1;
						if(i3 >= str.length()) break;
						goto find_unit;
					}
					i5 = str.find_last_of(NUMBERS OPERATORS BRACKETS SPACES, i);
					if(i5 == string::npos)
						i5 = 0;
					else
						i5++;
					if(i5 != i) {
						stmp = str.substr(i5, i - i5);
						for(int index = 0; index < prefixes.size(); index++) {
							i7 = prefixes[index]->longName(false).length();
							if(i7 > 0 && i7 <= i - i5) {
								b = true;
								for(i6 = 1; i6 <= i7; i6++) {
									if(str[i - i6] != prefixes[index]->longName(false)[i7 - i6]) {
										b = false;
										break;
									}
								}
								if(b) {
									value = prefixes[index]->exponent();
									i -= i7;
									break;
								}
							}
						}					
						for(int index = 0; value == 0 && index < prefixes.size(); index++) {								
							i7 = prefixes[index]->shortName(false).length();
							if(i7 > 0 && i7 <= i - i5) {
								b = true;
								for(i6 = 1; i6 <= i7; i6++) {
									if(str[i - i6] != prefixes[index]->shortName(false)[i7 - i6]) {
										b = false;
										break;
									}
								}
								if(b) {
									value = prefixes[index]->exponent();
									i -= i7;
									break;
								}
							}							
						}
					}
					if(str.length() > i4 + 1 && is_in(NUMBERS, str[i4 + 1])) {
						str.insert(i4 + 1, 1, POWER_CH);
					}
					mngr = new Manager(u, value);
					stmp = LEFT_BRACKET_CH;					
					stmp += ID_WRAP_LEFT_CH;
					stmp += i2s(addId(mngr));
					mngr->unref();
					stmp += ID_WRAP_RIGHT_CH;
					stmp += RIGHT_BRACKET_CH;				
					str.replace(i, i4 - i + 1, stmp);
				} else {
					break;
				}
			}
		}
	}
	if(b_rpn) {
		int rpn_i = str.find(SPACE, 0);
		while(rpn_i != string::npos) {
			if(rpn_i == 0 || is_in(OPERATORS, str[rpn_i - 1]) || rpn_i + 1 == str.length() || is_in(SPACE OPERATORS, str[rpn_i + 1])) {
				str.erase(rpn_i, 1);
			} else {
				rpn_i++;
			}
			rpn_i = str.find(SPACE, rpn_i);
		}
	} else {
		remove_blanks(str);
	}
	i = 0;
	if(!b_units && !b_unknown) return;	
	u = NULL;
	while(true) {
		i = str.find_first_not_of(NUMBERS NOT_IN_NAMES DOT, i);
		if(i == string::npos) break;
		stmp = str[i];
		if(b_units) u = getUnit(stmp);
		if(u) {
			mngr = new Manager(u);
		} else if(stmp == "E") {
			mngr = NULL;
			i++;
//			str.replace(i, 1, MULTIPLICATION "10" POWER);
		} else if(b_unknown) {
			mngr = new Manager(stmp);
		} else {
			mngr = NULL;
			i++;
		}
		if(mngr) {
			stmp = LEFT_BRACKET_CH;
			stmp += ID_WRAP_LEFT_CH;
			stmp += i2s(addId(mngr));
			mngr->unref();
			stmp += ID_WRAP_RIGHT_CH;
			stmp += RIGHT_BRACKET_CH;
			str.replace(i, 1, stmp);
		}
	}
	f = getFunction("factorial");
	while(f) {
		i = str.find("!", i);	
		if(i == string::npos) break;		
		if(i != 0) {
			stmp = "";
			if(is_in(NUMBERS, str[i - 1])) {
				i3 = str.find_last_not_of(NUMBERS, i - 1);
				if(i3 == string::npos) {
					stmp2 = str.substr(0, i);
				} else {
					stmp2 = str.substr(i3 + 1, i - i3 - 1);
				}
			} else if(str[i - 1] == RIGHT_BRACKET_CH) {
				i3 = i - 2;
				i4 = 1;
				while(true) {
					i3 = str.find_last_of(BRACKETS, i3);
					if(i3 == string::npos) {
						break;
					}
					if(str[i3] == RIGHT_BRACKET_CH) {
						i4++;
					} else {
						i4--;
						if(i4 == 0) {
							stmp2 = str.substr(i3, i - i3);
							break;
						}
					}
					i3--;
				}
			}
			if(!stmp2.empty()) {
				mngr =  f->calculate(stmp2);
				if(mngr) {
					stmp = LEFT_BRACKET_CH;
					stmp += ID_WRAP_LEFT_CH;
					stmp += i2s(addId(mngr));
					mngr->unref();
					stmp += ID_WRAP_RIGHT_CH;
					stmp += RIGHT_BRACKET_CH;
				} else {
					stmp = "";
				}
				str.replace(i - stmp2.length(), stmp2.length() + 1, stmp);
			} else {
				i++;
			}
		} else {
			i++;
		}
	}
}
string Calculator::getName(string name, ExpressionItem *object, bool force, bool always_append) {
	switch(object->type()) {
		case TYPE_UNIT: {
			return getUnitName(name, (Unit*) object, force, always_append);
		}
	}
	if(force) {
		ExpressionItem *item = getActiveExpressionItem(name, object);
		if(item) {
			if(!item->isLocal()) {
				bool b = item->hasChanged();
				if(object->isActive()) {
					item->setActive(false);
				}
				if(!object->isLocal()) {
					item->setChanged(b);
				}
			} else {
				if(object->isActive()) {
					item->destroy();
				}
			}
		}
		return name;
	}
	int i2 = 1;
	if(name.empty()) {
		name = "x";
		always_append = true;
	}
	string stmp = name;
	if(always_append) {
		stmp += NAME_NUMBER_PRE_STR;
		stmp += "1";
	}
	for(int i = 0; i < (int) variables.size(); i++) {
		if(variables[i] != object && variables[i]->name() == stmp) {
			i2++;
			stmp = name;
			stmp += NAME_NUMBER_PRE_STR;
			stmp += i2s(i2);
		}
	}
	for(int i = 0; i < (int) functions.size(); i++) {
		if(functions[i] != object && functions[i]->name() == stmp) {
			i2++;
			stmp = name;
			stmp += NAME_NUMBER_PRE_STR;
			stmp += i2s(i2);
		}
	}
	if(i2 > 1 && !always_append)
		error(false, _("Name \"%s\" is in use. Replacing with \"%s\"."), name.c_str(), stmp.c_str(), NULL);
	return stmp;
}
string Calculator::getUnitName(string name, Unit *object, bool force, bool always_append) {
	if(force) {
		ExpressionItem *item = getActiveExpressionItem(name, object);
		if(item) {
			if(!item->isLocal()) {
				bool b = item->hasChanged();
				if(object->isActive()) {
					item->setActive(false);
				}
				if(!object->isLocal()) {
					item->setChanged(b);
				}			
			} else {
				if(object->isActive()) {
					item->destroy();
				}			
			}
		}
		return name;	
	}
	int i2 = 1;
	if(name.empty()) {
		name = "x";
		always_append = true;
	}
	string stmp = name;
	if(always_append)
		stmp += NAME_NUMBER_PRE_STR;
		stmp += "1";
	for(int i = 0; i < (int) units.size(); i++) {
		if(units[i] != object && ((units[i]->unitType() == COMPOSITE_UNIT && units[i]->referenceName() == stmp) || (units[i]->unitType() != COMPOSITE_UNIT && (units[i]->name() == stmp || units[i]->shortName(false) == stmp || units[i]->plural(false) == stmp)))) {
			i2++;
			stmp = name;
			stmp += NAME_NUMBER_PRE_STR;
			stmp += i2s(i2);
		}
	}
	if(i2 > 1 && !always_append)
		error(false, _("Name \"%s\" is in use. Replacing with \"%s\"."), name.c_str(), stmp.c_str(), NULL);
	return stmp;
}
bool Calculator::save(const char* file_name) {
	FILE *file = fopen(file_name, "w+");
	if(file == NULL)
		return false;
	string str;
	unsetLocale();
	fprintf(file, "##########################################\n");
	fprintf(file, "#       QALCULATE! DEFINITIONS FILE      #\n");
	fprintf(file, "##########################################\n");
	fprintf(file, "\n");
	fprintf(file, "*Version\t" VERSION);
	fprintf(file, "\n");
	for(int i = 0; i < functions.size(); i++) {
		if(functions[i]->isLocal()) {	
			if(!functions[i]->isBuiltin()) {
				fprintf(file, "*Function\t");
			} else {
				fprintf(file, "*BuiltinFunction\t");
			}
			fprintf(file, "%i\t", functions[i]->isActive());
			if(functions[i]->isBuiltin()) {
				fprintf(file, "%s\t", functions[i]->referenceName().c_str());
			}
			if(functions[i]->category().empty()) {
				fprintf(file, "0\t");
			} else {
				fprintf(file, "%s\t", functions[i]->category().c_str());
			}
			if(!functions[i]->isBuiltin()) {
				fprintf(file, "%s\t%s\t%i\t", functions[i]->name().c_str(), ((UserFunction*) functions[i])->equation().c_str(), functions[i]->isPrecise());
			}	
			if(functions[i]->title(false).empty()) {
				fprintf(file, "0\t");
			} else {
				fprintf(file, "%s\t", functions[i]->title(false).c_str());
			}
			str = functions[i]->description();
			gsub("\n", "\\", str);
			if(str.empty()) {
				fprintf(file, "0");
			} else {
				fprintf(file, "%s", str.c_str());
			}
			if(functions[i]->isBuiltin()) {
				for(int i2 = 1; i2 <= functions[i]->lastArgumentNameIndex(); i2++) {
					if(functions[i]->argumentName(i2).empty()) {
						fprintf(file, "\t%s", "0");
					} else {
						fprintf(file, "\t%s", functions[i]->argumentName(i2).c_str());
					}
				}			
			} else {
				for(int i2 = 1; i2 <= functions[i]->lastArgumentNameIndex() || i2 <= functions[i]->lastArgumentTypeIndex(); i2++) {
					switch(functions[i]->argumentType(i2)) {
						case ARGUMENT_TYPE_TEXT: {
							fprintf(file, "\tT");
							break;
						}
						case ARGUMENT_TYPE_DATE: {
							fprintf(file, "\tD");
							break;
						}
						case ARGUMENT_TYPE_POSITIVE: {
							fprintf(file, "\tP");
							break;
						}
						case ARGUMENT_TYPE_NONNEGATIVE: {
							fprintf(file, "\t!N");
							break;
						}
						case ARGUMENT_TYPE_NONZERO: {
							fprintf(file, "\t!0");
							break;
						}						
						case ARGUMENT_TYPE_INTEGER: {
							fprintf(file, "\tI");
							break;
						}
						case ARGUMENT_TYPE_POSITIVE_INTEGER: {
							fprintf(file, "\tPI");
							break;
						}
						case ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {
							fprintf(file, "\t!NI");
							break;
						}
						case ARGUMENT_TYPE_NONZERO_INTEGER: {
							fprintf(file, "\t!0I");
							break;
						}						
						case ARGUMENT_TYPE_FRACTION: {
							fprintf(file, "\tFr");
							break;
						}
						case ARGUMENT_TYPE_VECTOR: {
							fprintf(file, "\tV");
							break;
						}
						case ARGUMENT_TYPE_MATRIX: {
							fprintf(file, "\tM");
							break;
						}
						case ARGUMENT_TYPE_FUNCTION: {
							fprintf(file, "\tF");
							break;
						}
						case ARGUMENT_TYPE_UNIT: {
							fprintf(file, "\tU");
							break;
						}
						case ARGUMENT_TYPE_BOOLEAN: {
							fprintf(file, "\tB");
							break;
						}						
						default: {
							fprintf(file, "\t0");
						}
					}
					if(functions[i]->argumentName(i2).empty()) {
						fprintf(file, "\t%s", "0");
					} else {
						fprintf(file, "\t%s", functions[i]->argumentName(i2).c_str());
					}
				}
			}
			fprintf(file, "\n");
		} else {
			if(!functions[i]->isActive() && functions[i]->hasChanged()) {
				fprintf(file, "*Deactivate\t%s\n", functions[i]->referenceName().c_str());
			}			
		}
	}
	fprintf(file, "\n");
	CompositeUnit *cu;
	AliasUnit *au;
	Unit *u;
	int exp = 1;
	for(int i = 0; i < units.size(); i++) {
		if(units[i]->isLocal()) {
			switch(units[i]->unitType()) {
				case BASE_UNIT: {
					fprintf(file, "*Unit\t");
					break;
				}
				case ALIAS_UNIT: {
					au = (AliasUnit*) units[i];
					fprintf(file, "*AliasUnit\t");
					break;
				}
				case COMPOSITE_UNIT: {
					fprintf(file, "*CompositeUnit\t");
					break;
				}
			}
			fprintf(file, "%i\t", units[i]->isActive());
			if(units[i]->category().empty()) {
				fprintf(file, "0\t");
			} else {
				fprintf(file, "%s\t", units[i]->category().c_str());
			}
			if(units[i]->unitType() == COMPOSITE_UNIT) {
				cu = (CompositeUnit*) units[i];			
				fprintf(file, "%s\t", cu->referenceName().c_str());
				if(units[i]->title(false).empty())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s", units[i]->title(false).c_str());
				for(int i2 = 0; i2 < cu->units.size(); i2++) {
					fprintf(file, "\t%s\t%s\t%s", cu->units[i2]->firstBaseUnit()->shortName(true).c_str(), li2s(cu->units[i2]->firstBaseExp()).c_str(), li2s(cu->units[i2]->prefixExponent()).c_str());
				}
			} else {
				fprintf(file, "%s\t", units[i]->name().c_str());
				if(units[i]->plural(false).empty())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s\t", units[i]->plural(false).c_str());
				if(units[i]->shortName(false).empty())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s\t", units[i]->shortName(false).c_str());
				if(units[i]->title(false).empty())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s", units[i]->title(false).c_str());
			}
			if(units[i]->unitType() == ALIAS_UNIT) {
				fprintf(file, "\t%i", au->isPrecise());
				if(au->firstBaseUnit()->unitType() == COMPOSITE_UNIT) {
					fprintf(file, "\t%s\t%s\t%s", au->firstBaseUnit()->referenceName().c_str(), au->expression().c_str(), li2s(au->firstBaseExp()).c_str());
				} else {
					fprintf(file, "\t%s\t%s\t%s", au->firstShortBaseName().c_str(), au->expression().c_str(), li2s(au->firstBaseExp()).c_str());
				}	
				if(!au->reverseExpression().empty())
					fprintf(file, "\t%s", au->reverseExpression().c_str());
			}
			fprintf(file, "\n");
		} else {
			if(!units[i]->isActive() && units[i]->hasChanged()) {
				fprintf(file, "*Deactivate\t%s\n", units[i]->referenceName().c_str());
			}			
		}
	}
	fprintf(file, "\n");

	bool was_always_exact = alwaysExact();
	setAlwaysExact(true);	
	for(int i = 0; i < variables.size(); i++) {
		if(variables[i]->isLocal() && variables[i]->category() != _("Temporary")) {
			if(!variables[i]->isBuiltin()) {
				fprintf(file, "*Variable\t");
			} else {
				fprintf(file, "*BuiltinVariable\t");
			}
			fprintf(file, "%i\t", variables[i]->isActive());
			if(variables[i]->isBuiltin()) {
				fprintf(file, "%s\t", variables[i]->referenceName().c_str());
			}
			if(variables[i]->category().empty()) {
				fprintf(file, "0\t");
			} else {
				fprintf(file, "%s\t", variables[i]->category().c_str());
			}
			if(!variables[i]->isBuiltin()) {
				fprintf(file, "%s\t%s\t", variables[i]->name().c_str(), variables[i]->get()->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTIONAL_ONLY).c_str());
			}
			if(variables[i]->title(false).empty()) {
				fprintf(file, "0\t");
			} else {
				fprintf(file, "%s\t", variables[i]->title(false).c_str());
			}
			if(!variables[i]->isBuiltin()) {
				fprintf(file, "%i", variables[i]->isPrecise());
			}
			fprintf(file, "\n");
		} else {
			if(!variables[i]->isActive() && variables[i]->hasChanged()) {
				fprintf(file, "*Deactivate\t%s\n", variables[i]->referenceName().c_str());
			}
		}
	}
	setAlwaysExact(was_always_exact);
	
	fclose(file);
	setLocale();
}
bool Calculator::load(const char* file_name, bool is_user_defs) {
	FILE *file = fopen(file_name, "r");
	if(file == NULL)
		return false;
	string stmp, str, ntmp, vtmp, rtmp, etmp, shtmp, ctmp, ttmp, cutmp;
	Unit *u;
	AliasUnit *au;
	Variable *v;
	Function *func;
	Manager *mngr;
	Prefix *p;
	bool b, b2;
	bool unit_added = false, units_added = false, rerun = false;
	int rerun_i = 0;
	vector<string> unfinished_units;
	unsigned int i, i2, i3;
	char line[10000];
	bool functions_was = b_functions, variables_was = b_variables, units_was = b_units, unknown_was = b_unknown, calcvars_was = b_calcvars, always_exact_was = b_always_exact, rpn_was = b_rpn;
	b_functions = true; b_variables = true; b_units = true; b_unknown = true; b_calcvars = true; b_always_exact = true; b_rpn = false;
	while(1) {
		if(fgets(line, 10000, file) == NULL) {
			rerun = true;
			unit_added = false;
		}
		if(line[0] == '*' || rerun) {
			if(rerun) {
				if(unit_added) {
					unfinished_units.erase(unfinished_units.begin());
					rerun_i--;
					unit_added = false;
					units_added = true; 
				}
				if(rerun_i < unfinished_units.size()) {
					stmp = unfinished_units[rerun_i];
					rerun_i++;
				} else if(units_added && unfinished_units.size() > 0) {
					units_added = false;
					rerun_i = 0;
					stmp = unfinished_units[rerun_i];
					rerun_i++;					
				} else {
					break;
				}	
			} else {
				stmp = line;
			}
			if((i = stmp.find_first_of("\t")) != string::npos) {
				str = stmp.substr(0, i);
				if(str == "*Deactivate") {
					FIRST_READ_TAB_DELIMITED(ntmp)
						ExpressionItem *item = getActiveExpressionItem(ntmp);
						if(!item->isLocal()) {
							item->setActive(false);
						}
					}
				} else if(str == "*Activate") {
					FIRST_READ_TAB_DELIMITED(ntmp)
						ExpressionItem *item = getInactiveExpressionItem(ntmp);
						if(!item->isLocal()) {
							item->setActive(true);
						}
					}					
				} else if(str == "*Variable") {
					FIRST_READ_TAB_DELIMITED_SET_BOOL(b2)
						READ_TAB_DELIMITED_SET_0(ctmp)
							READ_TAB_DELIMITED(ntmp)
								READ_TAB_DELIMITED(vtmp)
									ttmp = "";
									READ_TAB_DELIMITED_SET_0(ttmp)
									}
									if(variableNameIsValid(ntmp)) {
										v = new Variable(ctmp, ntmp, vtmp, ttmp, is_user_defs, false, b2);
										v = addVariable(v);
										READ_TAB_DELIMITED_SET_BOOL(b)
											if(v) v->setPrecise(b);	
										}									
									}
								}
							}
						}
					}
				} else if(str == "*BuiltinVariable") {
					FIRST_READ_TAB_DELIMITED_SET_BOOL(b2)
						READ_TAB_DELIMITED(ntmp)
							v = getVariable(ntmp);
							READ_TAB_DELIMITED_SET_0(ctmp)
								if(v) {
									v->setLocal(is_user_defs, b2);
									v->setCategory(ctmp);
								}
								ttmp = "";
								READ_TAB_DELIMITED_SET_0(ttmp)
									if(v) {
										v->setTitle(ttmp);
									}
								}
							}
							if(v) {
								v->setChanged(false);
							}
						}
					}
				} else if(str == "*Function") {
					FIRST_READ_TAB_DELIMITED_SET_BOOL(b2)
						READ_TAB_DELIMITED_SET_0(ctmp)
							READ_TAB_DELIMITED(ntmp)
								if(functionNameIsValid(ntmp)) {
									TEST_TAB_DELIMITED
										vtmp = stmp.substr(i, i2 - i);
										func = addFunction(new UserFunction(ctmp, ntmp, vtmp, is_user_defs, -1, "", "", 0, b2));
										READ_TAB_DELIMITED_SET_BOOL(b)
											func->setPrecise(b);	
											READ_TAB_DELIMITED_SET_0(shtmp)
												func->setTitle(shtmp);
												READ_TAB_DELIMITED_SET_0(shtmp)
													gsub("\\", "\n", shtmp);
													func->setDescription(shtmp);
													int i4 = 1;
													while(true) {
														TEST_TAB_DELIMITED
															vtmp = stmp.substr(i, i2 - i);
															remove_blank_ends(vtmp);
															if(i4 % 2) {
																if(vtmp == "0") {
																} else if(vtmp == "T") {
																	func->setArgumentType(ARGUMENT_TYPE_TEXT, i4 / 2 + 1);
																} else if(vtmp == "D") {
																	func->setArgumentType(ARGUMENT_TYPE_DATE, i4 / 2 + 1);
																} else if(vtmp == "P") {
																	func->setArgumentType(ARGUMENT_TYPE_POSITIVE, i4 / 2 + 1);
																} else if(vtmp == "!N") {
																	func->setArgumentType(ARGUMENT_TYPE_NONNEGATIVE, i4 / 2 + 1);
																} else if(vtmp == "!0") {
																	func->setArgumentType(ARGUMENT_TYPE_NONZERO, i4 / 2 + 1);	
																} else if(vtmp == "I") {
																	func->setArgumentType(ARGUMENT_TYPE_INTEGER, i4 / 2 + 1);
																} else if(vtmp == "PI") {
																	func->setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, i4 / 2 + 1);
																} else if(vtmp == "!NI") {
																	func->setArgumentType(ARGUMENT_TYPE_NONNEGATIVE_INTEGER, i4 / 2 + 1);
																} else if(vtmp == "!0I") {
																	func->setArgumentType(ARGUMENT_TYPE_NONZERO_INTEGER, i4 / 2 + 1);	
																} else if(vtmp == "Fr") {
																	func->setArgumentType(ARGUMENT_TYPE_FRACTION, i4 / 2 + 1);
																} else if(vtmp == "V") {
																	func->setArgumentType(ARGUMENT_TYPE_VECTOR, i4 / 2 + 1);
																} else if(vtmp == "M") {
																	func->setArgumentType(ARGUMENT_TYPE_MATRIX, i4 / 2 + 1);
																} else if(vtmp == "F") {
																	func->setArgumentType(ARGUMENT_TYPE_FUNCTION, i4 / 2 + 1);
																} else if(vtmp == "U") {
																	func->setArgumentType(ARGUMENT_TYPE_UNIT, i4 / 2 + 1);
																} else if(vtmp == "B") {
																	func->setArgumentType(ARGUMENT_TYPE_BOOLEAN, i4 / 2 + 1);
																}
															} else {
																if(vtmp != "0") {
																	func->setArgumentName(vtmp, i4 / 2);
																}
															}
															i4++;
														} else {
															break;
														}
													}
												}
											}
										}
										func->setChanged(false);
									}
								}
							}
						}
					}
				} else if(str == "*BuiltinFunction") {
					FIRST_READ_TAB_DELIMITED_SET_BOOL(b2)
						READ_TAB_DELIMITED(ntmp)
							func = getFunction(ntmp);
							READ_TAB_DELIMITED_SET_0(ctmp)
								if(func) {
									func->setLocal(is_user_defs, b2);
									func->setCategory(ctmp);
								}
								READ_TAB_DELIMITED_SET_0(shtmp)								
									if(func) {
										func->setTitle(shtmp);
									}
									READ_TAB_DELIMITED_SET_0(shtmp)
										gsub("\\", "\n", shtmp);
										if(func) {
											func->setDescription(shtmp);
										}
										b = true;
										int i4 = 1;										
										while(true) {
											TEST_TAB_DELIMITED
												if(func && b) {
													func->clearArgumentNames();
													b = false;
												}
												if(func) {
													vtmp = stmp.substr(i, i2 - i);
													remove_blank_ends(vtmp);
													if(vtmp != "0") {
														func->setArgumentName(vtmp, i4);
													}
													i4++;
												} else {
													break;
												}
											} else {
												break;
											}
										}
									}
								}
							}
							if(func) {
								func->setChanged(false);
							}
						}
					}
				} else if(str == "*Unit") {
					FIRST_READ_TAB_DELIMITED_SET_BOOL(b2)
						READ_TAB_DELIMITED_SET_0(ctmp)
							READ_TAB_DELIMITED(ntmp)
								shtmp = "";
								ttmp = "";
								etmp = "";
								READ_TAB_DELIMITED_SET_0(etmp)
									READ_TAB_DELIMITED_SET_0(shtmp)
										READ_TAB_DELIMITED_SET_0(ttmp)
										}
									}
								}
								if(unitNameIsValid(ntmp) && unitNameIsValid(etmp) && unitNameIsValid(shtmp)) {
									addUnit(new Unit(ctmp, ntmp, etmp, shtmp, ttmp, is_user_defs, false, b2));
								}
							}
						}
					}
				} else if(str == "*CompositeUnit") {
					FIRST_READ_TAB_DELIMITED_SET_BOOL(b2)
						READ_TAB_DELIMITED_SET_0(ctmp)
							READ_TAB_DELIMITED(ntmp)
								ttmp = "";
								READ_TAB_DELIMITED_SET_0(ttmp)
									CompositeUnit* cu = NULL;
									i3 = 0;
									if(unitNameIsValid(ntmp)) {
										while(1) {
											READ_TAB_DELIMITED(vtmp)
												READ_TAB_DELIMITED(cutmp)
													READ_TAB_DELIMITED(rtmp)
														long int li_prefix = s2li(rtmp);
														if(li_prefix == 0) {
															p = NULL;
														} else {
															p = getExactPrefix(li_prefix);
															if(!p) {
																if(cu) {
																	delete cu;
																}
																cu = NULL;
																break;
															}												
														}
														u = getUnit(vtmp);
														if(!u) {
															u = getCompositeUnit(vtmp);
														}
														if(!u) {
															if(!rerun) {
																unfinished_units.push_back(stmp);
															}
															if(cu) delete cu;
															cu = NULL;
															break;
														}
														if(u) {
															if(i3 == 0) {
																cu = new CompositeUnit(ctmp, ntmp, ttmp, "", is_user_defs, false, b2);
															}
															if(cu) {
																cu->add(u, s2li(cutmp), p);
															}
														}
													} else {
														break;
													}
												} else {
													break;
												}
											} else {
												break;
											}
											i3++;
										}
										if(cu) {
											addUnit(cu);
											cu->setChanged(false);
											unit_added = true;
										}
									}
								}						
							}
						}
					}
				} else if(str == "*AliasUnit") {
					FIRST_READ_TAB_DELIMITED_SET_BOOL(b2)
						READ_TAB_DELIMITED_SET_0(ctmp)
							READ_TAB_DELIMITED(ntmp)
								shtmp = "";
								ttmp = "";
								etmp = "";
								READ_TAB_DELIMITED_SET_0(etmp)
									READ_TAB_DELIMITED_SET_0(shtmp)
										READ_TAB_DELIMITED_SET_0(ttmp)
											READ_TAB_DELIMITED_SET_BOOL(b)
												READ_TAB_DELIMITED(vtmp)
													u = getUnit(vtmp);
													if(!u) {
														u = getCompositeUnit(vtmp);
													}
													if(!u && !rerun) {
														unfinished_units.push_back(stmp);
													}
													if(u && (unitNameIsValid(ntmp) && unitNameIsValid(etmp) && unitNameIsValid(shtmp))) {
														au = new AliasUnit(ctmp, ntmp, etmp, shtmp, ttmp, u, "1", 1, "", is_user_defs, false, b2);
														TEST_TAB_DELIMITED
															au->setExpression(stmp.substr(i, i2 - i));
															TEST_TAB_DELIMITED
																au->setExponent(s2li(stmp.substr(i, i2 - i)));
																TEST_TAB_DELIMITED
																	au->setReverseExpression(stmp.substr(i, i2 - i));
																}
															}
														}
														addUnit(au);
														au->setPrecise(b);
														au->setChanged(false);
														unit_added = true;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				} else if(str == "*Prefix") {
					FIRST_READ_TAB_DELIMITED_SET_0(ntmp)
						READ_TAB_DELIMITED_SET_0(shtmp)
							READ_TAB_DELIMITED(vtmp)
								addPrefix(new Prefix(s2li(vtmp.c_str()), ntmp, shtmp));						
							}
						}
					}
				}
			}
		}
	}
	b_functions = functions_was; b_variables = variables_was; b_units = units_was; b_unknown = unknown_was; b_calcvars = calcvars_was; b_always_exact = always_exact_was; b_rpn = rpn_was;
	fclose(file);
	return true;
}

long double Calculator::getAngleValue(long double value) {
	switch(angleMode()) {
	    case RADIANS: {return value;}
	    case DEGREES: {return deg2rad(value);}
	    case GRADIANS: {return gra2rad(value);}
	}
}
Manager *Calculator::setAngleValue(Manager *mngr) {
	switch(angleMode()) {
		case DEGREES: {
			Fraction fr;
			fr.pi();
			Manager mngr_pi(&fr);
	    		mngr->add(&mngr_pi, MULTIPLY);
	    		mngr->addFloat(180, DIVIDE);			
			break;
		}
		case GRADIANS: {
			Fraction fr;
			fr.pi();
			Manager mngr_pi(&fr);
	    		mngr->add(&mngr_pi, MULTIPLY);			
	    		mngr->addFloat(200, DIVIDE);		
			break;
		}
	}
	return mngr;
}

bool Calculator::importCSV(const char *file_name, int first_row, bool headers, string delimiter, bool to_matrix, string name, string title, string category) {
	FILE *file = fopen(file_name, "r");
	if(file == NULL) {
		return false;
	}
	if(first_row < 1) {
		first_row = 1;
	}
	string filestr = file_name;
	int i = filestr.find_last_of("/");
	if(i != string::npos) {
		filestr = filestr.substr(i + 1, filestr.length() - (i + 1));
	}
	if(name.empty()) {
		int i = filestr.find_last_of(".");
		name = filestr.substr(0, i);
	}
	char line[10000];
	string stmp, str1, str2;
	int row = 0;
	int columns = 1;
	int column;
	vector<string> header;
	vector<Vector*> vectors;
	Matrix *mtrx;
	Manager *mngr;
	int is, is_n;
	bool v_added = false;
	while(fgets(line, 10000, file)) {
		row++;
		if(row >= first_row) {	
			stmp = line;
			remove_blank_ends(stmp);
			if(row == first_row) {
				if(stmp.empty()) {
					row--;
				} else {
					is = 0;
					while((is_n = stmp.find(delimiter, is)) != string::npos) {		
						columns++;
						if(headers) {
							str1 = stmp.substr(is, is_n - is);
							remove_blank_ends(str1);
							header.push_back(str1);
						}
						if(!to_matrix) {
							vectors.push_back(new Vector());
						}
						is = is_n + delimiter.length();
					}
					if(headers) {
						str1 = stmp.substr(is, stmp.length() - is);
						remove_blank_ends(str1);
						header.push_back(str1);
					}
					if(to_matrix) {
						mtrx = new Matrix(1, columns);
					} else {
						vectors.push_back(new Vector());
					}
				}
			}
			if((!headers || row > first_row) && !stmp.empty()) {
				if(to_matrix && v_added) {
					mtrx->addRow();
				}
				is = 0;
				column = 1;
				while(column <= columns) {
					is_n = stmp.find(delimiter, is);
					if(is_n == string::npos) {
						str1 = stmp.substr(is, stmp.length() - is);
					} else {
						str1 = stmp.substr(is, is_n - is);
						is = is_n + delimiter.length();
					}
					mngr = CALCULATOR->calculate(str1);
					if(to_matrix) {
						mtrx->set(mngr, mtrx->rows(), column);
					} else {
						if(v_added) {
							vectors[column - 1]->addComponent();
						}
						vectors[column - 1]->set(mngr, vectors[column - 1]->components());
					}
					mngr->unref();
					column++;
					if(is_n == string::npos) {
						break;
					}
				}
				for(; column <= columns; column++) {
					if(to_matrix) {
						mtrx->set(NULL, mtrx->rows(), column);
					} else {
						if(v_added) {
							vectors[column - 1]->addComponent();
						}
						vectors[column - 1]->set(NULL, vectors[column - 1]->components());
					}				
				}
				v_added = true;
			}
		}
	}
	if(to_matrix) {
		mngr = new Manager(mtrx);
		delete mtrx;
		addVariable(new Variable(category, name, mngr, title));
		mngr->unref();
	} else {
		if(vectors.size() > 1) {
			if(!category.empty()) {
				category += "/";	
			}
			category += name;
		}
		for(int i = 0; i < vectors.size(); i++) {
			mngr = new Manager(vectors[i]);
			delete vectors[i];
			str1 = "";
			str2 = "";
			if(vectors.size() > 1) {
				str1 += name;
				str1 += "_";
				if(title.empty()) {
					str2 += name;
					str2 += " ";
				} else {
					str2 += title;
					str2 += " ";
				}		
				if(i < header.size()) {
					str1 += header[i];
					str2 += header[i];
				} else {
					str1 += _("column");
					str1 += "_";
					str1 += i2s(i + 1);
					str2 += _("Column ");
					str2 += i2s(i + 1);				
				}
				gsub(" ", "_", str1);				
			} else {
				str1 = name;
				str2 = title;
				if(i < header.size()) {
					str2 += " (";
					str2 += header[i];
					str2 += ")";
				}
			}
			addVariable(new Variable(category, str1, mngr, str2));
			mngr->unref();			
		}
	}
	return true;
}
