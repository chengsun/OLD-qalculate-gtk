/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "BuiltinFunctions.h"

IFFunction::IFFunction() : Function("Logical", "if", 3, "If...Then...Else") {
}
Manager *IFFunction::calculate(const string &argv) {
	Manager *mngr = NULL;
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		unsigned int i = svargs[0].find_first_of("<=>", 0);
		bool result = false;
		int com = 0;
		if(i == string::npos) {
			CALCULATOR->error(false, _("Condition contains no comparison, interpreting as \"%s > 0\"."), svargs[0].c_str(), NULL);
			svargs[0] += " > 0";
			i = svargs[0].find_first_of("<=>", 0);
		} 
		string str1 = svargs[0].substr(0, i);
		string str2 = svargs[0].substr(i + 1, svargs[0].length() - i + 1);			
		remove_blank_ends(str2);
		char sign1 = svargs[0][i], sign2 = 0;
		if(str2[0] != sign1 && (str2[0] == '>' || str2[0] == '=' || str2[0] == '<')) {
			sign2 = str2[0];
			str2.erase(0);
		}
		Manager *mngr1 = CALCULATOR->calculate(str1);
		Manager *mngr2 = CALCULATOR->calculate(str2);			
		mngr1->add(mngr2, SUBTRACT);
		if(mngr1->isNumber()) {
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
			CALCULATOR->error(true, _("Comparison is not solvable, treating as FALSE."), NULL);
		}
		mngr1->unref();
		mngr2->unref();
		if(result) {			
			mngr = CALCULATOR->calculate(svargs[1]);
		} else {			
			mngr = CALCULATOR->calculate(svargs[2]);		
		}			
	} else {
		mngr = createFunctionManagerFromSVArgs(itmp);
	}
	clearSVArgs();
	return mngr;
}
GCDFunction::GCDFunction() : Function("Arithmetics", "gcd", 2, "Greatest Common Divisor") {
}
void GCDFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], vargs[1], NULL);
	if(!vargs[1]->isNumber()) mngr->set(this, vargs[0], vargs[1], NULL);	
	else mngr->set(gcd_d(vargs[0]->value(), vargs[1]->value()));
}
DaysFunction::DaysFunction() : Function("Date & Time", "daysto", 2, "Days to date") {
}
Manager *DaysFunction::calculate(const string &argv) {
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		int days = daysBetweenDates(svargs[0], svargs[1], 1);
		if(days < 0) {
			CALCULATOR->error(false, _("Error in date format for function %s()."), name().c_str(), NULL);
		} else {
			Manager *mngr = new Manager(days, 1, 0);
			return mngr;
		}	
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
DaysBetweenDatesFunction::DaysBetweenDatesFunction() : Function("Date & Time", "days_between_dates", 2, "Days between two dates", "", false, 3) {
}
Manager *DaysBetweenDatesFunction::calculate(const string &argv) {
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		int basis = s2i(svargs[2]);
		int days = daysBetweenDates(svargs[0], svargs[1], basis);
		if(days < 0) {
			CALCULATOR->error(false, _("Error in date format for function %s()."), name().c_str(), NULL);
		} else {
			Manager *mngr = new Manager(days, 1, 0);
			return mngr;
		}		
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
YearsBetweenDatesFunction::YearsBetweenDatesFunction() : Function("Date & Time", "years_between_dates", 2, "Years between two dates", "", false, 3) {
}
Manager *YearsBetweenDatesFunction::calculate(const string &argv) {
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		int basis = s2i(svargs[2]);
		Fraction *fr = yearsBetweenDates(svargs[0], svargs[1], basis);
		if(!fr) {
			CALCULATOR->error(false, _("Error in date format for function %s()."), name().c_str(), NULL);
		} else {
			Manager *mngr = new Manager(fr);
			return mngr;
		}		
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
DifferentiateFunction::DifferentiateFunction() : Function("Experimental", "diff", 2, "Differentiate") {
}
Manager *DifferentiateFunction::calculate(const string &argv) {
//	CALCULATOR->error(true, _("%s() is an experimental unfinished function!"), name().c_str(), NULL);
	Manager *mngr = NULL;
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		mngr = CALCULATOR->calculate(svargs[0]);
		mngr->differentiate(svargs[1]);
	} else {
		mngr = createFunctionManagerFromSVArgs(itmp);
	}
	clearSVArgs();
	return mngr;	
}

AbsFunction::AbsFunction() : Function("Arithmetics", "abs", 1, "Absolute Value") {

}
void AbsFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);	
	else if(vargs[0]->type() == FRACTION_MANAGER) {
		if(vargs[0]->fraction()->isNegative()) mngr->set(-vargs[0]->fraction()->numerator(), vargs[0]->fraction()->denominator(), vargs[0]->fraction()->exponent());
		else mngr->set(vargs[0]);
	} else mngr->set(fabsl(vargs[0]->value()));
}
CeilFunction::CeilFunction() : Function("Arithmetics", "ceil", 1, "Round upwards") {

}
void CeilFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else if(vargs[0]->type() == VALUE_MANAGER) mngr->set(ceill((double) vargs[0]->value()));
	else mngr->set(vargs[0]);
}
FloorFunction::FloorFunction() : Function("Arithmetics", "floor", 1, "Round downwards") {

}
void FloorFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
 	else if(vargs[0]->type() == VALUE_MANAGER) mngr->set(floorl((double) vargs[0]->value()));
	else mngr->set(vargs[0]);	
}
TruncFunction::TruncFunction() : Function("Arithmetics", "trunc", 1, "Round towards zero") {

}
void TruncFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else if(vargs[0]->type() == VALUE_MANAGER) mngr->set(truncl((double) vargs[0]->value()));
	else mngr->set(vargs[0]);
}
RoundFunction::RoundFunction() : Function("Arithmetics", "round", 1, "Round") {

}
void RoundFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else if(vargs[0]->type() == VALUE_MANAGER) mngr->set(roundl(vargs[0]->value()));
	else mngr->set(vargs[0]);
}
RemFunction::RemFunction() : Function("Arithmetics", "rem", 2, "Reminder (rem)") {

}
void RemFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], vargs[1], NULL);
	else if(!vargs[1]->isNumber()) mngr->set(this, vargs[0], vargs[1], NULL);	
	else {
		if(vargs[1] == 0) {
			CALCULATOR->error(true, _("The denominator in rem function cannot be zero"), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);
		} else {
			mngr->set(dreml(vargs[0]->value(), vargs[1]->value()));
		}
	}
}
ModFunction::ModFunction() : Function("Arithmetics", "mod", 2, "Reminder (mod)") {

}
void ModFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], vargs[1], NULL);
	else if(!vargs[1]->isNumber()) mngr->set(this, vargs[0], vargs[1], NULL);	
	else {
		if(vargs[1] == 0) {
			CALCULATOR->error(true, _("The denominator in mod function cannot be zero"), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);
		} else {
			mngr->set(fmodl(vargs[0]->value(), vargs[1]->value()));
		}
	}
}

