/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "support.h"

#include "Variable.h"
#include "util.h"
#include "Calculator.h"
#include "MathStructure.h"
#include "Number.h"

Assumptions::Assumptions() : i_type(ASSUMPTION_NUMBER_NONE), i_sign(ASSUMPTION_SIGN_UNKNOWN), fmin(NULL), fmax(NULL), b_incl_min(true), b_incl_max(true) {}
Assumptions::~Assumptions() {}

bool Assumptions::isPositive() {return i_sign == ASSUMPTION_SIGN_POSITIVE || (fmin && (fmin->isPositive() || (!b_incl_min && fmin->isNonNegative())));}
bool Assumptions::isNegative() {return i_sign == ASSUMPTION_SIGN_NEGATIVE || (fmax && (fmax->isNegative() || (!b_incl_max && fmax->isNonPositive())));}
bool Assumptions::isNonNegative() {return i_sign == ASSUMPTION_SIGN_NONNEGATIVE || i_sign == ASSUMPTION_SIGN_POSITIVE || (fmin && fmin->isNonNegative());}
bool Assumptions::isNonPositive() {return i_sign == ASSUMPTION_SIGN_NONPOSITIVE || i_sign == ASSUMPTION_SIGN_NEGATIVE || (fmax && fmax->isNonPositive());}
bool Assumptions::isInteger() {return i_type >= ASSUMPTION_NUMBER_INTEGER;}
bool Assumptions::isNumber() {return i_type >= ASSUMPTION_NUMBER_NUMBER || fmin || fmax || isPositive() || isNegative();}
bool Assumptions::isRational() {return i_type >= ASSUMPTION_NUMBER_RATIONAL;}
bool Assumptions::isReal() {return i_type >= ASSUMPTION_NUMBER_REAL || isPositive() || isNegative();}
bool Assumptions::isComplex() {return i_type == ASSUMPTION_NUMBER_COMPLEX;}
bool Assumptions::isNonZero() {return i_sign == ASSUMPTION_SIGN_NONZERO || isPositive() || isNegative();}

AssumptionNumberType Assumptions::numberType() {return i_type;}
AssumptionSign Assumptions::sign() {return i_sign;}
void Assumptions::setNumberType(AssumptionNumberType ant) {i_type = ant;}
void Assumptions::setSign(AssumptionSign as) {i_sign = as;}
	
