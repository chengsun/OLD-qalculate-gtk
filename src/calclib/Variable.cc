/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Variable.h"
#include "util.h"

Variable::Variable(Calculator *calc_, string cat_, string name_, Manager *mngr_, string title_, bool uservariable_) : buservariable(uservariable_) {
	calc = calc_;
	remove_blank_ends(name_);
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	sname = name_;
	stitle = title_;
	scat = cat_;
	mngr = mngr_;
	mngr->ref();
}
Variable::Variable(Calculator *calc_, string cat_, string name_, long double value_, string title_, bool uservariable_) : buservariable(uservariable_) {
	calc = calc_;
	remove_blank_ends(name_);
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	sname = name_;
	stitle = title_;
	scat = cat_;
	mngr = new Manager(calc, value_);
}
Variable::~Variable(void) {
	if(mngr) mngr->unref();
}
string Variable::title(void) {
	return stitle;
}
void Variable::title(string title_) {
	remove_blank_ends(title_);
	stitle = title_;
}
void Variable::set(Manager *mngr_) {
	if(mngr) mngr->unref();
	mngr = mngr_;
	mngr->ref();
}
Manager *Variable::get(void) {
	return mngr;
}
void Variable::name(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != sname) {
		sname = calc->getName(name_, (void*) this, force);
	}
	calc->variableNameChanged(this);
}
string Variable::name(void) {
	return sname;
}
void Variable::value(long double value_) {
	Manager *mngr_ = new Manager(calc, value_);
	set(mngr_);
	mngr_->unref();
}
string Variable::category(void) {
	return scat;
}
void Variable::category(string cat_) {
	remove_blank_ends(cat_);
	scat = cat_;
}
bool Variable::isUserVariable() {
	return buservariable;
}
