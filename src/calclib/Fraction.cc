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

Fraction::Fraction(long long int numerator_, long long int denominator_, long long int exp10_) {
	set(numerator_, denominator_, exp10_);
}
Fraction::Fraction(Fraction *fr) {
	if(!fr) return;
	num = fr->numerator();
	den = fr->denominator();
	exp = fr->exponent();
}
Fraction::Fraction(string str) {
	set(str);
}

Fraction::~Fraction() {
}
void Fraction::set(long long int numerator_, long long int denominator_, long long int exp10_) {
	num = numerator_;
	den = denominator_;
	exp = exp10_;
	while(num % 10 == 0) {
		num /= 10;
		exp++;
	}
	while(den % 10 == 0) {
		den /= 10;
		exp--;
	}	
}
void Fraction::set(string str) {
	den = 1;
	num = 0;
	exp = 0;	
	bool numbers_started = false, minus = false;
	for(int index = 0; index < str.size(); index++) {
		if(str[index] >= '0' && str[index] <= '9') {
			num *= 10;
			num += str[index] - '0';
			numbers_started = true;
		} else if(str[index] == 'E') {
			index++;
			numbers_started = false;
			bool exp_minus = false;
			while(index < str.size()) {
				if(str[index] >= '0' && str[index] <= '9') {				
					exp *= 10;
					exp += str[index] - '0';
					numbers_started = true;
				} else if(!numbers_started && str[index] == '-') {
					exp_minus = !exp_minus;
				}
				index++;
			}
			if(exp_minus) exp = -exp;
			break;
		} else if(!numbers_started && str[index] == '-') {
			minus = !minus;
		}
	}
	if(minus) num = -num;
	while(num % 10 == 0) {
		num /= 10;
		exp++;
	}
	while(den % 10 == 0) {
		den /= 10;
		exp--;
	}	
}

