/*
    Qalculate    

    Copyright (C) 2004  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef NUMBER_H
#define NUMBER_H

class Number;

#include "includes.h"
#include "Calculator.h"

#define WANT_OBFUSCATING_OPERATORS
#include <cln/cln.h>


class Number {
	
	private:

	protected:
	
		cln::cl_N value;
		bool b_approx;

	public:
	
		Number();
		Number(string number);
		Number(long int numerator, long int denominator = 1, long int exp_10 = 0);
		Number(const Number *o);
		virtual ~Number();
		
		void set(string number);
		void set(long int numerator, long int denominator = 1, long int exp_10 = 0);
		void setFloat(double d_value);
		void setCln(cln::cl_N cln_value);
		void setImaginaryPart(const Number *o);
		void set(const Number *o);
		void clear();
		
		const cln::cl_N &clnNumber() const;
		
		double floatValue() const;
		int intValue(bool *overflow = NULL) const;
		long int longIntValue(bool *overflow = NULL) const;
		
		bool isApproximate() const;
		void setApproximate(bool is_approximate = true);
		
		Number *realPart() const;
		Number *imaginaryPart() const;
		Number *numerator() const;
		Number *denominator() const;
		Number *complexNumerator() const;
		Number *complexDenominator() const;
		
		bool hasRealPart() const;
		bool isComplex() const;
		bool isInteger() const;
		bool isFraction() const;
		bool isZero() const;
		bool isOne() const;
		bool isI() const;
		bool isMinusI() const;
		bool isMinusOne() const;
		bool isNegative() const;
		bool isPositive() const;
		bool equals(const Number *o) const;
		int compare(const Number *o) const;
		bool isGreaterThan(const Number *o) const;
		bool isLessThan(const Number *o) const;
		bool isGreaterThanOrEqualTo(const Number *o) const;
		bool isLessThanOrEqualTo(const Number *o) const;
		bool equals(long int num, long int den = 1) const;
		int compare(long int num, long int den = 1) const;
		bool isGreaterThan(long int num, long int den = 1) const;
		bool isLessThan(long int num, long int den = 1) const;
		bool isGreaterThanOrEqualTo(long int num, long int den = 1) const;
		bool isLessThanOrEqualTo(long int num, long int den = 1) const;
		bool isEven() const;
		bool isOdd() const;
		
		bool add(const Number *o);
		bool subtract(const Number *o);
		bool multiply(const Number *o);
		bool divide(const Number *o);
		bool recip();
		int raise(const Number *o, int solution = 1);
		bool exp10(const Number *o = NULL);
		bool exp2(const Number *o = NULL);
		bool square();
		
		bool add(long int num, long int den = 1);
		bool subtract(long int num, long int den = 1);
		bool multiply(long int num, long int den = 1);
		bool divide(long int num, long int den = 1);
		int raise(long int num, long int den = 1, int solution = 1);
		bool exp10(long int num, long int den = 1);
		bool exp2(long int num, long int den = 1);
		
		bool negate();
		void setNegative(bool is_negative);
		bool abs();
		bool round(const Number *o = NULL);
		bool floor(const Number *o = NULL);
		bool ceil(const Number *o = NULL);
		bool trunc(const Number *o = NULL);
		bool mod(const Number *o);	
		bool frac();		
		bool rem(const Number *o);

		int getBoolean();
		void toBoolean();
		void setTrue(bool is_true);
		void setFalse();
		void setNOT();
		
		void e();
		void pi();
		void catalan();
		void pythagoras();
		void euler();	
		void apery();
		void golden();
		bool zeta();			
		
		bool sin();
		bool asin();
		bool sinh();
		bool asinh();
		bool cos();
		bool acos();
		bool cosh();
		bool acosh();
		bool tan();
		bool atan();
		bool tanh();
		bool atanh();
		bool ln();	
		bool log(const Number *o);
		bool log(long int num, long int den = 1);
		bool exp();
		bool gcd(const Number *o);
		
		bool factorial();
		bool binomial(const Number *m, const Number *k);
	
		int add(MathOperation op, const Number *o, int solution = 1); 
		
		bool floatify(cln::cl_I *num, cln::cl_I *den, int precision = DEFAULT_PRECISION, int max_decimals = -1, bool *infinite_series = NULL);
		string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, Prefix *prefix = NULL, bool *in_exact = NULL, bool *usable = NULL, bool toplevel = true, bool *plural = NULL, Number *l_exp = NULL, bool in_composite = false, bool in_power = false) const;
		void getPrintObjects(bool &minus, string &whole_, string &numerator_, string &denominator_, bool &exp_minus, string &exponent_, string &prefix_, NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, Prefix *prefix = NULL, bool *in_exact = NULL, bool *usable = NULL, bool toplevel = true, bool *plural = NULL, Number *l_exp = NULL, bool in_composite = false, bool in_power = false, Number *l_exp2 = NULL, Prefix **prefix1 = NULL, Prefix **prefix2 = NULL) const;
		
		string printNumerator(unsigned int base = 10, bool display_sign = true) const;
		string printDenominator(unsigned int base = 10, bool display_sign = true) const;
		string printImaginaryNumerator(unsigned int base = 10, bool display_sign = true) const;
		string printImaginaryDenominator(unsigned int base = 10, bool display_sign = true) const;
	
};

#endif
