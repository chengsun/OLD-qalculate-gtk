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

Calculator::Calculator() {
	srand48(time(0));
	angleMode(RADIANS);
	addBuiltinVariables();
	addBuiltinFunctions();
	addBuiltinUnits();
	b_functions = true;
	b_variables = true;
	b_units = true;
}
Calculator::~Calculator(void) {}
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
	addVariable(new Variable(this, "General", "pi", PI_VALUE, "", false));
	addVariable(new Variable(this, "General", "e", E_VALUE, "", false));
}
void Calculator::addBuiltinFunctions() {
	addFunction(new IFFunction(this));
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
void Calculator::error(bool critical, int count,...)	{
	string stmp;
	va_list ap;
	va_start(ap, count);
	for (int i = 0; i < count; i++) {
		stmp += va_arg(ap, const char*);
	}
	va_end(ap);
	errors.push(new Error(stmp, critical));
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
	if(u2 == object)
		return;
	if(u2 != NULL) {
		delUnit(u2);
	}
}
Manager *Calculator::calculate(string str) {
	setFunctionsAndVariables(str);
	EqContainer *e = new EqContainer(str, this, PLUS_CH);
	Manager *mngr = e->calculate();
	mngr->ref();
/*	int vtype = fpclassify(value);
	if(vtype == FP_NAN)
		error(true, 1, "Math error: not a number");
	else if(vtype == FP_INFINITE)
		error(true, 1, "Math error: infinite");*/
	checkFPExceptions();
	delete e;
	return mngr;
}
void Calculator::checkFPExceptions() {
	int raised = fetestexcept(FE_ALL_EXCEPT);
#ifdef FE_INEXACT
	//		if(raised & FE_INEXACT) error(true, 1, "Floating point error: inexact rounded result");
#endif
#ifdef FE_DIVBYZERO

	if(raised & FE_DIVBYZERO)
		error(true, 1, "Floating point error: division by zero");
#endif
#ifdef FE_UNDERFLOW
	//		if(raised & FE_UNDERFLOW) error(true, 1, "Floating point error: underflow");
#endif
#ifdef FE_OVERFLOW
	//		if(raised & FE_OVERFLOW) error(true, 1, "Floating point error: overflow");
#endif
#ifdef FE_INVALID

	if(raised & FE_INVALID)
		error(true, 1, "Floating point error: invalid operation");
#endif

	feclearexcept(FE_ALL_EXCEPT);
}
void Calculator::checkFPExceptions(const char *str) {
	int raised = fetestexcept(FE_ALL_EXCEPT);
#ifdef FE_INEXACT
	//		if(raised & FE_INEXACT) error(true, 2, "Floating point error: inexact rounded result in ", str);
#endif
#ifdef FE_DIVBYZERO

	if(raised & FE_DIVBYZERO)
		error(true, 2, "Floating point error: division by zero in ", str);
#endif
#ifdef FE_UNDERFLOW
	//		if(raised & FE_UNDERFLOW) error(true, 2, "Floating point error: underflow in ", str);
#endif
#ifdef FE_OVERFLOW
	//		if(raised & FE_OVERFLOW) error(true, 2, "Floating point error: overflow in ", str);
#endif
#ifdef FE_INVALID

	if(raised & FE_INVALID)
		error(true, 2, "Floating point error: invalid operation in ", str);
#endif

	feclearexcept(FE_ALL_EXCEPT);
}

long double Calculator::convert(long double value, Unit *from_unit, Unit *to_unit) {
	string str = d2s(value);
	return convert(str, from_unit, to_unit);
}
long double Calculator::convert(string str, Unit *from_unit, Unit *to_unit) {
	str += from_unit->shortName();
//	UnitManager um(this);
//	long double value = calculate(str, &um);
//	return um.convert(to_unit, value);
	return 0;
}
Unit* Calculator::addUnit(Unit *u, bool force) {
	u->name(getUnitName(u->name(), u, force));
	if(u->hasPlural()) {
		u->plural(getUnitName(u->plural(), u, force));
	}
	if(u->hasShortName()) {
		u->shortName(getUnitName(u->shortName(), u, force));
	}
	units.push_back(u);
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
		if(units[i]->name() == name_ || units[i]->shortName() == name_ || units[i]->plural() == name_) {
			return units[i];
		}
	}
	return NULL;
}