bool Fraction::equals(Fraction *fr) {
	if(!fr) return false;
	return num == fr->numerator() && den == fr->denominator() && exp == fr->exponent();
}
long long int Fraction::numerator() const {
	return num;
}
long long int Fraction::denominator() const {
	return den;
}
long long int Fraction::exponent() const {
	return exp;
}
long double Fraction::value() const {
	return exp10l(exp) * num / den;
}
bool Fraction::isNegative() const {
	return num < 0;
}
bool Fraction::isOne() const {
	return exp == 0 && num == 1 && den == 1;
}
bool Fraction::isMinusOne() const {
	return exp == 0 && num == -1 && den == 1;
}
bool Fraction::isInteger() const {
	return den == 1 && exp >= 0;
}
long long int Fraction::getInteger(bool &overflow) const {
	if(exp > 0) {
		long long int exp_v = 1;
		for(long long int i = 0; i < exp; i++) {
			exp_v *= 10;
		}
		return num / den * exp_v;
	} else if(exp < 0) {
		long long int exp_v = 1;
		for(long long int i = 0; i < -exp; i++) {
			exp_v *= 10;
		}
		return num / (den * exp_v);
	}	
	return num / den;
}
bool Fraction::isZero() const {
	return num == 0;
}
long double Fraction::getIntegerPart() const {
	
}
long double Fraction::getFractionPart() const {

}
bool Fraction::add(MathOperation op, Fraction *fr) {
	if(!fr) return false;
	printf("FRPRE [%s] %c [%s]\n", internalPrint(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str(), op2ch(op), fr->internalPrint(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str());		
	switch(op) {
		case SUBTRACT: {}
		case ADD: {
			long long int num2 = fr->numerator(), den2 = fr->denominator();
			long double d_test;
			if(op == SUBTRACT) {
				num2 = -num2;
			}							
			if(exp != fr->exponent()) {
				long long int exp1;
				bool b = false;
				if((fr->exponent() >= 0) || (exp >= 0)) {
					exp1 = fr->exponent();
					if(exp1 > 0) {
						d_test = num2 * exp10(exp1);
						if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
							for(long long int i = 0; i < exp1; i++) {
								num2 *= 10;
							}
							b = true;
						}
					} else {
						d_test = den2 * exp10(-exp1);
						if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
							for(long long int i = 0; i < -exp1; i++) {
								den2 *= 10;
							}
							b = true;
						}					
					}	
					if(b) {
						b = false;
						exp1 = exp;
						if(exp1 > 0) {
							d_test = num * exp10(exp1);
							if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
								for(long long int i = 0; i < exp1; i++) {
									num *= 10;
								}
								exp -= exp1;
								b = true;
							}
						} else {
							d_test = den * exp10(-exp1);
							if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
								for(long long int i = 0; i < -exp1; i++) {
									den *= 10;
								}
								exp -= exp1;							
								b = true;
							}					
						}						
					}				
				}
				if(!b) {
					exp1 = fr->exponent() - exp;
					if(exp1 > 0) {
						d_test = num2 * exp10(exp1);
						if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
							for(long long int i = 0; i < exp1; i++) {
								num2 *= 10;
							}
							b = true;
						}
					} else {
						d_test = den2 * exp10(-exp1);
						if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
							for(long long int i = 0; i < -exp1; i++) {
								den2 *= 10;
							}
							b = true;
						}					
					}					
				}
				if(!b) {
					exp1 = exp - fr->exponent();
					if(exp1 > 0) {
						d_test = num * exp10(exp1);
						if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
							for(long long int i = 0; i < exp1; i++) {
								num *= 10;
							}
							exp -= exp1;
							b = true;
						}
					} else {
						d_test = den * exp10(-exp1);
						if(d_test <= LONG_LONG_MAX && d_test >= LONG_LONG_MIN)  {
							for(long long int i = 0; i < -exp1; i++) {
								den *= 10;
							}
							exp -= exp1;							
							b = true;
						}					
					}					
				}												
				if(!b) return false;
			}
			d_test = (long double) num * (long double) den2 + (long double) num2 * (long double) den;
			if(d_test > LONG_LONG_MAX || d_test < LONG_LONG_MIN)  {
				return false;
			}
			num = num * den2 + num2 * den;
			d_test = (long double) den * (long double) den2;
			if(d_test > LONG_LONG_MAX || d_test < LONG_LONG_MIN)  {
				return false;
			}				
			den *= den2;			
			break;
		} 
		case MULTIPLY: {
			den *= fr->denominator();
			num *= fr->numerator();
			exp += fr->exponent();
			break;
		}
		case DIVIDE: {
			den *= fr->numerator();
			num *= fr->denominator();
			exp -= fr->exponent();
			break;
		}		
		case RAISE: {	
			if(fr->isInteger()) {
				bool overflow = false;
				long long int d = den;
				long long int n = num;
				long long int m_n = fr->getInteger(overflow);
				if(overflow) return false;			
				long double d_test = powl(num, m_n);
				if(d_test > LONG_LONG_MAX || d_test < LONG_LONG_MIN)  {
					return false;
				}
				exp *= m_n;									
				if(m_n < 0) {
					m_n = -m_n;
					den = n;
					num = d;
					d = den;
					n = num;
				}
				for(long long int i = 1; i < m_n; i++) {
					den *= d;
					num *= n;
				}
			} else {
				return false;
			}
			break;
		}
		case EXP10: {
			if(fr->isInteger()) {
				bool overflow = false;
				exp += fr->getInteger(overflow);
				if(overflow) return false;
			} else {
				return false;
			}
			break;
		}
	}
	if(num == 0) {
		return true;
	}
	if(den < 0) {
		num = -num;
		den = -den;
	}
	long long int divisor = gcd(num, den);
	if(divisor != 1) {
		num /= divisor;
		den /= divisor;
	}
	while(den != 1 && exp > 0) {
		num *= 10;
		divisor = gcd(num, den);
		if(divisor != 1) {
			exp--;
			num /= divisor;
			den /= divisor;			
		} else {
			num /= 10;
			break;
		}
	}
	while(num != 1 && exp < 0) {
		den *= 10;
		divisor = gcd(num, den);
		if(divisor != 1) {
			exp++;
			num /= divisor;
			den /= divisor;			
		} else {
			den /= 10;
			break;
		}
	}	
	while(num % 10 == 0) {
		num /= 10;
		exp++;
	}
	while(den % 10 == 0) {
		den /= 10;
		exp--;
	}
	printf("FRPOST [%s] %c [%s]\n", internalPrint(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str(), op2ch(op), fr->internalPrint(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str());	
	return true;	
}

string Fraction::internalPrint(NumberFormat nrformat, int displayflags, int precision, int min_decimals, int max_decimals, Prefix *prefix, bool *usable, bool toplevel, bool *plural, long int *l_exp, bool in_composite, bool in_power) {
	long long int whole = num / den;
	long long int part = num % den;
	long long int den_spec = den;
	long long int exp_spec = exp;	
	string str_spec = "", str_prefix = "";
	switch(nrformat) {
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
	}
	string str_base = "";
	if(whole != 0) {
		if(whole < 0) {
			if(displayflags & DISPLAY_FORMAT_TAGS) {
				str_base += SIGN_MINUS;
			} else {
				str_base += MINUS_STR;
			}		
			whole = -whole;
		}
		string str_whole = lli2s(whole);
//		remove_trailing_zeros(str_whole);
		str_base += str_whole;
	}
	if(part != 0) {
		if(part < 0) {
			if(whole == 0) {
				if(displayflags & DISPLAY_FORMAT_TAGS) {
					str_base += SIGN_MINUS;
				} else {
					str_base += MINUS_STR;
				}
			}
			part = -part;
		}
		string str_num = lli2s(part);
		string str_den = lli2s(den_spec);
		if(whole != 0) str_base += " ";
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
	return str_base + str_spec;	
}
string Fraction::print(NumberFormat nrformat, int displayflags, int precision, Prefix *prefix, bool *usable) {
	internalPrint(nrformat, displayflags, precision, -1, -1, prefix, usable, true);
}

