/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef FRACTION_H
#define FRACTION_H

class Fraction;

#include "includes.h"
#include "Prefix.h"
#include "Integer.h"

class Fraction {
  protected:
	Integer num, den;
	char vbuffer[1001];
	bool b_exact;
  public:
  	Fraction();
	Fraction(long int numerator_, long int denominator_ = 1, long int exp10_ = 0);
	Fraction(const Integer *numerator_, const Integer *denominator_ = NULL, const Integer *exp10_ = NULL);	
	Fraction(const Fraction *fr);
	Fraction(string str);
	~Fraction();
	void set(long int numerator_, long int denominator_ = 1, long int exp10_ = 0);
	void setFloat(long double value_);		
	void set(const Integer *numerator_ = NULL, const Integer *denominator_ = NULL, const Integer *exp10_ = NULL);	
	void set(const Fraction *fr);
	bool set(string str);	
	void clear();
	bool equals(const Fraction *fr) const;
	int compare(const Fraction *fr) const;
	const Integer *numerator() const;
	const Integer *denominator() const;
	long double value() const;
	bool isNegative() const;
	void setNegative(bool is_negative);	
	bool isInteger() const;
	bool isPrecise() const;
	void setPrecise(bool is_precise);
	Integer *getInteger() const;
	void add(const Fraction *fr);		
	void subtract(const Fraction *fr);			
	void multiply(const Fraction *fr);			
	void divide(const Fraction *fr);
	void gcd(const Fraction *fr);
	void sin();	
	void asin();	
	void sinh();	
	void asinh();
	void cos();	
	void acos();	
	void cosh();	
	void acosh();
	void tan();	
	void atan();	
	void tanh();	
	void atanh();
	void sqrt();	
	void cbrt();	
	void log();	
	void log2();	
	void log10();	
	void exp();
	void exp2();
	void exp10(const Fraction *fr = NULL);			
	void clean();
	void round();
	void abs();
	void floor();
	void ceil();
	void trunc();
	void mod();	
	void rem();
	void pow(const Fraction *fr);
	void root(const Integer *nth);	
	void root(long int nth = 2);
	bool floatify(int precision = DEFAULT_PRECISION);
	bool isZero() const;
	bool isOne() const;	
	bool isMinusOne() const;	
	bool add(MathOperation op, const Fraction *fr); 
	string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, Prefix *prefix = NULL, bool *in_exact = NULL, bool *usable = NULL, bool toplevel = true, bool *plural = NULL, long int *l_exp = NULL, bool in_composite = false, bool in_power = false) const;
};

#endif
