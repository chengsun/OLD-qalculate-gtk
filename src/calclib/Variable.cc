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
#include "MathStructure.h"
#include "Number.h"

Assumptions::Assumptions() : number_type(ASSUMPTION_NUMBER_NONE), sign(ASSUMPTION_SIGN_UNKNOWN) {}
Assumptions::~Assumptions() {}

bool Assumptions::isPositive() {return sign == ASSUMPTION_SIGN_POSITIVE;}
bool Assumptions::isNegative() {return sign == ASSUMPTION_SIGN_NEGATIVE;}
bool Assumptions::isNonNegative() {return sign == ASSUMPTION_SIGN_NONNEGATIVE || sign == ASSUMPTION_SIGN_POSITIVE;}
bool Assumptions::isInteger() {return number_type <= ASSUMPTION_NUMBER_INTEGER;}
bool Assumptions::isNumber() {return number_type <= ASSUMPTION_NUMBER_NUMBER;}
bool Assumptions::isRational() {return number_type <= ASSUMPTION_NUMBER_RATIONAL;}
bool Assumptions::isReal() {return number_type <= ASSUMPTION_NUMBER_REAL;}
bool Assumptions::isNonZero() {return sign == ASSUMPTION_SIGN_NONZERO || sign == ASSUMPTION_SIGN_POSITIVE || sign == ASSUMPTION_SIGN_NEGATIVE;}


Variable::Variable(string cat_, string name_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, name_, title_, "", is_local, is_builtin, is_active) {
	setChanged(false);
}
Variable::Variable() : ExpressionItem() {}
Variable::Variable(const Variable *variable) {set(variable);}
Variable::~Variable() {}
void Variable::set(const ExpressionItem *item) {
	ExpressionItem::set(item);
}


UnknownVariable::UnknownVariable(string cat_, string name_, string title_, bool is_local, bool is_builtin, bool is_active) : Variable(cat_, name_, title_, is_local, is_builtin, is_active) {
	setChanged(false);
}
UnknownVariable::UnknownVariable() : Variable() {
}
UnknownVariable::UnknownVariable(const UnknownVariable *variable) {
	set(variable);
}
UnknownVariable::~UnknownVariable() {
	if(o_assumption) delete o_assumption;
}
ExpressionItem *UnknownVariable::copy() const {
	return new UnknownVariable(this);
}
void UnknownVariable::set(const ExpressionItem *item) {
	if(item->type() == TYPE_VARIABLE && !((Variable*) item)->isKnown()) {
	}
	ExpressionItem::set(item);
}
void UnknownVariable::setAssumptions(Assumptions *ass) {
	if(o_assumption) delete o_assumption;
	o_assumption = ass;
}
Assumptions *UnknownVariable::assumptions() {
	return o_assumption;
}
bool UnknownVariable::isPositive() {return o_assumption && o_assumption->isPositive();}
bool UnknownVariable::isNegative() {return o_assumption && o_assumption->isNegative();}
bool UnknownVariable::isNonNegative() {return o_assumption && o_assumption->isNonNegative();}
bool UnknownVariable::isInteger() {return o_assumption && o_assumption->isInteger();}
bool UnknownVariable::isNumber() {return o_assumption && o_assumption->isNumber();}
bool UnknownVariable::isRational() {return o_assumption && o_assumption->isRational();}
bool UnknownVariable::isReal() {return o_assumption && o_assumption->isReal();}
bool UnknownVariable::isNonZero() {return o_assumption && o_assumption->isNonZero();}

