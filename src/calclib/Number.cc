/*
    Qalculate    

    Copyright (C) 2004  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Number.h"
#include "Prefix.h"

#include <sstream>
#include "util.h"

using namespace cln;

void cln::cl_abort() {
	CALCULATOR->error(true, "CLN Error: see terminal output (probably too large number)", NULL);
	if(CALCULATOR->busy()) {
		CALCULATOR->abort_this();
	} else {
		exit(0);
	}
}

string printCL_I(cl_I integ, int base = 10, bool display_sign = true) {
	if(base == BASE_ROMAN_NUMERALS) {
		if(!zerop(integ) && integ < 10000 && integ > -10000) {
			string str;
			int value = cl_I_to_int(integ);
			if(value < 0) {
				value = -value;
				if(display_sign) {
					str += "-";
				}
			}
			int times = value / 1000;
			for(; times > 0; times--) {
				str += "M";
			}
			value = value % 1000;
			times = value / 100;
			if(times == 9) {
				str += "C";
				str += "M";
				times = 0;
			} else if(times >= 5) {
				str += "D";
				times -= 5;
			} else if(times == 4) {
				times = 0;
				str += "C";
				str += "D";
			}
			for(; times > 0; times--) {
				str += "C";
			}
			value = value % 100;
			times = value / 10;
			if(times == 9) {
				str += "X";
				str += "C";
				times = 0;
			} else if(times >= 5) {
				str += "L";
				times -= 5;
			} else if(times == 4) {
				times = 0;
				str += "X";
				str += "L";
			}
			for(; times > 0; times--) {
				str += "X";
			}
			value = value % 10;
			times = value;
			if(times == 9) {
				str += "I";
				str += "X";
				times = 0;
			} else if(times >= 5) {
				str += "V";
				times -= 5;
			} else if(times == 4) {
				times = 0;
				str += "I";
				str += "V";
			}
			for(; times > 0; times--) {
				str += "I";
			}
			return str;
		}
		base = 10;
	}
	cl_print_flags flags;
	flags.rational_base = base;
	ostringstream stream;
	print_integer(stream, flags, integ);
	string cl_str = stream.str();
	if(minusp(integ)) {
		cl_str.erase(0, 1);
	}	
	if(cl_str[cl_str.length() - 1] == '.') {
		cl_str.erase(cl_str.length() - 1, 1);
	}
	string str = "";
	if(base == 16) {
		str += "0x";
	} else if(base == 8) {
		str += "0";
	} 
	str += cl_str;
	if(base == 2) {
		int i2 = str.length() % 4;
		if(i2 != 0) i2 = 4 - i2;
		for(int i = (int) str.length() - 4; i > 0; i -= 4) {
			str.insert(i, 1, ' ');
		} 
		for(; i2 > 0; i2--) {
			str.insert(str.begin(), 1, '0');
		}
	}
	if(minusp(integ) && display_sign) {
		str.insert(str.begin(), 1, '-');
	}		
	return str;
}


Number::Number() {
	clear();
}
Number::Number(string number) {
	set(number);
}
Number::Number(long int numerator, long int denominator, long int exp_10) {
	set(numerator, denominator, exp_10);
}
Number::Number(const Number *o) {
	set(o);
}
Number::~Number() {
}

void Number::set(string number) {
	cl_I num = 0;
	cl_I den = 1;
	remove_blank_ends(number);
	bool numbers_started = false, minus = false, in_decimals = false, b_cplx = false;
	for(unsigned int index = 0; index < number.size(); index++) {
		if(number[index] >= '0' && number[index] <= '9') {
			num *= 10;
			num += number[index] - '0';
			if(in_decimals) {
				den *= 10;
			}
			numbers_started = true;
		} else if(number[index] == 'E') {
			index++;
			numbers_started = false;
			bool exp_minus = false;
			cl_I exp;
			while(index < number.size()) {
				if(number[index] >= '0' && number[index] <= '9') {				
					exp *= 10;
					exp += number[index] - '0';
					numbers_started = true;
				} else if(!numbers_started && number[index] == '-') {
					exp_minus = !exp_minus;
				}
				index++;
			}
			if(exp_minus) {
				cl_I cl10 = 10;
				exp = cln::abs(exp);
				den *= expt_pos(cl10, exp);
			} else {
				cl_I cl10 = 10;
				num *= expt_pos(cl10, exp);
			}
			break;
		} else if(number[index] == '.') {
			in_decimals = true;
		} else if(number[index] == ':') {
			if(in_decimals) {
				CALCULATOR->error(true, _("\':\' in decimal number ignored (decimal point detected)."), NULL);
			} else {
				vector<cl_I> nums;
				nums.push_back(num);
				num = 0;
				for(index++; index < number.size(); index++) {
					if(number[index] >= '0' && number[index] <= '9') {				
						num *= 10;
						num += number[index] - '0';
					} else if(number[index] == ':') {
						nums.push_back(num);
						num = 0;
					} else if(number[index] == 'E')	{
						index--;
						break;
					} else if(number[index] == '.') {
						CALCULATOR->error(true, _("Decimal point in sexagesimal number treated as \':\'."), NULL);
						nums.push_back(num);
						num = 0;
					} else if(number[index] == 'i') {
						b_cplx = true;
					}
				}
				for(int i = nums.size() - 1; i >= 0; i--) {
					den *= 60;
					nums[i] *= den;
					num += nums[i];
				}
			}
		} else if(!numbers_started && number[index] == '-') {
			minus = !minus;
		} else if(number[index] == 'i') {
			b_cplx = true;
		}
	}
	if(minus) num = -num;
	if(b_cplx) {
		value = cln::complex(0, num / den);
	} else {
		value = num / den;
	}
	setApproximate(false);
}
void Number::set(long int numerator, long int denominator, long int exp_10) {
	value = numerator;
	if(denominator) {
		value /= denominator;
	}
	if(exp_10 != 0) {
		exp10(exp_10);
	}	
	setApproximate(false);
}
void Number::setFloat(double d_value) {
//	value = d_value;
//	setApproximate(true);
	set(d2s(d_value));
}
void Number::setCln(cl_N cln_value) {
	value = cln_value;
	setApproximate(false);
}
void Number::setImaginaryPart(const Number *o) {
	value = cln::complex(cln::realpart(value), cln::realpart(o->clnNumber()));
}
void Number::set(const Number *o) {
	value = o->clnNumber();
	setApproximate(o->isApproximate());
}
void Number::clear() {
	value = 0;
	setApproximate(false);
}

const cl_N &Number::clnNumber() const {
	return value;
}

double Number::floatValue() const {
	return double_approx(cln::realpart(value));
}
int Number::intValue(bool *overflow) const {
	if(isGreaterThan(INT_MAX)) {
		if(overflow) *overflow = true;
		return INT_MAX;
	} else if(isLessThan(INT_MIN)) {
		if(overflow) *overflow = true;
		return INT_MIN;
	}
	return cl_I_to_int(cln::round1(cln::realpart(value)));
}
long int Number::longIntValue(bool *overflow) const {
	if(isGreaterThan(LONG_MAX)) {
		if(overflow) *overflow = true;
		return LONG_MAX;
	} else if(isLessThan(LONG_MIN)) {
		if(overflow) *overflow = true;
		return LONG_MIN;
	}
	return cl_I_to_long(cln::round1(cln::realpart(value)));
}

bool Number::isApproximate() const {
	return b_approx;
}
void Number::setApproximate(bool is_approximate) {
	b_approx = is_approximate;
}

Number *Number::realPart() const {
	Number *real_part = new Number();
	real_part->setCln(cln::realpart(value));
	return real_part;
}
Number *Number::imaginaryPart() const {
	Number *imag_part = new Number();
	imag_part->setCln(cln::imagpart(value));
	return imag_part;
}
Number *Number::numerator() const {
	Number *num = new Number();
	num->setCln(cln::numerator(cln::rational(cln::realpart(value))));
	return num;
}
Number *Number::denominator() const {
	Number *den = new Number();
	den->setCln(cln::denominator(cln::rational(cln::realpart(value))));
	return den;
}
Number *Number::complexNumerator() const {
	Number *num = new Number();
	num->setCln(cln::numerator(cln::rational(cln::imagpart(value))));
	return num;
}
Number *Number::complexDenominator() const {
	Number *den = new Number();
	den->setCln(cln::denominator(cln::rational(cln::imagpart(value))));
	return den;
}

bool Number::hasRealPart() const {
	return !cln::zerop(cln::realpart(value));
}
bool Number::isComplex() const {
	return !cln::zerop(cln::imagpart(value));
}
bool Number::isInteger() const {
	return !isComplex() && cln::zerop(cln::truncate2(cln::realpart(value)).remainder);
}
bool Number::isFraction() const {
	if(!isComplex()) {
		cl_R real_value = cln::realpart(value);
		return real_value < 1 && real_value > -1;
	}
	return false; 
}
bool Number::isZero() const {
	return cln::zerop(value);
}
bool Number::isOne() const {
	return value == 1;
}
bool Number::isI() const {
	return cln::zerop(cln::realpart(value)) && cln::imagpart(value) == 1;
}
bool Number::isMinusOne() const {
	return value == -1;
}
bool Number::isMinusI() const {
	return cln::zerop(cln::realpart(value)) && cln::imagpart(value) == -1;
}
bool Number::isNegative() const {
	return !isComplex() && cln::minusp(cln::realpart(value));
}
bool Number::isPositive() const {
	return !isComplex() && cln::plusp(cln::realpart(value));
}
bool Number::realPartIsNegative() const {
	return cln::minusp(cln::realpart(value));
}
bool Number::realPartIsPositive() const {
	return cln::plusp(cln::realpart(value));
}
bool Number::imaginaryPartIsNegative() const {
	return cln::minusp(cln::imagpart(value));
}
bool Number::imaginaryPartIsPositive() const {
	return cln::plusp(cln::imagpart(value));
}
bool Number::hasNegativeSign() const {
	if(hasRealPart()) return realPartIsNegative();
	return imaginaryPartIsNegative();
}
bool Number::hasPositiveSign() const {
	if(hasRealPart()) return realPartIsPositive();
	return imaginaryPartIsPositive();
}
bool Number::equals(const Number *o) const {
	return value == o->clnNumber();
}
int Number::compare(const Number *o) const {
	if(!isComplex() && !o->isComplex()) {
		return cln::compare(cln::realpart(o->clnNumber()), cln::realpart(value));
	} else {
		if(equals(o)) return 0;
		return -2;
	}
}
int Number::compareImaginaryParts(const Number *o) const {
	return cln::compare(cln::imagpart(o->clnNumber()), cln::imagpart(value));
}
int Number::compareRealParts(const Number *o) const {
	return cln::compare(cln::realpart(o->clnNumber()), cln::realpart(value));
}
bool Number::isGreaterThan(const Number *o) const {
	return !isComplex() && !o->isComplex() && cln::realpart(value) > cln::realpart(o->clnNumber());
}
bool Number::isLessThan(const Number *o) const {
	return !isComplex() && !o->isComplex() && cln::realpart(value) < cln::realpart(o->clnNumber());
}
bool Number::isGreaterThanOrEqualTo(const Number *o) const {
	if(!isComplex()) {
		return cln::realpart(value) >= cln::realpart(o->clnNumber());
	} else {
		return equals(o);
	}
}
bool Number::isLessThanOrEqualTo(const Number *o) const {
	if(!isComplex()) {
		return cln::realpart(value) <= cln::realpart(o->clnNumber());
	} else {
		return equals(o);
	}
}
bool Number::equals(long int num, long int den) const {
	if(den == 1) return value == num;
	Number o(num, den);
	return equals(&o);
}
int Number::compare(long int num, long int den) const {
	if(den == 1) {
		if(!isComplex()) {
			return cln::compare(num, cln::realpart(value));
		} else {
			if(equals(num)) return 0;
			return -2;
		}
	}
	Number o(num, den);
	return compare(&o);
}
bool Number::isGreaterThan(long int num, long int den) const {
	if(den == 1) return !isComplex() && cln::realpart(value) > num;
	Number o(num, den);
	return isGreaterThan(&o);
}
bool Number::isLessThan(long int num, long int den) const {
	if(den == 1) return !isComplex() && cln::realpart(value) < num;
	Number o(num, den);
	return isLessThan(&o);
}
bool Number::isGreaterThanOrEqualTo(long int num, long int den) const {
	if(den == 1) {
		if(!isComplex()) {
			return cln::realpart(value) >= num;
		} else {
			return equals(num);
		}
	}
	Number o(num, den);
	return isGreaterThanOrEqualTo(&o);
}
bool Number::isLessThanOrEqualTo(long int num, long int den) const {
	if(den == 1) {
		if(!isComplex()) {
			return cln::realpart(value) <= num;
		} else {
			return equals(num);
		}
	}
	Number o(num, den);
	return isLessThanOrEqualTo(&o);
}
bool Number::isEven() const {
	return isInteger() && evenp(cln::numerator(cln::rational(cln::realpart(value))));
}
bool Number::isOdd() const {
	return isInteger() && oddp(cln::numerator(cln::rational(cln::realpart(value))));
}

bool Number::add(const Number *o) {
	if(o->isApproximate()) setApproximate();
	value += o->clnNumber();
	return true;
}
bool Number::subtract(const Number *o) {
	if(o->isApproximate()) setApproximate();
	value -= o->clnNumber();
	return true;
}
bool Number::multiply(const Number *o) {
	if(isZero()) return true;
	if(!o->isApproximate() && o->isZero()) {
		clear();
		return true;
	}
	if(o->isApproximate()) setApproximate();
	value *= o->clnNumber();
	return true;
}
bool Number::divide(const Number *o) {
	if(o->isZero()) {
		//division by zero!!!
		return false;
	}
	if(isZero()) {
		return true;
	}
	if(o->isApproximate()) setApproximate();
	value /= o->clnNumber();
	return true;
}
bool Number::recip() {
	if(isZero()) {
		//division by zero!!!
		return false;
	}
	value = cln::recip(value);
	return true;
}
int Number::raise(const Number *o, int solution) {
	if(isZero() && o->isNegative()) {
		CALCULATOR->error(true, _("Division by zero."), NULL);
		return false;
	}
	if(isZero()) {
		if(o->isZero()) {
			//0^0
			CALCULATOR->error(false, _("0^0 might be considered undefined"), NULL);
			set(1, 1);
			if(o->isApproximate()) {
				setApproximate();
			}
			return 1;
		}
		return 1;
	}
	if(o->isZero()) {
		set(1, 1);
		setApproximate(o->isApproximate());
		return 1;
	} 	
	if(o->isOne()) {
		return 1;
	}
	if(!isApproximate() && !o->isApproximate() && !o->isComplex()) {
		if(o->isInteger()) {
			cl_I exponent = cln::numerator(cln::rational(cln::realpart(o->clnNumber())));
			value = expt(value, exponent);
			return 1;
		} else if(!isComplex()) {
			cl_RA base = cln::rational(cln::realpart(value));
			cl_RA exponent = cln::rational(cln::realpart(o->clnNumber()));
			cl_I exp_num = cln::numerator(exponent);
			cl_I exp_den = cln::denominator(exponent);
			if(cln::abs(exp_den) < 10000) {
				base = expt(base, exp_num);
				bool b_complex = false;
				if(minusp(base) && evenp(exp_den)) {
					base = cln::abs(base);
					b_complex = true;
				}
				if(rootp(base, exp_den, &base)) {
					if(b_complex) {
						value = cln::complex(0, base);
					} else {
						value = base;
					}
					if(evenp(exp_den)) {
						if(solution == 2) {
							value = -value;
						}
						if(CALCULATOR->multipleRootsEnabled()) {
							return 2;
						}
					}
					return 1;
				}
			}
		}
	}
	if(CALCULATOR->alwaysExact()) return false;
	bool b_complex = isComplex();
	value = expt(value, o->clnNumber());
	setApproximate();
	if(!b_complex && !o->isComplex() && cln::evenp(cln::denominator(cln::rational(cln::realpart(o->clnNumber()))))) {
		if(solution == 2) {
			value = -value;
		}
		if(CALCULATOR->multipleRootsEnabled()) {
			return 2;
		}
	}
	return 1;
}
bool Number::exp10(const Number *o) {
	if(o && (o->isZero() || isZero())) {
		return true;
	}
	if(!o && isZero()) {
		set(1, 1);
		return true;
	}
	Number ten(10, 1);
	if(o) {
		if(!ten.raise(o)) {
			return false;
		}
		multiply(&ten);
	} else {
		if(!ten.raise(this)) {
			return false;
		}
		set(&ten);
	}
	return true;
}
bool Number::exp2(const Number *o) {
	if(o && (o->isZero() || isZero())) {
		return true;
	}
	if(!o && isZero()) {
		set(1, 1);
		return true;
	}
	Number two(2, 1);
	if(o) {
		if(!two.raise(o)) {
			return false;
		}
		multiply(&two);
	} else {
		if(!two.raise(this)) {
			return false;
		}
		set(&two);
	}
	return true;
}
bool Number::square() {
	value = cln::square(value);
	return true;
}

bool Number::add(long int num, long int den) {
	Number o(num, den);
	return add(&o);
}
bool Number::subtract(long int num, long int den) {
	Number o(num, den);
	return subtract(&o);
}
bool Number::multiply(long int num, long int den) {
	Number o(num, den);
	return multiply(&o);
}
bool Number::divide(long int num, long int den) {
	Number o(num, den);
	return divide(&o);
}
int Number::raise(long int num, long int den, int solution) {
	Number o(num, den);
	return raise(&o, solution);
}
bool Number::exp10(long int num, long int den) {
	Number o(num, den);
	return exp10(&o);
}
bool Number::exp2(long int num, long int den) {
	Number o(num, den);
	return exp2(&o);
}

bool Number::negate() {
	value = -value;
	return true;
}
void Number::setNegative(bool is_negative) {
	if(!isZero() && minusp(cln::realpart(value)) != is_negative) {
		value = cln::complex(-cln::realpart(value), cln::imagpart(value));
	}
}
bool Number::abs() {
	value = cln::abs(value);
	return true;
}
bool Number::round(const Number *o) {
	if(isComplex()) return false;
	if(o) {
		if(o->isComplex()) return false;
		if(o->isApproximate()) setApproximate();
		value = cln::round1(cln::realpart(value), cln::realpart(o->clnNumber()));
	} else {
		value = cln::round1(cln::realpart(value));
	}
	return true;
}
bool Number::floor(const Number *o) {
	if(isComplex()) return false;
	if(o) {
		if(o->isComplex()) return false;
		if(o->isApproximate()) setApproximate();
		value = cln::floor1(cln::realpart(value), cln::realpart(o->clnNumber()));
	} else {
		value = cln::floor1(cln::realpart(value));
	}
	return true;
}
bool Number::ceil(const Number *o) {
	if(isComplex()) return false;
	if(o) {
		if(o->isComplex()) return false;
		if(o->isApproximate()) setApproximate();
		value = cln::ceiling1(cln::realpart(value), cln::realpart(o->clnNumber()));
	} else {
		value = cln::ceiling1(cln::realpart(value));
	}
	return true;
}
bool Number::trunc(const Number *o) {
	if(isComplex()) return false;
	if(o) {
		if(o->isComplex()) return false;
		if(o->isApproximate()) setApproximate();
		value = cln::truncate1(cln::realpart(value), cln::realpart(o->clnNumber()));
	} else {
		value = cln::truncate1(cln::realpart(value));
	}
	return true;
}
bool Number::mod(const Number *o) {
	if(isComplex() || o->isComplex()) return false;
	value = cln::mod(cln::realpart(value), cln::realpart(o->clnNumber()));
	if(o->isApproximate()) setApproximate();
	return true;
}	
bool Number::frac() {
	if(isComplex()) return false;
	cl_N whole_value = cln::truncate1(cln::realpart(value));
	value -= whole_value;
	return true;
}
bool Number::rem(const Number *o) {
	if(isComplex() || o->isComplex()) return false;
	value = cln::rem(cln::realpart(value), cln::realpart(o->clnNumber()));
	if(o->isApproximate()) setApproximate();
	return true;
}

int Number::getBoolean() {
	if(isPositive()) {
		return 1;
	} else {
		return 0;
	}
}
void Number::toBoolean() {
	setTrue(isPositive());
}
void Number::setTrue(bool is_true) {
	if(is_true) {
		value = 1;
	} else {
		value = 0;
	}
}
void Number::setFalse() {
	setTrue(false);
}
void Number::setNOT() {
	setTrue(!isPositive());
}

void Number::e() {
	value = cln::exp1(); 
	setApproximate();
}
void Number::pi() {
	value = cln::pi(); 
	setApproximate();
}
void Number::catalan() {
	value = cln::catalanconst(); 
	setApproximate();
}
void Number::pythagoras() {
	value = 2;
	value = cln::sqrt(value); 
	setApproximate();
}
void Number::euler() {
	value = cln::eulerconst(); 
	setApproximate();
}
void Number::apery() {
	value = cln::zeta(3); 
	setApproximate();
}
void Number::golden() {
	value = 5;
	value = cln::sqrt(value); 
	value++;
	value /= 2;
	setApproximate();
}
bool Number::zeta() {
	if(isNegative() || !isInteger() || isZero() || isOne()) {
		CALCULATOR->error(true, _("Integral point for Riemann's zeta must be an integer > 1."), NULL);
		return false;
	}
	if(isGreaterThan(INT_MAX)) {
		CALCULATOR->error(true, _("Integral point for Riemann's zeta is too large."), NULL);
		return false;
	}
	if(CALCULATOR->alwaysExact()) return false;
	cl_I integ = cln::numerator(cln::rational(cln::realpart(value)));
	value = cln::zeta(cl_I_to_int(integ)); 
	setApproximate();
	return true;
}			

bool Number::sin() {
	if(isZero()) return true;
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::sin(value);
	setApproximate();
	return true;
}
bool Number::asin() {
	if(isZero()) return true;
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::asin(value);
	setApproximate();
	return true;
}
bool Number::sinh() {
	if(isZero()) return true;
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::sinh(value);
	setApproximate();
	return true;
}
bool Number::asinh() {
	if(isZero()) return true;
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::asinh(value);
	setApproximate();
	return true;
}
bool Number::cos() {
	if(isZero()) {
		set(1);
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::cos(value);
	setApproximate();
	return true;
}	
bool Number::acos() {
	if(isOne()) {
		clear();
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::acos(value);
	setApproximate();
	return true;
}
bool Number::cosh() {
	if(isZero()) {
		set(1);
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::cosh(value);
	setApproximate();
	return true;
}
bool Number::acosh() {
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::acosh(value);
	setApproximate();
	return true;
}
bool Number::tan() {
	if(isZero()) return true;
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::tan(value);
	setApproximate();
	return true;
}
bool Number::atan() {
	if(isZero()) return true;
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::atan(value);
	setApproximate();
	return true;
}
bool Number::tanh() {
	if(isZero()) return true;
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::tanh(value);
	setApproximate();
	return true;
}
bool Number::atanh() {
	if(isZero()) return true;
	if(isOne() || isMinusOne()) {
		CALCULATOR->error(true, _("The inverse hyperbolic tangent is infinite for 1 and -1."), NULL);
		return false;
	}
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::atanh(value);
	setApproximate();
	return true;
}
bool Number::ln() {
	if(isOne()) {
		clear();
		return true;
	}
	if(isZero()) {
		CALCULATOR->error(true, _("The natural logarithm is infinite for zero."), NULL);
		return false;
	}
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::log(value);
	setApproximate();
	return true;
}
bool Number::log(const Number *o) {
	if(isOne()) {
		clear();
		return true;
	}
	if(isZero()) {
		CALCULATOR->error(true, _("Logarithms is infinite for zero."), NULL);
		return false;
	}
	if(o->isZero() || o->isOne()) {
		CALCULATOR->error(true, _("Logarithms with a base of zero or one is undefined."), NULL);
		return false;
	}
	if(!isApproximate() && !o->isApproximate() && !isComplex() && !o->isComplex()) {		
		bool b_minus = false;
		cl_RA ra_value = rational(cln::realpart(value));
		if(ra_value < 1 && ra_value > -1) {
			//logp sets clns to zero when result is negative (when this < 1) !!?! 
			b_minus = true;
			ra_value = cln::recip(ra_value);
		}
		cl_RA ra_base = rational(cln::realpart(o->clnNumber()));
		if(logp(ra_value, ra_base, &ra_value)) {
			value = ra_value;
			if(b_minus) {
				value = -value;
			}
			return true;
		}
	}
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::log(value, o->clnNumber());
	setApproximate();
	return true;
}
bool Number::log(long int num, long int den) {
	Number o(num, den);
	return log(&o);
}
bool Number::exp() {
	if(CALCULATOR->alwaysExact()) return false;
	value = cln::exp(value);
	setApproximate();
	return true;
}
bool Number::gcd(const Number *o) {
	if(isComplex() || o->isComplex()) {
		return false;
	}
	if(o->isApproximate()) setApproximate();
	if(isZero() || o->isZero()) {
		clear(); 
		return true;
	}
	cl_RA ra_value = cln::rational(cln::realpart(value));
	cl_I num = cln::numerator(ra_value);
	cl_I den = cln::denominator(ra_value);
	cl_RA ra_o = cln::rational(cln::realpart(o->clnNumber()));
	cl_I num_o = cln::numerator(ra_o);
	cl_I den_o = cln::denominator(ra_o);
	num *= den_o;
	den *= num_o;
	value = cln::gcd(num, den);
	return true;
}

bool Number::factorial() {
	if(!isInteger() || isNegative()) {
		return false;
	}
	if(isZero()) {
		set(1);
		return true;
	} else if(isOne()) {
		return true;
	} else if(isNegative()) {
		return false;
	}
	Number i(this);
	i.add(-1);
	for(; !i.isOne(); i.add(-1)) {
		multiply(&i);
	}
	return true;
}
bool Number::binomial(const Number *m, const Number *k) {
	if(!m->isInteger() || !k->isInteger()) return false;
	if(k->isNegative()) return false;
	if(m->isZero() || m->isNegative()) return false;
	if(k->isGreaterThan(m)) return false;
	clear();
	if(k->isZero()) {
		set(1);
	} else if(k->isOne()) {
		set(m);
	} else if(m->equals(k)) {
		set(1);
	} else {
		cl_I im = cln::numerator(cln::rational(cln::realpart(m->clnNumber())));
		cl_I ik = cln::numerator(cln::rational(cln::realpart(k->clnNumber())));
		if(m->isGreaterThan(INT_MAX) || k->isGreaterThan(INT_MAX)) {
			ik--;
			Number k_fac(k);
			k_fac.factorial();
			cl_I ithis = im;
			for(; !zerop(ik); ik--) {
				im--;
				ithis *= im;
			}
			setCln(ithis);
			divide(&k_fac);
		} else {
			value = cln::binomial(cl_I_to_uint(im), cl_I_to_uint(ik));
		}
	}
	return true;
}

int Number::add(MathOperation op, const Number *o, int solution) {
	if(!o) return 0;
	switch(op) {
		case OPERATION_SUBTRACT: {
			if(subtract(o)) return 1;
			return 0;
		}
		case OPERATION_ADD: {
			if(add(o)) return 1;
			return 0;
		} 
		case OPERATION_MULTIPLY: {
			if(multiply(o)) return 1;
			return 0;
		}
		case OPERATION_DIVIDE: {
			if(divide(o)) return 1;
			return 0;
		}		
		case OPERATION_RAISE: {
			return raise(o, solution);
		}
		case OPERATION_EXP10: {
			if(exp10(o)) return 1;
			return 0;
		}
		case OPERATION_OR: {
			setTrue(isPositive() || o->isPositive());
			return 1;
		}
		case OPERATION_AND: {
			setTrue(isPositive() && o->isPositive());
			return 1;
		}
		case OPERATION_EQUALS: {
			setTrue(equals(o));
			return 1;
		}
		case OPERATION_GREATER: {
			int i = compare(o);
			if(i != -2) setTrue(i == -1);
			if(i != -2) return 1;
			return 0;
		}
		case OPERATION_LESS: {
			int i = compare(o);
			if(i != -2) setTrue(i == 1);
			if(i != -2) return 1;
			return 0;
		}
		case OPERATION_EQUALS_GREATER: {
			int i = compare(o);
			if(i != -2) setTrue(i == 0 || i == -1);
			if(i != -2) return 1;
			return 0;
		}
		case OPERATION_EQUALS_LESS: {
			int i = compare(o);
			if(i != -2) setTrue(i == 0 || i == 1);
			if(i != -2) return 1;
			return 0;
		}
		case OPERATION_NOT_EQUALS: {
			setTrue(!equals(o));
			return 1;
		}
	}
	return 0;	
}

bool Number::floatify(cl_I *num, cl_I *den, int precision, int max_decimals, bool *infinite_series) {
	cl_I remainder = 0, remainder2 = 0;
	cl_I_div_t div;
	Number exp_test(this);
	cl_I d = cln::denominator(cln::rational(cln::realpart(value)));
	*num = cln::numerator(cln::rational(cln::realpart(value)));
	*den = 1;
	div = truncate2(*num, d);
	remainder = div.remainder;
	*num = div.quotient;
	bool exact = (remainder == 0);
	vector<cl_I> remainders;
	if(infinite_series) {
		*infinite_series = false;
	}
	cl_I exp;
	if((!exact && max_decimals) || isApproximate()) {
		bool b_always_exact = CALCULATOR->alwaysExact();
		CALCULATOR->setAlwaysExact(false);
		exp_test.setNegative(false);
		if(exp_test.isLessThan(1)) {
			exp_test.setCln(-cln::log(cln::recip(exp_test.clnNumber()), 10));
		} else {
			exp_test.setCln(cln::log(exp_test.clnNumber(), 10));
		}
		exp_test.floor();
		exp = cln::numerator(cln::rational(cln::realpart(exp_test.clnNumber())));
/*		exp_test.floor();
		Integer expdiv(exp_test.numerator());
		expdiv.div10();
		while(!expdiv.isZero()) {
			exp.add(1);
			expdiv.div10();
		}*/
		exp++;
		if(exp > precision) {
			exp -= precision;
			precision = 0;
			exp--;
			cl_I exp10 = cln::expt_pos(cl_I(10), exp);
			div = cln::truncate2(*num, exp10);
			*num = div.quotient;
			exact = (div.remainder == 0);
			div = cln::truncate2(*num, 10);
			remainder = div.remainder;
			*num = div.quotient;
			exact = (exact && remainder == 0);
			remainder *= 10;
			if(minusp(remainder)) {
				remainder = -remainder;
			}
			if(remainder >= 5) {
				if(minusp(*num)) {
					*num -= 1;
				} else {
					*num += 1;
				}
			}		
			exp++;	
			*num *= cln::expt_pos(cl_I(10), exp);	
			remainder = 0;
			return exact;
		} else {
			precision -= cl_I_to_int(exp);
			exp = 0;
		}
		CALCULATOR->setAlwaysExact(b_always_exact);
	}
	if(max_decimals >= 0 && max_decimals < precision) {
		precision = max_decimals;
	}
	while(!exact && precision) {
		if(infinite_series && !(*infinite_series)) {
			remainders.push_back(remainder);
		}
		remainder *= 10;
		div = cln::truncate2(remainder, d);
		remainder2 = div.remainder;
		remainder = div.quotient;
		exact = (remainder2 == 0);
		*num *= 10;	
		*num += remainder;
		*den *= 10;
		remainder = remainder2;
		if(!exact && infinite_series && !(*infinite_series)) {
			for(unsigned int i = 0; i < remainders.size(); i++) {
				if(remainders[i] == remainder) {
					*infinite_series = true;
					break;
				}
			}
		}
		precision--;
	}
	if(!exact && !(infinite_series && *infinite_series)) {
		remainder *= 10;
		div = cln::truncate2(remainder, d);
		remainder2 = div.remainder;
		remainder = div.quotient;
		if(minusp(remainder)) {
			remainder = -remainder;
		}
		if(remainder >= 5) {
			if(minusp(*num)) *num -= 1;
			else *num += 1;			
		}
	}
	return exact;	
}

