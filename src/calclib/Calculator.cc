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
#include <fenv.h>
#include <locale.h>

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

Calculator::Calculator() {

	addStringAlternative(SIGN_POWER_0, "o");
	addStringAlternative(SIGN_POWER_1, "^1");
	addStringAlternative(SIGN_POWER_2, "^2");
	addStringAlternative(SIGN_POWER_3, "^3");
	addStringAlternative(SIGN_EURO, "euro");
	addStringAlternative(SIGN_MICRO, "micro");
	addStringAlternative("[", LEFT_BRACKET);	
	addStringAlternative("]", RIGHT_BRACKET);	
	addStringAlternative(";", COMMA);	
	addStringAlternative("\t", SPACE);	
	addStringAlternative("\n", SPACE);	


	ID_WRAP_LEFT_STR = "{";
	ID_WRAP_RIGHT_STR = "}";	
	ID_WRAP_S = "{}";	
	DOT_STR = ".";
	COMMA_STR = ",";
	DOT_S = ".";
	COMMA_S = ",;";
	NUMBERS_S = "0123456789";
	SIGNS_S = "+-*/^E";
	OPERATORS_S = "+-*/^E";
	BRACKETS_S = "()[]";
	LEFT_BRACKET_S = "([";
	LEFT_BRACKET_STR = "(";
	RIGHT_BRACKET_S = ")]";
	RIGHT_BRACKET_STR = ")";
	SPACE_S = " \t\n";
	SPACE_STR = " ";
	RESERVED_S = "@?!\\{}&':<>|\"";
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
	setlocale(LC_NUMERIC, "C");
	setLocale();
	
	ILLEGAL_IN_NAMES = (char*) malloc(sizeof(char) * (strlen(RESERVED_S) + strlen(OPERATORS_S) + strlen(SPACE_S) + strlen(DOT_S) + strlen(BRACKETS_S) + 1));
	sprintf(ILLEGAL_IN_NAMES, "%s%s%s%s%s", RESERVED_S, OPERATORS_S, SPACE_S, DOT_S, BRACKETS_S);
	ILLEGAL_IN_NAMES_MINUS_SPACE_STR = (char*) malloc(sizeof(char) * (strlen(RESERVED_S) + strlen(OPERATORS_S) + strlen(DOT_S) + strlen(BRACKETS_S) + 1));
	sprintf(ILLEGAL_IN_NAMES_MINUS_SPACE_STR, "%s%s%s%s\t\n", RESERVED_S, OPERATORS_S, DOT_S, BRACKETS_S);	
	ILLEGAL_IN_UNITNAMES = (char*) malloc(sizeof(char) * (strlen(ILLEGAL_IN_NAMES) + strlen(NUMBERS_S) + 1));
	sprintf(ILLEGAL_IN_UNITNAMES, "%s%s", ILLEGAL_IN_NAMES, NUMBERS_S);			
	
	srand48(time(0));
	angleMode(RADIANS);
	addBuiltinVariables();
	addBuiltinFunctions();
	addBuiltinUnits();
	b_functions = true;
	b_variables = true;
	b_units = true;
	b_unknown = true;
}
Calculator::~Calculator(void) {}

const char *Calculator::getDecimalPoint() const {return DOT_STR;}
const char *Calculator::getComma() const {return COMMA_STR;}	
void Calculator::setLocale() {
/*	lconv *locale = localeconv();
	DOT_STR = locale->decimal_point;
	COMMA_STR = ",";
	DOT_S = DOT_STR;	
	if(strcmp(DOT_STR, COMMA_STR) == 0) {
		COMMA_STR = ";";
		COMMA_S = ";";
	} else {
		COMMA_S = ",;";	
	}*/
}
void Calculator::unsetLocale() {
/*	COMMA_STR = ",";
	COMMA_S = ",;";	
	DOT_STR = ".";
	DOT_S = DOT_STR;*/
}
string &Calculator::remove_trailing_zeros(string &str, int decimals_to_keep, bool expand, bool decrease) {
	int i2 = str.find_first_of(DOT_S);
	if(i2 != string::npos) {
		string str2 = "";
		int i4 = str.find_first_not_of(NUMBERS_S, i2 + 1);
		if(i4 != string::npos) {
			str2 = str.substr(i4, str.length() - i4);
			str = str.substr(0, i4);
		}
		int expands = 0;
		int decimals = str.length() - i2 - 1;
		int i3 = 1;
		while(str[str.length() - i3] == ZERO_CH) {
			i3++;
		}
		i3--;
		int dtk = decimals_to_keep;
		if(decimals_to_keep) {
			decimals_to_keep = decimals_to_keep - (decimals - i3);
			if(decimals_to_keep > 0) {
				expands = decimals_to_keep - i3;
				if(expands < 0)
					expands = 0;
				i3 -= decimals_to_keep;
				if(i3 < 0)
					i3 = 0;
			}
		}
		if(expand && expands) {
			while(expands > 0) {
				str += ZERO_STR;
				expands--;
			}
		} else {
			if(is_in(str[str.length() - i3], DOT_S, NULL))
				i3++;
			if(i3) {
				str = str.substr(0, str.length() - i3);
			}
		}
		if(decrease) {
			if(dtk > 0) {
				if(i2 + dtk + 1 < str.length()) {
					if(str.length() > i2 + dtk + 1 && str[i2 + dtk + 1] >= FIVE_CH) {
						i3 = dtk;
						while(i3 + i2 >= 0) {
							if(str[i2 + i3] == NINE_CH)  {
								str[i2 + i3] = ZERO_CH;
								if(i3 + i2 == 0) {
									str.insert(0, 1, ONE_CH);
									i2++;
									break;
								}
							} else {
								str[i2 + i3] = str[i2 + i3] + 1;
								break;
							}
							i3--;
							if(i3 == 0)
								i3--;
						}
					}
					str = str.substr(0, i2 + dtk + 1);
				}
			} else {
				if(str.length() > i2 + 1 && str[i2 + 1] >= FIVE_CH) {
					i3 = i2 - 1;
					while(i3 >= 0) {
						if(str[i3] == NINE_CH)  {
							str[i3] = ZERO_CH;
							if(i3 == 0) {
								str.insert(0, 1, ONE_CH);
								i2++;
								break;
							}
						} else {
							str[i3] = str[i3] + 1;
							break;
						}
						i3--;
					}
				}
				str = str.substr(0, i2);
			}
		}
		if(is_in(str[str.length() - 1], DOT_S, NULL))
			str = str.substr(0, str.length() - 1);
		str += str2;
	} else if(expand) {
		string str2 = "";
		int i4 = str.find_first_not_of(NUMBERS_S, 0);
		if(i4 != string::npos) {
			str2 = str.substr(i4, str.length() - i4);
			str = str.substr(0, i4);
		}
		if(decimals_to_keep > 0)
			str += DOT_STR;
		while(decimals_to_keep > 0) {
			str += ZERO_STR;
			decimals_to_keep--;
		}
		str += str2;
	}
	return str;
}

