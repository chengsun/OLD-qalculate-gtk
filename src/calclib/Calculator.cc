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
#include "MathStructure.h"
#include "Unit.h"
#include "Variable.h"
#include "Function.h"
#include "ExpressionItem.h"
#include "Prefix.h"
#include "Number.h"

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

#define WANT_OBFUSCATING_OPERATORS
#include <cln/cln.h>
using namespace cln;

#define XML_GET_APPROX_FROM_PROP(node, b)		value = xmlGetProp(node, (xmlChar*) "approximate"); if(value) {b = !xmlStrcmp(value, (const xmlChar*) "true");} else {value = xmlGetProp(node, (xmlChar*) "precise"); if(value) {b = xmlStrcmp(value, (const xmlChar*) "true");} else {b = false;}} if(value) xmlFree(value);
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

CalculatorError::CalculatorError(string message_, bool critical_ = false) {
	bcritical = critical_;
	smessage = message_;
}
CalculatorError::CalculatorError(const CalculatorError &e) {
	bcritical = e.critical();
	smessage = e.message();
}
string CalculatorError::message() const {
	return smessage;
}
const char* CalculatorError::c_message() const {
	return smessage.c_str();
}
bool CalculatorError::critical() const {
	return bcritical;
}

void Calculator::addStringAlternative(string replacement, string standard) {
	signs.push_back(replacement);
	real_signs.push_back(standard);
}
bool Calculator::delStringAlternative(string replacement, string standard) {
	for(unsigned int i = 0; i < signs.size(); i++) {
		if(signs[i] == replacement && real_signs[i] == standard) {
			signs.erase(signs.begin() + i);
			real_signs.erase(real_signs.begin() + i);
			return true;
		}
	}
	return false;
}
void Calculator::addDefaultStringAlternative(string replacement, string standard) {
	default_signs.push_back(replacement);
	default_real_signs.push_back(standard);
}
bool Calculator::delDefaultStringAlternative(string replacement, string standard) {
	for(unsigned int i = 0; i < default_signs.size(); i++) {
		if(default_signs[i] == replacement && default_real_signs[i] == standard) {
			default_signs.erase(default_signs.begin() + i);
			default_real_signs.erase(default_real_signs.begin() + i);
			return true;
		}
	}
	return false;
}

Calculator *calculator;

MathStructure m_undefined, m_empty_vector, m_empty_matrix, m_zero, m_one;
EvaluationOptions no_evaluation;

void *calculate_proc(void *x) {
	CALCULATOR->b_busy = true;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	MathStructure *mstruct = (MathStructure*) x;
	mstruct->set(_("aborted"));
	mstruct->set(CALCULATOR->calculate(CALCULATOR->expression_to_calculate, CALCULATOR->tmp_evaluationoptions));
	CALCULATOR->b_busy = false;
	return NULL;
}
void *print_proc(void *x) {
	CALCULATOR->b_busy = true;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	const MathStructure *mstruct = (const MathStructure*) x;
	MathStructure mstruct2(*mstruct);
	mstruct2.format();
	CALCULATOR->tmp_print_result = mstruct2.print(CALCULATOR->tmp_printoptions);
	CALCULATOR->b_busy = false;
	return NULL;
}

Calculator::Calculator() {

	exchange_rates_warning_issued = false;

	setPrecision(DEFAULT_PRECISION);

	addStringAlternative(SIGN_POWER_1, "^1");
	addStringAlternative(SIGN_POWER_2, "^2");
	addStringAlternative(SIGN_POWER_3, "^3");
	addStringAlternative(SIGN_INFINITY, "infinity");
	addStringAlternative("(+infinity)", "plus_infinity");
	addStringAlternative("(-infinity)", "minus_infinity");
	addStringAlternative(SIGN_DIVISION, DIVISION);	
	addStringAlternative(SIGN_MULTIPLICATION, MULTIPLICATION);		
	addStringAlternative(SIGN_MULTIDOT, MULTIPLICATION);			
	addStringAlternative(SIGN_MINUS, MINUS);		
	addStringAlternative(SIGN_PLUS, PLUS);		
	addStringAlternative(SIGN_NOT_EQUAL, " " NOT EQUALS);		
	addStringAlternative(SIGN_GREATER_OR_EQUAL, GREATER EQUALS);	
	addStringAlternative(SIGN_LESS_OR_EQUAL, LESS EQUALS);			
	addStringAlternative(";", COMMA);
	addStringAlternative("\t", SPACE);
	addStringAlternative("\n", SPACE);
	addStringAlternative("**", POWER);	

	setLocale();

	NAME_NUMBER_PRE_S = "_~#";
	NAME_NUMBER_PRE_STR = "_";
	
	string str = _(" to ");
	local_to = (str != " to ");
	
	null_prefix = new Prefix(0, "", "");
	
	PrintOptions po;
	po.number_fraction_format = FRACTION_FRACTIONAL;
	po.short_multiplication = false;
	
	m_undefined.setUndefined();
	m_empty_vector.clearVector();
	m_empty_matrix.clearMatrix();
	m_zero.clear();
	m_one.set(1, 1);
	no_evaluation.approximation = APPROXIMATION_EXACT;
	no_evaluation.structuring = STRUCTURING_NONE;
	no_evaluation.sync_units = false;

	default_assumptions = new Assumptions;
	default_assumptions->setNumberType(ASSUMPTION_NUMBER_REAL);
	default_assumptions->setSign(ASSUMPTION_SIGN_NONZERO);
	
	u_rad = NULL;

	saved_locale = strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	ILLEGAL_IN_NAMES = DOT_S + RESERVED OPERATORS SPACES PARENTHESISS VECTOR_WRAPS;
	ILLEGAL_IN_NAMES_MINUS_SPACE_STR = DOT_S + RESERVED OPERATORS PARENTHESISS VECTOR_WRAPS;	
	ILLEGAL_IN_UNITNAMES = ILLEGAL_IN_NAMES + NUMBERS;			
	b_argument_errors = true;
	calculator = this;
	srand48(time(0));
	angleMode(RADIANS);
	addBuiltinVariables();
	addBuiltinFunctions();
	addBuiltinUnits();
	disable_errors_ref = 0;
	b_busy = false;
	b_gnuplot_open = false;
	gnuplot_pipe = NULL;
	pthread_attr_init(&calculate_thread_attr);

}
Calculator::~Calculator() {
	closeGnuplot();
}

bool Calculator::utf8_pos_is_valid_in_name(char *pos) {
	if(is_in(ILLEGAL_IN_NAMES, pos[0])) {
		return false;
	}
	if(pos[0] < 0) {
		string str;
		str += pos[0];
		while(pos[1] < 0) {
			str += pos[1];
			pos++;
		}
		return str != SIGN_DIVISION && str != SIGN_MULTIPLICATION && str != SIGN_MULTIDOT && str != SIGN_MINUS && str != SIGN_PLUS && str != SIGN_NOT_EQUAL && str != SIGN_GREATER_OR_EQUAL && str != SIGN_LESS_OR_EQUAL;
	}
	return true;
}

