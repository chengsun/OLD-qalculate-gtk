/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Fraction.h"
#include "util.h"
#include "Calculator.h"

Fraction::Fraction() {
	clear();
}
Fraction::Fraction(long int numerator_, long int denominator_, long int exp10_) {
	set(numerator_, denominator_, exp10_);
}
Fraction::Fraction(const Integer *numerator_, const Integer *denominator_, const Integer *exp10_) {
	set(numerator_, denominator_, exp10_);
}
Fraction::Fraction(const Fraction *fr) {
	set(fr);
}
Fraction::Fraction(string str) {
	set(str);
}

Fraction::~Fraction() {
}
void Fraction::clear() {
	b_exact = true;
	num.clear();
	den.set(1);
}
void Fraction::set(long int numerator_, long int denominator_, long int exp10_) {
	num.set(numerator_);
	den.set(denominator_);
	if(den.isNegative()) {
		num.setNegative(!num.isNegative());
		den.setNegative(false);
	}
	Integer *divisor;
	if(num.gcd(&den, &divisor)) {
		num.divide(divisor);
		den.divide(divisor);
	}
	if(exp10_ < 0) {
		den.exp10(exp10_);
	} else {
		num.exp10(exp10_);
	}	
	delete divisor;
	b_exact = true;
}
void Fraction::setFloat(long double value_) {
	set(d2s(value_));
}
void Fraction::set(const Integer *numerator_, const Integer *denominator_, const Integer *exp10_) {
	if(numerator_) num.set(numerator_);
	else num.set(1);
	if(denominator_) den.set(denominator_);
	else den.set(1);
	if(den.isNegative()) {
		num.setNegative(!num.isNegative());
		den.setNegative(false);
	}
	Integer *divisor;
	if(num.gcd(&den, &divisor)) {
		num.divide(divisor);
		den.divide(divisor);
	}
	if(exp10_) {
		if(exp10_->isNegative()) {
			den.exp10(exp10_);
		} else {
			num.exp10(exp10_);
		}	
	}
	delete divisor;
	b_exact = true;
}
bool Fraction::set(string str) {
	den.set(1);
	num.clear();
	remove_blank_ends(str);
	bool b = false;
	bool numbers_started = false, minus = false, in_decimals = false;
	for(int index = 0; index < str.size(); index++) {
		if(str[index] >= '0' && str[index] <= '9') {
			num.multiply(10);
			num.add(str[index] - '0');
			if(in_decimals) {
				den.multiply(10);
			}
			numbers_started = true;
		} else if(str[index] == 'E') {
			index++;
			numbers_started = false;
			bool exp_minus = false;
			Integer exp;
			while(index < str.size()) {
				if(str[index] >= '0' && str[index] <= '9') {				
					exp.multiply(10);
					exp.add(str[index] - '0');
					numbers_started = true;
				} else if(!numbers_started && str[index] == '-') {
					exp_minus = !exp_minus;
				}
				index++;
			}
			if(exp_minus) {
				den.exp10(&exp);
			} else {
				num.exp10(&exp);
			}
			break;
		} else if(str[index] == '.') {
			in_decimals = true;
		} else if(!numbers_started && str[index] == '-') {
			minus = !minus;
		}
	}
	if(minus) num.setNegative(!num.isNegative());
	Integer *divisor;
	if(num.gcd(&den, &divisor)) {
		num.divide(divisor);
		den.divide(divisor);
	}
	delete divisor;
	b_exact = true;
}
void Fraction::set(const Fraction *fr) {
	if(!fr) return;
	num.set(fr->numerator());
	den.set(fr->denominator());
	b_exact = fr->isPrecise();
}
bool Fraction::equals(const Fraction *fr) const {
	if(!fr) return false;
	return num.equals(fr->numerator()) && den.equals(fr->denominator());
}
int Fraction::compare(const Fraction *fr) const {
	if(equals(fr)) return 0;
	if(fr->value() > value()) return 1;
	return -1;
}
const Integer *Fraction::numerator() const {
	return &num;
}
const Integer *Fraction::denominator() const {
	return &den;
}
long double Fraction::value() const {
	return strtold(print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY, PRECISION + 5).c_str(), NULL);			
}
bool Fraction::isNegative() const {
	return num.isNegative();
}
bool Fraction::isOne() const {
	return num.isOne() && den.isOne();
}
bool Fraction::isMinusOne() const {
	return num.isMinusOne() && den.isOne();
}
bool Fraction::isInteger() const {
	return den.isOne();
}
bool Fraction::isPrecise() const {
	return b_exact;
}
void Fraction::setPrecise(bool is_precise) {
	b_exact = is_precise;
}
Integer *Fraction::getInteger() const {
	Integer *integer = new Integer(&num);
	integer->divide(&den);		
	return integer;
}
bool Fraction::isZero() const {
	return num.isZero();
}
void Fraction::subtract(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;
	if(fr->isZero()) {return;}	
	num.multiply(fr->denominator());
	Integer num2(fr->numerator());
	num2.multiply(&den);		
	num2.setNegative(!num2.isNegative());
	num.add(&num2);
	den.multiply(fr->denominator());			
	clean();
}
void Fraction::add(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;
	if(fr->isZero()) {return;}	
	num.multiply(fr->denominator());
	Integer num2(fr->numerator());
	num2.multiply(&den);		
	num.add(&num2);
	den.multiply(fr->denominator());			
	clean();
}
void Fraction::multiply(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;
	if(fr->isZero()) {clear(); return;}
	den.multiply(fr->denominator());
	num.multiply(fr->numerator());
	clean();
}
void Fraction::divide(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;
	if(fr->isZero()) {
		//division by zero!!!
		clear(); 
		return;
	}
	num.multiply(fr->denominator());
	den.multiply(fr->numerator());	
	clean();
}
void Fraction::sin() {
	setFloat(sinl(value()));
	b_exact = false;
}
void Fraction::asin() {
	setFloat(asinl(value()));
	b_exact = false;
}
void Fraction::sinh() {
	setFloat(sinhl(value()));
	b_exact = false;
}
void Fraction::asinh() {
	setFloat(asinhl(value()));
	b_exact = false;
}
void Fraction::cos() {
	setFloat(cosl(value()));
	b_exact = false;
}
void Fraction::acos() {
	setFloat(acosl(value()));
	b_exact = false;
}
void Fraction::cosh() {
	setFloat(coshl(value()));
	b_exact = false;
}
void Fraction::acosh() {
	setFloat(acoshl(value()));
	b_exact = false;
}
void Fraction::tan() {
	setFloat(tanl(value()));
	b_exact = false;
}
void Fraction::atan() {
	setFloat(atanl(value()));
	b_exact = false;
}
void Fraction::tanh() {
	setFloat(tanhl(value()));
	b_exact = false;
}
void Fraction::atanh() {
	setFloat(atanhl(value()));
	b_exact = false;
}
void Fraction::sqrt() {
	setFloat(sqrtl(value()));
	b_exact = false;
}	
void Fraction::cbrt() {
	setFloat(cbrtl(value()));
	b_exact = false;
}
void Fraction::log() {
	setFloat(logl(value()));
	b_exact = false;
}
void Fraction::log2() {
	setFloat(log2l(value()));
	b_exact = false;
}
void Fraction::log10() {
	setFloat(log10l(value()));
	b_exact = false;
}
void Fraction::exp() {
	setFloat(expl(value()));
	b_exact = false;
}
void Fraction::exp2() {
	setFloat(exp2l(value()));
	b_exact = false;
}
void Fraction::exp10(const Fraction *fr) {
	Fraction ten(10);
	if(fr) {
		ten.pow(fr);
		multiply(&ten);
	} else {
		ten.pow(this);
		set(&ten);
	}
}
bool Fraction::add(MathOperation op, const Fraction *fr) {
	if(!fr) return false;
//	printf("FRPRE [%s] %c [%s]\n", print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str(), op2ch(op), fr->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str());		
	switch(op) {
		case SUBTRACT: {
			subtract(fr);
			break;
		}
		case ADD: {
			add(fr);
			break;
		} 
		case MULTIPLY: {
			multiply(fr);
			break;
		}
		case DIVIDE: {
			divide(fr);
			break;
		}		
		case RAISE: {
			pow(fr);
			break;
		}
		case EXP10: {
			exp10(fr);
			break;
		}
	}
//	printf("FRPOST [%s] %c [%s]\n", print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str(), op2ch(op), fr->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str());	
	return true;	
}
void Fraction::clean() {
	if(num.isZero()) return;
	if(den.isNegative()) {
		den.setNegative(false);
		num.setNegative(!num.isNegative());
	}
	Integer *divisor;
	if(num.gcd(&den, &divisor)) {
		num.divide(divisor);
		den.divide(divisor);
	}
	delete divisor;	
}
void Fraction::round() {
	Integer *reminder;
	bool was_negative = num.isNegative();
	if(!num.divide(&den, &reminder)) {
		den.subtract(reminder);
		int comp = den.compare(reminder);
		if(comp >= 0) {
			if(was_negative) num.add(-1);
			else num.add(1);
		}
	}
	den.set(1);
	delete reminder;
}
void Fraction::abs() {
	num.setNegative(false);
}
void Fraction::floor() {
	if(num.isNegative()) {
		if(!num.divide(&den)) {
			num.add(-1);
		}
		den.set(1);
	} else {
		trunc();
	}
}
void Fraction::ceil() {
	if(num.isNegative()) {
		trunc();	
	} else {
		if(!num.divide(&den)) {
			num.add(1);
		}
		den.set(1);
	}
}
void Fraction::trunc() {
	num.divide(&den);
	den.set(1);
}
void Fraction::mod() {
	Integer *reminder;
	num.divide(&den, &reminder);
	num.set(reminder);
	den.set(1);
	delete reminder;
}
void Fraction::rem() {
	Integer *reminder;
	Integer *chosen;
	if(!num.divide(&den, &reminder)) {
		chosen = reminder;
		den.subtract(reminder);
		int comp = den.compare(reminder);
		if(comp > 0) {	
			chosen = &den;
		}
	} else {
		chosen = reminder;
	}
	num.set(chosen);
	den.set(1);
	delete reminder;
}
void Fraction::pow(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;
	if(fr->isZero()) {
		set(1);
		return;
	} 
	if(fr->isOne()) return;
	if(fr->denominator()->isOne()) {
//	root(fr->denominator());
		Integer exp(fr->numerator());
		if(exp.isNegative()) {
			exp.setNegative(false);
			Integer d(&den);
			den.set(&num);
			num.set(&d);
		}
		num.pow(&exp);
		den.pow(&exp);
		clean();
	} else {
		setFloat(powl(value(), fr->value()));
		b_exact = false;
	}
}
void Fraction::root(const Integer *nth) {
	if(nth->isOne()) return;
	Fraction *x = this;
	Fraction a(this);
	Fraction n(nth);
	Fraction n_m1(&n);
	Fraction one(1);
	n_m1.add(SUBTRACT, &one);
	for(int i = 0; i <= 10; i++) {
		Fraction tmp(x);
		tmp.add(RAISE, &n);
		tmp.add(SUBTRACT, &a);		
		Fraction tmp2(x);
		tmp2.add(RAISE, &n_m1);
		tmp2.add(MULTIPLY, &n);
		tmp.add(DIVIDE, &tmp2);
		x->add(SUBTRACT, &tmp);
	}
	clean();
	b_exact = false;
}
void Fraction::root(long int nth) {
	Integer n(nth);
	root(&n);
}

