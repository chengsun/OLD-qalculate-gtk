/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Prefix.h"
#include "Calculator.h"
#include "Number.h"

Prefix::Prefix(int exp10, string long_name, string short_name) {
	exp = exp10;
	l_name = long_name;
	s_name = short_name;
}
Prefix::~Prefix() {
}
const string &Prefix::shortName(bool return_long_if_no_short) const {
	if(return_long_if_no_short && s_name.empty()) {
		return l_name;
	}
	return s_name;
}
const string &Prefix::longName(bool return_short_if_no_long) const {
	if(return_short_if_no_long && l_name.empty()) {
		return s_name;
	}
	return l_name;
}
void Prefix::setShortName(string short_name) {
	s_name = short_name;
	CALCULATOR->prefixNameChanged(this);	
}
void Prefix::setLongName(string long_name) {
	l_name = long_name;
	CALCULATOR->prefixNameChanged(this);
}
const string &Prefix::name(bool short_default) const {
	if((s_name.empty() && short_default) || (!l_name.empty() && !short_default)) return l_name;
	return s_name;
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