string Number::print(NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Number *l_exp, bool in_composite, bool in_power) const {

	bool minus, exp_minus;
	string whole_, numerator_, denominator_, exponent_, prefix_;
	getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags, min_decimals, max_decimals, prefix, in_exact, usable, toplevel, plural, l_exp, in_composite, in_power);

	string str;
	if(minus) {
		if(displayflags & DISPLAY_FORMAT_NONASCII) {
			str += SIGN_MINUS;
		} else {
			str += "-";
		}			
	}
	str += whole_;
	if(!numerator_.empty()) {
		if(!whole_.empty()) {
			str += " ";
		}
		if(displayflags & DISPLAY_FORMAT_TAGS) {	
			str += "<sup>";
			str += numerator_;
			str += "</sup>";		
		} else {
			str += numerator_;
		}
		if(displayflags & DISPLAY_FORMAT_NONASCII) {
			str += SIGN_DIVISION;
		} else {
			str += "/";	
		}

		if(displayflags & DISPLAY_FORMAT_TAGS) {	
			str += "<small>";
 			str += denominator_;
			str += "</small>";		
		} else {
			str += denominator_;
		}	
		if(!exponent_.empty()) {
			str += " ";
			if(displayflags & DISPLAY_FORMAT_NONASCII) {
				str += SIGN_MULTIDOT;
			} else {
				str += "*";
			}
			str += " 1";				
		}		
	}
	if(!exponent_.empty()) {
		if(displayflags & DISPLAY_FORMAT_TAGS) {	
			str += "<small>";
		 	str += "E";
			str += "</small>";		
		} else {
			str += "E";
		}	
		if(exp_minus) {
			if(displayflags & DISPLAY_FORMAT_NONASCII) {
				str += SIGN_MINUS;
			} else {
				str += "-";
			}					
		}
		str += exponent_;		
	}
	if(!prefix_.empty()) {
		if(!str.empty()) {
			str += " ";
		}
		str += prefix_;
	}
	if(str.empty()) {
		str = "0";
	}
	return str;
}

