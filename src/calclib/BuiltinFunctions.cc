/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "BuiltinFunctions.h"

IFFunction::IFFunction(Calculator *calc_) : Function(calc_, "Logical", "if", 3, "If...Then...Else", "Tests a condition and returns a value depending on the result.") {
	addArgName("Condition (use <, <=, =, <>, => or >)");
	addArgName("Value if condition met");	
	addArgName("Value if condition NOT met");		
}
Manager *IFFunction::calculate(const string &argv) {
	Manager *mngr = NULL;
	string str[3];
	str[0] = "";
	str[1] = "";
	str[2] = "";
	int itmp = args(argv, str);
	if(itmp >= minargs()) {
		if(itmp > maxargs() && maxargs() >= 0)
			calc->error(false, _("Additional arguments for function %s() was ignored. Function can only use %s arguments."), name().c_str(), i2s(maxargs()).c_str(), NULL);
		unsigned int i = str[0].find_first_of("<=>", 0);
		bool result = false;
		int com = 0;
		if(i == string::npos) {
			calc->error(false, _("Condition contains no comparison, interpreting as \"%s > 0\"."), str[0].c_str(), NULL);
			str[0] += " > 0";
			i = str[0].find_first_of("<=>", 0);
		} 
		string str1 = str[0].substr(0, i);
		string str2 = str[0].substr(i + 1, str[0].length() - i + 1);			
		remove_blank_ends(str2);
		char sign1 = str[0][i], sign2 = 0;
		if(str2[0] != sign1 && (str2[0] == '>' || str2[0] == '=' || str2[0] == '<')) {
			sign2 = str2[0];
			str2.erase(0);
		}
		Manager *mngr1 = calc->calculate(str1);
		Manager *mngr2 = calc->calculate(str2);			
		mngr1->add(mngr2, MINUS_CH);
		if(mngr1->type() == VALUE_MANAGER || mngr1->type() == NULL_MANAGER) {
			if(sign1 == '=') {
				if(sign2 == 0) 
					result = mngr1->value() == 0L;
				else if(sign2 == '>') 
					result = mngr1->value() >= 0L;
				else if(sign2 == '<') 
					result = mngr1->value() <= 0L; 
			} else if(sign1 == '>') {
				if(sign2 == 0) 
					result = mngr1->value() > 0L;
				else if(sign2 == '=') 
					result = mngr1->value() >= 0L;
				else if(sign2 == '<') 
					result = mngr1->value() != 0L; 
			} else if(sign1 == '<') {
				if(sign2 == 0) 
					result = mngr1->value() < 0L;
				else if(sign2 == '>') 
					result = mngr1->value() != 0L;
				else if(sign2 == '=') 
					result = mngr1->value() <= 0L; 
			}
		} else {
			calc->error(true, _("Comparison is not solvable, treating as FALSE."), NULL);
		}
		mngr1->unref();
		mngr2->unref();
		if(result) {			
			mngr = calc->calculate(str[1]);
		} else {			
			mngr = calc->calculate(str[2]);		
		}		
//		calculate2(mngr);
//		calc->checkFPExceptions(sname.c_str());
	} else {
		calc->error(true, _("You need at least %s arguments in function %s()."), i2s(minargs()).c_str(), name().c_str(), NULL);
		Manager *mngr = new Manager(calc, this, NULL);
	}
//	for(unsigned int i = 0; i < vargs.size(); i++) {
//		vargs[i]->unref();
//	}
//	vargs.clear();
	return mngr;
}
AbsFunction::AbsFunction(Calculator *calc_) : Function(calc_, "Arithmetics", "abs", 1, "Absolute Value") {
	addArgName("Number");
}
void AbsFunction::calculate2(Manager *mngr) {
	//fabsl(vargs[0]->value());
	mngr->set(vargs[0]);
	if(mngr->negative()) {
		mngr->add(-1.0L, MULTIPLICATION_CH);
	}
}
CeilFunction::CeilFunction(Calculator *calc_) : Function(calc_, "Arithmetics", "ceil", 1, "Round upwards") {
	addArgName("Number");
}
void CeilFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], NULL);
	else mngr->set(ceill(vargs[0]->value()));
}
FloorFunction::FloorFunction(Calculator *calc_) : Function(calc_, "Arithmetics", "floor", 1, "Round downwards") {
	addArgName("Number");
}
void FloorFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], NULL);
	else mngr->set(floorl(vargs[0]->value()));
}
TruncFunction::TruncFunction(Calculator *calc_) : Function(calc_, "Arithmetics", "trunc", 1, "Round towards zero") {
	addArgName("Number");
}
void TruncFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], NULL);
	else mngr->set(truncl(vargs[0]->value()));
}
RoundFunction::RoundFunction(Calculator *calc_) : Function(calc_, "Arithmetics", "round", 1, "Round") {
	addArgName("Number");
}
void RoundFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], NULL);
	else mngr->set(roundl(vargs[0]->value()));
}
RemFunction::RemFunction(Calculator *calc_) : Function(calc_, "Arithmetics", "rem", 2, "Reminder (rem)") {
	addArgName("Numerator");
	addArgName("Denominator");
}
void RemFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], vargs[1], NULL);
	else if(vargs[1]->type() != VALUE_MANAGER && vargs[1]->type() != NULL_MANAGER) mngr->set(this, vargs[0], vargs[1], NULL);	
	else {
		if(vargs[1] == 0) {
			calc->error(true, _("The denominator in rem function cannot be zero"), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);
		} else {
			mngr->set(dreml(vargs[0]->value(), vargs[1]->value()));
		}
	}
}
ModFunction::ModFunction(Calculator *calc_) : Function(calc_, "Arithmetics", "mod", 2, "Reminder (mod)") {
	addArgName("Numerator");
	addArgName("Denominator");
}
void ModFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], vargs[1], NULL);
	else if(vargs[1]->type() != VALUE_MANAGER && vargs[1]->type() != NULL_MANAGER) mngr->set(this, vargs[0], vargs[1], NULL);	
	else {
		if(vargs[1] == 0) {
			calc->error(true, _("The denominator in mod function cannot be zero"), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);
		} else {
			mngr->set(fmodl(vargs[0]->value(), vargs[1]->value()));
		}
	}
}

