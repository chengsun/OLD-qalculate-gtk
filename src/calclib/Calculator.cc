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
#include <config.h>

#include <fenv.h>
#include <locale.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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


#define XML_GET_FALSE_FROM_PROP(node, name, b)		value = xmlGetProp(node, (xmlChar*) name); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else {b = true;} if(value) xmlFree(value);
#define XML_GET_TRUE_FROM_PROP(node, name, b)		value = xmlGetProp(node, (xmlChar*) name); if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = false;} else {b = false;} if(value) xmlFree(value);
#define XML_GET_BOOL_FROM_PROP(node, name, b)		value = xmlGetProp(node, (xmlChar*) name); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = true;} if(value) xmlFree(value);
#define XML_GET_FALSE_FROM_TEXT(node, b)		value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else {b = true;} if(value) xmlFree(value);
#define XML_GET_TRUE_FROM_TEXT(node, b)			value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = true;} else {b = true;} if(value) xmlFree(value);
#define XML_GET_BOOL_FROM_TEXT(node, b)			value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = true;} if(value) xmlFree(value);
#define XML_GET_STRING_FROM_PROP(node, name, str)	value = xmlGetProp(node, (xmlChar*) name); if(value) str = (char*) value; else str = ""; if(value) xmlFree(value);
#define XML_GET_STRING_FROM_TEXT(node, str)		value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value) str = (char*) value; else str = ""; if(value) xmlFree(value);
#define XML_DO_FROM_PROP(node, name, action)		value = xmlGetProp(node, (xmlChar*) name); if(value) action((char*) value); else action(""); if(value) xmlFree(value);
#define XML_DO_FROM_TEXT(node, action)			value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value) action((char*) value); else action(""); if(value) xmlFree(value);
#define XML_GET_INT_FROM_PROP(node, name, i)		value = xmlGetProp(child, (xmlChar*) "index"); if(value) i = s2i((char*) value); if(value) xmlFree(value);


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

void *calculate_proc(void *x) {
	CALCULATOR->b_busy = true;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	Manager *mngr = (Manager*) x;
	mngr->set("aborted");
	Manager *mngr2 = CALCULATOR->calculate(CALCULATOR->expression_to_calculate, false);
	mngr->set(mngr2);
	mngr2->unref();
	CALCULATOR->b_busy = false;
}
void *print_proc(void *x) {
	CALCULATOR->b_busy = true;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	Manager *mngr = (Manager*) x;
	CALCULATOR->tmp_print_result = mngr->print(CALCULATOR->tmp_nrformat, CALCULATOR->tmp_displayflags, CALCULATOR->tmp_min_decimals, CALCULATOR->tmp_max_decimals, CALCULATOR->tmp_in_exact, CALCULATOR->tmp_usable, CALCULATOR->tmp_prefix);
	CALCULATOR->b_busy = false;
}

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
	addStringAlternative(SIGN_NOT_EQUAL, " " NOT EQUALS);		
	addStringAlternative(SIGN_GREATER_OR_EQUAL, GREATER EQUALS);	
	addStringAlternative(SIGN_LESS_OR_EQUAL, LESS EQUALS);		
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
	OPERATORS_S = "+-*/^!<>|=";
	BRACKETS_S = "()[]";
	LEFT_BRACKET_S = "([";
	LEFT_BRACKET_STR = "(";
	RIGHT_BRACKET_S = ")]";
	RIGHT_BRACKET_STR = ")";
	SPACE_S = " \t\n";
	SPACE_STR = " ";
	RESERVED_S = "@?\\{}:\"\',;";
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
	
	b_busy = false;
	pthread_attr_init(&calculate_thread_attr);	    	
	
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
	if(!item) return NULL;
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
						name = ((Unit*) item)->singular(false);
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
	if(name.empty()) return NULL;
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
				if(units[i]->name() == name || units[i]->singular(false) == name || units[i]->plural(false) == name) {
					return units[i];
				}
			}
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getInactiveExpressionItem(string name, ExpressionItem *item) {
	if(name.empty()) return NULL;
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
				if(units[i]->name() == name || units[i]->singular(false) == name || units[i]->plural(false) == name) {
					return units[i];
				}
			}
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getExpressionItem(string name, ExpressionItem *item) {
	if(name.empty()) return NULL;
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
					l = ((Unit*) (*it))->singular(false).length();
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
					l = ((Unit*) (*it))->singular(false).length();
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
	addVariable(new EVariable());
	addVariable(new PiVariable());	
	addVariable(new EulerVariable());
	addVariable(new AperyVariable());	
	addVariable(new CatalanVariable());
	addVariable(new PythagorasVariable());
	addVariable(new GoldenVariable());
}
void Calculator::addBuiltinFunctions() {
#ifdef HAVE_LIBCLN
	addFunction(new ZetaFunction());
#endif	
	addFunction(new ErrorFunction());
	addFunction(new ForFunction());
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
	addFunction(new YearFracFunction());		
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
void Calculator::saveState() {
	b_functions_was = b_functions; b_variables_was = b_variables; b_units_was = b_units; b_unknown_was = b_unknown; b_calcvars_was = b_calcvars; b_always_exact_was = b_always_exact; b_rpn_was = b_rpn;
}
void Calculator::restoreState() {
	b_functions = b_functions_was; b_variables = b_variables_was; b_units = b_units_was; b_unknown = b_unknown_was; b_calcvars = b_calcvars_was; b_always_exact = b_always_exact_was; b_rpn = b_rpn_was;
}
void Calculator::clearBuffers() {
	ids_p.clear();
	for(hash_map<int, Manager*>::iterator it = ids.begin(); it != ids.end(); ++it) {
		delete it->second;
	}
	ids.clear();
}
void Calculator::abort() {
	while(pthread_cancel(calculate_thread) == 0) {
		usleep(100);
	}
	restoreState();
	clearBuffers();
	b_busy = false;
}
bool Calculator::busy() {
	return b_busy;
}
Manager *Calculator::calculate(string str, bool enable_abort, int usecs) {
	Manager *mngr;
	if(enable_abort || usecs > 0) {
		saveState();
		b_busy = true;
		bool had_usecs = usecs > 0;
		mngr = new Manager();
		expression_to_calculate = str;
		while(!pthread_create(&calculate_thread, &calculate_thread_attr, calculate_proc, mngr) == 0) {
			usleep(100);
		}
		while(usecs > 0 && b_busy) {
			usleep(1000);
			usecs -= 1000;
		}	
		if(had_usecs && b_busy) {
			abort();
			return NULL;
		}
	} else {
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
		EqContainer *e = new EqContainer(str, OPERATION_ADD);
		mngr = e->calculate();
		mngr->finalize();
		if(!str2.empty()) {
			convert(mngr, str2);
		}
		mngr->ref();
		delete e;
		checkFPExceptions();
	}
	return mngr;
/*	int vtype = fpclassify(value);
	if(vtype == FP_NAN)
		error(true, _("Math error: not a number"), NULL);
	else if(vtype == FP_INFINITE)
		error(true, _("Math error: infinite"), NULL);*/		
}
string Calculator::printManagerTimeOut(Manager *mngr, int usecs, NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, bool *in_exact, bool *usable, Prefix *prefix) {
	if(!mngr) return "";
	tmp_nrformat = nrformat; tmp_displayflags = displayflags; tmp_min_decimals = min_decimals; tmp_max_decimals = max_decimals; tmp_in_exact = in_exact; tmp_usable = usable; tmp_prefix = prefix;
	saveState();
	b_busy = true;
	while(!pthread_create(&calculate_thread, &calculate_thread_attr, print_proc, mngr) == 0) {
		usleep(100);
	}
	while(usecs > 0 && b_busy) {
		usleep(1000);
		usecs -= 1000;
	}	
	if(b_busy) {
		abort();
		tmp_print_result = "timed out";
	}		
	return tmp_print_result;
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
	mngr->addUnit(from_unit, OPERATION_MULTIPLY);
	from_unit->hasComplexRelationTo(to_unit);
	mngr->convert(to_unit);
	mngr->finalize();	
//	mngr->convert(to_unit);
	mngr->addUnit(to_unit, OPERATION_DIVIDE);
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
			mngr->addUnit(to_unit, OPERATION_DIVIDE);
			mngr->finalize();			
			Manager *mngr2 = new Manager(to_unit);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				mngr->mngrs.push_back(mngr2);
			} else {
				mngr->transform(mngr2, MULTIPLICATION_MANAGER, OPERATION_MULTIPLY);
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
			mngr->add(mngr3, OPERATION_DIVIDE);
			mngr->finalize();			
			Manager *mngr2 = new Manager(cu);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				mngr->mngrs.push_back(mngr2);
			} else {
				mngr->transform(mngr2, MULTIPLICATION_MANAGER, OPERATION_MULTIPLY);
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
		if(!u->singular(false).empty()) {
			u->setSingular(getUnitName(u->singular(false), u, force));
		}
	}
	if(!u->isLocal() && units.size() > 0 && units[units.size() - 1]->isLocal()) {
		units.insert(units.begin(), u);
	} else {	
		units.push_back(u);
	}
	u->setRegistered(true);
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
		if(it == ufv.end()) {
			break;
		}
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
	if(name_.empty()) return NULL;
	for(int i = 0; i < (int) units.size(); i++) {
		if(units[i]->unitType() != COMPOSITE_UNIT && (units[i]->name() == name_ || units[i]->singular(false) == name_ || units[i]->plural(false) == name_)) {
			return units[i];
		}
	}
	return NULL;
}
Unit* Calculator::getCompositeUnit(string internal_name_) {
	if(internal_name_.empty()) return NULL;
	for(int i = 0; i < (int) units.size(); i++) {
		if(units[i]->unitType() == COMPOSITE_UNIT && units[i]->referenceName() == internal_name_) {
			return units[i];
		}
	}
	return NULL;
}

