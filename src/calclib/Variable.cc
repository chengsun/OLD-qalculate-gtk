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
#include "Fraction.h"

Variable::Variable(string cat_, string name_, Manager *mngr_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, name_, title_, "", is_local, is_builtin, is_active) {
	mngr = mngr_;
	if(mngr) mngr->ref();
	setChanged(false);
}
Variable::Variable(string cat_, string name_, string expression_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, name_, title_, "", is_local, is_builtin, is_active) {
	mngr = NULL;
	set(expression_);
	setChanged(false);
}
Variable::Variable() : ExpressionItem() {
	mngr = NULL;
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
void Variable::set(string expression_) {
	if(mngr) mngr->unref();
	bool b_always_exact = CALCULATOR->alwaysExact();
	CALCULATOR->setAlwaysExact(true);
	mngr = CALCULATOR->calculate(expression_);
	CALCULATOR->setAlwaysExact(b_always_exact);
	setChanged(true);
}
Manager *Variable::get() {
	return mngr;
}
const Manager *Variable::get() const {
	return mngr;
}

DynamicVariable::DynamicVariable(string cat_, string name_, string title_, bool is_local, bool is_builtin, bool is_active) : Variable(cat_, name_, new Manager(), title_, is_local, is_builtin, is_active) {
	calculated_precision = 0;
	setPrecise(false);
	setChanged(false);
}
DynamicVariable::DynamicVariable(const DynamicVariable *variable) {
	set(variable);
	setPrecise(false);	
	setChanged(false);
}
DynamicVariable::DynamicVariable() : Variable() {
	mngr = new Manager();
	calculated_precision = 0;
	setPrecise(false);	
	setChanged(false);
}
DynamicVariable::~DynamicVariable() {}
void DynamicVariable::set(const ExpressionItem *item) {
	calculated_precision = 0;
	ExpressionItem::set(item);
}
void DynamicVariable::set(Manager *mngr_) {}
void DynamicVariable::set(string expression_) {}
Manager *DynamicVariable::get() {
	if(calculated_precision != CALCULATOR->getPrecision()) {
		calculate();
	}
	return Variable::get();
}
const Manager *DynamicVariable::get() const {
	if(calculated_precision != CALCULATOR->getPrecision() || !mngr) {
		calculate();
	}
	return Variable::get();
}
int DynamicVariable::calculatedPrecision() const {
	return calculated_precision;
}


PiVariable::PiVariable() : DynamicVariable("Constants", "pi", "Archimede's Constant (pi)") {}
void PiVariable::calculate() const {
	Fraction fr; fr.pi(); mngr->set(&fr);
}
EVariable::EVariable() : DynamicVariable("Constants", "e", "The Base of Natural Logarithms (e)") {}
void EVariable::calculate() const {
	Fraction fr; fr.e(); mngr->set(&fr);
}
PythagorasVariable::PythagorasVariable() : DynamicVariable("Constants", "pythagoras", "Pythagora's Constant (sqrt 2)") {}
void PythagorasVariable::calculate() const {
	Fraction fr; fr.pythagoras(); mngr->set(&fr);
}
EulerVariable::EulerVariable() : DynamicVariable("Constants", "euler", "Euler's Constant") {}
void EulerVariable::calculate() const {
	Fraction fr; fr.euler(); mngr->set(&fr);
}
GoldenVariable::GoldenVariable() : DynamicVariable("Constants", "golden", "The Golden Ratio") {}
void GoldenVariable::calculate() const {
	Fraction fr; fr.golden(); mngr->set(&fr);
}
AperyVariable::AperyVariable() : DynamicVariable("Constants", "apery", "Apery's Constant") {}
void AperyVariable::calculate() const {
	Fraction fr; fr.apery(); mngr->set(&fr);
}
CatalanVariable::CatalanVariable() : DynamicVariable("Constants", "catalan", "Catalan's Constant") {}
void CatalanVariable::calculate() const {
	Fraction fr; fr.catalan(); mngr->set(&fr);
}

