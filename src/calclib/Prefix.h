/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef PREFIX_H
#define PREFIX_H

class Prefix;

#include "includes.h"
#include "Calculator.h"

class Prefix {
  protected:
	string l_name, s_name;
	long int exp;
  public:
  	Prefix(long int exp10, string long_name, string short_name = "");
	~Prefix();
	const string &shortName() const;
	const string &longName() const;
	void setShortName(string short_name);
	void setLongName(string long_name);
	const string &name(bool short_default = true) const;
	long int exponent(long int exp_ = 1) const;
	void setExponent(long int exp_);	
	long double value(long int exp_ = 1) const;
};

#endif
