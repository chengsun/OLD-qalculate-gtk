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
#include "Calculator.h"
#include "Manager.h"

Variable::Variable(string cat_, string name_, Manager *mngr_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, name_, title_, "", is_local, is_builtin, is_active) {
	mngr = mngr_;
	mngr->ref();
}
Variable::Variable(const Variable *variable) {
	set(variable);
}
Variable::~Variable() {
	if(mngr) mngr->unref();
}
ExpressionItem *Variable::copy() const {
	return new Variable(this);
}
void Variable::set(const ExpressionItem *item) {
	if(item->type() == TYPE_VARIABLE) {
		set(new Manager(((Variable*) item)->get()));
	}
	ExpressionItem::set(item);
}
int Variable::type() const {
	return TYPE_VARIABLE;
}
void Variable::set(Manager *mngr_) {
	if(mngr) mngr->unref();
	mngr = mngr_;
	mngr->ref();
	setChanged(true);
}
Manager *Variable::get() {
	return mngr;
}
const Manager *Variable::get() const {
	return mngr;
}