KnownVariable::KnownVariable(string cat_, string name_, const MathStructure &o, string title_, bool is_local, bool is_builtin, bool is_active) : Variable(cat_, name_, title_, is_local, is_builtin, is_active) {
	mstruct = new MathStructure(o);
	b_expression = false;
	sexpression = "";
	calculated_precision = 0;
	setChanged(false);
}
KnownVariable::KnownVariable(string cat_, string name_, string expression_, string title_, bool is_local, bool is_builtin, bool is_active) : Variable(cat_, name_, title_, is_local, is_builtin, is_active) {
	mstruct = NULL;
	calculated_precision = 0;
	set(expression_);
	setChanged(false);
}
KnownVariable::KnownVariable() : Variable() {
	mstruct = NULL;
}
KnownVariable::KnownVariable(const KnownVariable *variable) {
	mstruct = NULL;
	set(variable);
}
KnownVariable::~KnownVariable() {
	if(mstruct) delete mstruct;
}
ExpressionItem *KnownVariable::copy() const {
	return new KnownVariable(this);
}
bool KnownVariable::isExpression() const {
	return b_expression;
}
string KnownVariable::expression() const {
	return sexpression;
}
void KnownVariable::set(const ExpressionItem *item) {
	if(item->type() == TYPE_VARIABLE && ((Variable*) item)->isKnown()) {
		calculated_precision = 0;
		sexpression = ((KnownVariable*) item)->expression();
		b_expression = ((KnownVariable*) item)->isExpression();
		if(!b_expression) {
			set(((KnownVariable*) item)->get());
		}
	}
	ExpressionItem::set(item);
}
void KnownVariable::set(const MathStructure &o) {
	if(!mstruct) mstruct = new MathStructure(o);
	else mstruct->set(o);
	calculated_precision = 0;
	b_expression = false;
	sexpression = "";
	setChanged(true);
}
void KnownVariable::set(string expression_) {
	if(mstruct) delete mstruct;
	mstruct = NULL;
	b_expression = true;
	sexpression = expression_;
	remove_blank_ends(sexpression);
	calculated_precision = 0;
	setChanged(true);
}
const MathStructure &KnownVariable::get() {
	if(b_expression && !mstruct) {
		mstruct = new MathStructure(CALCULATOR->parse(sexpression));
	}
	return *mstruct;
}
bool KnownVariable::isPositive() {return get().representsPositive();}
bool KnownVariable::isNegative() {return get().representsNegative();}
bool KnownVariable::isNonNegative() {return get().representsNonNegative();}
bool KnownVariable::isInteger() {return get().representsInteger();}
bool KnownVariable::isNumber() {return get().representsNumber();}
bool KnownVariable::isRational() {return get().representsRational();}
bool KnownVariable::isReal() {return get().representsReal();}
bool KnownVariable::isNonZero() {return get().representsNonZero();}

DynamicVariable::DynamicVariable(string cat_, string name_, string title_, bool is_local, bool is_builtin, bool is_active) : KnownVariable(cat_, name_, MathStructure(), title_, is_local, is_builtin, is_active) {
	mstruct = NULL;
	calculated_precision = 0;
	setApproximate();
	setChanged(false);
}
DynamicVariable::DynamicVariable(const DynamicVariable *variable) {
	mstruct = NULL;
	set(variable);
	setApproximate();	
	setChanged(false);
}
DynamicVariable::DynamicVariable() : KnownVariable() {
	mstruct = NULL;
	calculated_precision = 0;
	setApproximate();	
	setChanged(false);
}
DynamicVariable::~DynamicVariable() {
	if(mstruct) delete mstruct;
}
void DynamicVariable::set(const ExpressionItem *item) {
	ExpressionItem::set(item);
}
void DynamicVariable::set(const MathStructure &o) {}
void DynamicVariable::set(string expression_) {}
const MathStructure &DynamicVariable::get() {
	if(calculated_precision != CALCULATOR->getPrecision() || !mstruct) {
		mstruct = new MathStructure();
		calculated_precision = CALCULATOR->getPrecision();
		calculate();
	}
	return *mstruct;
}
int DynamicVariable::calculatedPrecision() const {
	return calculated_precision;
}


PiVariable::PiVariable() : DynamicVariable("Constants", "pi", "Archimede's Constant (pi)") {}
void PiVariable::calculate() const {
	Number nr; nr.pi(); mstruct->set(nr);
}
EVariable::EVariable() : DynamicVariable("Constants", "e", "The Base of Natural Logarithms (e)") {}
void EVariable::calculate() const {
	Number nr; nr.e(); mstruct->set(nr);
}
EulerVariable::EulerVariable() : DynamicVariable("Constants", "euler", "Euler's Constant") {}
void EulerVariable::calculate() const {
	Number nr; nr.euler(); mstruct->set(nr);
}
CatalanVariable::CatalanVariable() : DynamicVariable("Constants", "catalan", "Catalan's Constant") {}
void CatalanVariable::calculate() const {
	Number nr; nr.catalan(); mstruct->set(nr);
}