Variable* Calculator::addVariable(Variable *v, bool force) {
	v->setName(getName(v->name(), v, force));
	if(!v->isLocal() && variables.size() > 0 && variables[variables.size() - 1]->isLocal()) {
		variables.insert(variables.begin(), v);
	} else {
		variables.push_back(v);
	}
	v->setRegistered(true);
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
				l = ((Unit*) (*it))->singular(false).length();
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
				l = ((Unit*) (*it))->singular(false).length();
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
				l = ((Unit*) (*it))->singular(false).length();
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
					l = ((Unit*) (*it))->singular(false).length();
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
	if(!u->singular(false).empty()) {
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
					l = ((Unit*) (*it))->singular(false).length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('u');
				break;
			} else if(l <= u->singular(false).length()) {
				ufv.insert(it, (void*) u);
				ufv_t.insert(ufv_t.begin() + i, 'u');
				break;
			}
			i++;
		}
	}

}
void Calculator::unitSingularChanged(Unit *u) {
	unitNameChanged(u);
}
void Calculator::unitPluralChanged(Unit *u) {
	unitNameChanged(u);
}

Variable* Calculator::getVariable(string name_) {
	if(name_.empty()) return NULL;
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
	f->setRegistered(true);
	f->setChanged(false);
	return f;
}
Function* Calculator::getFunction(string name_) {
	if(name_.empty()) return NULL;
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
	const string *name;
	string stmp, stmp2;
	long int prefix_exp;
	bool b, moved_forward;
	int i, i2, i3, i4, i5, i6, i7, i8, i9;
	int chars_left;
	int name_length, name_length_old;
	Function *f;
	Variable *v;
	Unit *u;
	Prefix *p;
	Manager *mngr;
	for(int i = 0; i < signs.size(); i++) {
		gsub(signs[i], real_signs[i], str);
	}
	for(int str_index = 0; str_index < str.length(); str_index++) {
		chars_left = str.length() - str_index;
		if(str[str_index] == '\"' || str[str_index] == '\'') {
			if(str_index == str.length() - 1) {
				str.erase(str_index, 1);
			} else {
				i = str.find(str[str_index], str_index + 1);
				if(i == string::npos) {
					i = str.length();
					name_length = i - str_index;
				} else {
					name_length = i - str_index + 1;
				}
				stmp = LEFT_BRACKET_CH;
				stmp += ID_WRAP_LEFT_CH;
				mngr = new Manager(str.substr(str_index + 1, i - str_index - 1));
				stmp += i2s(addId(mngr));
				mngr->unref();
				stmp += ID_WRAP_RIGHT_CH;
				stmp += RIGHT_BRACKET_CH;
				str.replace(str_index, name_length, stmp);
				str_index += stmp.length() - 1;
			}
		} else if(str[str_index] == '!') {
			if(str_index != 0 && (chars_left == 1 || str[str_index + 1] != EQUALS_CH) && (f = getFunction("factorial"))) {
				stmp = "";
				if(is_in(NUMBERS, str[str_index - 1])) {
					i3 = str.find_last_not_of(NUMBERS, str_index - 1);
					if(i3 == string::npos) {
						stmp2 = str.substr(0, str_index);
					} else {
						stmp2 = str.substr(i3 + 1, str_index - i3 - 1);
					}
				} else if(str[str_index - 1] == RIGHT_BRACKET_CH) {
					i3 = str_index - 2;
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
								stmp2 = str.substr(i3, str_index - i3);
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
					str.replace(str_index - stmp2.length(), stmp2.length() + 1, stmp);
					str_index -=  stmp2.length();
					str_index += stmp.length() - 1;
				}
			}
		} else if(is_not_in(NUMBERS NOT_IN_NAMES, str[str_index])) {
			for(int ufv_index = 0; ufv_index < ufv.size(); ufv_index++) {
				name = NULL;
				prefix_exp = 0;
				switch(ufv_t[ufv_index]) {
					case 'v': {
						if(b_variables) {
							name = &((ExpressionItem*) ufv[ufv_index])->name();
						}
						break;
					}
					case 'f': {
						if(b_functions) {
							name = &((ExpressionItem*) ufv[ufv_index])->name();
						}
						break;
					}
					case 'U': {
						if(b_units) {
							name = &((ExpressionItem*) ufv[ufv_index])->name();
						}
						break;
					}
					case 'u': {
						if(b_units) {
							name = &((Unit*) ufv[ufv_index])->singular();
						}
						break;
					}
					case 'Y': {
						if(b_units) {
							name = &((Unit*) ufv[ufv_index])->plural();
						}
						break;
					}
					case 'p': {
						if(b_units) {
							name = &((Prefix*) ufv[ufv_index])->shortName();
						}
						break;
					}
					case 'P': {
						if(b_units) {
							name = &((Prefix*) ufv[ufv_index])->longName();
						}
						break;
					}
				}
				if(name) name_length = name->length();
				if(name && (*name)[0] == str[str_index] && (name_length == 1 || (name_length <= chars_left && (*name)[1] == str[str_index + 1] && *name == str.substr(str_index, name_length)))) {
					moved_forward = false;
					switch(ufv_t[ufv_index]) {
						case 'v': {
							v = (Variable*) ufv[ufv_index];
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
							str.replace(str_index, name_length, stmp);
							str_index += stmp.length();
							moved_forward = true;
							break;
						}
						case 'f': {
							f = (Function*) ufv[ufv_index];
							b = false;
							i4 = -1;
							if(f->args() == 0) {
								i5 = str.find_first_not_of(SPACES, str_index + name_length);
								if(i5 != string::npos && str[i5] == LEFT_BRACKET_CH) {
									i5 = str.find_first_not_of(SPACES, i5 + 1);							
									if(i5 != string::npos && str[i5] == RIGHT_BRACKET_CH) {
										i4 = i5 - str_index + 1;
									}
								}
								mngr = f->calculate("");
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
								if(i4 < 0) i4 = name_length;
							} else if(b_rpn && f->args() == 1 && str_index > 0 && str[str_index - 1] == SPACE_CH && (str_index + name_length >= str.length() || str[str_index + name_length] != LEFT_BRACKET_CH) && (i6 = str.find_last_not_of(SPACE, str_index - 1)) != string::npos) {
								i5 = str.rfind(SPACE, i6);	
								if(i5 == string::npos) {
									stmp = str.substr(0, i6 + 1);	
								} else {
									stmp = str.substr(i5 + 1, i6 - i5);
								}
								mngr =  f->calculate(stmp);
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
								if(i5 == string::npos) {
									str.replace(0, str_index + name_length, stmp);
								} else {
									str.replace(i5 + 1, str_index + name_length - i5 - 1, stmp);
								}
								str_index += name_length;
								moved_forward = true;
							} else {
								b = false;
								i5 = 1;
								i6 = 0;
								while(i5 > 0 && !b) {
									if(i6 + str_index + name_length >= str.length()) {
										b = true;
										i5 = 2;
										i6++;
										break;
									} else {
										char c = str[str_index + name_length + i6];
										if(c == LEFT_BRACKET_CH && i5 != 2) {
											b = true;
										} else if(c == ' ') {
											if(i5 == 2) {
												b = true;
											}
										} else if(i5 == 2 && is_in(OPERATORS, str[str_index + name_length + i6])) {
											b = true;
										} else {
											//if(i6 > 0) {
												i5 = 2;
											//} else {
											//	i5 = -1;
											//}		
										}
									}
									i6++;
								}
								if(b && i5 == 2) {
									stmp2 = str.substr(str_index + name_length, i6 - 1);
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
									i4 = i6 + 1 + name_length - 2;
									b = false;
								}
								i9 = i6;
								if(b) {
									b = false;
									i6 = i6 + 1 + str_index + name_length;
									i7 = i6 - 1;
									i8 = i7;
									while(true) {
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
									stmp2 = str.substr(str_index + name_length + i9, i6 - (str_index + name_length + i9));
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
									i4 = i6 + 1 - str_index;
								}
							}
							if(i4 > 0) {
								str.replace(str_index, i4, stmp);
								str_index += stmp.length();
								moved_forward = true;
							}
							break;
						}
						case 'U': {}
						case 'u': {}
						case 'Y': {
							replace_text_by_unit_place:
							u = (Unit*) ufv[ufv_index];
							if(str.length() > str_index + name_length && is_in(NUMBERS, str[str_index + name_length])) {
								str.insert(str_index + name_length, 1, POWER_CH);
							}
							mngr = new Manager(u, prefix_exp);
							stmp = LEFT_BRACKET_CH;					
							stmp += ID_WRAP_LEFT_CH;
							stmp += i2s(addId(mngr));
							mngr->unref();
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_BRACKET_CH;				
							str.replace(str_index, name_length, stmp);
							str_index += stmp.length();
							moved_forward = true;
							break;
						}
						case 'p': {}
						case 'P': {
							if(str_index + name_length == str.length() || is_in(NOT_IN_NAMES, str[str_index + name_length])) {
								break;
							}
							p = (Prefix*) ufv[ufv_index];
							str_index += name_length;
							chars_left = str.length() - str_index;
							name_length_old = name_length;
							for(int ufv_index2 = 0; ufv_index2 < ufv.size(); ufv_index2++) {
								name = NULL;
								switch(ufv_t[ufv_index2]) {
									case 'U': {
										name = &((ExpressionItem*) ufv[ufv_index2])->name();
										break;
									}
									case 'u': {
										name = &((Unit*) ufv[ufv_index2])->singular();
										break;
									}
									case 'Y': {
										name = &((Unit*) ufv[ufv_index2])->plural();
										break;
									}
								}
								if(name) name_length = name->length();
								if(name && (*name)[0] == str[str_index] && (name_length == 1 || (name_length <= chars_left && (*name)[1] == str[str_index + 1] && *name == str.substr(str_index, name_length)))) {
									prefix_exp = p->exponent();
									str.erase(str_index - name_length_old, name_length_old);
									str_index -= name_length_old;
									ufv_index = ufv_index2;
									goto replace_text_by_unit_place;
								}
							}
							str_index -= name_length_old;
							chars_left = str.length() - str_index;
							break;
						}
					}
					if(moved_forward) {
						str_index--;
						break;
					}
				}
			}
			if(!moved_forward) {
				if(b_unknown && str[str_index] != EXP_CH) {
					mngr = new Manager(str.substr(str_index, 1));
					stmp = LEFT_BRACKET_CH;
					stmp += ID_WRAP_LEFT_CH;
					stmp += i2s(addId(mngr));
					mngr->unref();
					stmp += ID_WRAP_RIGHT_CH;
					stmp += RIGHT_BRACKET_CH;
					str.replace(str_index, 1, stmp);
					str_index += stmp.length() - 1;
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
}
/*void Calculator::setFunctionsAndVariables(string &str) {
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
	while(!b) {
		i = str.find("\'", i);
		if(i == string::npos) break;
		i3 = str.find("\'", i + 1);
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
	gsub("\'", "", str);		
	i = -1; i3 = 0; b = false;
	for(int i2 = 0; i2 < ufv.size(); i2++) {
		i = 0, i3 = 0;
		b = false;
		if(ufv_t[i2] == 'v' && b_variables) {
			v = (Variable*) ufv[i2];
			while(true) {
				if((i = str.find(v->name(), i3)) != string::npos) {
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
			while(true) {
				b = false;
				if((i = str.find(f->name(), i3)) != string::npos) {
					i4 = -1;
					if(f->args() == 0) {
						i5 = str.find_first_not_of(SPACES, i + f->name().length());
						if(i5 != string::npos && str[i5] == LEFT_BRACKET_CH) {
							i5 = str.find_first_not_of(SPACES, i5 + 1);							
							if(i5 != string::npos && str[i5] == RIGHT_BRACKET_CH) {
								i4 = i5 - i + 1;
							}
						}
						mngr = f->calculate("");
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
					} else if(b_rpn && f->args() == 1 && i > 0 && str[i - 1] == SPACE_CH && (i + f->name().length() >= str.length() || str[i + f->name().length()] != LEFT_BRACKET_CH) && (i6 = str.find_last_not_of(SPACE, i - 1)) != string::npos) {
						i5 = str.rfind(SPACE, i6);	
						if(i5 == string::npos) {
							stmp = str.substr(0, i6 + 1);	
						} else {
							stmp = str.substr(i5 + 1, i6 - i5);
						}
						mngr =  f->calculate(stmp);
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
						if(i5 == string::npos) {
							str.replace(0, i + f->name().length(), stmp);
						} else {
							str.replace(i5 + 1, i + f->name().length() - i5 - 1, stmp);
						}
					} else {
						b = false;
						i5 = 1;
						i6 = 0;
						while(i5 > 0 && !b) {
							if(i6 + i + f->name().length() >= str.length()) {
								b = true;
								i5 = 2;
								i6++;
								break;
							} else {
								char c = str[i + f->name().length() + i6];
								if(c == LEFT_BRACKET_CH && i5 != 2) {
									b = true;
								} else if(c == ' ') {
									if(i5 == 2) {
										b = true;
									}
								} else if(i5 == 2 && is_in(OPERATORS, str[i + f->name().length() + i6])) {
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
							i8 = i7;*/
/*							i8 = 0;
							while(1) {
								i5 = str.find_first_of(RIGHT_BRACKET LEFT_BRACKET, i7);
								if(i5 == string::npos) {
									for(int index = 0; index < i8; index++) {
										str.append(1, RIGHT_BRACKET_CH);
									}
								}
							}*/
/*							while(1) {
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
			while(true) {
				if((i = str.find(ch, i3)) != string::npos) {
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
									i7 = ((Unit*) ufv[i6])->singular(false).length();
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
										if((ufv_t[i6] == 'u' && str[i4 + i8] != ((Unit*) ufv[i6])->singular(false)[i8 - 1]) || (ufv_t[i6] == 'U' && str[i4 + i8] != ((Unit*) ufv[i6])->name()[i8 - 1]) || (ufv_t[i6] == 'Y' && str[i4 + i8] != ((Unit*) ufv[i6])->plural(false)[i8 - 1])) {
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
				if((ufv_t[i2] == 'u' && (i = str.find(u->singular(), i3)) != (int) string::npos) || (ufv_t[i2] == 'U' && (i = str.find(u->name(), i3)) != (int) string::npos) || (ufv_t[i2] == 'Y' && (i = str.find(u->plural(), i3)) != (int) string::npos)) {
					if(ufv_t[i2] == 'u')
						i4 = i + u->singular().length() - 1;
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
	gsub("!=", "<>", str);	
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
}*/
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
		if(units[i] != object && ((units[i]->unitType() == COMPOSITE_UNIT && units[i]->referenceName() == stmp) || (units[i]->unitType() != COMPOSITE_UNIT && (units[i]->name() == stmp || units[i]->singular(false) == stmp || units[i]->plural(false) == stmp)))) {
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
bool Calculator::loadGlobalDefinitions() {
	string dir = PACKAGE_DATA_DIR;
	string filename;
	dir += "/qalculate/";
	filename = dir;
	filename += "prefixes.xml";
	if(!loadDefinitions(filename.c_str(), false)) {
		return false;
	}	
	filename = dir;
	filename += "functions.xml";
	if(!loadDefinitions(filename.c_str(), false)) {
		return false;
	}
	filename = dir;
	filename += "units.xml";
	if(!loadDefinitions(filename.c_str(), false)) {
		return false;
	}
	filename = dir;
	filename += "variables.xml";
	if(!loadDefinitions(filename.c_str(), false)) {
		return false;
	}
	return true;
}
static int file_selector_nodirs(const struct dirent *file) {
	return strcmp(file->d_name, "..") != 0 && strcmp(file->d_name, ".") != 0;
}
bool Calculator::loadLocalDefinitions() {
	string homedir = "";
	string filename;
	struct passwd *pw = getpwuid(getuid());
	if(pw) {
		homedir = pw->pw_dir;
		homedir += "/";
	}
	homedir += ".qalculate/";
	homedir += "definitions/";	
	struct dirent **eps;
	int n = scandir(homedir.c_str(), &eps, file_selector_nodirs, alphasort);
	for(int i = 0; i < n; i++) {
		filename = homedir;
		filename += eps[i]->d_name;
		loadDefinitions(filename.c_str(), true);
	}
	return true;
}
int Calculator::loadDefinitions(const char* file_name, bool is_user_defs) {
	xmlDocPtr doc;
	xmlNodePtr cur, child, child2, child3;
	vector<xmlNodePtr> unfinished_nodes;
	string version, stmp, name, type, svalue, plural, singular, category, description, title, reverse, base;
	long int exponent, litmp;
	bool precise, active, b;
	Fraction fr;
	Function *f;
	Variable *v;
	Unit *u;
	AliasUnit *au;
	CompositeUnit *cu;
	Prefix *p;
	Argument *arg;
	int itmp;
	IntegerArgument *iarg;
	FractionArgument *farg;	
	Manager *mngr;
	xmlChar *value;
	int in_unfinished = 0;
	bool done_something = false;
	doc = xmlParseFile(file_name);
	if(doc == NULL) {
		return false;
	}
	cur = xmlDocGetRootElement(doc);
	if(cur == NULL) {
		xmlFreeDoc(doc);
		return false;
	}
	while(cur != NULL) {
		if(!xmlStrcmp(cur->name, (const xmlChar*) "QALCULATE")) {
			XML_GET_STRING_FROM_PROP(cur, "version", version)
			break;
		}
		cur = cur->next;
	}
	if(cur == NULL) {
		error(true, "File not identified as Qalculate! definitions file: %s.", file_name, NULL);
		xmlFreeDoc(doc);
		return false;
	}
	bool functions_was = b_functions, variables_was = b_variables, units_was = b_units, unknown_was = b_unknown, calcvars_was = b_calcvars, always_exact_was = b_always_exact, rpn_was = b_rpn;
	b_functions = true; b_variables = true; b_units = true; b_unknown = true; b_calcvars = true; b_always_exact = true; b_rpn = false;
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		if(!xmlStrcmp(cur->name, (const xmlChar*) "activate")) {
			XML_GET_STRING_FROM_TEXT(cur, name)
			ExpressionItem *item = getInactiveExpressionItem(name);
			if(item && !item->isLocal()) {
				item->setActive(true);
				done_something = true;
			}
		} else if(!xmlStrcmp(cur->name, (const xmlChar*) "deactivate")) {
			XML_GET_STRING_FROM_TEXT(cur, name)
			ExpressionItem *item = getActiveExpressionItem(name);
			if(item && !item->isLocal()) {
				item->setActive(false);
				done_something = true;
			}
		} else if(!xmlStrcmp(cur->name, (const xmlChar*) "function")) {
			XML_GET_STRING_FROM_PROP(cur, "name", name)
			if(!name.empty() && functionNameIsValid(name)) {	
				XML_GET_FALSE_FROM_PROP(cur, "active", active)
				f = addFunction(new UserFunction("", name, "", is_user_defs, 0, "", "", 0, active));
				done_something = true;
				child = cur->xmlChildrenNode;
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "expression")) {
						XML_DO_FROM_TEXT(child, ((UserFunction*) f)->setEquation);
						XML_GET_FALSE_FROM_PROP(child, "precise", b)
						f->setPrecise(b);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {	
						XML_DO_FROM_TEXT(child, f->setCategory);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
						XML_DO_FROM_TEXT(child, f->setTitle);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "condition")) {
						XML_DO_FROM_TEXT(child, f->setCondition);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
						XML_DO_FROM_TEXT(child, f->setDescription);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "argument")) {
						farg = NULL; iarg = NULL;
						XML_GET_STRING_FROM_PROP(child, "type", type);
						if(type == "text") {
							arg = new TextArgument();
						} else if(type == "date") {
							arg = new DateArgument();
						} else if(type == "integer") {
							iarg = new IntegerArgument();
							arg = iarg;
						} else if(type == "number") {
							farg = new FractionArgument();
							arg = farg;
						} else if(type == "vector") {
							arg = new VectorArgument();
						} else if(type == "matrix") {
							arg = new MatrixArgument();
						} else if(type == "boolean") {
							arg = new BooleanArgument();
						} else if(type == "function") {
							arg = new FunctionArgument();
						} else if(type == "unit") {
							arg = new UnitArgument();
						} else if(type == "variable") {
							arg = new VariableArgument();
						} else if(type == "object") {
							arg = new ExpressionItemArgument();
						} else {
							arg = new Argument();
						}
						child2 = child->xmlChildrenNode;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
								XML_DO_FROM_TEXT(child2, arg->setName);
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "min")) {
								if(farg) {
									XML_DO_FROM_TEXT(child2, fr.set);
									farg->setMin(&fr);
									XML_GET_FALSE_FROM_PROP(child, "include_equals", b)
									farg->setIncludeEqualsMin(b);
								} else if(iarg) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);
									Integer integ(stmp);
									iarg->setMin(&integ);
								}
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "max")) {
								if(farg) {
									XML_DO_FROM_TEXT(child2, fr.set);
									farg->setMax(&fr);
									XML_GET_FALSE_FROM_PROP(child, "include_equals", b)
									farg->setIncludeEqualsMax(b);
								} else if(iarg) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);
									Integer integ(stmp);
									iarg->setMax(&integ);
								}
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "condition")) {
								XML_DO_FROM_TEXT(child2, arg->setCustomCondition);	
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "matrix_allowed")) {
								XML_GET_TRUE_FROM_TEXT(child2, b);
								arg->setMatrixAllowed(b);
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "zero_forbidden")) {
								XML_GET_TRUE_FROM_TEXT(child2, b);
								arg->setZeroForbidden(b);
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "test")) {
								XML_GET_FALSE_FROM_TEXT(child2, b);
								arg->setTests(b);
							}
							child2 = child2->next;
						}						
						itmp = 1;
						XML_GET_INT_FROM_PROP(child, "index", itmp);
						f->setArgumentDefinition(itmp, arg); 
					}
					child = child->next;
				}
				f->setChanged(false);
			}
		} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_function")) {
			XML_GET_STRING_FROM_PROP(cur, "name", name)
			f = getFunction(name);
			if(f) {	
				XML_GET_FALSE_FROM_PROP(cur, "active", active)
				f->setLocal(is_user_defs, active);
				child = cur->xmlChildrenNode;
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
						XML_DO_FROM_TEXT(child, f->setDescription);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {	
						XML_DO_FROM_TEXT(child, f->setCategory);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
						XML_DO_FROM_TEXT(child, f->setTitle);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "argument")) {
						name = "";
						child2 = child->xmlChildrenNode;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
								XML_GET_STRING_FROM_TEXT(child2, name);
							}
							child2 = child2->next;
						}
						itmp = 1;
						XML_GET_INT_FROM_PROP(child, "index", itmp);
						if(f->getArgumentDefinition(itmp)) {
							f->getArgumentDefinition(itmp)->setName(name);
						} else {
							f->setArgumentDefinition(itmp, new Argument(name, false));
						}
					}
					child = child->next;
				}
				f->setChanged(false);
				done_something = true;
			}
		} else if(!xmlStrcmp(cur->name, (const xmlChar*) "variable")) {
			XML_GET_STRING_FROM_PROP(cur, "name", name)
			if(!name.empty() && variableNameIsValid(name)) {	
				XML_GET_FALSE_FROM_PROP(cur, "active", active)
				svalue = "";
				v = addVariable(new Variable("", name, "", "", is_user_defs, false, active));
				done_something = true;
				child = cur->xmlChildrenNode;
				b = true;
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "value")) {
						XML_DO_FROM_TEXT(child, v->set);
						XML_GET_FALSE_FROM_PROP(child, "precise", b);
						v->setPrecise(b);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
						XML_DO_FROM_TEXT(child, v->setDescription);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {	
						XML_DO_FROM_TEXT(child, v->setCategory);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
						XML_DO_FROM_TEXT(child, v->setTitle);
					}
					child = child->next;
				}
				v->setChanged(false);
			}
		} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_variable")) {
			XML_GET_STRING_FROM_PROP(cur, "name", name)
			v = getVariable(name);
			if(v) {	
				XML_GET_FALSE_FROM_PROP(cur, "active", active)
				v->setLocal(is_user_defs, active);
				child = cur->xmlChildrenNode;
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
						XML_DO_FROM_TEXT(child, v->setDescription);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {	
						XML_DO_FROM_TEXT(child, v->setCategory);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
						XML_DO_FROM_TEXT(child, v->setTitle);
					}
					child = child->next;
				}				
				v->setChanged(false);
				done_something = true;
			}
		} else if(!xmlStrcmp(cur->name, (const xmlChar*) "unit")) {
			XML_GET_STRING_FROM_PROP(cur, "type", type)
			if(type == "base") {	
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				XML_GET_FALSE_FROM_PROP(cur, "active", active)
				description = ""; category = ""; title = ""; singular = ""; plural = "";
				if(unitNameIsValid(name)) {
					child = cur->xmlChildrenNode;
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_STRING_FROM_TEXT(child, description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {	
							XML_GET_STRING_FROM_TEXT(child, category);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_STRING_FROM_TEXT(child, title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "singular")) {	
							XML_GET_STRING_FROM_TEXT(child, singular);
							if(!unitNameIsValid(singular)) {
								singular = "";
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "plural")) {	
							XML_GET_STRING_FROM_TEXT(child, plural);
							if(!unitNameIsValid(plural)) {
								plural = "";
							}
						}
						child = child->next;
					}		
					u = addUnit(new Unit(category, name, plural, singular, title, is_user_defs, false, active));		
					u->setDescription(description);
					u->setChanged(false);
					done_something = true;
				}
			} else if(type == "alias") {	
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				XML_GET_FALSE_FROM_PROP(cur, "active", active)
				u = NULL;
				child = cur->xmlChildrenNode;
				description = ""; category = ""; title = ""; singular = ""; plural = "";
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "base")) {
						child2 = child->xmlChildrenNode;
						exponent = 1;
						svalue = "";
						reverse = "";
						b = true;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "unit")) {
								XML_GET_STRING_FROM_TEXT(child2, base);
								u = getUnit(base);
								if(!u) {
									u = getCompositeUnit(base);
								}
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "relation")) {
								XML_GET_STRING_FROM_TEXT(child2, svalue);
								XML_GET_FALSE_FROM_PROP(child2, "precise", b)
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "reverse_relation")) {
								XML_GET_STRING_FROM_TEXT(child2, reverse);
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "exponent")) {
								XML_GET_STRING_FROM_TEXT(child2, stmp);
								if(stmp.empty()) {
									exponent = 1;
								} else {
									exponent = s2li(stmp);
								}
							}
							child2 = child2->next;
						}
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
						XML_GET_STRING_FROM_TEXT(child, description);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {	
						XML_GET_STRING_FROM_TEXT(child, category);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
						XML_GET_STRING_FROM_TEXT(child, title);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "singular")) {	
						XML_GET_STRING_FROM_TEXT(child, singular);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "plural")) {	
						XML_GET_STRING_FROM_TEXT(child, plural);
					}
					child = child->next;
				}
				if(!u) {
					if(!in_unfinished) {
						unfinished_nodes.push_back(cur);
					}
				} else if(unitNameIsValid(name) && unitNameIsValid(plural) && unitNameIsValid(singular)) {
					au = new AliasUnit(category, name, plural, singular, title, u, svalue, exponent, reverse, is_user_defs, false, active);
					au->setDescription(description);
					au->setPrecise(b);
					addUnit(au);
					au->setChanged(false);
					done_something = true;
				}
			} else if(type == "composite") {	
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				if(unitNameIsValid(name)) {
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					child = cur->xmlChildrenNode;
					cu = NULL;
					description = ""; category = ""; title = "";
					while(child != NULL) {
						u = NULL;
						if(!xmlStrcmp(child->name, (const xmlChar*) "part")) {
							child2 = child->xmlChildrenNode;
							p = NULL;
							exponent = 1;							
							while(child2 != NULL) {
								if(!xmlStrcmp(child2->name, (const xmlChar*) "unit")) {
									XML_GET_STRING_FROM_TEXT(child2, base);
									u = getUnit(base);
									if(!u) {
										u = getCompositeUnit(base);
									}
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "prefix")) {
									XML_GET_STRING_FROM_TEXT(child2, svalue);
									litmp = s2li(svalue);
									if(litmp == 0) {
										p = NULL;
									} else {
										p = getExactPrefix(litmp);
										if(!p) {
											if(cu) {
												delete cu;
											}
											cu = NULL;
											break;
										}												
									}
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "exponent")) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);
									if(stmp.empty()) {
										exponent = 1;
									} else {
										exponent = s2li(stmp);
									}
								}
								child2 = child2->next;
							}		
							if(u) {
								if(!cu) {
									cu = new CompositeUnit("", name, "", "", is_user_defs, false, active);
								}
								cu->add(u, exponent, p);
							} else {
								if(cu) delete cu;
								cu = NULL;
								if(!in_unfinished) {
									unfinished_nodes.push_back(cur);
								}
								break;
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_STRING_FROM_TEXT(child, description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {	
							XML_GET_STRING_FROM_TEXT(child, category);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_STRING_FROM_TEXT(child, title);
						}
						child = child->next;
					}
					if(cu) {
						cu->setCategory(category);
						cu->setTitle(title);
						cu->setDescription(description);
						addUnit(cu);
						cu->setChanged(false);
						done_something = true;
					}
				}
			}
		} else if(!xmlStrcmp(cur->name, (const xmlChar*) "prefix")) {
			child = cur->xmlChildrenNode;
			while(child != NULL) {
				if(!xmlStrcmp(child->name, (const xmlChar*) "name")) {
					XML_GET_STRING_FROM_TEXT(child, name);
				} else if(!xmlStrcmp(child->name, (const xmlChar*) "abbreviation")) {	
					XML_GET_STRING_FROM_TEXT(child, stmp);
				} else if(!xmlStrcmp(child->name, (const xmlChar*) "exponent")) {	
					XML_GET_STRING_FROM_TEXT(child, svalue);
				}
				child = child->next;
			}
			addPrefix(new Prefix(s2li(svalue), name, stmp));
			done_something = true;
		}
		if(in_unfinished) {
			cur = NULL;
			if(done_something) {
				in_unfinished--;
				unfinished_nodes.erase(unfinished_nodes.begin() + in_unfinished);
			}
			if(unfinished_nodes.size() > in_unfinished) {
				cur = unfinished_nodes[in_unfinished];
			} else if(done_something && unfinished_nodes.size() > 0) {
				cur = unfinished_nodes[0];
				in_unfinished = 0;
				done_something = false;
			}
			in_unfinished++;
		} else {
			cur = cur->next;
		}
		if(cur == NULL && !in_unfinished && unfinished_nodes.size() > 0) {
			cur = unfinished_nodes[0];
			in_unfinished = 1;
			done_something = false;
		}
	}
	b_functions = functions_was; b_variables = variables_was; b_units = units_was; b_unknown = unknown_was; b_calcvars = calcvars_was; b_always_exact = always_exact_was; b_rpn = rpn_was;
	xmlFreeDoc(doc);
	return true;
}
bool Calculator::saveDefinitions() {
	string homedir = "";
	string filename;
	struct passwd *pw = getpwuid(getuid());
	if(pw) {
		homedir = pw->pw_dir;
		homedir += "/";
	}
	homedir += ".qalculate/";
	mkdir(homedir.c_str(), S_IRWXU);	
	homedir += "definitions/";	
	mkdir(homedir.c_str(), S_IRWXU);
	filename = homedir;
	filename += "functions.xml";
	if(!saveFunctions(filename.c_str())) {
		return false;
	}
	filename = homedir;
	filename += "units.xml";
	if(!saveUnits(filename.c_str())) {
		return false;
	}
	filename = homedir;
	filename += "variables.xml";
	if(!saveVariables(filename.c_str())) {
		return false;
	}
	return true;
}