SinFunction::SinFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "sin", 1, "Sine") {}
void SinFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(sinl(calc->getAngleValue(vargs[0]->value())));
}
CosFunction::CosFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "cos", 1, "Cosine") {}
void CosFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(cosl(calc->getAngleValue(vargs[0]->value())));
}
TanFunction::TanFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "tan", 1, "Tangent") {}
void TanFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(tanl(calc->getAngleValue(vargs[0]->value())));
}
SinhFunction::SinhFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "sinh", 1, "Hyperbolic sine") {}
void SinhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(sinhl(calc->getAngleValue(vargs[0]->value())));
}
CoshFunction::CoshFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "cosh", 1, "Hyperbolic cosine") {}
void CoshFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(coshl(calc->getAngleValue(vargs[0]->value())));
}
TanhFunction::TanhFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "tanh", 1, "Hyperbolic tangent") {}
void TanhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(tanhl(calc->getAngleValue(vargs[0]->value())));
}
AsinFunction::AsinFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "asin", 1, "Arcsine") {}
void AsinFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(asinl(calc->getAngleValue(vargs[0]->value())));
}
AcosFunction::AcosFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "acos", 1, "Arccosine") {}
void AcosFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(acosl(calc->getAngleValue(vargs[0]->value())));
}
AtanFunction::AtanFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "atan", 1, "Arctangent") {}
void AtanFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(atanl(calc->getAngleValue(vargs[0]->value())));
}
AsinhFunction::AsinhFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "asinh", 1, "Hyperbolic arcsine") {}
void AsinhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(asinhl(calc->getAngleValue(vargs[0]->value())));
}
AcoshFunction::AcoshFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "acosh", 1, "Hyperbolic arccosine") {}
void AcoshFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(acoshl(calc->getAngleValue(vargs[0]->value())));
}
AtanhFunction::AtanhFunction(Calculator *calc_) : Function(calc_, "Trigonometry", "atanh", 1, "Hyperbolic arctangent") {}
void AtanhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, calc->setAngleValue(vargs[0]), NULL);
	else mngr->set(atanhl(calc->getAngleValue(vargs[0]->value())));
}
LogFunction::LogFunction(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "ln", 1, "Natural Logarithm") {}
void LogFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], NULL);
	else mngr->set(logl(vargs[0]->value()));
}
Log10Function::Log10Function(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "log", 1, "Base-10 Logarithm") {}
void Log10Function::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], NULL);
	else mngr->set(log10l(vargs[0]->value()));
}
Log2Function::Log2Function(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "log2", 1, "Base-2 Logarithm") {}
void Log2Function::calculate2(Manager *mngr) {
	if(vargs[0]->type() != VALUE_MANAGER && vargs[0]->type() != NULL_MANAGER) mngr->set(this, vargs[0], NULL);
	else mngr->set(log2l(vargs[0]->value()));
}
ExpFunction::ExpFunction(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "exp", 1, "e raised to the power X") {}
void ExpFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() == VALUE_MANAGER) mngr->set(expl(vargs[0]->value()));
	else if(vargs[0]->type() == NULL_MANAGER) mngr->set(1.0L);
	else {
		mngr->set(E_VALUE);
		mngr->add(vargs[0], POWER_CH);
	}	
}
Exp10Function::Exp10Function(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "exp10", 1, "10 raised the to power X") {}
void Exp10Function::calculate2(Manager *mngr) {
	if(vargs[0]->type() == VALUE_MANAGER) mngr->set(exp10l(vargs[0]->value()));
	else if(vargs[0]->type() == NULL_MANAGER) mngr->set(1.0L);
	else {
		mngr->set(10.0L);
		mngr->add(vargs[0], POWER_CH);
	}	
}
Exp2Function::Exp2Function(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "exp2", 1, "2 raised the to power X") {}
void Exp2Function::calculate2(Manager *mngr) {
	if(vargs[0]->type() == VALUE_MANAGER) mngr->set(exp2l(vargs[0]->value()));
	else if(vargs[0]->type() == NULL_MANAGER) mngr->set(1.0L);
	else {
		mngr->set(2.0L);
		mngr->add(vargs[0], POWER_CH);
	}	
}
SqrtFunction::SqrtFunction(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "sqrt", 1, "Square Root") {
	addArgName("Non-negative number");
}
void SqrtFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() == VALUE_MANAGER) {
		if(vargs[0]->value() < 0) {
			calc->error(true, _("Trying to calculate the square root of a negative number (%s)."), d2s(vargs[0]->value()).c_str(), NULL);
			mngr->set(this, vargs[0], NULL);
			return;
		}
		mngr->set(sqrtl(vargs[0]->value()));
	} else if(vargs[0]->type() != NULL_MANAGER) {
		mngr->set(vargs[0]);
		mngr->add(0.5L, POWER_CH);
	} else {
		mngr->set(0.0L);
	}	
}
CbrtFunction::CbrtFunction(Calculator *calc_) : Function(calc_, "Exponents and Logarithms", "cbrt", 1, "Cube Root") {}
void CbrtFunction::calculate2(Manager *mngr) {
	if(vargs[0]->type() == VALUE_MANAGER) mngr->set(cbrtl(vargs[0]->value()));
	else if(vargs[0]->type() != NULL_MANAGER) {
		mngr->set(vargs[0]);
		mngr->add(1.0L / 3.0L, POWER_CH);
	}
}
HypotFunction::HypotFunction(Calculator *calc_) : Function(calc_, "Geometry", "hypot", 2, "Hypotenuse") {
	addArgName("Side 1 of triangle");
	addArgName("Side 2 of triangle");
}
void HypotFunction::calculate2(Manager *mngr) {
	mngr->value(hypotl(vargs[0]->value(), vargs[1]->value()));
	if(vargs[0]->type() == VALUE_MANAGER && vargs[1]->type() == VALUE_MANAGER) mngr->set(hypotl(vargs[0]->value(), vargs[1]->value()));
	else if(vargs[0]->type() != NULL_MANAGER) {
		mngr->set(vargs[0]);
		mngr->add(2.0, POWER_CH);
		Manager *mngr2 = new Manager(vargs[1]);
		mngr2->add(2.0, POWER_CH);		
		mngr->add(mngr2, PLUS_CH);
		mngr2->unref();
		mngr->add(0.5, POWER_CH);
	}	
}
SumFunction::SumFunction(Calculator *calc_) : Function(calc_, "Statistics", "sum", -1, "Sum") {}
void SumFunction::calculate2(Manager *mngr) {
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], PLUS_CH);
	}
}
MeanFunction::MeanFunction(Calculator *calc_) : Function(calc_, "Statistics", "mean", -1, "Mean") {}
void MeanFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], PLUS_CH);
	}
	mngr->add(vargs.size(), DIVISION_CH);	
}
MedianFunction::MedianFunction(Calculator *calc_) : Function(calc_, "Statistics", "median", -1, "Median") {}
void MedianFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	list<long double> largs;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		largs.push_back(vargs[i]->value());
	}
	largs.sort();
	list<long double>::iterator it = largs.begin();
	for(unsigned int i = 0; i < largs.size() / 2 - 1; i++) {
	    ++it;
	}	
	if(vargs.size() % 2 == 0) {
		mngr->value(*it);
		++it;
		mngr->add(*it, PLUS_CH);
		mngr->add(2, DIVISION_CH);
	} else {
		++it;
		mngr->value(*it);
	}
}
MinFunction::MinFunction(Calculator *calc_) : Function(calc_, "Statistics", "min", -1, "Min") {}
void MinFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	long double value = vargs[0]->value();
	unsigned int ui = 0;
	for(unsigned int i = 1; i < vargs.size(); i++) {
		if(vargs[i]->value() < value) {
			value = vargs[i]->value();
			ui = i;
		}
	}
	mngr->set(vargs[ui]);
}
MaxFunction::MaxFunction(Calculator *calc_) : Function(calc_, "Statistics", "max", -1, "Max") {}
void MaxFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	long double value = vargs[0]->value();
	unsigned int ui = 0;
	for(unsigned int i = 1; i < vargs.size(); i++) {
		if(vargs[i]->value() > value) {
			value = vargs[i]->value();
			ui = i;
		}
	}
	mngr->set(vargs[ui]);
}
ModeFunction::ModeFunction(Calculator *calc_) : Function(calc_, "Statistics", "mode", -1, "Mode") {}
void ModeFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	long double value;
	unsigned int ui = 0;
	int n = 0;
	bool b;
	vector<long double> ds;
	vector<int> is;
	hash_map<long double, int> tmpmap;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		b = true;
		for(unsigned int i2 = 0; i2 < ds.size(); i2++) {
			if(ds[i2] == vargs[i]->value()) {
				is[i2]++;
				b = false;
				break;
			}
		}
		if(b) {
			ds.push_back(vargs[i]->value());
			is.push_back(1);
		}
	}
	for(unsigned int i = 0; i < is.size(); i++) {
		if(is[i] > n) {
			n = is[i];
			value = ds[i];
		}
	}
	for(unsigned int i = 0; i < vargs.size(); i++) {
		if(vargs[i]->value() == value) {
			ui = i;
			break;
		}
	}