int Calculator::addId(Manager *mngr) {
	for(int i = 0; ; i++) {
		if(!ids.count(i)) {
			ids[i] = mngr;
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
void Calculator::delId(int id) {
	if(ids.count(id)) {
		ids[id]->unref();
		ids.erase(id);
	}
}
bool Calculator::functionsEnabled(void) {
	return b_functions;
}
void Calculator::setFunctionsEnabled(bool enable) {
	b_functions = enable;
}
bool Calculator::variablesEnabled(void) {
	return b_variables;
}
void Calculator::setVariablesEnabled(bool enable) {
	b_variables = enable;
}
bool Calculator::unknownVariablesEnabled(void) {
	return b_unknown;
}
void Calculator::setUnknownVariablesEnabled(bool enable) {
	b_unknown = enable;
}
bool Calculator::unitsEnabled(void) {
	return b_units;
}
void Calculator::setUnitsEnabled(bool enable) {
	b_units = enable;
}
int Calculator::angleMode(void) {
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
	addVariable(new Variable(this, "General", "pi", PI_VALUE, "", false, true));
	addVariable(new Variable(this, "General", "e", E_VALUE, "", false, true));
}
void Calculator::addBuiltinFunctions() {
	addFunction(new IFFunction(this));
	addFunction(new DifferentiateFunction(this));	
	addFunction(new GCDFunction(this));	
	addFunction(new AbsFunction(this));
	addFunction(new CeilFunction(this));
	addFunction(new FloorFunction(this));
	addFunction(new TruncFunction(this));
	addFunction(new RoundFunction(this));
	addFunction(new ModFunction(this));
	addFunction(new RemFunction(this));
	addFunction(new SinFunction(this));
	addFunction(new CosFunction(this));
	addFunction(new TanFunction(this));
	addFunction(new SinhFunction(this));
	addFunction(new CoshFunction(this));
	addFunction(new TanhFunction(this));
	addFunction(new AsinFunction(this));
	addFunction(new AcosFunction(this));
	addFunction(new AtanFunction(this));
	addFunction(new AsinhFunction(this));
	addFunction(new AcoshFunction(this));
	addFunction(new AtanhFunction(this));
	addFunction(new LogFunction(this));
	addFunction(new Log2Function(this));
	addFunction(new Log10Function(this));
	addFunction(new ExpFunction(this));
	addFunction(new Exp2Function(this));
	addFunction(new Exp10Function(this));
	addFunction(new SqrtFunction(this));
	addFunction(new CbrtFunction(this));
	addFunction(new HypotFunction(this));
	addFunction(new SumFunction(this));
	addFunction(new MeanFunction(this));
	addFunction(new MedianFunction(this));
	addFunction(new MinFunction(this));
	addFunction(new MaxFunction(this));
	addFunction(new ModeFunction(this));
	addFunction(new NumberFunction(this));
	addFunction(new StdDevFunction(this));
	addFunction(new StdDevSFunction(this));
	addFunction(new RandomFunction(this));
	addFunction(new BASEFunction(this));
	addFunction(new BINFunction(this));
	addFunction(new OCTFunction(this));
	addFunction(new HEXFunction(this));
}
void Calculator::addBuiltinUnits() {
}
void Calculator::error(bool critical, const char *TEMPLATE, ...) {
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
Error* Calculator::error(void) {
	if(!errors.empty()) {
		return errors.top();
	}
	return NULL;
}
Error* Calculator::nextError(void) {
	if(!errors.empty()) {
		errors.pop();
		if(!errors.empty()) {
			return errors.top();
		}
	}
	return NULL;
}
void Calculator::deleteName(string name_, void *object) {
	Variable *v2 = getVariable(name_);
	if(v2 == object)
		return;
	if(v2 != NULL) {
		delVariable(v2);
	} else {
		Function *f2 = getFunction(name_);
		if(f2 == object)
			return;
		if(f2 != NULL) {
			delFunction(f2);
		}
	}
}
void Calculator::deleteUnitName(string name_, Unit *object) {
	Unit *u2 = getUnit(name_);
	if(u2) {
		if(u2 != object) {
			delUnit(u2);
		}
		return;
	} 
	u2 = getCompositeUnit(name_);	
	if(u2) {
		if(u2 != object) {
			delUnit(u2);
		}
	}
}
Manager *Calculator::calculate(string str) {
	setFunctionsAndVariables(str);
	EqContainer *e = new EqContainer(str, this, PLUS_CH);
	Manager *mngr = e->calculate();
	mngr->finalize();
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
	mngr->add(from_unit, MULTIPLICATION_CH);
	from_unit->hasComplexRelationTo(to_unit);
	mngr->convert(to_unit);
	mngr->finalize();	
//	mngr->convert(to_unit);
	mngr->add(to_unit, DIVISION_CH);
	mngr->finalize();	
	return mngr;
}
Manager *Calculator::convert(Manager *mngr, Unit *to_unit, bool always_convert) {
	if(to_unit->type() == 'D') return convertToCompositeUnit(mngr, (CompositeUnit*) to_unit, always_convert);
	if(to_unit->type() != 'A' || (((AliasUnit*) to_unit)->baseUnit()->type() != 'D' && ((AliasUnit*) to_unit)->baseExp() == 1.0L)) {
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
		if(mngr->convert(to_unit) || always_convert) {
			mngr->add(to_unit, DIVISION_CH);
			mngr->finalize();			
			Manager *mngr2 = new Manager(this, to_unit);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				mngr->mngrs.push_back(mngr2);
			} else {
				mngr->transform(mngr2, MULTIPLICATION_MANAGER, MULTIPLICATION_CH);
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
		if(mngr->convert(cu) || always_convert) {	
			mngr->add(mngr3, DIVISION_CH);
			mngr->finalize();			
			Manager *mngr2 = new Manager(this, cu);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				mngr->mngrs.push_back(mngr2);
			} else {
				mngr->transform(mngr2, MULTIPLICATION_MANAGER, MULTIPLICATION_CH);
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
	CompositeUnit *cu = new CompositeUnit(this, "", "temporary_composite_convert", "", composite_);
	convertToCompositeUnit(mngr, cu);
	return mngr;			
}
Unit* Calculator::addUnit(Unit *u, bool force) {
	if(u->type() == 'D') {
		u->name(getUnitName(((CompositeUnit*) u)->internalName(), u, force));		
	} else {
		u->name(getUnitName(u->name(), u, force));
		if(u->hasPlural()) {
			u->plural(getUnitName(u->plural(), u, force));
		}
		if(u->hasShortName()) {
			u->shortName(getUnitName(u->shortName(), u, force));
		}
	}
	units.push_back(u);
	u->setChanged(false);
	return u;
}
void Calculator::delUnit(Unit *u) {
	for(vector<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
		if(*it == u) {
			units.erase(it);
			break;
		}
	}
	delUFV((void*) u);
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
		if(units[i]->type() != 'D' && (units[i]->name() == name_ || units[i]->shortName() == name_ || units[i]->plural() == name_)) {
			return units[i];
		}
	}
	return NULL;
}
Unit* Calculator::getCompositeUnit(string internal_name_) {
	for(int i = 0; i < (int) units.size(); i++) {
		if(units[i]->type() == 'D' && ((CompositeUnit*) units[i])->internalName() == internal_name_) {
			return units[i];
		}
	}
	return NULL;
}

Variable* Calculator::addVariable(Variable *v, bool force) {
	variables.push_back(v);
	v->name(getName(v->name(), (void*) v, force));
	v->setChanged(false);
	return v;
}
void Calculator::variableNameChanged(Variable *v) {
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
				l = ((Unit*) (*it))->plural().length();
			else if(ufv_t[i] == 'u')
				l = ((Unit*) (*it))->shortName().length();
			else if(ufv_t[i] == 'p')
				l = 1;
			else if(ufv_t[i] == 'P')
				l = strlen((const char*) (*it));
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
				l = ((Unit*) (*it))->plural().length();
			else if(ufv_t[i] == 'u')
				l = ((Unit*) (*it))->shortName().length();
			else if(ufv_t[i] == 'p')
				l = 1;
			else if(ufv_t[i] == 'P')
				l = strlen((const char*) (*it));
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
	if(u->type() == 'D')
		return;
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
				l = ((Unit*) (*it))->plural().length();
			else if(ufv_t[i] == 'u')
				l = ((Unit*) (*it))->shortName().length();
			else if(ufv_t[i] == 'p')
				l = 1;
			else if(ufv_t[i] == 'P')
				l = strlen((const char*) (*it));
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
	if(u->hasPlural()) {
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
					l = ((Unit*) (*it))->plural().length();
				else if(ufv_t[i] == 'u')
					l = ((Unit*) (*it))->shortName().length();
				else if(ufv_t[i] == 'p')
					l = 1;
				else if(ufv_t[i] == 'P')
					l = strlen((const char*) (*it));
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('Y');
				break;
			} else if(l <= u->plural().length()) {
				ufv.insert(it, (void*) u);
				ufv_t.insert(ufv_t.begin() + i, 'Y');
				break;
			}
			i++;
		}
	}
	if(u->hasShortName()) {
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
					l = ((Unit*) (*it))->plural().length();
				else if(ufv_t[i] == 'u')
					l = ((Unit*) (*it))->shortName().length();
				else if(ufv_t[i] == 'p')
					l = 1;
				else if(ufv_t[i] == 'P')
					l = strlen((const char*) (*it));
			}
			if(it == ufv.end()) {
				ufv.push_back((void*) u);
				ufv_t.push_back('u');
				break;
			} else if(l <= u->shortName().length()) {
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

void Calculator::delVariable(Variable *v) {
	for(vector<Variable*>::iterator it = variables.begin(); it != variables.end(); ++it) {
		if(*it == v) {
			variables.erase(it);
			break;
		}
	}
	delUFV((void*) v);
}
Variable* Calculator::getVariable(string name_) {
	for(int i = 0; i < (int) variables.size(); i++) {
		if(variables[i]->name() == name_) {
			return variables[i];
		}
	}
	return NULL;
}
Function* Calculator::addFunction(Function *f, bool force) {
	f->name(getName(f->name(), (void*) f, force));
	functions.push_back(f);
	f->setChanged(false);
	return f;
}
void Calculator::delFunction(Function *f) {
	for(vector<Function*>::iterator it = functions.begin(); it != functions.end(); ++it) {
		if(*it == f) {
			functions.erase(it);
			break;
		}
	}
	delUFV((void*) f);
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
	if(name_.find_first_of(ILLEGAL_IN_NAMES) != string::npos)
		return false;
	//		if(name_.substr(0, 3) == "INF") return false;
	//		if(name_.substr(0, 3) == "NAN") return false;
	int i = name_.find_first_of(NUMBERS_S);
	int i2 = i;
	if(i == string::npos)
		return true;
	if(i == 0)
		return false;
	while(is_in(name_[i - 1], NAME_NUMBER_PRE_S, NULL)) {
		i = name_.find_first_of(NUMBERS_S, i + 1);
		if(i == string::npos)
			return true;
		while(i == i2 + 1) {
			i2 = i;
			i = name_.find_first_of(NUMBERS_S, i + 1);
			if(i == string::npos)
				return true;
		}
		i2 = i;
	}
	return false;
}
bool Calculator::functionNameIsValid(string name_) {
	return variableNameIsValid(name_);
}
bool Calculator::unitNameIsValid(string name_) {
	if(name_.find_first_of(ILLEGAL_IN_UNITNAMES) != string::npos)
		return false;
	return true;
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
	i = name_.find_first_of(NUMBERS_S);
	int i2 = i;
	while(i != string::npos) {
		if(i == 0) {
			name_.erase(name_.begin());
			i--;
			i2 = -2;
		} else if(is_not_in(name_[i - 1], NAME_NUMBER_PRE_S, NULL)) {
			name_.insert(i, NAME_NUMBER_PRE_STR);
			i++;
			i2++;
		}
		i = name_.find_first_of(NUMBERS_S, i + 1);
		if(i == string::npos)
			return name_;
		while(i == i2 + 1) {
			i2 = i;
			i = name_.find_first_of(NUMBERS_S, i + 1);
			if(i == string::npos)
				return name_;
		}
		i2 = i;
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
bool Calculator::nameTaken(string name_, void *object) {
	Function *f = getFunction(name_);
	if(f && f != object)
		return true;
	Variable *v = getVariable(name_);
	return v && v != object;
}
bool Calculator::unitNameTaken(string name_, Unit *unit) {
	Unit *u = getUnit(name_);
	if(u) return u != unit;
	u = getCompositeUnit(name_);
	return u && u != unit;	
}
bool Calculator::unitIsUsedByOtherUnits(Unit *u) {
	Unit *u2;
	for(int i = 0; i < units.size(); i++) {
		if(units[i] != u) {
			u2 = units[i];
			while(u2->type() == 'A') {
				u2 = ((AliasUnit*) u2)->firstBaseUnit();
				if(u2 == u)
					return true;
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
	Unit *u;
	char *ch;
	vector<int> uss;
	vector<int> ues;
	vector<char> ut;
	string stmp, stmp2;
	long double value;
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
			mngr = new Manager(this, stmp);
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
					stmp += i2s(addId(v->get()));
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
						i4 = f->name().length();
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
							while(1) {
								if((i5 = str.find(RIGHT_BRACKET_CH, i7)) != (int) string::npos) {
									if(i5 < (i6 = str.find(LEFT_BRACKET_CH, i8)) || i6 == string::npos) {
										i6 = i5;
										b = true;
										break;
									}
									i7 = i5 + 1;
									i8 = i6 + 1;
								} else {
									break;
								}
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
			ch = (char*) ufv[i2];
			while(1) {
				if((i = str.find(ch, i3)) != (int) string::npos) {
					i4 = i + strlen(ch) - 1;
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
									i7 = ((Unit*) ufv[i6])->shortName().length();
								} else if(ufv_t[i6] == 'U') {
									i7 = ((Unit*) ufv[i6])->name().length();
								} else if(ufv_t[i6] == 'Y') {
									i7 = ((Unit*) ufv[i6])->plural().length();
								} else {
									i7 = -1;
								}
								if(i7 > 0 && i7 <= i5 - i4) {
									b = false;
									for(i8 = 1; i8 <= i7; i8++) {
										if((ufv_t[i6] == 'u' && str[i4 + i8] != ((Unit*) ufv[i6])->shortName()[i8 - 1]) || (ufv_t[i6] == 'U' && str[i4 + i8] != ((Unit*) ufv[i6])->name()[i8 - 1]) || (ufv_t[i6] == 'Y' && str[i4 + i8] != ((Unit*) ufv[i6])->plural()[i8 - 1])) {
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
					if(!b)
					if(ufv_t[i2] == 'p') {
						if(!b || i == str.length() - 1 || is_not_in(BRACKETS OPERATORS, *ch)) {
							stmp = LEFT_BRACKET_CH;
							stmp += ID_WRAP_LEFT_CH;
							if(b) mngr = new Manager(this, s_prefix[*ch]);
							else mngr = new Manager(this, u, s_prefix[*ch]);
							stmp += i2s(addId(mngr));
							mngr->unref();
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_BRACKET_CH;
							if(!b) str.replace(i, 1 + i7, stmp);
							else str.replace(i, 1, stmp);
						} else {
							//stmp += ch;
							i3 = i + 1;
						}
					} else {
						stmp = LEFT_BRACKET_CH;
						stmp += ID_WRAP_LEFT_CH;
						if(b) mngr = new Manager(this, l_prefix[ch]);
						else mngr = new Manager(this, u, l_prefix[ch]);
						stmp += i2s(addId(mngr));
						mngr->unref();
						stmp += ID_WRAP_RIGHT_CH;
						stmp += RIGHT_BRACKET_CH;
						if(!b) str.replace(i, strlen(ch) + i7, stmp);
						else str.replace(i, strlen(ch), stmp);						
					}
					else i3 = i + 1;
				} else {
					break;
				}
			}
		} else if(b_units && (ufv_t[i2] == 'u' || ufv_t[i2] == 'U' || ufv_t[i2] == 'Y')) {
			u = (Unit*) ufv[i2];
			while(u->type() != 'D') {
				find_unit:
				value = 1;
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
						if(i5 == i - 1) {
							for(hash_map<char, long double>::iterator it = s_prefix.begin(); it != s_prefix.end(); ++it) {
								if(str[i5] == it->first) {
									value = it->second;
									i--;
									break;
								}
							}
						} else {
							stmp = str.substr(i5, i - i5);
							for(l_type::iterator it = l_prefix.begin(); it != l_prefix.end(); ++it) {
								i7 = strlen(it->first);
								if(i7 <= i - i5) {
									b = true;
									for(i6 = 1; i6 <= i7; i6++) {
										if(str[i - i6] != it->first[i7 - i6]) {
											b = false;
											break;
										}
									}
									if(b) {
										value = it->second;
										i -= i7;
										break;
									}
								}
							}
						}
					}
					if(str.length() > i4 + 1 && is_in(NUMBERS, str[i4 + 1])) {
						str.insert(i4 + 1, 1, POWER_CH);
					}
					mngr = new Manager(this, u, value);
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
	remove_blanks(str);
	i = 0;
	while(true) {
		i = str.find_first_not_of(NUMBERS NOT_IN_NAMES DOT, i);
		if(i == string::npos) break;
		stmp = str[i];
		u = getUnit(stmp);
		if(u) {
			mngr = new Manager(this, u);
		} else if(b_unknown) {
			mngr = new Manager(this, stmp);
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
}
string Calculator::getName(string name, void *object, bool force, bool always_append) {
	if(force) {
		deleteName(name, object);
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
		deleteUnitName(name, object);
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
		if(units[i] != object && ((units[i]->type() == 'D' && ((CompositeUnit*) units[i])->internalName() == stmp) || (units[i]->type() != 'D' && (units[i]->name() == stmp || units[i]->shortName() == stmp || units[i]->plural() == stmp)))) {
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
	for(int i = 0; i < variables.size(); i++) {
		if(variables[i]->isUserVariable()) {
			if(!variables[i]->isBuiltinVariable()) 
				fprintf(file, "*Variable\t");
			else
				fprintf(file, "*BuiltinVariable\t");
			if(variables[i]->isBuiltinVariable())
				fprintf(file, "%s\t", variables[i]->name().c_str());
			if(variables[i]->category().empty())
				fprintf(file, "0\t");
			else
				fprintf(file, "%s\t", variables[i]->category().c_str());
			if(!variables[i]->isBuiltinVariable())
				fprintf(file, "%s\t%s\t", variables[i]->name().c_str(), variables[i]->get()->print().c_str());
			if(variables[i]->title().empty())
				fprintf(file, "0\t");
			else
				fprintf(file, "%s\t", variables[i]->title().c_str());
			fprintf(file, "\n");
		}
	}
	fprintf(file, "\n");
	for(int i = 0; i < functions.size(); i++) {
		if(functions[i]->isUserFunction()) {	
			if(!functions[i]->isBuiltinFunction())
				fprintf(file, "*Function\t");
			else
				fprintf(file, "*BuiltinFunction\t");
			if(functions[i]->isBuiltinFunction())
				fprintf(file, "%s\t", functions[i]->name().c_str());
			if(functions[i]->category().empty())
				fprintf(file, "0\t");
			else
				fprintf(file, "%s\t", functions[i]->category().c_str());
			if(!functions[i]->isBuiltinFunction())
				fprintf(file, "%s\t%s\t", functions[i]->name().c_str(), ((UserFunction*) functions[i])->equation().c_str());
			if(functions[i]->title().empty())
				fprintf(file, "0\t");
			else
				fprintf(file, "%s\t", functions[i]->title().c_str());
			str = functions[i]->description();
			gsub("\n", "\\", str);
			if(str.empty())
				fprintf(file, "0");
			else
				fprintf(file, "%s", str.c_str());
			for(int i2 = 1; !functions[i]->argName(i2).empty(); i2++) {
				fprintf(file, "\t%s", functions[i]->argName(i2).c_str());
			}
			fprintf(file, "\n");
		}	
	}
	fprintf(file, "\n");
	CompositeUnit *cu;
	AliasUnit *au;
	Unit *u;
	int exp = 1;
	for(int i = 0; i < units.size(); i++) {
		if(units[i]->isUserUnit()) {
			switch(units[i]->type()) {
				case 'U': {
					fprintf(file, "*Unit\t");
					break;
				}
				case 'A': {
					au = (AliasUnit*) units[i];
					fprintf(file, "*AliasUnit\t");
					break;
				}
				case 'D': {
					fprintf(file, "*CompositeUnit\t");
					break;
				}
			}
			if(units[i]->category().empty())
				fprintf(file, "0\t");
			else
				fprintf(file, "%s\t", units[i]->category().c_str());
			if(units[i]->type() == 'D') {
				cu = (CompositeUnit*) units[i];			
				fprintf(file, "%s\t", cu->internalName().c_str());
				if(units[i]->title().empty())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s", units[i]->title().c_str());
				for(int i2 = 0; i2 < cu->units.size(); i2++) {
//					fprintf(file, "\t%s\t%s\t%LG", cu->units[cu->sorted[i2]]->firstBaseUnit()->shortName().c_str(), cu->units[cu->sorted[i2]]->firstBaseExp()->print().c_str(), cu->units[cu->sorted[i2]]->prefixValue());
					fprintf(file, "\t%s\t%s\t%LG", cu->units[i2]->firstBaseUnit()->shortName().c_str(), d2s(cu->units[i2]->firstBaseExp()).c_str(), cu->units[i2]->prefixValue());
				}
			} else {
				fprintf(file, "%s\t", units[i]->name().c_str());
				if(!units[i]->hasPlural())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s\t", units[i]->plural().c_str());
				if(!units[i]->hasShortName())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s\t", units[i]->shortName().c_str());
				if(units[i]->title().empty())
					fprintf(file, "0\t");
				else
					fprintf(file, "%s", units[i]->title().c_str());
			}
			if(units[i]->type() == 'A') {
//				fprintf(file, "\t%s\t%s\t%s", au->firstShortBaseName().c_str(), au->expression().c_str(), au->firstBaseExp()->print().c_str());
				if(au->firstBaseUnit()->type() == 'D') {
					fprintf(file, "\t%s\t%s\t%s", ((CompositeUnit*) (au->firstBaseUnit()))->internalName().c_str(), au->expression().c_str(), d2s(au->firstBaseExp()).c_str());
				} else {
					fprintf(file, "\t%s\t%s\t%s", au->firstShortBaseName().c_str(), au->expression().c_str(), d2s(au->firstBaseExp()).c_str());
				}	
				if(!au->reverseExpression().empty())
					fprintf(file, "\t%s", au->reverseExpression().c_str());
			}
			fprintf(file, "\n");
		}
	}
/*	fprintf(file, "\n");
	for(l_type::iterator it = l_prefix.begin(); it != l_prefix.end(); ++it) {
		fprintf(file, "*Prefix\t%s\t%LG\n", it->first, it->second);
	}
	for(hash_map<char, long double>::iterator it = s_prefix.begin(); it != s_prefix.end(); ++it) {
		fprintf(file, "*Prefix\t%c\t%LG\n", it->first, it->second);
	}*/
	fclose(file);
	setLocale();
}
bool Calculator::load(const char* file_name, bool is_user_defs) {
	FILE *file = fopen(file_name, "r");
	if(file == NULL)
		return false;
	unsetLocale();
	string stmp, str, ntmp, vtmp, rtmp, etmp, shtmp, ctmp, ttmp, cutmp;
	Unit *u;
	AliasUnit *au;
	Variable *v;
	Function *func;
	Manager *mngr;
	bool b;
	unsigned int i, i2, i3;
	long double value;
	char line[10000];
	while(1) {
		if(fgets(line, 10000, file) == NULL)
			break;
		if(line[0] == '*') {
			stmp = line;
			if((i = stmp.find_first_of("\t")) != string::npos) {
				str = stmp.substr(0, i);
				if(str == "*Variable") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ctmp = stmp.substr(i, i2 - i);
						if(ctmp == "0")
							ctmp = "";
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ntmp = stmp.substr(i, i2 - i);
							if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
								vtmp = stmp.substr(i, i2 - i);
								ttmp = "";
								if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
									ttmp = stmp.substr(i, i2 - i);
									if(ttmp == "0")
										ttmp = "";
								}
								if(variableNameIsValid(ntmp)) {
									mngr = calculate(vtmp);
									addVariable(new Variable(this, ctmp, ntmp, mngr, ttmp, is_user_defs));
									mngr->unref();
								}
							}
						}
					}
				} else if(str == "*BuiltinVariable") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ntmp = stmp.substr(i, i2 - i);
						v = getVariable(ntmp);
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ctmp = stmp.substr(i, i2 - i);
							if(ctmp == "0")
								ctmp = "";
							if(v)
								v->category(ctmp);
							ttmp = "";
							if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
								ttmp = stmp.substr(i, i2 - i);
								if(ttmp == "0")
									ttmp = "";
								if(v)
									v->title(ttmp);
							}
						}
						if(v) {
							v->setChanged(false);
							v->setUserVariable(is_user_defs);
						}
					}
				} else if(str == "*Function") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ctmp = stmp.substr(i, i2 - i);
						if(ctmp == "0")
							ctmp = "";
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ntmp = stmp.substr(i, i2 - i);
							if(functionNameIsValid(ntmp) && ((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos)) {
								vtmp = stmp.substr(i, i2 - i);
								func = addFunction(new UserFunction(this, ctmp, ntmp, vtmp, is_user_defs));
								if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
									shtmp = stmp.substr(i, i2 - i);
									if(shtmp == "0")
										shtmp = "";
									func->title(shtmp);
									if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
										shtmp = stmp.substr(i, i2 - i);
										gsub("\\", "\n", shtmp);
										if(shtmp == "0")
											shtmp = "";
										func->description(shtmp);
										while(1) {
											if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
												func->addArgName(stmp.substr(i, i2 - i));
											} else {
												break;
											}
										}
									}
								}
								func->setChanged(false);
							}
						}
					}
				} else if(str == "*BuiltinFunction") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ntmp = stmp.substr(i, i2 - i);
						func = getFunction(ntmp);
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ctmp = stmp.substr(i, i2 - i);
							if(ctmp == "0")
								ctmp = "";
							if(func)
								func->category(ctmp);
							if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
								shtmp = stmp.substr(i, i2 - i);
								if(shtmp == "0")
									shtmp = "";
								if(func)
									func->title(shtmp);
								if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
									shtmp = stmp.substr(i, i2 - i);
									gsub("\\", "\n", shtmp);
									if(shtmp == "0")
										shtmp = "";
									if(func)
										func->description(shtmp);
									b = true;										
									while(1) {
										if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
											if(func && b) {
												func->clearArgNames();
												b = false;
											}
											if(func)
												func->addArgName(stmp.substr(i, i2 - i));
											else
												break;
										} else {
											break;
										}
									}
								}
							}
						}
						if(func) {
							func->setChanged(false);
							func->setUserFunction(is_user_defs);
						}
					}
				} else if(str == "*Unit") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ctmp = stmp.substr(i, i2 - i);
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ntmp = stmp.substr(i, i2 - i);
							shtmp = "";
							ttmp = "";
							etmp = "";
							if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
								etmp = stmp.substr(i, i2 - i);
								if(etmp == "0")
									etmp = "";
								if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
									shtmp = stmp.substr(i, i2 - i);
									if(shtmp == "0")
										shtmp = "";
									if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
										ttmp = stmp.substr(i, i2 - i);
										if(ttmp == "0")
											ttmp = "";
									}
								}
							}
							if(unitNameIsValid(ntmp) && unitNameIsValid(etmp) && unitNameIsValid(shtmp))
								addUnit(new Unit(this, ctmp, ntmp, etmp, shtmp, ttmp, is_user_defs));
						}
					}
				} else if(str == "*CompositeUnit") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ctmp = stmp.substr(i, i2 - i);
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ntmp = stmp.substr(i, i2 - i);
							ttmp = "";
							if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
								ttmp = stmp.substr(i, i2 - i);
								if(ttmp == "0")
									ttmp = "";
								CompositeUnit* cu = NULL;
								i3 = 0;
								if(unitNameIsValid(ntmp)) {
									while(1) {
										if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
											vtmp = stmp.substr(i, i2 - i);
											if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
												cutmp = stmp.substr(i, i2 - i);
												if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
													rtmp = stmp.substr(i, i2 - i);
													u = getUnit(vtmp);
													if(!u) u = getCompositeUnit(vtmp);
													if(u) {
														if(i3 == 0)
															cu = new CompositeUnit(this, ctmp, ntmp, ttmp);
														if(cu) {
															mngr = calculate(cutmp);
															//cu->add(u, mngr, strtold(rtmp.c_str(), NULL));
															//Manager *mngr2 = calculate(rtmp);
															cu->add(u, mngr->value(), strtold(rtmp.c_str(), NULL));
															//cu->add(u, mngr->value(), mngr2->value());
															mngr->unref();
															//mngr2->unref();
														}
													}
												} else
													break;
											} else
												break;
										} else
											break;
										i3++;
									}
									if(cu) {
										addUnit(cu);
										cu->setUserUnit(is_user_defs);
										cu->setChanged(false);
									}
								}
							}						
						}
					}
				} else if(str == "*AliasUnit") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ctmp = stmp.substr(i, i2 - i);
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ntmp = stmp.substr(i, i2 - i);
							shtmp = "";
							ttmp = "";
							etmp = "";
							if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
								etmp = stmp.substr(i, i2 - i);
								if(etmp == "0")
									etmp = "";
								if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
									shtmp = stmp.substr(i, i2 - i);
									if(shtmp == "0")
										shtmp = "";
									if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
										ttmp = stmp.substr(i, i2 - i);
										if(ttmp == "0")
											ttmp = "";
										if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
											vtmp = stmp.substr(i, i2 - i);
											u = getUnit(vtmp);
											if(!u) {
												u = getCompositeUnit(vtmp);
											}
											if((unitNameIsValid(ntmp) && unitNameIsValid(etmp) && unitNameIsValid(shtmp)) && u) {
												au = new AliasUnit(this, ctmp, ntmp, etmp, shtmp, ttmp, u);
												if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
													au->expression(stmp.substr(i, i2 - i));
													if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
														//mngr = calculate(stmp.substr(i, i2 - i));
														//au->exp(mngr);
														//au->exp(mngr->value());
														au->exp(strtold(stmp.substr(i, i2 - i).c_str(), NULL));
														//mngr->unref();
														if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
															au->reverseExpression(stmp.substr(i, i2 - i));
														}
													}
												}
												addUnit(au);
												au->setUserUnit(is_user_defs);
												au->setChanged(false);
											}
										}
									}
								}
							}
						}
					}
				} else if(str == "*Prefix") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ntmp = stmp.substr(i, i2 - i);
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							vtmp = stmp.substr(i, i2 - i);
							//mngr = calculate(vtmp);
							addPrefix(ntmp, strtold(vtmp.c_str(), NULL));
							//addPrefix(ntmp, mngr->value());
							//mngr->unref();
						}
					}
				}
			}
		}
	}
	fclose(file);
	setLocale();
	return true;
}
bool Calculator::getPrefix(const char *str, long double *value) {
	if(strlen(str) == 1)
		return getPrefix(str[0], value);
	if(l_prefix.count(str) > 0) {
		if(value)
			*value = l_prefix[str];
		return true;
	}
	return false;
}
bool Calculator::getPrefix(const string &str, long double *value) {
	return getPrefix(str.c_str(), value);
}
bool Calculator::getPrefix(char c, long double *value) {
	if(s_prefix.count(c) > 0) {
		if(value)
			*value = s_prefix[c];
		return true;
	}
	return false;
}
char Calculator::getSPrefix(long double value) {
	for(s_type::iterator it = s_prefix.begin(); it != s_prefix.end(); ++it) {
		if(it->second == value)
			return it->first;
	}
	return 0;
}
const char *Calculator::getLPrefix(long double value) {
	for(l_type::iterator it = l_prefix.begin(); it != l_prefix.end(); ++it) {
		if(it->second == value)
			return it->first;
	}
	return NULL;
}