int Calculator::savePrefixes(const char* file_name, bool save_global) {
	if(!save_global) {
		return true;
	}
	unsetLocale();	
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");	
	xmlNodePtr cur, newnode, newnode2;	
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);	
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	cur = doc->children;
	for(int i = 0; i < prefixes.size(); i++) {
		newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "prefix", NULL);
		xmlNewTextChild(newnode, NULL, (xmlChar*) "name", (xmlChar*) prefixes[i]->longName(false).c_str());
		xmlNewTextChild(newnode, NULL, (xmlChar*) "abbreviation", (xmlChar*) prefixes[i]->shortName(false).c_str());
		xmlNewTextChild(newnode, NULL, (xmlChar*) "exponent", (xmlChar*) li2s(prefixes[i]->exponent()).c_str());
	}	
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);
	setLocale();
	return returnvalue;
}

int Calculator::saveVariables(const char* file_name, bool save_global) {
	unsetLocale();	
	string str;
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");	
	xmlNodePtr cur, newnode, newnode2;	
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);	
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	cur = doc->children;
	bool was_always_exact = alwaysExact();
	setAlwaysExact(true);
	if(!save_global) {		
		for(int i = 0; i < variables.size(); i++) {
			if(!variables[i]->isLocal() && variables[i]->hasChanged()) {
				if(variables[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) variables[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) variables[i]->referenceName().c_str());
				}
			}
		}
	}	
	for(int i = 0; i < variables.size(); i++) {
		if((save_global || variables[i]->isLocal()) && variables[i]->category() != _("Temporary")) {
			if(variables[i]->isBuiltin()) {
				newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_variable", NULL);
			} else {
				newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "variable", NULL);
			}
			if(variables[i]->isBuiltin()) {
				xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) variables[i]->referenceName().c_str());
			} else {
				xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) variables[i]->name().c_str());
			}
			if(!variables[i]->category().empty()) {
				if(save_global) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "_category", (xmlChar*) variables[i]->category().c_str());
				} else {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "category", (xmlChar*) variables[i]->category().c_str());
				}
			}
			if(!variables[i]->title(false).empty()) {
				if(save_global) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) variables[i]->title(false).c_str());
				} else {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) variables[i]->title(false).c_str());
				}
			}
			if(!variables[i]->description().empty()) {
				str = variables[i]->description();
				if(save_global) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
				} else {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
				}
			}
			if(variables[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "true");
			else xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
			if(!variables[i]->isBuiltin()) {
				newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) variables[i]->get()->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTIONAL_ONLY).c_str());
				if(variables[i]->isPrecise()) xmlNewProp(newnode2, (xmlChar*) "precise", (xmlChar*) "true");
				else xmlNewProp(newnode2, (xmlChar*) "precise", (xmlChar*) "false");					
			}
		}	
	}
	setAlwaysExact(was_always_exact);
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);
	setLocale();
	return returnvalue;
}

