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
#include "Prefix.h"

#include <iostream>

#ifdef HAVE_LIBCLN
#define WANT_OBFUSCATING_OPERATORS
#include <cln/cln.h>
using namespace cln;
#endif

#define TO_CL_RA		cl_RA clfr = num.getCL_I() / den.getCL_I();
#define FROM_CL_RA		clfr = cln::rational(clr); num.set(cln::numerator(clfr)); den.set(cln::denominator(clfr));
#define CLN_NON_COMPLEX_FUNCTION(FUNC)	cl_RA clfr = num.getCL_I() / den.getCL_I(); cl_R clr = cln::FUNC(clfr); clfr = cln::rational(clr); num.set(cln::numerator(clfr)); den.set(cln::denominator(clfr)); b_exact = false;
#define CLN_FUNCTION(FUNC)	cl_RA clfr = num.getCL_I() / den.getCL_I(); cl_RA cli = complex_num.getCL_I() / complex_den.getCL_I(); cl_N cln = cln::complex(clfr, cli); cl_N clr = cln::FUNC(cln); clfr = cln::rational(cln::realpart(clr)); num.set(cln::numerator(clfr)); den.set(cln::denominator(clfr)); cli = cln::rational(cln::imagpart(clr)); complex_num.set(cln::numerator(cli)); complex_den.set(cln::denominator(cli)); b_exact = false;
//#define CLN_COMPLEX_FUNCTION(FUNC)	cl_RA clfr = num.getCL_I() / den.getCL_I(); cl_N clr = cln::FUNC(clfr); clfr = cln::rational(cln::realpart(clr)); num.set(cln::numerator(clfr)); den.set(cln::denominator(clfr)); b_exact = false;
#define CLN_COMPLEX_FUNCTION(FUNC)	cl_RA clfr = num.getCL_I() / den.getCL_I(); cl_RA cli = complex_num.getCL_I() / complex_den.getCL_I(); cl_N cln = cln::complex(clfr, cli); cl_N clr = cln::FUNC(cln); clfr = cln::rational(cln::realpart(clr)); num.set(cln::numerator(clfr)); den.set(cln::denominator(clfr)); cli = cln::rational(cln::imagpart(clr)); complex_num.set(cln::numerator(cli)); complex_den.set(cln::denominator(cli)); b_exact = false;


Fraction::Fraction() {
	clear();
}
Fraction::Fraction(long int numerator_, long int denominator_, long int exp10_, bool nogcd) {
	set(numerator_, denominator_, exp10_, nogcd);
}
Fraction::Fraction(const Integer *numerator_, const Integer *denominator_, const Integer *exp10_, bool nogcd) {
	set(numerator_, denominator_, exp10_, nogcd);
}
Fraction::Fraction(const Fraction *fr) {
	set(fr);
}
Fraction::Fraction(string str) {
	set(str);
}

Fraction::~Fraction() {}
void Fraction::clear() {
	b_exact = true;
	complex_num.clear();
	complex_den.set(1);
	num.clear();
	den.set(1);
}
void Fraction::set(long int numerator_, long int denominator_, long int exp10_, bool nogcd) {
	num.set(numerator_);
	den.set(denominator_);
	if(den.isNegative()) {
		num.setNegative(!num.isNegative());
		den.setNegative(false);
	}
	if(exp10_ < 0) {
		den.exp10(-exp10_);
	} else {
		num.exp10(exp10_);
	}	
	if(!nogcd) {
		Integer divisor;
		if(num.gcd(&den, &divisor)) {
			num.divide(&divisor);
			den.divide(&divisor);
		}
	}
	complex_num.clear();
	complex_den.set(1);
	b_exact = true;
}
void Fraction::setFloat(long double value_) {
	set(d2s(value_));
}
void Fraction::set(const Integer *numerator_, const Integer *denominator_, const Integer *exp10_, bool nogcd) {
	if(numerator_) num.set(numerator_);
	else num.set(1);
	if(denominator_) den.set(denominator_);
	else den.set(1);
	if(den.isNegative()) {
		num.setNegative(!num.isNegative());
		den.setNegative(false);
	}
	if(exp10_) {
		if(exp10_->isNegative()) {
			Integer exp10_pos(exp10_);
			exp10_pos.setNegative(false);
			den.exp10(&exp10_pos);
		} else {
			num.exp10(exp10_);
		}	
	}	
	if(!nogcd) {
		Integer divisor;
		if(num.gcd(&den, &divisor)) {
			num.divide(&divisor);
			den.divide(&divisor);
		}
	}
	complex_num.clear();
	complex_den.set(1);
	b_exact = true;
}
bool Fraction::set(string str) {
	den.set(1);
	num.clear();
	complex_num.clear();
	complex_den.set(1);
	remove_blank_ends(str);
	bool numbers_started = false, minus = false, in_decimals = false, b_cplx = false;
	for(unsigned int index = 0; index < str.size(); index++) {
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
				exp.setNegative(false);
				den.exp10(&exp);
			} else {
				num.exp10(&exp);
			}
			break;
		} else if(str[index] == '.') {
			in_decimals = true;
		} else if(!numbers_started && str[index] == '-') {
			minus = !minus;
		} else if(str[index] == 'i') {
			b_cplx = true;
		}
	}
	if(minus) num.setNegative(!num.isNegative());
	Integer divisor;
	if(num.gcd(&den, &divisor)) {
		num.divide(&divisor);
		den.divide(&divisor);
	}
	b_exact = true;
	if(b_cplx) {
		complex_num.set(&num);
		complex_den.set(&den);
		num.clear();
		den.set(1);
	}	
	return true;
}
void Fraction::set(const Fraction *fr) {
	if(!fr) return;
	num.set(fr->numerator());
	den.set(fr->denominator());
	complex_num.set(fr->complexNumerator());
	complex_den.set(fr->complexDenominator());
	b_exact = fr->isPrecise();
}
void Fraction::e() {
#ifdef HAVE_LIBCLN
	cl_F clf = cln::exp1(); 
	cl_RA clfr = cln::rational(clf); 
	num.set(cln::numerator(clfr)); 
	den.set(cln::denominator(clfr)); 
	setComplex(NULL);
	b_exact = false;
#else
	set(E_STRING);	
	b_exact = false;	
#endif
}
void Fraction::pi() {
#ifdef HAVE_LIBCLN
	cl_F clf = cln::pi(); 
	cl_RA clfr = cln::rational(clf); 
	num.set(cln::numerator(clfr)); 
	den.set(cln::denominator(clfr)); 
	setComplex(NULL);
	b_exact = false;
#else
	set(PI_STRING);	
	b_exact = false;	
#endif
}
void Fraction::catalan() {
#ifdef HAVE_LIBCLN
	cl_F clf = cln::catalanconst(); 
	cl_RA clfr = cln::rational(clf); 
	num.set(cln::numerator(clfr)); 
	den.set(cln::denominator(clfr));
	setComplex(NULL);
	b_exact = false;
#else
	set(CATALAN_STRING);	
	b_exact = false;
#endif
}
void Fraction::pythagoras() {
#ifdef HAVE_LIBCLN
	cl_I cl2 = 2;
	cl_R clf = cln::sqrt(cl2); 
	cl_RA clfr = cln::rational(clf); 
	num.set(cln::numerator(clfr)); 
	den.set(cln::denominator(clfr)); 
	setComplex(NULL);
	b_exact = false;
#else
	set(PYTHAGORAS_STRING);	
	b_exact = false;
#endif
}
void Fraction::euler() {
#ifdef HAVE_LIBCLN
	cl_F clf = cln::eulerconst(); 
	cl_RA clfr = cln::rational(clf); 
	num.set(cln::numerator(clfr)); 
	den.set(cln::denominator(clfr)); 
	setComplex(NULL);
	b_exact = false;
#else
	set(EULER_STRING);	
	b_exact = false;
#endif
}

