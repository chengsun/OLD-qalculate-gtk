/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Prefix.h"

Prefix::Prefix(long int exp10, string long_name, string short_name) {
	exp = exp10;
	l_name = long_name;
	s_name = short_name;
}
Prefix::~Prefix() {
}
const string &Prefix::shortName() const {
	return s_name;
}
const string &Prefix::longName() const {
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
long int Prefix::exponent(long int exp_) const {
	return exp * exp_;
}
void Prefix::setExponent(long int exp_) {
	exp = exp_;
}
long double Prefix::value(long int exp_) const {
	return exp10l(exponent(exp_));
}
