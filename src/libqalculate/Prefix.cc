/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "support.h"

#include "Prefix.h"
#include "Calculator.h"
#include "Number.h"

Prefix::Prefix(int exp10, string long_name, string short_name, string unicode_name) {
	exp = exp10;
	l_name = long_name;
	s_name = short_name;
	u_name = unicode_name;
}
Prefix::~Prefix() {
}
const string &Prefix::shortName(bool return_long_if_no_short, bool use_unicode) const {
	if(use_unicode && !u_name.empty()) return u_name;
	if(return_long_if_no_short && s_name.empty()) {
		return l_name;
	}
	return s_name;
}
const string &Prefix::longName(bool return_short_if_no_long, bool use_unicode) const {
	if(return_short_if_no_long && l_name.empty()) {
		if(use_unicode && !u_name.empty()) return u_name;
		return s_name;
	}
	return l_name;
}
const string &Prefix::unicodeName(bool return_short_if_no_unicode) const {
	if(return_short_if_no_unicode && u_name.empty()) {
		return s_name;
	}
	return u_name;
}
void Prefix::setShortName(string short_name) {
	s_name = short_name;
	CALCULATOR->prefixNameChanged(this);	
}
void Prefix::setLongName(string long_name) {
	l_name = long_name;
	CALCULATOR->prefixNameChanged(this);
}
void Prefix::setUnicodeName(string unicode_name) {
	u_name = unicode_name;
	CALCULATOR->prefixNameChanged(this);
}
const string &Prefix::name(bool short_default, bool use_unicode) const {
	if(short_default) {
		return shortName(true, use_unicode);
	}
	return longName(true, use_unicode);
}
int Prefix::exponent(int iexp) const {
	return exp * iexp;
}
Number Prefix::exponent(const Number &nexp) const {
	return nexp * exp;
}
void Prefix::setExponent(int iexp) {
	exp = exp;
}
Number Prefix::value(const Number &nexp) const {
	Number nr(exponent(nexp));
	nr.exp10();
	return nr;
}
Number Prefix::value(int iexp) const {
	Number nr(exponent(iexp));
	nr.exp10();
	return nr;
}