void Fraction::golden() {
#ifdef HAVE_LIBCLN
	Fraction fr_half(1, 2);
	set(5);
	sqrt();
	multiply(&fr_half);
	add(&fr_half);
#else
	set(GOLDEN_STRING);	
	b_exact = false;
#endif
}

#ifdef HAVE_LIBCLN
bool Fraction::zeta() {
	if(isNegative() || !isInteger() || isZero() || isOne()) {
		CALCULATOR->error(true, _("Integral point for Riemann's zeta must be an integer > 1."), NULL);
		return false;
	}
	if(num.isGreaterThan(INT_MAX)) {
		CALCULATOR->error(true, _("Integral point for Riemann's zeta is too large."), NULL);
		return false;
	}
	if(CALCULATOR->alwaysExact()) return false;
	cl_F clf = cln::zeta(num.getInt()); 
	cl_RA clfr = cln::rational(clf); 
	num.set(cln::numerator(clfr)); 
	den.set(cln::denominator(clfr)); 
	setComplex(NULL);
	b_exact = false;	
	return true;
}
#endif

void Fraction::apery() {
#ifdef HAVE_LIBCLN
	cl_F clf = cln::zeta(3); 
	cl_RA clfr = cln::rational(clf); 
	num.set(cln::numerator(clfr)); 
	den.set(cln::denominator(clfr)); 
	setComplex(NULL);
	b_exact = false;
#else
	set(APERY_STRING);	
	b_exact = false;
#endif
}
bool Fraction::equals(const Fraction *fr) const {
	if(!fr) {
		return isZero();
	}
	return num.equals(fr->numerator()) && den.equals(fr->denominator()) && complex_num.equals(fr->complexNumerator()) && complex_den.equals(fr->complexDenominator());
}
int Fraction::compare(const Fraction *fr) const {
	if(equals(fr)) return 0;
	if(!isComplex()) {
		if(!fr->isComplex()) {
			Integer num_this(&num);
			Integer num_that(fr->numerator());
			num_this.multiply(fr->denominator());
			num_that.multiply(&den);
			return num_this.compare(&num_that);
		}
		return -2;
	} else {
		if(fr->isComplex()) {
			if(fr->complexNumerator()->equals(&complex_num) && fr->complexDenominator()->equals(&complex_den)) {
				Integer num_this(&num);
				Integer num_that(fr->numerator());
				num_this.multiply(fr->denominator());
				num_that.multiply(&den);
				return num_this.compare(&num_that);
			}
			return -2;
		} else {
			return -2;
		}
	}
}
bool Fraction::isGreaterThan(const Fraction *fr) const {
	return compare(fr) == -1;
}
bool Fraction::isLessThan(const Fraction *fr) const {
	return compare(fr) == 1;
}
bool Fraction::isPlural() const {
	if(isNegative()) {
		Integer num_p(&num);
		return num_p.isGreaterThan(&den);
	} else {
		return num.isGreaterThan(&den);
	}
}
const Integer *Fraction::numerator() const {
	return &num;
}
const Integer *Fraction::denominator() const {
	return &den;
}
const Integer *Fraction::complexNumerator() const {
	return &complex_num;
}
const Integer *Fraction::complexDenominator() const {
	return &complex_den;
}
bool Fraction::isComplex() const {
	return !complex_num.isZero();
}
void Fraction::setComplex(const Fraction *fr) {
	if(fr) {
		complex_num.set(fr->numerator());
		complex_den.set(fr->denominator());
	} else {
		complex_num.clear();
		complex_den.set(1);
	}
}
long double Fraction::value() const {
#ifdef HAVE_LIBCLN
	cl_RA clfr = num.getCL_I() / den.getCL_I();
	return cln::double_approx(clfr);
#else
	return strtold(print(NUMBER_FORMAT_DECIMALS, DISPLAY_FORMAT_DECIMAL_ONLY, CALCULATOR->getPrecision() + 5).c_str(), NULL);			
#endif	
}
bool Fraction::isPositive() const {
	return !isComplex() && num.isPositive();
}
bool Fraction::isNegative() const {
	return !isComplex() && num.isNegative();
}
void Fraction::setNegative(bool is_negative) {
	num.setNegative(is_negative);
}
bool Fraction::isOne() const {
	return num.isOne() && den.isOne() && !isComplex();
}
bool Fraction::isMinusOne() const {
	return num.isMinusOne() && den.isOne() && !isComplex();
}
bool Fraction::isInteger() const {
	return den.isOne() && !isComplex();
}
bool Fraction::isPrecise() const {
	return b_exact;
}
void Fraction::setPrecise(bool is_precise) {
	b_exact = is_precise;
}
Integer *Fraction::getInteger() const {
	Integer *integer = new Integer(&num);
	if(!den.isOne()) {
		integer->divide(&den);		
	}
	return integer;
}
bool Fraction::isZero() const {
	return num.isZero() && !isComplex();
}
bool Fraction::subtract(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;
	
	if(!fr->numerator()->isZero()) {
		num.multiply(fr->denominator());
		Integer num2(fr->numerator());
		num2.multiply(&den);		
		num2.setNegative(!num2.isNegative());
		num.add(&num2);
		den.multiply(fr->denominator());
	}
	
	if(fr->isComplex()) {
		complex_num.multiply(fr->complexDenominator());
		Integer num2(fr->complexNumerator());
		num2.multiply(&complex_den);		
		num2.setNegative(!num2.isNegative());
		complex_num.add(&num2);
		complex_den.multiply(fr->complexDenominator());
	}
	
	clean();
	return true;
}
bool Fraction::add(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;

	if(!fr->numerator()->isZero()) {
		num.multiply(fr->denominator());
		Integer num2(fr->numerator());
		num2.multiply(&den);		
		num.add(&num2);
		den.multiply(fr->denominator());
	}
	
	if(fr->isComplex()) {
		complex_num.multiply(fr->complexDenominator());
		Integer num2(fr->complexNumerator());
		num2.multiply(&complex_den);		
		complex_num.add(&num2);
		complex_den.multiply(fr->complexDenominator());
	}
	
	clean();
	return true;	
}
bool Fraction::multiply(const Fraction *fr) {
	
	if(isZero()) {
		return true;
	}

	if(!fr->isPrecise()) {
	
		b_exact = false;
		
	}
		
	if(fr->isZero()) {
	
		clear(); 
		return true;
		
	}
	
/*	cl_RA clfr = num.getCL_I() / den.getCL_I();
	cl_RA clfr_i = complex_num.getCL_I() / complex_den.getCL_I();
	cl_N clc1 = cln::complex(clfr, clfr_i);
	clfr = fr->numerator()->getCL_I() / fr->denominator()->getCL_I();
	clfr_i = fr->complexNumerator()->getCL_I() / fr->complexDenominator()->getCL_I();
	cl_N clc2 = cln::complex(clfr, clfr_i);
	cl_N clc = clc1 * clc2;
	cout << clc;
	cout << "\n";*/
	
	if(isComplex() && fr->isComplex()) {
		
		Integer den_ad(&den);
		Integer num_ad(&num);
		if(!num.isZero()) {
			den_ad.multiply(fr->complexDenominator());
			num_ad.multiply(fr->complexNumerator());
		}
		
		Integer den_bd(&complex_den);
		Integer num_bd(&complex_num);
		den_bd.multiply(fr->complexDenominator());
		num_bd.multiply(fr->complexNumerator());
		num_bd.setNegative(!num_bd.isNegative());
		
		if(!num.isZero()) {
			den.multiply(fr->denominator());
			num.multiply(fr->numerator());
			if(num.isZero()) {
				den.set(1);
			}
		}
		
		num.multiply(&den_bd);
		num_bd.multiply(&den);		
		num.add(&num_bd);
		den.multiply(&den_bd);
		
		complex_den.multiply(fr->denominator());
		complex_num.multiply(fr->numerator());
		if(complex_num.isZero()) {
			complex_den.set(1);
		}
		
		complex_num.multiply(&den_ad);
		num_ad.multiply(&complex_den);
		complex_num.add(&num_ad);
		complex_den.multiply(&den_ad);
		
	} else if(isComplex()) {
		
		if(!num.isZero()) {
			den.multiply(fr->denominator());
			num.multiply(fr->numerator());
		}
		
		complex_den.multiply(fr->denominator());
		complex_num.multiply(fr->numerator());
	
	} else if(fr->isComplex()) {
		
		complex_den.set(&den);
		complex_num.set(&num);
	
		den.multiply(fr->denominator());
		num.multiply(fr->numerator());
		if(num.isZero()) {
			den.set(1);
		}
		
		complex_den.multiply(fr->complexDenominator());
		complex_num.multiply(fr->complexNumerator());
	
	} else {
	
		den.multiply(fr->denominator());
		num.multiply(fr->numerator());
		
	}	
	
	clean();
	
	return true;
	
}
bool Fraction::divide(const Fraction *fr) {
	if(fr->isZero()) {
		//division by zero!!!
		return false;
	}
	if(isZero()) {
		return true;
	}
	if(!fr->isPrecise()) b_exact = false;

/*	cl_RA clfr = num.getCL_I() / den.getCL_I();
	cl_RA clfr_i = complex_num.getCL_I() / complex_den.getCL_I();
	cl_N clc1 = cln::complex(clfr, clfr_i);
	clfr = fr->numerator()->getCL_I() / fr->denominator()->getCL_I();
	clfr_i = fr->complexNumerator()->getCL_I() / fr->complexDenominator()->getCL_I();
	cl_N clc2 = cln::complex(clfr, clfr_i);
	cl_N clc = clc1 / clc2;
	cout << clc;
	cout << "\n";*/
	
	if(fr->isComplex()) {

		Integer den_ad(&den);
		Integer num_ad(&num);
		if(!num.isZero()) {
			den_ad.multiply(fr->complexDenominator());
			num_ad.multiply(fr->complexNumerator());
		}
		
		Integer den_bd(&complex_den);
		Integer num_bd(&complex_num);
		if(!complex_num.isZero()) {
			den_bd.multiply(fr->complexDenominator());
			num_bd.multiply(fr->complexNumerator());
		}
		
		if(!num.isZero()) {
			den.multiply(fr->denominator());
			num.multiply(fr->numerator());
		}
		
		if(!num_bd.isZero()) {
			num.multiply(&den_bd);
			num_bd.multiply(&den);		
			num.add(&num_bd);
			den.multiply(&den_bd);
		}
		
		if(!complex_num.isZero()) {
			complex_den.multiply(fr->denominator());
			complex_num.multiply(fr->numerator());
			if(complex_num.isZero()) {
				complex_den.set(1);
			}
		}
		
		if(!num_ad.isZero()) {
			complex_num.multiply(&den_ad);
			num_ad.multiply(&complex_den);		
			num_ad.setNegative(!num_ad.isNegative());
			complex_num.add(&num_ad);
			complex_den.multiply(&den_ad);
		}
		
		Fraction fr_d(fr->complexNumerator(), fr->complexDenominator(), NULL, true);
		Fraction fr_dc(&fr_d);
		fr_dc.multiply(&fr_d);
		if(!fr->numerator()->isZero()) {
			Fraction fr_c(fr->numerator(), fr->denominator(), NULL, true);
			Fraction fr_cd(&fr_c);
			fr_cd.multiply(&fr_c);
			fr_dc.add(&fr_cd);
		}
		divide(&fr_dc);
		
		return true;
		
	} else if(isComplex()) {
		
		if(!num.isZero()) {
			num.multiply(fr->denominator());
			den.multiply(fr->numerator());
		}
		
		complex_num.multiply(fr->denominator());
		complex_den.multiply(fr->numerator());
	
	} else {
	
		num.multiply(fr->denominator());
		den.multiply(fr->numerator());
		
	}
	
	clean();
	return true;
}
bool Fraction::gcd(const Fraction *fr) {
	if(!fr->isPrecise()) b_exact = false;
	if(fr->isZero()) {
		clear(); 
		return true;
	}
	num.multiply(fr->denominator());
	den.multiply(fr->numerator());		
	Integer divisor;
	num.gcd(&den, &divisor);
	set(&divisor);
	return true;
}
bool Fraction::sin() {
	if(isZero()) {
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(sin)
	return true;
#else
	setFloat(sinl(value()));
	b_exact = false;
	return true;
#endif
}
bool Fraction::asin() {
	if(isZero()) {
		return true;
	}
	Integer abs_num(&num);
	abs_num.setNegative(false);
/*	if(den.isLessThan(&abs_num)) {
		CALCULATOR->error(true, _("The arc sine is only defined for -1 to 1."), NULL);
		return false;
	}*/
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_COMPLEX_FUNCTION(asin)
	return true;
#else	
	setFloat(asinl(value()));
	b_exact = false;
	return true;
#endif
}
bool Fraction::sinh() {
	if(isZero()) {
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(sinh)
	return true;
#else
	setFloat(sinhl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::asinh() {
	if(isZero()) {
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_COMPLEX_FUNCTION(asinh)
	return true;
#else		
	setFloat(asinhl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::cos() {
	if(isZero()) {
		set(1);
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(cos)
	return true;
#else
	setFloat(cosl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::acos() {
	if(isOne()) {
		clear();
		return true;
	}
/*	Integer abs_num(&num);
	abs_num.setNegative(false);	
	if(den.isLessThan(&abs_num)) {
		CALCULATOR->error(true, _("The arc cosine is only defined for -1 to 1."), NULL);
		return false;
	}*/
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_COMPLEX_FUNCTION(asin)
	return true;
#else		
	setFloat(acosl(value()));
	b_exact = false;
	return true;
#endif
}
bool Fraction::cosh() {
	if(isZero()) {
		set(1);
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(cosh)
	return true;
#else
	setFloat(coshl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::acosh() {
/*	if(isNegative() || den.compare(&num) == -1) {
		CALCULATOR->error(true, _("The inverse hyperbolic cosine is undefined for x < 1."), NULL);
		return false;
	}*/
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_COMPLEX_FUNCTION(acosh)
	return true;
#else			
	setFloat(acoshl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::tan() {
	if(isZero()) {
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(tan)
	return true;
#else
	setFloat(tanl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::atan() {
	if(isZero()) {
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(atan)
	return true;
#else
	setFloat(atanl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::tanh() {
	if(isZero()) {
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(tanh)
	return true;
#else
	setFloat(tanhl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::atanh() {
	if(isZero()) {
		return true;
	}
/*	Integer abs_num(&num);
	abs_num.setNegative(false);
	if(!den.isGreaterThan(&abs_num)) {
		CALCULATOR->error(true, _("The inverse hyperbolic tangent is undefined for x > 1 or x < -1 and infinite for 1 and -1."), NULL);
		return false;
	}*/
	if(isOne() || isMinusOne()) {
		CALCULATOR->error(true, _("The inverse hyperbolic tangent is infinite for 1 and -1."), NULL);
		return false;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_COMPLEX_FUNCTION(asin)
	return true;
#else			
	setFloat(atanhl(value()));
	b_exact = false;
	return true;
#endif	
}
int Fraction::sqrt(int solution) {
/*	setFloat(sqrtl(value()));
	b_exact = false;*/
/*	if(isNegative()) {
		CALCULATOR->error(true, _("The square root is undefined for negative numbers."), NULL);	
		return false;
	}*/
	if(!root(2)) {
		return false;
	}
	if(solution == 2) {
		setNegative(true);
	}
	return 2;
}	
bool Fraction::cbrt() {
/*	setFloat(cbrtl(value()));
	b_exact = false;*/
	return root(3);
}
bool Fraction::log() {
	if(isOne()) {
		clear();
		return true;
	}
/*	if(isZero() || isNegative()) {
		CALCULATOR->error(true, _("The natural logarithm is undefined for negative numbers and infinite for zero."), NULL);
		return false;
	}*/
	if(isZero()) {
		CALCULATOR->error(true, _("The natural logarithm is infinite for zero."), NULL);
		return false;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
//	CLN_NON_COMPLEX_FUNCTION(ln)
	CLN_FUNCTION(log)
	return true;
#else
	setFloat(logl(value()));
	b_exact = false;
	return true;
#endif	
}
bool Fraction::log(Fraction *fr, bool tryexact) {
	if(isOne()) {
		clear();
		return true;
	}
/*	if(isZero() || isNegative()) {
		CALCULATOR->error(true, _("Logarithms is undefined for negative numbers and infinite for zero."), NULL);
		return false;
	}
	if(fr->isZero() || fr->isNegative()) {
		CALCULATOR->error(true, _("Logarithms with a negative or zero base is undefined."), NULL);
		return false;	
	}*/
	if(isZero()) {
		CALCULATOR->error(true, _("Logarithms is infinite for zero."), NULL);
		return false;
	}
#ifdef HAVE_LIBCLN
	cl_RA clbase = fr->numerator()->getCL_I() / fr->denominator()->getCL_I();	
	if(tryexact) {		
		if(!isComplex() && !fr->isComplex()) {
			bool b_minus = false;
			if(den.isGreaterThan(&num)) {
				//logp sets clns to zero when result is negative (when this < 1) !!?! 
				b_minus = true;
				Integer den_save(&den);
				den.set(&num);
				num.set(&den_save);
			}
			cl_RA clfr = num.getCL_I() / den.getCL_I();
			cl_RA clns;
			if(logp(clfr, clbase, &clns)) {
				num.set(cln::numerator(clns));
				den.set(cln::denominator(clns));	
				if(b_minus) {
					num.setNegative(true);
				}
				return true;
			}
			if(b_minus) {
				Integer den_save(&den);
				den.set(&num);
				num.set(&den_save);
			}
		}
		if(CALCULATOR->alwaysExact()) return false;
	} else {
		bool b_minus = false;
		if(den.isGreaterThan(&num)) {
			b_minus = true;
			Integer den_save(&den);
			den.set(&num);
			num.set(&den_save);
		}
		cl_RA clfr = num.getCL_I() / den.getCL_I();
		cl_R clns = cln::log(clfr, clbase);
		clfr = cln::rational(clns);
		num.set(cln::numerator(clfr));
		den.set(cln::denominator(clfr));	
		if(b_minus) {
			num.setNegative(true);
		}
		return true;
	}
	b_exact = false;		
	cl_RA clfr = num.getCL_I() / den.getCL_I();
	cl_RA clfr_i = complex_num.getCL_I() / complex_den.getCL_I();
	cl_N clc1 = cln::complex(clfr, clfr_i);	
	clfr = fr->numerator()->getCL_I() / fr->denominator()->getCL_I();
	clfr_i = fr->complexNumerator()->getCL_I() / fr->complexDenominator()->getCL_I();
	cl_N clc2 = cln::complex(clfr, clfr_i);	
	cl_N clc = cln::log(clc1, clc2);
	clfr = cln::rational(cln::realpart(clc));
	clfr_i = cln::rational(cln::imagpart(clc));
	num.set(cln::numerator(clfr));
	den.set(cln::denominator(clfr));
	complex_num.set(cln::numerator(clfr_i));
	complex_den.set(cln::denominator(clfr_i));
	return true;
#else
	bool p = isPrecise();
	Fraction test(this);	
	Fraction log_den(fr);
	log_den.log();
	log();
	divide(&log_den);
	setPrecise(p);
	if(isPrecise()) {
		Fraction base(fr);
		base.pow(this);
		if(!base.isPrecise() || !base.equals(&test)) {
			if(CALCULATOR->alwaysExact()) {
				set(test);
				return false;
			}
			b_exact = false;
		}
	}
	return true;
#endif	
}
bool Fraction::log2() {
	if(isOne()) {
		clear();
		return true;
	}
	if(isZero() || isNegative()) {
		CALCULATOR->error(true, _("Logarithms is undefined for negative numbers and infinite for zero."), NULL);
		return false;
	}
#ifdef HAVE_LIBCLN
	Fraction fr(2);
	return log(&fr);
#else	
	Fraction test(this);	
	setFloat(log2l(value()));
	if(isPrecise()) {
		Fraction fr_2(2);
		fr_2.pow(this);
		if(!fr_2.isPrecise() || !fr_2.equals(&test)) {
			if(CALCULATOR->alwaysExact()) {
				set(test);
				return false;
			}
			b_exact = false;
		}
	}
	return true;
#endif		
}
bool Fraction::log10() {
	if(isOne()) {
		clear();
		return true;
	}
	if(isZero() || isNegative()) {
		CALCULATOR->error(true, _("Logarithms is undefined for negative numbers and infinite for zero."), NULL);
		return false;
	}
#ifdef HAVE_LIBCLN
	Fraction fr(10);
	return log(&fr);
#else	
	Fraction test(this);	
	setFloat(log10l(value()));
	if(isPrecise()) {
		Fraction fr_2(2);
		fr_2.pow(this);
		if(!fr_2.isPrecise() || !fr_2.equals(&test)) {
			if(CALCULATOR->alwaysExact()) {
				set(test);
				return false;
			}		
			b_exact = false;
		}
	}	
	return true;
#endif		
}
bool Fraction::exp() {
	if(isZero()) {
		set(1);
		return true;
	}
	if(CALCULATOR->alwaysExact()) return false;
#ifdef HAVE_LIBCLN
	CLN_FUNCTION(exp)
	return true;
#else
	setFloat(expl(value()));
	b_exact = false;
	return true;
#endif
}
bool Fraction::exp2() {
	Fraction fr_2(2);
	if(!fr_2.pow(this)) {
		return false;
	}
	set(&fr_2);
	return true;
}
bool Fraction::exp10(const Fraction *fr) {
	Fraction ten(10);
	if(fr) {
		if(!ten.pow(fr)) return false;
		multiply(&ten);
	} else {
		if(!ten.pow(this)) return false;
		set(&ten);
	}
	return true;
}
int Fraction::add(MathOperation op, const Fraction *fr, int solution) {
	if(!fr) return false;
	switch(op) {
		case OPERATION_SUBTRACT: {
			return subtract(fr);
		}
		case OPERATION_ADD: {
			return add(fr);
		} 
		case OPERATION_MULTIPLY: {
			return multiply(fr);
		}
		case OPERATION_DIVIDE: {
			return divide(fr);
		}		
		case OPERATION_RAISE: {
			return pow(fr, solution);
		}
		case OPERATION_EXP10: {
			return exp10(fr);
		}
		case OPERATION_OR: {
			setTrue(isPositive() || fr->isPositive());
			return true;
		}
		case OPERATION_AND: {
			setTrue(isPositive() && fr->isPositive());
			return true;
		}
		case OPERATION_EQUALS: {
			setTrue(equals(fr));
			return true;
		}
		case OPERATION_GREATER: {
			int i = compare(fr);
			if(i != -2) setTrue(i == -1);
			return i != -2;
		}
		case OPERATION_LESS: {
			int i = compare(fr);
			if(i != -2) setTrue(i == 1);
			return i != -2;
		}
		case OPERATION_EQUALS_GREATER: {
			int i = compare(fr);
			if(i != -2) setTrue(i == 0 || i == -1);
			return i != -2;
		}
		case OPERATION_EQUALS_LESS: {
			int i = compare(fr);
			if(i != -2) setTrue(i == 0 || i == 1);
			return i != -2;
		}
		case OPERATION_NOT_EQUALS: {
			setTrue(!equals(fr));
			return true;
		}
	}
	return false;	
}
int Fraction::getBoolean() {
	if(isPositive()) {
		return 1;
	} else {
		return 0;
	}
}
void Fraction::toBoolean() {
	setTrue(isPositive());
}
void Fraction::setTrue(bool is_true) {
	if(is_true) {
		num.set(1);
		den.set(1);
	} else {
		clear();
	}
}
void Fraction::setFalse() {
	clear();
}
void Fraction::setNOT() {
	setTrue(!isPositive());
}
void Fraction::clean() {
	if(den.isZero()) {
		num.clear();
		den.set(1);
	} else if(num.isZero()) {
		den.set(1);
	} else {
		if(den.isNegative()) {
			den.setNegative(false);
			num.setNegative(!num.isNegative());
		}
		Integer divisor;
		if(num.gcd(&den, &divisor)) {
			num.divide(&divisor);
			den.divide(&divisor);
		}
	}
	
	if(complex_den.isZero()) {
		complex_num.clear();
		complex_den.set(1);
	} else if(complex_num.isZero()) {
		complex_den.set(1);
	} else {
		if(complex_den.isNegative()) {
			complex_den.setNegative(false);
			complex_num.setNegative(!complex_num.isNegative());
		}
		Integer divisor;
		if(complex_num.gcd(&complex_den, &divisor)) {
			complex_num.divide(&divisor);
			complex_den.divide(&divisor);
		}
	}
}
bool Fraction::round() {
	Integer *remainder;
	if(!num.divide(&den, &remainder)) {
		remainder->setNegative(false);
		den.subtract(remainder);
		int comp = den.compare(remainder);
		if(comp >= 0) {
			if(num.isNegative()) num.add(-1);
			else num.add(1);
		}
	}
	den.set(1);
	delete remainder;
	return true;
}
bool Fraction::abs() {
	if(isComplex()) {
		num.multiply(&num);
		den.multiply(&den);
		complex_num.multiply(&complex_num);
		complex_den.multiply(&complex_den);
		
		num.multiply(&complex_den);
		complex_num.multiply(&den);		
		num.add(&complex_num);
		den.multiply(&complex_den);	
		
		complex_num.clear();
		complex_den.set(1);
		clean();
		
		sqrt();
	} else {
		num.setNegative(false);
	}
	return true;
}
bool Fraction::frac() {
	if(isComplex()) return false;
	Integer num_copy(&num);
	num_copy.divide(&den);
	Fraction fr(&num_copy);
	return subtract(&fr);
}
bool Fraction::floor() {
	if(isComplex()) return false;
	if(num.isNegative()) {
		if(!num.divide(&den)) {
			num.add(-1);
		}
		den.set(1);
	} else {
		trunc();
	}
	return true;
}
bool Fraction::ceil() {
	if(isComplex()) return false;
	if(num.isNegative()) {
		trunc();	
	} else {
		if(!num.divide(&den)) {
			num.add(1);
		}
		den.set(1);
	}
	return true;
}
bool Fraction::trunc() {
	if(isComplex()) return false;
	num.divide(&den);
	den.set(1);
	return true;
}
bool Fraction::mod() {
	if(isComplex()) return false;
#ifdef HAVE_LIBCLN
	cl_I cli = cln::mod(num.getCL_I(), den.getCL_I());
	num.set(cli); 
	den.set(1);
	return true;
#else	
	Fraction fr(this);
	fr.floor();
	Integer sub(fr.numerator());
	sub.multiply(&den);
	num.subtract(&sub);
	den.set(1);
	return true;
#endif
}
bool Fraction::rem() {
	if(isComplex()) return false;
#ifdef HAVE_LIBCLN
	cl_I cli = cln::rem(num.getCL_I(), den.getCL_I());
	num.set(cli); 
	den.set(1);
	return true;
#else	
	Fraction fr(this);
	fr.truncate();
	Integer sub(fr.numerator());
	sub.multiply(&den);
	num.subtract(&sub);
	den.set(1);
	return true;
#endif
}
void Fraction::pow_sub(Integer *exp) {
	if(exp->isZero()) {
		set(1);
	} else if(!exp->isOne()) {
		Fraction this_copy(this);	
		bool even = exp->isEven();
		exp->divide(2);
		pow_sub(exp);
		multiply(&this_copy);
		if(!even) {
			multiply(&this_copy);
		}
	}	
}
int Fraction::pow(const Fraction *fr, int solution) {
	if(isZero() && fr->isNegative()) {
		CALCULATOR->error(true, _("Division by zero."), NULL);
		return false;
	}
	if(fr->isComplex()) {
		return false;
	}
	if(isZero()) {
		if(fr->isZero()) {
			//0^0
			CALCULATOR->error(false, _("0^0 might be considered undefined"), NULL);
			return 1;
		}
		return 1;
	}
	if(!fr->isPrecise()) b_exact = false;
	if(fr->isZero()) {
		set(1);
		return 1;
	} 	
	bool b_minus = false, b_cplx = false;	
	if(fr->isOne()) return 1;
	if(isNegative()) {
		if(fr->denominator()->isEven()) {
			//CALCULATOR->error(true, _("An even root is undefined for negative numbers."), NULL);	
			//return false;
			num.setNegative(false);
			b_cplx = true;
		} else {
			num.setNegative(false);
			b_minus = true;
		}
	}
	if(!root(fr->denominator())) {
		if(b_minus || b_cplx) {
			num.setNegative(true);
		}
		return false;
	}
	if(b_cplx) {
		complex_num.set(&num);
		complex_den.set(&den);
		num.clear();
		den.set(1);
	}
	if(b_minus) {
		num.setNegative(true);
	}	
	Integer exp(fr->numerator());
	if(exp.isNegative()) {
		exp.setNegative(false);
		Integer d(&den);
		den.set(&num);
		num.set(&d);
	}
	if(isComplex()) {
		pow_sub(&exp);
	} else {
		num.pow(&exp);
		den.pow(&exp);
	}
	clean();
	if(!isComplex() && fr->denominator()->isEven() && !fr->numerator()->isEven()) {
		if(solution == 2) {
			setNegative(true);
		}
		if(CALCULATOR->multipleRootsEnabled()) {
			return 2;
		}
	}
	return 1;	
}
bool Fraction::root(const Integer *nth) {
	if(nth->isOne()) return true;
#ifdef HAVE_LIBCLN
	if(!isComplex()) {
		cl_RA clfr = num.getCL_I() / den.getCL_I();
		cl_RA clns;
		if(rootp(clfr, nth->getCL_I(), &clns)) {
			num.set(cln::numerator(clns));
			den.set(cln::denominator(clns));	
			return true;
		}
		if(CALCULATOR->alwaysExact()) return false;
		b_exact = false;
		cl_RA cl_y = cln::recip(nth->getCL_I());	
		cl_R clr = cln::exp(cl_y * cln::ln(clfr));
		clfr = cln::rational(clr);
		num.set(cln::numerator(clfr));
		den.set(cln::denominator(clfr));
		return true;
	} else {
		if(CALCULATOR->alwaysExact()) return false;
		cl_RA clfr = num.getCL_I() / den.getCL_I();
		cl_RA clfr_i = complex_num.getCL_I() / complex_den.getCL_I();
		cl_N clc = cln::complex(clfr, clfr_i);
		b_exact = false;
		cl_RA cl_y = cln::recip(nth->getCL_I());	
		cl_N clr = cln::exp(cl_y * cln::log(clc));
		clfr = cln::rational(cln::realpart(clr));
		clfr_i = cln::rational(cln::imagpart(clr));
		num.set(cln::numerator(clfr));
		den.set(cln::denominator(clfr));
		complex_num.set(cln::numerator(clfr_i));
		complex_den.set(cln::denominator(clfr_i));
		return true;
	}
#else
	if(isComplex()) {
		return false;
	}
	Fraction nth_fr(NULL, nth);
	Fraction *x = this;
	Fraction a(this);
	set(d2s(powl(value(), nth_fr.value()), 8));	
	Fraction n(nth);
	Fraction n_m1(&n);
	Fraction one(1);
	n_m1.add(OPERATION_SUBTRACT, &one);
	int iter = CALCULATOR->getPrecision();
	if(iter > 100) iter = 10;	
	else if(iter > 40) iter = 7;
	else if(iter > 20) iter = 5;
	else if(iter > 8) iter = 3;
	else iter = 1;
	for(int i = 0; i <= iter; i++) {
		Fraction tmp(x);
		tmp.add(OPERATION_RAISE, &n);
		tmp.add(OPERATION_SUBTRACT, &a);		
		Fraction tmp2(x);
		tmp2.add(OPERATION_RAISE, &n_m1);
		tmp2.add(OPERATION_MULTIPLY, &n);
		tmp.add(OPERATION_DIVIDE, &tmp2);
		x->add(OPERATION_SUBTRACT, &tmp);
		floatify(CALCULATOR->getPrecision() + 5);
		clean();		
	}
	floatify(CALCULATOR->getPrecision() + 5);
	clean();
	Fraction test(this);
	Fraction nth_pow(nth);
	test.pow(&nth_pow);
	if(!test.equals(&a)) {
		if(CALCULATOR->alwaysExact()) {
			set(&a);
			return false;
		}
		b_exact = false;	
	}
	return true;	
#endif		
}
bool Fraction::root(long int nth) {
	Integer n(nth);
	return root(&n);
}

bool Fraction::floatify(int precision, int max_decimals, bool *infinite_series) {
	//if log10=integer do nothing
	Integer *remainder = NULL, *remainder2 = NULL;
	Fraction exp_test(this);
	Integer d(&den);
	den.set(1);
	bool exact = num.divide(&d, &remainder);
	vector<Integer*> remainders;
	if(infinite_series) {
		*infinite_series = false;
	}
	Integer exp;
	if((!exact && max_decimals) || !isPrecise()) {
		bool b_always_exact = CALCULATOR->alwaysExact();
		CALCULATOR->setAlwaysExact(false);
		exp_test.setNegative(false);
		Fraction fr10(10);
		exp_test.log(&fr10, false);
		exp_test.floor();
		exp.set(exp_test.numerator());
/*		exp_test.floor();
		Integer expdiv(exp_test.numerator());
		expdiv.div10();
		while(!expdiv.isZero()) {
			exp.add(1);
			expdiv.div10();
		}*/
		exp.add(1);
		if(exp.isGreaterThan(precision)) {
			exp.subtract(precision);
			precision = 0;
			exp.add(-1);
			Integer exp10(10);
			exp10.pow(&exp);
			exact = num.divide(&exp10);			
			if(remainder) delete remainder;	
			exact = num.divide(10, &remainder) && exact;
			remainder->multiply(10);
			remainder->setNegative(false);
			if(remainder->compare(5) < 1) {
				if(num.isNegative()) {
					num.add(-1);
				} else {
					num.add(1);
				}
			}		
			exp.add(1);	
			num.exp10(&exp);	
			delete remainder;
			return exact;
		} else {
			precision -= exp.getInt();
			exp.clear();
		}
		CALCULATOR->setAlwaysExact(b_always_exact);
	}
	if(max_decimals >= 0 && max_decimals < precision) {
		precision = max_decimals;
	}
	while(!exact && precision) {
		if(infinite_series && !(*infinite_series)) {
			remainders.push_back(new Integer(remainder));
		}
		remainder->multiply(10);
		exact = remainder->divide(&d, &remainder2);
		num.multiply(10);	
		num.add(remainder);
		den.multiply(10);
		delete remainder;
		remainder = remainder2;
		if(!exact && infinite_series && !(*infinite_series)) {
			for(unsigned int i = 0; i < remainders.size(); i++) {
				if(remainders[i]->equals(remainder)) {
					*infinite_series = true;
					break;
				}
			}
		}
		precision--;
	}
	for(unsigned int i = 0; i < remainders.size(); i++) {
		delete remainders[i];
	}
	if(!exact && !(infinite_series && *infinite_series)) {
		remainder->multiply(10);
		remainder->divide(&d, &remainder2);
		remainder->setNegative(false);
		int comp = remainder->compare(5);
		if(comp <= 0) {
			if(num.isNegative()) num.add(-1);
			else num.add(1);			
		}
	}
	if(!exact) b_exact = false;
	if(remainder2) delete remainder2;
	return exact;	
}

string Fraction::print(NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Integer *l_exp, bool in_composite, bool in_power) const {

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
//void Fraction::getPrintObjects(bool &minus, string &whole_, string &numerator_, string &denominator_, bool &exp_minus, string &exponent_, bool &cminus, string &cwhole_, string &cnumerator_, string &cdenominator_, bool &cexp_minus, string &cexponent_, Prefix **prefix1, Prefix **prefix2, NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Integer *l_exp, Integer *l_exp2, bool in_composite, bool in_power) const {
void Fraction::getPrintObjects(bool &minus, string &whole_, string &numerator_, string &denominator_, bool &exp_minus, string &exponent_, string &prefix_, NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Integer *l_exp, bool in_composite, bool in_power, Integer *l_exp2, Prefix **prefix1, Prefix **prefix2) const {
	if(CALCULATOR->alwaysExact()) max_decimals = -1;
	if(in_exact && !isPrecise()) *in_exact = true;
	Integer exp;
	if(nrformat != NUMBER_FORMAT_DECIMALS && !isZero()) {
		if((!(displayflags & DISPLAY_FORMAT_FRACTION) && !(displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY))) {
			if(!isZero()) {
				Fraction exp_pre(this);
				exp_pre.setNegative(false);
				bool b_always_exact = CALCULATOR->alwaysExact();
				CALCULATOR->setAlwaysExact(false);	
/*				exp_pre.floor();
				Integer expdiv(exp_pre.numerator());
				expdiv.div10();
				while(!expdiv.isZero()) {
					exp.add(1);
					expdiv.div10();
				}*/
				Fraction fr10(10);
				if(exp_pre.log(&fr10, false)) {
					exp_pre.floor();
					exp.set(exp_pre.numerator());
				}
				CALCULATOR->setAlwaysExact(b_always_exact);
			}
		} else {
			if(num.mod10()) {
				Integer num_test(&num);				
				while(num_test.div10()) {	
					exp.add(1);		
				}		
			} else {
				Integer den_test(&den);
				while(den_test.div10()) {	
					exp.add(-1);
				}
			}
		}
	}
	Integer exp_spec(&exp);

	minus = isNegative();
	whole_ = "";
	numerator_ = "";
	denominator_ = "";
	exponent_ = "";
	exp_minus = false;
	prefix_ = "";	
	bool force_fractional = false;
	int base = 10;
	if(displayflags & DISPLAY_FORMAT_USE_PREFIXES) {
		Prefix *p;
		if(l_exp) {
			Integer tmp_exp;
			if(prefix) {
				exp_spec.subtract(prefix->exponent(l_exp, &tmp_exp));
				prefix_ = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
				if(prefix1) *prefix1 = prefix;
			} else {
				tmp_exp.set(&exp);
				if(l_exp2 && prefix2 && CALCULATOR->denominatorPrefixEnabled()) {	
					tmp_exp.divide(l_exp);
					tmp_exp.setNegative(false);
					if(tmp_exp.isGreaterThan(5)) {
						tmp_exp.set(&exp);
						tmp_exp.divide(2);
						if(!tmp_exp.isEven()) {
							if(tmp_exp.isNegative()) tmp_exp.add(-1);
							else tmp_exp.add(1);
						}
					} else {
						tmp_exp.set(&exp);
					}
				}
				p = CALCULATOR->getBestPrefix(&tmp_exp, l_exp);
				Integer test_exp(&exp);
				test_exp.subtract(p->exponent(l_exp, &tmp_exp));
				if((exp.isPositive() && exp.compare(&test_exp) == -1) || (exp.isNegative() && exp.compare(&test_exp) == 1)) {
					exp_spec.set(&test_exp);
					prefix_ = p->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
					if(prefix1) *prefix1 = p;
				}
			}
			if((displayflags & DISPLAY_FORMAT_SHORT_UNITS) && (displayflags & DISPLAY_FORMAT_NONASCII)) {
				gsub("micro", SIGN_MICRO, prefix_);
			}
			if(l_exp2 && prefix2 && CALCULATOR->denominatorPrefixEnabled()) {
				l_exp2->setNegative(!l_exp2->isNegative());
				p = CALCULATOR->getBestPrefix(&exp_spec, l_exp2);
				Integer test_exp(&exp_spec);
				test_exp.subtract(p->exponent(l_exp2, &tmp_exp));
				if((exp_spec.isPositive() && exp_spec.compare(&test_exp) == -1) || (exp_spec.isNegative() && exp_spec.compare(&test_exp) == 1)) {
					exp_spec.set(&test_exp);
					if(prefix2) *prefix2 = p;
				}
			}
		} else if(l_exp2) {
			Integer tmp_exp;
			l_exp2->setNegative(true);
			if(prefix) {
				exp_spec.subtract(prefix->exponent(l_exp2, &tmp_exp));
				prefix_ = prefix->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
				if(prefix2) *prefix2 = prefix;
			} else {
				p = CALCULATOR->getBestPrefix(&exp, l_exp2);
				Integer test_exp(&exp);
				test_exp.subtract(p->exponent(l_exp2, &tmp_exp));
				if((exp.isPositive() && exp.compare(&test_exp) == -1) || (exp.isNegative() && exp.compare(&test_exp) == 1)) {
					exp_spec.set(&test_exp);
					prefix_ = p->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
					if(prefix2) *prefix2 = p;
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
			force_fractional = true;
			break;			
		}
		case NUMBER_FORMAT_OCTAL: {
			base = 8;
			force_fractional = true;
			break;			
		}
		case NUMBER_FORMAT_BIN: {
			base = 2;
			force_fractional = true;
			break;
		}					
		case NUMBER_FORMAT_NORMAL: {
			if(exp_spec.isGreaterThan(-PRECISION) && exp_spec.isLessThan(PRECISION)) { 
				break;
			}
		}			
		case NUMBER_FORMAT_EXP: {
			if(exp_spec.isGreaterThan(-3) && exp_spec.isLessThan(3)) { 
				break;
			}		
		}
		case NUMBER_FORMAT_EXP_PURE: {
			if(!exp_spec.isZero()) {
				exponent_ = exp_spec.print(10, false);
				exp_spec.clear();
			}
			break;
		}		
	}
	Integer den_spec(&den);	
	Integer whole(&num);
	exp.subtract(&exp_spec);
	if(exp.isNegative()) {
		exp_minus = true;
		exp.setNegative(false);
		whole.exp10(&exp);	
	} else if(exp.isPositive()) {
		den_spec.exp10(&exp);
	}
/*	Integer divisor;
	if(whole.gcd(&den_spec, &divisor)) {
		whole.divide(&divisor);
		den_spec.divide(&divisor);
	}*/
	if(plural) {
		*plural = whole.isGreaterThan(&den_spec);
	}
	if(in_composite && whole.equals(&den_spec)) {
		return;
	} else if(!force_fractional && !(displayflags & DISPLAY_FORMAT_FRACTION) && !(displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY)) {
		Fraction fr(&whole, &den_spec, NULL, true);
		bool infinite_series = false;
		if(displayflags & DISPLAY_FORMAT_INDICATE_INFINITE_SERIES) {
			fr.floatify(PRECISION, max_decimals, &infinite_series);
		} else {
			fr.floatify(PRECISION, max_decimals);
		}
		if(!fr.isPrecise()) {
			if(isPrecise() && ((displayflags & DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT) || CALCULATOR->alwaysExact())) {
				displayflags = displayflags | DISPLAY_FORMAT_FRACTIONAL_ONLY;
				return getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags, min_decimals, max_decimals, prefix, in_exact, usable, toplevel, plural, l_exp, in_composite, in_power);
			}
		}
		if(in_exact && !fr.isPrecise() && !infinite_series) {
			*in_exact = true;
		}
		whole_ = fr.numerator()->print(10, false);
		int l10 = 0;
		Integer d(fr.denominator());
		while(d.div10()) {
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
		Integer *part;
		Integer whole_test(&whole);
		bool b = whole_test.divide(&den_spec, &part);	
		if((displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY) && !b) {
			part->set(&whole);
			whole.clear();
		} else {
			whole.set(&whole_test);
		}
		Integer divisor;
		if(part->gcd(&den_spec, &divisor)) {
			part->divide(&divisor);
			den_spec.divide(&divisor);
		}
		if(!whole.isZero()) {
			whole_ = whole.print(base, false);
		}
		if(!part->isZero()) {
			numerator_ = part->print(base, false);
			denominator_ = den_spec.print(base, false);
		}		
		delete part;
	}
	if(whole_.empty() && numerator_.empty() && denominator_.empty() && exponent_.empty() && prefix_.empty()) {
		whole_ = "0";
	}
}