void Calculator::addPrefix(const string &ntmp, long double value) {
	char *str = (char*) malloc(sizeof(char) * ntmp.length() + 1);
	char c;
	strcpy(str, ntmp.c_str());
	if(ntmp.length() == 1) {
		s_prefix[ntmp[0]] = value;
		c = 'p';
	} else {
		l_prefix[(const char*) str] = value;
		c = 'P';
	}
	int l, i = 0;
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
				l = ((Unit*) (*it))->plural().length();
			else if(ufv_t[i] == 'u')
				l = ((Unit*) (*it))->shortName().length();
			else if(ufv_t[i] == 'p')
				l = 1;
			else if(ufv_t[i] == 'P')
				l = strlen((const char*) (*it));
		}
		if(it == ufv.end()) {
			ufv.push_back((void*) str);
			ufv_t.push_back(c);
			break;
		} else if(l < strlen(str) || (l == strlen(str) && ufv_t[i] != 'u' && ufv_t[i] != 'U' && ufv_t[i] != 'Y')) {
			ufv.insert(it, (void*) str);
			ufv_t.insert(ufv_t.begin() + i, c);
			break;
		}
		i++;
	}

}

string Calculator::value2str(long double &value, int precision) {
	sprintf(vbuffer, "%.*LG", precision, value);
	string stmp = vbuffer;
	return stmp;
}
string Calculator::value2str_decimals(long double &value, int precision) {
	sprintf(vbuffer, "%.*Lf", precision, value);
	string stmp = vbuffer;
	return stmp;
}
string Calculator::value2str_bin(long double &value, int precision) {
	if(value < 0) {
		error(false, "Conversion to binary cannot handle negative numbers, showing decimal value.", NULL);
		return value2str(value, precision);	
	}
	if(value > 10000000) {
		error(false, "Conversion to binary cannot handle such large numbers, showing decimal value.", NULL);
		return value2str(value, precision);
	}	
	string stmp;
	long int val = lroundl(value);
	long int mask = ((long int) pow(2, (((long int) log2(val)) / 8 + 1) * 8)) / 2;
	int i = 0;
	while(mask) {
		if(i == 4) {
			i = 0;
			stmp += " ";
		}
		i++;
		if(mask & val) stmp += "1";
		else stmp += "0";
		mask /= 2;
	}
	return stmp;
}
string Calculator::value2str_octal(long double &value, int precision) {
	if(value < 0) {
		error(false, "Conversion to octal cannot handle negative numbers, showing decimal value.", NULL);
		return value2str(value, precision);	
	}
	if(value > 2000000000) {
		error(false, "Conversion to octal cannot handle such large numbers, showing decimal value.", NULL);
		return value2str(value, precision);
	}
	sprintf(vbuffer, "%#lo", lroundl(value));
	string stmp = vbuffer;
	return stmp;
}
string Calculator::value2str_hex(long double &value, int precision) {
	if(value < 0) {
		error(false, "Conversion to hexadecimal cannot handle negative numbers, showing decimal value.", NULL);
		return value2str(value, precision);	
	}
	if(value > 2000000000) {
		error(false, "Conversion to hexadecimal cannot handle such large numbers, showing decimal value.", NULL);
		return value2str(value, precision);
	}
	sprintf(vbuffer, "%#lx", lroundl(value));
	string stmp = vbuffer;
	return stmp;
}

