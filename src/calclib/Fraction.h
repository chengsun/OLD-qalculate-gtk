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

#include "includes.h"
#include "Integer.h"

class Fraction {
  protected:
	Integer num, den;
	bool b_exact;
  public:
  	Fraction();
	Fraction(long int numerator_, long int denominator_ = 1, long int exp10_ = 0, bool nogcd = false);
	Fraction(const Integer *numerator_, const Integer *denominator_ = NULL, const Integer *exp10_ = NULL, bool nogcd = false);	
	Fraction(const Fraction *fr);
	Fraction(string str);
	~Fraction();
	void set(long int numerator_, long int denominator_ = 1, long int exp10_ = 0, bool nogcd = false);
	void setFloat(long double value_);		
	void set(const Integer *numerator_ = NULL, const Integer *denominator_ = NULL, const Integer *exp10_ = NULL, bool nogcd = false);	
	void set(const Fraction *fr);
	bool set(string str);	
	void e();
	void pi();
	void catalan();
	void pythagoras();
	void euler();	
	void apery();
	void golden();
#ifdef HAVE_LIBCLN			
	bool zeta();			
#endif	
	void clear();
	bool equals(const Fraction *fr) const;
	int compare(const Fraction *fr) const;
	bool isGreaterThan(const Fraction *fr) const; 	
	bool isLessThan(const Fraction *fr) const;
	bool isPlural() const;
	const Integer *numerator() const;
	const Integer *denominator() const;
	long double value() const;
	bool isPositive() const;
	bool isNegative() const;
	void setNegative(bool is_negative);	
	bool isInteger() const;
	bool isPrecise() const;
	void setPrecise(bool is_precise);
	Integer *getInteger() const;
	bool add(const Fraction *fr);		
	bool subtract(const Fraction *fr);			
	bool multiply(const Fraction *fr);			
	bool divide(const Fraction *fr);
	bool gcd(const Fraction *fr);
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
	int sqrt(int solution = 1);	
	bool cbrt();	
	bool log();	
	bool log(Fraction *fr, bool tryexact = true);		
	bool log2();	
	bool log10();	
	bool exp();
	bool exp2();
	bool exp10(const Fraction *fr = NULL);			
	void clean();
	bool round();
	bool abs();
	bool floor();
	bool ceil();
	bool trunc();
	bool mod();	
	bool frac();		
	bool rem();
	int pow(const Fraction *fr, int solution = 1);
	bool root(const Integer *nth);	
	bool root(long int nth = 2);
	bool floatify(int precision = DEFAULT_PRECISION, int max_decimals = -1, bool *infinite_series = NULL);
	bool isZero() const;
	bool isOne() const;	
	bool isMinusOne() const;	
	int add(MathOperation op, const Fraction *fr, int solution = 1); 
	int getBoolean();
	void toBoolean();
	void setTrue(bool is_true);
	void setFalse();
	void setNOT();
	string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, Prefix *prefix = NULL, bool *in_exact = NULL, bool *usable = NULL, bool toplevel = true, bool *plural = NULL, Integer *l_exp = NULL, bool in_composite = false, bool in_power = false) const;
	void getPrintObjects(bool &minus, string &whole_, string &numerator_, string &denominator_, bool &exp_minus, string &exponent_, string &prefix_, NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, Prefix *prefix = NULL, bool *in_exact = NULL, bool *usable = NULL, bool toplevel = true, bool *plural = NULL, Integer *l_exp = NULL, bool in_composite = false, bool in_power = false, Integer *l_exp2 = NULL, Prefix **prefix1 = NULL, Prefix **prefix2 = NULL) const;	
};

#endif