Variable* Calculator::addVariable(Variable *v, bool force) {
	variables.push_back(v);
	v->name(getName(v->name(), (void*) v, force));
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
	while(is_in(NAME_NUMBER_PRE_S, name_[i - 1])) {
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
	if(name_.find_first_of(ILLEGAL_IN_NAMES NAME_NUMBER_PRE_S NUMBERS_S) != string::npos)
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
		} else if(is_not_in(NAME_NUMBER_PRE_S, name_[i - 1])) {
			name_.insert(i, 1, NAME_NUMBER_PRE_CH);
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
		i = name_.find_first_of(ILLEGAL_IN_NAMES_MINUS_SPACE_STR NUMBERS_S, i);
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
	if(!b_variables)
		return;
	int i = -1, i3 = 0, i4, i5, i6, i7, i8, i9;
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
	gsub(SIGN_POWER_0, "o", str);
	gsub(SIGN_POWER_1, "^1", str);
	gsub(SIGN_POWER_2, "^2", str);
	gsub(SIGN_POWER_3, "^3", str);
	gsub(SIGN_EURO, "euro", str);
	gsub(SIGN_MICRO, "micro", str);
	gsub(ID_WRAP_LEFT_STR, "", str);
	gsub(ID_WRAP_RIGHT_STR, "", str);
	Manager *mngr;
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
						stmp = LEFT_BRACKET_CH;
						stmp += ID_WRAP_LEFT_CH;
						mngr =  f->calculate("");
						stmp += i2s(addId(mngr));
						mngr->unref();
						stmp += ID_WRAP_RIGHT_CH;
						stmp += RIGHT_BRACKET_CH;
						i4 = f->name().length();
					} else {
						b = false;
						i5 = 1;
						i6 = 0;
						while(i5 > 0 && !b) {
							if(i6 + i + (int) f->name().length() >= (int) str.length()) {
								if(i5 == 2) {
									b = true;
									i6++;
								}
								break;
							} else {
								char c = str[i + (int) f->name().length() + i6];
								if(is_in(LEFT_BRACKET_S, c)) {
									b = true;
								} else if(is_in(NUMBERS_S DOT_S, c)) {
									if(f->args() == 1 && i6 > 0)
										i5 = 2;
									else
										i5 = -1;
								} else if(is_in(SPACE_S, c))
									if(i5 == 2) {
										b = true;
									} else if(i5 == 2 && is_in(BRACKETS_S OPERATORS_S, str[i + (int) f->name().length() + i6]))
										b = true;
									else
										i5 = -1;
							}
							i6++;
						}
						if(b && i5 == 2) {

							stmp2 = str.substr(i + f->name().length(), i6 - 1);

							stmp = LEFT_BRACKET_CH;
							stmp += ID_WRAP_LEFT_CH;
							mngr =  f->calculate(stmp2);
							stmp += i2s(addId(mngr));
							mngr->unref();
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_BRACKET_CH;

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
								if((i5 = str.find_first_of(RIGHT_BRACKET_S, i7)) != (int) string::npos) {
									if(i5 < (i6 = str.find_first_of(LEFT_BRACKET_S, i8)) || i6 == string::npos) {
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
							stmp = LEFT_BRACKET_CH;
							stmp += ID_WRAP_LEFT_CH;
							mngr =  f->calculate(stmp2);
							stmp += i2s(addId(mngr));
							mngr->unref();
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_BRACKET_CH;

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
		} else if(ufv_t[i2] == 'p' || ufv_t[i2] == 'P') {
			ch = (char*) ufv[i2];
			while(1) {
				if((i = str.find(ch, i3)) != (int) string::npos) {
					i4 = i + strlen(ch) - 1;
					b = true;
					if(b_units) {
						i5 = str.find_first_of(NUMBERS_S OPERATORS_S BRACKETS_S SPACE_S, i4);
						if(i5 == string::npos)
							i5 = str.length() - 1;
						else
							i5--;
						if(i5 != i) {
							for(i6 = 0; i6 < (int) ufv.size(); i6++) {
								if(ufv_t[i6] == 'u')
									i7 = ((Unit*) ufv[i6])->shortName().length();
								else if(ufv_t[i6] == 'U')
									i7 = ((Unit*) ufv[i6])->name().length();
								else if(ufv_t[i6] == 'Y')
									i7 = ((Unit*) ufv[i6])->plural().length();
								else
									i7 = -1;
								if(i7 > 0 && i7 <= i5 - i4) {
									b = false;
									for(i8 = 1; i8 <= i7; i8++) {
										if((ufv_t[i6] == 'u' && str[i4 + i8] != ((Unit*) ufv[i6])->shortName()[i8 - 1]) || (ufv_t[i6] == 'U' && str[i4 + i8] != ((Unit*) ufv[i6])->name()[i8 - 1]) || (ufv_t[i6] == 'Y' && str[i4 + i8] != ((Unit*) ufv[i6])->plural()[i8 - 1])) {
											u = (Unit*) ufv[i6];
											b = true;
											if(str.length() > i4 + 1 && is_in(NUMBERS_S, str[i4 + 1])) {
												str.insert(i4 + 1, POWER_STR);
											}
											break;
										}
									}
									if(!b) {
										i4 += i7;
										break;
									}
								}
							}
						}
					}
					if(ufv_t[i2] == 'p') {
						if(!b || i == str.length() - 1 || is_not_in(BRACKETS_S OPERATORS_S, *ch)) {
							stmp = LEFT_BRACKET_CH;
							stmp += ID_WRAP_LEFT_CH;
							if(b) mngr = new Manager(this, s_prefix[*ch]);
							else mngr = new Manager(this, u, s_prefix[*ch]);
							stmp += i2s(addId(mngr));
							mngr->unref();
							stmp += ID_WRAP_RIGHT_CH;
							stmp += RIGHT_BRACKET_CH;
							str.replace(i, 1, stmp);
						} else {
							stmp += ch;
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
						str.replace(i, strlen(ch), stmp);
					}
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
					if(i4 != str.length() - 1 && is_not_in(SPACE_S NUMBERS_S OPERATORS_S BRACKETS_S DOT_S, str[i4 + 1])) {
						i3 = i + 1;
						if(i3 >= str.length()) break;
						goto find_unit;
					}
					i5 = str.find_last_of(NUMBERS_S OPERATORS_S BRACKETS_S SPACE_S, i);
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
					if(str.length() > i4 + 1 && is_in(NUMBERS_S, str[i4 + 1])) {
						str.insert(i4 + 1, POWER_STR);
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
	while(1) {
		i = str.find_first_not_of(NUMBERS_S DOT_S ILLEGAL_IN_NAMES, i);
		if(i == string::npos) break;
		stmp = str[i];
		u = getUnit(stmp);
		if(u) mngr = new Manager(this, u);
		else mngr = new Manager(this, stmp);
		stmp = LEFT_BRACKET_CH;
		stmp += ID_WRAP_LEFT_CH;
		stmp += i2s(addId(mngr));
		mngr->unref();
		stmp += ID_WRAP_RIGHT_CH;
		stmp += RIGHT_BRACKET_CH;
		str.replace(i, 1, stmp);
	}
}
string Calculator::getName(string name, void *object, bool force, bool always_append) {
	if(force) {
		deleteName(name, object);
		return name;
	}
	int i2 = 1;
	char buffer[10];
	if(name.empty()) {
		name = "x";
		always_append = true;
	}
	string stmp = name;
	if(always_append)
		stmp += NAME_NUMBER_PRE_STR "1";
	for(int i = 0; i < (int) variables.size(); i++) {
		if(variables[i] != object && variables[i]->name() == stmp) {
			i2++;
			sprintf(buffer, NAME_NUMBER_PRE_STR "%i", i2);
			stmp = name + buffer;
		}
	}
	for(int i = 0; i < (int) functions.size(); i++) {
		if(functions[i] != object && functions[i]->name() == stmp) {
			i2++;
			sprintf(buffer, NAME_NUMBER_PRE_STR "%i", i2);
			stmp = name + buffer;
		}
	}
	if(i2 > 1 && !always_append)
		error(false, 4, "Name exists: ", name.c_str(), ", new name: ", stmp.c_str());
	return stmp;
}
string Calculator::getUnitName(string name, Unit *object, bool force, bool always_append) {
	if(force) {
		deleteUnitName(name, object);
		return name;
	}
	int i2 = 1;
	char buffer[10];
	if(name.empty()) {
		name = "x";
		always_append = true;
	}
	string stmp = name;
	if(always_append)
		stmp += NAME_NUMBER_PRE_STR "1";
	for(int i = 0; i < (int) units.size(); i++) {
		if(units[i] != object && (units[i]->name() == stmp || units[i]->shortName() == stmp || units[i]->plural() == stmp)) {
			i2++;
			sprintf(buffer, NAME_NUMBER_PRE_STR "%i", i2);
			stmp = name + buffer;
		}
	}
	if(i2 > 1 && !always_append)
		error(false, 4, "Name exists: ", name.c_str(), ", new name: ", stmp.c_str());
	return stmp;
}
bool Calculator::save(const char* file_name) {
	FILE *file = fopen(file_name, "w+");
	if(file == NULL)
		return false;
	string str;
	for(int i = 0; i < variables.size(); i++) {
		if(variables[i]->isUserVariable())
			fprintf(file, "*Variable\t");
		else
			fprintf(file, "*BuiltinVariable\t");
		if(!variables[i]->isUserVariable())
			fprintf(file, "%s\t", variables[i]->name().c_str());
		if(variables[i]->category().empty())
			fprintf(file, "0\t");
		else
			fprintf(file, "%s\t", variables[i]->category().c_str());
		if(variables[i]->isUserVariable())
			fprintf(file, "%s\t%s\t", variables[i]->name().c_str(), variables[i]->get()->print().c_str());
		if(variables[i]->title().empty())
			fprintf(file, "0\t");
		else
			fprintf(file, "%s\t", variables[i]->title().c_str());
		fprintf(file, "\n");
	}
	fprintf(file, "\n");
	for(int i = 0; i < functions.size(); i++) {
		if(functions[i]->isUserFunction())
			fprintf(file, "*Function\t");
		else
			fprintf(file, "*BuiltinFunction\t");
		if(!functions[i]->isUserFunction())
			fprintf(file, "%s\t", functions[i]->name().c_str());
		if(functions[i]->category().empty())
			fprintf(file, "0\t");
		else
			fprintf(file, "%s\t", functions[i]->category().c_str());
		if(functions[i]->isUserFunction())
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
	fprintf(file, "\n");
	CompositeUnit *cu;
	AliasUnit *au;
	Unit *u;
	int exp = 1;
	for(int i = 0; i < units.size(); i++) {
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
			if(units[i]->title().empty())
				fprintf(file, "0\t");
			else
				fprintf(file, "%s", units[i]->title().c_str());
			cu = (CompositeUnit*) units[i];
			cu->sort();
			for(int i = 0; i < cu->sorted.size(); i++) {
//				fprintf(file, "\t%s\t%s\t%LG", cu->units[cu->sorted[i]]->firstBaseUnit()->shortName().c_str(), cu->units[cu->sorted[i]]->firstBaseExp()->print().c_str(), cu->units[cu->sorted[i]]->prefixValue());
				fprintf(file, "\t%s\t%s\t%LG", cu->units[cu->sorted[i]]->firstBaseUnit()->shortName().c_str(), d2s(cu->units[cu->sorted[i]]->firstBaseExp()).c_str(), cu->units[cu->sorted[i]]->prefixValue());
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
//			fprintf(file, "\t%s\t%s\t%s", au->firstShortBaseName().c_str(), au->expression().c_str(), au->firstBaseExp()->print().c_str());
			fprintf(file, "\t%s\t%s\t%s", au->firstShortBaseName().c_str(), au->expression().c_str(), d2s(au->firstBaseExp()).c_str());
			if(!au->reverseExpression().empty())
				fprintf(file, "\t%s", au->reverseExpression().c_str());
		}
		fprintf(file, "\n");
	}
	fprintf(file, "\n");
	for(l_type::iterator it = l_prefix.begin(); it != l_prefix.end(); ++it) {
		fprintf(file, "*Prefix\t%s\t%LG\n", it->first, it->second);
	}
	for(hash_map<char, long double>::iterator it = s_prefix.begin(); it != s_prefix.end(); ++it) {
		fprintf(file, "*Prefix\t%c\t%LG\n", it->first, it->second);
	}
	fclose(file);
}
bool Calculator::load(const char* file_name) {
	FILE *file = fopen(file_name, "r");
	if(file == NULL)
		return false;
	string stmp, str, ntmp, vtmp, rtmp, etmp, shtmp, ctmp, ttmp;
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
			return true;
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
									addVariable(new Variable(this, ctmp, ntmp, mngr, ttmp));
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
								func = addFunction(new UserFunction(this, ctmp, ntmp, vtmp));
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
									while(1) {
										b = true;
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
								addUnit(new Unit(this, ctmp, ntmp, etmp, shtmp, ttmp));
						}
					}
				} else if(str == "*CompositeUnit") {
					if((i = stmp.find_first_not_of("\t\n", i)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
						ctmp = stmp.substr(i, i2 - i);
						if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
							ttmp = stmp.substr(i, i2 - i);
							if(ttmp == "0")
								ttmp = "";
							CompositeUnit* cu = NULL;
							i3 = 0;
							while(1) {
								if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
									vtmp = stmp.substr(i, i2 - i);
									if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
										etmp = stmp.substr(i, i2 - i);
										if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
											rtmp = stmp.substr(i, i2 - i);
											u = getUnit(vtmp);
											if(u) {
												if(i3 == 0)
													cu = new CompositeUnit(this, ctmp, ttmp);
												if(cu) {
													mngr = calculate(etmp);
													//cu->add(u, mngr, strtold(rtmp.c_str(), NULL));
													cu->add(u, mngr->value(), strtold(rtmp.c_str(), NULL));
													mngr->unref();
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
							if(cu)
								addUnit(cu);
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
											if((unitNameIsValid(ntmp) && unitNameIsValid(etmp) && unitNameIsValid(shtmp)) && u) {
												au = new AliasUnit(this, ctmp, ntmp, etmp, shtmp, ttmp, u);
												if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
													au->expression(stmp.substr(i, i2 - i));
													if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
														mngr = calculate(stmp.substr(i, i2 - i));
														//au->exp(mngr);
														au->exp(mngr->value());
														mngr->unref();
														if((i = stmp.find_first_not_of("\t\n", i2)) != string::npos && (i2 = stmp.find_first_of("\t\n", i)) != string::npos) {
															au->reverseExpression(stmp.substr(i, i2 - i));
														}
													}
												}
												addUnit(au);
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
							addPrefix(ntmp, strtold(vtmp.c_str(), NULL));
						}
					}
				}
			}
		}
	}
	fclose(file);
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
string Calculator::value2str_octal(long double &value, int precision) {
	sprintf(vbuffer, "%#lo", lroundl(value));
	string stmp = vbuffer;
	return stmp;
}
string Calculator::value2str_hex(long double &value, int precision) {
	sprintf(vbuffer, "%#lx", lroundl(value));
	string stmp = vbuffer;
	return stmp;
}
string Calculator::value2str_prefix(long double &value, int precision, bool use_short_prefixes, long double *new_value) {
	string str;
	long int i1, i2, iv, iv2 = 0;
	char state = 2;
	hash_map<char, long double>::iterator it, itt;
	l_type::iterator it2, itt2;
	i1 = (long int) floorl(log10l(value));
	if(i1 != 0) {
		if(use_short_prefixes) {
			for(it = s_prefix.begin(); it != s_prefix.end(); ++it) {
//				if(u)
//					iv = lroundl(log10l(u->prefixValue(it->second)));
//				else
					iv = lroundl(log10l(it->second));
				if(iv == i1) {
					itt = it;
					iv2 = iv;
					state = 1;
					break;
				} else if((i1 > 0 && ((iv < i1 && iv > i1 - 10 && iv > iv2) || (iv2 > i1 && iv > i1 - 10 && (iv < iv2 || iv2 == 0)))) || (i1 < 0 && ((iv < i1 && iv > iv2) || (iv2 > i1 && iv < i1 + 4 && (iv < iv2 || iv2 == 0))))) {
					iv2 = iv;
					itt = it;
					state = 2;
				}
			}
		}
		for(it2 = l_prefix.begin(); it2 != l_prefix.end() && state > 1; it2++) {
//			if(u)
//				iv = lroundl(log10l(u->prefixValue(it2->second)));
//			else
				iv = lroundl(log10l(it2->second));
			if(iv == i1) {
				itt2 = it2;
				iv2 = iv;
				state = 3;
				break;
			} else if((i1 > 0 && ((iv < i1 && iv > i1 - 10 && iv > iv2) || (iv2 > i1 && iv > i1 - 10 && (iv < iv2 || iv2 == 0)))) || (i1 < 0 && ((iv < i1 && iv > iv2) || (iv2 > i1 && iv < i1 + 4 && (iv < iv2 || iv2 == 0))))) {
				iv2 = iv;
				itt2 = it2;
				state = 4;
			}
		}
		if(!use_short_prefixes && state != 3) {
			for(it = s_prefix.begin(); it != s_prefix.end(); ++it) {
//				if(u)
//					iv = lroundl(log10l(u->prefixValue(it->second)));
//				else
					iv = lroundl(log10l(it->second));
				if(iv == i1) {
					itt = it;
					iv2 = iv;
					state = 1;
					break;
				} else if((i1 > 0 && ((iv < i1 && iv > i1 - 10 && iv > iv2) || (iv2 > i1 && iv > i1 - 10 && (iv < iv2 || iv2 == 0)))) || (i1 < 0 && ((iv < i1 && iv > iv2) || (iv2 > i1 && iv < i1 + 4 && (iv < iv2 || iv2 == 0))))) {
					iv2 = iv;
					itt = it;
					state = 2;
				}
			}
		}
	}
	if(iv2 != 0) {
		long double vtmp;
		if(state > 2)
			vtmp = itt2->second;
		else
			vtmp = itt->second;
//		if(u)
//			vtmp = value / u->prefixValue(vtmp);
//		else
			vtmp = value / vtmp;
		if(new_value)
			*new_value = vtmp;
		str = value2str(vtmp, precision);
		str += ' ';
		if(state > 2)
			str += itt2->first;
		else
			str += itt->first;
		return str;
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
		str.insert(i2 + 1, 1, DOT_CH);
	if(neg != 0)
		str.insert(0, 1, MINUS_CH);
	if(i1 != 0) {
		str += EXP_CH;
		if(i1 > 0)
			str += PLUS_CH;
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

