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

class Fraction {
  protected:
	long long int num, den, exp;
	char vbuffer[1001];
  public:
	Fraction(long long int numerator_ = 1, long long int denominator_ = 1, long long int exp10_ = 0);
	Fraction(Fraction *fr);
	Fraction(string str);
	~Fraction();
	void set(long long int numerator_ = 1, long long int denominator_ = 1, long long int exp10_ = 0);
	bool set(string str);	
	bool equals(Fraction *fr);
	long long int numerator() const;
	long long int denominator() const;
	long long int exponent() const;
	long double value() const;
	bool isNegative() const;
	bool isInteger() const;
	long long int getInteger(bool &overflow) const;	
	bool isZero() const;
	bool isOne() const;	
	bool isMinusOne() const;	
	bool add(MathOperation op, Fraction *fr); 
	string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int precision = PRECISION, int min_decimals = 0, int max_decimals = -1, Prefix *prefix = NULL, bool *usable = NULL, bool toplevel = true, bool *plural = NULL, long int *l_exp = NULL, bool in_composite = false, bool in_power = false);
};

#endif