bool Calculator::showArgumentErrors() const {
	return b_argument_errors;
}
void Calculator::beginTemporaryStopErrors() {
	disable_errors_ref++;
}
void Calculator::endTemporaryStopErrors() {
	disable_errors_ref--;
}
Variable *Calculator::getVariable(unsigned int index) const {
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
					if(nindex == 4) {
						name = ((Unit*) item)->unicodeName(false);
						if(name.empty()) {
							nindex++;
						}
					}					
					if(nindex > 4) {
						return NULL;
					}
					break;
				}
			}
			default: {
				if(nindex > 2) {
					return NULL;
				}
				if(nindex == 1) {
					name = item->referenceName();
				} else if(nindex == 2) {
					name = item->unicodeName(false);
					if(name.empty()) {
						nindex++;
					}
				}
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
	for(unsigned int index = 0; index < variables.size(); index++) {
		if(variables[index] != item && variables[index]->isActive() && (variables[index]->referenceName() == name || variables[index]->unicodeName(false) == name)) {
			return variables[index];
		}
	}
	for(unsigned int index = 0; index < functions.size(); index++) {
		if(functions[index] != item && functions[index]->isActive() && (functions[index]->referenceName() == name || functions[index]->unicodeName(false) == name)) {
			return functions[index];
		}
	}
	for(unsigned int i = 0; i < units.size(); i++) {
		if(units[i] != item && units[i]->isActive()) {
			if(units[i]->unitType() == COMPOSITE_UNIT) {
				if(name == units[i]->referenceName()) {
					return units[i];
				}
			} else {
				if(units[i]->name() == name || units[i]->singular(false) == name || units[i]->plural(false) == name || units[i]->unicodeName(false) == name) {
					return units[i];
				}
			}
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getInactiveExpressionItem(string name, ExpressionItem *item) {
	if(name.empty()) return NULL;
	for(unsigned int index = 0; index < variables.size(); index++) {
		if(variables[index] != item && !variables[index]->isActive() && (variables[index]->referenceName() == name || variables[index]->unicodeName(false) == name)) {
			return variables[index];
		}
	}
	for(unsigned int index = 0; index < functions.size(); index++) {
		if(functions[index] != item && !functions[index]->isActive() && (functions[index]->referenceName() == name || functions[index]->unicodeName(false) == name)) {
			return functions[index];
		}
	}
	for(unsigned int i = 0; i < units.size(); i++) {
		if(units[i] != item && !units[i]->isActive()) {
			if(units[i]->unitType() == COMPOSITE_UNIT) {
				if(name == units[i]->referenceName()) {
					return units[i];
				}
			} else {
				if(units[i]->name() == name || units[i]->singular(false) == name || units[i]->plural(false) == name || units[i]->unicodeName(false) == name) {
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
Unit *Calculator::getUnit(unsigned int index) const {
	if(index >= 0 && index < units.size()) {
		return units[index];
	}
	return NULL;
}
Function *Calculator::getFunction(unsigned int index) const {
	if(index >= 0 && index < functions.size()) {
		return functions[index];
	}
	return NULL;
}

void Calculator::setDefaultAssumptions(Assumptions *ass) {
	if(default_assumptions) delete default_assumptions;
	default_assumptions = ass;
}
Assumptions *Calculator::defaultAssumptions() {
	return default_assumptions;
}

Prefix *Calculator::getPrefix(unsigned int index) const {
	if(index >= 0 && index < prefixes.size()) {
		return prefixes[index];
	}
	return NULL;
}
Prefix *Calculator::getPrefix(string name_) const {
	for(unsigned int i = 0; i < prefixes.size(); i++) {
		if(prefixes[i]->shortName(false) == name_ || prefixes[i]->longName(false) == name_ || prefixes[i]->unicodeName(false) == name_) {
			return prefixes[i];
		}
	}
	return NULL;
}
Prefix *Calculator::getExactPrefix(int exp10, int exp) const {
	for(unsigned int i = 0; i < prefixes.size(); i++) {
		if(prefixes[i]->exponent(exp) == exp10) {
			return prefixes[i];
		} else if(prefixes[i]->exponent(exp) > exp10) {
			break;
		}
	}
	return NULL;
}
Prefix *Calculator::getExactPrefix(const Number &o, int exp) const {
	ComparisonResult c;
	for(unsigned int i = 0; i < prefixes.size(); i++) {
		c = o.compare(prefixes[i]->value(exp));
		if(c == COMPARISON_RESULT_EQUAL) {
			return prefixes[i];
		} else if(c == COMPARISON_RESULT_GREATER) {
			break;
		}
	}
	return NULL;
}
Prefix *Calculator::getNearestPrefix(int exp10, int exp) const {
	if(prefixes.size() <= 0) return NULL;
	int i = 0;
	if(exp < 0) {
		i = prefixes.size() - 1;
	}
	while((exp < 0 && i >= 0) || (exp >= 0 && i < (int) prefixes.size())) {	
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
		if(exp < 0) {
			i--;
		} else {
			i++;
		}
	}
	return prefixes[prefixes.size() - 1];
}
Prefix *Calculator::getBestPrefix(int exp10, int exp, bool all_prefixes) const {
	if(prefixes.size() <= 0) return NULL;
	int prev_i = 0;
	int i = 0;
	if(exp < 0) {
		i = prefixes.size() - 1;
	}
	while((exp < 0 && i >= 0) || (exp >= 0 && i < (int) prefixes.size())) {	
		if(all_prefixes || prefixes[i]->exponent() % 3 == 0) {
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
		if(exp < 0) {
			i--;
		} else {
			i++;
		}
	}
	return prefixes[prev_i];
}
Prefix *Calculator::getBestPrefix(const Number &exp10, const Number &exp, bool all_prefixes) const {
	if(prefixes.size() <= 0) return NULL;
	int prev_i = 0;
	int i = 0;
	ComparisonResult c;
	if(exp.isNegative()) {
		i = prefixes.size() - 1;
	}
	while((exp.isNegative() && i >= 0) || (!exp.isNegative() && i < (int) prefixes.size())) {
		if(all_prefixes || prefixes[i]->exponent() % 3 == 0) {
			c = exp10.compare(prefixes[i]->exponent(exp));
			if(c == COMPARISON_RESULT_EQUAL) {
				return prefixes[i];
			} else if(c == COMPARISON_RESULT_GREATER) {
				if(i == 0) {
					return prefixes[i];
				}
				Number exp10_1(exp10);
				exp10_1 -= prefixes[i - 1]->exponent(exp);
				Number exp10_2(prefixes[i]->exponent(exp));
				exp10_2 -= exp10;
				exp10_2 *= 2;
				exp10_2 += 2;
				if(exp10_1.isLessThan(exp10_2)) {
					return prefixes[prev_i];
				} else {
					return prefixes[i];
				}
			}
			prev_i = i;
		}
		if(exp.isNegative()) {
			i--;
		} else {
			i++;
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
	unsigned int l, i = 0;
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'u')
					l = ((Unit*) (*it))->singular(false).length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
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
	i = 0;
	if(!p->unicodeName(false).empty()) {
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) p);
				ufv_t.push_back('q');
				break;
			} else if(l <= p->unicodeName(false).length()) {
				ufv.insert(it, (void*) p);
				ufv_t.insert(ufv_t.begin() + i, 'q');
				break;
			}
			i++;
		}
	}
}

void Calculator::setPrecision(int precision) {
	if(precision <= 0) precision = DEFAULT_PRECISION;
	if(precision < 10) {
		cln::default_float_format = float_format(precision + (10 - precision) + 5);	
	} else {
		cln::default_float_format = float_format(precision + 5);	
	}
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

unsigned int Calculator::addId(const MathStructure &m_struct, bool persistent) {
	if(freed_ids.size() > 0) {
		ids.push_back(freed_ids.back());
		id_structs.push_back(m_struct);
		ids_p.push_back(persistent);
		freed_ids.pop_back();
		return ids.back();
	} else {
		ids_i++;
		ids.push_back(ids_i);
		id_structs.push_back(m_struct);
		ids_p.push_back(persistent);
		return ids_i;
	}
	return 0;
}
const MathStructure *Calculator::getId(unsigned int id) {
	for(unsigned int i = 0; i < ids.size(); i++) {
		if(ids[i] == id) {
			return &id_structs[i];
		}
	} 
	return NULL;
}

void Calculator::delId(unsigned int id, bool force) {
	for(unsigned int i = 0; i < ids.size(); i++) {
		if(ids[i] == id) {
			if(!ids_p[i] || force) {
				freed_ids.push_back(ids[i]);
				ids.erase(ids.begin() + i);
				id_structs.erase(id_structs.begin() + i);
				ids_p.erase(ids_p.begin() + i);	
			}
			break;
		}
	}
}

int Calculator::angleMode() const {
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
	v_e = (KnownVariable*) addVariable(new EVariable());
	v_pi = (KnownVariable*) addVariable(new PiVariable());	
	Number nr(1, 1);
	MathStructure mstruct;
	mstruct.number().setImaginaryPart(nr);
	v_i = (KnownVariable*) addVariable(new KnownVariable("", "i", mstruct, "Imaginary i (sqrt(-1))", false, true));
	mstruct.number().setInfinity();
	v_inf = (KnownVariable*) addVariable(new KnownVariable("", "infinity", mstruct, "Infinity", false, true));
	mstruct.number().setPlusInfinity();
	v_pinf = (KnownVariable*) addVariable(new KnownVariable("", "plus_infinity", mstruct, "+Infinity", false, true));
	mstruct.number().setMinusInfinity();
	v_minf = (KnownVariable*) addVariable(new KnownVariable("", "minus_infinity", mstruct, "-Infinity", false, true));
	addVariable(new EulerVariable());
	addVariable(new CatalanVariable());
	
	v_x = (UnknownVariable*) addVariable(new UnknownVariable("Unknowns", "x", "", true, false));
	v_y = (UnknownVariable*) addVariable(new UnknownVariable("Unknowns", "y", "", true, false));
	v_z = (UnknownVariable*) addVariable(new UnknownVariable("Unknowns", "z", "", true, false));
}
void Calculator::addBuiltinFunctions() {

	f_vector = addFunction(new VectorFunction());
	f_sort = addFunction(new SortFunction());
	f_rank = addFunction(new RankFunction());
	f_limits = addFunction(new LimitsFunction());
	f_component = addFunction(new ComponentFunction());
	f_components = addFunction(new ComponentsFunction());
	f_merge_vectors = addFunction(new MergeVectorsFunction());
	f_matrix = addFunction(new MatrixFunction());
	f_matrix_to_vector = addFunction(new MatrixToVectorFunction());
	f_area = addFunction(new AreaFunction());
	f_rows = addFunction(new RowsFunction());
	f_columns = addFunction(new ColumnsFunction());
	f_row = addFunction(new RowFunction());
	f_column = addFunction(new ColumnFunction());
	f_elements = addFunction(new ElementsFunction());
	f_element = addFunction(new ElementFunction());
	f_transpose = addFunction(new TransposeFunction());
	f_identity = addFunction(new IdentityFunction());
	f_determinant = addFunction(new DeterminantFunction());
	f_permanent = addFunction(new PermanentFunction());
	f_adjoint = addFunction(new AdjointFunction());
	f_cofactor = addFunction(new CofactorFunction());
	f_inverse = addFunction(new InverseFunction());

	f_factorial = addFunction(new FactorialFunction());
	f_binomial = addFunction(new BinomialFunction());
	
	f_abs = addFunction(new AbsFunction());
	f_signum = addFunction(new SignumFunction());
	f_gcd = addFunction(new GcdFunction());
	f_round = addFunction(new RoundFunction());
	f_floor = addFunction(new FloorFunction());
	f_ceil = addFunction(new CeilFunction());
	f_trunc = addFunction(new TruncFunction());
	f_int = addFunction(new IntFunction());
	f_frac = addFunction(new FracFunction());
	f_rem = addFunction(new RemFunction());
	f_mod = addFunction(new ModFunction());
	
	f_re = addFunction(new ReFunction());
	f_im = addFunction(new ImFunction());
	f_arg = addFunction(new ArgFunction());

	f_sqrt = addFunction(new SqrtFunction());
	f_sq = addFunction(new SquareFunction());
	
	f_exp = addFunction(new ExpFunction());
	
	f_ln = addFunction(new LogFunction());
	f_logn = addFunction(new LognFunction());
	
	f_sin = addFunction(new SinFunction());
	f_cos = addFunction(new CosFunction());
	f_tan = addFunction(new TanFunction());
	f_asin = addFunction(new AsinFunction());
	f_acos = addFunction(new AcosFunction());
	f_atan = addFunction(new AtanFunction());
	f_sinh = addFunction(new SinhFunction());
	f_cosh = addFunction(new CoshFunction());
	f_tanh = addFunction(new TanhFunction());
	f_asinh = addFunction(new AsinhFunction());
	f_acosh = addFunction(new AcoshFunction());
	f_atanh = addFunction(new AtanhFunction());
	f_radians_to_default_angle_unit = addFunction(new RadiansToDefaultAngleUnitFunction());
	
	f_zeta = addFunction(new ZetaFunction());
	f_gamma = addFunction(new GammaFunction());
	f_beta = addFunction(new BetaFunction());
	
	f_total = addFunction(new TotalFunction());
	f_percentile = addFunction(new PercentileFunction());
	f_min = addFunction(new MinFunction());
	f_max = addFunction(new MaxFunction());
	f_mode = addFunction(new ModeFunction());
	f_rand = addFunction(new RandFunction());
	
	f_days = addFunction(new DaysFunction());
	f_yearfrac = addFunction(new YearFracFunction());
	f_week = addFunction(new WeekFunction());
	f_weekday = addFunction(new WeekdayFunction());
	f_month = addFunction(new MonthFunction());
	f_day = addFunction(new DayFunction());
	f_year = addFunction(new YearFunction());
	f_yearday = addFunction(new YeardayFunction());
	f_time = addFunction(new TimeFunction());
	
	f_base = addFunction(new BaseFunction());
	f_bin = addFunction(new BinFunction());
	f_oct = addFunction(new OctFunction());
	f_hex = addFunction(new HexFunction());
	f_roman = addFunction(new RomanFunction());
	
	f_ascii = addFunction(new AsciiFunction());
	f_char = addFunction(new CharFunction());
	
	f_length = addFunction(new LengthFunction());
	f_concatenate = addFunction(new ConcatenateFunction());
	
	f_replace = addFunction(new ReplaceFunction());
	f_for = addFunction(new ForFunction());
	f_sum = addFunction(new SumFunction());
	f_product = addFunction(new ProductFunction());
	f_process = addFunction(new ProcessFunction());
	f_process_matrix = addFunction(new ProcessMatrixFunction());
	f_csum = addFunction(new CustomSumFunction());
	
	f_diff = addFunction(new DeriveFunction());
	f_solve = addFunction(new SolveFunction());

}
void Calculator::addBuiltinUnits() {
	u_euro = addUnit(new Unit(_("Currency"), "EUR", "euros", "euro", "European Euros", false, true, SIGN_EURO));
}
void Calculator::error(bool critical, const char *TEMPLATE, ...) {
	if(disable_errors_ref) return;
	string error_str = TEMPLATE;
	va_list ap;
	va_start(ap, TEMPLATE);
	const char *str;
	while(true) {
		unsigned int i = error_str.find("%s");
		if(i == string::npos) break;	
		str = va_arg(ap, const char*);
		if(!str) break;
		error_str.replace(i, 2, str);
	}
	va_end(ap);
	bool dup_error = false;
	for(unsigned int i = 0; i < errors.size(); i++) {
		if(error_str == errors[i].message()) {
			dup_error = true;
			break;
		}
	}
	if(!dup_error) {
		errors.push_back(CalculatorError(error_str, critical));
	}
}
CalculatorError* Calculator::error() {
	if(!errors.empty()) {
		return &errors.back();
	}
	return NULL;
}
CalculatorError* Calculator::nextError() {
	if(!errors.empty()) {
		errors.pop_back();
		if(!errors.empty()) {
			return &errors.back();
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
}
void Calculator::restoreState() {
}
void Calculator::clearBuffers() {
	for(unsigned int i = 0; i < ids.size(); i++) {
		if(!ids_p[i]) {
			freed_ids.push_back(ids[i]);
			ids.erase(ids.begin() + i);
			id_structs.erase(id_structs.begin() + i);
			ids_p.erase(ids_p.begin() + i);	
			i--;
		}
		break;
	}
}
void Calculator::abort() {
	while(pthread_cancel(calculate_thread) == 0) {
		usleep(100);
	}
	restoreState();
	clearBuffers();
	b_busy = false;
}
void Calculator::abort_this() {
	restoreState();
	clearBuffers();
	b_busy = false;
	pthread_exit(PTHREAD_CANCELED);
}
bool Calculator::busy() {
	return b_busy;
}
string Calculator::localizeExpression(string str) const {
	if(DOT_STR == DOT && COMMA_STR == COMMA) return str;
	vector<unsigned int> q_begin;
	vector<unsigned int> q_end;
	unsigned int i3 = 0;
	while(true) {
		i3 = str.find_first_of("\"\'", i3);
		if(i3 == string::npos) {
			break;
		}
		q_begin.push_back(i3);
		i3 = str.find(str[i3], i3 + 1);
		if(i3 == string::npos) {
			q_end.push_back(str.length() - 1);
			break;
		}
		q_end.push_back(i3);
		i3++;
	}
	if(COMMA_STR != COMMA) {
		unsigned int ui = str.find(COMMA);
		while(ui != string::npos) {
			bool b = false;
			for(unsigned int ui2 = 0; ui2 < q_end.size(); ui2++) {
				if(ui <= q_end[ui2] && ui >= q_begin[ui2]) {
					ui = str.find(COMMA, q_end[ui2] + 1);
					b = true;
					break;
				}
			}
			if(!b) {
				str.replace(ui, strlen(COMMA), COMMA_STR);
				ui = str.find(COMMA, ui + COMMA_STR.length());
			}
		}
	}
	if(DOT_STR != DOT) {
		unsigned int ui = str.find(DOT);
		while(ui != string::npos) {
			bool b = false;
			for(unsigned int ui2 = 0; ui2 < q_end.size(); ui2++) {
				if(ui <= q_end[ui2] && ui >= q_begin[ui2]) {
					ui = str.find(DOT, q_end[ui2] + 1);
					b = true;
					break;
				}
			}
			if(!b) {
				str.replace(ui, strlen(DOT), DOT_STR);
				ui = str.find(DOT, ui + DOT_STR.length());
			}
		}
	}
	return str;
}
string Calculator::unlocalizeExpression(string str) const {
	if(DOT_STR == DOT && COMMA_STR == COMMA) return str;
	vector<unsigned int> q_begin;
	vector<unsigned int> q_end;
	unsigned int i3 = 0;
	while(true) {
		i3 = str.find_first_of("\"\'", i3);
		if(i3 == string::npos) {
			break;
		}
		q_begin.push_back(i3);
		i3 = str.find(str[i3], i3 + 1);
		if(i3 == string::npos) {
			q_end.push_back(str.length() - 1);
			break;
		}
		q_end.push_back(i3);
		i3++;
	}
	if(DOT_STR != DOT) {
		unsigned int ui = str.find(DOT_STR);
		while(ui != string::npos) {
			bool b = false;
			for(unsigned int ui2 = 0; ui2 < q_end.size(); ui2++) {
				if(ui <= q_end[ui2] && ui >= q_begin[ui2]) {
					ui = str.find(DOT_STR, q_end[ui2] + 1);
					b = true;
					break;
				}
			}
			if(!b) {
				str.replace(ui, DOT_STR.length(), DOT);
				ui = str.find(DOT_STR, ui + strlen(DOT));
			}
		}
	}
	if(COMMA_STR != COMMA) {
		unsigned int ui = str.find(COMMA_STR);
		while(ui != string::npos) {
			bool b = false;
			for(unsigned int ui2 = 0; ui2 < q_end.size(); ui2++) {
				if(ui <= q_end[ui2] && ui >= q_begin[ui2]) {
					ui = str.find(COMMA_STR, q_end[ui2] + 1);
					b = true;
					break;
				}
			}
			if(!b) {
				str.replace(ui, COMMA_STR.length(), COMMA);
				ui = str.find(COMMA_STR, ui + strlen(COMMA));
			}
		}
	}
	return str;
}
bool Calculator::calculate(MathStructure &mstruct, string str, int usecs, const EvaluationOptions &eo) {
	mstruct = string(_("calculating..."));
	saveState();
	b_busy = true;
	bool had_usecs = usecs > 0;
	expression_to_calculate = str;
	tmp_evaluationoptions = eo;
	while(!pthread_create(&calculate_thread, &calculate_thread_attr, calculate_proc, &mstruct) == 0) {
		usleep(100);
	}
	while(usecs > 0 && b_busy) {
		usleep(1000);
		usecs -= 1000;
	}	
	if(had_usecs && b_busy) {
		abort();
		mstruct = string(_("aborted"));
		return false;
	}
	return true;
}
MathStructure Calculator::calculate(string str, const EvaluationOptions &eo) {
	unsigned int i = 0; 
	string str2 = "";
	if(eo.parse_options.units_enabled && (i = str.find(_(" to "))) != string::npos) {
		int l = strlen(_(" to "));
		str2 = str.substr(i + l, str.length() - i - l);		
		remove_blank_ends(str2);
		if(!str2.empty()) {
			str = str.substr(0, i);
		}
	} else if(local_to && eo.parse_options.units_enabled && (i = str.find(" to ")) != string::npos) {
		int l = strlen(" to ");
		str2 = str.substr(i + l, str.length() - i - l);		
		remove_blank_ends(str2);
		if(!str2.empty()) {
			str = str.substr(0, i);
		}
	}
	MathStructure mstruct(parse(str, eo.parse_options));
	mstruct.eval(eo);
	if(!str2.empty()) {
		return convert(mstruct, str2, eo);
	}
	return mstruct;
}
string Calculator::printMathStructureTimeOut(const MathStructure &mstruct, int usecs, const PrintOptions &po) {
	tmp_printoptions = po;
	saveState();
	b_busy = true;
	while(!pthread_create(&calculate_thread, &calculate_thread_attr, print_proc, (void*) &mstruct) == 0) {
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

MathStructure Calculator::convert(double value, Unit *from_unit, Unit *to_unit, const EvaluationOptions &eo) {
	return convert(value, from_unit, to_unit, eo);
}
MathStructure Calculator::convert(string str, Unit *from_unit, Unit *to_unit, const EvaluationOptions &eo) {
	MathStructure mstruct(parse(str, eo.parse_options));
	mstruct *= from_unit;
	mstruct.eval(eo);
	mstruct.convert(to_unit, true);
	mstruct.divide(to_unit, true);
	mstruct.eval(eo);
	return mstruct;
}
MathStructure Calculator::convert(const MathStructure &mstruct, Unit *to_unit, const EvaluationOptions &eo, bool always_convert) {
	if(to_unit->unitType() == COMPOSITE_UNIT) return convertToCompositeUnit(mstruct, (CompositeUnit*) to_unit, eo, always_convert);
	if(to_unit->unitType() != ALIAS_UNIT || (((AliasUnit*) to_unit)->baseUnit()->unitType() != COMPOSITE_UNIT && ((AliasUnit*) to_unit)->baseExp() == 1)) {
		MathStructure mstruct_new(mstruct);
		if(!mstruct_new.convert(to_unit, true)) {
			mstruct_new = mstruct;
		} else {
			mstruct_new.eval(eo);
			return mstruct_new;
		}
	}
	MathStructure mstruct_new(mstruct);
	if(mstruct_new.isAddition()) {
		for(unsigned int i = 0; i < mstruct_new.size(); i++) {
			mstruct_new[i] = convert(mstruct_new[i], to_unit, eo, false);
		}
		mstruct_new.childrenUpdated();
		EvaluationOptions eo2 = eo;
		eo2.calculate_functions = false;
		eo2.sync_units = false;
		eo2.keep_prefixes = true;
		mstruct_new.eval(eo2);
	} else {
		bool b = false;
		if(mstruct_new.convert(to_unit) || always_convert) {	
			b = true;
		} else if(to_unit->unitType() == ALIAS_UNIT && ((AliasUnit*) to_unit)->baseUnit()->unitType() == COMPOSITE_UNIT) {
			CompositeUnit *cu = (CompositeUnit*) ((AliasUnit*) to_unit)->baseUnit();
			switch(mstruct.type()) {
				case STRUCT_UNIT: {
					if(cu->containsRelativeTo(mstruct_new.unit())) {
						b = true;
					}
					break;
				} 
				case STRUCT_MULTIPLICATION: {
					for(unsigned int i = 1; i <= mstruct_new.countChilds(); i++) {
						if(mstruct_new.getChild(i)->isUnit() && cu->containsRelativeTo(mstruct_new.getChild(i)->unit())) {
							b = true;
							break;
						}
						if(mstruct_new.getChild(i)->isPower() && mstruct_new.getChild(i)->base()->isUnit() && cu->containsRelativeTo(mstruct_new.getChild(i)->base()->unit())) {
							b = true;
							break;
						}
					}
					break;
				}
				case STRUCT_POWER: {				
					if(mstruct_new.base()->isUnit() && cu->containsRelativeTo(mstruct_new.base()->unit())) {
						b = true;
					}
					break;
				}
			}
		}
		if(b) {			
			mstruct_new.divide(to_unit);
			EvaluationOptions eo2 = eo;
			eo2.calculate_functions = false;
			eo2.sync_units = true;
			eo2.keep_prefixes = false;
			mstruct_new.eval(eo2);
			if(mstruct_new.isOne()) mstruct_new.set(to_unit);
			else mstruct_new.multiply(to_unit, true);
			eo2.sync_units = false;
			eo2.keep_prefixes = true;
			mstruct_new.eval(eo2);
		}
	}
	return mstruct_new;
}
MathStructure Calculator::convertToBaseUnits(const MathStructure &mstruct, const EvaluationOptions &eo) {
	MathStructure mstruct_new(mstruct);
	for(unsigned int i = 0; i < units.size(); i++) {
		if(units[i]->unitType() == BASE_UNIT) {
			mstruct_new.convert(units[i], true);
		}
	}
	EvaluationOptions eo2 = eo;
	eo2.calculate_functions = false;
	mstruct_new.eval(eo2);
	return mstruct_new;
}
Unit *Calculator::getBestUnit(Unit *u, bool allow_only_div) {
	switch(u->unitType()) {
		case BASE_UNIT: {
			return u;
		}
		case ALIAS_UNIT: {
			AliasUnit *au = (AliasUnit*) u;
			if(au->baseExp() == 1 && au->baseUnit()->unitType() == BASE_UNIT) {
				return (Unit*) au->baseUnit();
			} else if(au->firstBaseUnit()->unitType() == COMPOSITE_UNIT || au->firstBaseExp() != 1) {
				return u;
			} else {
				return getBestUnit((Unit*) au->firstBaseUnit());
			}
		}
		case COMPOSITE_UNIT: {
			CompositeUnit *cu = (CompositeUnit*) u;
			int exp, b_exp;
			int points = 0;
			bool minus = false;
			int new_points;
			int new_points_m;
			int max_points = 0;
			for(unsigned int i = 0; cu->get(i, &exp); i++) {
				if(exp < 0) {
					max_points -= exp;
				} else {
					max_points += exp;
				}
			}
			Unit *best_u = NULL;
			Unit *cu_unit, *bu;
			AliasUnit *au;
			for(unsigned int i = 0; i < units.size(); i++) {
				if(units[i]->unitType() == BASE_UNIT && (points == 0 || (points == 1 && minus))) {
					new_points = 0;
					for(unsigned int i2 = 0; true; i2++) {
						cu_unit = cu->get(i2);
						if(!cu_unit) {
							break;
						} else if(cu_unit->baseUnit() == units[i]) {
							points = 1;
							best_u = units[i];
							minus = false;
							break;
						}
					}
				} else if(units[i]->unitType() == ALIAS_UNIT) {
					au = (AliasUnit*) units[i];
					bu = (Unit*) au->baseUnit();
					b_exp = au->baseExp();
					new_points = 0;
					new_points_m = 0;
					if(b_exp != 1 || bu->unitType() == COMPOSITE_UNIT) {
						if(bu->unitType() == BASE_UNIT) {
							for(unsigned int i2 = 0; true; i2++) {
								cu_unit = cu->get(i2, &exp);
								if(!cu_unit) {
									break;
								} else if(cu_unit == bu) {
									bool m = false;
									if(b_exp < 0 && exp < 0) {
										b_exp = -b_exp;
										exp = -exp;
									} else if(b_exp < 0) {
										b_exp = -b_exp;	
										m = true;
									} else if(exp < 0) {
										exp = -exp;
										m = true;
									}
									new_points = exp - b_exp;
									if(new_points < 0) {
										new_points = -new_points;
									}
									new_points = exp - new_points;
									if(!allow_only_div && m && new_points >= max_points) {
										new_points = -1;
									}
									if(new_points > points || (!m && minus && new_points == points)) {
										points = new_points;
										minus = m;
										best_u = units[i];
									}
									break;
								}
							}
						} else if(au->firstBaseExp() != 1 || au->firstBaseUnit()->unitType() == COMPOSITE_UNIT) {
							MathStructure cu_mstruct = ((CompositeUnit*) bu)->generateMathStructure();
							cu_mstruct.raise(b_exp);
							cu_mstruct = convertToBaseUnits(cu_mstruct);
							if(cu_mstruct.isMultiplication()) {
								for(unsigned int i2 = 1; i2 <= cu_mstruct.countChilds(); i2++) {
									bu = NULL;
									if(cu_mstruct.getChild(i2)->isUnit()) {
										bu = cu_mstruct.getChild(i2)->unit();
										b_exp = 1;
									} else if(cu_mstruct.getChild(i2)->isPower() && cu_mstruct.getChild(i2)->base()->isUnit() && cu_mstruct.getChild(i2)->exponent()->isNumber() && cu_mstruct.getChild(i2)->exponent()->number().isInteger()) {
										bu = cu_mstruct.getChild(i2)->base()->unit();
										b_exp = cu_mstruct.getChild(i2)->exponent()->number().intValue();
									}
									if(bu) {
										for(unsigned int i3 = 0; true; i3++) {
											cu_unit = cu->get(i3, &exp);
											if(!cu_unit) {
												break;
											} else if(cu_unit == bu) {
												bool m = false;
												if(exp < 0 && b_exp > 0) {
													new_points -= b_exp;
													exp = -exp;
													m = true;
												} else if(exp > 0 && b_exp < 0) {
													new_points += b_exp;
													b_exp = -b_exp;
													m = true;
												} else {
													if(b_exp < 0) new_points_m += b_exp;
													else new_points_m -= b_exp;
												}
												if(exp < 0) {
													exp = -exp;
													b_exp = -b_exp;
												}
												if(exp >= b_exp) {
													if(m) new_points_m += exp - (exp - b_exp);
													else new_points += exp - (exp - b_exp);
												} else {
													if(m) new_points_m += exp - (b_exp - exp);
													else new_points += exp - (b_exp - exp);
												}
												break;
											}
										}
										if(!cu_unit) {
											if(b_exp < 0) b_exp = -b_exp;
											new_points -= b_exp;	
											new_points_m -= b_exp;
										}
									}
								}
								if(!allow_only_div && new_points_m >= max_points) {
									new_points_m = -1;
								}
								if(new_points >= points && new_points >= new_points_m) {
									minus = false;
									points = new_points;
									best_u = au;
								} else if(new_points_m > points || (new_points_m == points && minus)) {
									minus = true;
									points = new_points_m;
									best_u = au;
								}
							}
						}
					}
				}
				if(points >= max_points && !minus) break;
			}
			best_u = getBestUnit(best_u);
			if(points > 1 && points < max_points - 1) {
				CompositeUnit *cu_new = new CompositeUnit("", "temporary_composite_convert");
				bool return_cu = minus;
				if(minus) {
					cu_new->add(best_u, -1);
				} else {
					cu_new->add(best_u);
				}
				MathStructure cu_mstruct = ((CompositeUnit*) u)->generateMathStructure();
				if(minus) cu_mstruct *= best_u;
				else cu_mstruct /= best_u;
				cu_mstruct = convertToBaseUnits(cu_mstruct);
				CompositeUnit *cu2 = new CompositeUnit("", "temporary_composite_convert_to_best_unit");
				bool b = false;
				for(unsigned int i = 1; i <= cu_mstruct.countChilds(); i++) {
					if(cu_mstruct.getChild(i)->isUnit()) {
						b = true;
						cu2->add(cu_mstruct.getChild(i)->unit());
					} else if(cu_mstruct.getChild(i)->isPower() && cu_mstruct.getChild(i)->base()->isUnit() && cu_mstruct.getChild(i)->exponent()->isNumber() && cu_mstruct.getChild(i)->exponent()->number().isInteger()) {
						b = true;
						cu2->add(cu_mstruct.getChild(i)->base()->unit(), cu_mstruct.getChild(i)->exponent()->number().intValue());
					}
				}
				if(b) {
					Unit *u2 = getBestUnit(cu2, true);
					Unit *cu_unit2;
					if(u2->unitType() == COMPOSITE_UNIT) {
						for(unsigned int i3 = 0; true; i3++) {
							cu_unit = ((CompositeUnit*) u2)->get(i3, &exp);
							if(!cu_unit) {
								break;
							} else {
								for(unsigned int i4 = 0; true; i4++) {
									cu_unit2 = cu_new->get(i4, &b_exp);
									if(!cu_unit2) {
										break;
									} else if(cu_unit2 == cu_unit) {
										cu_new->setExponent(i4, b_exp + exp);
										break;
									}
								}
								if(!cu_unit2) cu_new->add(cu_unit, exp);
							}
						}
						return_cu = true;
						delete u2;
					} else if(u2->unitType() == ALIAS_UNIT) {
						return_cu = true;
						for(unsigned int i3 = 0; true; i3++) {
							cu_unit = cu_new->get(i3, &exp);
							if(!cu_unit) {
								break;
							} else if(cu_unit == u2) {
								cu_new->setExponent(i3, exp + 1);
								break;
							}
						}
						if(!cu_unit) cu_new->add(u2);
					}
				}
				delete cu2;
				if(return_cu) {
					return cu_new;
				} else {
					delete cu_new;
					return best_u;
				}
			}
			if(minus) {
				CompositeUnit *cu_new = new CompositeUnit("", "temporary_composite_convert");
				cu_new->add(best_u, -1);
				return cu_new;
			} else {
				return best_u;
			}
		}
	}
	return u;	
}
MathStructure Calculator::convertToBestUnit(const MathStructure &mstruct, const EvaluationOptions &eo) {
	EvaluationOptions eo2 = eo;
	eo2.calculate_functions = false;
	eo2.sync_units = false;
	switch(mstruct.type()) {
		case STRUCT_NOT: {}
		case STRUCT_OR: {}
		case STRUCT_AND: {}
		case STRUCT_COMPARISON: {}
		case STRUCT_ALTERNATIVES: {}
		case STRUCT_FUNCTION: {}
		case STRUCT_VECTOR: {}
		case STRUCT_ADDITION: {
			MathStructure mstruct_new(mstruct);
			for(unsigned int i = 0; i < mstruct_new.size(); i++) {
				mstruct_new[i] = convertToBestUnit(mstruct_new[i], eo);
			}
			mstruct_new.childrenUpdated();
			mstruct_new.eval(eo2);
			return mstruct_new;
		}
		case STRUCT_POWER: {
			MathStructure mstruct_new(mstruct);
			if(mstruct_new.base()->isUnit() && mstruct_new.exponent()->isNumber() && mstruct_new.exponent()->number().isInteger()) {
				CompositeUnit *cu = new CompositeUnit("", "temporary_composite_convert_to_best_unit");
				cu->add(mstruct_new.base()->unit(), mstruct_new.exponent()->number().intValue());
				mstruct_new = convert(mstruct_new, getBestUnit(cu), eo, true);
				delete cu;
			} else {
				mstruct_new[0] = convertToBestUnit(mstruct_new[0], eo);
				mstruct_new[1] = convertToBestUnit(mstruct_new[1], eo);
				mstruct_new.childrenUpdated();
				mstruct_new.eval(eo2);
			}
			return mstruct_new;
		}
		case STRUCT_UNIT: {
			return convert(mstruct, getBestUnit(mstruct.unit()), eo, true);
		}
		case STRUCT_MULTIPLICATION: {
			MathStructure mstruct_new(convertToBaseUnits(mstruct, eo));
			CompositeUnit *cu = new CompositeUnit("", "temporary_composite_convert_to_best_unit");
			bool b = false;
			for(unsigned int i = 1; i <= mstruct_new.countChilds(); i++) {
				if(mstruct_new.getChild(i)->isUnit()) {
					b = true;
					cu->add(mstruct_new.getChild(i)->unit());
				} else if(mstruct_new.getChild(i)->isPower() && mstruct_new.getChild(i)->base()->isUnit() && mstruct_new.getChild(i)->exponent()->isNumber() && mstruct_new.getChild(i)->exponent()->number().isInteger()) {
					b = true;
					cu->add(mstruct_new.getChild(i)->base()->unit(), mstruct_new.getChild(i)->exponent()->number().intValue());
				} else {
					mstruct_new[i - 1] = convertToBestUnit(mstruct_new[i - 1]);
					mstruct_new.childUpdated(i);
				}
			}
			if(b) mstruct_new = convert(mstruct_new, getBestUnit(cu), eo, true);
			delete cu;
			mstruct_new.eval(eo2);
			return mstruct_new;
		}
	}
	return mstruct;
}
MathStructure Calculator::convertToCompositeUnit(const MathStructure &mstruct, CompositeUnit *cu, const EvaluationOptions &eo, bool always_convert) {
	MathStructure mstruct_cu(cu->generateMathStructure());
	MathStructure mstruct_new(mstruct);
	if(mstruct_new.isAddition()) {
		for(unsigned int i = 0; i < mstruct_new.size(); i++) {
			mstruct_new[i] = convertToCompositeUnit(mstruct_new[i], cu, eo, false);
		}
		mstruct_new.childrenUpdated();
		EvaluationOptions eo2 = eo;
		eo2.calculate_functions = false;
		eo2.sync_units = false;
		eo2.keep_prefixes = true;
		mstruct_new.eval(eo2);
	} else {
		bool b = false;
		if(mstruct_new.convert(cu, true) || always_convert) {	
			b = true;
		} else {
			switch(mstruct_new.type()) {
				case STRUCT_UNIT: {
					if(cu->containsRelativeTo(mstruct_new.unit())) {
						b = true;
					}
					break;
				} 
				case STRUCT_MULTIPLICATION: {
					for(unsigned int i = 1; i <= mstruct_new.countChilds(); i++) {
						if(mstruct_new.getChild(i)->isUnit() && cu->containsRelativeTo(mstruct_new.getChild(i)->unit())) {
							b = true;
						}
						if(mstruct_new.getChild(i)->isPower() && mstruct_new.getChild(i)->base()->isUnit() && cu->containsRelativeTo(mstruct_new.getChild(i)->base()->unit())) {
							b = true;
						}
					}
					break;
				}
				case STRUCT_POWER: {
					if(mstruct_new.base()->isUnit() && cu->containsRelativeTo(mstruct_new.base()->unit())) {
						b = true;
					}
					break;				
				}
			}
		}
		if(b) {	
			mstruct_new.divide(mstruct_cu);
			EvaluationOptions eo2 = eo;
			eo2.calculate_functions = false;
			eo2.sync_units = true;
			eo2.keep_prefixes = false;
			mstruct_new.eval(eo2);
			if(mstruct_new.isOne()) mstruct_new = mstruct_cu;
			else mstruct_new.multiply(mstruct_cu, true);
			eo2.sync_units = false;
			eo2.keep_prefixes = true;
			mstruct_new.eval(eo2);
		}
	}
	return mstruct_new;
}
MathStructure Calculator::convert(const MathStructure &mstruct, string composite_, const EvaluationOptions &eo) {
	remove_blank_ends(composite_);
	if(composite_.empty()) return mstruct;
	Unit *u = getUnit(composite_);
	if(u) return convert(mstruct, u);
	for(unsigned int i = 0; i < signs.size(); i++) {
		if(composite_ == signs[i]) {
			u = getUnit(real_signs[i]);
			break;
		}
	}
	if(u) return convert(mstruct, u, eo);
	CompositeUnit cu("", "temporary_composite_convert", "", composite_);
	return convertToCompositeUnit(mstruct, &cu, eo);
}
Unit* Calculator::addUnit(Unit *u, bool force) {
	if(u->unitType() == COMPOSITE_UNIT) {
		u->setName(getName(u->referenceName(), u, force));		
	} else {
		u->setName(getName(u->name(), u, force));
		if(!u->plural(false).empty()) {
			u->setPlural(getName(u->plural(false), u, force));
		}
		if(!u->singular(false).empty()) {
			u->setSingular(getName(u->singular(false), u, force));
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
		if(units[i]->unitType() != COMPOSITE_UNIT && (units[i]->name() == name_ || units[i]->singular(false) == name_ || units[i]->plural(false) == name_ || units[i]->unicodeName(false) == name_)) {
			return units[i];
		}
	}
	return NULL;
}
Unit* Calculator::getActiveUnit(string name_) {
	if(name_.empty()) return NULL;
	for(unsigned int i = 0; i < units.size(); i++) {
		if(units[i]->isActive() && units[i]->unitType() != COMPOSITE_UNIT && (units[i]->name() == name_ || units[i]->singular(false) == name_ || units[i]->plural(false) == name_ || units[i]->unicodeName(false) == name_)) {
			return units[i];
		}
	}
	return NULL;
}
Unit* Calculator::getCompositeUnit(string internal_name_) {
	if(internal_name_.empty()) return NULL;
	for(unsigned int i = 0; i < units.size(); i++) {
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
	unsigned int l, i = 0;
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
			else if(ufv_t[i] == 'z')
				l = ((Unit*) (*it))->unicodeName(false).length();
			else if(ufv_t[i] == 'g')
				l = ((Function*) (*it))->unicodeName().length();
			else if(ufv_t[i] == 'w')
				l = ((Variable*) (*it))->unicodeName().length();
			else if(ufv_t[i] == 'p')
				l = ((Prefix*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'P')
				l = ((Prefix*) (*it))->longName(false).length();				
			else if(ufv_t[i] == 'q')
				l = ((Prefix*) (*it))->unicodeName(false).length();
		}
		if(it == ufv.end()) {
			ufv.push_back((void*) v);
			ufv_t.push_back('v');
			break;
		} else if(l < v->name().length() || (l == v->name().length() && (ufv_t[i] == 'v' || ufv_t[i] == 'w'))) {
			ufv.insert(it, (void*) v);
			ufv_t.insert(ufv_t.begin() + i, 'v');
			break;
		}
		i++;
	}
	if(!v->unicodeName(false).empty()) {
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();				
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) v);
				ufv_t.push_back('w');
				break;
			} else if(l < v->unicodeName(false).length() || (l == v->unicodeName(false).length() && (ufv_t[i] == 'v' || ufv_t[i] == 'w'))) {
				ufv.insert(it, (void*) v);
				ufv_t.insert(ufv_t.begin() + i, 'w');
				break;
			}
			i++;
		}
	}
}
void Calculator::functionNameChanged(Function *f, bool priviliged) {
	if(!f->isActive()) return;
	unsigned int l, i = 0;
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
			else if(ufv_t[i] == 'z')
				l = ((Unit*) (*it))->unicodeName(false).length();
			else if(ufv_t[i] == 'g')
				l = ((Function*) (*it))->unicodeName().length();
			else if(ufv_t[i] == 'w')
				l = ((Variable*) (*it))->unicodeName().length();
			else if(ufv_t[i] == 'p')
				l = ((Prefix*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'P')
				l = ((Prefix*) (*it))->longName(false).length();				
			else if(ufv_t[i] == 'q')
				l = ((Prefix*) (*it))->unicodeName(false).length();
		}
		if(it == ufv.end()) {
			ufv.push_back((void*) f);
			ufv_t.push_back('f');
			break;
		} else if(l < f->name().length() || (l == f->name().length() && (ufv_t[i] != 'p' && ufv_t[i] != 'P' && ufv_t[i] != 'q'))) {
			ufv.insert(it, (void*) f);
			ufv_t.insert(ufv_t.begin() + i, 'f');
			break;
		}
		i++;
	}
	if(!f->unicodeName(false).empty()) {
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();				
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) f);
				ufv_t.push_back('g');
				break;
			} else if(l < f->unicodeName(false).length() || (l == f->unicodeName(false).length() && (ufv_t[i] != 'p' && ufv_t[i] != 'P' && ufv_t[i] != 'q'))) {
				ufv.insert(it, (void*) f);
				ufv_t.insert(ufv_t.begin() + i, 'g');
				break;
			}
			i++;
		}
	}
}
void Calculator::unitNameChanged(Unit *u) {
	if(!u->isActive()) return;
	if(u->unitType() == COMPOSITE_UNIT) {
		return;
	}
	unsigned int l, i = 0;
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
			else if(ufv_t[i] == 'z')
				l = ((Unit*) (*it))->unicodeName(false).length();
			else if(ufv_t[i] == 'g')
				l = ((Function*) (*it))->unicodeName().length();
			else if(ufv_t[i] == 'w')
				l = ((Variable*) (*it))->unicodeName().length();
			else if(ufv_t[i] == 'p')
				l = ((Prefix*) (*it))->shortName(false).length();
			else if(ufv_t[i] == 'P')
				l = ((Prefix*) (*it))->longName(false).length();
			else if(ufv_t[i] == 'q')
				l = ((Prefix*) (*it))->unicodeName(false).length();
		}
		if(it == ufv.end()) {
			ufv.push_back((void*) u);
			ufv_t.push_back('U');
			break;
		} else if(l < u->name().length() || (l == u->name().length() && (ufv_t[i] != 'p' && ufv_t[i] != 'P' && ufv_t[i] != 'q' && ufv_t[i] != 'f' && ufv_t[i] != 'g'))) {
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('Y');
				break;
			} else if(l < u->plural(false).length() || (l == u->plural(false).length() && (ufv_t[i] != 'p' && ufv_t[i] != 'P' && ufv_t[i] != 'q' && ufv_t[i] != 'f' && ufv_t[i] != 'g'))) {
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('u');
				break;
			} else if(l < u->singular(false).length() || (l == u->singular(false).length() && (ufv_t[i] != 'p' && ufv_t[i] != 'P' && ufv_t[i] != 'q' && ufv_t[i] != 'f' && ufv_t[i] != 'g'))) {
				ufv.insert(it, (void*) u);
				ufv_t.insert(ufv_t.begin() + i, 'u');
				break;
			}
			i++;
		}
	}
	if(!u->unicodeName(false).empty()) {
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
				else if(ufv_t[i] == 'z')
					l = ((Unit*) (*it))->unicodeName(false).length();
				else if(ufv_t[i] == 'g')
					l = ((Function*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'w')
					l = ((Variable*) (*it))->unicodeName().length();
				else if(ufv_t[i] == 'p')
					l = ((Prefix*) (*it))->shortName(false).length();
				else if(ufv_t[i] == 'P')
					l = ((Prefix*) (*it))->longName(false).length();
				else if(ufv_t[i] == 'q')
					l = ((Prefix*) (*it))->unicodeName(false).length();
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('z');
				break;
			} else if(l < u->unicodeName(false).length() || (l == u->unicodeName(false).length() && (ufv_t[i] != 'p' && ufv_t[i] != 'P' && ufv_t[i] != 'q' && ufv_t[i] != 'f' && ufv_t[i] != 'g'))) {
				ufv.insert(it, (void*) u);
				ufv_t.insert(ufv_t.begin() + i, 'z');
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
	for(unsigned int i = 0; i < variables.size(); i++) {
		if(variables[i]->name() == name_ || variables[i]->unicodeName(false) == name_) {
			return variables[i];
		}
	}
	return NULL;
}
Variable* Calculator::getActiveVariable(string name_) {
	if(name_.empty()) return NULL;
	for(unsigned int i = 0; i < variables.size(); i++) {
		if(variables[i]->isActive() && (variables[i]->name() == name_ || variables[i]->unicodeName(false) == name_)) {
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
	for(unsigned int i = 0; i < functions.size(); i++) {
		if(functions[i]->name() == name_ || functions[i]->unicodeName(false) == name_) {
			return functions[i];
		}
	}
	return NULL;
}
Function* Calculator::getActiveFunction(string name_) {
	if(name_.empty()) return NULL;
	for(unsigned int i = 0; i < functions.size(); i++) {
		if(functions[i]->isActive() && (functions[i]->name() == name_ || functions[i]->unicodeName(false) == name_)) {
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
	unsigned int i = 0;
	while(true) {
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
	unsigned int i = 0;
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
bool Calculator::nameTaken(string name, ExpressionItem *object) {
	if(name.empty()) return false;
	if(object) {
		switch(object->type()) {
			case TYPE_VARIABLE: {}
			case TYPE_UNIT: {
				for(unsigned int index = 0; index < variables.size(); index++) {
					if(variables[index]->isActive() && (variables[index]->referenceName() == name || variables[index]->unicodeName(false) == name)) {
						return variables[index] != object;
					}
				}
				for(unsigned int i = 0; i < units.size(); i++) {
					if(units[i]->isActive()) {
						if(units[i]->unitType() == COMPOSITE_UNIT) {
							if(name == units[i]->referenceName()) {
								return units[i] != object;
							}
						} else {
							if(units[i]->name() == name || units[i]->singular(false) == name || units[i]->plural(false) == name || units[i]->unicodeName(false) == name) {
								return units[i] != object;
							}
						}
					}
				}
				break;
			}
			case TYPE_FUNCTION: {
				for(unsigned int index = 0; index < functions.size(); index++) {
					if(functions[index]->isActive() && (functions[index]->referenceName() == name || functions[index]->unicodeName(false) == name)) {
						return functions[index] != object;
					}
				}
				break;
			}
		}
	}
	return false;
}
bool Calculator::unitIsUsedByOtherUnits(const Unit *u) const {
	const Unit *u2;
	for(unsigned int i = 0; i < units.size(); i++) {
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
MathStructure Calculator::parse(string str, const ParseOptions &po) {
	const string *name;
	string stmp, stmp2;
	bool b, moved_forward;
	int i, i3, i4, i5, i6, i7, i8, i9;
	int chars_left;
	int name_length, name_length_old;
	int found_function_index, found_function_name_length;
	int ufv_index;
	Function *f;
	Variable *v;
	Unit *u;
	Prefix *p = NULL;
	vector<int> q_begin;
	vector<int> q_end;
	i3 = 0;
	while(true) {
		i3 = str.find_first_of("\"\'", i3);
		if(i3 == (int) string::npos) {
			break;
		}
		q_begin.push_back(i3);
		i3 = str.find(str[i3], i3 + 1);
		if(i3 == (int) string::npos) {
			q_end.push_back(str.length() - 1);
			break;
		}
		q_end.push_back(i3);
		i3++;
	}
	
	for(unsigned int i = 0; i < signs.size(); i++) {
		//gsub(signs[i], real_signs[i], str);
		unsigned int ui = str.find(signs[i]);
		while(ui != string::npos) {
			b = false;
			for(unsigned int ui2 = 0; ui2 < q_end.size(); ui2++) {
				if((int) ui <= q_end[ui2] && (int) ui >= q_begin[ui2]) {
					ui = str.find(signs[i], q_end[ui2] + 1);
					b = true;
					break;
				}
			}
			if(!b) {
				str.replace(ui, signs[i].length(), real_signs[i]);
				ui = str.find(signs[i], ui + real_signs[i].length());
			}
		}
	}

	for(int str_index = 0; str_index < (int) str.length(); str_index++) {
		chars_left = str.length() - str_index;
		moved_forward = false;
		if(str[str_index] == LEFT_VECTOR_WRAP_CH) {
			i4 = 1;
			i3 = str_index;
			while(true) {
				i3 = str.find_first_of(LEFT_VECTOR_WRAP RIGHT_VECTOR_WRAP, i3 + 1);
				if(i3 == (int) string::npos) {
					for(; i4 > 0; i4--) {
						str += RIGHT_VECTOR_WRAP;
					}
					i3 = str.length() - 1;
				} else if(str[i3] == LEFT_VECTOR_WRAP_CH) {
					i4++;
				} else if(str[i3] == RIGHT_VECTOR_WRAP_CH) {
					i4--;
					if(i4 > 0) {
						i5 = str.find_first_not_of(SPACE, i3 + 1);
						if(i5 != (int) string::npos && str[i5] == LEFT_VECTOR_WRAP_CH) {
							str.insert(i5, COMMA);
						}
					}
				}
				if(i4 == 0) {
					stmp2 = str.substr(str_index + 1, i3 - str_index - 1);
					stmp = LEFT_PARENTHESIS_CH;
					stmp += ID_WRAP_LEFT_CH;
					stmp += i2s(addId(f_vector->calculate(stmp2)));
					stmp += ID_WRAP_RIGHT_CH;
					str.replace(str_index, i3 + 1 - str_index, stmp);
					str_index += stmp.length() - 1;
					break;
				}
			}	
		} else if(str[str_index] == '\"' || str[str_index] == '\'') {
			if(str_index == (int) (str.length()) - 1) {
				str.erase(str_index, 1);
			} else {
				i = str.find(str[str_index], str_index + 1);
				if(i == (int) string::npos) {
					i = str.length();
					name_length = i - str_index;
				} else {
					name_length = i - str_index + 1;
				}
				stmp = LEFT_PARENTHESIS_CH;
				stmp += ID_WRAP_LEFT_CH;
				MathStructure mstruct(str.substr(str_index + 1, i - str_index - 1));
				stmp += i2s(addId(mstruct));
				stmp += ID_WRAP_RIGHT_CH;
				stmp += RIGHT_PARENTHESIS_CH;
				str.replace(str_index, name_length, stmp);
				str_index += stmp.length() - 1;
			}
		} else if(str[str_index] == '!') {
			if(str_index > 0 && (chars_left == 1 || str[str_index + 1] != EQUALS_CH)) {
				stmp = "";
				if(is_in(NUMBERS, str[str_index - 1])) {
					i3 = str.find_last_not_of(NUMBERS, str_index - 1);
					if(i3 == (int) string::npos) {
						stmp2 = str.substr(0, str_index);
					} else {
						stmp2 = str.substr(i3 + 1, str_index - i3 - 1);
					}
				} else if(str[str_index - 1] == RIGHT_PARENTHESIS_CH) {
					i3 = str_index - 2;
					i4 = 1;
					while(true) {
						i3 = str.find_last_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, i3);
						if(i3 == (int) string::npos) {
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
					stmp = LEFT_PARENTHESIS_CH;
					stmp += ID_WRAP_LEFT_CH;
					stmp += i2s(addId(f_factorial->parse(stmp2)));
					stmp += ID_WRAP_RIGHT_CH;
					stmp += RIGHT_PARENTHESIS_CH;
					str.replace(str_index - stmp2.length(), stmp2.length() + 1, stmp);
					str_index -=  stmp2.length();
					str_index += stmp.length() - 1;
				}
			}
		} else if(is_not_in(NUMBERS NOT_IN_NAMES, str[str_index])) {
			found_function_index = -1;
			found_function_name_length = -1;
			for(ufv_index = 0; ufv_index < (int) ufv.size(); ufv_index++) {
				name = NULL;
				p = NULL;
				switch(ufv_t[ufv_index]) {
					case 'v': {
						if(po.variables_enabled) {
							name = &((ExpressionItem*) ufv[ufv_index])->name();
						}
						break;
					}
					case 'w': {
						if(po.variables_enabled) {
							name = &((ExpressionItem*) ufv[ufv_index])->unicodeName();
						}
						break;
					}
					case 'f': {
						if(po.functions_enabled && found_function_index < 0) {
							name = &((ExpressionItem*) ufv[ufv_index])->name();
						}
						break;
					}
					case 'g': {
						if(po.functions_enabled && found_function_index < 0) {
							name = &((ExpressionItem*) ufv[ufv_index])->unicodeName();
						}
						break;
					}
					case 'U': {
						if(po.units_enabled) {
							name = &((ExpressionItem*) ufv[ufv_index])->name();
						}
						break;
					}
					case 'u': {
						if(po.units_enabled) {
							name = &((Unit*) ufv[ufv_index])->singular();
						}
						break;
					}
					case 'Y': {
						if(po.units_enabled) {
							name = &((Unit*) ufv[ufv_index])->plural();
						}
						break;
					}
					case 'z': {
						if(po.units_enabled) {
							name = &((Unit*) ufv[ufv_index])->unicodeName();
						}
						break;
					}
					case 'p': {
						if(!p && po.units_enabled) {
							name = &((Prefix*) ufv[ufv_index])->shortName();
						}
						break;
					}
					case 'P': {
						if(!p && po.units_enabled) {
							name = &((Prefix*) ufv[ufv_index])->longName();
						}
						break;
					}
					case 'q': {
						if(!p && po.units_enabled) {
							name = &((Prefix*) ufv[ufv_index])->unicodeName();
						}
						break;
					}
				}
				if(name) name_length = name->length();
				if(name && (int) name_length >= found_function_name_length && (*name)[0] == str[str_index] && (name_length == 1 || (name_length <= chars_left && (*name)[1] == str[str_index + 1] && *name == str.substr(str_index, name_length)))) {
					moved_forward = false;
					switch(ufv_t[ufv_index]) {
						case 'w': {}
						case 'v': {
							v = (Variable*) ufv[ufv_index];
							stmp = LEFT_PARENTHESIS_CH;
							stmp += ID_WRAP_LEFT_CH;
							stmp += i2s(addId(v));
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_PARENTHESIS_CH;
							str.replace(str_index, name_length, stmp);
							str_index += stmp.length();
							moved_forward = true;
							break;
						}
						case 'g': {}
						case 'f': {
							i5 = str.find_first_not_of(SPACES, str_index + name_length);
							if(i5 == (int) string::npos || str[i5] != LEFT_PARENTHESIS_CH) {
								found_function_index = ufv_index;
								found_function_name_length = name_length;
								break;
							}
							set_function:
							f = (Function*) ufv[ufv_index];
							b = false;
							i4 = -1;
							if(f->args() == 0) {
								i5 = str.find_first_not_of(SPACES, str_index + name_length);
								if(i5 != (int) string::npos && str[i5] == LEFT_PARENTHESIS_CH) {
									i5 = str.find_first_not_of(SPACES, i5 + 1);							
									if(i5 != (int) string::npos && str[i5] == RIGHT_PARENTHESIS_CH) {
										i4 = i5 - str_index + 1;
									}
								}
								stmp = LEFT_PARENTHESIS_CH;
								stmp += ID_WRAP_LEFT_CH;
								stmp += i2s(addId(f->parse("")));
								stmp += ID_WRAP_RIGHT_CH;
								stmp += RIGHT_PARENTHESIS_CH;
								if(i4 < 0) i4 = name_length;
							} else if(po.rpn && f->args() == 1 && str_index > 0 && str[str_index - 1] == SPACE_CH && (str_index + name_length >= (int) str.length() || str[str_index + name_length] != LEFT_PARENTHESIS_CH) && (i6 = str.find_last_not_of(SPACE, str_index - 1)) != (int) string::npos) {
								i5 = str.rfind(SPACE, i6);	
								if(i5 == (int) string::npos) {
									stmp2 = str.substr(0, i6 + 1);	
								} else {
									stmp2 = str.substr(i5 + 1, i6 - i5);
								}
								stmp = LEFT_PARENTHESIS_CH;
								stmp += ID_WRAP_LEFT_CH;
								stmp += i2s(addId(f->parse(stmp2)));
								stmp += ID_WRAP_RIGHT_CH;
								stmp += RIGHT_PARENTHESIS_CH;
								if(i5 == (int) string::npos) {
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
									if(i6 + str_index + name_length >= (int) str.length()) {
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
									stmp = LEFT_PARENTHESIS_CH;
									stmp += ID_WRAP_LEFT_CH;
									stmp += i2s(addId(f->parse(stmp2)));
									stmp += ID_WRAP_RIGHT_CH;
									stmp += RIGHT_PARENTHESIS_CH;
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
										if(i5 == (int) string::npos) {
											str.append(1, RIGHT_PARENTHESIS_CH);
											i5 = str.length() - 1;
										}
										if(i5 < (i6 = str.find(LEFT_PARENTHESIS_CH, i8)) || i6 == (int) string::npos) {
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
									stmp = LEFT_PARENTHESIS_CH;
									stmp += ID_WRAP_LEFT_CH;
									stmp += i2s(addId(f->parse(stmp2)));
									stmp += ID_WRAP_RIGHT_CH;
									stmp += RIGHT_PARENTHESIS_CH;
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
						case 'z': {}
						case 'Y': {
							replace_text_by_unit_place:
							u = (Unit*) ufv[ufv_index];
							if((int) str.length() > str_index + name_length && is_in(NUMBERS, str[str_index + name_length]) && u->baseUnit() != u_euro) {
								str.insert(str_index + name_length, 1, POWER_CH);
							}
							stmp = LEFT_PARENTHESIS_CH;					
							stmp += ID_WRAP_LEFT_CH;
							stmp += i2s(addId(MathStructure(u, p)));
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_PARENTHESIS_CH;				
							str.replace(str_index, name_length, stmp);
							str_index += stmp.length();
							moved_forward = true;
							p = NULL;
							break;
						}
						case 'p': {}
						case 'q': {}
						case 'P': {
							if(str_index + name_length == (int) str.length() || is_in(NOT_IN_NAMES, str[str_index + name_length])) {
								break;
							}
							p = (Prefix*) ufv[ufv_index];
							str_index += name_length;
							chars_left = str.length() - str_index;
							name_length_old = name_length;
							for(int ufv_index2 = 0; ufv_index2 < (int) ufv.size(); ufv_index2++) {
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
									case 'z': {
										name = &((Unit*) ufv[ufv_index2])->unicodeName();
										break;
									}
								}
								if(name) name_length = name->length();
								if(name && (*name)[0] == str[str_index] && (name_length == 1 || (name_length <= chars_left && (*name)[1] == str[str_index + 1] && *name == str.substr(str_index, name_length)))) {
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
			if(!moved_forward && found_function_index >= 0) {
				ufv_index = found_function_index;
				name = &((ExpressionItem*) ufv[ufv_index])->name();
				name_length = found_function_name_length;
				goto set_function;
			}
			if(!moved_forward) {
				if(po.unknowns_enabled && str[str_index] != EXP_CH) {
					int i = 1;
					while(i <= chars_left && str[str_index + i] < 0) {
						i++;
					}
					stmp = LEFT_PARENTHESIS_CH;
					stmp += ID_WRAP_LEFT_CH;
					stmp += i2s(addId(str.substr(str_index, i)));
					stmp += ID_WRAP_RIGHT_CH;
					stmp += RIGHT_PARENTHESIS_CH;
					str.replace(str_index, i, stmp);
					str_index += stmp.length() - 1;	
				}	
			}
		}
	}
	if(po.rpn) {
		unsigned int rpn_i = str.find(SPACE, 0);
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
	return parseOperators(str, po);

}

MathStructure Calculator::parseNumber(string str, const ParseOptions &po) {
	MathStructure mstruct;
	string ssave = str;
	char s = PLUS_CH;
	for(int i = 0; i < (int) str.length() - 1; i++) {
		if(str[i] == PLUS_CH || str[i] == SPACE_CH) {
			str.erase(i, 1);
			i--;
		} else if(str[i] == MINUS_CH) {
			if(s == MINUS_CH)
				s = PLUS_CH;
			else
				s = MINUS_CH;
			str.erase(i, 1);
			i--;
		} else if(is_in(OPERATORS, str[i])) {
			error(false, _("Misplaced '%s' ignored"), str.substr(0, 1).c_str(), NULL);
			str.erase(i, 1);
			i--;
		}
	}
	for(int i = str.length() - 1; i >= 0; i--) {	
		if(is_in(OPERATORS, str[i])) {
			error(false, _("Misplaced '%s' ignored"), str.substr(i, 1).c_str(), NULL);
			str.erase(i, 1);
		}
	}
	if(str[0] == ID_WRAP_LEFT_CH && str[str.length() - 1] == ID_WRAP_RIGHT_CH) {
		int id = s2i(str.substr(1, str.length() - 2));
		mstruct.set(*getId(id));
		if(s == MINUS_CH) mstruct.negate();
		delId(id);
		return mstruct;
	}
	int itmp;
	if(str.empty() || ((itmp = str.find_first_not_of(" ")) == (int) string::npos)) {
		return mstruct;
	}
	if((itmp = str.find_first_not_of(NUMBER_ELEMENTS MINUS, 0)) != (int) string::npos) {
		string stmp = str.substr(itmp, str.length() - itmp);
		str.erase(itmp, str.length() - itmp);
		if(itmp == 0) {
			error(true, _("\"%s\" is not a valid variable/function/unit."), ssave.c_str(), NULL);
			mstruct.set(1, 1);
			return mstruct;
		} else {
			error(true, _("Trailing characters in expression \"%s\" was ignored."), ssave.c_str(), NULL);
		}
	}
	if(s == MINUS_CH) {
		str.insert(str.begin(), 1, MINUS_CH);
	}
	Number nr(str);
	mstruct.set(nr);
	return mstruct;
	
}

void Calculator::parseAdd(string &str, MathStructure &mstruct, const ParseOptions &po) {
	if(str.length() > 0) {
		if(str.find_first_not_of(OPERATORS EXP) != string::npos) {
			if((str.find_first_not_of(NUMBER_ELEMENTS ID_WRAPS, 1) != string::npos && str.find_first_not_of(NUMBER_ELEMENTS ID_WRAPS PLUS MINUS, 0) != 0) || (str.length() > 0 && str[0] == NOT_CH)) {
				mstruct.set(parseOperators(str, po));
			} else {
				mstruct.set(parseNumber(str, po));
			}
		}
	}
}
void Calculator::parseAdd(string &str, MathStructure &mstruct, const ParseOptions &po, MathOperation s) {
	if(str.length() > 0) {
		if(str.find_first_not_of(OPERATORS EXP) != string::npos) {
			if((str.find_first_not_of(NUMBER_ELEMENTS ID_WRAPS, 1) != string::npos && str.find_first_not_of(NUMBER_ELEMENTS ID_WRAPS PLUS MINUS, 0) != 0) || (str.length() > 0 && str[0] == NOT_CH)) {
				mstruct.add(parseOperators(str, po), s);
			} else {
				mstruct.add(parseNumber(str, po), s, true);
			}
		}
	}
}

MathStructure Calculator::parseOperators(string str, const ParseOptions &po) {
	MathStructure mstruct;
	int i = 0, i2 = 0, i3 = 0;
	string str2, str3;
	while(true) {
		//find first right parenthesis and then the last left parenthesis before
		i2 = str.find(RIGHT_PARENTHESIS_CH);
		if(i2 == (int) string::npos) {
			i = str.rfind(LEFT_PARENTHESIS_CH);	
			if(i == (int) string::npos) {
				//if no parenthesis break
				break;
			} else {
				//right parenthesis missing -- append
				str += RIGHT_PARENTHESIS_CH;
				i2 = (int) str.length() - 1;
			}
		} else {
			if(i2 > 0) {
				i = str.rfind(LEFT_PARENTHESIS_CH, i2 - 1);
			} else {
				i = (int) string::npos;
			}
			if(i == (int) string::npos) {
				//left parenthesis missing -- prepend
				str.insert(str.begin(), 1, LEFT_PARENTHESIS_CH);
				i = 0;
				i2++;
			}
		}
		while(true) {
			//remove unnecessary double parenthesis and the found parenthesis
			if(i > 0 && i2 < (int) str.length() - 1 && str[i - 1] == LEFT_PARENTHESIS_CH && str[i2 + 1] == RIGHT_PARENTHESIS_CH) {
				str.erase(str.begin() + (i - 1));
				i--; i2--;
				str.erase(str.begin() + (i2 + 1));
			} else {
				break;
			}
		}
		if(i > 0 && is_in(NUMBER_ELEMENTS ID_WRAPS, str[i - 1])) {
			if(po.rpn) {
				str.insert(i2 + 1, MULTIPLICATION);	
				str.insert(i, SPACE);
				i++;
				i2++;					
			} else {
				str.insert(i, MULTIPLICATION_2);
				i++;
				i2++;
			}
		}
		if(i2 < (int) str.length() - 1 && is_in(NUMBER_ELEMENTS ID_WRAPS, str[i2 + 1])) {
			if(po.rpn) {
				i3 = str.find(SPACE, i2 + 1);
				if(i3 == (int) string::npos) {
					str += MULTIPLICATION;
				} else {
					str.replace(i3, 1, MULTIPLICATION);
				}
				str.insert(i2 + 1, SPACE);
			} else {
				str.insert(i2 + 1, MULTIPLICATION_2);
			}
		}
		if(po.rpn && i > 0 && i2 + 1 == (int) str.length() && is_in(NUMBER_ELEMENTS OPERATORS ID_WRAPS, str[i - 1])) {
			str += MULTIPLICATION_CH;	
		}
		str2 = str.substr(i + 1, i2 - (i + 1));
		mstruct.set(parseOperators(str2, po));
		str2 = ID_WRAP_LEFT_CH;
		str2 += i2s(addId(mstruct));
		str2 += ID_WRAP_RIGHT_CH;
		str.replace(i, i2 - i + 1, str2);
		mstruct.clear();
	}
	if((i = str.find(AND, 1)) != (int) string::npos && i != (int) str.length() - 1) {
		bool b = false;
		while(i != (int) string::npos && i != (int) str.length() - 1) {
			str2 = str.substr(0, i);
			if(str[i + 1] == AND_CH) {
				i++;
				if(i == (int) str.length() - 1) break;
			}
			str = str.substr(i + 1, str.length() - (i + 1));
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_AND);
			} else {
				parseAdd(str2, mstruct, po);
				b = true;
			}
			i = str.find(AND, 1);
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_AND);
		} else {
			parseAdd(str, mstruct, po);
		}
		return mstruct;
	}
	if((i = str.find(OR, 1)) != (int) string::npos && i != (int) str.length() - 1) {
		bool b = false;
		while(i != (int) string::npos && i != (int) str.length() - 1) {
			str2 = str.substr(0, i);
			if(str[i + 1] == OR_CH) {
				i++;
				if(i == (int) str.length() - 1) break;
			}
			str = str.substr(i + 1, str.length() - (i + 1));
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_OR);
			} else {
				parseAdd(str2, mstruct, po);
				b = true;
			}
			i = str.find(OR, 1);
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_OR);
		} else {
			parseAdd(str, mstruct, po);
		}
		return mstruct;
	}	
	if(str[0] == NOT_CH) {
		str.erase(str.begin());
		parseAdd(str, mstruct, po);
		mstruct.setNOT();
		return mstruct;
	}
	if((i = str.find_first_of(LESS GREATER EQUALS NOT, 0)) != (int) string::npos) {
		bool b = false;
		bool c = false;
		while(i != (int) string::npos && str[i] == NOT_CH && (int) str.length() > i + 1 && str[i + 1] == NOT_CH) {
			i++;
			if(i + 1 == (int) str.length()) {
				c = true;
			}
		}
		MathOperation s = OPERATION_ADD;
		while(!c) {
			if(i == (int) string::npos) {
				str2 = str.substr(0, str.length());
			} else {
				str2 = str.substr(0, i);
			}
			if(b) {
				switch(i3) {
					case EQUALS_CH: {s = OPERATION_EQUALS; break;}
					case GREATER_CH: {s = OPERATION_GREATER; break;}
					case LESS_CH: {s = OPERATION_LESS; break;}
					case GREATER_CH * EQUALS_CH: {s = OPERATION_EQUALS_GREATER; break;}
					case LESS_CH * EQUALS_CH: {s = OPERATION_EQUALS_LESS; break;}
					case GREATER_CH * LESS_CH: {s = OPERATION_NOT_EQUALS; break;}
				}
				parseAdd(str2, mstruct, po, s);
			}
			if(i == (int) string::npos) {
				return mstruct;
			}
			if(!b) {
				parseAdd(str2, mstruct, po);
				b = true;
			}
			if((int) str.length() > i + 1 && is_in(LESS GREATER NOT EQUALS, str[i + 1])) {
				if(str[i] == str[i + 1]) {
					i3 = str[i];
				} else {
					i3 = str[i] * str[i + 1];
					if(i3 == NOT_CH * EQUALS_CH) {
						i3 = GREATER_CH * LESS_CH;
					} else if(i3 == NOT_CH * LESS_CH) {
						i3 = GREATER_CH;
					} else if(i3 == NOT_CH * GREATER_CH) {
						i3 = LESS_CH;
					}
				}
				i++;
			} else {
				i3 = str[i];
			}
			str = str.substr(i + 1, str.length() - (i + 1));
			i = str.find_first_of(LESS GREATER NOT EQUALS, 0);
			while(i != (int) string::npos && str[i] == NOT_CH && (int) str.length() > i + 1 && str[i + 1] == NOT_CH) {
				i++;
				if(i + 1 == (int) str.length()) {
					i = (int) string::npos;
				}
			}
		}
	}		
	i = 0;
	i3 = 0;	
	if(po.rpn) {
		ParseOptions po2 = po;
		po2.rpn = false;
		vector<string> stack;
		bool b = false;
		MathOperation s;
		while(true) {
			i = str.find_first_of(OPERATORS EXP SPACE, i3 + 1);
			if(i == (int) string::npos) {
				if(i3 != 0) {
					str2 = str.substr(i3 + 1, str.length() - i3 - 1);
				} else {
					str2 = str.substr(i3, str.length() - i3);
				}
				if(str2.length() > 0) {
					if(b) {
						parseAdd(str2, mstruct, po2, s);
					} else {
						parseAdd(str2, mstruct, po2);
					}
				} else {
					b = false;
				}
				if(b || stack.size() > 0) {
					error(true, _("RPN syntax error."), NULL);
				}
				return mstruct;
			}
			if(i3 != 0) {
				str2 = str.substr(i3 + 1, i - i3 - 1);
			} else {
				str2 = str.substr(i3, i - i3);
			}
			stack.push_back(str2);
			if(str[i] != SPACE_CH) {
				switch(str[i]) {
					case PLUS_CH: {s = OPERATION_ADD; break;}
					case MINUS_CH: {s = OPERATION_SUBTRACT; break;}
					case MULTIPLICATION_CH: {s = OPERATION_MULTIPLY; break;}
					case DIVISION_CH: {s = OPERATION_DIVIDE; break;}
					case POWER_CH: {s = OPERATION_RAISE; break;}
				}
				if((!b && stack.size() < 2) || (b && stack.size() < 1)) {
					error(true, _("RPN syntax error."), NULL);
					if(!b && stack.size() > 0) {
						b = true;
						parseAdd(stack[0], mstruct, po);
					}
				} else {
					for(unsigned int index = 0; index < stack.size(); index++) {
						if(!b) {
							b = true;
							parseAdd(stack[index], mstruct, po);
						} else {
							parseAdd(stack[index], mstruct, po, s);
						}
					}
					stack.clear();
				}
			}
			i3 = i;
		}
	}
	if((i = str.find_first_of(PLUS MINUS, 1)) != (int) string::npos && i != (int) str.length() - 1) {
		bool b = false;
		bool min = false;
		while(i != (int) string::npos && i != (int) str.length() - 1) {
			if(is_not_in(OPERATORS EXP, str[i - 1])) {
				str2 = str.substr(0, i);
				if(b) {
					if(min) {
						parseAdd(str2, mstruct, po, OPERATION_SUBTRACT);
					} else {
						parseAdd(str2, mstruct, po, OPERATION_ADD);
					}
				} else {
					parseAdd(str2, mstruct, po);
					b = true;
				}
				min = str[i] == MINUS_CH;
				str = str.substr(i + 1, str.length() - (i + 1));
				i = str.find_first_of(PLUS MINUS, 1);
			} else {
				i = str.find_first_of(PLUS MINUS, i + 1);
			}
			
		}
		if(b) {
			if(min) {
				parseAdd(str, mstruct, po, OPERATION_SUBTRACT);
			} else {
				parseAdd(str, mstruct, po, OPERATION_ADD);
			}
			return mstruct;
		}
	}
	if((i = str.find_first_of(MULTIPLICATION DIVISION, 1)) != (int) string::npos && i != (int) str.length() - 1) {
		bool b = false;
		bool div = false;
		while(i != (int) string::npos && i != (int) str.length() - 1) {		
			str2 = str.substr(0, i);
			if(b) {
				if(div) {
					parseAdd(str2, mstruct, po, OPERATION_DIVIDE);
				} else {
					parseAdd(str2, mstruct, po, OPERATION_MULTIPLY);
				}
			} else {
				parseAdd(str2, mstruct, po);
				b = true;
			}
			div = str[i] == DIVISION_CH;
			str = str.substr(i + 1, str.length() - (i + 1));
			i = str.find_first_of(MULTIPLICATION DIVISION, 1);
		}
		if(b) {
			if(div) {
				parseAdd(str, mstruct, po, OPERATION_DIVIDE);
			} else {
				parseAdd(str, mstruct, po, OPERATION_MULTIPLY);
			}
		} else {
			parseAdd(str, mstruct, po);
		}
	} else if((i = str.find(MULTIPLICATION_2_CH, 1)) != (int) string::npos && i != (int) str.length() - 1) {
		bool b = false;
		while(i != (int) string::npos && i != (int) str.length() - 1) {
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_MULTIPLY);
			} else {
				parseAdd(str2, mstruct, po);
				b = true;
			}
			i = str.find(MULTIPLICATION_2_CH, 1);
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_MULTIPLY);
		} else {
			parseAdd(str, mstruct, po);
		}
	} else if((i = str.find(POWER_CH, 1)) != (int) string::npos && i != (int) str.length() - 1) {
/*		str2 = str.substr(0, i);
		str = str.substr(i + 1, str.length() - (i + 1));
		parseAdd(str2, mstruct, po);
		parseAdd(str, mstruct, po, OPERATION_RAISE);*/
		bool b = false;
		while(i != (int) string::npos && i != (int) str.length() - 1) {
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_RAISE);
			} else {
				parseAdd(str2, mstruct, po);
				b = true;
			}
			i = str.find(POWER_CH, 1);
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_RAISE);
		} else {
			parseAdd(str, mstruct, po);
		}
	} else if((i = str.find(EXP_CH, 1)) != (int) string::npos && i != (int) str.length() - 1) {
		str2 = str.substr(0, i);
		str = str.substr(i + 1, str.length() - (i + 1));
		parseAdd(str2, mstruct, po);
		parseAdd(str, mstruct, po, OPERATION_EXP10);
	} else {
		return parseNumber(str, po);
	}
	return mstruct;
}

string Calculator::getName(string name, ExpressionItem *object, bool force, bool always_append) {
	ExpressionItem *item = NULL;
	if(!object) {
	} else if(object->type() == TYPE_FUNCTION) {
		item = getActiveFunction(name);
	} else {
		item = getActiveVariable(name);
		if(!item) {
			item = getActiveUnit(name);
		}
		if(!item) {
			item = getCompositeUnit(name);
		}
	}
	if(item && force && !name.empty() && item != object && object) {
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
		return name;
	}
	int i2 = 1;
	bool changed = false;
	if(name.empty()) {
		name = "var";
		always_append = true;
		item = NULL;
		changed = true;
	}
	string stmp = name;
	if(always_append) {
		stmp += NAME_NUMBER_PRE_STR;
		stmp += "1";
	}
	if(changed || (item && item != object)) {
		if(item) {
			i2++;
			stmp = name;
			stmp += NAME_NUMBER_PRE_STR;
			stmp += i2s(i2);
		}
		while(true) {
			if(!object) {
				item = getActiveFunction(stmp);
				if(!item) {
					item = getActiveVariable(stmp);
				}
				if(!item) {
					item = getActiveUnit(stmp);
				}
				if(!item) {
					item = getCompositeUnit(stmp);
				}
			} else if(object->type() == TYPE_FUNCTION) {
				item = getActiveFunction(stmp);
			} else {
				item = getActiveVariable(stmp);
				if(!item) {
					item = getActiveUnit(stmp);
				}
				if(!item) {
					item = getCompositeUnit(stmp);
				}
			}
			if(item && item != object) {
				i2++;
				stmp = name;
				stmp += NAME_NUMBER_PRE_STR;
				stmp += i2s(i2);
			} else {
				break;
			}
		}
	}
	if(i2 > 1 && !always_append) {
		error(false, _("Name \"%s\" is in use. Replacing with \"%s\"."), name.c_str(), stmp.c_str(), NULL);
	}
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
	list<string> eps;
	struct dirent *ep;
	DIR *dp;
	dp = opendir(homedir.c_str());
	if(dp) {
		while((ep = readdir(dp))) {
#ifdef _DIRENT_HAVE_D_TYPE
			if(ep->d_type != DT_DIR) {
#endif
				if(strcmp(ep->d_name, "..") != 0 && strcmp(ep->d_name, ".") != 0) {
					eps.push_back(ep->d_name);
				}
#ifdef _DIRENT_HAVE_D_TYPE			
			}
#endif
		}
		closedir(dp);
	}
	eps.sort();
	for(list<string>::iterator it = eps.begin(); it != eps.end(); ++it) {
		filename = homedir;
		filename += *it;
		loadDefinitions(filename.c_str(), true);
	}
	return true;
}
int Calculator::loadDefinitions(const char* file_name, bool is_user_defs) {

	xmlDocPtr doc;
	xmlNodePtr cur, child, child2;
	string version, stmp, lang_tmp, name, uname, type, svalue, plural, singular, category_title, category, description, title, reverse, base, argname;
	bool best_title, next_best_title, best_category_title, next_best_category_title, best_description, next_best_description;
	bool best_plural, next_best_plural, best_singular, next_best_singular, best_argname, next_best_argname;

	string locale;
	char *clocale = setlocale(LC_MESSAGES, "");
	if(clocale) {
		locale = clocale;
		if(locale == "POSIX" || locale == "C") {
			locale = "";
		}
	}

	string localebase;
	if(locale.length() > 2) {
		localebase = locale.substr(0, 2);
	} else {
		localebase = locale;
	}

	int exponent, litmp;
	bool active, hidden, b;
	Number nr;
	Function *f;
	Variable *v;
	Unit *u;
	AliasUnit *au;
	CompositeUnit *cu;
	Prefix *p;
	Argument *arg;
	int itmp;
	IntegerArgument *iarg;
	NumberArgument *farg;	
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
	ParseOptions po;

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
					uname = "";
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "expression")) {
							XML_DO_FROM_TEXT(child, ((UserFunction*) f)->setEquation);
							XML_GET_APPROX_FROM_PROP(child, b)
							f->setApproximate(b);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
							XML_GET_STRING_FROM_TEXT(child, uname);
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
							} else if(type == "symbol") {
								arg = new SymbolicArgument();
							} else if(type == "date") {
								arg = new DateArgument();
							} else if(type == "integer") {
								iarg = new IntegerArgument();
								arg = iarg;
							} else if(type == "number") {
								farg = new NumberArgument();
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
							} else if(type == "giac") {
								arg = new GiacArgument();
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
										XML_DO_FROM_TEXT(child2, nr.set);
										farg->setMin(&nr);
										XML_GET_FALSE_FROM_PROP(child, "include_equals", b)
										farg->setIncludeEqualsMin(b);
									} else if(iarg) {
										XML_GET_STRING_FROM_TEXT(child2, stmp);
										Number integ(stmp);
										iarg->setMin(&integ);
									}
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "max")) {
									if(farg) {
										XML_DO_FROM_TEXT(child2, nr.set);
										farg->setMax(&nr);
										XML_GET_FALSE_FROM_PROP(child, "include_equals", b)
										farg->setIncludeEqualsMax(b);
									} else if(iarg) {
										XML_GET_STRING_FROM_TEXT(child2, stmp);
										Number integ(stmp);
										iarg->setMax(&integ);
									}
								} else if(farg && !xmlStrcmp(child2->name, (const xmlChar*) "complex_allowed")) {
									XML_GET_FALSE_FROM_TEXT(child2, b);
									farg->setComplexAllowed(b);
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
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "alert")) {
									XML_GET_FALSE_FROM_TEXT(child2, b);
									arg->setAlerts(b);
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
					if(!uname.empty()) f->setUnicodeName(uname);
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
					uname = "";
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
							XML_GET_STRING_FROM_TEXT(child, uname);
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
					if(!uname.empty()) f->setUnicodeName(uname);
					f->setDescription(description);
					f->setTitle(title);
					f->setCategory(category);
					f->setHidden(hidden);
					f->setChanged(false);
					done_something = true;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "unknown")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				if(!name.empty() && variableNameIsValid(name)) {	
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					svalue = "";
					if(name == "x") v = v_x;
					else if(name == "y") v = v_y;
					else if(name == "z") v = v_z;
					else v = addVariable(new UnknownVariable(category, name, "", is_user_defs, false, active));
					done_something = true;
					child = cur->xmlChildrenNode;
					b = true;
					hidden = false;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
					uname = "";
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "type")) {
							XML_GET_STRING_FROM_TEXT(child, stmp);
							if(!((UnknownVariable*) v)->assumptions()) ((UnknownVariable*) v)->setAssumptions(new Assumptions());
							if(stmp == "integer") ((UnknownVariable*) v)->assumptions()->setNumberType(ASSUMPTION_NUMBER_INTEGER);
							else if(stmp == "rational") ((UnknownVariable*) v)->assumptions()->setNumberType(ASSUMPTION_NUMBER_RATIONAL);
							else if(stmp == "real") ((UnknownVariable*) v)->assumptions()->setNumberType(ASSUMPTION_NUMBER_REAL);
							else if(stmp == "complex") ((UnknownVariable*) v)->assumptions()->setNumberType(ASSUMPTION_NUMBER_COMPLEX);
							else if(stmp == "number") ((UnknownVariable*) v)->assumptions()->setNumberType(ASSUMPTION_NUMBER_NUMBER);
							else if(stmp == "none") ((UnknownVariable*) v)->assumptions()->setNumberType(ASSUMPTION_NUMBER_NONE);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "sign")) {
							XML_GET_STRING_FROM_TEXT(child, stmp);
							if(!((UnknownVariable*) v)->assumptions()) ((UnknownVariable*) v)->setAssumptions(new Assumptions());
							if(stmp == "non-zero") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NONZERO);
							else if(stmp == "non-positive") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NONPOSITIVE);
							else if(stmp == "negative") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NEGATIVE);
							else if(stmp == "non-negative") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NONNEGATIVE);
							else if(stmp == "positive") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_POSITIVE);
							else if(stmp == "unknown") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_UNKNOWN);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_DO_FROM_TEXT(child, v->setDescription);
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
							XML_GET_STRING_FROM_TEXT(child, uname);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						}
						child = child->next;
					}
					if(!uname.empty()) v->setUnicodeName(uname);
					v->setDescription(description);
					v->setTitle(title);
					v->setHidden(hidden);
					v->setChanged(false);
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "variable")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)
				if(!name.empty() && variableNameIsValid(name)) {	
					XML_GET_FALSE_FROM_PROP(cur, "active", active)
					svalue = "";
					v = addVariable(new KnownVariable(category, name, "", "", is_user_defs, false, active));
					done_something = true;
					child = cur->xmlChildrenNode;
					b = true;
					hidden = false;
					title = ""; best_title = false; next_best_title = false;
					description = ""; best_description = false; next_best_description = false;
					uname = "";
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "value")) {
							XML_DO_FROM_TEXT(child, ((KnownVariable*) v)->set);
							XML_GET_APPROX_FROM_PROP(child, b);
							v->setApproximate(b);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_DO_FROM_TEXT(child, v->setDescription);
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
							XML_GET_STRING_FROM_TEXT(child, uname);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						}
						child = child->next;
					}
					if(!uname.empty()) v->setUnicodeName(uname);
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
					uname = "";
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
							XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
							XML_GET_STRING_FROM_TEXT(child, uname);
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
							XML_GET_TRUE_FROM_TEXT(child, hidden);
						}
						child = child->next;
					}		
					if(!uname.empty()) v->setUnicodeName(uname);
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
						uname = "";
						hidden = false;
						description = ""; best_description = false; next_best_description = false;
						while(child != NULL) {
							if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {	
								XML_GET_TRUE_FROM_TEXT(child, hidden);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {	
								XML_GET_LOCALE_STRING_FROM_TEXT(child, title, best_title, next_best_title);
							} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
								XML_GET_STRING_FROM_TEXT(child, uname);
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
						u = addUnit(new Unit(category, name, plural, singular, title, is_user_defs, false, active, uname));		
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
					uname = "";
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
									XML_GET_APPROX_FROM_PROP(child2, b)
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "reverse_relation")) {
									XML_GET_STRING_FROM_TEXT(child2, reverse);
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "exponent")) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);
									if(stmp.empty()) {
										exponent = 1;
									} else {
										exponent = s2i(stmp);
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
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
							XML_GET_STRING_FROM_TEXT(child, uname);
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
						au = new AliasUnit(category, name, plural, singular, title, u, svalue, exponent, reverse, is_user_defs, false, active, uname);
						au->setDescription(description);
						au->setApproximate(b);
						au->setHidden(hidden);
						addUnit(au);
						au->setChanged(false);
						done_something = true;
						if(name == "rad") u_rad = au;
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
										litmp = s2i(svalue);
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
											exponent = s2i(stmp);
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
					uname = "";;
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
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
							XML_GET_STRING_FROM_TEXT(child, uname);
							if(!unitNameIsValid(uname)) {
								uname = "";
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
					if(!uname.empty()) {
						u->setUnicodeName(uname);
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
				uname = "";
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "name")) {
						XML_GET_STRING_FROM_TEXT(child, name);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "abbreviation")) {	
						XML_GET_STRING_FROM_TEXT(child, stmp);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {	
						XML_GET_STRING_FROM_TEXT(child, uname);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "exponent")) {	
						XML_GET_STRING_FROM_TEXT(child, svalue);
					}
					child = child->next;
				}
				addPrefix(new Prefix(s2i(svalue), name, stmp, uname));
				done_something = true;
			}
			cur = NULL;
			if(in_unfinished) {
				if(done_something) {
					in_unfinished--;
					unfinished_nodes.erase(unfinished_nodes.begin() + in_unfinished);
					unfinished_cats.erase(unfinished_cats.begin() + in_unfinished);
				}
				if((int) unfinished_nodes.size() > in_unfinished) {
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
			unsigned int cat_i = category.rfind("/");
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
	xmlNodePtr cur, newnode;	
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);	
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	cur = doc->children;
	for(unsigned int i = 0; i < prefixes.size(); i++) {
		newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "prefix", NULL);
		if(!prefixes[i]->longName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "name", (xmlChar*) prefixes[i]->longName(false).c_str());
		if(!prefixes[i]->shortName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "abbreviation", (xmlChar*) prefixes[i]->shortName(false).c_str());
		if(!prefixes[i]->unicodeName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "unicode", (xmlChar*) prefixes[i]->unicodeName(false).c_str());
		xmlNewTextChild(newnode, NULL, (xmlChar*) "exponent", (xmlChar*) i2s(prefixes[i]->exponent()).c_str());
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
	node_tree_item top;
	top.category = "";
	top.node = doc->children;
	node_tree_item *item;
	string cat, cat_sub;
	for(unsigned int i = 0; i < variables.size(); i++) {
		if((save_global || variables[i]->isLocal() || variables[i]->hasChanged()) && variables[i]->category() != _("Temporary")) {
			item = &top;
			if(!variables[i]->category().empty()) {
				cat = variables[i]->category();
				unsigned int cat_i = cat.find("/"); int cat_i_prev = -1;
				bool b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(unsigned int i2 = 0; i2 < item->items.size(); i2++) {
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
					if(variables[i]->isKnown()) {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_variable", NULL);
					} else {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_unknown", NULL);
					}
				} else {
					if(variables[i]->isKnown()) {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "variable", NULL);
					} else {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "unknown", NULL);
					}
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
				if(!variables[i]->unicodeName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "unicode", (xmlChar*) variables[i]->unicodeName(false).c_str());
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
					if(variables[i]->isKnown()) {
						if(((KnownVariable*) variables[i])->isExpression()) {
							newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) ((KnownVariable*) variables[i])->expression().c_str());
						} else {
							newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) ((KnownVariable*) variables[i])->get().print(save_printoptions).c_str());
						}
						if(variables[i]->isApproximate()) xmlNewProp(newnode2, (xmlChar*) "approximate", (xmlChar*) "true");
					} else {
						if(((UnknownVariable*) variables[i])->assumptions()) {
							switch(((UnknownVariable*) variables[i])->assumptions()->numberType()) {
								case ASSUMPTION_NUMBER_INTEGER: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "integer");
									break;
								}
								case ASSUMPTION_NUMBER_RATIONAL: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "rational");
									break;
								}
								case ASSUMPTION_NUMBER_REAL: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "real");
									break;
								}
								case ASSUMPTION_NUMBER_COMPLEX: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "complex");
									break;
								}
								case ASSUMPTION_NUMBER_NUMBER: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "number");
									break;
								}
								case ASSUMPTION_NUMBER_NONE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "none");
									break;
								}
							}
							switch(((UnknownVariable*) variables[i])->assumptions()->sign()) {
								case ASSUMPTION_SIGN_NONZERO: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "non-zero");
									break;
								}
								case ASSUMPTION_SIGN_NONPOSITIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "non-positive");
									break;
								}
								case ASSUMPTION_SIGN_NEGATIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "negative");
									break;
								}
								case ASSUMPTION_SIGN_NONNEGATIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "non-negative");
									break;
								}
								case ASSUMPTION_SIGN_POSITIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "positive");
									break;
								}
								case ASSUMPTION_SIGN_UNKNOWN: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "unknown");
									break;
								}
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