void Number::getPrintObjects(bool &minus, string &whole_, string &numerator_, string &denominator_, bool &exp_minus, string &exponent_, string &prefix_, NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Number *l_exp, bool in_composite, bool in_power, Number *l_exp2, Prefix **prefix1, Prefix **prefix2) const {
	if(CALCULATOR->alwaysExact()) max_decimals = -1;
	if(in_exact && isApproximate()) *in_exact = true;
	cl_I exp = 0;
	cl_I num = cln::numerator(cln::rational(cln::realpart(value)));
	cl_I den = cln::denominator(cln::rational(cln::realpart(value)));
	if(!isZero() && nrformat != NUMBER_FORMAT_SEXAGESIMAL && nrformat != NUMBER_FORMAT_TIME) {
		if(!isZero()) {
			if((!(displayflags & DISPLAY_FORMAT_FRACTION) && !(displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY))) {		
				Number exp_pre(this);
				exp_pre.setNegative(false);
				bool b_always_exact = CALCULATOR->alwaysExact();
				CALCULATOR->setAlwaysExact(false);	
				if(exp_pre.isLessThan(1)) {
					exp_pre.setCln(-cln::log(cln::recip(exp_pre.clnNumber()), 10));
				} else if(exp_pre.isOne()) {
					exp_pre.clear();
				} else {
					exp_pre.setCln(cln::log(exp_pre.clnNumber(), 10));
				}
				exp_pre.floor();
				exp = cln::numerator(cln::rational(cln::realpart(exp_pre.clnNumber())));
				CALCULATOR->setAlwaysExact(b_always_exact);
			} else {
				if(cln::rem(num, 10) == 0) {
					cl_I num_test = num;				
					while(1) {	
						cl_I_div_t div = cln::truncate2(num_test, 10);
						num_test = div.quotient;
						if(div.remainder != 0) {
							break;
						}
						exp++;		
					}			
				} else {	
					cl_I den_test = den;
					while(1) {	
						cl_I_div_t div = cln::truncate2(den_test, 10);
						den_test = div.quotient;
						if(div.remainder != 0) {
							break;
						}
						exp--;		
					}
				}
			}
		}
	}
	cl_I exp_spec = exp;
	minus = isNegative();
	whole_ = "";
	numerator_ = "";
	denominator_ = "";
	exponent_ = "";
	exp_minus = false;
	prefix_ = "";	
	bool force_rational = false;
	int base = 10;
	if(displayflags & DISPLAY_FORMAT_USE_PREFIXES && nrformat != NUMBER_FORMAT_SEXAGESIMAL && nrformat != NUMBER_FORMAT_TIME) {
		Prefix *p;
		if(l_exp) {
			Number tmp_exp;
			if(prefix) {
				prefix->exponent(l_exp, &tmp_exp);
				if(tmp_exp.isInteger()) {
					exp_spec -= cln::numerator(cln::rational(cln::realpart(tmp_exp.clnNumber())));
					prefix_ = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
					if(prefix1) *prefix1 = prefix;
				}
			} else {
				tmp_exp.setCln(exp);
				if(l_exp2 && prefix2 && CALCULATOR->denominatorPrefixEnabled()) {	
					tmp_exp.trunc(l_exp);
					tmp_exp.setNegative(false);
					if(tmp_exp.isGreaterThan(5)) {
						tmp_exp.setCln(exp);
						Number nr(2, 1);
						tmp_exp.trunc(&nr);
						if(!oddp(cln::numerator(cln::rational(cln::realpart(tmp_exp.clnNumber()))))) {
							if(tmp_exp.isNegative()) tmp_exp.add(-1);
							else tmp_exp.add(1);
						}
					} else {
						tmp_exp.setCln(exp);
					}
				}
				p = CALCULATOR->getBestPrefix(&tmp_exp, l_exp);
				Number test_exp;
				test_exp.setCln(exp);
				test_exp.subtract(p->exponent(l_exp, &tmp_exp));
				if(test_exp.isInteger()) {
					Number nr_exp;
					nr_exp.setCln(exp);
					if((cln::plusp(exp) && nr_exp.compare(&test_exp) == -1) || (cln::minusp(exp) && nr_exp.compare(&test_exp) == 1)) {
						exp_spec = cln::numerator(cln::rational(cln::realpart(test_exp.clnNumber())));
						prefix_ = p->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
						if(prefix1) *prefix1 = p;
					}
				}
			}
			if((displayflags & DISPLAY_FORMAT_SHORT_UNITS) && (displayflags & DISPLAY_FORMAT_NONASCII)) {
				gsub("micro", SIGN_MICRO, prefix_);
			}
			if(l_exp2 && prefix2 && CALCULATOR->denominatorPrefixEnabled()) {
				l_exp2->setNegative(!l_exp2->isNegative());
				Number nr_exp_spec;
				nr_exp_spec.setCln(exp_spec);
				p = CALCULATOR->getBestPrefix(&nr_exp_spec, l_exp2);
				Number test_exp(&nr_exp_spec);
				test_exp.subtract(p->exponent(l_exp2, &tmp_exp));
				if(test_exp.isInteger()) {
					if((nr_exp_spec.isPositive() && nr_exp_spec.compare(&test_exp) == -1) || (nr_exp_spec.isNegative() && nr_exp_spec.compare(&test_exp) == 1)) {
						exp_spec = cln::numerator(cln::rational(cln::realpart(test_exp.clnNumber())));
						if(prefix2) *prefix2 = p;
					}
				}
			}
		} else if(l_exp2) {
			Number tmp_exp;
			l_exp2->setNegative(true);
			if(prefix) {
				prefix->exponent(l_exp2, &tmp_exp);
				if(tmp_exp.isInteger()) {
					exp_spec -= cln::numerator(cln::rational(cln::realpart(tmp_exp.clnNumber())));
					prefix_ = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
					if(prefix2) *prefix2 = prefix;
				}
			} else {
				Number nr_exp;
				nr_exp.setCln(exp);
				p = CALCULATOR->getBestPrefix(&nr_exp, l_exp2);
				Number test_exp(&nr_exp);
				test_exp.subtract(p->exponent(l_exp2, &tmp_exp));
				if(test_exp.isInteger()) {
					if((cln::plusp(exp) && nr_exp.compare(&test_exp) == -1) || (cln::minusp(exp) && nr_exp.compare(&test_exp) == 1)) {
						exp_spec = cln::numerator(cln::rational(cln::realpart(test_exp.clnNumber())));
						prefix_ = p->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
						if(prefix2) *prefix2 = p;
					}
				}
			}
			if((displayflags & DISPLAY_FORMAT_SHORT_UNITS) && (displayflags & DISPLAY_FORMAT_NONASCII)) {
				gsub("micro", SIGN_MICRO, prefix_);
			}
		}
	}
	switch(nrformat) {
		case NUMBER_FORMAT_DECIMALS: {
			break;
		}		
		case NUMBER_FORMAT_HEX: {
			base = 16;
			force_rational = true;
			break;			
		}
		case NUMBER_FORMAT_OCTAL: {
			base = 8;
			force_rational = true;
			break;			
		}
		case NUMBER_FORMAT_BIN: {
			base = 2;
			force_rational = true;
			break;
		}
		case NUMBER_FORMAT_ROMAN: {
			base = BASE_ROMAN_NUMERALS;
			force_rational = true;
			break;
		}					
		case NUMBER_FORMAT_NORMAL: {
			if(exp_spec > -PRECISION && exp_spec < PRECISION) { 
				break;
			}
		}			
		case NUMBER_FORMAT_EXP: {
			if(exp_spec > -3 && exp_spec < 3) { 
				break;
			}		
		}
		case NUMBER_FORMAT_EXP_PURE: {
			if(!cln::zerop(exp_spec)) {
				exponent_ = printCL_I(exp_spec, 10, false);
				exp_spec = 0;
			}
			break;
		}
		default: {
			break;
		}		
	}
	cl_I den_spec = den;	
	cl_I whole = num;
	exp -= exp_spec;
	if(cln::minusp(exp)) {
		exp_minus = true;
		exp = -exp;
		whole *= expt_pos(cl_I(10), exp);
	} else if(cln::plusp(exp)) {
		den_spec *= expt_pos(cl_I(10), exp);
	}
	if(plural) {
		*plural = whole > den_spec;
	}
	if(in_composite && exponent_.empty() && whole == den_spec) {
		return;
	} else if(nrformat == NUMBER_FORMAT_SEXAGESIMAL || nrformat == NUMBER_FORMAT_TIME) {
		Number nr(this);
		nr.trunc();
		whole_ = nr.printNumerator(10, false);
		if(nrformat == NUMBER_FORMAT_SEXAGESIMAL) {
			if(displayflags & DISPLAY_FORMAT_NONASCII) {
				whole_ += SIGN_POWER_0;
			} else {
				whole_ += "o";
			}	
		}
		nr.set(this);
		nr.frac();
		nr.multiply(60);
		Number nr2(&nr);
		nr.trunc();
		if(nrformat == NUMBER_FORMAT_TIME) {
			whole_ += ":";
			if(nr.isLessThan(10)) {
				whole_ += "0";
			}
		}
		whole_ += nr.printNumerator(10, false);
		if(nrformat == NUMBER_FORMAT_SEXAGESIMAL) {
			whole_ += "'";
		}	
		nr2.frac();
		if(!nr2.isZero() || nrformat == NUMBER_FORMAT_SEXAGESIMAL) {
			nr2.multiply(60);
			nr.set(&nr2);
			nr.trunc();
			nr2.frac();
			if(!nr2.isZero()) {
				if(in_exact) *in_exact = true;
				if(nr2.isGreaterThanOrEqualTo(1, 2)) {
					nr.add(1);
				}
			}
			if(nrformat == NUMBER_FORMAT_TIME) {
				whole_ += ":";
				if(nr.isLessThan(10)) {
					whole_ += "0";
				}
			}
			whole_ += nr.printNumerator(10, false);
			if(nrformat == NUMBER_FORMAT_SEXAGESIMAL) {
				whole_ += "\"";
			}
		}
	} else if(!force_rational && !(displayflags & DISPLAY_FORMAT_FRACTION) && !(displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY)) {
		Number nr;
		nr.setCln(whole / den_spec);
		bool infinite_series = false;
		cl_I nr_num, nr_den;
		bool b_exact;
		if(displayflags & DISPLAY_FORMAT_INDICATE_INFINITE_SERIES) {
			b_exact = nr.floatify(&nr_num, &nr_den, PRECISION, max_decimals, &infinite_series);
		} else {
			b_exact = nr.floatify(&nr_num, &nr_den, PRECISION, max_decimals);
		}
		if(!b_exact) {
			if(!isApproximate() && ((displayflags & DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT) || CALCULATOR->alwaysExact())) {
				displayflags = displayflags | DISPLAY_FORMAT_FRACTIONAL_ONLY;
				return getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags, min_decimals, max_decimals, prefix, in_exact, usable, toplevel, plural, l_exp, in_composite, in_power);
			}
		}
		if(in_exact && !b_exact && !infinite_series) {
			*in_exact = true;
		}
		whole_ = printCL_I(nr_num, 10, false);
		int l10 = 0;
		cl_I d = nr_den;
		while(1) {	
			cl_I_div_t div = cln::truncate2(d, 10);
			d = div.quotient;
			if(div.remainder != 0) {
				break;
			}
			l10++;		
		}
/*		Fraction d(fr.denominator());
		d.log10();
		l10 = d.numerator()->getInt();*/
		if(l10) {
			l10 = whole_.length() - l10;
			for(; l10 < 1; l10++) {
				whole_.insert(whole_.begin(), 1, '0');
			}
			whole_.insert(l10, CALCULATOR->getDecimalPoint());
		}
		if(min_decimals > 0) {
			int index = whole_.find(CALCULATOR->getDecimalPoint());
			if(index == (int) string::npos) {
				whole_ += CALCULATOR->getDecimalPoint();
				for(int i = 0; i < min_decimals; i++) {
					whole_ += '0';
				}
			} else {
				index += strlen(CALCULATOR->getDecimalPoint());
				index = whole_.length() - index;
				index = min_decimals - index;
				for(int i = 0; i < index; i++) {
					whole_ += '0';
				}				
			}
		}
		if(infinite_series) {
			whole_ += "...";
		}
	} else {
		cl_I whole_test = whole;
		cl_I_div_t div = cln::truncate2(whole_test, den_spec);
		cl_I part = div.remainder;
		whole_test = div.quotient;
		bool b = part == 0;
		if((displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY) && !b) {
			part = whole;
			whole = 0;
		} else {
			whole = whole_test;
		}
		cl_I divisor = cln::gcd(part, den_spec);
		if(divisor > 1) {
			part = cln::truncate1(part, divisor);
			den_spec = cln::truncate1(den_spec, divisor);
		}
		if(!cln::zerop(whole)) {
			whole_ = printCL_I(whole, base, false);
		}
		if(!cln::zerop(part)) {
			numerator_ = printCL_I(part, base, false);
			denominator_ = printCL_I(den_spec, base, false);
		}	
	}
	if(whole_.empty() && numerator_.empty() && denominator_.empty() && prefix_.empty()) {
		if(exponent_.empty()) {
			whole_ = "0";
		} else {
			whole_ = "1";
		}
	}
}

string Number::printNumerator(int base, bool display_sign) const {
	return printCL_I(cln::numerator(cln::rational(cln::realpart(value))), base, display_sign);
}
string Number::printDenominator(int base, bool display_sign) const {
	return printCL_I(cln::denominator(cln::rational(cln::realpart(value))), base, display_sign);
}
string Number::printImaginaryNumerator(int base, bool display_sign) const {
	return printCL_I(cln::numerator(cln::rational(cln::imagpart(value))), base, display_sign);
}
string Number::printImaginaryDenominator(int base, bool display_sign) const {
	return printCL_I(cln::denominator(cln::rational(cln::imagpart(value))), base, display_sign);
}
