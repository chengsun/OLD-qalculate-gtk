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

#include "includes.h"
#include "Calculator.h"

#define WANT_OBFUSCATING_OPERATORS
#include <cln/cln.h>

class Number {
	
	private:

	protected:
	
		void removeFloatZeroPart();

		cln::cl_N value;
		bool b_inf, b_pinf, b_minf;

	public:
	
		Number();
		Number(string number, int base = 10);
		Number(int numerator, int denominator = 1, int exp_10 = 0);
		Number(const Number &o);
		virtual ~Number();
		
		void set(string number, int base = 10);
		void set(int numerator, int denominator = 1, int exp_10 = 0);
		void setInfinity();
		void setPlusInfinity();
		void setMinusInfinity();
		void setFloat(double d_value);

		void setInternal(const cln::cl_N &cln_value);

		void setImaginaryPart(const Number &o);
		void setImaginaryPart(int numerator, int denominator = 1, int exp_10 = 0);
		void set(const Number &o);
		void clear();

		const cln::cl_N &internalNumber() const;
		
		double floatValue() const;
		int intValue(bool *overflow = NULL) const;
		
		bool isApproximate() const;
		void setApproximate(bool is_approximate = true);
		
		bool isUndefined() const;
		bool isInfinite() const;
		bool isInfinity() const;
		bool isPlusInfinity() const;
		bool isMinusInfinity() const;
		
		Number realPart() const;
		Number imaginaryPart() const;
		Number numerator() const;
		Number denominator() const;
		Number complexNumerator() const;
		Number complexDenominator() const;

		void operator = (const Number &o);
		Number operator - () const;
		Number operator * (const Number &o) const;
		Number operator / (const Number &o) const;
		Number operator + (const Number &o) const;
		Number operator - (const Number &o) const;
		Number operator ^ (const Number &o) const;
		Number operator && (const Number &o) const;
		Number operator || (const Number &o) const;
		Number operator ! () const;
		
		void operator *= (const Number &o);
		void operator /= (const Number &o);
		void operator += (const Number &o);
		void operator -= (const Number &o);
		void operator ^= (const Number &o);
		
		bool operator == (const Number &o) const;
		bool operator != (const Number &o) const;
		
		bool hasRealPart() const;
		bool hasImaginaryPart() const;
		bool isComplex() const;
		bool isInteger() const;
		bool isRational() const;
		bool isReal() const;
		bool isFraction() const;
		bool isZero() const;
		bool isOne() const;
		bool isI() const;
		bool isMinusI() const;
		bool isMinusOne() const;
		bool isNegative() const;
		bool isNonNegative() const;
		bool isPositive() const;
		bool realPartIsNegative() const;
		bool realPartIsPositive() const;
		bool imaginaryPartIsNegative() const;
		bool imaginaryPartIsPositive() const;
		bool hasNegativeSign() const;
		bool hasPositiveSign() const;
		bool equals(const Number &o) const;
		int compare(const Number &o) const;
		int compareImaginaryParts(const Number &o) const;
		int compareRealParts(const Number &o) const;
		bool isGreaterThan(const Number &o) const;
		bool isLessThan(const Number &o) const;
		bool isGreaterThanOrEqualTo(const Number &o) const;
		bool isLessThanOrEqualTo(const Number &o) const;
		bool isEven() const;
		bool denominatorIsEven() const;
		bool numeratorIsEven() const;
		bool isOdd() const;
		
		bool add(const Number &o);
		bool subtract(const Number &o);
		bool multiply(const Number &o);
		bool divide(const Number &o);
		bool recip();
		bool raise(const Number &o, int solution = 1);
		bool exp10(const Number &o);
		bool exp2(const Number &o);
		bool exp10();
		bool exp2();
		bool square();
		
		bool negate();
		void setNegative(bool is_negative);
		bool abs();
		bool signum();
		bool round(const Number &o);
		bool floor(const Number &o);
		bool ceil(const Number &o);
		bool trunc(const Number &o);
		bool mod(const Number &o);
		bool round();
		bool floor();
		bool ceil();
		bool trunc();	
		bool frac();		
		bool rem(const Number &o);

		int getBoolean() const;
		void toBoolean();
		void setTrue(bool is_true);
		void setFalse();
		void setNOT();
		
		void e();
		void pi();
		void catalan();
		void euler();	
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
		bool log(const Number &o);
		bool exp();
		bool gcd(const Number &o);
		
		bool factorial();
		bool binomial(const Number &m, const Number &k);
	
		bool add(const Number &o, MathOperation op); 

		string printNumerator(int base = 10, bool display_sign = true) const;
		string printDenominator(int base = 10, bool display_sign = true) const;
		string printImaginaryNumerator(int base = 10, bool display_sign = true) const;
		string printImaginaryDenominator(int base = 10, bool display_sign = true) const;

		string print(const PrintOptions &po = default_print_options, const InternalPrintStruct &ips = top_ips) const;
	
};

#endif