SinFunction::SinFunction() : Function("Trigonometry", "sin", 1, "Sine") {}
void SinFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(sinl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
CosFunction::CosFunction() : Function("Trigonometry", "cos", 1, "Cosine") {}
void CosFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);	
	else mngr->set(cosl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
TanFunction::TanFunction() : Function("Trigonometry", "tan", 1, "Tangent") {}
void TanFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(tanl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
SinhFunction::SinhFunction() : Function("Trigonometry", "sinh", 1, "Hyperbolic sine") {}
void SinhFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(sinhl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
CoshFunction::CoshFunction() : Function("Trigonometry", "cosh", 1, "Hyperbolic cosine") {}
void CoshFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(coshl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
TanhFunction::TanhFunction() : Function("Trigonometry", "tanh", 1, "Hyperbolic tangent") {}
void TanhFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(tanhl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
AsinFunction::AsinFunction() : Function("Trigonometry", "asin", 1, "Arcsine") {}
void AsinFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(asinl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
AcosFunction::AcosFunction() : Function("Trigonometry", "acos", 1, "Arccosine") {}
void AcosFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(acosl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
AtanFunction::AtanFunction() : Function("Trigonometry", "atan", 1, "Arctangent") {}
void AtanFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(atanl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
AsinhFunction::AsinhFunction() : Function("Trigonometry", "asinh", 1, "Hyperbolic arcsine") {}
void AsinhFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(asinhl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
AcoshFunction::AcoshFunction() : Function("Trigonometry", "acosh", 1, "Hyperbolic arccosine") {}
void AcoshFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(acoshl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
AtanhFunction::AtanhFunction() : Function("Trigonometry", "atanh", 1, "Hyperbolic arctangent") {}
void AtanhFunction::calculate2(Manager *mngr) {
//	if(!vargs[0]->isNumber()) mngr->set(this, CALCULATOR->setAngleValue(vargs[0]), NULL);
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(atanhl(CALCULATOR->getAngleValue(vargs[0]->value())));
}
LogFunction::LogFunction() : Function("Exponents and Logarithms", "ln", 1, "Natural Logarithm") {}
void LogFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(logl(vargs[0]->value()));
}
Log10Function::Log10Function() : Function("Exponents and Logarithms", "log", 1, "Base-10 Logarithm") {}
void Log10Function::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(log10l(vargs[0]->value()));
}
Log2Function::Log2Function() : Function("Exponents and Logarithms", "log2", 1, "Base-2 Logarithm") {}
void Log2Function::calculate2(Manager *mngr) {
	if(!vargs[0]->isNumber()) mngr->set(this, vargs[0], NULL);
	else mngr->set(log2l(vargs[0]->value()));
}
ExpFunction::ExpFunction() : Function("Exponents and Logarithms", "exp", 1, "e raised to the power X") {}
void ExpFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isNonNullNumber()) mngr->set(expl(vargs[0]->value()));
	else if(vargs[0]->type() == NULL_MANAGER) mngr->set(1, 1);
	else {
		mngr->set(E_VALUE);
		mngr->add(vargs[0], RAISE);
	}	
}
Exp10Function::Exp10Function() : Function("Exponents and Logarithms", "exp10", 1, "10 raised the to power X") {}
void Exp10Function::calculate2(Manager *mngr) {
	if(vargs[0]->isNonNullNumber()) mngr->set(exp10l(vargs[0]->value()));
	else if(vargs[0]->type() == NULL_MANAGER) mngr->set(1, 1);
	else {
		mngr->set(10.0L);
		mngr->add(vargs[0], RAISE);
	}	
}
Exp2Function::Exp2Function() : Function("Exponents and Logarithms", "exp2", 1, "2 raised the to power X") {}
void Exp2Function::calculate2(Manager *mngr) {
	if(vargs[0]->isNonNullNumber()) mngr->set(exp2l(vargs[0]->value()));
	else if(vargs[0]->type() == NULL_MANAGER) mngr->set(1, 1);
	else {
		mngr->set(2.0L);
		mngr->add(vargs[0], RAISE);
	}	
}
SqrtFunction::SqrtFunction() : Function("Exponents and Logarithms", "sqrt", 1, "Square Root") {

}
void SqrtFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isNonNullNumber()) {
		if(vargs[0]->value() < 0) {
			CALCULATOR->error(true, _("Trying to calculate the square root of a negative number (%s)."), d2s(vargs[0]->value()).c_str(), NULL);
			mngr->set(this, vargs[0], NULL);
			return;
		}
		mngr->set(sqrtl(vargs[0]->value()));
	} else if(vargs[0]->type() != NULL_MANAGER) {
		mngr->set(vargs[0]);
		Manager *mngr2 = new Manager(1, 2);
		mngr->add(mngr2, RAISE);
		mngr2->unref();
	} else {
		mngr->set(0.0L);
	}	
}
CbrtFunction::CbrtFunction() : Function("Exponents and Logarithms", "cbrt", 1, "Cube Root") {}
void CbrtFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isNonNullNumber()) mngr->set(cbrtl(vargs[0]->value()));
	else if(vargs[0]->type() != NULL_MANAGER) {
		mngr->set(vargs[0]);
		Manager *mngr2 = new Manager(1, 3);		
		mngr->add(mngr2, RAISE);
		mngr2->unref();
	}
}
HypotFunction::HypotFunction() : Function("Geometry", "hypot", 2, "Hypotenuse") {

}
void HypotFunction::calculate2(Manager *mngr) {
	mngr->value(hypotl(vargs[0]->value(), vargs[1]->value()));
	if(vargs[0]->isNonNullNumber() && vargs[1]->isNonNullNumber()) mngr->set(hypotl(vargs[0]->value(), vargs[1]->value()));
	else if(vargs[0]->type() != NULL_MANAGER) {
		mngr->set(vargs[0]);
		mngr->addInteger(2, RAISE);
		Manager *mngr2 = new Manager(vargs[1]);
		mngr2->addInteger(2, RAISE);		
		mngr->add(mngr2, RAISE);
		mngr2->unref();
		mngr2 = new Manager(1, 2);
		mngr->add(mngr2, RAISE);
		mngr2->unref();
	}	
}
SumFunction::SumFunction() : Function("Statistics", "sum", -1, "Sum") {}
void SumFunction::calculate2(Manager *mngr) {
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], ADD);
	}
}
MeanFunction::MeanFunction() : Function("Statistics", "mean", -1, "Mean") {}
void MeanFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], ADD);
	}
	mngr->addInteger(vargs.size(), DIVIDE);	
}
MedianFunction::MedianFunction() : Function("Statistics", "median", -1, "Median") {}
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
		mngr->addFloat(*it, ADD);
		mngr->addFloat(2, DIVIDE);
	} else {
		++it;
		mngr->value(*it);
	}
}
MinFunction::MinFunction() : Function("Statistics", "min", -1, "Min") {}
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
MaxFunction::MaxFunction() : Function("Statistics", "max", -1, "Max") {}
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
ModeFunction::ModeFunction() : Function("Statistics", "mode", -1, "Mode") {}
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
NumberFunction::NumberFunction() : Function("Statistics", "number", -1, "Number") {}
void NumberFunction::calculate2(Manager *mngr) {
	mngr->value(vargs.size());
}
StdDevFunction::StdDevFunction() : Function("Statistics", "stddev", -1, "Standard Deviation") {}
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
StdDevSFunction::StdDevSFunction() : Function("Statistics", "stddevs", -1, "Standard Deviation (random sampling)") {}
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
RandomFunction::RandomFunction() : Function("General", "rand", 0, "Random Number") {}
Manager *RandomFunction::calculate(const string &eq) {
	Manager *mngr = new Manager();
	mngr->set(drand48());
	return mngr;
}