/*	if(u)
		u->add
		(uargs[ui]);*/
	mngr->value(value);
}
NumberFunction::NumberFunction(Calculator *calc_) : Function(calc_, "Statistics", "number", -1, "Number") {}
void NumberFunction::calculate2(Manager *mngr) {
	mngr->value(vargs.size());
}
StdDevFunction::StdDevFunction(Calculator *calc_) : Function(calc_, "Statistics", "stddev", -1, "Standard Deviation") {}
void StdDevFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	long double mean = 0, value = 0;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mean += vargs[i]->value();
	}
	mean = mean / vargs.size();
	for(unsigned int i = 0; i < vargs.size(); i++) {
		value += powl(vargs[i]->value() - mean, 2);
	}
/*	if(u)
		u->add
		(uargs[0]);*/
	mngr->value(sqrtl(value / vargs.size()));
}
StdDevSFunction::StdDevSFunction(Calculator *calc_) : Function(calc_, "Statistics", "stddevs", -1, "Standard Deviation (random sampling)") {}
void StdDevSFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	long double mean = 0, value = 0;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mean += vargs[i]->value();
	}
	mean = mean / vargs.size();
	for(unsigned int i = 0; i < vargs.size(); i++) {
		value += powl(vargs[i]->value() - mean, 2);
	}