int Calculator::saveUnits(const char* file_name, bool save_global) {
	unsetLocale();	
	string str;
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");	
	xmlNodePtr cur, newnode, newnode2, newnode3;	
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);	
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	CompositeUnit *cu;
	AliasUnit *au;
	Unit *u;
	cur = doc->children;
	if(!save_global) {
		for(int i = 0; i < units.size(); i++) {
			if(!units[i]->isLocal() && units[i]->hasChanged()) {
				if(units[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) units[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) units[i]->referenceName().c_str());
				}
			}
		}
	}
	for(int i = 0; i < units.size(); i++) {
		if(save_global || units[i]->isLocal()) {
			newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "unit", NULL);
			switch(units[i]->unitType()) {
				case BASE_UNIT: {
					xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "base");
					break;
				}
				case ALIAS_UNIT: {
					au = (AliasUnit*) units[i];
					xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "alias");
					break;
				}
				case COMPOSITE_UNIT: {
					cu = (CompositeUnit*) units[i];
					xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "composite");
					break;
				}
			}		
			if(units[i]->unitType() == COMPOSITE_UNIT) {
				xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) cu->referenceName().c_str());
				for(int i2 = 0; i2 < cu->units.size(); i2++) {
					newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "part", NULL);
					xmlNewTextChild(newnode2, NULL, (xmlChar*) "unit", (xmlChar*) cu->units[i2]->firstBaseUnit()->referenceName().c_str());
					xmlNewTextChild(newnode2, NULL, (xmlChar*) "prefix", (xmlChar*) li2s(cu->units[i2]->prefixExponent()).c_str());
					xmlNewTextChild(newnode2, NULL, (xmlChar*) "exponent", (xmlChar*) li2s(cu->units[i2]->firstBaseExp()).c_str());
				}
			} else {
				xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) units[i]->referenceName().c_str());
				if(!units[i]->singular(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_singular", (xmlChar*) units[i]->singular(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "singular", (xmlChar*) units[i]->singular(false).c_str());
					}
				}
				if(!units[i]->plural(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_plural", (xmlChar*) units[i]->plural(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "plural", (xmlChar*) units[i]->plural(false).c_str());
					}
				}
			}
			if(units[i]->unitType() == ALIAS_UNIT) {
				newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "base", NULL);
				xmlNewTextChild(newnode2, NULL, (xmlChar*) "unit", (xmlChar*) au->firstBaseUnit()->referenceName().c_str());								
				newnode3 = xmlNewTextChild(newnode2, NULL, (xmlChar*) "relation", (xmlChar*) au->expression().c_str());
				if(units[i]->isPrecise()) xmlNewProp(newnode3, (xmlChar*) "precise", (xmlChar*) "true");
				else xmlNewProp(newnode3, (xmlChar*) "precise", (xmlChar*) "false");				
				if(!au->reverseExpression().empty()) {
					xmlNewTextChild(newnode2, NULL, (xmlChar*) "reverse_relation", (xmlChar*) au->reverseExpression().c_str());	
				}
				xmlNewTextChild(newnode2, NULL, (xmlChar*) "exponent", (xmlChar*) li2s(au->firstBaseExp()).c_str());
			}
			if(!units[i]->category().empty()) {
				if(save_global) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "_category", (xmlChar*) units[i]->category().c_str());
				} else {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "category", (xmlChar*) units[i]->category().c_str());
				}
			}
			if(!units[i]->title(false).empty()) {
				if(save_global) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) units[i]->title(false).c_str());
				} else {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) units[i]->title(false).c_str());
				}
			}
			if(!units[i]->description().empty()) {
				str = units[i]->description();
				if(save_global) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
				} else {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
				}
			}
			if(units[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "true");
			else xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
		}
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);
	setLocale();
	return returnvalue;
}