BASEFunction::BASEFunction() : Function("General", "BASE", 2, "Number Base") {

}
Manager *BASEFunction::calculate(const string &eq) {
	int itmp = stringArgs(eq);
	if(testArgCount(itmp)) {
		Manager *mngr = CALCULATOR->calculate(svargs[1]);	
		int base = (int) mngr->value();
		long double value = 0;
		if(base < 2 || base > 36) {
			CALCULATOR->error(false, _("Base must be between 2 and 36 (was %s) for function %s()."), i2s(base).c_str(), name().c_str(), NULL);
			Manager *mngr2 = new Manager(base);
			Manager *mngr3 = new Manager(svargs[0]);			
			mngr->set(this, mngr3, mngr2, NULL);
			mngr2->unref();
			mngr3->unref();
		} else {
			value = (long double) strtol(svargs[0].c_str(), NULL, base);
			clearSVArgs();
			mngr->set(value);
		}
		return mngr;
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
BINFunction::BINFunction() : Function("General", "BIN", 1, "Binary") {

}
Manager *BINFunction::calculate(const string &eq) {
	Manager *mngr = new Manager();
	string str = eq;
	remove_blanks(str);
	mngr->set((long double) strtol(str.c_str(), NULL, 2));
	return mngr;
}
OCTFunction::OCTFunction() : Function("General", "OCT", 1, "Octal") {

}
Manager *OCTFunction::calculate(const string &eq) {
	Manager *mngr = new Manager();
	mngr->set((long double) strtol(eq.c_str(), NULL, 8));
	return mngr;
}
HEXFunction::HEXFunction() : Function("General", "HEX", 1, "Hexadecimal") {

}
Manager *HEXFunction::calculate(const string &eq) {
	string stmp;
	if(eq.length() >= 2 && eq[0] == '0' && (eq[1] == 'x' || eq[1] == 'X'))
		stmp = eq;
	else {
		stmp = "0x";
		stmp += eq;
	}
	Manager *mngr = new Manager();
	mngr->set(strtold(stmp.c_str(), NULL));
	return mngr;
}

