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

#ifndef HAVE_LIBCLN
#include <fenv.h>
#endif

#include <locale.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <wait.h>
#include <queue>
#include <stack>

#ifdef HAVE_LIBCLN
#define WANT_OBFUSCATING_OPERATORS
#include <cln/cln.h>
using namespace cln;
#endif

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
#define XML_GET_LOCALE_STRING_FROM_TEXT(node, str, best, next_best)		value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); lang = xmlNodeGetLang(node); if(!best) {if(!lang) {if(!next_best) {if(value) str = (char*) value; else str = ""; if(locale.empty()) {best = true;}}} else {lang_tmp = (char*) lang; if(lang_tmp == locale) {best = true; if(value) str = (char*) value; else str = "";} else if(!next_best && lang_tmp.length() >= 2 && lang_tmp.substr(0, 2) == localebase) {next_best = true; if(value) str = (char*) value; else str = "";} else if(!next_best && str.empty() && value) {str = (char*) value;}}} if(value) xmlFree(value); if(lang) xmlFree(lang) 

plot_parameters::plot_parameters() {
	auto_y_min = true;
	auto_x_min = true;
	auto_y_max = true;
	auto_x_max = true;
	y_log = false;
	x_log = false;
	y_log_base = 10;
	x_log_base = 10;
	grid = false;
	color = true;
	linewidth = -1;
	show_all_borders = false;
	legend_placement = PLOT_LEGEND_TOP_RIGHT;
}
plot_data_parameters::plot_data_parameters() {
	yaxis2 = false;
	xaxis2 = false;
}
void Calculator::addStringAlternative(string replacement, string standard) {
	signs.push_back(replacement);
	real_signs.push_back(standard);
}
bool Calculator::delStringAlternative(string replacement, string standard) {
	for(int i = 0; i < signs.size(); i++) {
		if(signs[i] == replacement && real_signs[i] == standard) {
			signs.erase(signs.begin() + i);
			real_signs.erase(real_signs.begin() + i);
			return true;
		}
	}
}
void Calculator::addDefaultStringAlternative(string replacement, string standard) {
	default_signs.push_back(replacement);
	default_real_signs.push_back(standard);
}
bool Calculator::delDefaultStringAlternative(string replacement, string standard) {
	for(int i = 0; i < default_signs.size(); i++) {
		if(default_signs[i] == replacement && default_real_signs[i] == standard) {
			default_signs.erase(default_signs.begin() + i);
			default_real_signs.erase(default_real_signs.begin() + i);
			return true;
		}
	}
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
	addStringAlternative(SIGN_POWER_0 "C", "oC");
	addStringAlternative(SIGN_POWER_0 "F", "oF");
	addStringAlternative(SIGN_POWER_0 "R", "oR");
	addStringAlternative(SIGN_POWER_0, "deg");
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
	addStringAlternative("[", LEFT_PARENTHESIS);	
	addStringAlternative("]", RIGHT_PARENTHESIS);	
	addStringAlternative(";", COMMA);	
	addStringAlternative("\t", SPACE);	
	addStringAlternative("\n", SPACE);	


	NAME_NUMBER_PRE_S = "_~#";
	NAME_NUMBER_PRE_STR = "_";

	saved_locale = strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	ILLEGAL_IN_NAMES = DOT_S + RESERVED OPERATORS SPACES PARENTHESISS;
	ILLEGAL_IN_NAMES_MINUS_SPACE_STR = DOT_S + RESERVED OPERATORS PARENTHESISS;	
	ILLEGAL_IN_UNITNAMES = ILLEGAL_IN_NAMES + NUMBERS;			
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
	b_use_all_prefixes = false;
	b_multiple_roots = true;
	disable_errors_ref = 0;
	b_busy = false;
	b_gnuplot_open = false;
	gnuplot_pipe = NULL;
	pthread_attr_init(&calculate_thread_attr);	    	
}
Calculator::~Calculator() {
	closeGnuplot();
}

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
bool Calculator::multipleRootsEnabled() const {
	return b_multiple_roots;
}
void Calculator::setMultipleRootsEnabled(bool enable_multiple_roots) {
	b_multiple_roots = enable_multiple_roots;
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
	int prev_i = 0;
	for(int i = 0; i < prefixes.size(); i++) {
		if(b_use_all_prefixes || prefixes[i]->exponent() % 3 == 0) {
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
			prev_i = i;
		}
	}
	return prefixes[prev_i];
}
Prefix *Calculator::getBestPrefix(const Integer *exp10, const Integer *exp) const {
	if(prefixes.size() <= 0) return NULL;
	Integer tmp_exp;
	int prev_i = 0;
	for(int i = 0; i < prefixes.size(); i++) {
		if(b_use_all_prefixes || prefixes[i]->exponent() % 3 == 0) {
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
					return prefixes[prev_i];
				} else {
					return prefixes[i];
				}
			}
			prev_i = i;
		}
	}
	return prefixes[prev_i];
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
//			} else if(l < p->longName(false).length() || (l == p->longName(false).length() && ufv_t[i] != 'u' && ufv_t[i] != 'U' && ufv_t[i] != 'Y')) {
			} else if(l <= p->longName(false).length()) {			
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
//			} else if(l < p->shortName(false).length() || (l == p->shortName(false).length() && ufv_t[i] != 'u' && ufv_t[i] != 'U' && ufv_t[i] != 'Y')) {
			} else if(l <= p->shortName(false).length()) {
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
	if(precision < 10) {
		cln::default_float_format = float_format(precision + (10 - precision) + 5);	
	} else {
		cln::default_float_format = float_format(precision + 5);	
	}
#endif
	i_precision = precision;
}
int Calculator::getPrecision() const {
	return i_precision;
}

