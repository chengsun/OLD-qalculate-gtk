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
#include "Number.h"

Variable::Variable(string cat_, string name_, Manager *mngr_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, name_, title_, "", is_local, is_builtin, is_active) {
	mngr = mngr_;
	if(mngr) mngr->ref();
	b_expression = false;
	sexpression = "";
	calculated_precision = 0;
	setChanged(false);
}
Variable::Variable(string cat_, string name_, string expression_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, name_, title_, "", is_local, is_builtin, is_active) {
	mngr = NULL;
	calculated_precision = 0;
	set(expression_);
	setChanged(false);
}
Variable::Variable() : ExpressionItem() {
	mngr = NULL;
}
Variable::Variable(const Variable *variable) {
	mngr = NULL;
	set(variable);
}
Variable::~Variable() {
	if(mngr) mngr->unref();
}
ExpressionItem *Variable::copy() const {
	return new Variable(this);
}
bool Variable::isExpression() const {
	return b_expression;
}
string Variable::expression() const {
	return sexpression;
}
void Variable::set(const ExpressionItem *item) {
	if(item->type() == TYPE_VARIABLE) {
		calculated_precision = 0;
		sexpression = ((Variable*) item)->expression();
		b_expression = ((Variable*) item)->isExpression();
		if(!b_expression) {
			set(((Variable*) item)->copyManager());
		}
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
	calculated_precision = 0;
	b_expression = false;
	sexpression = "";
	setChanged(true);
}
void Variable::set(string expression_) {
	b_expression = true;
	sexpression = expression_;
	remove_blank_ends(sexpression);
	calculated_precision = 0;
/*	if(mngr) mngr->unref();
	bool b_always_exact = CALCULATOR->alwaysExact();
	CALCULATOR->setAlwaysExact(true);
	mngr = CALCULATOR->calculate(expression_);
	CALCULATOR->setAlwaysExact(b_always_exact);*/
	setChanged(true);
}
Manager *Variable::get() {
	if(b_expression && (calculated_precision != CALCULATOR->getPrecision() || !mngr)) {
		calculated_precision = CALCULATOR->getPrecision();
		if(mngr) {
			mngr->unref();
		}
		mngr = CALCULATOR->calculate(sexpression);
	}
	return mngr;
}
Manager *Variable::copyManager() const {
	return new Manager(mngr);
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
//	calculated_precision = 0;
	ExpressionItem::set(item);
}
void DynamicVariable::set(Manager *mngr_) {}
void DynamicVariable::set(string expression_) {}
Manager *DynamicVariable::get() {
	if(calculated_precision != CALCULATOR->getPrecision()) {
		calculated_precision = CALCULATOR->getPrecision();
		calculate();
	}
	return mngr;
}
Manager *DynamicVariable::copyManager() const {
	if(calculated_precision != CALCULATOR->getPrecision()) {
		calculate();
	}
	return new Manager(mngr);
}
int DynamicVariable::calculatedPrecision() const {
	return calculated_precision;
}


PiVariable::PiVariable() : DynamicVariable("Constants", "pi", "Archimede's Constant (pi)") {}
void PiVariable::calculate() const {
	Number nr; nr.pi(); mngr->set(&nr);
}
EVariable::EVariable() : DynamicVariable("Constants", "e", "The Base of Natural Logarithms (e)") {}
void EVariable::calculate() const {
	Number nr; nr.e(); mngr->set(&nr);
}
PythagorasVariable::PythagorasVariable() : DynamicVariable("Constants", "pythagoras", "Pythagora's Constant (sqrt 2)") {}
void PythagorasVariable::calculate() const {
	Number nr; nr.pythagoras(); mngr->set(&nr);
}
EulerVariable::EulerVariable() : DynamicVariable("Constants", "euler", "Euler's Constant") {}
void EulerVariable::calculate() const {
	Number nr; nr.euler(); mngr->set(&nr);
}
GoldenVariable::GoldenVariable() : DynamicVariable("Constants", "golden", "The Golden Ratio") {}
void GoldenVariable::calculate() const {
	Number nr; nr.golden(); mngr->set(&nr);
}
AperyVariable::AperyVariable() : DynamicVariable("Constants", "apery", "Apery's Constant") {}
void AperyVariable::calculate() const {
	Number nr; nr.apery(); mngr->set(&nr);
}
CatalanVariable::CatalanVariable() : DynamicVariable("Constants", "catalan", "Catalan's Constant") {}
void CatalanVariable::calculate() const {
	Number nr; nr.catalan(); mngr->set(&nr);
}

