/*
    Qalculate    

    Copyright (C) 2004  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Number.h"

#include <sstream>
#include "util.h"

#define REAL_PRECISION_FLOAT_RE(x)		cln::cl_float(cln::realpart(x), cln::float_format(PRECISION + 1))
#define REAL_PRECISION_FLOAT_IM(x)		cln::cl_float(cln::imagpart(x), cln::float_format(PRECISION + 1))
#define REAL_PRECISION_FLOAT(x)			cln::cl_float(x, cln::float_format(PRECISION + 1))

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
Number::Number(string number, int base) {
	set(number, base);
}
Number::Number(int numerator, int denominator, int exp_10) {
	set(numerator, denominator, exp_10);
}
Number::Number(const Number &o) {
	set(o);
}
Number::~Number() {
}

void Number::set(string number, int base) {

	b_inf = false; b_pinf = false; b_minf = false;

	if(base == BASE_ROMAN_NUMERALS) {
		remove_blanks(number);
		Number nr;
		Number cur;
		bool large = false;
		vector<Number> numbers;
		bool capital = false;
		for(unsigned int i = 0; i < number.length(); i++) {
			switch(number[i]) {
				case 'I': {
					if(!capital && i == number.length() - 1) {
						cur.set(2);
						CALCULATOR->error(false, _("Assuming the unusual practice of letting a last capital I mean 2 in a roman numeral."), NULL);
						break;
					}
				}
				case 'J': {capital = true;}
				case 'i': {}
				case 'j': {
					cur.set(1);
					break;
				}
				case 'V': {capital = true;}
				case 'v': {
					cur.set(5);
					break;
				}
				case 'X': {capital = true;}
				case 'x': {
					cur.set(10);
					break;
				}
				case 'L': {capital = true;}
				case 'l': {
					cur.set(50);
					break;
				}
				case 'C': {capital = true;}
				case 'c': {
					cur.set(100);
					break;
				}
				case 'D': {capital = true;}
				case 'd': {
					cur.set(500);
					break;
				}
				case 'M': {capital = true;}
				case 'm': {
					cur.set(1000);
					break;
				}
				case '(': {
					int multi = 1, multi2;
					bool turn = false;
					bool error = false;
					i++;
					for(; i < number.length(); i++) {
						if(number[i] == '|') {
							if(!turn) {
								turn = true;
								multi2 = multi;
							} else {
								error = true;
								break;
							}
						} else if(number[i] == ')') {
							if(turn) {
								multi2--;
								if(multi2 < 1) {
									break;
								}	
							} else {
								error = true;
								break;
							}
						} else if(number[i] == '(') {
							if(!turn) {
								multi++;	
							} else {
								error = true;
								break;
							}
						} else {
							error = true;
							i--;
							break;
						}
					}
					if(error | !turn) {
						CALCULATOR->error(true, _("Error in roman numerals: %s."), number.c_str(), NULL);
					} else {
						cur.set(10);
						cur.raise(multi);
						cur.multiply(100);
					}
					break;
				}
				case '|': {
					if(large) {
						cur.clear();
						large = false;
						break;
					} else if(number.length() > i + 1 && number[i + 2] == ')') {
						i++;
						int multi = 1;
						for(; i < number.length(); i++) {
							if(number[i] != ')') {
								i--;
								break;
							}
							multi++;
						}
						cur.set(10);
						cur.raise(multi);
						cur.multiply(50);
						break;
					} else if(number.length() > i + 2 && number[i + 2] == '|') {
						cur.clear();
						large = true;
						break;
					}
				}
				default: {
					cur.clear();
					CALCULATOR->error(true, _("Unknown roman numeral: %s."), number.substr(i, 1).c_str(), NULL);
				}
			}
			if(!cur.isZero()) {
				if(large) {
					cur.multiply(100000);
				}
				numbers.resize(numbers.size() + 1);
				numbers[numbers.size() - 1].set(cur);
			}
		}
		vector<Number> values;
		values.resize(numbers.size());
		bool error = false;
		int rep = 1;
		for(unsigned int i = 0; i < numbers.size(); i++) {
			if(i == 0 || numbers[i].isLessThanOrEqualTo(numbers[i - 1])) {
				nr.add(numbers[i]);
				if(i > 0 && numbers[i].equals(numbers[i - 1])) {
					rep++;
					if(rep > 3 && numbers[i].isLessThan(1000)) {
						error = true;
					} else if(rep > 1 && (numbers[i].equals(5) || numbers[i].equals(50) || numbers[i].equals(500))) {
						error = true;
					}
				} else {
					rep = 1;
				}
			} else {	
				numbers[i - 1].multiply(10);
				if(numbers[i - 1].isLessThan(numbers[i])) {
					error = true;
				}
				numbers[i - 1].divide(10);
				for(int i2 = i - 2; ; i2--) {
					if(i2 < 0) {
						nr.negate();
						nr.add(numbers[i]);
						break;
					} else if(numbers[i2].isGreaterThan(numbers[i2 + 1])) {
						Number nr2(nr);
						nr2.subtract(values[i2]);
						nr.subtract(nr2);
						nr.subtract(nr2);
						nr.add(numbers[i]);
						if(numbers[i2].isLessThan(numbers[i])) {
							error = true;
						}
						break;
					}
					error = true;
				}
			}
			values[i].set(nr);
		}
		if(error) {
			PrintOptions po;
			po.base = BASE_ROMAN_NUMERALS;
			CALCULATOR->error(false, _("Errors in roman numerals: \"%s\". Interpreted as %s, which should be written as %s."), number.c_str(), nr.print().c_str(), nr.print(po).c_str(), NULL);
		}
		values.clear();
		numbers.clear();
		set(nr);
		return;
	}
	cl_I num = 0;
	cl_I den = 1;
	remove_blank_ends(number);
	if(base == 16 && number.length() >= 2 && number[0] == '0' && (number[1] == 'x' || number[1] == 'X')) {
		number = number.substr(2, number.length() - 2);
	} else if(base == 8 && number.length() > 1 && number[0] == '0' && number[1] != '.') {
		number.erase(number.begin());
	}
	if(base > 36) base = 36;
	if(base < 0) base = 10;
	bool numbers_started = false, minus = false, in_decimals = false, b_cplx = false;
	for(unsigned int index = 0; index < number.size(); index++) {
		if(number[index] >= '0' && (base >= 10 && number[index] <= '9') || (base < 10 && number[index] < '0' + base)) {
			num = num * base;
			num = num + number[index] - '0';
			if(in_decimals) {
				den = den * base;
			}
			numbers_started = true;
		} else if(base > 10 && number[index] >= 'a' && number[index] <= 'z') {
			num = num * base;
			num = num + number[index] - 'a';
			if(in_decimals) {
				den = den * base;
			}
			numbers_started = true;
		} else if(base > 10 && number[index] >= 'A' && number[index] <= 'Z') {
			num = num * base;
			num = num + number[index] - 'A';
			if(in_decimals) {
				den = den * base;
			}
			numbers_started = true;
		} else if(number[index] == 'E' && base == 10) {
			index++;
			numbers_started = false;
			bool exp_minus = false;
			cl_I exp;
			while(index < number.size()) {
				if(number[index] >= '0' && number[index] <= '9') {				
					exp = exp * 10;
					exp = exp + number[index] - '0';
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
		} else if(number[index] == ':' && base == 10) {
			if(in_decimals) {
				CALCULATOR->error(true, _("\':\' in decimal number ignored (decimal point detected)."), NULL);
			} else {
				vector<cl_I> nums;
				nums.push_back(num);
				num = 0;
				for(index++; index < number.size(); index++) {
					if(number[index] >= '0' && number[index] <= '9') {				
						num = num * 10;
						num = num + number[index] - '0';
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
					den = den * 60;
					nums[i] = nums[i] * den;
					num = num + nums[i];
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
}
void Number::set(int numerator, int denominator, int exp_10) {
	b_inf = false; b_pinf = false; b_minf = false;
	value = numerator;
	if(denominator) {
		value = value / denominator;
	}
	if(exp_10 != 0) {
		exp10(exp_10);
	}	
}
void Number::setFloat(double d_value) {
	b_inf = false; b_pinf = false; b_minf = false;
	value = d_value;
}
void Number::setInternal(const cl_N &cln_value) {
	b_inf = false; b_pinf = false; b_minf = false;
	value = cln_value;
}
void Number::setImaginaryPart(const Number &o) {
	value = cln::complex(cln::realpart(value), cln::realpart(o.internalNumber()));
}
void Number::setImaginaryPart(int numerator, int denominator, int exp_10) {
	Number o(numerator, denominator, exp_10);
	setImaginaryPart(o);
}
void Number::set(const Number &o) {
	b_inf = o.isInfinity(); 
	b_pinf = o.isPlusInfinity(); 
	b_minf = o.isMinusInfinity();
	value = o.internalNumber();
}
void Number::setInfinity() {
	b_inf = true;
	b_pinf = false;
	b_minf = false;
	value = 0;
}
void Number::setPlusInfinity() {
	b_inf = false;
	b_pinf = true;
	b_minf = false;
	value = 0;
}
void Number::setMinusInfinity() {
	b_inf = false; 
	b_pinf = false;
	b_minf = true;
	value = 0;
}
void Number::clear() {
	b_inf = false; b_pinf = false; b_minf = false;
	value = 0;
}

const cl_N &Number::internalNumber() const {
	return value;
}

double Number::floatValue() const {
	return double_approx(cln::realpart(value));
}
int Number::intValue(bool *overflow) const {
	cl_I i = cln::round1(cln::realpart(value));
	if(i > long(INT_MAX)) {
		if(overflow) *overflow = true;
		return INT_MAX;
	} else if(i < long(INT_MIN)) {
		if(overflow) *overflow = true;
		return INT_MIN;
	}
	return cl_I_to_int(i);
}

bool Number::isApproximate() const {
	return !isInfinite() && (!cln::instanceof(cln::realpart(value), cln::cl_RA_ring) || (isComplex() && !cln::instanceof(cln::imagpart(value), cln::cl_RA_ring)));	
}
void Number::setApproximate(bool is_approximate) {
	if(!isInfinite() && is_approximate != isApproximate()) {
		if(is_approximate) {
			value = cln::complex(cln::cl_float(cln::realpart(value)), cln::cl_float(cln::imagpart(value)));
			removeFloatZeroPart();
		} else {
			value = cln::complex(cln::rational(cln::realpart(value)), cln::rational(cln::imagpart(value)));
		}
	}
}
bool Number::isUndefined() const {
	return false;
}
bool Number::isInfinite() const {
	return b_pinf || b_minf || b_inf;
}
bool Number::isInfinity() const {
	return b_inf;
}
bool Number::isPlusInfinity() const {
	return b_pinf;
}
bool Number::isMinusInfinity() const {
	return b_minf;
}

Number Number::realPart() const {
	if(isInfinite()) return *this;
	Number real_part;
	real_part.setInternal(cln::realpart(value));
	return real_part;
}
Number Number::imaginaryPart() const {
	if(isInfinite()) return Number();
	Number imag_part;
	imag_part.setInternal(cln::imagpart(value));
	return imag_part;
}
Number Number::numerator() const {
	Number num;
	num.setInternal(cln::numerator(cln::rational(cln::realpart(value))));
	return num;
}
Number Number::denominator() const {
	Number den;
	den.setInternal(cln::denominator(cln::rational(cln::realpart(value))));
	return den;
}
Number Number::complexNumerator() const {
	Number num;
	num.setInternal(cln::numerator(cln::rational(cln::imagpart(value))));
	return num;
}
Number Number::complexDenominator() const {
	Number den;
	den.setInternal(cln::denominator(cln::rational(cln::imagpart(value))));
	return den;
}

void Number::operator = (const Number &o) {set(o);}
Number Number::operator - () const {Number o(*this); o.negate(); return o;}
Number Number::operator * (const Number &o) const {Number o2(*this); o2.multiply(o); return o2;}
Number Number::operator / (const Number &o) const {Number o2(*this); o2.divide(o); return o2;}
Number Number::operator + (const Number &o) const {Number o2(*this); o2.add(o); return o2;}
Number Number::operator - (const Number &o) const {Number o2(*this); o2.subtract(o); return o2;}
Number Number::operator ^ (const Number &o) const {Number o2(*this); o2.raise(o); return o2;}
Number Number::operator && (const Number &o) const {Number o2(*this); o2.add(o, OPERATION_AND); return o2;}
Number Number::operator || (const Number &o) const {Number o2(*this); o2.add(o, OPERATION_OR); return o2;}
Number Number::operator ! () const {Number o(*this); o.setNOT(); return o;}
		
void Number::operator *= (const Number &o) {multiply(o);}
void Number::operator /= (const Number &o) {divide(o);}
void Number::operator += (const Number &o) {add(o);}
void Number::operator -= (const Number &o) {subtract(o);}
void Number::operator ^= (const Number &o) {raise(o);}
	
bool Number::operator == (const Number &o) const {return equals(o);}
bool Number::operator != (const Number &o) const {return !equals(o);}

bool Number::hasRealPart() const {
	return isInfinite() || !cln::zerop(cln::realpart(value));
}
bool Number::hasImaginaryPart() const {
	return !isInfinite() && !cln::zerop(cln::imagpart(value));
}
void Number::removeFloatZeroPart() {
	if(!isInfinite() && isApproximate() && !cln::zerop(cln::imagpart(value))) {
		cl_F f_value = REAL_PRECISION_FLOAT_RE(value) + REAL_PRECISION_FLOAT_IM(value);
		if(REAL_PRECISION_FLOAT(f_value) == REAL_PRECISION_FLOAT_RE(value)) {
			value = cln::realpart(value);
		} else if(REAL_PRECISION_FLOAT(f_value) == REAL_PRECISION_FLOAT_IM(value)) {
			value = cln::complex(0, cln::imagpart(value));
		}
	}
}
bool Number::isComplex() const {
	return !isInfinite() && !cln::zerop(cln::imagpart(value));
}
bool Number::isInteger() const {
	if(isInfinite()) return false;
	if(isComplex()) return false;
	if(isApproximate()) {
		return cln::zerop(cln::truncate2(REAL_PRECISION_FLOAT_RE(value)).remainder);
	}
	return cln::zerop(cln::truncate2(cln::realpart(value)).remainder);
}
bool Number::isRational() const {
	return !isInfinite() && !isComplex() && !isApproximate();
}
bool Number::isReal() const {
	return !isInfinite() && !isComplex();
}
bool Number::isFraction() const {
	if(isInfinite()) return false;
	if(!isComplex()) {
		cl_R real_value = cln::realpart(value);
		return real_value < 1 && real_value > -1;
	}
	return false; 
}
bool Number::isZero() const {
	if(isInfinite()) return false;
	return cln::zerop(value);
}
bool Number::isOne() const {
	if(isInfinite()) return false;
	return value == 1;
}
bool Number::isI() const {
	if(isInfinite()) return false;
	return cln::zerop(cln::realpart(value)) && cln::imagpart(value) == 1;
}
bool Number::isMinusOne() const {
	if(isInfinite()) return false;
	return value == -1;
}
bool Number::isMinusI() const {
	if(isInfinite()) return false;
	return cln::zerop(cln::realpart(value)) && cln::imagpart(value) == -1;
}
bool Number::isNegative() const {
	return b_minf || (!isInfinite() && !isComplex() && cln::minusp(cln::realpart(value)));
}
bool Number::isNonNegative() const {
	return b_pinf || (!isInfinite() && !isComplex() && !cln::minusp(cln::realpart(value)));
}
bool Number::isPositive() const {
	return b_pinf || (!isInfinite() && !isComplex() && cln::plusp(cln::realpart(value)));
}
bool Number::realPartIsNegative() const {
	return b_minf || (!isInfinite() && cln::minusp(cln::realpart(value)));
}
bool Number::realPartIsPositive() const {
	return b_pinf || (!isInfinite() && cln::plusp(cln::realpart(value)));
}
bool Number::imaginaryPartIsNegative() const {
	return !isInfinite() && cln::minusp(cln::imagpart(value));
}
bool Number::imaginaryPartIsPositive() const {
	return !isInfinite() && cln::plusp(cln::imagpart(value));
}
bool Number::hasNegativeSign() const {
	if(hasRealPart()) return realPartIsNegative();
	return imaginaryPartIsNegative();
}
bool Number::hasPositiveSign() const {
	if(hasRealPart()) return realPartIsPositive();
	return imaginaryPartIsPositive();
}
bool Number::equals(const Number &o) const {
	if(b_inf) return false;
	if(b_pinf) return o.isPlusInfinity();
	if(b_minf) return o.isMinusInfinity();
	if(o.isInfinite()) return false;
	if(isApproximate() || o.isApproximate()) {
		if(!isComplex() && !o.isComplex()) {
			return REAL_PRECISION_FLOAT_RE(value) == REAL_PRECISION_FLOAT_RE(o.internalNumber());
		} else if(isComplex() && o.isComplex()) {
			if(REAL_PRECISION_FLOAT_RE(value) != REAL_PRECISION_FLOAT_RE(o.internalNumber())) return false;
			return REAL_PRECISION_FLOAT_IM(value) == REAL_PRECISION_FLOAT_IM(o.internalNumber());
		} else {
			return false;
		}
	}
	return value == o.internalNumber();
}
int Number::compare(const Number &o) const {
	if(b_inf || o.isInfinity()) return -2;
	if(b_pinf) {
		if(o.isPlusInfinity()) return 0;
		else return -1;
	}
	if(b_minf) {
		if(o.isMinusInfinity()) return 0;
		else return 1;
	}
	if(o.isPlusInfinity()) return 1;
	if(o.isMinusInfinity()) return -1;
	if(!isComplex() && !o.isComplex()) {
		if(isApproximate() || o.isApproximate()) {
			return cln::compare(REAL_PRECISION_FLOAT_RE(o.internalNumber()), REAL_PRECISION_FLOAT_RE(value));
		}
		return cln::compare(cln::realpart(o.internalNumber()), cln::realpart(value));
	} else {
		if(equals(o)) return 0;
		return -2;
	}
}
int Number::compareImaginaryParts(const Number &o) const {
	return cln::compare(cln::imagpart(o.internalNumber()), cln::imagpart(value));
}
int Number::compareRealParts(const Number &o) const {
	return cln::compare(cln::realpart(o.internalNumber()), cln::realpart(value));
}
bool Number::isGreaterThan(const Number &o) const {
	if(b_minf || b_inf || o.isInfinity() || o.isPlusInfinity()) return false;
	if(o.isMinusInfinity()) return true;
	if(b_pinf) return true;
	if(isComplex() || o.isComplex()) return false;
	if(isApproximate() || o.isApproximate()) {
		return REAL_PRECISION_FLOAT_RE(value) > REAL_PRECISION_FLOAT_RE(o.internalNumber());
	}
	return cln::realpart(value) > cln::realpart(o.internalNumber());
}
bool Number::isLessThan(const Number &o) const {
	if(o.isMinusInfinity() || o.isInfinity() || b_inf || b_pinf) return false;
	if(b_minf || o.isPlusInfinity()) return true;
	if(isComplex() || o.isComplex()) return false;
	if(isApproximate() || o.isApproximate()) {
		return REAL_PRECISION_FLOAT_RE(value) < REAL_PRECISION_FLOAT_RE(o.internalNumber());
	}
	return cln::realpart(value) < cln::realpart(o.internalNumber());
}
bool Number::isGreaterThanOrEqualTo(const Number &o) const {
	if(b_inf || o.isInfinity()) return false;
	if(b_minf) return o.isMinusInfinity();
	if(b_pinf) return true;
	if(!isComplex() && !o.isComplex()) {
		if(isApproximate() || o.isApproximate()) {
			return REAL_PRECISION_FLOAT_RE(value) >= REAL_PRECISION_FLOAT_RE(o.internalNumber());
		}
		return cln::realpart(value) >= cln::realpart(o.internalNumber());
	} else {
		return equals(o);
	}
}
bool Number::isLessThanOrEqualTo(const Number &o) const {
	if(b_inf || o.isInfinity()) return false;
	if(b_pinf) return o.isPlusInfinity();
	if(b_minf) return true;
	if(!isComplex() && !o.isComplex()) {
		if(isApproximate() || o.isApproximate()) {
			return REAL_PRECISION_FLOAT_RE(value) <= REAL_PRECISION_FLOAT_RE(o.internalNumber());
		}
		return cln::realpart(value) <= cln::realpart(o.internalNumber());
	} else {
		return equals(o);
	}
}
bool Number::isEven() const {
	return isInteger() && evenp(cln::numerator(cln::rational(cln::realpart(value))));
}
bool Number::denominatorIsEven() const {
	return !isInfinite() && !isComplex() && evenp(cln::denominator(cln::rational(cln::realpart(value))));
}
bool Number::numeratorIsEven() const {
	return !isInfinite() && !isComplex() && evenp(cln::numerator(cln::rational(cln::realpart(value))));
}
bool Number::isOdd() const {
	return isInteger() && oddp(cln::numerator(cln::rational(cln::realpart(value))));
}

bool Number::add(const Number &o) {
	if(b_inf) return !o.isInfinite();
	if(o.isInfinity()) {
		if(isInfinite()) return false;
		setInfinity();
		return true;
	}
	if(b_minf) return !o.isPlusInfinity();
	if(b_pinf) return !o.isMinusInfinity();
	if(o.isInfinity()) {
		b_pinf = true;
		value = 0;
		return true;
	}
	if(o.isMinusInfinity()) {
		b_minf = true;
		value = 0;
		return true;
	}
	value = value + o.internalNumber();
	removeFloatZeroPart();
	return true;
}
bool Number::subtract(const Number &o) {
	if(b_inf) return !o.isInfinite();
	if(o.isInfinity()) {
		if(isInfinite()) return false;
		setInfinity();
		return true;
	}
	if(b_pinf) return !o.isPlusInfinity();
	if(b_minf) return !o.isMinusInfinity();
	if(o.isPlusInfinity()) {
		setPlusInfinity();
		return true;
	}
	if(o.isMinusInfinity()) {
		setMinusInfinity();
		return true;
	}
	value = value - o.internalNumber();
	removeFloatZeroPart();
	return true;
}
bool Number::multiply(const Number &o) {
	if(o.isInfinite() && isZero()) return false;
	if(isInfinite() && o.isZero()) return false;
	if((isInfinite() && o.isComplex()) || (o.isInfinite() && isComplex())) {
		setInfinity();
		return true;
	}
	if(isInfinity()) return true;
	if(o.isInfinity()) {
		setInfinity();
		return true;
	}
	if(b_pinf || b_minf) {
		if(o.isNegative()) {
			b_pinf = !b_pinf;
			b_minf = !b_minf;
		}
		return true;
	}
	if(o.isPlusInfinity()) {
		if(isNegative()) setMinusInfinity();
		else setPlusInfinity();
		return true;
	}
	if(o.isMinusInfinity()) {
		if(isNegative()) setPlusInfinity();
		else setMinusInfinity();
		return true;
	}
	if(isZero()) return true;
	if(!o.isApproximate() && o.isZero()) {
		clear();
		return true;
	}
	value = value * o.internalNumber();
	removeFloatZeroPart();
	return true;
}
bool Number::divide(const Number &o) {
	if(isInfinite() && o.isInfinite()) return false;
	if(isInfinite() && o.isZero()) {
		setInfinity();
		return true;
	}
	if(o.isInfinite()) {
		clear();
		return true;
	}
	if(isInfinite()) {
		if(o.isComplex()) {
			setInfinity();
		} else if(o.isNegative()) {
			b_pinf = !b_pinf;
			b_minf = !b_minf;
		}
		return true;
	}
	if(o.isZero()) {
		if(isZero()) return false;
		//division by zero!!!
		setInfinity();
		return true;
	}
	if(isZero()) {
		return true;
	}
	value = value / o.internalNumber();
	removeFloatZeroPart();
	return true;
}
bool Number::recip() {
	if(isZero()) {
		//division by zero!!!
		setInfinity();
		return false;
	}
	if(isInfinite()) {
		clear();
		return true;
	}
	value = cln::recip(value);
	removeFloatZeroPart();
	return true;
}
bool Number::raise(const Number &o, int solution) {
	if(o.isInfinity()) return false;
	if(isInfinite()) {	
		if(o.isNegative()) {
			clear();
			return true;
		}
		if(o.isZero()) {
			return false;
		}
		if(isMinusInfinity()) {
			if(o.isEven()) {
				setPlusInfinity();
			} else if(!o.isInteger()) {
				setInfinity();
			}
		}
		return true;
	}
	if(o.isMinusInfinity()) {
		if(isZero()) {
			setInfinity();
		} else if(isComplex()) {
			return false;
		} else {
			clear();
		}
		return true;
	}
	if(o.isPlusInfinity()) {
		if(isZero()) {
		} else if(isComplex() || isNegative()) {
			return false;
		} else {
			setPlusInfinity();
		}
		return true;
	}
	if(isZero() && o.isNegative()) {
		CALCULATOR->error(true, _("Division by zero."), NULL);
		return false;
	}
	if(isZero()) {
		if(o.isZero()) {
			//0^0
			CALCULATOR->error(false, _("0^0 might be considered undefined"), NULL);
			set(1, 1);
			setApproximate(o.isApproximate());
			return true;
		}
		return true;
	}
	bool neg = false;
	if(isNegative() && !o.isComplex() && !o.isApproximate() && !o.numeratorIsEven() && !o.denominatorIsEven()) {
		neg = true;
		value = cln::abs(value);
	}
	value = expt(value, o.internalNumber());
	if(neg) {
		value = -value;
	}
	removeFloatZeroPart();
	return true;
}
bool Number::exp10(const Number &o) {
	if(o.isZero() || isZero()) {
		return true;
	}
	Number ten(10, 1);
	if(!ten.raise(o)) {
		return false;
	}
	multiply(ten);
	return true;
}
bool Number::exp10() {
	if(isZero()) {
		set(1, 1);
		return true;
	}
	Number ten(10, 1);
	if(!ten.raise(*this)) {
		return false;
	}
	set(ten);
	return true;
}
bool Number::exp2(const Number &o) {
	if(o.isZero() || isZero()) {
		return true;
	}
	Number two(2, 1);
	if(!two.raise(o)) {
		return false;
	}
	multiply(two);
	return true;
}
bool Number::exp2() {
	if(isZero()) {
		set(1, 1);
		return true;
	}
	Number two(2, 1);
	if(!two.raise(*this)) {
		return false;
	}
	set(two);
	return true;
}
bool Number::square() {
	if(isInfinite()) {
		setPlusInfinity();
		return true;
	}
	value = cln::square(value);
	return true;
}

bool Number::negate() {
	if(isInfinite()) {
		b_pinf = !b_pinf;
		b_minf = !b_minf;
		return true;
	}
	value = -value;
	return true;
}
void Number::setNegative(bool is_negative) {
	if(!isZero() && minusp(cln::realpart(value)) != is_negative) {
		if(isInfinite()) {b_pinf = !b_pinf; b_minf = !b_minf; return;}
		value = cln::complex(-cln::realpart(value), cln::imagpart(value));
	}
}
bool Number::abs() {
	if(isInfinite()) {
		setPlusInfinity();
		return true;
	}
	value = cln::abs(value);
	return true;
}
bool Number::signum() {
	if(isInfinite()) return false;
	value = cln::signum(value);
	return true;
}
bool Number::round() {
	if(isInfinite() || isComplex()) return false;
	value = cln::round1(cln::realpart(value));
	return true;
}
bool Number::floor() {
	if(isInfinite() || isComplex()) return false;
	value = cln::floor1(cln::realpart(value));
	return true;
}
bool Number::ceil() {
	if(isInfinite() || isComplex()) return false;
	value = cln::ceiling1(cln::realpart(value));
	return true;
}
bool Number::trunc() {
	if(isInfinite() || isComplex()) return false;
	value = cln::truncate1(cln::realpart(value));
	return true;
}
bool Number::round(const Number &o) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && round();
	}
	if(isComplex()) return false;
	if(o.isComplex()) return false;
	value = cln::round1(cln::realpart(value), cln::realpart(o.internalNumber()));
	return true;
}
bool Number::floor(const Number &o) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && floor();
	}
	if(isComplex()) return false;
	if(o.isComplex()) return false;
	value = cln::floor1(cln::realpart(value), cln::realpart(o.internalNumber()));
	return true;
}
bool Number::ceil(const Number &o) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && ceil();
	}
	if(isComplex()) return false;
	if(o.isComplex()) return false;
	value = cln::ceiling1(cln::realpart(value), cln::realpart(o.internalNumber()));
	return true;
}
bool Number::trunc(const Number &o) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && trunc();
	}
	if(isComplex()) return false;
	if(o.isComplex()) return false;
	value = cln::truncate1(cln::realpart(value), cln::realpart(o.internalNumber()));
	return true;
}
bool Number::mod(const Number &o) {
	if(isInfinite() || o.isInfinite()) return false;
	if(isComplex() || o.isComplex()) return false;
	value = cln::mod(cln::realpart(value), cln::realpart(o.internalNumber()));
	return true;
}	
bool Number::frac() {
	if(isInfinite() || isComplex()) return false;
	cl_N whole_value = cln::truncate1(cln::realpart(value));
	value -= whole_value;
	return true;
}
bool Number::rem(const Number &o) {
	if(isInfinite() || o.isInfinite()) return false;
	if(isComplex() || o.isComplex()) return false;
	value = cln::rem(cln::realpart(value), cln::realpart(o.internalNumber()));
	return true;
}

int Number::getBoolean() const {
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
	setInternal(cln::exp1());
}
void Number::pi() {
	setInternal(cln::pi());
}
void Number::catalan() {
	setInternal(cln::catalanconst());
}
void Number::euler() {
	setInternal(cln::eulerconst());
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
	value = cln::zeta(intValue()); 
	return true;
}			

bool Number::sin() {
	if(isInfinite()) return false;
	if(isZero()) return true;
	value = cln::sin(value);
	removeFloatZeroPart();
	return true;
}
bool Number::asin() {
	if(isInfinite()) return false;
	if(isZero()) return true;
	value = cln::asin(value);
	removeFloatZeroPart();
	return true;
}
bool Number::sinh() {
	if(isInfinite()) return true;
	if(isZero()) return true;
	value = cln::sinh(value);
	removeFloatZeroPart();
	return true;
}
bool Number::asinh() {
	if(isInfinite()) return true;
	if(isZero()) return true;
	value = cln::asinh(value);
	removeFloatZeroPart();
	return true;
}
bool Number::cos() {
	if(isInfinite()) return false;
	if(isZero() && !isApproximate()) {
		set(1);
		return true;
	}
	value = cln::cos(value);
	removeFloatZeroPart();
	return true;
}	
bool Number::acos() {
	if(isInfinite()) return false;
	if(isOne() && !isApproximate()) {
		clear();
		return true;
	}
	value = cln::acos(value);
	removeFloatZeroPart();
	return true;
}
bool Number::cosh() {
	if(isInfinite()) {
		setInfinity();
		return true;
	}
	if(isZero() && !isApproximate()) {
		set(1);
		return true;
	}
	value = cln::cosh(value);
	removeFloatZeroPart();
	return true;
}
bool Number::acosh() {
	if(isPlusInfinity() || isInfinity()) return true;
	if(isMinusInfinity()) return false;
	value = cln::acosh(value);
	removeFloatZeroPart();
	return true;
}
bool Number::tan() {
	if(isInfinite()) return false;
	if(isZero()) return true;
	value = cln::tan(value);
	removeFloatZeroPart();
	return true;
}
bool Number::atan() {
	if(isInfinity()) return false;
	if(isZero()) return true;
	if(isInfinite()) {
		pi();
		divide(2);
		if(isMinusInfinity()) negate();
		return true;
	}
	value = cln::atan(value);
	removeFloatZeroPart();
	return true;
}
bool Number::tanh() {
	if(isInfinity()) return true;
	if(isPlusInfinity()) set(1);
	if(isMinusInfinity()) set(-1);
	if(isZero()) return true;
	value = cln::tanh(value);
	removeFloatZeroPart();
	return true;
}
bool Number::atanh() {
	if(isInfinite()) return false;
	if(isZero()) return true;
	if(isOne()) {
		setPlusInfinity();
		return true;
	}
	if(isMinusOne()) {
		setMinusInfinity();
		return true;
	}
	value = cln::atanh(value);
	removeFloatZeroPart();
	return true;
}
bool Number::ln() {
	if(isPlusInfinity()) return true;
	if(isInfinite()) return false;
	if(isOne() && !isApproximate()) {
		clear();
		return true;
	}
	if(isZero()) {
		setMinusInfinity();
		return true;
	}
	value = cln::log(value);
	removeFloatZeroPart();
	return true;
}
bool Number::log(const Number &o) {
	if(isPlusInfinity()) return true;
	if(isInfinite()) return false;
	if(isOne() && !isApproximate() && !o.isApproximate()) {
		clear();
		return true;
	}
	if(isZero()) {
		setMinusInfinity();
		return true;
	}
	if(o.isZero()) {
		clear();
		return true;
	}
	if(o.isOne()) {
		setInfinity();
		return true;
	}
	value = cln::log(value, o.internalNumber());
	removeFloatZeroPart();
	return true;
}
bool Number::exp() {
	if(isInfinity()) return false;
	if(isPlusInfinity()) return true;
	if(isMinusInfinity()) {
		clear();
		return true;
	}
	value = cln::exp(value);
	return true;
}
bool Number::gcd(const Number &o) {
	if(!isInteger() || !o.isInteger()) {
		return false;
	}
	if(isZero() || o.isZero()) {
		clear(); 
		return true;
	}
	cl_I num = cln::numerator(cln::rational(cln::realpart(value)));
	cl_I num_o = cln::numerator(cln::rational(cln::realpart(o.internalNumber())));
	value = cln::gcd(num, num_o);
	return true;
}

bool Number::factorial() {
	if(!isInteger()) {
		return false;
	}
	if(isNegative()) {
		setPlusInfinity();
		return true;
	}
	if(isZero()) {
		set(1);
		return true;
	} else if(isOne()) {
		return true;
	} else if(isNegative()) {
		return false;
	}
	Number i(*this);
	i.add(-1);
	for(; !i.isOne(); i.add(-1)) {
		multiply(i);
	}
	return true;
}
bool Number::binomial(const Number &m, const Number &k) {
	if(!m.isInteger() || !k.isInteger()) return false;
	if(k.isNegative()) return false;
	if(m.isZero() || m.isNegative()) return false;
	if(k.isGreaterThan(m)) return false;
	if(k.isZero()) {
		set(1);
	} else if(k.isOne()) {
		set(m);
	} else if(m.equals(k)) {
		set(1);
	} else {
		clear();
		cl_I im = cln::numerator(cln::rational(cln::realpart(m.internalNumber())));
		cl_I ik = cln::numerator(cln::rational(cln::realpart(k.internalNumber())));
		if(m.isGreaterThan(INT_MAX) || k.isGreaterThan(INT_MAX)) {
			ik--;
			Number k_fac(k);
			k_fac.factorial();
			cl_I ithis = im;
			for(; !zerop(ik); ik--) {
				im--;
				ithis *= im;
			}
			value = ithis;
			divide(k_fac);
		} else {
			value = cln::binomial(cl_I_to_uint(im), cl_I_to_uint(ik));
		}
	}
	return true;
}

bool Number::add(const Number &o, MathOperation op) {
	switch(op) {
		case OPERATION_SUBTRACT: {
			return subtract(o);
		}
		case OPERATION_ADD: {
			return add(o);
		} 
		case OPERATION_MULTIPLY: {
			return multiply(o);
		}
		case OPERATION_DIVIDE: {
			return divide(o);
		}		
		case OPERATION_RAISE: {
			return raise(o);
		}
		case OPERATION_EXP10: {
			return exp10(o);
		}
		case OPERATION_OR: {
			setTrue(isPositive() || o.isPositive());
			return true;
		}
		case OPERATION_XOR: {
			if(isPositive()) setTrue(!o.isPositive());
			else setTrue(o.isPositive());
			return true;
		}
		case OPERATION_AND: {
			setTrue(isPositive() && o.isPositive());
			return true;
		}
		case OPERATION_EQUALS: {
			setTrue(equals(o));
			return true;
		}
		case OPERATION_GREATER: {
			int i = compare(o);
			if(i != -2) setTrue(i == -1);
			if(i != -2) return true;
			return false;
		}
		case OPERATION_LESS: {
			int i = compare(o);
			if(i != -2) setTrue(i == 1);
			if(i != -2) return true;
			return false;
		}
		case OPERATION_EQUALS_GREATER: {
			int i = compare(o);
			if(i != -2) setTrue(i == 0 || i == -1);
			if(i != -2) return true;
			return false;
		}
		case OPERATION_EQUALS_LESS: {
			int i = compare(o);
			if(i != -2) setTrue(i == 0 || i == 1);
			if(i != -2) return true;
			return false;
		}
		case OPERATION_NOT_EQUALS: {
			setTrue(!equals(o));
			return true;
		}
	}
	return false;	
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

string Number::print(const PrintOptions &po, const InternalPrintStruct &ips) const {

	if(ips.minus) *ips.minus = false;
	if(ips.num) *ips.num = "";
	if(ips.den) *ips.den = "";
	if(ips.re) *ips.re = "";
	if(ips.im) *ips.im = "";
	string str;
	int base;
	int min_decimals = po.min_decimals;
	if(min_decimals < 0) min_decimals = 0;
	if(po.base <= 1) base = 10;
	else if(po.base > 36) base = 36;
	else base = po.base;
	if(isComplex()) {
		bool bre = hasRealPart();
		if(bre) {
			Number re, im;
			re.setInternal(cln::realpart(value));
			im.setInternal(cln::imagpart(value));
			str = re.print(po, ips);
			if(ips.re) *ips.re = str;
			InternalPrintStruct ips_n = ips;
			bool neg = false;
			ips_n.minus = &neg;
			string str2 = im.print(po, ips_n);
			if(ips.im) *ips.im = str2;
			if(*ips_n.minus) {
				str += " - ";
			} else {
				str += " + ";
			}
			str += str2;	
		} else {
			Number im;
			im.setInternal(cln::imagpart(value));
			str = im.print(po, ips);
			if(ips.im) *ips.im = str;
		}
		if(!po.short_multiplication) {
			if(po.spacious) {
				str += " * ";
			} else {
				str += "*";
			}
		}
		str += "i";
		if(ips.num) *ips.num = str;
	} else if(isInteger()) {
		bool neg = cln::minusp(cln::realpart(value));
		string mpz_str = printCL_I(cln::numerator(cln::rational(cln::realpart(value))), base, false);
		int expo = 0;
		if(mpz_str.length() > 0 && po.number_fraction_format == FRACTION_DECIMAL) {
			expo = mpz_str.length() - 1;
		} else if(mpz_str.length() > 0) {
			for(int i = mpz_str.length() - 1; i >= 0; i--) {
				if(mpz_str[i] != '0') {
					break;
				}
				expo++;
			} 
		}
		if(base != 10) expo = 0;
		if(po.min_exp < 0) {
			if(expo > -PRECISION && expo < PRECISION) { 
				expo = 0;
			}
		} else if(po.min_exp != 0) {
			if(expo > -po.min_exp && expo < po.min_exp) { 
				expo = 0;
			}
		} else {
			expo = 0;
		}
		if(expo > 0) {
			if(po.number_fraction_format == FRACTION_DECIMAL) {
				mpz_str.insert(mpz_str.length() - expo, ".");
				mpz_str = mpz_str.substr(0, PRECISION + 1);
			} else if(po.number_fraction_format == FRACTION_DECIMAL_EXACT) {
				mpz_str.insert(mpz_str.length() - expo, ".");
			} else {
				mpz_str = mpz_str.substr(0, mpz_str.length() - expo);
			}
		}
		str = "";
		if(ips.minus) {
			*ips.minus = neg;
		} else if(neg) {
			str += "-";
		}
		if(base == 16) {
			str += "0x";
		} else if(base == 8) {
			str += "0";
		} 
		str += mpz_str;
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
		if(po.number_fraction_format == FRACTION_DECIMAL) {
			int pos = str.length() - 1;
			for(; pos >= (int) (str.length() - expo) + min_decimals; pos--) {
				if(str[pos] != '0') {
					break;
				}
			}
			str = str.substr(0, pos + 1);
			if(po.max_decimals >= 0 && expo > po.max_decimals) {
				int i = (str.length() - expo) + po.max_decimals;
				if(str[i] >= '5') {
					str[i - 1]++;
				}
				str = str.substr(0, i);
				if(po.is_approximate) *po.is_approximate = true;
			} else if(min_decimals > expo) {
				for(int i = min_decimals - (str.length() - expo); i > 0; i--) {
					str += "0";
				}
			}
			if(str[str.length() - 1] == '.') str.erase(str.end() - 1);
		}
		if(po.number_fraction_format == FRACTION_DECIMAL && min_decimals > 0) {
			str += ".";
			for(int i = min_decimals; i > 0; i--) {
				str += "0";
			}
		}
		if(expo != 0) {
			str += "E";
			str += i2s(expo);
		}
		if(ips.num) *ips.num = str;
	} else if(isInfinity()) {
		if(po.use_unicode_signs) {
			str = SIGN_INFINITY;
		} else {
			str = _("infinity");
		}
	} else if(isPlusInfinity()) {
		str = "(";
		str += "+";
		if(po.use_unicode_signs) {
			str += SIGN_INFINITY;
		} else {
			str += _("infinity");
		}
		str += ")";
	} else if(isMinusInfinity()) {
		str = "(";
		str += "-";
		if(po.use_unicode_signs) {
			str += SIGN_INFINITY;
		} else {
			str += _("infinity");
		}
		str += ")";
	} else {
		if(isApproximate() || po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT) {
		
			cln::cl_I num, d = cln::denominator(cln::rational(cln::realpart(value))), remainder = 0, remainder2 = 0, exp = 0;
			cln::cl_I_div_t div;
			bool neg = cln::minusp(cln::realpart(value));
			if(neg) {
				num = -cln::numerator(cln::rational(cln::realpart(value)));
			} else {
				num = cln::numerator(cln::rational(cln::realpart(value)));
			}
			
			int l10 = 0;
			div = cln::truncate2(num, d);
			remainder = div.remainder;
			num = div.quotient;
			
			bool exact = cln::zerop(remainder);
				
			vector<cln::cl_I> remainders;
			bool infinite_series = false;
			int precision = PRECISION;
			if(base > 10) precision = (precision * 10) / base;
			
			bool started = false;
			if(!cln::zerop(num)) {
				str = printCL_I(num, base, true);
				precision -= str.length();
				started = true;
			}
			while(!exact && precision) {
				if(po.indicate_infinite_series && !infinite_series) {
					remainders.push_back(remainder);
				}
				remainder *= base;
				div = cln::truncate2(remainder, d);
				remainder2 = div.remainder;
				remainder = div.quotient;
				exact = cln::zerop(remainder2);
				if(!started) {
					started = !cln::zerop(remainder);
				}
				if(started) {
					num *= base;	
					num += remainder;
				}
				l10++;
				remainder = remainder2;
				if(po.indicate_infinite_series && !exact && !infinite_series) {
					for(unsigned int i = 0; i < remainders.size(); i++) {
						if(remainders[i] == remainder) {
							infinite_series = true;
							break;
						}
					}
				}
				if(started) {
					precision--;
				}
			}
			remainders.clear();
			if(!exact && !infinite_series) {
				remainder *= base;
				div = cln::truncate2(remainder, d);
				remainder2 = div.remainder;
				remainder = div.quotient;
				if(remainder >= base / 2 + base % 2) {
					num += 1;
				}
			}
			if(!exact && !infinite_series) {
				if(po.number_fraction_format == FRACTION_DECIMAL_EXACT) {
					PrintOptions po2 = po;
					po2.number_fraction_format = FRACTION_FRACTIONAL;
					return print(po2, ips);
				}
				if(po.is_approximate) *po.is_approximate = true;
			}
			
			str = printCL_I(num, base, true);
			
			int expo = str.length() - l10 - 1;
			if(str.empty() || base != 10) expo = 0;
			if(po.min_exp < 0) {
				if(expo > -PRECISION && expo < PRECISION) { 
					expo = 0;
				}
			} else if(po.min_exp != 0) {
				if(expo > -po.min_exp && expo < po.min_exp) { 
					expo = 0;
				}
			} else {
				expo = 0;
			}
			if(expo != 0) {
				l10 += expo;
			}
			if(l10 > 0) {
				l10 = str.length() - l10;
				if(l10 < 1) {
					str.insert(str.begin(), 1 - l10, '0');
					l10 = 1;
				}
				str.insert(l10, ".");
				int l2 = 0;
				while(str[str.length() - 1 - l2] == '0') {
					l2++;
				}
				if(l2 == l10) l2++;
				if(l2 > 0) {
					str = str.substr(0, str.length() - l2);
				}
			}
			if(str.empty()) {
				str = "0";			
			}
			if(min_decimals > 0) {
				if(l10 > 0) {
					if(min_decimals > l10) {
						for(int i = min_decimals - l10; i > 0; i--) {
							str += "0";
						}
					}
				} else {
					str += ".";
					for(int i = min_decimals; i > 0; i--) {
						str += "0";
					}
				}
			}
			if(str[str.length() - 1] == '.') {
				str.erase(str.end() - 1);
			}
			if(infinite_series) {
				str += "...";
			}
			if(expo != 0) {
				str += "E";
				str += i2s(expo);
			}
			if(base == 16) {
				str.insert(0, "0x");
			} else if(base == 8) {
				str.insert(0, "0");
			} 
			if(ips.minus) {
				*ips.minus = neg;
			} else if(neg) {
				str.insert(0, "-");
			}
			if(ips.num) *ips.num = str;
		} else {
			Number num, den;
			num.setInternal(cln::numerator(cln::rational(cln::realpart(value))));
			den.setInternal(cln::denominator(cln::rational(cln::realpart(value))));
			str = num.print(po, ips);
			if(ips.num) *ips.num = str;
			str += " / ";
			InternalPrintStruct ips_n = ips;
			ips_n.minus = NULL;
			string str2 = den.print(po, ips_n);
			if(ips.den) *ips.den = str2;
			str += str2;
		}
	}
	return str;
}

/*void Number::getPrintObjects(bool &minus, string &whole_, string &numerator_, string &denominator_, bool &exp_minus, string &exponent_, string &prefix_, NumberFormat nrformat, int displayflags, int min_decimals, int po.max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Number *l_exp, bool in_composite, bool in_power, Number *l_exp2, Prefix **prefix1, Prefix **prefix2) const {
	if(CALCULATOR->alwaysExact()) po.max_decimals = -1;
	if(in_exact && isApproximate()) *in_exact = true;
	cl_I exp = 0;
	cl_I num = cln::numerator(cln::rational(cln::realpart(value)));
	cl_I den = cln::denominator(cln::rational(cln::realpart(value)));
	if(!isZero() && nrformat != NUMBER_FORMAT_SEXAGESIMAL && nrformat != NUMBER_FORMAT_TIME) {
		if(!isZero()) {
			if((!(displayflags & DISPLAY_FORMAT_FRACTION) && !(displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY))) {		
				Number exp_pre = realPart();
				exp_pre.setNegative(false);
				bool b_always_exact = CALCULATOR->alwaysExact();
				CALCULATOR->setAlwaysExact(false);	
				if(exp_pre.isLessThan(1)) {
					exp_pre.setInternal(-cln::log(cln::recip(exp_pre.internalNumber()), 10));
				} else if(exp_pre.isOne()) {
					exp_pre.clear();
				} else {
					exp_pre.setInternal(cln::log(exp_pre.internalNumber(), 10));
				}
				exp_pre.floor();
				exp = cln::numerator(cln::rational(cln::realpart(exp_pre.internalNumber())));
				CALCULATOR->setAlwaysExact(b_always_exact);
			} else {
				if(cln::rem(num, 10) == 0) {
					cl_I num_test = num;				
					while(true) {	
						cl_I_div_t div = cln::truncate2(num_test, 10);
						num_test = div.quotient;
						if(div.remainder != 0) {
							break;
						}
						exp++;		
					}			
				} else {	
					cl_I den_test = den;
					while(true) {	
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
					exp_spec -= cln::numerator(cln::rational(cln::realpart(tmp_exp.internalNumber())));
					prefix_ = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
					if(prefix1) *prefix1 = prefix;
				}
			} else {
				tmp_exp.setInternal(exp);
				if(l_exp2 && prefix2 && CALCULATOR->denominatorPrefixEnabled()) {	
					tmp_exp.trunc(l_exp);
					tmp_exp.setNegative(false);
					if(tmp_exp.isGreaterThan(5)) {
						tmp_exp.setInternal(exp);
						Number nr(2, 1);
						tmp_exp.trunc(&nr);
						if(!oddp(cln::numerator(cln::rational(cln::realpart(tmp_exp.internalNumber()))))) {
							if(tmp_exp.isNegative()) tmp_exp.add(-1);
							else tmp_exp.add(1);
						}
					} else {
						tmp_exp.setInternal(exp);
					}
				}
				p = CALCULATOR->getBestPrefix(&tmp_exp, l_exp);
				Number test_exp;
				test_exp.setInternal(exp);
				test_exp.subtract(p->exponent(l_exp, &tmp_exp));
				if(test_exp.isInteger()) {
					Number nr_exp;
					nr_exp.setInternal(exp);
					if((cln::plusp(exp) && nr_exp.compare(&test_exp) == -1) || (cln::minusp(exp) && nr_exp.compare(&test_exp) == 1)) {
						exp_spec = cln::numerator(cln::rational(cln::realpart(test_exp.internalNumber())));
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
				nr_exp_spec.setInternal(exp_spec);
				p = CALCULATOR->getBestPrefix(&nr_exp_spec, l_exp2);
				Number test_exp(&nr_exp_spec);
				test_exp.subtract(p->exponent(l_exp2, &tmp_exp));
				if(test_exp.isInteger()) {
					if((nr_exp_spec.isPositive() && nr_exp_spec.compare(&test_exp) == -1) || (nr_exp_spec.isNegative() && nr_exp_spec.compare(&test_exp) == 1)) {
						exp_spec = cln::numerator(cln::rational(cln::realpart(test_exp.internalNumber())));
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
					exp_spec -= cln::numerator(cln::rational(cln::realpart(tmp_exp.internalNumber())));
					prefix_ = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
					if(prefix2) *prefix2 = prefix;
				}
			} else {
				Number nr_exp;
				nr_exp.setInternal(exp);
				p = CALCULATOR->getBestPrefix(&nr_exp, l_exp2);
				Number test_exp(&nr_exp);
				test_exp.subtract(p->exponent(l_exp2, &tmp_exp));
				if(test_exp.isInteger()) {
					if((cln::plusp(exp) && nr_exp.compare(&test_exp) == -1) || (cln::minusp(exp) && nr_exp.compare(&test_exp) == 1)) {
						exp_spec = cln::numerator(cln::rational(cln::realpart(test_exp.internalNumber())));
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
		Number nr;
		nr.setInternal(cln::realpart(value));
		nr.trunc();
		whole_ = nr.printNumerator(10, false);
		if(nrformat == NUMBER_FORMAT_SEXAGESIMAL) {
			if(displayflags & DISPLAY_FORMAT_NONASCII) {
				whole_ += SIGN_POWER_0;
			} else {
				whole_ += "o";
			}	
		}
		nr.setInternal(cln::realpart(value));
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
		nr.setInternal(whole / den_spec);
		bool infinite_series = false;
		cl_I nr_num, nr_den;
		bool b_exact;
		if(displayflags & DISPLAY_FORMAT_INDICATE_INFINITE_SERIES) {
			b_exact = nr.floatify(&nr_num, &nr_den, PRECISION, po.max_decimals, &infinite_series);
		} else {
			b_exact = nr.floatify(&nr_num, &nr_den, PRECISION, po.max_decimals);
		}
		if(!b_exact) {
			if(!isApproximate() && ((displayflags & DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT) || CALCULATOR->alwaysExact())) {
				displayflags = displayflags | DISPLAY_FORMAT_FRACTIONAL_ONLY;
				return getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags, min_decimals, po.max_decimals, prefix, in_exact, usable, toplevel, plural, l_exp, in_composite, in_power);
			}
		}
		if(in_exact && !b_exact && !infinite_series) {
			*in_exact = true;
		}
		whole_ = printCL_I(nr_num, 10, false);
		int l10 = 0;
		cl_I d = nr_den;
		while(true) {	
			cl_I_div_t div = cln::truncate2(d, 10);
			d = div.quotient;
			if(div.remainder != 0) {
				break;
			}
			l10++;		
		}*/
/*		Fraction d(fr.denominator());
		d.log10();
		l10 = d.numerator()->getInt();*/
/*		if(l10) {
			l10 = whole_.length() - l10;
			if(l10 < 1) {
				whole_.insert(whole_.begin(), 1 - l10, '0');
				l10 = 1;
			}
			whole_.insert(l10, CALCULATOR->getDecimalPoint());
			l10 = 0;
			while(whole_[whole_.length() - 1 - l10] == '0') {
				l10++;
			}
			if(whole_.length() >= strlen(CALCULATOR->getDecimalPoint())) {
				bool b = true;
				for(unsigned int i = strlen(CALCULATOR->getDecimalPoint()); i > 0; i--) {
					if(whole_[whole_.length() - i - l10] != CALCULATOR->getDecimalPoint()[i - 1]) {
						b = false;
						break;
					}
				}
				if(b) {
					l10 += strlen(CALCULATOR->getDecimalPoint());
				}
			}
			if(l10 > 0) {
				whole_ = whole_.substr(0, whole_.length() - l10);
			}
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
}*/