string Calculator::value2str_prefix(long double &value, long double &exp, int precision, bool use_short_prefixes, long double *new_value, long double prefix_) {
	long double d1;
	if(prefix_ >= 0.0L) {
		string str = "";
		if(use_short_prefixes) {
			char c = getSPrefix(prefix_);
			if(!c) {
				str = getLPrefix(prefix_);
			} else {
				str = c;
			}
		} else {
			str = getLPrefix(prefix_);
			if(str.empty()) {
				char c = getSPrefix(prefix_);
				if(!c) {
					str = "";
				} else {
					str = c;
				}
			}		
		}		
		if(!str.empty()) {
			d1 = powl(prefix_, exp);
			d1 = value / d1;
			string str2 = value2str(d1, precision);
			str2 += ' ';
			str2 += str;
			if(new_value)
				*new_value = d1;
			return str2;
		}
		if(new_value)
			*new_value = value;	
		return value2str(value, precision);		
	}
	long double d2, d3;
	hash_map<char, long double>::iterator it, itt;
	l_type::iterator it2, itt2;
	if(value == 1.0L || value == 0.0L) {
		if(new_value) 
			*new_value = value;
		return value2str(value, precision);
	}
	for(it = s_prefix.begin(); it != s_prefix.end(); ++it) {
		d1 = log10l(value / powl(it->second, exp));
		if(d1 < 0) {
			d1 = -(d1 * 2) + 2;				
		}
		if(it == s_prefix.begin() || d1 < d2) {
			itt = it;
			d2 = d1;
		}
	}
	for(it2 = l_prefix.begin(); it2 != l_prefix.end(); ++it2) {
		d1 = log10l(value / powl(it2->second, exp));
		if(d1 < 0) {
			d1 = -(d1 * 2) + 2;				
		}
		if(it2 == l_prefix.begin() || d1 < d3) {
			itt2 = it2;
			d3 = d1;
		}
	}
	if(itt2->second == itt->second) {
	} else if(d3 < d2) {
		use_short_prefixes = false;
	} else {
		use_short_prefixes = true;
	}
	string str;
	if(use_short_prefixes) {
		d1 = powl(itt->second, exp);
		str = itt->first;
	} else {
		d1 = powl(itt2->second, exp);
		str = itt2->first;
	}
	d1 = value / d1;
	if((value > 1 && value > d1) || (value < 1 && value < d1)) {
		string str2 = value2str(d1, precision);
		str2 += ' ';
		str2 += str;
		if(new_value)
			*new_value = d1;
		return str2;
	}
	if(new_value)
		*new_value = value;	
	return value2str(value, precision);
}