const char *Calculator::getDecimalPoint() const {return DOT_STR.c_str();}
const char *Calculator::getComma() const {return COMMA_STR.c_str();}	
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
		delStringAlternative(",", DOT);	
	}
	setlocale(LC_NUMERIC, "C");
}
void Calculator::unsetLocale() {
	COMMA_STR = ",";
	COMMA_S = ",;";	
	DOT_STR = ".";
	DOT_S = ".";
	delStringAlternative(",", DOT);
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
bool Calculator::allPrefixesEnabled() {
	return b_use_all_prefixes;
}
void Calculator::setAllPrefixesEnabled(bool enable) {
	b_use_all_prefixes = enable;
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
	addFunction(new WarningFunction());
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
	addFunction(new RangeFunction());
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
	addFunction(new SumFunction());
	addFunction(new MinFunction());
	addFunction(new MaxFunction());
	addFunction(new ModeFunction());
	addFunction(new RandomFunction());
	addFunction(new BASEFunction());
	addFunction(new BINFunction());
	addFunction(new OCTFunction());
	addFunction(new HEXFunction());
	addFunction(new TitleFunction());
	addFunction(new SaveFunction());
	addFunction(new ConcatenateFunction());
	addFunction(new LengthFunction());
}
void Calculator::addBuiltinUnits() {
	addUnit(new Unit(_("Currency"), "EUR", "euros", "euro", "European Euros", false, true));
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
	if(errors.size() == 0 || error_str != errors.top()->message()) {
		errors.push(new Error(error_str, critical));
	}
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
#ifndef HAVE_LIBCLN
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
#endif
}
void Calculator::checkFPExceptions(const char *str) {
#ifndef HAVE_LIBCLN
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
#endif
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
		for(int i = 0; i < mngr->countChilds(); i++) {
			convert(mngr->getChild(i), to_unit, false);
			if(!mngr->getChild(i)->isPrecise()) mngr->setPrecise(false);
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
					if(cu->containsRelativeTo(mngr->unit())) {
						b = true;
					}
					break;
				} 
				case MULTIPLICATION_MANAGER: {
					for(int i = 0; i < mngr->countChilds(); i++) {
						if(mngr->getChild(i)->isUnit() && cu->containsRelativeTo(mngr->getChild(i)->unit())) {
							b = true;
						}
						if(mngr->getChild(i)->isPower() && mngr->getChild(i)->getChild(0)->isUnit() && cu->containsRelativeTo(mngr->getChild(i)->getChild(0)->unit())) {
							b = true;
						}
					}
					break;
				}
				case POWER_MANAGER: {
					if(mngr->getChild(0)->isUnit() && cu->containsRelativeTo(mngr->getChild(0)->unit())) {
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
			if(mngr->isMultiplication()) {
				mngr->push_back(mngr2);
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
		for(int i = 0; i < mngr->countChilds(); i++) {
			convertToCompositeUnit(mngr->getChild(i), cu, false);
			if(!mngr->getChild(i)->isPrecise()) mngr->setPrecise(false);
		}
		mngr->sort();
	} else {
		bool b = false;
		if(mngr->convert(cu) || always_convert) {	
			b = true;
		} else {
			switch(mngr->type()) {
				case UNIT_MANAGER: {
					if(cu->containsRelativeTo(mngr->unit())) {
						b = true;
					}
					break;
				} 
				case MULTIPLICATION_MANAGER: {
					for(int i = 0; i < mngr->countChilds(); i++) {
						if(mngr->getChild(i)->isUnit() && cu->containsRelativeTo(mngr->getChild(i)->unit())) {
							b = true;
						}
						if(mngr->getChild(i)->isPower() && mngr->getChild(i)->getChild(0)->isUnit() && cu->containsRelativeTo(mngr->getChild(i)->getChild(0)->unit())) {
							b = true;
						}
					}
					break;
				}
				case POWER_MANAGER: {
					if(mngr->getChild(0)->isUnit() && cu->containsRelativeTo(mngr->getChild(0)->unit())) {
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
				mngr->push_back(mngr2);
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
		} else if(l < u->name().length() || (l == u->name().length() && ufv_t[i] != 'p' && ufv_t[i] != 'P')) {
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
			} else if(l < u->plural(false).length() || (l == u->plural(false).length() && ufv_t[i] != 'p' && ufv_t[i] != 'P')) {
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
			} else if(l < u->singular(false).length() || (l == u->singular(false).length() && ufv_t[i] != 'p' && ufv_t[i] != 'P')) {
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
	return name_.find_first_of(ILLEGAL_IN_NAMES) == string::npos && !is_in(NUMBERS, name_[0]);
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
	gsub(SPACE, UNDERSCORE, name_);
	while(is_in(NUMBERS, name_[0])) {
		name_.erase(name_.begin());
	}
	return name_;
}
string Calculator::convertToValidFunctionName(string name_) {
	return convertToValidVariableName(name_);
}
string Calculator::convertToValidUnitName(string name_) {
	int i = 0;
	string stmp = ILLEGAL_IN_NAMES_MINUS_SPACE_STR + NUMBERS;
	while(true) {
		i = name_.find_first_of(stmp, i);
		if(i == string::npos)
			break;
		name_.erase(name_.begin() + i);
	}
	gsub(SPACE, UNDERSCORE, name_);
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
		moved_forward = false;
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
				stmp = LEFT_PARENTHESIS_CH;
				stmp += ID_WRAP_LEFT_CH;
				mngr = new Manager(str.substr(str_index + 1, i - str_index - 1));
				stmp += i2s(addId(mngr));
				mngr->unref();
				stmp += ID_WRAP_RIGHT_CH;
				stmp += RIGHT_PARENTHESIS_CH;
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
				} else if(str[str_index - 1] == RIGHT_PARENTHESIS_CH) {
					i3 = str_index - 2;
					i4 = 1;
					while(true) {
						i3 = str.find_last_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, i3);
						if(i3 == string::npos) {
							break;
						}
						if(str[i3] == RIGHT_PARENTHESIS_CH) {
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
						stmp = LEFT_PARENTHESIS_CH;
						stmp += ID_WRAP_LEFT_CH;
						stmp += i2s(addId(mngr));
						mngr->unref();
						stmp += ID_WRAP_RIGHT_CH;
						stmp += RIGHT_PARENTHESIS_CH;
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
							stmp = LEFT_PARENTHESIS_CH;
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
							stmp += RIGHT_PARENTHESIS_CH;
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
								if(i5 != string::npos && str[i5] == LEFT_PARENTHESIS_CH) {
									i5 = str.find_first_not_of(SPACES, i5 + 1);							
									if(i5 != string::npos && str[i5] == RIGHT_PARENTHESIS_CH) {
										i4 = i5 - str_index + 1;
									}
								}
								mngr = f->calculate("");
								if(mngr) {
									stmp = LEFT_PARENTHESIS_CH;
									stmp += ID_WRAP_LEFT_CH;
									stmp += i2s(addId(mngr));
									mngr->unref();
									stmp += ID_WRAP_RIGHT_CH;
									stmp += RIGHT_PARENTHESIS_CH;
								} else {
									stmp = "";
								}
								if(i4 < 0) i4 = name_length;
							} else if(b_rpn && f->args() == 1 && str_index > 0 && str[str_index - 1] == SPACE_CH && (str_index + name_length >= str.length() || str[str_index + name_length] != LEFT_PARENTHESIS_CH) && (i6 = str.find_last_not_of(SPACE, str_index - 1)) != string::npos) {
								i5 = str.rfind(SPACE, i6);	
								if(i5 == string::npos) {
									stmp = str.substr(0, i6 + 1);	
								} else {
									stmp = str.substr(i5 + 1, i6 - i5);
								}
								mngr =  f->calculate(stmp);
								if(mngr) {
									stmp = LEFT_PARENTHESIS_CH;
									stmp += ID_WRAP_LEFT_CH;
									stmp += i2s(addId(mngr));
									mngr->unref();
									stmp += ID_WRAP_RIGHT_CH;
									stmp += RIGHT_PARENTHESIS_CH;
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
										if(c == LEFT_PARENTHESIS_CH && i5 != 2) {
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
										stmp = LEFT_PARENTHESIS_CH;
										stmp += ID_WRAP_LEFT_CH;
										stmp += i2s(addId(mngr));
										mngr->unref();
										stmp += ID_WRAP_RIGHT_CH;
										stmp += RIGHT_PARENTHESIS_CH;
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
										i5 = str.find(RIGHT_PARENTHESIS_CH, i7);
										if(i5 == string::npos) {
											str.append(1, RIGHT_PARENTHESIS_CH);
											i5 = str.length() - 1;
										}
										if(i5 < (i6 = str.find(LEFT_PARENTHESIS_CH, i8)) || i6 == string::npos) {
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
										stmp = LEFT_PARENTHESIS_CH;
										stmp += ID_WRAP_LEFT_CH;
										stmp += i2s(addId(mngr));
										mngr->unref();
										stmp += ID_WRAP_RIGHT_CH;
										stmp += RIGHT_PARENTHESIS_CH;
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
							stmp = LEFT_PARENTHESIS_CH;					
							stmp += ID_WRAP_LEFT_CH;
							stmp += i2s(addId(mngr));
							mngr->unref();
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_PARENTHESIS_CH;				
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
					int i = 1;
					while(i <= chars_left && str[str_index + i] < 0) {
						i++;
					}
					mngr = new Manager(str.substr(str_index, i));
					stmp = LEFT_PARENTHESIS_CH;
					stmp += ID_WRAP_LEFT_CH;
					stmp += i2s(addId(mngr));
					mngr->unref();
					stmp += ID_WRAP_RIGHT_CH;
					stmp += RIGHT_PARENTHESIS_CH;
					str.replace(str_index, i, stmp);
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
string Calculator::getName(string name, ExpressionItem *object, bool force, bool always_append) {
	if(object) {
		switch(object->type()) {
			case TYPE_UNIT: {
				return getUnitName(name, (Unit*) object, force, always_append);
			}
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
		name = "var";
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
	filename += "units.xml";	
	if(!loadDefinitions(filename.c_str(), false)) {
		return false;
	}
	filename = dir;
	filename += "functions.xml";	
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
	string version, stmp, lang_tmp, name, type, svalue, plural, singular, category_title, category, description, title, reverse, base, argname;
	bool best_title, next_best_title, best_category_title, next_best_category_title, best_description, next_best_description;
	bool best_plural, next_best_plural, best_singular, next_best_singular, best_argname, next_best_argname;

	string locale = setlocale(LC_ALL, "");
	if(locale == "POSIX" || locale == "C") {
		locale = "";
	}
	string localebase = locale.substr(0, 2);

	long int exponent, litmp;
	bool precise, active, hidden, b;
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
	xmlChar *value, *lang;
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
		error(true, _("File not identified as Qalculate! definitions file: %s."), file_name, NULL);
		xmlFreeDoc(doc);
		return false;
	}
	bool functions_was = b_functions, variables_was = b_variables, units_was = b_units, unknown_was = b_unknown, calcvars_was = b_calcvars, always_exact_was = b_always_exact, rpn_was = b_rpn;
	b_functions = true; b_variables = true; b_units = true; b_unknown = true; b_calcvars = true; b_always_exact = true; b_rpn = false;

	vector<xmlNodePtr> unfinished_nodes;
	vector<string> unfinished_cats;
	queue<xmlNodePtr> sub_items;
	vector<queue<xmlNodePtr> > nodes;

	category = "";
	nodes.resize(1);
	
	while(true) {
		if(!in_unfinished) {
			category_title = ""; best_category_title = false; next_best_category_title = false;	
			child = cur->xmlChildrenNode;
			while(child != NULL) {
				if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {
					XML_GET_LOCALE_STRING_FROM_TEXT(child, category_title, best_category_title, next_best_category_title);
				} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {
					nodes.back().push(child);
				} else {
					sub_items.push(child);
				}
				child = child->next;
			}
			if(!category.empty()) {
				category += "/";
			}
			category += category_title;
		}
		while(!sub_items.empty() || (in_unfinished && cur)) {
			if(!in_unfinished) {
				cur = sub_items.front();
				sub_items.pop();
			}
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
					f = addFunction(new UserFunction(category, name, "", is_user_defs, 0, "", "", 0, active));
					done_something = true;
					child = cur->xmlChildrenNode;
					hidden = false;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "expression")) {
							XML_DO_FROM_TEXT(child, ((UserFunction*) f)->setEquation);
							XML_GET_FALSE_FROM_PROP(child, "precise", b)
							f->setPrecise(b);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "condition")) {
							XML_DO_FROM_TEXT(child, f->setCondition);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
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
							} else if(type == "angle") {
								arg = new AngleArgument();
							} else {
								arg = new Argument();
							}
							child2 = child->xmlChildrenNode;
							argname = ""; best_argname = false; next_best_argname = false;
							while(child2 != NULL) {
								if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
									XML_GET_LOCALE_STRING_FROM_TEXT(child2, argname, best_argname, next_best_argname);
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
							arg->setName(argname);					
							itmp = 1;
							XML_GET_INT_FROM_PROP(child, "index", itmp);
							f->setArgumentDefinition(itmp, arg); 
						}
						child = child->next;
					}
					f->setDescription(description);
					f->setTitle(title);
					f->setHidden(hidden);
					f->setChanged(false);
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_function")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				f = getFunction(name);
				if(f) {	
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					f->setLocal(is_user_defs, active);
					child = cur->xmlChildrenNode;
					hidden = false;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "argument")) {
							child2 = child->xmlChildrenNode;
							argname = ""; best_argname = false; next_best_argname = false;
							while(child2 != NULL) {
								if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
									XML_GET_LOCALE_STRING_FROM_TEXT(child2, argname, best_argname, next_best_argname);
								}
								child2 = child2->next;
							}
							itmp = 1;
							XML_GET_INT_FROM_PROP(child, "index", itmp);
							if(f->getArgumentDefinition(itmp)) {
								f->getArgumentDefinition(itmp)->setName(argname);
							} else {
								f->setArgumentDefinition(itmp, new Argument(argname, false));
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						}
						child = child->next;
					}
					f->setDescription(description);
					f->setTitle(title);
					f->setCategory(category);
					f->setHidden(hidden);
					f->setChanged(false);
					done_something = true;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "variable")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				if(!name.empty() && variableNameIsValid(name)) {	
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					svalue = "";
					v = addVariable(new Variable(category, name, "", "", is_user_defs, false, active));
					done_something = true;
					child = cur->xmlChildrenNode;
					b = true;
					hidden = false;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "value")) {
							XML_DO_FROM_TEXT(child, v->set);
							XML_GET_FALSE_FROM_PROP(child, "precise", b);
							v->setPrecise(b);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_DO_FROM_TEXT(child, v->setDescription);
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						}
						child = child->next;
					}
					v->setDescription(description);
					v->setTitle(title);
					v->setHidden(hidden);
					v->setChanged(false);
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_variable")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				v = getVariable(name);
				if(v) {	
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					v->setLocal(is_user_defs, active);
					child = cur->xmlChildrenNode;
					hidden = false;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						}
						child = child->next;
					}		
					v->setCategory(category);
					v->setDescription(description);
					v->setTitle(title);	
					v->setHidden(hidden);	
					v->setChanged(false);
					done_something = true;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "unit")) {
				XML_GET_STRING_FROM_PROP(cur, "type", type)
				if(type == "base") {	
					XML_GET_STRING_FROM_PROP(cur, "name", name)
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					if(unitNameIsValid(name)) {
						child = cur->xmlChildrenNode;
						singular = ""; best_singular = false; next_best_singular = false;
						plural = ""; best_plural = false; next_best_plural = false;
						title = ""; best_title = false; next_best_title = false;
						hidden = false;
						description = ""; best_description = false; next_best_description = false;
						while(child != NULL) {
							if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
								XML_GET_TRUE_FROM_TEXT(child, hidden);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
								XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "singular")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child, singular, best_singular, next_best_singular);
								if(!unitNameIsValid(singular)) {
									singular = "";
								}
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "plural")) {	
								XML_GET_LOCALE_STRING_FROM_TEXT(child, plural, best_plural, next_best_plural);
								if(!unitNameIsValid(plural)) {
									plural = "";
								}
							}
							child = child->next;
						}		
						u = addUnit(new Unit(category, name, plural, singular, title, is_user_defs, false, active));		
						u->setDescription(description);
						u->setHidden(hidden);
						u->setChanged(false);
						done_something = true;
					}
				} else if(type == "alias") {	
					XML_GET_STRING_FROM_PROP(cur, "name", name)
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					u = NULL;
					child = cur->xmlChildrenNode;
					hidden = false;
					singular = ""; best_singular = false; next_best_singular = false;
					plural = ""; best_plural = false; next_best_plural = false;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
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
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "singular")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, singular, best_singular, next_best_singular);
							if(!unitNameIsValid(singular)) {
								singular = "";
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "plural")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, plural, best_plural, next_best_plural);
							if(!unitNameIsValid(plural)) {
								plural = "";
							}
						}
						child = child->next;
					}
					if(!u) {
						if(!in_unfinished) {
							unfinished_nodes.push_back(cur);
							unfinished_cats.push_back(category);
						}
					} else if(unitNameIsValid(name)) {
						au = new AliasUnit(category, name, plural, singular, title, u, svalue, exponent, reverse, is_user_defs, false, active);
						au->setDescription(description);
						au->setPrecise(b);
						au->setHidden(hidden);
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
						title = ""; best_title = false; next_best_title = false;
						description = ""; best_description = false; next_best_description = false;
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
										unfinished_cats.push_back(category);
									}
									break;
								}
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
								XML_GET_TRUE_FROM_TEXT(child, hidden);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
								XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
							}
							child = child->next;
						}
						if(cu) {
							cu->setCategory(category);
							cu->setTitle(title);
							cu->setDescription(description);
							cu->setHidden(hidden);
							addUnit(cu);
							cu->setChanged(false);
							done_something = true;
						}
					}
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_unit")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				u = getUnit(name);
				if(!u) {
					u = getCompositeUnit(name);
				}
				if(u) {	
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					u->setLocal(is_user_defs, active);
					child = cur->xmlChildrenNode;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
					singular = ""; best_singular = false; next_best_singular = false;
					plural = ""; best_plural = false; next_best_plural = false;
					hidden = false;
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "singular")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, singular, best_singular, next_best_singular);
							if(!unitNameIsValid(singular)) {
								singular = "";
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "plural")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, plural, best_plural, next_best_plural);
							if(!unitNameIsValid(plural)) {
								plural = "";
							}
						}
						child = child->next;
					}	
					if(!singular.empty()) {
						u->setSingular(singular);
					}
					if(!plural.empty()) {
						u->setPlural(plural);
					}	
					u->setCategory(category);
					u->setDescription(description);
					u->setTitle(title);
					u->setHidden(hidden);		
					u->setChanged(false);
					done_something = true;
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
			cur = NULL;
			if(in_unfinished) {
				if(done_something) {
					in_unfinished--;
					unfinished_nodes.erase(unfinished_nodes.begin() + in_unfinished);
					unfinished_cats.erase(unfinished_cats.begin() + in_unfinished);
				}
				if(unfinished_nodes.size() > in_unfinished) {
					cur = unfinished_nodes[in_unfinished];
					category = unfinished_cats[in_unfinished];
				} else if(done_something && unfinished_nodes.size() > 0) {
					cur = unfinished_nodes[0];
					category = unfinished_cats[0];
					in_unfinished = 0;
					done_something = false;
				}
				in_unfinished++;
				done_something = false;
			}
		}
		if(in_unfinished) break;
		while(!nodes.empty() && nodes.back().empty()) {
			int cat_i = category.rfind("/");
			if(cat_i == string::npos) {
				category = "";
			} else {
				category = category.substr(0, cat_i);
			}
			nodes.pop_back();
		}
		if(!nodes.empty()) {
			cur = nodes.back().front();
			nodes.back().pop();
			nodes.resize(nodes.size() + 1);
		} else {
			if(unfinished_nodes.size() > 0) {
				cur = unfinished_nodes[0];
				category = unfinished_cats[0];
				in_unfinished = 1;
				done_something = false;
			} else {
				cur = NULL;
			}
		} 
		if(cur == NULL) {
			break;
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

struct node_tree_item {
	xmlNodePtr node;;
	string category;
	vector<node_tree_item> items;
};

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
	//cur = doc->children;
	bool was_always_exact = alwaysExact();
	setAlwaysExact(true);
	node_tree_item top;
	top.category = "";
	top.node = doc->children;
	node_tree_item *item;
	string cat, cat_sub;
	for(int i = 0; i < variables.size(); i++) {
		if((save_global || variables[i]->isLocal() || variables[i]->hasChanged()) && variables[i]->category() != _("Temporary")) {	
			item = &top;
			if(!variables[i]->category().empty()) {
				cat = variables[i]->category();
				int cat_i = cat.find("/"), cat_i_prev = -1;
				bool b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(int i2 = 0; i2 < item->items.size(); i2++) {
						if(cat_sub == item->items[i2].category) {
							item = &item->items[i2];
							b = true;
							break;
						}
					}
					if(!b) {
						item->items.resize(item->items.size() + 1);
						item->items[item->items.size() - 1].node = xmlNewTextChild(item->node, NULL, (xmlChar*) "category", NULL);
						item = &item->items[item->items.size() - 1];
						item->category = cat_sub;
						if(save_global) {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "_title", (xmlChar*) item->category.c_str());
						} else {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "title", (xmlChar*) item->category.c_str());
						}
					}
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i;
					cat_i = cat.find("/", cat_i_prev + 1);
				}
			}
			cur = item->node;
			if(!save_global && !variables[i]->isLocal() && variables[i]->hasChanged()) {
				if(variables[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) variables[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) variables[i]->referenceName().c_str());
				}
			} else if(save_global || variables[i]->isLocal()) {
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
				if(variables[i]->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
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
				if(!variables[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
				if(!variables[i]->isBuiltin()) {
					if(variables[i]->isExpression()) {
						newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) variables[i]->expression().c_str());
					} else {
						newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) variables[i]->get()->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTIONAL_ONLY).c_str());
					}
					if(!variables[i]->isPrecise()) xmlNewProp(newnode2, (xmlChar*) "precise", (xmlChar*) "false");					
				}
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
	node_tree_item top;
	top.category = "";
	top.node = doc->children;
	node_tree_item *item;
	string cat, cat_sub;
	for(int i = 0; i < units.size(); i++) {
		if(save_global || units[i]->isLocal() || units[i]->hasChanged()) {	
			item = &top;
			if(!units[i]->category().empty()) {
				cat = units[i]->category();
				int cat_i = cat.find("/"), cat_i_prev = -1;
				bool b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(int i2 = 0; i2 < item->items.size(); i2++) {
						if(cat_sub == item->items[i2].category) {
							item = &item->items[i2];
							b = true;
							break;
						}
					}
					if(!b) {
						item->items.resize(item->items.size() + 1);
						item->items[item->items.size() - 1].node = xmlNewTextChild(item->node, NULL, (xmlChar*) "category", NULL);
						item = &item->items[item->items.size() - 1];
						item->category = cat_sub;
						if(save_global) {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "_title", (xmlChar*) item->category.c_str());
						} else {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "title", (xmlChar*) item->category.c_str());
						}
					}
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i;
					cat_i = cat.find("/", cat_i_prev + 1);
				}
			}
			cur = item->node;	
			if(!save_global && !units[i]->isLocal() && units[i]->hasChanged()) {
				if(units[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) units[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) units[i]->referenceName().c_str());
				}
			} else if(save_global || units[i]->isLocal()) {
				if(units[i]->isBuiltin()) {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_unit", NULL);
					xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) units[i]->referenceName().c_str());
					if(units[i]->unitType() != COMPOSITE_UNIT) {
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
				} else {
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
						if(!units[i]->isPrecise()) xmlNewProp(newnode3, (xmlChar*) "precise", (xmlChar*) "false");				
						if(!au->reverseExpression().empty()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "reverse_relation", (xmlChar*) au->reverseExpression().c_str());	
						}
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "exponent", (xmlChar*) li2s(au->firstBaseExp()).c_str());
					}
				}
				if(units[i]->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
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
				if(!units[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
			}
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
	node_tree_item top;
	top.category = "";
	top.node = doc->children;
	node_tree_item *item;
	string cat, cat_sub;
	Argument *arg;
	IntegerArgument *iarg;
	FractionArgument *farg;
	string str;
	for(int i = 0; i < functions.size(); i++) {
		if(save_global || functions[i]->isLocal() || functions[i]->hasChanged()) {	
			item = &top;
			if(!functions[i]->category().empty()) {
				cat = functions[i]->category();
				int cat_i = cat.find("/"), cat_i_prev = -1;
				bool b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(int i2 = 0; i2 < item->items.size(); i2++) {
						if(cat_sub == item->items[i2].category) {
							item = &item->items[i2];
							b = true;
							break;
						}
					}
					if(!b) {
						item->items.resize(item->items.size() + 1);
						item->items[item->items.size() - 1].node = xmlNewTextChild(item->node, NULL, (xmlChar*) "category", NULL);
						item = &item->items[item->items.size() - 1];
						item->category = cat_sub;
						if(save_global) {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "_title", (xmlChar*) item->category.c_str());
						} else {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "title", (xmlChar*) item->category.c_str());
						}
					}
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i;
					cat_i = cat.find("/", cat_i_prev + 1);
				}
			}
			cur = item->node;
			if(!save_global && !functions[i]->isLocal() && functions[i]->hasChanged()) {
				if(functions[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) functions[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) functions[i]->referenceName().c_str());
				}
			} else if(save_global || functions[i]->isLocal()) {	
				if(functions[i]->isBuiltin()) {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_function", NULL);
					xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) functions[i]->referenceName().c_str());
					if(!functions[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
					if(functions[i]->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
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
					if(!functions[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
					if(functions[i]->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
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
					if(!functions[i]->isPrecise()) xmlNewProp(newnode2, (xmlChar*) "precise", (xmlChar*) "false");			
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
								case ARGUMENT_TYPE_ANGLE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "angle"); break;}
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
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);
	setLocale();
	return returnvalue;
}

long double Calculator::getAngleValue(long double value) {
	switch(angleMode()) {
	    case RADIANS: {return value;}
	    case DEGREES: {return deg2rad(value);}
	    case GRADIANS: {return gra2rad(value);}
	}
}
Manager *Calculator::setAngleValue(Manager *mngr) {
	if(mngr->isFraction()) {
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
	} else {
		Unit *rad = getUnit("rad");
		mngr->addUnit(rad, OPERATION_DIVIDE);
		mngr->finalize();
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
bool Calculator::loadExchangeRates() {
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *value;
	string homedir = "", filename, currency, rate;
	struct passwd *pw = getpwuid(getuid());
	if(pw) {
		homedir = pw->pw_dir;
		homedir += "/";
	}
	homedir += ".qalculate/";
	filename = homedir;
	filename += "eurofxref-daily.xml";
	doc = xmlParseFile(filename.c_str());
	if(doc == NULL) {
		fetchExchangeRates();
		doc = xmlParseFile(filename.c_str());
		if(doc == NULL) {
			return false;
		}
	}
	cur = xmlDocGetRootElement(doc);
	if(cur == NULL) {
		xmlFreeDoc(doc);
		return false;
	}
	Unit *euro = getUnit("EUR");
	Unit *u;
	if(!euro) {
		return false;
	}
	while(cur) {
		if(!xmlStrcmp(cur->name, (const xmlChar*) "Cube")) {
			XML_GET_STRING_FROM_PROP(cur, "currency", currency);
			if(!currency.empty()) {
				XML_GET_STRING_FROM_PROP(cur, "rate", rate);
				if(!rate.empty()) {
					rate = "1/" + rate;
					u = getUnit(currency);
					if(!u) {
						addUnit(new AliasUnit(_("Currency"), currency, "", "", "", euro, rate, 1, "", false, true));
					} else if(u->unitType() == ALIAS_UNIT) {
						((AliasUnit*) u)->setExpression(rate);
					}
				}
			}
		}
		if(cur->children) {
			cur = cur->children;
		} else if(cur->next) {
			cur = cur->next;
		} else {
			cur = cur->parent;
			if(cur) {
				cur = cur->next;
			}
		}
	}
}
bool Calculator::canFetch() {
	if(system("wget --version") == 0) {
		return true;
	}
	return false;
}
bool Calculator::fetchExchangeRates() {
	pid_t pid;
	int status;
	string homedir = "", filename_arg;
	struct passwd *pw = getpwuid(getuid());
	if(pw) {
		homedir = pw->pw_dir;
		homedir += "/";
	}
	homedir += ".qalculate/";
	mkdir(homedir.c_str(), S_IRWXU);	
	filename_arg =  "--output-document=";
	filename_arg += homedir;
	filename_arg += "eurofxref-daily.xml";	
	
	pid = fork();
	if(pid == 0) {
		execlp("wget", "--quiet", filename_arg.c_str(), "--tries=1", "--timeout=15", "http://www.ecb.int/stats/eurofxref/eurofxref-daily.xml", NULL);
		_exit(EXIT_FAILURE);
	} else if(pid < 0) {
		//error
		status = -1;
	} else {
		if(waitpid(pid, &status, 0) != pid) {
			status = -1;
		}
	}
	if(status != 0) error(true, _("Failed to download exchange rates from ECB."), NULL);
	return status == 0;
}

bool Calculator::canPlot() {
	FILE *pipe = popen("gnuplot -", "w");
	if(!pipe) {
		return false;
	}
	fputs("show version\n", pipe);
	return pclose(pipe) == 0;
}
Vector *Calculator::expressionToVector(string expression, const Manager *min, const Manager *max, int steps, Vector **x_vector, string x_var) {

	if(x_var[0] == '\\') {
		string x_var_sub = "\"";
		x_var_sub += x_var;
		x_var_sub += "\"";
		gsub(x_var, x_var_sub, expression);	
	}
	
	CALCULATOR->beginTemporaryStopErrors();
	Manager *mngr = calculate(expression);
	CALCULATOR->endTemporaryStopErrors();
	Vector *v = mngr->generateVector(x_var, min, max, steps, x_vector);
	mngr->unref();
	return v;
	
}
Vector *Calculator::expressionToVector(string expression, float min, float max, int steps, Vector **x_vector, string x_var) {
	Manager min_mngr(min), max_mngr(max);
	return expressionToVector(expression, &min_mngr, &max_mngr, steps, x_vector, x_var);
}
Vector *Calculator::expressionToVector(string expression, Vector *x_vector, string x_var) {
	
	if(x_var[0] == '\\') {
		string x_var_sub = "\"";
		x_var_sub += x_var;
		x_var_sub += "\"";
		gsub(x_var, x_var_sub, expression);	
	}
	
	CALCULATOR->beginTemporaryStopErrors();
	Manager *mngr = calculate(expression);
	CALCULATOR->endTemporaryStopErrors();	
	Vector *v = mngr->generateVector(x_var, x_vector);
	mngr->unref();
	return v;
	
}
bool Calculator::plotVectors(plot_parameters *param, Vector *y_vector, ...) {

	Vector *v;
	plot_data_parameters *pdp;
	vector<Vector*> y_vectors;
	vector<Vector*> x_vectors;
	vector<plot_data_parameters*> pdps;
	y_vectors.push_back(y_vector);
	va_list ap;
	va_start(ap, y_vector); 
	while(true) {
		v = va_arg(ap, Vector*);
		if(v == NULL) break;
		x_vectors.push_back(v);
		pdp = va_arg(ap, plot_data_parameters*);
		if(pdp == NULL) break;
		pdps.push_back(pdp);
		v = va_arg(ap, Vector*);
		if(v == NULL) break;
		y_vectors.push_back(v);
	}
	va_end(ap);	

	return plotVectors(param, y_vectors, x_vectors, pdps);

}

bool Calculator::plotVectors(plot_parameters *param, vector<Vector*> &y_vectors, vector<Vector*> &x_vectors, vector<plot_data_parameters*> &pdps, bool persistent) {

	string homedir = "";
	string filename;
	struct passwd *pw = getpwuid(getuid());
	if(pw) {
		homedir = pw->pw_dir;
		homedir += "/";
	}
	homedir += ".qalculate/";
	mkdir(homedir.c_str(), S_IRWXU);	
	homedir += "tmp/";	
	mkdir(homedir.c_str(), S_IRWXU);

	string commandline_extra;

	if(!param) {
		plot_parameters pp;
		param = &pp;
	}
	
	Vector *x_vector, *y_vector;
	string plot;
	
	if(param->filename.empty()) {
		if(!param->color) {
			commandline_extra += " -mono";
		}
		if(param->font.empty()) {
			commandline_extra += " -font \"-*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*\"";
		}
		plot += "set terminal x11\n";
	} else {
		persistent = true;
		if(param->filetype == PLOT_FILETYPE_AUTO) {
			int i = param->filename.find(".");
			if(i == string::npos) {
				param->filetype = PLOT_FILETYPE_PNG;
				error(false, _("No extension in file name. Saving as PNG image."), NULL);
			} else {
				string ext = param->filename.substr(i + 1, param->filename.length() - (i + 1));
				if(ext == "png") {
					param->filetype = PLOT_FILETYPE_PNG;
				} else if(ext == "ps") {
					param->filetype = PLOT_FILETYPE_PS;
				} else if(ext == "eps") {
					param->filetype = PLOT_FILETYPE_EPS;
				} else if(ext == "svg") {
					param->filetype = PLOT_FILETYPE_SVG;
				} else if(ext == "fig") {
					param->filetype = PLOT_FILETYPE_FIG;
				} else if(ext == "tex") {
					param->filetype = PLOT_FILETYPE_LATEX;
				} else {
					param->filetype = PLOT_FILETYPE_PNG;
					error(false, _("Unknown extension in file name. Saving as PNG image."), NULL);
				}
			}
		}
		plot += "set terminal ";
		switch(param->filetype) {
			case PLOT_FILETYPE_PNG: {
				plot += "png ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				break;
			}
			case PLOT_FILETYPE_FIG: {
				plot += "fig ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				break;
			}
			case PLOT_FILETYPE_SVG: {
				plot += "svg";
				break;
			}
			case PLOT_FILETYPE_LATEX: {
				plot += "latex ";
				break;
			}
			case PLOT_FILETYPE_PS: {
				plot += "postscript ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				plot += " \"Times\"";
				break;
			}
			case PLOT_FILETYPE_EPS: {
				plot += "postscript eps ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				plot += " \"Times\"";
				break;
			}
		}
		plot += "\nset output \"";
		plot += param->filename;
		plot += "\"\n";
	}

	switch(param->legend_placement) {
		case PLOT_LEGEND_NONE: {plot += "set nokey\n"; break;}
		case PLOT_LEGEND_TOP_LEFT: {plot += "set key top left\n"; break;}
		case PLOT_LEGEND_TOP_RIGHT: {plot += "set key top right\n"; break;}
		case PLOT_LEGEND_BOTTOM_LEFT: {plot += "set key bottom left\n"; break;}
		case PLOT_LEGEND_BOTTOM_RIGHT: {plot += "set key bottom right\n"; break;}
		case PLOT_LEGEND_BELOW: {plot += "set key below\n"; break;}
		case PLOT_LEGEND_OUTSIDE: {plot += "set key outside\n"; break;}
	}
	if(!param->x_label.empty()) {
		plot += "set xlabel \"";
		plot += param->x_label;
		plot += "\"\n";	
	}
	if(!param->y_label.empty()) {
		plot += "set ylabel \"";
		plot += param->y_label;
		plot += "\"\n";	
	}
	if(!param->title.empty()) {
		plot += "set title \"";
		plot += param->title;
		plot += "\"\n";	
	}
	if(param->grid) {
		plot += "set grid\n";
	}
	if(param->y_log) {
		plot += "set logscale y ";
		plot += i2s(param->y_log_base);
		plot += "\n";
	}
	if(param->x_log) {
		plot += "set logscale x ";
		plot += i2s(param->x_log_base);
		plot += "\n";
	}
	if(param->show_all_borders) {
		plot += "set border 15\n";
	} else {
		bool xaxis2 = false, yaxis2 = false;
		for(int i = 0; i < pdps.size(); i++) {
			if(pdps[i] && pdps[i]->xaxis2) {
				xaxis2 = true;
			}
			if(pdps[i] && pdps[i]->yaxis2) {
				yaxis2 = true;
			}
		}
		if(xaxis2 && yaxis2) {
			plot += "set border 15\nset x2tics\nset y2tics\n";
		} else if(xaxis2) {
			plot += "set border 7\nset x2tics\n";
		} else if(yaxis2) {
			plot += "set border 11\nset y2tics\n";
		} else {
			plot += "set border 3\n";
		}
		plot += "set xtics nomirror\nset ytics nomirror\n";
	}
	plot += "plot ";
	for(int i = 0; i < y_vectors.size(); i++) {
		if(y_vectors[i]) {
			if(i != 0) {
				plot += ",";
			}
			plot += "\"";
			plot += homedir;
			plot += "gnuplot_data";
			plot += i2s(i + 1);
			plot += "\"";
			if(i < pdps.size()) {
				switch(pdps[i]->smoothing) {
					case PLOT_SMOOTHING_UNIQUE: {plot += " smooth unique"; break;}
					case PLOT_SMOOTHING_CSPLINES: {plot += " smooth csplines"; break;}
					case PLOT_SMOOTHING_BEZIER: {plot += " smooth bezier"; break;}
					case PLOT_SMOOTHING_SBEZIER: {plot += " smooth sbezier"; break;}
				}
				if(pdps[i]->xaxis2 && pdps[i]->yaxis2) {
					plot += " axis x2y2";
				} else if(pdps[i]->xaxis2) {
					plot += " axis x2y1";
				} else if(pdps[i]->yaxis2) {
					plot += " axis x1y2";
				}
				if(!pdps[i]->title.empty()) {
					plot += " title \"";
					plot += pdps[i]->title;
					plot += "\"";
				}
				switch(pdps[i]->style) {
					case PLOT_STYLE_LINES: {plot += " with lines"; break;}
					case PLOT_STYLE_POINTS: {plot += " with points"; break;}
					case PLOT_STYLE_POINTS_LINES: {plot += " with linespoints"; break;}
					case PLOT_STYLE_BOXES: {plot += " with boxes"; break;}
					case PLOT_STYLE_HISTOGRAM: {plot += " with histeps"; break;}
					case PLOT_STYLE_STEPS: {plot += " with steps"; break;}
					case PLOT_STYLE_CANDLESTICKS: {plot += " with candlesticks"; break;}
					case PLOT_STYLE_DOTS: {plot += " with dots"; break;}
				}
				if(param->linewidth < 1) {
					plot += " lw 2";
				} else {
					plot += " lw ";
					plot += i2s(param->linewidth);
				}
			}
		}
	}
	plot += "\n";
	
	bool b_always_exact = alwaysExact();
	setAlwaysExact(false);	

	string filename_data;
	string plot_data;
	for(int serie = 0; serie < y_vectors.size(); serie++) {
		y_vector = y_vectors[serie];
		if(serie < x_vectors.size()) {
			x_vector = x_vectors[serie];
		} else {
			x_vector = NULL;
		}
		if(y_vector) {
			filename_data = homedir;
			filename_data += "gnuplot_data";
			filename_data += i2s(serie + 1);
			FILE *fdata = fopen(filename_data.c_str(), "w+");
			if(!fdata) {
				error(true, _("Could not create temporary file %s"), filename_data.c_str(), NULL);
				return false;
			}
			plot_data = "";
			for(int i = 1; i <= y_vector->components(); i++) {
				if(x_vector && x_vector->components() == y_vector->components()) {
					plot_data += x_vector->get(i)->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY);
					plot_data += " ";
				}
				plot_data += y_vector->get(i)->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY);
				plot_data += "\n";	
			}
			fputs(plot_data.c_str(), fdata);
			fflush(fdata);
			fclose(fdata);
		}
	}
	
	setAlwaysExact(b_always_exact);	
	
	return invokeGnuplot(plot, commandline_extra, persistent);
}
bool Calculator::invokeGnuplot(string commands, string commandline_extra, bool persistent) {
	FILE *pipe = NULL;
	if(!b_gnuplot_open || !gnuplot_pipe || persistent || commandline_extra != gnuplot_cmdline) {
		if(!persistent) {
			closeGnuplot();
		}
		string commandline = "gnuplot";
		if(persistent) {
			commandline += " -persist";
		}
		commandline += commandline_extra;
		commandline += " -";
		pipe = popen(commandline.c_str(), "w");
		if(!pipe) {
			error(true, _("Failed to invoke gnuplot. Make sure that you have gnuplot installed in your path."), NULL);
			return false;
		}
		if(!persistent && pipe) {
			gnuplot_pipe = pipe;
			b_gnuplot_open = true;
			gnuplot_cmdline = commandline_extra;
		}
	} else {
		pipe = gnuplot_pipe;
	}
	if(!pipe) {
		return false;
	}
	if(!persistent) {
		fputs("clear\n", pipe);
		fputs("reset\n", pipe);
	}
	fputs(commands.c_str(), pipe);
	fflush(pipe);
	if(persistent) {
		return pclose(pipe) == 0;
	}
	return true;
}
bool Calculator::closeGnuplot() {
	if(gnuplot_pipe) {
		int rv = pclose(gnuplot_pipe);
		gnuplot_pipe = NULL;
		b_gnuplot_open = false;
		return rv == 0;
	}
	gnuplot_pipe = NULL;
	b_gnuplot_open = false;
	return true;
}
bool Calculator::gnuplotOpen() {
	return b_gnuplot_open && gnuplot_pipe;
}