void Assumptions::setMin(const Number *nmin) {
	if(!nmin) {
		if(fmin) {
			delete fmin;
		}
		return;
	}
	if(!fmin) {
		fmin = new Number(*nmin);
	} else {
		fmin->set(*nmin);
	}
}
void Assumptions::setIncludeEqualsMin(bool include_equals) {
	b_incl_min = include_equals;
}
bool Assumptions::includeEqualsMin() const {
	return b_incl_min;
}
const Number *Assumptions::min() const {
	return fmin;
}
void Assumptions::setMax(const Number *nmax) {
	if(!nmax) {
		if(fmax) {
			delete fmax;
		}
		return;
	}
	if(!fmax) {
		fmax = new Number(*nmax);
	} else {
		fmax->set(*nmax);
	}
}
void Assumptions::setIncludeEqualsMax(bool include_equals) {
	b_incl_max = include_equals;
}
bool Assumptions::includeEqualsMax() const {
	return b_incl_max;
}
const Number *Assumptions::max() const {
	return fmax;
}


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
	o_assumption = NULL;
}
UnknownVariable::UnknownVariable() : Variable() {
	o_assumption = NULL;
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
	if(item->type() == TYPE_VARIABLE && item->subtype() == SUBTYPE_UNKNOWN_VARIABLE) {
		if(o_assumption) delete o_assumption;
		o_assumption = ((UnknownVariable*) item)->assumptions();
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
bool UnknownVariable::representsPositive(bool allow_units) { 
	if(o_assumption) return o_assumption->isPositive();
	return CALCULATOR->defaultAssumptions()->isPositive();
}
bool UnknownVariable::representsNegative(bool allow_units) { 
	if(o_assumption) return o_assumption->isNegative();
	return CALCULATOR->defaultAssumptions()->isNegative();
}
bool UnknownVariable::representsNonNegative(bool allow_units) { 
	if(o_assumption) return o_assumption->isNonNegative();
	return CALCULATOR->defaultAssumptions()->isNonNegative();
}
bool UnknownVariable::representsNonPositive(bool allow_units) { 
	if(o_assumption) return o_assumption->isNonPositive();
	return CALCULATOR->defaultAssumptions()->isNonPositive();
}
bool UnknownVariable::representsInteger(bool allow_units) { 
	if(o_assumption) return o_assumption->isInteger();
	return CALCULATOR->defaultAssumptions()->isInteger();
}
bool UnknownVariable::representsNumber(bool allow_units) { 
	if(o_assumption) return o_assumption->isNumber();
	return CALCULATOR->defaultAssumptions()->isNumber();
}
bool UnknownVariable::representsRational(bool allow_units) { 
	if(o_assumption) return o_assumption->isRational();
	return CALCULATOR->defaultAssumptions()->isRational();
}
bool UnknownVariable::representsReal(bool allow_units) { 
	if(o_assumption) return o_assumption->isReal();
	return CALCULATOR->defaultAssumptions()->isReal();
}
bool UnknownVariable::representsComplex(bool allow_units) { 
	if(o_assumption) return o_assumption->isComplex();
	return CALCULATOR->defaultAssumptions()->isComplex();
}
bool UnknownVariable::representsNonZero(bool allow_units) { 
	if(o_assumption) return o_assumption->isNonZero();
	return CALCULATOR->defaultAssumptions()->isNonZero();
}

KnownVariable::KnownVariable(string cat_, string name_, const MathStructure &o, string title_, bool is_local, bool is_builtin, bool is_active) : Variable(cat_, name_, title_, is_local, is_builtin, is_active) {
	mstruct = new MathStructure(o);
	setApproximate(mstruct->isApproximate());
	setPrecision(mstruct->precision());
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
	if(item->type() == TYPE_VARIABLE && item->subtype() == SUBTYPE_KNOWN_VARIABLE) {
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
	setApproximate(mstruct->isApproximate());
	setPrecision(mstruct->precision());
	calculated_precision = 0;
	b_expression = false;
	sexpression = "";
	setChanged(true);
}
void KnownVariable::set(string expression_) {
	if(mstruct) {
		delete mstruct;
	}	
	mstruct = NULL;
	b_expression = true;
	sexpression = expression_;
	remove_blank_ends(sexpression);
	calculated_precision = 0;
	setChanged(true);
}
const MathStructure &KnownVariable::get() {
	if(b_expression && !mstruct) {
		ParseOptions po;
		if(isApproximate() && precision() < 1) {
			po.read_precision = READ_PRECISION_WHEN_DECIMALS;
		}
		mstruct = new MathStructure();
		CALCULATOR->parse(mstruct, sexpression, po);
		if(precision() > 0 && (mstruct->precision() < 1 || precision() < mstruct->precision())) {
			mstruct->setPrecision(precision());
		}
		if(isApproximate() && !mstruct->isApproximate()) {
			mstruct->setApproximate();
		}
	}
	return *mstruct;
}
bool KnownVariable::representsPositive(bool allow_units) {return get().representsPositive(allow_units);}
bool KnownVariable::representsNegative(bool allow_units) {return get().representsNegative(allow_units);}
bool KnownVariable::representsNonNegative(bool allow_units) {return get().representsNonNegative(allow_units);}
bool KnownVariable::representsNonPositive(bool allow_units) {return get().representsNonPositive(allow_units);}
bool KnownVariable::representsInteger(bool allow_units) {return get().representsInteger(allow_units);}
bool KnownVariable::representsNumber(bool allow_units) {return get().representsNumber(allow_units);}
bool KnownVariable::representsRational(bool allow_units) {return get().representsRational(allow_units);}
bool KnownVariable::representsReal(bool allow_units) {return get().representsReal(allow_units);}
bool KnownVariable::representsComplex(bool allow_units) {return get().representsComplex(allow_units);}
bool KnownVariable::representsNonZero(bool allow_units) {return get().representsNonZero(allow_units);}
bool KnownVariable::representsEven(bool allow_units) {return get().representsEven(allow_units);}
bool KnownVariable::representsOdd(bool allow_units) {return get().representsOdd(allow_units);}
bool KnownVariable::representsUndefined(bool include_childs, bool include_infinite) {return get().representsUndefined(include_childs, include_infinite);}

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


PiVariable::PiVariable() : DynamicVariable("Constants", "pi") {}
void PiVariable::calculate() const {
	Number nr; nr.pi(); mstruct->set(nr);
}
EVariable::EVariable() : DynamicVariable("Constants", "e") {}
void EVariable::calculate() const {
	Number nr; nr.e(); mstruct->set(nr);
}
EulerVariable::EulerVariable() : DynamicVariable("Constants", "euler") {}
void EulerVariable::calculate() const {
	Number nr; nr.euler(); mstruct->set(nr);
}
CatalanVariable::CatalanVariable() : DynamicVariable("Constants", "catalan") {}
void CatalanVariable::calculate() const {
	Number nr; nr.catalan(); mstruct->set(nr);
}