string Calculator::value2str_exp(long double &value, int precision) {
	string str;
	int i1, i2;
	int decpt = 0, neg = 0;
	str = qecvt(value, precision, &decpt, &neg);
	decpt--;
	i1 = decpt - decpt % 3;
	i2 = decpt % 3;
	if(i2 < 0) {
		i2 = -i2;
		if(i2 == 1)
			i2 = 2;
		else if(i2 == 2)
			i2 = 1;
		i1 -= 3;
	}
	if(i2 + 2 < str.length())
		str.insert(i2 + 1, DOT_STR);
	if(neg != 0)
		str.insert(0, MINUS_STR);
	if(i1 != 0) {
		str += EXP_STR;
		if(i1 > 0)
			str += PLUS_STR;
		str += i2s(i1);
	}
	return str;
}

string Calculator::value2str_exp_pure(long double &value, int precision) {
	sprintf(vbuffer, "%.*LE", precision, value);
	string stmp = vbuffer;
	return stmp;
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
	    		mngr->add(PI_VALUE, MULTIPLICATION_CH);
	    		mngr->add(180, DIVISION_CH);			
			break;
		}
		case GRADIANS: {
	    		mngr->add(PI_VALUE, MULTIPLICATION_CH);
	    		mngr->add(200, DIVISION_CH);		
			break;
		}
	}
	return mngr;
}