bool Fraction::floatify(int precision) {
	//if log10=integer do nothing
	Integer *reminder, *reminder2 = NULL;
	Integer d(&den);
	den.set(1);
	bool exact = num.divide(&d, &reminder);
	while(!exact && precision) {
		reminder->multiply(10);
		exact = reminder->divide(&d, &reminder2);
		num.multiply(10);
		if(!reminder->equals(reminder2)) num.add(reminder);
		den.multiply(10);
		delete reminder;
		reminder = reminder2;
		precision--;
	}
	if(!exact) {
		reminder->multiply(10);
		reminder->divide(&d, &reminder2);
		d.subtract(reminder2);		
		int comp = d.compare(reminder2);
		if(comp >= 0) {
			if(num.isNegative()) num.add(-1);
			else num.add(1);			
		}
		b_exact = false;
	}
	if(reminder2) delete reminder2;
	return exact;	
}
string Fraction::print(NumberFormat nrformat, int displayflags, int precision, int min_decimals, int max_decimals, Prefix *prefix, bool *usable, bool toplevel, bool *plural, long int *l_exp, bool in_composite, bool in_power) const {
	if(displayflags & DISPLAY_FORMAT_DECIMAL_ONLY) {
		Fraction fr(this);
		fr.floatify(precision);
		string str = fr.numerator()->print();
		int l10 = 0;
		Integer d(fr.denominator());
		while(d.div10()) {
			l10++;
		}
		if(l10) {
			l10 = str.length() - l10;
			for(; l10 < 1; l10++) {
				str.insert(str.begin(), '0');
			}
			str.insert(l10, DOT_STR);
		}
		return str;
	}
	Integer whole(&num);
	Integer *part;
	whole.divide(&den, &part);
	Integer den_spec(&den);
	Integer exp_spec();
	string str_spec = "", str_prefix = "";

/*	switch(nrformat) {
		case NUMBER_FORMAT_PREFIX: {
			if(l_exp) {
				if(prefix) {
					exp_spec -= prefix->exponent(*l_exp);
					str_prefix = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
				} else {
					prefix = CALCULATOR->getBestPrefix(exp, *l_exp);
					long long int test_exp = exp - prefix->exponent(*l_exp);
					if((exp > 1 && exp > test_exp) || (exp < 1 && exp < test_exp)) {
						exp_spec = test_exp;
						str_prefix = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);						
					}
				}
				if((displayflags & DISPLAY_FORMAT_SHORT_UNITS) && (displayflags & DISPLAY_FORMAT_NONASCII)) {
					gsub("micro", SIGN_MICRO, str_prefix);
				}
				if(in_composite && exp_spec == 0 && den == 1 && num == 1) {
					return str_prefix;
				}
			}
		}
		case NUMBER_FORMAT_DECIMALS: {
		}		
		case NUMBER_FORMAT_HEX: {
		}
		case NUMBER_FORMAT_OCTAL: {
		}
		case NUMBER_FORMAT_BIN: {
		}					
		case NUMBER_FORMAT_NORMAL: {
			if(exp_spec != 0 && exp_spec > -precision && exp_spec < precision) {
				part = num;
				if(exp_spec < 0) {
					for(long long int i = 0; i < -exp_spec; i++) {
						den_spec *= 10;
					}
				} else if(exp_spec > 0) {
					for(long long int i = 0; i < exp_spec; i++) {
						part *= 10;
					}				
				}
				whole = part / den_spec;
				part = part % den_spec;		
				break;
			}
		}			
		case NUMBER_FORMAT_EXP: {
		}
		case NUMBER_FORMAT_EXP_PURE: {
			if(exp_spec != 0) {
				str_spec += " ";
				if(displayflags & DISPLAY_FORMAT_NONASCII) {
					str_spec += SIGN_MULTIDOT;
				} else {
					str_spec += MULTIPLICATION_STR;
				}
				str_spec += " 1";
				if(displayflags & DISPLAY_FORMAT_TAGS) {	
					str_spec += "<small>";
				 	str_spec += "E";
					str_spec += "</small>";		
				} else {
					str_spec += "E";
				}
				str_spec += lli2s(exp_spec);
			}
			break;
		}		
	}*/

	string str_base = "";
	if(!whole.isZero()) {
		if(whole.isNegative()) {
			if(displayflags & DISPLAY_FORMAT_TAGS) {
				str_base += SIGN_MINUS;
			} else {
				str_base += MINUS_STR;
			}	
			whole.setNegative(false);	
		}
		string str_whole = whole.print();
		str_base += str_whole;
	}

	if(!part->isZero()) {
		if(part->isNegative()) {
			if(whole.isZero()) {
				if(displayflags & DISPLAY_FORMAT_TAGS) {
					str_base += SIGN_MINUS;
				} else {
					str_base += MINUS_STR;
				}
			}
			part->setNegative(false);
		}

		string str_num = part->print();
		string str_den = den_spec.print();

		if(!whole.isZero()) str_base += " ";
		if(displayflags & DISPLAY_FORMAT_TAGS) {	
			str_base += "<sup>";
		 	str_base += str_num;
			str_base += "</sup>";		
		} else {
			str_base += str_num;
		}

		if(displayflags & DISPLAY_FORMAT_NONASCII) {
			str_base += SIGN_DIVISION;
		} else {
			str_base += DIVISION_STR;	
		}

		if(displayflags & DISPLAY_FORMAT_TAGS) {	
			str_base += "<small>";
	 		str_base += str_den;
			str_base += "</small>";		
		} else {
			str_base += str_den;
		}	
	}

	if(!str_prefix.empty()) {
		str_spec += " ";
		str_spec += str_prefix;
	}
	delete part;
	if(str_base.empty() && str_spec.empty()) str_base = "0";
	return str_base + str_spec;	
}

