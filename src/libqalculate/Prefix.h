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

#include <libqalculate/includes.h>

class Prefix {
  protected:
	string l_name, s_name, u_name;
	int exp;
  public:
  	Prefix(int exp10, string long_name, string short_name = "", string unicode_name = "");
	~Prefix();
	const string &shortName(bool return_long_if_no_short = true, bool use_unicode = false) const;
	const string &longName(bool return_short_if_no_long = true, bool use_unicode = false) const;
	const string &unicodeName(bool return_short_if_no_uni = true) const;
	void setShortName(string short_name);
	void setLongName(string long_name);
	void setUnicodeName(string unicode_name);
	const string &name(bool short_default = true, bool use_unicode = false) const;
	int exponent(int iexp = 1) const;
	Number exponent(const Number &nexp) const;	
	void setExponent(int iexp);	
	Number value(const Number &nexp) const;
	Number value(int iexp = 1) const;
	
};

#endif