/*	if(u)
		u->add
		(uargs[0]);*/
	mngr->value(sqrtl(value / (vargs.size() - 1)));
}
RandomFunction::RandomFunction(Calculator *calc_) : Function(calc_, "General", "rand", 0, "Random Number", "Generates a pseudo-random number between 0 and 1") {}
Manager *RandomFunction::calculate(const string &eq) {
	Manager *mngr = new Manager(calc);
	mngr->set(drand48());
	return mngr;
}

BASEFunction::BASEFunction(Calculator *calc_) : Function(calc_, "General", "BASE", 2, "Number Base", "Returns a decimal integer from a number of specified base between 2 and 36") {
	addArgName("Number");
	addArgName("Base");	
}
Manager *BASEFunction::calculate(const string &eq) {
	int itmp;
	if((itmp = stringArgs(eq)) >= minargs()) {
		if(itmp > maxargs())
			calc->error(false, _("Additional arguments for function %s() was ignored. Function can only use %s arguments."), name().c_str(), i2s(maxargs()).c_str(), NULL);			
		Manager *mngr = calc->calculate(svargs[1]);	
		int base = (int) mngr->value();
		long double value = 0;
		if(base < 2 || base > 36) {
			calc->error(false, _("Base must be between 2 and 36 (was %s) for function %s()."), i2s(base).c_str(), name().c_str(), NULL);
			mngr->unref();
		} else {
			value = (long double) strtol(svargs[0].c_str(), NULL, base);
			svargs.clear();
			mngr->set(value);
			return mngr;
		}
	} else {
		calc->error(true, _("You need at least %s arguments in function %s()."), i2s(minargs()).c_str(), name().c_str(), NULL);
	}
	Manager *mngr = new Manager(calc, this, NULL);
	for(int i = 0; i < itmp; i++) {
		Manager *mngr2 = new Manager(calc, svargs[i]);
		mngr->addFunctionArg(mngr2);
		mngr2->unref();
	}
	svargs.clear();
	return mngr;			
}
BINFunction::BINFunction(Calculator *calc_) : Function(calc_, "General", "BIN", 1, "Binary", "Returns a decimal integer from a binary number") {
	addArgName("Binary number");
}
Manager *BINFunction::calculate(const string &eq) {
	Manager *mngr = new Manager(calc);
	mngr->set((long double) strtol(eq.c_str(), NULL, 2));
	return mngr;
}
OCTFunction::OCTFunction(Calculator *calc_) : Function(calc_, "General", "OCT", 1, "Octal", "Returns a decimal integer from an octal number") {
	addArgName("Octal number");
}
Manager *OCTFunction::calculate(const string &eq) {
	Manager *mngr = new Manager(calc);
	mngr->set((long double) strtol(eq.c_str(), NULL, 8));
	return mngr;
}
HEXFunction::HEXFunction(Calculator *calc_) : Function(calc_, "General", "HEX", 1, "Hexadecimal", "Returns a decimal value from a hexadecimal number") {
	addArgName("Hexadecimal number");
}
Manager *HEXFunction::calculate(const string &eq) {
	string stmp;
	if(eq.length() >= 2 && eq[0] == '0' && (eq[1] == 'x' || eq[1] == 'X'))
		stmp = eq;
	else {
		stmp = "0x";
		stmp += eq;
	}
	Manager *mngr = new Manager(calc);
	mngr->set(strtold(stmp.c_str(), NULL));
	return mngr;
}