int Calculator::saveUnits(const char* file_name, bool save_global) {
	unsetLocale();	
	string str;
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");	
	xmlNodePtr cur, newnode, newnode2, newnode3;	
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);	
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	CompositeUnit *cu;
	AliasUnit *au;
	node_tree_item top;
	top.category = "";
	top.node = doc->children;
	node_tree_item *item;
	string cat, cat_sub;
	for(unsigned int i = 0; i < units.size(); i++) {
		if(save_global || units[i]->isLocal() || units[i]->hasChanged()) {	
			item = &top;
			if(!units[i]->category().empty()) {
				cat = units[i]->category();
				unsigned int cat_i = cat.find("/"); int cat_i_prev = -1;
				bool b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(unsigned int i2 = 0; i2 < item->items.size(); i2++) {
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
						if(!units[i]->unicodeName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "unicode", (xmlChar*) units[i]->unicodeName(false).c_str());
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
						for(unsigned int i2 = 0; i2 < cu->units.size(); i2++) {
							newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "part", NULL);
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "unit", (xmlChar*) cu->units[i2]->firstBaseUnit()->referenceName().c_str());
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "prefix", (xmlChar*) i2s(cu->units[i2]->prefixExponent()).c_str());
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "exponent", (xmlChar*) i2s(cu->units[i2]->firstBaseExp()).c_str());
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
						if(!units[i]->unicodeName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "unicode", (xmlChar*) units[i]->unicodeName(false).c_str());
					}
					if(units[i]->unitType() == ALIAS_UNIT) {
						newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "base", NULL);
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "unit", (xmlChar*) au->firstBaseUnit()->referenceName().c_str());								
						newnode3 = xmlNewTextChild(newnode2, NULL, (xmlChar*) "relation", (xmlChar*) au->expression().c_str());
						if(units[i]->isApproximate()) xmlNewProp(newnode3, (xmlChar*) "approximate", (xmlChar*) "true");
						if(!au->reverseExpression().empty()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "reverse_relation", (xmlChar*) au->reverseExpression().c_str());	
						}
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "exponent", (xmlChar*) i2s(au->firstBaseExp()).c_str());
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
	NumberArgument *farg;
	string str;
	for(unsigned int i = 0; i < functions.size(); i++) {
		if(save_global || functions[i]->isLocal() || functions[i]->hasChanged()) {	
			item = &top;
			if(!functions[i]->category().empty()) {
				cat = functions[i]->category();
				unsigned int cat_i = cat.find("/"); int cat_i_prev = -1;
				bool b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(unsigned int i2 = 0; i2 < item->items.size(); i2++) {
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
					if(!functions[i]->unicodeName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "unicode", (xmlChar*) functions[i]->unicodeName(false).c_str());
					if(!functions[i]->description().empty()) {
						str = functions[i]->description();
						if(save_global) {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
						} else {
							xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
						}
					}
					cur = newnode;
					for(unsigned int i2 = 1; i2 <= functions[i]->lastArgumentDefinitionIndex(); i2++) {
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
					if(functions[i]->isApproximate()) xmlNewProp(newnode2, (xmlChar*) "approximate", (xmlChar*) "true");
					if(!functions[i]->condition().empty()) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "condition", (xmlChar*) functions[i]->condition().c_str());
					}
					cur = newnode;
					for(unsigned int i2 = 1; i2 <= functions[i]->lastArgumentDefinitionIndex(); i2++) {
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
								case ARGUMENT_TYPE_SYMBOLIC: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "symbol"); break;}
								case ARGUMENT_TYPE_DATE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "date"); break;}
								case ARGUMENT_TYPE_INTEGER: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "integer"); break;}
								case ARGUMENT_TYPE_NUMBER: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "number"); break;}
								case ARGUMENT_TYPE_VECTOR: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "vector"); break;}
								case ARGUMENT_TYPE_MATRIX: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "matrix"); break;}
								case ARGUMENT_TYPE_BOOLEAN: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "boolean"); break;}
								case ARGUMENT_TYPE_FUNCTION: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "function"); break;}
								case ARGUMENT_TYPE_UNIT: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "unit"); break;}
								case ARGUMENT_TYPE_VARIABLE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "variable"); break;}
								case ARGUMENT_TYPE_EXPRESSION_ITEM: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "object"); break;}
								case ARGUMENT_TYPE_ANGLE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "angle"); break;}
								case ARGUMENT_TYPE_GIAC: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "giac"); break;}
								default: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "free");}
							}
							xmlNewProp(newnode, (xmlChar*) "index", (xmlChar*) i2s(i2).c_str());
							if(!arg->tests()) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "test", (xmlChar*) "false");
							}
							if(!arg->alerts()) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "alert", (xmlChar*) "false");
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
										xmlNewTextChild(newnode, NULL, (xmlChar*) "min", (xmlChar*) iarg->min()->print(save_printoptions).c_str()); 
									}
									if(iarg->max()) {
										xmlNewTextChild(newnode, NULL, (xmlChar*) "max", (xmlChar*) iarg->max()->print(save_printoptions).c_str()); 
									}
									break;
								}
								case ARGUMENT_TYPE_NUMBER: {
									farg = (NumberArgument*) arg;
									if(farg->min()) {
										newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "min", (xmlChar*) farg->min()->print(save_printoptions).c_str()); 
										if(farg->includeEqualsMin()) {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "true");
										} else {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "false");
										}
									}
									if(farg->max()) {
										newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "max", (xmlChar*) farg->max()->print(save_printoptions).c_str()); 
										if(farg->includeEqualsMax()) {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "true");
										} else {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "false");
										}
									}
									if(!farg->complexAllowed()) {
										xmlNewTextChild(newnode, NULL, (xmlChar*) "complex_allowed", (xmlChar*) "false");
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

MathStructure Calculator::setAngleValue(const MathStructure &mstruct) {
	bool b = false;
	if(mstruct.isUnit_exp()) {
		b = true;
	} else if(mstruct.isMultiplication()) {
		for(unsigned int i = 0; i < mstruct.countChilds(); i++) {
			if(mstruct.getChild(i)->isUnit_exp()) {
				b = true;
				break;
			}
		}
	}
	MathStructure mstruct_new(mstruct);
	if(!b) {
		switch(angleMode()) {
			case DEGREES: {
		    		mstruct_new *= v_pi;
	    			mstruct_new /= 180;
				break;
			}
			case GRADIANS: {
				mstruct_new *= v_pi;
	    			mstruct_new /= 200;
				break;
			}
		}
	} else {
		mstruct_new /= getUnit("rad");
		//mstruct_new.finalize();
	}
	return mstruct_new;
}

bool Calculator::importCSV(MathStructure &mstruct, const char *file_name, int first_row, string delimiter, vector<string> *headers) {
	FILE *file = fopen(file_name, "r");
	if(file == NULL) {
		return false;
	}
	if(first_row < 1) {
		first_row = 1;
	}
	char line[10000];
	string stmp, str1, str2;
	int row = 0, rows = 1;
	int columns = 1;
	int column;
	mstruct = m_empty_matrix;
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
					while((is_n = stmp.find(delimiter, is)) != (int) string::npos) {		
						columns++;
						if(headers) {
							str1 = stmp.substr(is, is_n - is);
							remove_blank_ends(str1);
							headers->push_back(str1);
						}
						is = is_n + delimiter.length();
					}
					if(headers) {
						str1 = stmp.substr(is, stmp.length() - is);
						remove_blank_ends(str1);
						headers->push_back(str1);
					}
					mstruct.resizeMatrix(1, columns, m_undefined);
				}
			}
			if((!headers || row > first_row) && !stmp.empty()) {
				is = 0;
				column = 1;
				if(v_added) {
					mstruct.addRow(m_undefined);
					rows++;
				}
				while(column <= columns) {
					is_n = stmp.find(delimiter, is);
					if(is_n == (int) string::npos) {
						str1 = stmp.substr(is, stmp.length() - is);
					} else {
						str1 = stmp.substr(is, is_n - is);
						is = is_n + delimiter.length();
					}
					mstruct[rows - 1][column - 1] = CALCULATOR->parse(str1);
					column++;
					if(is_n == (int) string::npos) {
						break;
					}
				}
				v_added = true;
			}
		}
	}
	return true;
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
	unsigned int i = filestr.find_last_of("/");
	if(i != string::npos) {
		filestr = filestr.substr(i + 1, filestr.length() - (i + 1));
	}
	if(name.empty()) {
		i = filestr.find_last_of(".");
		name = filestr.substr(0, i);
	}
	char line[10000];
	string stmp, str1, str2;
	int row = 0;
	int columns = 1, rows = 1;
	int column;
	vector<string> header;
	vector<MathStructure> vectors;
	MathStructure mstruct = m_empty_matrix;
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
					while((is_n = stmp.find(delimiter, is)) != (int) string::npos) {		
						columns++;
						if(headers) {
							str1 = stmp.substr(is, is_n - is);
							remove_blank_ends(str1);
							header.push_back(str1);
						}
						if(!to_matrix) {
							vectors.push_back(m_empty_vector);
						}
						is = is_n + delimiter.length();
					}
					if(headers) {
						str1 = stmp.substr(is, stmp.length() - is);
						remove_blank_ends(str1);
						header.push_back(str1);
					}
					if(to_matrix) {
						mstruct.resizeMatrix(1, columns, m_undefined);
					} else {
						vectors.push_back(m_empty_vector);
					}
				}
			}
			if((!headers || row > first_row) && !stmp.empty()) {
				if(to_matrix && v_added) {
					mstruct.addRow(m_undefined);
					rows++;
				}
				is = 0;
				column = 1;
				while(column <= columns) {
					is_n = stmp.find(delimiter, is);
					if(is_n == (int) string::npos) {
						str1 = stmp.substr(is, stmp.length() - is);
					} else {
						str1 = stmp.substr(is, is_n - is);
						is = is_n + delimiter.length();
					}
					if(to_matrix) {
						mstruct[rows - 1][column - 1] = CALCULATOR->parse(str1);
					} else {
						vectors[column - 1].addItem(CALCULATOR->parse(str1));
					}
					column++;
					if(is_n == (int) string::npos) {
						break;
					}
				}
				for(; column <= columns; column++) {
					if(!to_matrix) {
						vectors[column - 1].addItem(m_undefined);
					}				
				}
				v_added = true;
			}
		}
	}
	if(to_matrix) {
		addVariable(new KnownVariable(category, name, mstruct, title));
	} else {
		if(vectors.size() > 1) {
			if(!category.empty()) {
				category += "/";	
			}
			category += name;
		}
		for(unsigned int i = 0; i < vectors.size(); i++) {
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
			addVariable(new KnownVariable(category, str1, vectors[i], str2));
		}
	}
	return true;
}
int Calculator::testCondition(string expression) {
	MathStructure mstruct = calculate(expression);
	if(mstruct.isNumber()) {
		if(mstruct.number().isPositive()) {
			return 1;
		} else {
			return 0;
		}
	}
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
	Unit *u;
	while(cur) {
		if(!xmlStrcmp(cur->name, (const xmlChar*) "Cube")) {
			XML_GET_STRING_FROM_PROP(cur, "currency", currency);
			if(!currency.empty()) {
				XML_GET_STRING_FROM_PROP(cur, "rate", rate);
				if(!rate.empty()) {
					rate = "1/" + rate;
					u = getUnit(currency);
					if(!u) {
						addUnit(new AliasUnit(_("Currency"), currency, "", "", "", u_euro, rate, 1, "", false, true));
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
	exchange_rates_warning_issued = false;
	return true;
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
bool Calculator::checkExchangeRatesDate() {
	if(exchange_rates_warning_issued) return true;
	string homedir = "";
	struct passwd *pw = getpwuid(getuid());
	if(pw) {
		homedir = pw->pw_dir;
		homedir += "/";
	}
	homedir += ".qalculate/";
	homedir += "eurofxref-daily.xml";
	bool up_to_date = false;
	struct stat stats;
	if(stat(homedir.c_str(), &stats) == 0) {
		if(time(NULL) - stats.st_mtime <= 604800) {
			up_to_date = true;
		}
	}
	if(!up_to_date) {
		error(false, _("It has been more than one week since the exchange rates last were updated."));
		exchange_rates_warning_issued = true;
	}
	return up_to_date;
}

bool Calculator::canPlot() {
	FILE *pipe = popen("gnuplot -", "w");
	if(!pipe) {
		return false;
	}
	if(pclose(pipe) != 0) return false;
	pipe = popen("gnuplot -", "w");
	if(!pipe) {
		return false;
	}
	fputs("show version\n", pipe);
	return pclose(pipe) == 0;
}
MathStructure Calculator::expressionToPlotVector(string expression, const MathStructure &min, const MathStructure &max, int steps, MathStructure *x_vector, string x_var) {
	/*if(x_var[0] == '\\') {
		string x_var_sub = "\"";
		x_var_sub += x_var;
		x_var_sub += "\"";
		gsub(x_var, x_var_sub, expression);	
	}*/
	Variable *v = getVariable(x_var);
	MathStructure x_mstruct;
	if(v) x_mstruct = v;
	else x_mstruct = x_var;
	EvaluationOptions eo;
	eo.approximation = APPROXIMATION_APPROXIMATE;
	return parse(expression).generateVector(x_mstruct, min, max, steps, x_vector, eo);
}
MathStructure Calculator::expressionToPlotVector(string expression, float min, float max, int steps, MathStructure *x_vector, string x_var) {
	MathStructure min_mstruct(min), max_mstruct(max);
	EvaluationOptions eo;
	eo.approximation = APPROXIMATION_APPROXIMATE;
	return expressionToPlotVector(expression, min_mstruct, max_mstruct, steps, x_vector, x_var).eval(eo);
}
MathStructure Calculator::expressionToPlotVector(string expression, const MathStructure &x_vector, string x_var) {
	/*if(x_var[0] == '\\') {
		string x_var_sub = "\"";
		x_var_sub += x_var;
		x_var_sub += "\"";
		gsub(x_var, x_var_sub, expression);		
	}*/
	Variable *v = getVariable(x_var);
	MathStructure x_mstruct;
	if(v) x_mstruct = v;
	else x_mstruct = x_var;
	EvaluationOptions eo;
	eo.approximation = APPROXIMATION_APPROXIMATE;
	return parse(expression).generateVector(x_mstruct, x_vector, eo).eval(eo);
}
/*bool Calculator::plotVectors(plot_parameters *param, const MathStructure *y_vector, ...) {

	plot_data_parameters *pdp;
	vector<MathStructure*> y_vectors;
	vector<MathStructure*> x_vectors;
	vector<plot_data_parameters*> pdps;
	const MathStructure *v;
	y_vectors.push_back(y_vector);
	va_list ap;
	va_start(ap, y_vector); 
	while(true) {
		v = va_arg(ap, const MathStructure*);
		if(v == NULL) break;
		x_vectors.push_back(v);
		pdp = va_arg(ap, plot_data_parameters*);
		if(pdp == NULL) break;
		pdps.push_back(pdp);
		v = va_arg(ap, const MathStructure*);
		if(v == NULL) break;
		y_vectors.push_back(v);
	}
	va_end(ap);	

	return plotVectors(param, y_vectors, x_vectors, pdps);
}*/

bool Calculator::plotVectors(plot_parameters *param, const vector<MathStructure> &y_vectors, const vector<MathStructure> &x_vectors, vector<plot_data_parameters*> &pdps, bool persistent) {

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
	string title;

	if(!param) {
		plot_parameters pp;
		param = &pp;
	}
	
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
			unsigned int i = param->filename.find(".");
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
			default: {
				plot += "png ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
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
		title = param->x_label;
		gsub("\"", "\\\"", title);
		plot += "set xlabel \"";
		plot += title;
		plot += "\"\n";	
	}
	if(!param->y_label.empty()) {
		string title = param->y_label;
		gsub("\"", "\\\"", title);
		plot += "set ylabel \"";
		plot += title;
		plot += "\"\n";	
	}
	if(!param->title.empty()) {
		title = param->title;
		gsub("\"", "\\\"", title);
		plot += "set title \"";
		plot += title;
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
		for(unsigned int i = 0; i < pdps.size(); i++) {
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
	for(unsigned int i = 0; i < y_vectors.size(); i++) {
		if(!y_vectors[i].isUndefined()) {
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
					default: {}
				}
				if(pdps[i]->xaxis2 && pdps[i]->yaxis2) {
					plot += " axis x2y2";
				} else if(pdps[i]->xaxis2) {
					plot += " axis x2y1";
				} else if(pdps[i]->yaxis2) {
					plot += " axis x1y2";
				}
				if(!pdps[i]->title.empty()) {
					title = pdps[i]->title;
					gsub("\"", "\\\"", title);
					plot += " title \"";
					plot += title;
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
	
	string filename_data;
	string plot_data;
	PrintOptions po;
	po.number_fraction_format = FRACTION_DECIMAL;
	for(unsigned int serie = 0; serie < y_vectors.size(); serie++) {
		if(!y_vectors[serie].isUndefined()) {
			filename_data = homedir;
			filename_data += "gnuplot_data";
			filename_data += i2s(serie + 1);
			FILE *fdata = fopen(filename_data.c_str(), "w+");
			if(!fdata) {
				error(true, _("Could not create temporary file %s"), filename_data.c_str(), NULL);
				return false;
			}
			plot_data = "";
			for(unsigned int i = 1; i <= y_vectors[serie].components(); i++) {
				if(serie < x_vectors.size() && !x_vectors[serie].isUndefined() && x_vectors[serie].components() == y_vectors[serie].components()) {
					plot_data += x_vectors[serie].getComponent(i)->print(po);
					plot_data += " ";
				}
				plot_data += y_vectors[serie].getComponent(i)->print(po);
				plot_data += "\n";	
				if(getDecimalPoint() != ".") {
					gsub(getDecimalPoint(), ".", plot_data);
				}
			}
			fputs(plot_data.c_str(), fdata);
			fflush(fdata);
			fclose(fdata);
		}
	}
	
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

