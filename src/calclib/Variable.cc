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

Variable::Variable(string cat_, string name_, Manager *mngr_, string title_, bool uservariable_, bool is_builtin) : b_user(uservariable_), b_builtin(is_builtin) {
	remove_blank_ends(name_);
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	sname = name_;
	stitle = title_;
	scat = cat_;
	mngr = mngr_;
	mngr->ref();
	b_changed = false;
}
Variable::Variable(string cat_, string name_, long double value_, string title_, bool uservariable_, bool is_builtin) : b_user(uservariable_), b_builtin(is_builtin) {
	remove_blank_ends(name_);
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	sname = name_;
	stitle = title_;
	scat = cat_;
	mngr = new Manager(value_);
	b_changed = false;
}
Variable::~Variable(void) {
	if(mngr) mngr->unref();
}
string Variable::title(bool return_name_if_no_title) const {
	if(return_name_if_no_title && stitle.empty()) {
		return name();
	}
	return stitle;
}
void Variable::setTitle(string title_) {
	remove_blank_ends(title_);
	stitle = title_;
	b_changed = true;
}
void Variable::set(Manager *mngr_) {
	if(mngr) mngr->unref();
	mngr = mngr_;
	mngr->ref();
	b_changed = true;
}
Manager *Variable::get(void) {
	return mngr;
}
const Manager *Variable::get(void) const {
	return mngr;
}
void Variable::setName(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != sname) {
		sname = CALCULATOR->getName(name_, (void*) this, force);
		b_changed = true;
	}
	CALCULATOR->variableNameChanged(this);
}
string Variable::name(void) const {
	return sname;
}
void Variable::setValue(long double value_) {
	Manager *mngr_ = new Manager(value_);
	set(mngr_);
	mngr_->unref();
	b_changed = true;
}
string Variable::category(void) const {
	return scat;
}
void Variable::setCategory(string cat_) {
	remove_blank_ends(cat_);
	scat = cat_;
}
bool Variable::isUserVariable() const {
	return b_user;
}
bool Variable::isBuiltinVariable() const {
	return b_builtin;
}
bool Variable::hasChanged() const {
	return b_changed;
}
bool Variable::setUserVariable(bool is_user_var) {
	b_user = is_user_var;
}
bool Variable::setChanged(bool has_changed) {
	b_changed = has_changed;
}
bool Variable::isPrecise() const {
	return b_exact;
}
void Variable::setPrecise(bool is_precise) {
	b_exact = is_precise;
}