int Calculator::saveFunctions(const char* file_name, bool save_global) {
	unsetLocale();	
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");	
	xmlNodePtr cur, newnode, newnode2;	
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);	
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	Argument *arg;
	IntegerArgument *iarg;
	FractionArgument *farg;
	string str;
	cur = doc->children;
	if(!save_global) {
		for(int i = 0; i < functions.size(); i++) {
			if(!functions[i]->isLocal() && functions[i]->hasChanged()) {
				if(functions[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) functions[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) functions[i]->referenceName().c_str());
				}
			}
		}
	}
	for(int i = 0; i < functions.size(); i++) {
		cur = doc->children;
		if(save_global || functions[i]->isLocal()) {	
			if(functions[i]->isBuiltin()) {
				newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_function", NULL);
				xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) functions[i]->referenceName().c_str());
				if(functions[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "true");
				else xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");			
				if(!functions[i]->category().empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_category", (xmlChar*) functions[i]->category().c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "category", (xmlChar*) functions[i]->category().c_str());
					}
				}
				if(!functions[i]->title(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) functions[i]->title(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) functions[i]->title(false).c_str());
					}
				}
				if(!functions[i]->description().empty()) {
					str = functions[i]->description();
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
					}
				}
				cur = newnode;
				for(int i2 = 1; i2 <= functions[i]->lastArgumentDefinitionIndex(); i2++) {
					arg = functions[i]->getArgumentDefinition(i2);
					if(arg && !arg->name().empty()) {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "argument", NULL);
						if(save_global) {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) arg->name().c_str());
						} else {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) arg->name().c_str());
						}
						xmlNewProp(newnode, (xmlChar*) "index", (xmlChar*) i2s(i2).c_str());
					}
				}
			} else {
				newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "function", NULL);
				xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) functions[i]->name().c_str());
				if(functions[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "true");
				else xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
				if(!functions[i]->category().empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_category", (xmlChar*) functions[i]->category().c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "category", (xmlChar*) functions[i]->category().c_str());
					}
				}
				if(!functions[i]->title(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) functions[i]->title(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) functions[i]->title(false).c_str());
					}
				}
				if(!functions[i]->description().empty()) {
					str = functions[i]->description();
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
					}
				}
				newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "expression", (xmlChar*) ((UserFunction*) functions[i])->equation().c_str());
				if(functions[i]->isActive()) xmlNewProp(newnode2, (xmlChar*) "precise", (xmlChar*) "true");
				else xmlNewProp(newnode2, (xmlChar*) "precise", (xmlChar*) "false");			
				if(!functions[i]->condition().empty()) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "condition", (xmlChar*) functions[i]->condition().c_str());
				}
				cur = newnode;
				for(int i2 = 1; i2 <= functions[i]->lastArgumentDefinitionIndex(); i2++) {
					arg = functions[i]->getArgumentDefinition(i2);
					if(arg) {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "argument", NULL);
						if(!arg->name().empty()) {
							if(save_global) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) arg->name().c_str());
							} else {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) arg->name().c_str());
							}
						}
						switch(arg->type()) {
							case ARGUMENT_TYPE_TEXT: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "text"); break;}
							case ARGUMENT_TYPE_DATE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "date"); break;}
							case ARGUMENT_TYPE_INTEGER: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "integer"); break;}
							case ARGUMENT_TYPE_FRACTION: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "number"); break;}
							case ARGUMENT_TYPE_VECTOR: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "vector"); break;}
							case ARGUMENT_TYPE_MATRIX: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "matrix"); break;}
							case ARGUMENT_TYPE_BOOLEAN: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "boolean"); break;}
							case ARGUMENT_TYPE_FUNCTION: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "function"); break;}
							case ARGUMENT_TYPE_UNIT: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "unit"); break;}
							case ARGUMENT_TYPE_VARIABLE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "variable"); break;}
							case ARGUMENT_TYPE_EXPRESSION_ITEM: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "object"); break;}
							default: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "free");}
						}
						xmlNewProp(newnode, (xmlChar*) "index", (xmlChar*) i2s(i2).c_str());
						if(!arg->tests()) {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "test", (xmlChar*) "false");
						}
						if(arg->zeroForbidden()) {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "zero_forbidden", (xmlChar*) "true");
						}
						if(arg->matrixAllowed()) {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "matrix_allowed", (xmlChar*) "true");
						}
						switch(arg->type()) {
							case ARGUMENT_TYPE_INTEGER: {
								iarg = (IntegerArgument*) arg;
								if(iarg->min()) {
									xmlNewTextChild(newnode, NULL, (xmlChar*) "min", (xmlChar*) iarg->min()->print().c_str()); 
								}
								if(iarg->max()) {
									xmlNewTextChild(newnode, NULL, (xmlChar*) "max", (xmlChar*) iarg->max()->print().c_str()); 
								}
								break;
							}
							case ARGUMENT_TYPE_FRACTION: {
								farg = (FractionArgument*) arg;
								if(farg->min()) {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "min", (xmlChar*) farg->min()->print().c_str()); 
									if(farg->includeEqualsMin()) {
										xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "true");
									} else {
										xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "false");
									}
								}
								if(farg->max()) {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "max", (xmlChar*) farg->max()->print().c_str()); 
									if(farg->includeEqualsMax()) {
										xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "true");
									} else {
										xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "false");
									}
								}
								break;						
							}
						}					
						if(!arg->getCustomCondition().empty()) {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "condition", (xmlChar*) arg->getCustomCondition().c_str());
						}
					}
				}
			}
		}
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);
	setLocale();
	return returnvalue;
}
/*bool Calculator::save(const char* file_name) {
	FILE *file = fopen(file_name, "w+");
	if(file == NULL)
		return false;
	string str;
	unsetLocale();
	fprintf(file, "##########################################\n");
	fprintf(file, "#       QALCULATE! DEFINITIONS FILE      #\n");
	fprintf(file, "##########################################\n");
	fprintf(file, "\n");
	fprintf(file, "*FileFormat\tQalculate! Definitions\n");
	fprintf(file, "*Version\t0.3");
	fprintf(file, "\n");
	Argument *arg;
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
			//print arguments
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
}*/
/*bool Calculator::load(const char* file_name, bool is_user_defs) {
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
	Argument *arg;
	bool b, b2;
	bool unit_added = false, units_added = false, rerun = false;
	bool b_qalculate = false, b_version = false;
	int rerun_i = 0;
	vector<string> unfinished_units;
	unsigned int i, i2, i3, i4;
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
				if(!b_qalculate && str == "*FileFormat") {	
					FIRST_READ_TAB_DELIMITED(ntmp)
						remove_blank_ends(ntmp);
						if(ntmp != "Qalculate! Definitions") {
							break;
						}
						b_qalculate = true;
					}
				} else if(!b_version && b_qalculate && str == "*Version") {				
					FIRST_READ_TAB_DELIMITED(ntmp)
						remove_blank_ends(ntmp);
						b_version = true;						
						if(ntmp != "0.3") {
							error(true, "Unknown definitions file version in %s. Will not load.", file_name, NULL);
							break;
						}
					}				
				} else if(b_version && b_qalculate)
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
									READ_TAB_DELIMITED(vtmp)
										func = addFunction(new UserFunction(ctmp, ntmp, vtmp, is_user_defs, -1, "", "", 0, b2));
										READ_TAB_DELIMITED_SET_BOOL(b)
											func->setPrecise(b);	
											READ_TAB_DELIMITED_SET_0(shtmp)
												func->setTitle(shtmp);
												READ_TAB_DELIMITED_SET_0(shtmp)
													gsub("\\", "\n", shtmp);
													func->setDescription(shtmp);
													i4 = 1;
													while(true) {
														READ_TAB_DELIMITED_SET_0(vtmp)
															READ_TAB_DELIMITED_SET_0(ntmp)
																if(vtmp.empty()) {
																	if(!ntmp.empty()) {
																		func->setArgumentDefinition(i4, new Argument(ntmp));
																	}
																} else if(vtmp == "T") {
																	func->setArgumentDefinition(i4, new TextArgument(ntmp));
																} else if(vtmp == "D") {
																	func->setArgumentDefinition(i4, new DateArgument(ntmp));
																} else if(vtmp == "P") {
																	func->setArgumentDefinition(i4, new FractionArgument(ntmp, ARGUMENT_MIN_MAX_POSITIVE));
																} else if(vtmp == "!N") {
																	func->setArgumentDefinition(i4, new FractionArgument(ntmp, ARGUMENT_MIN_MAX_NONNEGATIVE));
																} else if(vtmp == "!0") {
																	func->setArgumentDefinition(i4, new FractionArgument(ntmp, ARGUMENT_MIN_MAX_NONZERO));
																} else if(vtmp == "I") {
																	func->setArgumentDefinition(i4, new IntegerArgument(ntmp));
																} else if(vtmp == "PI") {
																	func->setArgumentDefinition(i4, new IntegerArgument(ntmp, ARGUMENT_MIN_MAX_POSITIVE));
																} else if(vtmp == "!NI") {
																	func->setArgumentDefinition(i4, new IntegerArgument(ntmp, ARGUMENT_MIN_MAX_NONNEGATIVE));
																} else if(vtmp == "!0I") {
																	func->setArgumentDefinition(i4, new IntegerArgument(ntmp, ARGUMENT_MIN_MAX_NONZERO));
																} else if(vtmp == "Fr") {
																	func->setArgumentDefinition(i4, new FractionArgument(ntmp));
																} else if(vtmp == "V") {
																	func->setArgumentDefinition(i4, new VectorArgument(ntmp, false, true));
																} else if(vtmp == "M") {
																	func->setArgumentDefinition(i4, new MatrixArgument(ntmp));
																} else if(vtmp == "F") {
																	func->setArgumentDefinition(i4, new FunctionArgument(ntmp));
																} else if(vtmp == "U") {
																	func->setArgumentDefinition(i4, new UnitArgument(ntmp));
																} else if(vtmp == "B") {
																	func->setArgumentDefinition(i4, new BooleanArgument(ntmp));
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
										int i4 = 1;										
										while(func) {
											READ_TAB_DELIMITED_SET_0(vtmp)
												if(!vtmp.empty()) {
													if(func->getArgumentDefinition(i4)) {
														func->getArgumentDefinition(i4)->setName(vtmp);
													} else {
														func->setArgumentDefinition(i4, new Argument(vtmp, false));
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
	if(!b_qalculate) {
		error(true, "File not identified as Qalculate! definitions file: %s.", file_name, NULL);
		return false;
	} else if(!b_version) {
		error(true, "No version information present definitions file: %s. No definitions loaded.", file_name, NULL);
		return false;
	}	
	return true;
}*/

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
	    		mngr->add(&mngr_pi, OPERATION_MULTIPLY);
	    		mngr->addFloat(180, OPERATION_DIVIDE);			
			break;
		}
		case GRADIANS: {
			Fraction fr;
			fr.pi();
			Manager mngr_pi(&fr);
	    		mngr->add(&mngr_pi, OPERATION_MULTIPLY);			
	    		mngr->addFloat(200, OPERATION_DIVIDE);		
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
int Calculator::testCondition(string expression) {
	Manager *mngr = calculate(expression);
	if(mngr->isFraction()) {
		if(mngr->fraction()->isPositive()) {
			mngr->unref();
			return 1;
		} else {
			mngr->unref();
			return 0;
		}
	}
	mngr->unref();
	return -1;
}


