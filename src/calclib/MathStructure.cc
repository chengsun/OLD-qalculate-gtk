/*
    Qalculate    

    Copyright (C) 2004  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "MathStructure.h"
#include "Calculator.h"
#include "Number.h"
#include "Function.h"
#include "Variable.h"
#include "Unit.h"
#include "Prefix.h"


#define MERGE_APPROX_AND_PREC(o)	if(!b_approx && o.isApproximate()) b_approx = true; if(o.precision() > 0 && (i_precision < 1 || o.precision() < i_precision)) i_precision = o.precision();

#define APPEND(o)	v_order.push_back(v_subs.size()); v_subs.push_back(o); if(!b_approx && o.isApproximate()) b_approx = true; if(o.precision() > 0 && (i_precision < 1 || o.precision() < i_precision)) i_precision = o.precision();
#define PREPEND(o)	v_order.insert(v_order.begin(), v_subs.size()); v_subs.push_back(o); if(!b_approx && o.isApproximate()) b_approx = true; if(o.precision() > 0 && (i_precision < 1 || o.precision() < i_precision)) i_precision = o.precision();
#define CLEAR		v_order.clear(); v_subs.clear();
#define REDUCE(v_size)	for(unsigned int v_index = v_size; v_index < v_order.size(); v_index++) {v_subs.erase(v_subs.begin() + v_order[v_index]);} v_order.resize(v_size);
#define CHILD(v_index)	v_subs[v_order[v_index]]
#define SIZE		v_order.size()
#define LAST		v_subs[v_order[v_order.size() - 1]]
#define ERASE(v_index)	v_subs.erase(v_subs.begin() + v_order[v_index]); for(unsigned int v_index2 = 0; v_index2 < v_order.size(); v_index2++) {if(v_order[v_index2] > v_order[v_index]) v_order[v_index2]--;} v_order.erase(v_order.begin() + v_index);


void idm1(const MathStructure &mnum, bool &bfrac, bool &bint);
void idm2(const MathStructure &mnum, bool &bfrac, bool &bint, Number &nr);
void idm3(MathStructure &mnum, Number &nr);


inline void MathStructure::init() {
#ifdef HAVE_GIAC
	giac_unknown = NULL;
#endif
	m_type = STRUCT_NUMBER;
	b_approx = false;
	i_precision = -1;
}

MathStructure::MathStructure() {
	init();
}
MathStructure::MathStructure(const MathStructure &o) {
	init();
	set(o);
}
MathStructure::MathStructure(int num, int den, int exp10) {
	init();
	set(num, den, exp10);
}
MathStructure::MathStructure(string sym) {
	init();
	set(sym);
}
MathStructure::MathStructure(double float_value) {
	init();
	set(float_value);
}
MathStructure::MathStructure(const MathStructure *o, ...) {
	init();
	clear();
	va_list ap;
	va_start(ap, o); 
	if(o) {
		APPEND((*o))
		while(true) {
			o = va_arg(ap, const MathStructure*);
			if(!o) break;
			APPEND((*o))
		}
	}
	va_end(ap);	
	m_type = STRUCT_VECTOR;
}
MathStructure::MathStructure(Function *o, ...) {
	init();
	clear();
	va_list ap;
	va_start(ap, o); 
	o_function = o;
	while(true) {
		const MathStructure *mstruct = va_arg(ap, const MathStructure*);
		if(!mstruct) break;
		APPEND((*mstruct))
	}
	va_end(ap);	
	m_type = STRUCT_FUNCTION;
}
MathStructure::MathStructure(Unit *u, Prefix *p) {
	init();
	set(u, p);
}
MathStructure::MathStructure(Variable *o) {
	init();
	set(o);
}
MathStructure::MathStructure(const Number &o) {
	init();
	set(o);
}
MathStructure::~MathStructure() {
	clear();
#ifdef HAVE_GIAC	
	if(giac_unknown) {
		delete giac_unknown;
	}
#endif
}

void MathStructure::set(const MathStructure &o) {
	clear();
	switch(o.type()) {
		case STRUCT_NUMBER: {
			o_number.set(o.number());
			break;
		}
#ifdef HAVE_GIAC		
		case STRUCT_UNKNOWN: {
			giac_unknown = new giac::gen(*o.unknown());
			break;
		}
#endif		
		case STRUCT_SYMBOLIC: {
			s_sym = o.symbol();
			break;
		}
		case STRUCT_FUNCTION: {
			o_function = o.function();
			break;
		}
		case STRUCT_VARIABLE: {
			o_variable = o.variable();
			break;
		}
		case STRUCT_UNIT: {
			o_unit = o.unit();
			o_prefix = o.prefix();
			b_plural = o.isPlural();
			break;
		}
		case STRUCT_COMPARISON: {
			ct_comp = o.comparisonType();
			break;
		}
	}
	for(unsigned int i = 0; i < o.size(); i++) {
		APPEND(o[i])
	}
	b_approx = o.isApproximate();
	i_precision = o.precision();
	m_type = o.type();
}
void MathStructure::set(int num, int den, int exp10) {
	clear();
	o_number.set(num, den, exp10);
	b_approx = o_number.isApproximate();
	i_precision = o_number.precision();
	m_type = STRUCT_NUMBER;
}
void MathStructure::set(double float_value) {
	clear();
	o_number.setFloat(float_value);
	b_approx = o_number.isApproximate();
	i_precision = o_number.precision();
	m_type = STRUCT_NUMBER;
}
void MathStructure::set(string sym) {
	clear();
	s_sym = sym;
	m_type = STRUCT_SYMBOLIC;
}
void MathStructure::set(const MathStructure *o, ...) {
	clear();
	va_list ap;
	va_start(ap, o); 
	if(o) {
		APPEND((*o))
		while(true) {
			o = va_arg(ap, const MathStructure*);
			if(!o) break;
			APPEND((*o))
		}
	}
	va_end(ap);	
	m_type = STRUCT_VECTOR;
}
void MathStructure::set(Function *o, ...) {
	clear();
	va_list ap;
	va_start(ap, o); 
	o_function = o;
	while(true) {
		const MathStructure *mstruct = va_arg(ap, const MathStructure*);
		if(!mstruct) break;
		APPEND((*mstruct))
	}
	va_end(ap);	
	m_type = STRUCT_FUNCTION;
}
void MathStructure::set(Unit *u, Prefix *p) {
	clear();
	o_unit = u;
	o_prefix = p;
	m_type = STRUCT_UNIT;
}
void MathStructure::set(Variable *o) {
	clear();
	o_variable = o;
	m_type = STRUCT_VARIABLE;
}
void MathStructure::set(const Number &o) {
	clear();
	o_number.set(o);
	b_approx = o_number.isApproximate();
	i_precision = o_number.precision();
	m_type = STRUCT_NUMBER;
}
void MathStructure::setInfinity() {
	clear();
	o_number.setInfinity();
	m_type = STRUCT_NUMBER;
}
void MathStructure::setUndefined() {
	clear();
	m_type = STRUCT_UNDEFINED;
}

void MathStructure::operator = (const MathStructure &o) {set(o);}
MathStructure MathStructure::operator - () const {
	MathStructure o2(*this);
	o2.negate();
	return o2;
}
MathStructure MathStructure::operator * (const MathStructure &o) const {
	MathStructure o2(*this);
	o2.multiply(o);
	return o2;
}
MathStructure MathStructure::operator / (const MathStructure &o) const {
	MathStructure o2(*this);
	o2.divide(o);
	return o2;
}
MathStructure MathStructure::operator + (const MathStructure &o) const {
	MathStructure o2(*this);
	o2.add(o);
	return o;
}
MathStructure MathStructure::operator - (const MathStructure &o) const {
	MathStructure o2(*this);
	o2.subtract(o);
	return o2;
}
MathStructure MathStructure::operator ^ (const MathStructure &o) const {
	MathStructure o2(*this);
	o2.raise(o);
	return o2;
}
MathStructure MathStructure::operator && (const MathStructure &o) const {
	MathStructure o2(*this);
	o2.add(o, OPERATION_AND);
	return o2;
}
MathStructure MathStructure::operator || (const MathStructure &o) const {
	MathStructure o2(*this);
	o2.add(o, OPERATION_OR);
	return o2;
}
MathStructure MathStructure::operator ! () const {
	MathStructure o2(*this);
	o2.setNOT();
	return o2;
}

void MathStructure::operator *= (const MathStructure &o) {multiply(o);}
void MathStructure::operator /= (const MathStructure &o) {divide(o);}
void MathStructure::operator += (const MathStructure &o) {add(o);}
void MathStructure::operator -= (const MathStructure &o) {subtract(o);}
void MathStructure::operator ^= (const MathStructure &o) {raise(o);}

bool MathStructure::operator == (const MathStructure &o) const {return equals(o);}
bool MathStructure::operator != (const MathStructure &o) const {return !equals(o);}

const MathStructure &MathStructure::operator [] (unsigned int index) const {return CHILD(index);}
MathStructure &MathStructure::operator [] (unsigned int index) {return CHILD(index);}

void MathStructure::clear() {
	m_type = STRUCT_NUMBER;
	o_number.clear();
#ifdef HAVE_GIAC	
	if(giac_unknown) {
		delete giac_unknown;
		giac_unknown = NULL;
	}
#endif
	o_function = NULL;
	o_variable = NULL;
	o_unit = NULL;
	o_prefix = NULL;
	b_plural = false;
	CLEAR;
}
void MathStructure::clearVector() {
	clear();
	m_type = STRUCT_VECTOR;
}
void MathStructure::clearMatrix() {
	clearVector();
	v_subs.resize(1);
	v_order.push_back(0);
	CHILD(0).clearVector();
}

const Number &MathStructure::number() const {
	return o_number;
}
Number &MathStructure::number() {
	return o_number;
}
void MathStructure::numberUpdated() {
	if(m_type != STRUCT_NUMBER) return;
	MERGE_APPROX_AND_PREC(o_number)
}
void MathStructure::childUpdated(unsigned int index, bool recursive) {
	if(index > SIZE || index < 1) return;
	if(recursive) CHILD(index - 1).childrenUpdated(true);
	MERGE_APPROX_AND_PREC(CHILD(index - 1))
}
void MathStructure::childrenUpdated(bool recursive) {
	for(unsigned int i = 0; i < SIZE; i++) {
		if(recursive) CHILD(i).childrenUpdated(true);
		MERGE_APPROX_AND_PREC(CHILD(i))
	}
}
#ifdef HAVE_GIAC
const giac::gen *MathStructure::unknown() const {
	return giac_unknown;
}
#endif
const string &MathStructure::symbol() const {
	return s_sym;
}
ComparisonType MathStructure::comparisonType() const {
	return ct_comp;
}
void MathStructure::setComparisonType(ComparisonType comparison_type) {
	ct_comp = comparison_type;
}
void MathStructure::setType(int mtype) {
	m_type = mtype;
}
Unit *MathStructure::unit() const {
	return o_unit;
}
Prefix *MathStructure::prefix() const {
	return o_prefix;
}
void MathStructure::setPrefix(Prefix *p) {
	if(isUnit()) o_prefix = p;
}
bool MathStructure::isPlural() const {
	return b_plural;
}
void MathStructure::setPlural(bool is_plural) {
	if(isUnit()) b_plural = is_plural;
}
void MathStructure::setFunction(Function *f) {
	o_function = f;
}
void MathStructure::setUnit(Unit *u) {
	o_unit = u;
}
void MathStructure::setVariable(Variable *v) {
	o_variable = v;
}
Function *MathStructure::function() const {
	return o_function;
}
Variable *MathStructure::variable() const {
	return o_variable;
}

bool MathStructure::isAddition() const {return m_type == STRUCT_ADDITION;}
bool MathStructure::isMultiplication() const {return m_type == STRUCT_MULTIPLICATION;}
bool MathStructure::isPower() const {return m_type == STRUCT_POWER;}
bool MathStructure::isSymbolic() const {return m_type == STRUCT_SYMBOLIC;}
bool MathStructure::isEmptySymbol() const {return m_type == STRUCT_SYMBOLIC && s_sym.empty();}
bool MathStructure::isVector() const {return m_type == STRUCT_VECTOR;}
bool MathStructure::isMatrix() const {
	if(m_type != STRUCT_VECTOR || SIZE < 1) return false;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(!CHILD(i).isVector() || (i > 0 && CHILD(i).size() != CHILD(i - 1).size())) return false;
	}
	return true;
}
bool MathStructure::isFunction() const {return m_type == STRUCT_FUNCTION;}
bool MathStructure::isUnit() const {return m_type == STRUCT_UNIT;}
bool MathStructure::isUnit_exp() const {return m_type == STRUCT_UNIT || (m_type == STRUCT_POWER && CHILD(0).isUnit());}
bool MathStructure::isNumber_exp() const {return m_type == STRUCT_NUMBER || (m_type == STRUCT_POWER && CHILD(0).isNumber());}
bool MathStructure::isVariable() const {return m_type == STRUCT_VARIABLE;}
bool MathStructure::isComparison() const {return m_type == STRUCT_COMPARISON;}
bool MathStructure::isAND() const {return m_type == STRUCT_AND;}
bool MathStructure::isOR() const {return m_type == STRUCT_OR;}
bool MathStructure::isXOR() const {return m_type == STRUCT_XOR;}
bool MathStructure::isNOT() const {return m_type == STRUCT_NOT;}
bool MathStructure::isInverse() const {return m_type == STRUCT_INVERSE;}
bool MathStructure::isDivision() const {return m_type == STRUCT_DIVISION;}
bool MathStructure::isNegate() const {return m_type == STRUCT_NEGATE;}
bool MathStructure::isInfinity() const {return m_type == STRUCT_NUMBER && o_number.isInfinite();}
bool MathStructure::isUndefined() const {return m_type == STRUCT_UNDEFINED || (m_type == STRUCT_NUMBER && o_number.isUndefined());}
bool MathStructure::isInteger() const {return m_type == STRUCT_NUMBER && o_number.isInteger();};
bool MathStructure::isNumber() const {return m_type == STRUCT_NUMBER;}
bool MathStructure::isZero() const {return m_type == STRUCT_NUMBER && o_number.isZero();}
bool MathStructure::isOne() const {return m_type == STRUCT_NUMBER && o_number.isOne();}
bool MathStructure::isMinusOne() const {return m_type == STRUCT_NUMBER && o_number.isMinusOne();}

bool MathStructure::hasNegativeSign() const {
	return (m_type == STRUCT_NUMBER && o_number.hasNegativeSign()) || m_type == STRUCT_NEGATE || (m_type == STRUCT_MULTIPLICATION && SIZE > 0 && CHILD(0).hasNegativeSign());
}

bool MathStructure::representsNumber() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return true;}
		case STRUCT_VARIABLE: {return o_variable->isNumber();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isNumber();}
		case STRUCT_FUNCTION: {return o_function->representsNumber(*this);}
		case STRUCT_UNIT: {return true;}
		case STRUCT_ADDITION: {}
		case STRUCT_POWER: {}
		case STRUCT_MULTIPLICATION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsNumber()) return false;
			}
			return true;
		}
		default: {return false;}
	}
}
bool MathStructure::representsInteger() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isInteger();}
		case STRUCT_VARIABLE: {return o_variable->isInteger();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isInteger();}
		case STRUCT_FUNCTION: {return o_function->representsInteger(*this);}
		case STRUCT_UNIT: {return true;}
		case STRUCT_ADDITION: {}
		case STRUCT_MULTIPLICATION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsInteger()) return false;
			}
			return true;
		}
		case STRUCT_POWER: {
			return CHILD(0).representsInteger() && CHILD(1).representsInteger() && CHILD(1).representsPositive();
		}
		default: {return false;}
	}
}
bool MathStructure::representsPositive() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isPositive();}
		case STRUCT_VARIABLE: {return o_variable->isPositive();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isPositive();}
		case STRUCT_FUNCTION: {return o_function->representsPositive(*this);}
		case STRUCT_UNIT: {return true;}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsPositive()) return false;
			}
			return true;
		}
		case STRUCT_MULTIPLICATION: {
			bool b = true;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).representsNegative()) {
					b = !b;
				} else if(!CHILD(i).representsPositive()) {
					return false;
				}
			}
			return b;
		}
		case STRUCT_POWER: {
			return (CHILD(0).representsPositive() && CHILD(1).representsReal()) || (CHILD(0).representsNegative() && CHILD(1).representsEven() && CHILD(1).representsInteger() && CHILD(1).representsPositive());
		}
		default: {return false;}
	}
}
bool MathStructure::representsNegative() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isNegative();}
		case STRUCT_VARIABLE: {return o_variable->isNegative();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isNegative();}
		case STRUCT_FUNCTION: {return o_function->representsNegative(*this);}
		case STRUCT_UNIT: {return false;}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsNegative()) return false;
			}
			return true;
		}
		case STRUCT_MULTIPLICATION: {
			bool b = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).representsNegative()) {
					b = !b;
				} else if(!CHILD(i).representsPositive()) {
					return false;
				}
			}
			return b;
		}
		case STRUCT_POWER: {
			return CHILD(1).representsInteger() && CHILD(1).representsPositive() && CHILD(1).representsOdd() && CHILD(0).representsNegative();
		}
		default: {return false;}
	}
}
bool MathStructure::representsNonNegative() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isNonNegative();}
		case STRUCT_VARIABLE: {return o_variable->isNonNegative();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isNonNegative();}
		case STRUCT_FUNCTION: {return o_function->representsNonNegative(*this);}
		case STRUCT_UNIT: {return true;}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsNonNegative()) return false;
			}
			return true;
		}
		case STRUCT_MULTIPLICATION: {
			bool b = true;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).representsNegative()) {
					b = !b;
				} else if(!CHILD(i).representsNonNegative()) {
					return false;
				}
			}
			return b;
		}
		case STRUCT_POWER: {
			return (CHILD(0).isZero() && CHILD(1).representsNonNegative()) || representsPositive();
		}
		default: {return false;}
	}
}
bool MathStructure::representsNonPositive() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isNonPositive();}
		case STRUCT_VARIABLE: {return o_variable->isNonPositive();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isNonPositive();}
		case STRUCT_FUNCTION: {return o_function->representsNonPositive(*this);}
		case STRUCT_UNIT: {return false;}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsNonPositive()) return false;
			}
			return true;
		}
		case STRUCT_MULTIPLICATION: {
			bool b = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).representsNegative()) {
					b = !b;
				} else if(!CHILD(i).representsPositive()) {
					return false;
				}
			}
			return b;
		}
		case STRUCT_POWER: {
			return (CHILD(0).isZero() && CHILD(1).representsPositive()) || representsNegative();
		}
		default: {return false;}
	}
}
bool MathStructure::representsRational() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isRational();}
		case STRUCT_VARIABLE: {return o_variable->isRational();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isRational();}
		case STRUCT_FUNCTION: {return o_function->representsRational(*this);}
		case STRUCT_UNIT: {return true;}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsRational()) return false;
			}
			return true;
		}
		case STRUCT_MULTIPLICATION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsRational()) {
					return false;
				}
			}
			return true;
		}
		case STRUCT_POWER: {
			return CHILD(1).representsInteger() && CHILD(0).representsRational() && (CHILD(0).representsPositive() || (CHILD(0).representsNegative() && CHILD(1).representsEven() && CHILD(1).representsPositive()));
		}
		default: {return false;}
	}
}
bool MathStructure::representsReal() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isReal();}
		case STRUCT_VARIABLE: {return o_variable->isReal();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isReal();}
		case STRUCT_FUNCTION: {return o_function->representsReal(*this);}
		case STRUCT_UNIT: {return true;}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsReal()) return false;
			}
			return true;
		}
		case STRUCT_MULTIPLICATION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsReal()) {
					return false;
				}
			}
			return true;
		}
		case STRUCT_POWER: {
			return (CHILD(0).representsPositive() && CHILD(1).representsReal()) || (CHILD(0).representsReal() && CHILD(1).representsEven() && CHILD(1).representsInteger() && CHILD(1).representsPositive());
		}
		default: {return false;}
	}
}
bool MathStructure::representsComplex() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isComplex();}
		case STRUCT_VARIABLE: {return o_variable->isComplex();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isComplex();}
		case STRUCT_FUNCTION: {return o_function->representsComplex(*this);}
		case STRUCT_ADDITION: {
			bool c = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).representsComplex()) {
					if(c) return false;
					c = true;
				} else if(!CHILD(i).representsReal() || !CHILD(i).representsNonZero()) {
					return false;
				}
			}
			return c;
		}
		case STRUCT_MULTIPLICATION: {
			bool c = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).representsComplex()) {
					if(c) return false;
					c = true;
				} else if(!CHILD(i).representsReal()) {
					return false;
				}
			}
			return c;
		}
		case STRUCT_UNIT: {return false;}
		default: {return false;}
	}
}
bool MathStructure::representsNonZero() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return !o_number.isZero();}
		case STRUCT_VARIABLE: {return o_variable->isNonZero();}
		case STRUCT_SYMBOLIC: {return CALCULATOR->defaultAssumptions()->isNonZero();}
		case STRUCT_FUNCTION: {return o_function->representsNonZero(*this);}
		case STRUCT_UNIT: {return true;}
		case STRUCT_ADDITION: {
			bool neg = false, started = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if((!started || neg) && CHILD(i).representsNegative()) {
					neg = true;
				} else if(neg || !CHILD(i).representsPositive()) {
					return false;
				}
				started = true;
			}
			return true;
		}
		case STRUCT_MULTIPLICATION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).representsNonZero()) {
					return false;
				}
			}
			return true;
		}
		case STRUCT_POWER: {
			return CHILD(0).representsNonZero();
		}
		default: {return false;}
	}
}
bool MathStructure::representsEven() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isEven();}
		case STRUCT_FUNCTION: {return o_function->representsEven(*this);}
		default: {return false;}
	}
}
bool MathStructure::representsOdd() const {
	switch(m_type) {
		case STRUCT_NUMBER: {return o_number.isOdd();}
		case STRUCT_FUNCTION: {return o_function->representsOdd(*this);}
		default: {return false;}
	}
}
bool MathStructure::representsUndefined(bool include_childs, bool include_infinite) const {
	switch(m_type) {
		case STRUCT_NUMBER: {
			if(include_infinite) {
				return o_number.isInfinite();
			}
			return false;
		}
		case STRUCT_UNDEFINED: {return true;}
		case STRUCT_POWER: {return (CHILD(0).isZero() && CHILD(1).representsNegative()) || (CHILD(0).isInfinity() && CHILD(1).isZero());}
		case STRUCT_FUNCTION: {return o_function->representsUndefined(*this);}
		default: {
			if(include_childs) {
				for(unsigned int i = 0; i < SIZE; i++) {
					if(CHILD(i).representsUndefined(include_childs, include_infinite)) return true;
				}
			}
			return false;
		}
	}
}


void MathStructure::setApproximate(bool is_approx) {
	b_approx = is_approx;
	if(b_approx) {
		if(i_precision < 1) i_precision = PRECISION;
	} else {
		i_precision = -1;
	}
}
bool MathStructure::isApproximate() const {
	return b_approx;
}

int MathStructure::precision() const {
	return i_precision;
}
void MathStructure::setPrecision(int prec) {
	i_precision = prec;
	if(i_precision > 0) b_approx = true;
}

void MathStructure::transform(int mtype, const MathStructure &o) {
	MathStructure struct_this(*this);
	clear();
	m_type = mtype;
	APPEND(struct_this);
	APPEND(o);
}
void MathStructure::transform(int mtype) {
	MathStructure struct_this(*this);
	clear();
	m_type = mtype;
	APPEND(struct_this);
}
void MathStructure::add(const MathStructure &o, MathOperation op, bool append) {
	switch(op) {
		case OPERATION_ADD: {
			add(o, append);
			break;
		}
		case OPERATION_SUBTRACT: {
			subtract(o, append);
			break;
		}
		case OPERATION_MULTIPLY: {
			multiply(o, append);
			break;
		}
		case OPERATION_DIVIDE: {
			divide(o, append);
			break;
		}
		case OPERATION_RAISE: {
			raise(o);
			break;
		}
		case OPERATION_EXP10: {
			MathStructure mstruct(10, 1);
			mstruct.raise(o);
			multiply(mstruct, append);
			break;
		}
		case OPERATION_AND: {
			if(m_type == STRUCT_AND && append) {
				APPEND(o);
			} else {
				transform(STRUCT_AND, o);
			}
			break;
		}
		case OPERATION_OR: {
			if(m_type == STRUCT_OR && append) {
				APPEND(o);
			} else {
				transform(STRUCT_OR, o);
			}
			break;
		}
		case OPERATION_XOR: {
			if(m_type == STRUCT_XOR && append) {
				APPEND(o);
			} else {
				transform(STRUCT_XOR, o);
			}
			break;
		}
		case OPERATION_EQUALS: {}
		case OPERATION_NOT_EQUALS: {}
		case OPERATION_GREATER: {}
		case OPERATION_LESS: {}
		case OPERATION_EQUALS_GREATER: {}
		case OPERATION_EQUALS_LESS: {
			if(append && m_type == STRUCT_COMPARISON && append) {
				MathStructure o2(CHILD(1));
				o2.add(o, op);
				transform(STRUCT_AND, o2);
			} else if(append && m_type == STRUCT_AND && LAST.type() == STRUCT_COMPARISON) {
				MathStructure o2(LAST[1]);
				o2.add(o, op);
				APPEND(o2);
			} else {
				transform(STRUCT_COMPARISON, o);
				switch(op) {
					case OPERATION_EQUALS: {ct_comp = COMPARISON_EQUALS; break;}
					case OPERATION_NOT_EQUALS: {ct_comp = COMPARISON_NOT_EQUALS; break;}
					case OPERATION_GREATER: {ct_comp = COMPARISON_GREATER; break;}
					case OPERATION_LESS: {ct_comp = COMPARISON_LESS; break;}
					case OPERATION_EQUALS_GREATER: {ct_comp = COMPARISON_EQUALS_GREATER; break;}
					case OPERATION_EQUALS_LESS: {ct_comp = COMPARISON_EQUALS_LESS; break;}
					default: {}
				}
			}
			break;
		}
		default: {
		}
	}
}
void MathStructure::add(const MathStructure &o, bool append) {
	if(m_type == STRUCT_ADDITION && append) {
		APPEND(o);
	} else {
		transform(STRUCT_ADDITION, o);
	}
}
void MathStructure::subtract(const MathStructure &o, bool append) {
	MathStructure o2(o);
	o2.negate();
	add(o2, append);
}
void MathStructure::multiply(const MathStructure &o, bool append) {
	if(m_type == STRUCT_MULTIPLICATION && append) {
		APPEND(o);
	} else {
		transform(STRUCT_MULTIPLICATION, o);
	}
}
void MathStructure::divide(const MathStructure &o, bool append) {
//	transform(STRUCT_DIVISION, o);
	MathStructure o2(o);
	o2.inverse();
	multiply(o2, append);
}
void MathStructure::raise(const MathStructure &o) {
	transform(STRUCT_POWER, o);
}
void MathStructure::negate() {
	//transform(STRUCT_NEGATE);
	MathStructure struct_this(*this);
	clear();
	m_type = STRUCT_MULTIPLICATION;
	APPEND(m_minus_one);
	APPEND(struct_this);
}
void MathStructure::inverse() {
	//transform(STRUCT_INVERSE);
	raise(m_minus_one);
}
void MathStructure::setNOT() {
	transform(STRUCT_NOT);
}		

bool MathStructure::equals(const MathStructure &o) const {
	if(m_type != o.type()) return false;
	if(SIZE != o.size()) return false;
	switch(m_type) {
		case STRUCT_UNDEFINED: {return true;}
		case STRUCT_SYMBOLIC: {return s_sym == o.symbol();}
		case STRUCT_NUMBER: {return o_number.equals(o.number());}
		case STRUCT_VARIABLE: {return o_variable == o.variable();}
		case STRUCT_UNIT: {return o_unit == o.unit() && o_prefix == o.prefix();}
		case STRUCT_COMPARISON: {if(ct_comp != o.comparisonType()) return false; break;}
		case STRUCT_FUNCTION: {if(o_function != o.function()) return false; break;}
	}
	if(SIZE < 1) return false;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(!CHILD(i).equals(o[i])) return false;
	}
	return true;
}		
ComparisonResult MathStructure::compare(const MathStructure &o) const {
	if(isNumber() && o.isNumber()) {
		return o_number.compare(o.number());
	}
	if(equals(o)) return COMPARISON_RESULT_EQUAL;
	if(o.representsReal() && representsComplex()) return COMPARISON_RESULT_NOT_EQUAL;
	if(representsReal() && o.representsComplex()) return COMPARISON_RESULT_NOT_EQUAL;
	MathStructure mtest(*this);
	mtest -= o;
	EvaluationOptions eo = default_evaluation_options;
	eo.approximation = APPROXIMATION_APPROXIMATE;
	mtest.calculatesub(eo, eo);
	bool incomp = false;
	if(mtest.isAddition()) {
		for(unsigned int i = 1; i < mtest.size(); i++) {
			if(!mtest[i - 1].isUnitCompatible(mtest[i])) {
				incomp = true;
				break;
			}
		}
	}
	if(mtest.isZero()) return COMPARISON_RESULT_EQUAL;
	else if(mtest.representsPositive()) {if(incomp) return COMPARISON_RESULT_NOT_EQUAL; return COMPARISON_RESULT_LESS;}
	else if(mtest.representsNegative()) {if(incomp) return COMPARISON_RESULT_NOT_EQUAL; return COMPARISON_RESULT_GREATER;}
	else if(mtest.representsNonZero()) return COMPARISON_RESULT_NOT_EQUAL;
	else if(mtest.representsNonPositive()) {if(incomp) return COMPARISON_RESULT_NOT_EQUAL; return COMPARISON_RESULT_EQUAL_OR_LESS;}
	else if(mtest.representsNonNegative()) {if(incomp) return COMPARISON_RESULT_NOT_EQUAL; return COMPARISON_RESULT_EQUAL_OR_GREATER;}
	return COMPARISON_RESULT_UNKNOWN;
}

int MathStructure::merge_addition(const MathStructure &mstruct, const EvaluationOptions &eo) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		Number nr(o_number);
		if(nr.add(mstruct.number()) && (eo.approximation == APPROXIMATION_APPROXIMATE || !nr.isApproximate() || o_number.isApproximate() || mstruct.number().isApproximate())) {
			o_number = nr;
			numberUpdated();
			return 1;
		}
		return -1;
	}
	if(isZero()) {
		if(b_approx) {
			int prec_copy = i_precision;
			set(mstruct);
			b_approx = true;
			if(i_precision < 0 || prec_copy < i_precision) i_precision = prec_copy;
		} else {
			set(mstruct);
		}
		return 1;
	}
	if(mstruct.isZero()) {
		MERGE_APPROX_AND_PREC(mstruct)
		return 1;
	}
	if(m_type == STRUCT_NUMBER && o_number.isInfinite()) {
		if(mstruct.representsNumber()) {
			MERGE_APPROX_AND_PREC(mstruct)
			return 1;
		}
	} else if(mstruct.isNumber() && mstruct.number().isInfinite()) {
		if(representsNumber()) {
			clear();
			o_number = mstruct.number();
			MERGE_APPROX_AND_PREC(mstruct)
			return 1;
		}
	}
	if(representsUndefined() || mstruct.representsUndefined()) return -1;
	switch(m_type) {
		case STRUCT_VECTOR: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {
					if(SIZE == mstruct.size()) {
						for(unsigned int i = 0; i < SIZE; i++) {
							CHILD(i) += mstruct[i]; 
						}
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
				}
				default: {
					return -1;
				}
			}
			return -1;
		}
		case STRUCT_ADDITION: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {
					return -1;
				}
				case STRUCT_ADDITION: {
					for(unsigned int i = 0; i < mstruct.size(); i++) {
						APPEND(mstruct[i]);
					}
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
				default: {
					APPEND(mstruct);
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {return -1;}
				case STRUCT_ADDITION: {
					return 0;
				}
				case STRUCT_MULTIPLICATION: {
					unsigned int i1 = 0, i2 = 0;
					bool b = true;
					if(CHILD(0).isNumber()) i1 = 1;
					if(mstruct[0].isNumber()) i2 = 1;
					if(SIZE - i1 == mstruct.size() - i2) {
						for(unsigned int i = i1; i < SIZE; i++) {
							if(CHILD(i) != mstruct[i + i2 - i1]) {
								b = false;
								break;
							}
						}
						if(b) {
							if(i1 == 0) {
								PREPEND(MathStructure(1));
							}
							if(i2 == 0) {
								CHILD(0).number()++;
							} else {
								CHILD(0).number() += mstruct[0].number();
							}
							MERGE_APPROX_AND_PREC(mstruct)
							return 1;
						}
					}
					b = true; unsigned int divs = 0;
					for(; b && i1 < SIZE; i1++) {
						if(CHILD(i1).isPower() && CHILD(i1)[1].hasNegativeSign()) {
							divs++;
							for(; i2 < mstruct.size(); i2++) {
								b = false;
								if(mstruct[i2].isPower() && mstruct[i2][1].hasNegativeSign()) {
									if(mstruct[i2] == CHILD(i1)) {
										b = true;
									}
									i2++;
									break;
								}
							}
						}
					}
					if(b && divs > 0) {
						for(; i2 < mstruct.size(); i2++) {
							if(mstruct[i2].isPower() && mstruct[i2][1].hasNegativeSign()) {
								b = false;
								break;
							}
						}
					}
					if(b && divs > 0) {
						if(SIZE - divs == 0) {
							if(mstruct.size() - divs == 0) {
								PREPEND(MathStructure(2, 1))
							} else if(mstruct.size() - divs == 1) {
								PREPEND(m_one);
								for(unsigned int i = 0; i < mstruct.size(); i++) {
									if(!mstruct[i].isPower() || !mstruct[i][1].hasNegativeSign()) {
										CHILD(0).add(mstruct[i], true);
										break;
									}
								}
							} else {
								PREPEND(m_one);
								CHILD(0).add(mstruct, true);
								for(int i = 0; i < (int) CHILD(0)[CHILD(0).size() - 1].size(); i++) {
									if(CHILD(0)[CHILD(0).size() - 1][i].isPower() && CHILD(0)[CHILD(0).size() - 1][i][1].hasNegativeSign()) {
										CHILD(0)[CHILD(0).size() - 1].delChild(i + 1);
										i--;
									}
								}
							}
						} else if(SIZE - divs == 1) {
							unsigned int index = 0;
							for(; index < SIZE; index++) {
								if(!CHILD(index).isPower() || !CHILD(index)[1].hasNegativeSign()) {
									break;
								}
							}
							if(mstruct.size() - divs == 0) {
								CHILD(index).add(m_one, true);
							} else if(mstruct.size() - divs == 1) {
								for(unsigned int i = 0; i < mstruct.size(); i++) {
									if(!mstruct[i].isPower() || !mstruct[i][1].hasNegativeSign()) {
										CHILD(index).add(mstruct[i], true);
										break;
									}
								}
							} else {
								CHILD(index).add(mstruct, true);
								for(int i = 0; i < (int) CHILD(index)[CHILD(index).size() - 1].size(); i++) {
									if(CHILD(index)[CHILD(index).size() - 1][i].isPower() && CHILD(index)[CHILD(index).size() - 1][i][1].hasNegativeSign()) {
										CHILD(index)[CHILD(index).size() - 1].delChild(i + 1);
										i--;
									}
								}
							}
						} else {
							for(int i = 0; i < (int) SIZE; i++) {
								if(CHILD(i).isPower() && CHILD(i)[1].hasNegativeSign()) {
									ERASE(i);
									i--;
								}
							}
							if(mstruct.size() - divs == 0) {
								add(m_one);
							} else if(mstruct.size() - divs == 1) {
								for(unsigned int i = 0; i < mstruct.size(); i++) {
									if(!mstruct[i].isPower() || !mstruct[i][1].hasNegativeSign()) {
										add(mstruct[i]);
										break;
									}
								}
							} else {
								add(mstruct);
								for(int i = 0; i < (int) CHILD(1).size(); i++) {
									if(CHILD(1)[i].isPower() && CHILD(1)[i][1].hasNegativeSign()) {
										CHILD(1).delChild(i + 1);
										i--;
									}
								}
							}
							for(unsigned int i = 0; i < mstruct.size(); i++) {
								if(mstruct[i].isPower() && mstruct[i][1].hasNegativeSign()) {
									multiply(mstruct[i], true);
								}
							}
						}
						return 1;
					}
					break;
				}
				case STRUCT_POWER: {
					if(mstruct[1].hasNegativeSign()) {
						bool b = false;
						for(unsigned int i = 0; i < SIZE; i++) {
							if(CHILD(i).isPower() && CHILD(i)[1].hasNegativeSign()) {
								if(b) {
									b = false;
									break;
								}
								if(mstruct == CHILD(i)) {
									b = true;
								}
								if(!b) break;
							}
						}
						if(b) {						
							for(unsigned int index = 0; index < SIZE; index++) {
								if(!CHILD(index).isPower() || !CHILD(index)[1].hasNegativeSign()) {
									CHILD(index).add(m_one, true);
									break;
								}
							}
							return 1;
						}
					}
				}
				default: {
					if(SIZE == 2 && CHILD(0).isNumber() && CHILD(1) == mstruct) {
						CHILD(0).number()++;
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
				}					
			}
			break;
		}
		default: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {return -1;}
				case STRUCT_ADDITION: {}
				case STRUCT_MULTIPLICATION: {
					return 0;
				}
				default: {
					if(equals(mstruct)) {
						multiply(2);
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
				}
			}	
		}		
	}
	return -1;
}

bool reducable(const MathStructure &mnum, const MathStructure &mden, Number &nr) {
	switch(mnum.type()) {
		case STRUCT_NUMBER: {}
		case STRUCT_ADDITION: {
			break;
		}
		default: {
			if(mnum.representsNonZero()) {
				bool reduce = true;
				for(unsigned int i = 0; i < mden.size() && reduce; i++) {
					switch(mden[i].type()) {
						case STRUCT_MULTIPLICATION: {
							reduce = false;
							for(unsigned int i2 = 0; i2 < mden[i].size(); i2++) {
								if(mnum == mden[i][i2]) {
									reduce = true;
									if(!nr.isOne() && !nr.isFraction()) nr.set(1, 1);
									break;
								} else if(mden[i][i2].isPower() && mden[i][i2][1].isNumber() && mden[i][i2][1].number().isReal() && mnum == mden[i][i2][0]) {
									if(!mden[i][i2][1].number().isPositive()) {
										break;
									}
									if(mden[i][i2][1].number().isLessThan(nr)) nr = mden[i][i2][1].number();
									reduce = true;
									break;
								}
							}
							break;
						}
						case STRUCT_POWER: {
							if(mden[i][1].isNumber() && mden[i][1].number().isReal() && mnum == mden[i][0]) {
								if(!mden[i][1].number().isPositive()) {
									reduce = false;
									break;
								}
								if(mden[i][1].number().isLessThan(nr)) nr = mden[i][1].number();
								break;
							}
						}
						default: {
							if(mnum != mden[i]) {
								reduce = false;
								break;
							}
							if(!nr.isOne() && !nr.isFraction()) nr.set(1, 1);
						}
					}
				}
				return reduce;
			}
		}
	}
	return false;
}
void reduce(const MathStructure &mnum, MathStructure &mden, Number &nr) {
	switch(mnum.type()) {
		case STRUCT_NUMBER: {}
		case STRUCT_ADDITION: {
			break;
		}
		default: {
			for(unsigned int i = 0; i < mden.size(); i++) {
				switch(mden[i].type()) {
					case STRUCT_MULTIPLICATION: {
						for(unsigned int i2 = 0; i2 < mden[i].size(); i2++) {
							if(mden[i][i2] == mnum) {
								if(!nr.isOne()) {
									MathStructure mexp(1, 1);
									mexp.number() -= nr;
									mden[i][i2].raise(mexp);
								} else {
									if(mden[i].size() == 1) {
										mden[i].set(1, 1);
									} else {
										mden[i].delChild(i2 + 1);
										if(mden[i].size() == 1) {
											MathStructure msave(mden[i][0]);
											mden[i] = msave;
										}
									}
								}
								break;
							} else if(mden[i][i2].isPower() && mden[i][i2][1].isNumber() && mden[i][i2][1].number().isReal() && mnum.equals(mden[i][i2][0])) {
								mden[i][i2][1].number() -= nr;
								if(mden[i][i2][1].number().isOne()) {
									MathStructure msave(mden[i][i2][0]);
									mden[i][i2] = msave;
								}
								break;
							}
						}
						break;
					}
					case STRUCT_POWER: {
						if(mden[i][1].isNumber() && mden[i][1].number().isReal() && mnum.equals(mden[i][0])) {
							mden[i][1].number() -= nr;
							if(mden[i][1].number().isOne()) {
								MathStructure msave(mden[i][0]);
								mden[i] = msave;
							}
							break;
						}
					}
					default: {
						if(!nr.isOne()) {
							MathStructure mexp(1, 1);
							mexp.number() -= nr;
							mden[i].raise(mexp);
						} else {
							mden[i].set(1, 1);
						}
					}
				}
			}
		}
	}
}

int MathStructure::merge_multiplication(const MathStructure &mstruct, const EvaluationOptions &eo, bool do_append) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		Number nr(o_number);
		if(nr.multiply(mstruct.number()) && (eo.approximation == APPROXIMATION_APPROXIMATE || !nr.isApproximate() || o_number.isApproximate() || mstruct.number().isApproximate()) && (eo.allow_complex || !nr.isComplex() || o_number.isComplex() || mstruct.number().isComplex()) && (eo.allow_infinite || !nr.isInfinite() || o_number.isInfinite() || mstruct.number().isInfinite())) {
			o_number = nr;
			numberUpdated();
			return 1;
		}
		return -1;
	}
	if(mstruct.isOne()) {
		MERGE_APPROX_AND_PREC(mstruct)
		return 1;
	} else if(isOne()) {
		if(b_approx) {
			int prec_copy = i_precision;
			set(mstruct);
			b_approx = true;
			if(i_precision < 0 || prec_copy < i_precision) i_precision = prec_copy;
		} else {
			set(mstruct);
		}
		return 1;
	}
	if(m_type == STRUCT_NUMBER && o_number.isInfinite()) {
		if(o_number.isMinusInfinity()) {
			if(mstruct.representsPositive()) {
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			} else if(mstruct.representsNegative()) {
				o_number.setPlusInfinity();
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			}
		} else if(o_number.isPlusInfinity()) {
			if(mstruct.representsPositive()) {
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			} else if(mstruct.representsNegative()) {
				o_number.setMinusInfinity();
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			}
		}
		if(mstruct.representsReal() && mstruct.representsNonZero()) {
			o_number.setInfinity();
			MERGE_APPROX_AND_PREC(mstruct)
			return 1;
		}
	} else if(mstruct.isNumber() && mstruct.number().isInfinite()) {
		if(mstruct.number().isMinusInfinity()) {
			if(representsPositive()) {
				clear();
				o_number.setMinusInfinity();
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			} else if(representsNegative()) {
				clear();
				o_number.setPlusInfinity();
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			}
		} else if(mstruct.number().isPlusInfinity()) {
			if(representsPositive()) {
				clear();
				o_number.setPlusInfinity();
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			} else if(representsNegative()) {
				clear();
				o_number.setMinusInfinity();
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			}
		}
		if(representsReal() && representsNonZero()) {
			clear();
			o_number.setInfinity();
			MERGE_APPROX_AND_PREC(mstruct)
			return 1;
		}
	}
	if(representsUndefined() || mstruct.representsUndefined()) return -1;
	const MathStructure *mnum = NULL, *mden = NULL;
	if(eo.do_polynomial_division || eo.reduce_divisions) {
		if(!isNumber() && mstruct.isPower() && mstruct[0].isAddition() && mstruct[0].size() > 1 && mstruct[1].isNumber() && mstruct[1].number().isMinusOne()) {
			if((!isPower() || !CHILD(1).hasNegativeSign()) && representsNumber() && mstruct[0].representsNumber() &&  (eo.assume_denominators_nonzero || mstruct[0].representsNonZero())) {
				mnum = this;
				mden = &mstruct[0];
			}
		} else if(!mstruct.isNumber() && isPower() && CHILD(0).isAddition() && CHILD(0).size() > 1 && CHILD(1).isNumber() && CHILD(1).number().isMinusOne()) {
			if((!mstruct.isPower() || !mstruct[1].hasNegativeSign()) && mstruct.representsNumber() && CHILD(0).representsNumber() && (eo.assume_denominators_nonzero || CHILD(0).representsNonZero())) {
				mnum = &mstruct;
				mden = &CHILD(0);
			}
		}
	}
	if(mnum && mden && eo.do_polynomial_division && ((mnum->isAddition() && mnum->size() > 1) || (mden->isAddition() && mden->size() > 1))) {
		const MathStructure *xvar = NULL;
		const Number *xexp = NULL;
		const MathStructure *mnum_first;
		if(mnum->isAddition() && mnum->size() > 1) {
			mnum_first = &(*mnum)[0];
		} else {
			mnum_first = mnum;
		}
		if(mnum_first->isNumber()) {
		} else if(mnum_first->isPower() && (*mnum_first)[0].size() == 0 && (*mnum_first)[1].isNumber() && (*mnum_first)[1].number().isInteger() && (*mnum_first)[1].number().isPositive()) {
			xvar = &(*mnum_first)[0];
			xexp = &(*mnum_first)[1].number();
		} else if(mnum_first->isMultiplication() && mnum_first->size() == 2 && (*mnum_first)[0].isNumber()) {
			if((*mnum_first)[1].isPower()) {
				if((*mnum_first)[1][0].size() == 0 && (*mnum_first)[1][1].isNumber() && (*mnum_first)[1][1].number().isInteger() && (*mnum_first)[1][1].number().isPositive()) {
					xvar = &(*mnum_first)[1][0];
					xexp = &(*mnum_first)[1][1].number();
				}
			} else if((*mnum_first)[1].size() == 0) {
				xvar = &(*mnum_first)[1];
			}
		} else if(mnum_first->size() == 0) {
			xvar = mnum_first;
		}
		if(xvar) {
			bool b = false;
			const MathStructure *mden_first;
			if(mden->isAddition() && mden->size() > 1) {
				mden_first = &(*mden)[0];
			} else {
				mden_first = mden;
			}
			if(mden_first->isPower() && xexp && (*mden_first)[0].size() == 0 && (*mden_first)[1].isNumber() && xvar->equals((*mden_first)[0]) && (*mden_first)[1].number().isInteger() && (*mden_first)[1].number().isPositive() && !xexp->isLessThan((*mden_first)[1].number())) {
				b = true;
			} else if(mden_first->isMultiplication() && mden_first->size() == 2 && (*mden_first)[0].isNumber()) {
				if((*mden_first)[1].isPower()) {
					if(xexp && (*mden_first)[1][0].size() == 0 && (*mden_first)[1][1].isNumber() && xvar->equals((*mden_first)[1][0]) && (*mden_first)[1][1].number().isInteger() && (*mden_first)[1][1].number().isPositive() && !xexp->isLessThan((*mden_first)[1][1].number())) {
						b = true;
					}
				} else if(xvar->equals((*mden_first)[1])) {
					b = true;
				}
			} else if(mden_first->equals(*xvar)) {
				b = true;
			}
			bool had_num = false;
			for(unsigned int i = 1; b && mnum->isAddition() && i < mnum->size(); i++) {
				switch((*mnum)[i].type()) {
					case STRUCT_NUMBER: {
						if(had_num) {
							b = false;
						} else {
							had_num = true;
						}
						break;
					}
					case STRUCT_POWER: {
						if(!(*mnum)[i][1].isNumber() || !xvar->equals((*mnum)[i][0]) || !(*mnum)[i][1].number().isInteger() || !(*mnum)[i][1].number().isPositive()) {
							b = false;
						}
						break;
					}
					case STRUCT_MULTIPLICATION: {
						if(!(*mnum)[i].size() == 2 || !(*mnum)[i][0].isNumber()) {
							b = false;
						} else if((*mnum)[i][1].isPower()) {
							if(!(*mnum)[i][1][1].isNumber() || !xvar->equals((*mnum)[i][1][0]) || !(*mnum)[i][1][1].number().isInteger() || !(*mnum)[i][1][1].number().isPositive()) {
								b = false;
							}
						} else if(!xvar->equals((*mnum)[i][1])) {
							b = false;
						}
						break;
					}
					default: {
						if(!xvar->equals((*mnum)[i])) {
							b = false;
						}
					}
				}
			}
			had_num = false;
			for(unsigned int i = 1; b && mden->isAddition() && i < mden->size(); i++) {
				switch((*mden)[i].type()) {
					case STRUCT_NUMBER: {
						if(had_num) {
							b = false;
						} else {
							had_num = true;
						}
						break;
					}
					case STRUCT_POWER: {
						if(!(*mden)[i][1].isNumber() || !xvar->equals((*mden)[i][0]) || !(*mden)[i][1].number().isInteger() || !(*mden)[i][1].number().isPositive()) {
							b = false;
						}
						break;
					}
					case STRUCT_MULTIPLICATION: {
						if(!(*mden)[i].size() == 2 || !(*mden)[i][0].isNumber()) {
							b = false;
						} else if((*mden)[i][1].isPower()) {
							if(!(*mden)[i][1][1].isNumber() || !xvar->equals((*mden)[i][1][0]) || !(*mden)[i][1][1].number().isInteger() || !(*mden)[i][1][1].number().isPositive()) {
								b = false;
							}
						} else if(!xvar->equals((*mden)[i][1])) {
							b = false;
						}
						break;
					}
					default: {
						if(!xvar->equals((*mden)[i])) {
							b = false;
						}
					}
				}
			}
			if(b) {
				vector<Number> nfactors;
				vector<Number> nexps;
				vector<Number> dfactors;
				vector<Number> dexps;
				for(unsigned int i = 0; b && i == 0 || (i < mnum->size()); i++) {
					if(i > 0) mnum_first = &(*mnum)[i];
					switch(mnum_first->type()) {
						case STRUCT_NUMBER: {
							nfactors.push_back(mnum_first->number());
							nexps.push_back(Number());
							break;
						}
						case STRUCT_POWER: {
							nfactors.push_back(Number(1, 1));
							nexps.push_back((*mnum_first)[1].number());
							break;
						}
						case STRUCT_MULTIPLICATION: {
							nfactors.push_back((*mnum_first)[0].number());
							if((*mnum_first)[1].isPower()) {
								nexps.push_back((*mnum_first)[1][1].number());
							} else {
								nexps.push_back(Number(1, 1));
							}
							break;
						}
						default: {
							nfactors.push_back(Number(1, 1));
							nexps.push_back(Number(1, 1));
						}
					}
					if(!mnum->isAddition()) break;
				}
				for(unsigned int i = 0; b && i == 0 || (i < mden->size()); i++) {
					if(i > 0) mden_first = &(*mden)[i];
					switch(mden_first->type()) {
						case STRUCT_NUMBER: {
							dfactors.push_back(mden_first->number());
							dexps.push_back(Number());
							break;
						}
						case STRUCT_POWER: {
							dfactors.push_back(Number(1, 1));
							dexps.push_back((*mden_first)[1].number());
							break;
						}
						case STRUCT_MULTIPLICATION: {
							dfactors.push_back((*mden_first)[0].number());
							if((*mden_first)[1].isPower()) {
								dexps.push_back((*mden_first)[1][1].number());
							} else {
								dexps.push_back(Number(1, 1));
							}
							break;
						}
						default: {
							dfactors.push_back(Number(1, 1));
							dexps.push_back(Number(1, 1));
						}
					}
					if(!mden->isAddition()) break;
				}
				bool approx = b_approx || mstruct.isApproximate();
				int prec = i_precision;
				if(prec < 1 || mstruct.precision() < prec) prec = mstruct.precision();
				MathStructure xvar2(*xvar);
				clear();
				MathStructure *mtmp;
				Number nftmp, netmp, nfac, nexp;
				bool bdone;
				while(true) {
					if(nexps.empty() || nexps[0].isLessThan(dexps[0])) break;
					nfac = nfactors[0];
					nfac /= dfactors[0];
					nexp = nexps[0];
					nexp -= dexps[0];
					if(isZero()) {
						mtmp = this;
					} else {
						add(m_zero, true);
						mtmp = &CHILD(SIZE - 1);
					}
					if(!nfac.isOne()) {
						mtmp->set(nfac);
						if(!nexp.isZero()) {
							mtmp->multiply(xvar2);
							if(!nexp.isOne()) {
								(*mtmp)[1].raise(nexp);
							}
						}
					} else {
						mtmp->set(xvar2);
						if(!nexp.isOne()) {
							mtmp->raise(nexp);
						}
					}
					nfactors.erase(nfactors.begin());
					nexps.erase(nexps.begin());
					for(unsigned int i = 1; i < dfactors.size(); i++) {
						nftmp = dfactors[i];
						nftmp *= nfac;
						netmp = dexps[i];
						netmp += nexp;
						if(nfactors.empty()) {
							nftmp.negate();
							nfactors.push_back(nftmp);
							nexps.push_back(netmp);
						} else {
							bdone = false;
							for(unsigned int i = 0; i < nfactors.size(); i++) {
								if(netmp == nexps[i]) {
									if(nfactors[i] == nftmp) {
										nfactors.erase(nfactors.begin() + i);
										nexps.erase(nexps.begin() + i);
									} else {
										nfactors[i] -= nftmp;
									}
									bdone = true;
									break;
								} else if(netmp.isGreaterThan(nexps[i])) {
									nftmp.negate();
									nfactors.insert(nfactors.begin() + i, nftmp);
									nexps.insert(nexps.begin() + i, netmp);
									bdone = true;
									break;
								}
							}
							if(!bdone) {
								nftmp.negate();
								nfactors.push_back(nftmp);
								nexps.push_back(netmp);
							}
						}
					}
				}
				if(!nexps.empty()) {
					add(m_zero, true);
					CHILD(SIZE - 1).transform(STRUCT_MULTIPLICATION);
					for(unsigned int i = 0; i < nexps.size(); i++) {
						if(CHILD(SIZE - 1)[0].isZero()) {
							mtmp = &CHILD(SIZE - 1)[0];
						} else {
							CHILD(SIZE - 1)[0].add(m_zero, true);
							mtmp = &CHILD(SIZE - 1)[0][CHILD(SIZE - 1)[0].size() - 1];
						}
						if(!nfactors[i].isOne()) {
							mtmp->set(nfactors[i]);
							if(!nexps[i].isZero()) {
								mtmp->multiply(xvar2);
								if(!nexps[i].isOne()) {
									(*mtmp)[1].raise(nexps[i]);
								}
							}
						} else {
							mtmp->set(xvar2);
							if(!nexps[i].isOne()) {
								mtmp->raise(nexps[i]);
							}
						}
					}
					CHILD(SIZE - 1).multiply(m_zero, true);
					CHILD(SIZE - 1)[1].raise(m_minus_one);
					for(unsigned int i = 0; i < dexps.size(); i++) {
						if(CHILD(SIZE - 1)[1][0].isZero()) {
							mtmp = &CHILD(SIZE - 1)[1][0];
						} else {
							CHILD(SIZE - 1)[1][0].add(m_zero, true);
							mtmp = &CHILD(SIZE - 1)[1][0][CHILD(SIZE - 1)[1][0].size() - 1];
						}
						if(!dfactors[i].isOne()) {
							mtmp->set(dfactors[i]);
							if(!dexps[i].isZero()) {
								mtmp->multiply(xvar2);
								if(!dexps[i].isOne()) {
									(*mtmp)[1].raise(dexps[i]);
								}
							}
						} else {
							mtmp->set(xvar2);
							if(!dexps[i].isOne()) {
								mtmp->raise(dexps[i]);
							}
						}
					}
				}
				i_precision = prec;
				b_approx = approx;
				return 1;
			}
		}
	}
	if(mnum && mden && eo.reduce_divisions && mden->isAddition() && mden->size() > 1) {
		switch(mnum->type()) {
			case STRUCT_ADDITION: {
				break;
			}
			case STRUCT_MULTIPLICATION: {
				Number nr;
				vector<Number> nrs;
				vector<unsigned int> reducables;
				for(unsigned int i = 0; i < mnum->size(); i++) {
					switch((*mnum)[i].type()) {
						case STRUCT_ADDITION: {break;}
						case STRUCT_POWER: {
							if((*mnum)[i][1].isNumber() && (*mnum)[i][1].number().isReal()) {
								if((*mnum)[i][1].number().isPositive()) {
									nr.set((*mnum)[i][1].number());
									if(reducable((*mnum)[i][0], *mden, nr)) {
										nrs.push_back(nr);
										reducables.push_back(i);
									}
								}
								break;
							}
						}
						default: {
							nr.set(1, 1);
							if(reducable((*mnum)[i], *mden, nr)) {
								nrs.push_back(nr);
								reducables.push_back(i);
							}
						}
					}
				}
				if(reducables.size() > 0) {
					if(mnum == this) {
						transform(STRUCT_MULTIPLICATION, mstruct);
					} else {
						transform(STRUCT_MULTIPLICATION);
						PREPEND(mstruct);
					}
					unsigned int i_erased = 0;
					for(unsigned int i = 0; i < reducables.size(); i++) {
						switch(CHILD(0)[reducables[i] - i_erased].type()) {
							case STRUCT_POWER: {
								if(CHILD(0)[reducables[i] - i_erased][1].isNumber() && CHILD(0)[reducables[i] - i_erased][1].number().isReal()) {
									reduce(CHILD(0)[reducables[i] - i_erased][0], CHILD(1)[0], nrs[i]);
									if(nrs[i] == CHILD(0)[reducables[i] - i_erased][1].number()) {
										CHILD(0).delChild(reducables[i] - i_erased + 1);
										i_erased++;
									} else {
										CHILD(0)[reducables[i] - i_erased][1].number() -= nrs[i];
										if(CHILD(0)[reducables[i] - i_erased][1].number().isOne()) {
											MathStructure msave(CHILD(0)[reducables[i] - i_erased][0]);
											CHILD(0)[reducables[i] - i_erased] = msave;
										}
									}
									break;
								}
							}
							default: {
								reduce(CHILD(0)[reducables[i] - i_erased], CHILD(1)[0], nrs[i]);
								if(nrs[i].isOne()) {
									CHILD(0).delChild(reducables[i] - i_erased + 1);
									i_erased++;
								} else {
									MathStructure mexp(1, 1);
									mexp.number() -= nrs[i];
									CHILD(0)[reducables[i] - i_erased].raise(mexp);
								}
							}
						}
					}
					if(CHILD(0).size() == 0) {
						MathStructure msave(CHILD(1));
						set(msave);
					} else if(CHILD(0).size() == 1) {
						MathStructure msave(CHILD(0)[0]);
						CHILD(0) == msave;
					}
					return 1;
				}
				break;
			}
			case STRUCT_POWER: {
				if((*mnum)[1].isNumber() && (*mnum)[1].number().isReal()) {
					if((*mnum)[1].number().isPositive()) {
						Number nr((*mnum)[1].number());
						if(reducable((*mnum)[0], *mden, nr)) {
							if(nr != (*mnum)[1].number()) {
								MathStructure mnum2((*mnum)[0]);
								if(mnum == this) {
									CHILD(1).number() -= nr;
									if(CHILD(1).number().isOne()) {
										set(mnum2);
									}
									transform(STRUCT_MULTIPLICATION, mstruct);
								} else {
									transform(STRUCT_MULTIPLICATION);
									PREPEND(mstruct);
									CHILD(0)[1].number() -= nr;
									if(CHILD(0)[1].number().isOne()) {
										CHILD(0) = mnum2;
									}
								}
								reduce(mnum2, CHILD(1)[0], nr);
							} else {
								if(mnum == this) {
									MathStructure mnum2((*mnum)[0]);
									set(mstruct);
									reduce(mnum2, CHILD(0), nr);
								} else {
									reduce((*mnum)[0], CHILD(0), nr);
								}
							}
							return 1;
						}
					}
					break;
				}
			}
			default: {
				Number nr(1, 1);
				if(reducable(*mnum, *mden, nr)) {
					if(mnum == this) {
						MathStructure mnum2(*mnum);
						set(mstruct);
						reduce(mnum2, CHILD(0), nr);
					} else {
						reduce(*mnum, CHILD(0), nr);
					}
					return 1;
				}
			}
		}
	}
	switch(m_type) {
		case STRUCT_VECTOR: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {
					if(isMatrix() && mstruct.isMatrix()) {
						if(CHILD(0).size() != mstruct.size()) {
							CALCULATOR->error(true, _("The second matrix must have as many rows (was %s) as the first has columns (was %s) for matrix multiplication."), i2s(mstruct.size()).c_str(), i2s(CHILD(0).size()).c_str(), NULL);
							return -1;
						}
						MathStructure msave(*this);
						clearMatrix();
						resizeMatrix(size(), mstruct[0].size(), m_zero);
						MathStructure mtmp;
						for(unsigned int index_r = 0; index_r < SIZE; index_r++) {
							for(unsigned int index_c = 0; index_c < CHILD(0).size(); index_c++) {
								for(unsigned int index = 0; index < msave[0].size(); index++) {
									mtmp = msave[index_r][index];
									mtmp *= mstruct[index][index_c];
									CHILD(index_r)[index_c] += mtmp;
								}			
							}		
						}
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					} else {
						if(SIZE == mstruct.size()) {
							for(unsigned int i = 0; i < SIZE; i++) {
								CHILD(i) *= mstruct[i];
							}
							m_type = STRUCT_ADDITION;
							MERGE_APPROX_AND_PREC(mstruct)
							return 1;
						}
					}
					return -1;
				}
				default: {
					for(unsigned int i = 0; i < SIZE; i++) {
						CHILD(i) *= mstruct;
					}
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
			}
		}
		case STRUCT_ADDITION: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {
					return 0;
				}
				case STRUCT_ADDITION: {
					MathStructure msave(*this);
					CLEAR;
					for(unsigned int i = 0; i < mstruct.size(); i++) {
						APPEND(msave);
						CHILD(i) *= mstruct[i];
					}
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
				case STRUCT_POWER: {
					if(mstruct[1].isNumber() && *this == mstruct[0]) {
						if(representsNonNegative() || mstruct[1].representsPositive()) {
							set(mstruct);
							CHILD(1) += m_one;
							return 1;
						}
					}
					if(mstruct[1].hasNegativeSign()) {
						int ret;
						bool merged[SIZE]; unsigned int merges = 0;
						for(unsigned int i = 0; i < SIZE; i++) {
							if(CHILD(i).isOne()) ret = -1;
							else ret = CHILD(i).merge_multiplication(mstruct, eo, false);
							if(ret == 0) {
								MathStructure mstruct2(mstruct);
								ret = mstruct2.merge_multiplication(CHILD(i), eo, false);
								if(ret == 1) {
									CHILD(i) = mstruct2;
								}
							}
							if(ret == 1) {
								merged[i] = true;
								merges++;
							} else {
								merged[i] = false;
							}
						}
						if(merges == 0) {
							return -1;
						} else if(merges == SIZE) {
							return 1;
						} else if(merges == SIZE - 1) {
							for(unsigned int i = 0; i < SIZE; i++) {
								if(!merged[i]) {
									CHILD(i) *= mstruct;
									break;
								}
							}
						} else {
							MathStructure mdiv;
							merges = 0;
							for(unsigned int i = 0; i - merges < SIZE; i++) {
								if(!merged[i]) {
									if(merges > 0) {
										mdiv[0].add(CHILD(i - merges), merges > 1);
									} else {
										mdiv *= mstruct;
										mdiv[0] = CHILD(i - merges);
									}
									ERASE(i - merges);
									merges++;
								}
							}
							add(mdiv, SIZE > 1);
						}
						return 1;
					}
				}
				case STRUCT_MULTIPLICATION: {
				}
				default: {
					for(unsigned int i = 0; i < SIZE; i++) {
						CHILD(i).multiply(mstruct);
					}
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
			}
			return -1;
		}
		case STRUCT_MULTIPLICATION: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {}
				case STRUCT_ADDITION: {
					return 0;
				}
				case STRUCT_MULTIPLICATION: {
					for(unsigned int i = 0; i < mstruct.size(); i++) {
						APPEND(mstruct[i]);
					}
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
				case STRUCT_POWER: {
					if(mstruct[1].isNumber() && *this == mstruct[0]) {
						if(representsNonNegative() || mstruct[1].representsPositive()) {
							set(mstruct);
							CHILD(1) += m_one;
							return 1;
						}
					}
				}
				default: {
					if(do_append) {
						APPEND(mstruct);
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
				}
			}
			return -1;
		}
		case STRUCT_POWER: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {}
				case STRUCT_ADDITION: {}
				case STRUCT_MULTIPLICATION: {
					return 0;
				}
				case STRUCT_POWER: {
					if(mstruct[0] == CHILD(0)) {
						if(CHILD(0).representsNonZero() || (eo.assume_denominators_nonzero && !CHILD(1).isZero()) || (CHILD(1).representsPositive() && mstruct[1].representsPositive())) {
							MathStructure mstruct2(CHILD(1));
							mstruct2 += mstruct[1];
							if(mstruct2.calculatesub(eo, eo)) {
								CHILD(1) = mstruct2;
								return 1;
							}
						}
					} else if(mstruct[1] == CHILD(1)) {
						MathStructure mstruct2(CHILD(0));
						mstruct2 *= mstruct[0];
						if(mstruct2.calculatesub(eo, eo)) {
							CHILD(0) = mstruct2;
							MERGE_APPROX_AND_PREC(mstruct)
							return 1;
						}
					}
					break;
				}
				default: {
					if(!mstruct.isNumber() && CHILD(1).isNumber() && CHILD(0) == mstruct) {
						if(CHILD(0).representsNonZero() || (eo.assume_denominators_nonzero && !CHILD(1).isZero()) || CHILD(1).representsPositive()) {
							CHILD(1) += m_one;
							MERGE_APPROX_AND_PREC(mstruct)
							return 1;
						}
					}
					if(mstruct.isZero()) {
						clear();
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
					break;
				}
			}
			return -1;
		}
		default: {
			switch(mstruct.type()) {
				case STRUCT_VECTOR: {}
				case STRUCT_ADDITION: {}
				case STRUCT_MULTIPLICATION: {}
				case STRUCT_POWER: {
					return 0;
				}
				default: {
					if((mstruct.isZero() && !representsUndefined(true, true)) || (isZero() && !mstruct.representsUndefined(true, true))) {
						clear(); 
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
					if(equals(mstruct)) {
						raise(2);
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
					break;
				}
			}
			break;
		}			
	}
	return -1;
}

int MathStructure::merge_power(const MathStructure &mstruct, const EvaluationOptions &eo) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		Number nr(o_number);
		if(nr.raise(mstruct.number()) && (eo.approximation == APPROXIMATION_APPROXIMATE || !nr.isApproximate() || o_number.isApproximate() || mstruct.number().isApproximate()) && (eo.allow_complex || !nr.isComplex() || o_number.isComplex() || mstruct.number().isComplex()) && (eo.allow_infinite || !nr.isInfinite() || o_number.isInfinite() || mstruct.number().isInfinite())) {
			o_number = nr;
			numberUpdated();
			return 1;
		}
		if(mstruct.number().isRational()) {
			if(!mstruct.number().numeratorIsOne()) {
				nr = o_number;
				if(nr.raise(mstruct.number().numerator()) && (eo.approximation == APPROXIMATION_APPROXIMATE || !nr.isApproximate() || o_number.isApproximate() || mstruct.number().isApproximate()) && (eo.allow_complex || !nr.isComplex() || o_number.isComplex() || mstruct.number().isComplex()) && (eo.allow_infinite || !nr.isInfinite() || o_number.isInfinite() || mstruct.number().isInfinite())) {
					o_number = nr;
					numberUpdated();
					nr.set(mstruct.number().denominator());
					nr.recip();
					raise(nr);
					return 1;
				}
			} else if(eo.split_squares && o_number.isInteger() && mstruct.number().denominatorIsTwo()) {
				nr.set(1, 1);
				bool b = true, overflow;
				int val;
				while(b) {
					b = false;
					overflow = false;
					val = o_number.intValue(&overflow);
					if(overflow) {
						cln::cl_I cval = cln::numerator(cln::rational(cln::realpart(o_number.internalNumber())));
						for(unsigned int i = 0; i < NR_OF_PRIMES; i++) {
							if(cln::zerop(cln::rem(cval, SQUARE_PRIMES[i]))) {
								nr *= PRIMES[i];
								o_number /= SQUARE_PRIMES[i];
								b = true;
								break;
							}
						}
					} else {
						for(unsigned int i = 0; i < NR_OF_PRIMES; i++) {
							if(SQUARE_PRIMES[i] > val) {
								break;
							} else if(val % SQUARE_PRIMES[i] == 0) {
								nr *= PRIMES[i];
								o_number /= SQUARE_PRIMES[i];
								b = true;
								break;
							}
						}
					}
				}
				if(!nr.isOne()) {
					transform(STRUCT_MULTIPLICATION);
					CHILD(0) ^= mstruct;
					PREPEND(nr);
					return 1;
				}
			}
		}
		return -1;
	}
	if(mstruct.isOne()) {
		MERGE_APPROX_AND_PREC(mstruct)
		return 1;
	}
	if(m_type == STRUCT_NUMBER && o_number.isInfinite()) {
		if(mstruct.representsNegative()) {
			o_number.clear();
			MERGE_APPROX_AND_PREC(mstruct)
			return 1;
		} else if(mstruct.representsNonZero() && mstruct.representsNumber()) {
			if(o_number.isMinusInfinity()) {
				if(mstruct.representsEven()) {
					o_number.setPlusInfinity();
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				} else if(mstruct.representsOdd()) {
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				} else if(mstruct.representsInteger()) {
					o_number.setInfinity();
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
			}
		}
	} else if(mstruct.isNumber() && mstruct.number().isInfinite()) {
		if(mstruct.number().isInfinity()) {
		} else if(mstruct.number().isMinusInfinity()) {
			if(representsReal() && representsNonZero()) {
				clear();
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			}
		} else if(mstruct.number().isPlusInfinity()) {
			if(representsPositive()) {
				int prev_prec = i_precision;
				bool prev_approx = b_approx;
				set(mstruct);
				if(prev_approx) b_approx = true;
				if(prev_prec > 0 && (i_precision < 1 || prev_prec < i_precision)) i_precision = prev_prec;
				return 1;
			}
		}
	}
	if(representsUndefined() || mstruct.representsUndefined()) return -1;
	if(isZero() && mstruct.representsPositive()) {
		return 1;
	}
	if(mstruct.isZero() && !representsUndefined(true, true)) {
		set(1, 1);
		MERGE_APPROX_AND_PREC(mstruct)
		return 1;
	}
	switch(m_type) {
		case STRUCT_VECTOR: {
			if(mstruct.isNumber() && mstruct.number().isInteger()) {
				if(isMatrix()) {
					if(matrixIsSymmetric()) {
						Number nr(mstruct.number());
						bool b_neg = false;
						if(nr.isNegative()) {
							nr.setNegative(false);
							b_neg = true;
						}
						if(!nr.isOne()) {
							MathStructure msave(*this);
							nr--;
							while(nr.isPositive()) {
								merge_multiplication(msave, eo);
								nr--;
							}
						}
						if(b_neg) {
							invertMatrix(eo);
						}
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
					return -1;
				} else {
					if(mstruct.number().isMinusOne()) {
						return -1;
					}
					Number nr(mstruct.number());
					if(nr.isNegative()) {
						nr++;
					} else {
						nr--;
					}
					MathStructure msave(*this);
					merge_multiplication(msave, eo);
					raise(nr);
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
				
			}
			goto default_power_merge;
		}
		case STRUCT_ADDITION: {
			if(mstruct.isNumber() && mstruct.number().isInteger()) {
				if(mstruct.number().isMinusOne()) {
					bool bnum = false;
					for(unsigned int i = 0; !bnum && i < SIZE; i++) {
						switch(CHILD(i).type()) {
							case STRUCT_NUMBER: {
								if(!CHILD(i).number().isZero() && CHILD(i).number().isReal()) {
									bnum = true;
								}
								break;
							}
							case STRUCT_MULTIPLICATION: {
								if(CHILD(i).size() > 0 && CHILD(i)[0].isNumber()) {
									if(!CHILD(i)[0].number().isZero() && CHILD(i)[0].number().isReal()) {
										bnum = true;
									}
									break;
								}
							}
						}
					}
					if(bnum) {
						Number nr;
						unsigned int negs = 0;
						for(unsigned int i = 0; i < SIZE; i++) {
							switch(CHILD(i).type()) {
								case STRUCT_NUMBER: {
									if(!CHILD(i).number().isZero() && CHILD(i).number().isReal()) {
										if(CHILD(i).number().isNegative()) negs++;
										if(nr.isZero()) {
											nr = CHILD(i).number();
										} else {
											if(negs) {
												if(CHILD(i).number().isNegative()) {
													nr.setNegative(true);
													if(CHILD(i).number().isGreaterThan(nr)) {
														nr = CHILD(i).number();
													}
													break;
												} else {
													nr.setNegative(false);
												}
											}
											if(CHILD(i).number().isLessThan(nr)) {
												nr = CHILD(i).number();
											}
										}
									}
									break;
								}
								case STRUCT_MULTIPLICATION: {
									if(CHILD(i).size() > 0 && CHILD(i)[0].isNumber()) {
										if(!CHILD(i)[0].number().isZero() && CHILD(i)[0].number().isReal()) {
											if(CHILD(i)[0].number().isNegative()) negs++;
											if(nr.isZero()) {
												nr = CHILD(i)[0].number();
											} else {
												if(negs) {
													if(CHILD(i)[0].number().isNegative()) {
														nr.setNegative(true);
														if(CHILD(i)[0].number().isGreaterThan(nr)) {
															nr = CHILD(i)[0].number();
														}
														break;
													} else {
														nr.setNegative(false);
													}
												}
												if(CHILD(i)[0].number().isLessThan(nr)) {
													nr = CHILD(i)[0].number();
												}
											}
										}
										break;
									}
								}
								default: {
									if(nr.isZero() || !nr.isFraction()) {
										nr.set(1, 1);
									}
								}
							}
							
						}
						nr.setNegative(negs > SIZE - negs);
						if(bnum && !nr.isOne() && !nr.isZero()) {
							nr.recip();
							for(unsigned int i = 0; i < SIZE; i++) {
								switch(CHILD(i).type()) {
									case STRUCT_NUMBER: {
										CHILD(i).number() *= nr;
										break;
									}
									case STRUCT_MULTIPLICATION: {
										CHILD(i)[0].number() *= nr;
										break;
									}
									default: {
										CHILD(i) *= nr;
									}
								}
							}
							raise(mstruct);
							multiply(nr);
							return 1;
						}
					}
				} else if(eo.simplify_addition_powers && !mstruct.number().isZero()) {
					bool neg = mstruct.number().isNegative();
					MathStructure mstruct1(CHILD(0));
					MathStructure mstruct2(CHILD(1));
					for(unsigned int i = 2; i < SIZE; i++) {
						mstruct2.add(CHILD(i), true);
					}
					Number m(mstruct.number());
					m.setNegative(false);
					Number k(1);
					Number p1(m);
					Number p2(1);
					p1--;
					Number bn;
					CLEAR
					APPEND(mstruct1);
					CHILD(0).raise(m);
					while(k.isLessThan(m)) {
						bn.binomial(m, k);
						APPEND(MathStructure(bn));
						LAST.multiply(mstruct1);
						if(!p1.isOne()) {
							LAST[1].raise(p1);
						}
						LAST.multiply(mstruct2, true);
						if(!p2.isOne()) {
							LAST[2].raise(p2);
						}
						k++;
						p1--;
						p2++;
					}
					APPEND(mstruct2);
					LAST.raise(m);
					if(neg) inverse();
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
			}
			goto default_power_merge;
		}
		case STRUCT_MULTIPLICATION: {
			if(mstruct.representsInteger()) {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).raise(mstruct);	
				}
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			} else {
				bool b = true;
				for(unsigned int i = 0; i < SIZE; i++) {
					if(!CHILD(i).representsNonNegative()) {
						b = false;
					}
				}
				if(b) {
					for(unsigned int i = 0; i < SIZE; i++) {
						CHILD(i).raise(mstruct);	
					}
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
			}
			goto default_power_merge;
		}
		case STRUCT_POWER: {
			if(mstruct.representsInteger() || CHILD(0).representsReal()) {
				if(CHILD(1).isNumber() && CHILD(1).number().isRational()) {
					if(CHILD(1).number().numeratorIsEven() && !mstruct.representsInteger()) {
						MathStructure mstruct_base(CHILD(0));
						CHILD(0).set(CALCULATOR->f_abs, &mstruct_base, NULL);
					}
					CHILD(1) *= mstruct;
					MERGE_APPROX_AND_PREC(mstruct)
					return 1;
				}
			}
			goto default_power_merge;
		}
		case STRUCT_VARIABLE: {
			if(o_variable == CALCULATOR->v_e) {
				if(mstruct.isMultiplication() && mstruct.size() == 2 && mstruct[1].isVariable() && mstruct[1].variable() == CALCULATOR->v_pi) {
					if(mstruct[0].isNumber()) {
						if(mstruct[0].number().isI() || mstruct[0].number().isMinusI()) {
							set(-1, 1);
							return 1;
						} else if(mstruct[0].number().isComplex() && !mstruct[0].number().hasRealPart()) {
							Number img(mstruct[0].number().imaginaryPart());
							if(img.isInteger()) {
								if(img.isEven()) {
									set(1, 1);
								} else {
									set(-1, 1);
								}
								MERGE_APPROX_AND_PREC(mstruct)
								return 1;
							} else if(img == Number(1, 2)) {
								clear();
								img.set(1, 1);
								o_number.setImaginaryPart(img);
								MERGE_APPROX_AND_PREC(mstruct)
								return 1;
							} else if(img == Number(-1, 2)) {
								clear();
								img.set(-1, 1);
								o_number.setImaginaryPart(img);
								MERGE_APPROX_AND_PREC(mstruct)
								return 1;
							}
						}
					}
				} else if(mstruct.isFunction() && mstruct.function() == CALCULATOR->f_ln && mstruct.size() == 1) {
					if(mstruct[0].representsPositive() || mstruct[0].representsComplex()) {
						set(mstruct.getChild(0));
						break;
					}
				}
			}
			goto default_power_merge;
		}
		default: {
			default_power_merge:
			if(mstruct.isAddition()) {
				MathStructure msave(*this);
				clear();
				m_type = STRUCT_MULTIPLICATION;
				for(unsigned int i = 0; i < mstruct.size(); i++) {
					APPEND(msave);
					LAST.raise(mstruct[i]);
				}
				MERGE_APPROX_AND_PREC(mstruct)
				return 1;
			} else if(mstruct.isMultiplication() && mstruct.size() > 1) {
				MathStructure mthis(*this);
				for(unsigned int i = 0; i < mstruct.size(); i++) {
					if(i == 0) mthis.raise(mstruct[i]);
					else mthis[1] = mstruct[i];
					if(mthis.calculatesub(eo, eo)) {
						set(mthis);
						if(mstruct.size() == 2) {
							if(i == 0) raise(mstruct[1]);
							else raise(mstruct[0]);
						} else {
							raise(mstruct);
							CHILD(1).delChild(i + 1);
						}
						MERGE_APPROX_AND_PREC(mstruct)
						return 1;
					}
				}
			}
			break;
		}
	}
	return -1;
}

bool MathStructure::calculatesub(const EvaluationOptions &eo, const EvaluationOptions &feo) {
	bool b = true;
	bool c = false;
	while(b) {
		b = false;
		switch(m_type) {
			case STRUCT_VARIABLE: {
				if(eo.calculate_variables && o_variable->isKnown()) {
					if(eo.approximation == APPROXIMATION_APPROXIMATE || !o_variable->isApproximate()) {
						set(((KnownVariable*) o_variable)->get());
						if(eo.calculate_functions) {
							calculateFunctions(feo);
							unformat(feo);
						}
						b = true;
					}
				}
				break;
			}
			case STRUCT_POWER: {
				CHILD(0).calculatesub(eo, feo);
				CHILD(1).calculatesub(eo, feo);
				childrenUpdated();
				if(CHILD(0).merge_power(CHILD(1), eo) == 1) {
					MathStructure new_this(CHILD(0));
					set(new_this);
					b = true;
				}
				break;
			}
			case STRUCT_ADDITION: {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).calculatesub(eo, feo);
				}
				childrenUpdated();
				evalSort();
				for(int i = 0; i < (int) SIZE - 1; i++) {
					for(int i2 = i + 1; i2 < (int) SIZE; i2++) {
						int r = CHILD(i).merge_addition(CHILD(i2), eo);
						if(r == 0) {
							r = CHILD(i2).merge_addition(CHILD(i), eo);
							if(r == 1) {
								CHILD(i) = CHILD(i2);
							}
						}
						if(r == 1) {
							ERASE(i2);
							i2--;
							b = true;
						}
					}
				}
				if(SIZE == 1) {
					MathStructure new_this(CHILD(0));
					set(new_this);
				} else if(SIZE == 0) {
					clear();
				}
				break;
			}
			case STRUCT_MULTIPLICATION: {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).calculatesub(eo, feo);
				}
				childrenUpdated();
				evalSort();
				for(int i = 0; i < (int) SIZE - 1; i++) {
					for(int i2 = i + 1; i2 < (int) SIZE; i2++) {
						int r = CHILD(i).merge_multiplication(CHILD(i2), eo);
						if(r == 0) {
							r = CHILD(i2).merge_multiplication(CHILD(i), eo);
							if(r == 1) {
								CHILD(i) = CHILD(i2);
							}
						}
						if(r == 1) {
							ERASE(i2);
							i2--;
							b = true;
						}
					}
				}
				if(SIZE == 1) {
					MathStructure new_this(CHILD(0));
					set(new_this);
				} else if(SIZE == 0) {
					clear();
				}
				break;
			}
			case STRUCT_AND: {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).calculatesub(eo, feo);
				}
				childrenUpdated();
				bool is_true = true;
				for(int i = 0; i < (int) SIZE; i++) {
					if(CHILD(i).representsNonPositive()) {
						clear();
						b = true;
						is_true = false;
						break;
					} else if(CHILD(i).representsPositive()) {
						b = true;
						ERASE(i);
						i--;
					} else {
						is_true = false;
					}
				}
				if(is_true) {
					set(1, 1);
				} else if(m_type == STRUCT_AND && SIZE == 1) {
					APPEND(MathStructure((int) 0));
					m_type = STRUCT_COMPARISON;
					ct_comp = COMPARISON_GREATER;
				}
				break;
			}
			case STRUCT_OR: {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).calculatesub(eo, feo);
				}
				childrenUpdated();
				bool is_false = true;
				for(int i = 0; i < (int) SIZE; i++) {
					if(CHILD(i).representsNonPositive()) {
						b = true;
						ERASE(i);
						i--;
					} else if(CHILD(i).representsPositive()) {
						b = true;
						set(1, 1);
						is_false = false;
						break;
					} else {
						is_false = false;
					}
				}
				if(is_false) {
					clear();
				} else if(m_type == STRUCT_OR && SIZE == 1) {
					APPEND(MathStructure(int(0)));
					m_type = STRUCT_COMPARISON;
					ct_comp = COMPARISON_GREATER;
				}
				break;
			}
			case STRUCT_XOR: {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).calculatesub(eo, feo);
				}
				childrenUpdated();
				bool is_false = true;
				bool had_true = false;
				for(int i = 0; i < (int) SIZE; i++) {
					if(CHILD(i).representsNonPositive()) {
						b = true;
						ERASE(i);
						i--;
					} else if(CHILD(i).representsPositive()) {
						if(had_true) {
							clear();
							b = true;
							break;
						}
						b = true;
						is_false = false;
						had_true = true;
					} else {
						is_false = false;
					}
				}
				if(is_false) {
					clear();
				} else if(had_true && SIZE == 0) {
					set(1, 1);
				} else {
					if(SIZE == 1) {
						if(had_true) {
							APPEND(MathStructure(int(0)));
							m_type = STRUCT_COMPARISON;
							ct_comp = COMPARISON_EQUALS_LESS;
						} else {
							APPEND(MathStructure(int(0)));
							m_type = STRUCT_COMPARISON;
							ct_comp = COMPARISON_GREATER;
						}
					} else {
						if(had_true) {
							m_type = STRUCT_AND;
							setNOT();
						}
					}
				}
				break;
			}
			case STRUCT_NOT: {
				CHILD(0).calculatesub(eo, feo);
				childrenUpdated();
				if(CHILD(0).representsPositive()) {
					clear();
					b = true;
				} else if(CHILD(0).representsNonPositive()) {
					set(1, 1);
					b = true;
				}
				break;
			}
			case STRUCT_COMPARISON: {
				if(!eo.test_comparisons) {
					CHILD(0).calculatesub(eo, feo);
					CHILD(1).calculatesub(eo, feo);
					childrenUpdated();
					b = false;
					break;
				}
				if(ct_comp == COMPARISON_EQUALS || ct_comp == COMPARISON_NOT_EQUALS) {
					if((CHILD(0).representsReal() && CHILD(1).representsComplex()) || (CHILD(1).representsReal() && CHILD(0).representsComplex())) {
						if(ct_comp == COMPARISON_EQUALS) {
							clear();
						} else {
							clear();
							o_number.set(1, 1);
						}
						b = true;
						break;
					}
				}
				if(!CHILD(1).isZero()) {
					CHILD(0) -= CHILD(1);
					CHILD(0).calculatesub(eo, feo);
					CHILD(1).clear();
					b = true;
				} else {
					CHILD(0).calculatesub(eo, feo);
				}
				childrenUpdated();
				bool incomp = false;
				if(CHILD(0).isAddition()) {
					for(unsigned int i = 1; i < CHILD(0).size(); i++) {
						if(!CHILD(0)[i - 1].isUnitCompatible(CHILD(0)[i])) {
							incomp = true;
							break;
						}
					}
				}
				switch(ct_comp) {
					case COMPARISON_EQUALS: {
						if(incomp) {
							clear();
							b = true;
						} else if(CHILD(0).isZero()) {
							clear();
							o_number.set(1, 1);
							b = true;	
						} else if(CHILD(0).representsNonZero()) {
							clear();
							b = true;
						}
						break;
					}
					case COMPARISON_NOT_EQUALS:  {
						if(incomp) {
							clear();
							o_number.set(1, 1);
							b = true;
						} else if(CHILD(0).representsNonZero()) {
							clear();
							o_number.set(1, 1);
							b = true;	
						} else if(CHILD(0).isZero()) {
							clear();
							b = true;
						}
						break;
					}
					case COMPARISON_LESS:  {
						if(incomp) {
						} else if(CHILD(0).representsNegative()) {
							clear();
							o_number.set(1, 1);
							b = true;	
						} else if(CHILD(0).representsNonNegative()) {
							clear();
							b = true;
						}
						break;
					}
					case COMPARISON_GREATER:  {
						if(incomp) {
						} else if(CHILD(0).representsPositive()) {
							clear();
							o_number.set(1, 1);
							b = true;	
						} else if(CHILD(0).representsNonPositive()) {
							clear();
							b = true;
						}
						break;
					}
					case COMPARISON_EQUALS_LESS:  {
						if(incomp) {
						} else if(CHILD(0).representsNonPositive()) {
							clear();
							o_number.set(1, 1);
							b = true;	
						} else if(CHILD(0).representsPositive()) {
							clear();
							b = true;
						}
						break;
					}
					case COMPARISON_EQUALS_GREATER:  {
						if(incomp) {
						} else if(CHILD(0).representsNonNegative()) {
							clear();
							o_number.set(1, 1);
							b = true;	
						} else if(CHILD(0).representsNegative()) {
							clear();
							b = true;
						}
						break;
					}
				}
				break;
			}
			case STRUCT_FUNCTION: {
				if(o_function == CALCULATOR->f_abs) {
					calculateFunctions(eo);
					unformat(eo);
					b = m_type != STRUCT_FUNCTION;
					break;
				}
			}
			default: {
				for(unsigned int i = 0; i < SIZE; i++) {
					if(CHILD(i).calculatesub(eo, feo)) c = true;
				}
				childrenUpdated();
			}
		}
		if(b) c = true;
	}
	return c;
}

bool MathStructure::calculateFunctions(const EvaluationOptions &eo) {
	if(m_type == STRUCT_FUNCTION) {
		printf("%i %i\n", i_precision, b_approx);
		if(!o_function->testArgumentCount(SIZE)) {
			return false;
		}
		if(o_function->maxargs() > -1 && (int) SIZE > o_function->maxargs()) {
			REDUCE(o_function->maxargs());
		}
		m_type = STRUCT_VECTOR;
		Argument *arg = NULL, *last_arg = NULL;
		int last_i = 0;

		for(unsigned int i = 0; i < SIZE; i++) {
			arg = o_function->getArgumentDefinition(i + 1);
			if(arg) {
				last_arg = arg;
				last_i = i;
				if(!arg->test(CHILD(i), i + 1, o_function, eo)) {
					m_type = STRUCT_FUNCTION;
					return false;
				}
			}
		}

		if(last_arg && o_function->maxargs() < 0 && last_i >= o_function->minargs()) {
			for(unsigned int i = last_i + 1; i < SIZE; i++) {
				if(!last_arg->test(CHILD(i), i + 1, o_function, eo)) {
					m_type = STRUCT_FUNCTION;
					return false;
				}
			}
		}

		if(!o_function->testCondition(*this)) {
			m_type = STRUCT_FUNCTION;
			return false;
		}

		MathStructure mstruct;
		int i = o_function->calculate(mstruct, *this, eo);
		if(i > 0) {
			printf("%i %i\n", i_precision, b_approx);
			if(b_approx && !mstruct.isApproximate()) mstruct.setApproximate(); if(i_precision > 0 && (mstruct.precision() < 1 || i_precision < mstruct.precision())) mstruct.setPrecision(i_precision);
			set(mstruct);
			calculateFunctions(eo);
			return true;
		} else {
			if(i < 0) {
				i = -i;
				if(i <= (int) SIZE) CHILD(i - 1) = mstruct;
			}
			m_type = STRUCT_FUNCTION;
			return false;
		}
		return false;
	}
	bool b = false;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).calculateFunctions(eo)) b = true;
	}
	if(b) childrenUpdated();
	return b;
}

int evalSortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2, const MathStructure &parent);
int evalSortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2, const MathStructure &parent) {
	if(parent.isMultiplication()) {
		if(mstruct1.containsType(STRUCT_VECTOR) && mstruct2.containsType(STRUCT_VECTOR)) {
			return 0;
		}
	}
	if(parent.isAddition()) {
		if(mstruct1.isMultiplication() && mstruct1.size() > 0) {
			unsigned int start = 0;
			while(mstruct1[start].isNumber() && mstruct1.size() > start + 1) {
				start++;
			}
			int i2;
			if(mstruct2.isMultiplication()) {
				if(mstruct2.size() < 1) return -1;
				unsigned int start2 = 0;
				while(mstruct2[start2].isNumber() && mstruct2.size() > start2 + 1) {
					start2++;
				}
				for(unsigned int i = 0; i + start < mstruct1.size(); i++) {
					if(i + start2 >= mstruct2.size()) return 1;
					i2 = evalSortCompare(mstruct1[i + start], mstruct2[i + start2], parent);
					if(i2 != 0) return i2;
				}
				if(mstruct1.size() - start == mstruct2.size() - start2) return 0;
				return -1;
			} else {
				i2 = evalSortCompare(mstruct1[start], mstruct2, parent);
				if(i2 != 0) return i2;
			}
		} else if(mstruct2.isMultiplication() && mstruct2.size() > 0) {
			unsigned int start = 0;
			while(mstruct2[start].isNumber() && mstruct2.size() > start + 1) {
				start++;
			}
			int i2;
			if(mstruct1.isMultiplication()) {
				return 1;
			} else {
				i2 = evalSortCompare(mstruct1, mstruct2[start], parent);
				if(i2 != 0) return i2;
			}
		} 
	}
	if(mstruct1.type() != mstruct2.type()) {
		if(!parent.isMultiplication()) {
			if(mstruct2.isNumber()) return -1;
			if(mstruct1.isNumber()) return 1;
		}
		if(!parent.isMultiplication() || (!mstruct1.isNumber() && !mstruct2.isNumber())) {
			if(mstruct2.isPower()) {
				int i = evalSortCompare(mstruct1, mstruct2[0], parent);
				if(i == 0) {
					return evalSortCompare(m_one, mstruct2[1], parent);
				}
				return i;
			}
			if(mstruct1.isPower()) {
				int i = evalSortCompare(mstruct1[0], mstruct2, parent);
				if(i == 0) {
					return evalSortCompare(mstruct1[1], m_one, parent);
				}
				return i;
			}
		}
		if(mstruct2.isInverse()) return -1;
		if(mstruct2.isInverse()) return 1;
		if(mstruct2.isDivision()) return -1;
		if(mstruct2.isDivision()) return 1;
		if(mstruct2.isNegate()) return -1;
		if(mstruct2.isNegate()) return 1;
		if(mstruct2.type() == STRUCT_ALTERNATIVES) return -1;
		if(mstruct2.type() == STRUCT_ALTERNATIVES) return 1;
		if(mstruct2.isAND()) return -1;
		if(mstruct2.isAND()) return 1;
		if(mstruct2.isOR()) return -1;
		if(mstruct2.isOR()) return 1;
		if(mstruct2.isXOR()) return -1;
		if(mstruct2.isXOR()) return 1;
		if(mstruct2.isNOT()) return -1;
		if(mstruct2.isNOT()) return 1;
		if(mstruct2.isComparison()) return -1;
		if(mstruct2.isComparison()) return 1;
		if(mstruct2.type() == STRUCT_UNKNOWN) return -1;
		if(mstruct2.type() == STRUCT_UNKNOWN) return 1;
		if(mstruct2.isUndefined()) return -1;
		if(mstruct2.isUndefined()) return 1;
		if(mstruct2.isFunction()) return -1;
		if(mstruct1.isFunction()) return 1;
		if(mstruct2.isAddition()) return -1;
		if(mstruct1.isAddition()) return 1;
		if(mstruct2.isMultiplication()) return -1;
		if(mstruct1.isMultiplication()) return 1;
		if(mstruct2.isPower()) return -1;
		if(mstruct1.isPower()) return 1;
		if(mstruct2.isUnit()) return -1;
		if(mstruct1.isUnit()) return 1;
		if(mstruct2.isSymbolic()) return -1;
		if(mstruct1.isSymbolic()) return 1;
		if(mstruct2.isVariable()) return -1;
		if(mstruct1.isVariable()) return 1;
		if(parent.isMultiplication()) {
			if(mstruct2.isNumber()) return -1;
			if(mstruct1.isNumber()) return 1;
		}
		return -1;
	}
	switch(mstruct1.type()) {
		case STRUCT_NUMBER: {
			if(!mstruct1.number().isComplex() && !mstruct2.number().isComplex()) {
				ComparisonResult cmp = mstruct1.number().compare(mstruct2.number());
				if(cmp == COMPARISON_RESULT_LESS) return -1;
				else if(cmp == COMPARISON_RESULT_GREATER) return 1;
				return 0;
			} else {
				if(!mstruct1.number().hasRealPart()) {
					if(mstruct2.number().hasRealPart()) {
						return 1;
					} else {
						ComparisonResult cmp = mstruct1.number().compareImaginaryParts(mstruct2.number());
						if(cmp == COMPARISON_RESULT_LESS) return -1;
						else if(cmp == COMPARISON_RESULT_GREATER) return 1;
						return 0;
					}
				} else if(mstruct2.number().hasRealPart()) {
					ComparisonResult cmp = mstruct1.number().compareRealParts(mstruct2.number());
					if(cmp == COMPARISON_RESULT_EQUAL) {
						cmp = mstruct1.number().compareImaginaryParts(mstruct2.number());
					} 
					if(cmp == COMPARISON_RESULT_LESS) return -1;
					else if(cmp == COMPARISON_RESULT_GREATER) return 1;
					return 0;
				} else {
					return -1;
				}
			}
			return -1;
		} 
		case STRUCT_UNIT: {
			if(mstruct1.unit() < mstruct2.unit()) return -1;
			if(mstruct1.unit() == mstruct2.unit()) return 0;
			return 1;
		}
		case STRUCT_SYMBOLIC: {
			if(mstruct1.symbol() < mstruct2.symbol()) return -1;
			else if(mstruct1.symbol() == mstruct2.symbol()) return 0;
			return 1;
		}
		case STRUCT_VARIABLE: {
			if(mstruct1.variable() < mstruct2.variable()) return -1;
			else if(mstruct1.variable() == mstruct2.variable()) return 0;
			return 1;
		}
		case STRUCT_FUNCTION: {
			if(mstruct1.function() < mstruct2.function()) return -1;
			if(mstruct1.function() == mstruct2.function()) {
				for(unsigned int i = 0; i < mstruct2.size(); i++) {
					if(i >= mstruct1.size()) {
						return -1;	
					}
					int i2 = evalSortCompare(mstruct1[i], mstruct2[i], parent);
					if(i2 != 0) return i2;
				}
				return 0;
			}
			return 1;
		}
		case STRUCT_POWER: {
			int i = evalSortCompare(mstruct1[0], mstruct2[0], parent);
			if(i == 0) {
				return evalSortCompare(mstruct1[1], mstruct2[1], parent);
			}
			return i;
		}
		default: {
			if(mstruct2.size() < mstruct1.size()) return -1;
			else if(mstruct2.size() > mstruct1.size()) return 1;
			int ie;
			for(unsigned int i = 0; i < mstruct1.size(); i++) {
				ie = evalSortCompare(mstruct1[i], mstruct2[i], parent);
				if(ie != 0) {
					return ie;
				}
			}
		}
	}
	return 0;
}

void MathStructure::evalSort(bool recursive) {
	if(recursive) {
		for(unsigned int i = 0; i < SIZE; i++) {
			CHILD(i).evalSort(true);
		}
	}
	if(m_type != STRUCT_ADDITION && m_type != STRUCT_MULTIPLICATION && m_type != STRUCT_AND && m_type != STRUCT_OR && m_type != STRUCT_XOR) return;
	vector<unsigned int> sorted;
	bool b;
	for(unsigned int i = 0; i < SIZE; i++) {
		b = false;
		for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
			if(evalSortCompare(CHILD(i), v_subs[sorted[i2]], *this) < 0) {	
				sorted.insert(sorted.begin() + i2, v_order[i]);
				b = true;
				break;
			}
		}
		if(!b) sorted.push_back(v_order[i]);
	}
	for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
		v_order[i2] = sorted[i2];
	}
}

int sortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2, const MathStructure &parent, const PrintOptions &po);
int sortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2, const MathStructure &parent, const PrintOptions &po) {
	if(parent.isMultiplication()) {
		if(mstruct1.containsType(STRUCT_VECTOR) && mstruct2.containsType(STRUCT_VECTOR)) {
			return 0;
		}
	}
	if(parent.isAddition() && po.sort_options.minus_last) {
		bool m1 = mstruct1.hasNegativeSign(), m2 = mstruct2.hasNegativeSign();
		if(m1 && !m2) {
			return 1;
		} else if(m2 && !m1) {
			return -1;
		}
	}
	bool isdiv1 = false, isdiv2 = false;
	if(!po.negative_exponents) {
		if(mstruct1.isMultiplication()) {
			for(unsigned int i = 0; i < mstruct1.size(); i++) {
				if(mstruct1[i].isPower() && mstruct1[i][1].hasNegativeSign()) {
					isdiv1 = true;
					break;
				}
			}
		} else if(mstruct1.isPower() && mstruct1[1].hasNegativeSign()) {
			isdiv1 = true;
		}
		if(mstruct2.isMultiplication()) {
			for(unsigned int i = 0; i < mstruct2.size(); i++) {
				if(mstruct2[i].isPower() && mstruct2[i][1].hasNegativeSign()) {
					isdiv2 = true;
					break;
				}
			}
		} else if(mstruct2.isPower() && mstruct2[1].hasNegativeSign()) {
			isdiv2 = true;
		}
	}
	if(parent.isAddition() && isdiv1 == isdiv2) {
		if(mstruct1.isMultiplication() && mstruct1.size() > 0) {
			unsigned int start = 0;
			while(mstruct1[start].isNumber() && mstruct1.size() > start + 1) {
				start++;
			}
			int i2;
			if(mstruct2.isMultiplication()) {
				if(mstruct2.size() < 1) return -1;
				unsigned int start2 = 0;
				while(mstruct2[start2].isNumber() && mstruct2.size() > start2 + 1) {
					start2++;
				}
				for(unsigned int i = 0; i + start < mstruct1.size(); i++) {
					if(i + start2 >= mstruct2.size()) return 1;
					i2 = sortCompare(mstruct1[i + start], mstruct2[i + start2], parent, po);
					if(i2 != 0) return i2;
				}
				if(mstruct1.size() - start == mstruct2.size() - start2) return 0;
				return -1;
			} else {
				i2 = sortCompare(mstruct1[start], mstruct2, parent, po);
				if(i2 != 0) return i2;
			}
		} else if(mstruct2.isMultiplication() && mstruct2.size() > 0) {
			unsigned int start = 0;
			while(mstruct2[start].isNumber() && mstruct2.size() > start + 1) {
				start++;
			}
			int i2;
			if(mstruct1.isMultiplication()) {
				return 1;
			} else {
				i2 = sortCompare(mstruct1, mstruct2[start], parent, po);
				if(i2 != 0) return i2;
			}
		} 
	}
	if(mstruct1.type() != mstruct2.type()) {
		if(mstruct1.isVariable() && mstruct2.isSymbolic()) {
			if(parent.isMultiplication()) {
				if(mstruct1.variable()->isKnown()) return -1;
			}
			if(mstruct1.variable()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name < mstruct2.symbol()) return -1;
			else return 1;
		}
		if(mstruct2.isVariable() && mstruct1.isSymbolic()) {
			if(parent.isMultiplication()) {
				if(mstruct2.variable()->isKnown()) return 1;
			}
			if(mstruct1.symbol() < mstruct2.variable()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name) return -1;
			else return 1;
		}
		if(!parent.isMultiplication() || (!mstruct1.isNumber() && !mstruct2.isNumber())) {
			if(mstruct2.isPower()) {
				int i = sortCompare(mstruct1, mstruct2[0], parent, po);
				if(i == 0) {
					return sortCompare(m_one, mstruct2[1], parent, po);
				}
				return i;
			}
			if(mstruct1.isPower()) {
				int i = sortCompare(mstruct1[0], mstruct2, parent, po);
				if(i == 0) {
					return sortCompare(mstruct1[1], m_one, parent, po);
				}
				return i;
			}
		}
		if(parent.isMultiplication()) {
			if(mstruct2.isUnit()) return -1;
			if(mstruct1.isUnit()) return 1;
		}
		if(mstruct2.isInverse()) return -1;
		if(mstruct2.isInverse()) return 1;
		if(mstruct2.isDivision()) return -1;
		if(mstruct2.isDivision()) return 1;
		if(mstruct2.isNegate()) return -1;
		if(mstruct2.isNegate()) return 1;
		if(mstruct2.type() == STRUCT_ALTERNATIVES) return -1;
		if(mstruct2.type() == STRUCT_ALTERNATIVES) return 1;
		if(mstruct2.isAND()) return -1;
		if(mstruct2.isAND()) return 1;
		if(mstruct2.isOR()) return -1;
		if(mstruct2.isOR()) return 1;
		if(mstruct2.isXOR()) return -1;
		if(mstruct2.isXOR()) return 1;
		if(mstruct2.isNOT()) return -1;
		if(mstruct2.isNOT()) return 1;
		if(mstruct2.isComparison()) return -1;
		if(mstruct2.isComparison()) return 1;
		if(mstruct2.type() == STRUCT_UNKNOWN) return -1;
		if(mstruct2.type() == STRUCT_UNKNOWN) return 1;
		if(mstruct2.isUndefined()) return -1;
		if(mstruct2.isUndefined()) return 1;
		if(mstruct2.isFunction()) return -1;
		if(mstruct1.isFunction()) return 1;
		if(mstruct2.isAddition()) return -1;
		if(mstruct1.isAddition()) return 1;
		if(!parent.isMultiplication()) {
			if(isdiv2 && mstruct2.isMultiplication()) return -1;
			if(isdiv1 && mstruct1.isMultiplication()) return 1;
			if(mstruct2.isNumber()) return -1;
			if(mstruct1.isNumber()) return 1;
		}
		if(mstruct2.isMultiplication()) return -1;
		if(mstruct1.isMultiplication()) return 1;
		if(mstruct2.isPower()) return -1;
		if(mstruct1.isPower()) return 1;
		if(mstruct2.isUnit()) return -1;
		if(mstruct1.isUnit()) return 1;
		if(mstruct2.isSymbolic()) return -1;
		if(mstruct1.isSymbolic()) return 1;
		if(mstruct2.isVariable()) return -1;
		if(mstruct1.isVariable()) return 1;
		if(parent.isMultiplication()) {
			if(mstruct2.isNumber()) return -1;
			if(mstruct1.isNumber()) return 1;
		}
		return -1;
	}
	switch(mstruct1.type()) {
		case STRUCT_NUMBER: {
			if(!mstruct1.number().isComplex() && !mstruct2.number().isComplex()) {
				ComparisonResult cmp;
				if(parent.isMultiplication() && mstruct2.number().isNegative() != mstruct1.number().isNegative()) cmp = mstruct2.number().compare(mstruct1.number());
				else cmp = mstruct1.number().compare(mstruct2.number());
				if(cmp == COMPARISON_RESULT_LESS) return -1;
				else if(cmp == COMPARISON_RESULT_GREATER) return 1;
				return 0;
			} else {
				if(!mstruct1.number().hasRealPart()) {
					if(mstruct2.number().hasRealPart()) {
						return 1;
					} else {
						ComparisonResult cmp = mstruct1.number().compareImaginaryParts(mstruct2.number());
						if(cmp == COMPARISON_RESULT_LESS) return -1;
						else if(cmp == COMPARISON_RESULT_GREATER) return 1;
						return 0;
					}
				} else if(mstruct2.number().hasRealPart()) {
					ComparisonResult cmp = mstruct1.number().compareRealParts(mstruct2.number());
					if(cmp == COMPARISON_RESULT_EQUAL) {
						cmp = mstruct1.number().compareImaginaryParts(mstruct2.number());
					} 
					if(cmp == COMPARISON_RESULT_LESS) return -1;
					else if(cmp == COMPARISON_RESULT_GREATER) return 1;
					return 0;
				} else {
					return -1;
				}
			}
			return -1;
		} 
		case STRUCT_UNIT: {
			if(mstruct1.unit() == mstruct2.unit()) return 0;
			if(mstruct1.unit()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, mstruct1.isPlural(), po.use_reference_names).name < mstruct2.unit()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, mstruct2.isPlural(), po.use_reference_names).name) return -1;
			return 1;
		}
		case STRUCT_SYMBOLIC: {
			if(mstruct1.symbol() < mstruct2.symbol()) return -1;
			else if(mstruct1.symbol() == mstruct2.symbol()) return 0;
			return 1;
		}
		case STRUCT_VARIABLE: {
			if(mstruct1.variable() == mstruct2.variable()) return 0;
			if(parent.isMultiplication()) {
				if(mstruct1.variable()->isKnown() && !mstruct2.variable()->isKnown()) return -1;
				if(!mstruct1.variable()->isKnown() && mstruct2.variable()->isKnown()) return 1;
			}
			if(mstruct1.variable()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name < mstruct2.variable()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name) return -1;
			return 1;
		}
		case STRUCT_FUNCTION: {
			if(mstruct1.function() == mstruct2.function()) {
				for(unsigned int i = 0; i < mstruct2.size(); i++) {
					if(i >= mstruct1.size()) {
						return -1;	
					}
					int i2 = sortCompare(mstruct1[i], mstruct2[i], parent, po);
					if(i2 != 0) return i2;
				}
				return 0;
			}
			if(mstruct1.function()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name < mstruct2.function()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name) return -1;
			return 1;
		}
		case STRUCT_POWER: {
			int i = sortCompare(mstruct1[0], mstruct2[0], parent, po);
			if(i == 0) {
				return sortCompare(mstruct1[1], mstruct2[1], parent, po);
			}
			return i;
		}
		case STRUCT_MULTIPLICATION: {
			if(isdiv1 != isdiv2) {
				if(isdiv1) return 1;
				return -1;
			}
		}
		default: {
			int ie;
			for(unsigned int i = 0; i < mstruct1.size(); i++) {
				if(i >= mstruct2.size()) return 1;
				ie = sortCompare(mstruct1[i], mstruct2[i], parent, po);
				if(ie != 0) {
					return ie;
				}
			}
		}
	}
	return 0;
}

void MathStructure::sort(const PrintOptions &po, bool recursive) {
	if(recursive) {
		for(unsigned int i = 0; i < SIZE; i++) {
			CHILD(i).sort(po);
		}
	}
	if(m_type != STRUCT_ADDITION && m_type != STRUCT_MULTIPLICATION && m_type != STRUCT_AND && m_type != STRUCT_OR && m_type != STRUCT_XOR) return;
	vector<unsigned int> sorted;
	bool b;
	PrintOptions po2 = po;
	po2.sort_options.minus_last = po.sort_options.minus_last && !containsUnknowns();
	for(unsigned int i = 0; i < SIZE; i++) {
		b = false;
		for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
			if(sortCompare(CHILD(i), v_subs[sorted[i2]], *this, po2) < 0) {
				sorted.insert(sorted.begin() + i2, v_order[i]);
				b = true;
				break;
			}
		}
		if(!b) sorted.push_back(v_order[i]);
	}
	for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
		v_order[i2] = sorted[i2];
	}
}
bool MathStructure::containsAdditionPower() const {
	if(m_type == STRUCT_POWER && CHILD(0).isAddition()) return true;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).containsAdditionPower()) return true;
	}
	return false;
}

unsigned int MathStructure::countTotalChilds(bool count_function_as_one) const {
	if(m_type == STRUCT_FUNCTION || SIZE == 0) return 1;
	unsigned int count = 0;
	for(unsigned int i = 0; i < SIZE; i++) {
		count += CHILD(i).countTotalChilds();
	}
	return count;
}

void try_isolate_x(MathStructure &mstruct, EvaluationOptions &eo3, const EvaluationOptions &eo);
void try_isolate_x(MathStructure &mstruct, EvaluationOptions &eo3, const EvaluationOptions &eo) {
	if(mstruct.isComparison()) {
		if(mstruct.comparisonType() == COMPARISON_EQUALS || mstruct.comparisonType() == COMPARISON_NOT_EQUALS) {
			MathStructure mtest(mstruct);
			eo3.test_comparisons = false;
			mtest.calculatesub(eo3, eo);
			eo3.test_comparisons = eo.test_comparisons;
			if(mtest.isolate_x(eo3)) {
				mstruct = mtest;
			}
		}
	} else {
		for(unsigned int i = 0; i < mstruct.size(); i++) {
			try_isolate_x(mstruct[i], eo3, eo);
		}
	}
}

MathStructure &MathStructure::eval(const EvaluationOptions &eo) {
	if(eo.structuring == STRUCTURING_NONE) return *this;
	unformat(eo);
	if(eo.sync_units) {
		syncUnits();
		unformat(eo);
	}
	if(eo.calculate_functions) {
		calculateFunctions(eo);
		unformat(eo);
	}
	EvaluationOptions eo2 = eo;
	eo2.simplify_addition_powers = false;
	eo2.do_polynomial_division = false;
	eo2.reduce_divisions = false;
	if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
		EvaluationOptions eo3 = eo2;
		eo3.approximation = APPROXIMATION_EXACT;
		eo3.split_squares = false;
		calculatesub(eo3, eo);
		eo3.approximation = APPROXIMATION_APPROXIMATE;
		calculatesub(eo3, eo);
		if(eo2.isolate_x) {
			isolate_x(eo3);
		}
	} else {
		calculatesub(eo2, eo);
		if(eo2.isolate_x) {
			isolate_x(eo2);
		}
	}
	if(eo.simplify_addition_powers || eo.reduce_divisions) {
		eo2.simplify_addition_powers = eo.simplify_addition_powers;
		eo2.reduce_divisions = eo.reduce_divisions;
		if((eo.simplify_addition_powers && containsAdditionPower()) || (eo.reduce_divisions && containsDivision())) {
			if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
				EvaluationOptions eo3 = eo2;
				eo3.approximation = APPROXIMATION_APPROXIMATE;
				calculatesub(eo3, eo);
				if(eo2.isolate_x) {
					isolate_x(eo3);
				}
			} else {
				calculatesub(eo2, eo);
				if(eo2.isolate_x) {
					isolate_x(eo2);
				}
			}
		}
	}
	if(eo.do_polynomial_division) {
		eo2.do_polynomial_division = true;
		if(containsDivision()) {
			MathStructure mtest(*this);
			EvaluationOptions eo3 = eo2;
			eo3.reduce_divisions = false;
			if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
				eo3.approximation = APPROXIMATION_APPROXIMATE;
			}
			mtest.calculatesub(eo3, eo);
			if(mtest.countTotalChilds() < countTotalChilds()) {
				set(mtest);
			}
			if(eo2.isolate_x) {
				isolate_x(eo3);
			}
		}
	}
	if(eo2.isolate_x && containsType(STRUCT_COMPARISON)) {		
		EvaluationOptions eo3 = eo2;
		if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
			eo3.approximation = APPROXIMATION_APPROXIMATE;
		}
		eo3.assume_denominators_nonzero = true;
		try_isolate_x(*this, eo3, eo);
	}
	if(eo2.sync_units && eo2.sync_complex_unit_relations) {
		if(syncUnits(true)) {
			unformat(eo2);
			if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
				EvaluationOptions eo3 = eo2;
				eo3.approximation = APPROXIMATION_EXACT;
				eo3.split_squares = false;
				calculatesub(eo3, eo);
				eo3.approximation = APPROXIMATION_APPROXIMATE;
				calculatesub(eo3, eo);
				if(eo2.isolate_x) {
					isolate_x(eo3);
				}
			} else {
				calculatesub(eo2, eo);
				if(eo2.isolate_x) {
					isolate_x(eo2);
				}
			}
		}
	}
	return *this;
}

bool factorize_find_multiplier(const MathStructure &mstruct, MathStructure &mnew, MathStructure &factor_mstruct) {
	factor_mstruct.set(1, 1);
	switch(mstruct.type()) {
		case STRUCT_ADDITION: {
			bool bfrac = false, bint = true;
			idm1(mstruct, bfrac, bint);
			if(bfrac || bint) {
				Number gcd(1, 1);
				idm2(mstruct, bfrac, bint, gcd);
				if((bint || bfrac) && !gcd.isOne()) {		
					if(bfrac) gcd.recip();
					factor_mstruct.set(gcd);
				}
			}
			if(mstruct.size() > 0) {
				unsigned int i = 0;
				const MathStructure *cur_mstruct;
				while(true) {
					if(mstruct[0].isMultiplication()) {
						if(i >= mstruct[0].size()) {
							break;
						}
						cur_mstruct = &mstruct[0][i];
					} else {
						cur_mstruct = &mstruct[0];
					}
					if(!cur_mstruct->isNumber() && (!cur_mstruct->isPower() || cur_mstruct->exponent()->isNumber())) {
						const MathStructure *exp = NULL;
						const MathStructure *bas;
						if(cur_mstruct->isPower()) {
							exp = cur_mstruct->exponent();
							bas = cur_mstruct->base();
						} else {
							bas = cur_mstruct;
						}
						bool b = true;
						for(unsigned int i2 = 1; i2 < mstruct.size(); i2++) {
							b = false;
							unsigned int i3 = 0;
							const MathStructure *cmp_mstruct;
							while(true) {
								if(mstruct[i2].isMultiplication()) {
									if(i3 >= mstruct[i2].size()) {
										break;
									}
									cmp_mstruct = &mstruct[i2][i3];
								} else {
									cmp_mstruct = &mstruct[i2];
								}
								if(cmp_mstruct->equals(*bas)) {
									if(exp) {
										exp = NULL;
									}
									b = true;
									break;
								} else if(cmp_mstruct->isPower() && cmp_mstruct->base()->equals(*bas)) {
									if(exp) {
										if(cmp_mstruct->exponent()->isNumber()) {
											if(cmp_mstruct->exponent()->number().isLessThan(exp->number())) {
												exp = cmp_mstruct->exponent();
											}
											b = true;
											break;
										} else {
											exp = NULL;
										}
									} else {
										b = true;
										break;
									}
								}
								if(!mstruct[i2].isMultiplication()) {
									break;
								}
								i3++;
							}
							if(!b) break;
						}
						if(b) {
							if(exp) {
								MathStructure mstruct(*bas);
								mstruct ^= *exp;
								if(factor_mstruct.isOne()) factor_mstruct.set(mstruct);
								else factor_mstruct.multiply(mstruct, true);
							} else {
								if(factor_mstruct.isOne()) factor_mstruct.set(*bas);
								else factor_mstruct.multiply(*bas, true);
							}
						}
					}
					if(!mstruct[0].isMultiplication()) {
						break;
					}
					i++;
				}
			}
			if(!factor_mstruct.isOne()) {
				if(&mstruct != &mnew) mnew.set(mstruct);
				MathStructure *mfactor;
				unsigned int i = 0;
				while(true) {
					if(factor_mstruct.isMultiplication()) {
						if(i >= factor_mstruct.size()) break;
						mfactor = &factor_mstruct[i];
					} else {
						mfactor = &factor_mstruct;
					}
					for(unsigned int i2 = 0; i2 < mnew.size(); i2++) {
						switch(mnew[i2].type()) {
							case STRUCT_NUMBER: {
								if(mfactor->isNumber()) {
									mnew[i2].number() /= mfactor->number();
								}
								break;
							}
							case STRUCT_POWER: {
								if(mfactor->isNumber()) {
									mnew[i2].transform(STRUCT_MULTIPLICATION);
									mnew[i2].insertChild(MathStructure(1, 1), 1);
									mnew[i2][0].number() /= mfactor->number();
								} else if(mfactor->isPower()) {
									if(mfactor->equals(mnew[i2])) {
										mnew[i2].set(1, 1);
									} else {
										mnew[i2][1].number() -= mfactor->exponent()->number();
										if(mnew[i2][1].number().isOne()) {
											MathStructure mstruct2(mnew[i2][0]);
											mnew[i2] = mstruct2;
										}
									}
								} else {
									mnew[i2][1].number() -= 1;
									if(mnew[i2][1].number().isOne()) {
										MathStructure mstruct2(mnew[i2][0]);
										mnew[i2] = mstruct2;
									} else if(mnew[i2][1].number().isZero()) {
										mnew[i2].set(1, 1);
									}
								}
								break;
							}
							case STRUCT_MULTIPLICATION: {
								bool b = true;
								if(mfactor->isNumber() && (mnew[i2].size() < 1 || !mnew[i2][0].isNumber())) {
									mnew[i2].insertChild(MathStructure(1, 1), 1);
								}
								for(unsigned int i3 = 0; i3 < mnew[i2].size() && b; i3++) {
									switch(mnew[i2][i3].type()) {
										case STRUCT_NUMBER: {
											if(mfactor->isNumber()) {
												if(mfactor->equals(mnew[i2][i3])) {
													mnew[i2].delChild(i3 + 1);
												} else {
													mnew[i2][i3].number() /= mfactor->number();
												}
												b = false;
											}
											break;
										}
										case STRUCT_POWER: {
											if(mfactor->isPower() && mfactor->base()->equals(mnew[i2][i3][0])) {
												if(mfactor->equals(mnew[i2][i3])) {
													mnew[i2].delChild(i3 + 1);
												} else {
													mnew[i2][i3][1].number() -= mfactor->exponent()->number();
													if(mnew[i2][i3][1].number().isOne()) {
														MathStructure mstruct2(mnew[i2][i3][0]);
														mnew[i2][i3] = mstruct2;
													} else if(mnew[i2][i3][1].number().isZero()) {
														mnew[i2].delChild(i3 + 1);
													}
												}
												b = false;
											} else if(mfactor->equals(mnew[i2][i3][0])) {
												if(mnew[i2][i3][1].number() == 2) {
													MathStructure mstruct2(mnew[i2][i3][0]);
													mnew[i2][i3] = mstruct2;
												} else if(mnew[i2][i3][1].number().isOne()) {
													mnew[i2].delChild(i3 + 1);
												} else {
													mnew[i2][i3][1].number() -= 1;
												}
												b = false;
											}
											break;
										}
										default: {
											if(mfactor->equals(mnew[i2][i3])) {
												mnew[i2].delChild(i3 + 1);
												b = false;
											}
										}
									}
								}
								if(mnew[i2].size() == 1) {
									MathStructure mstruct2(mnew[i2][0]);
									mnew[i2] = mstruct2;
								}
								break;
							}
							default: {
								if(mfactor->isNumber()) {
									mnew[i2].transform(STRUCT_MULTIPLICATION);
									mnew[i2].insertChild(MathStructure(1, 1), 1);
									mnew[i2][0].number() /= mfactor->number();
								} else {
									mnew[i2].set(1, 1);
								}
							}
						}
					}
					if(factor_mstruct.isMultiplication()) {
						i++;
					} else {
						break;
					}
				}
				return true;
			}
		}
	}
	return false;
}

bool MathStructure::factorize(const EvaluationOptions &eo) {
	switch(type()) {
		case STRUCT_ADDITION: {
			if(SIZE <= 3 && SIZE > 1) {
				MathStructure *xvar = NULL;
				Number nr2(1, 1);
				if(CHILD(0).isPower() && CHILD(0)[0].size() == 0 && CHILD(0)[1].isNumber() && CHILD(0)[1].number().isTwo()) {
					xvar = &CHILD(0)[0];
				} else if(CHILD(0).isMultiplication() && CHILD(0).size() == 2 && CHILD(0)[0].isNumber()) {
					if(CHILD(0)[1].isPower()) {
						if(CHILD(0)[1][0].size() == 0 && CHILD(0)[1][1].isNumber() && CHILD(0)[1][1].number().isTwo()) {
							xvar = &CHILD(0)[1][0];
							nr2.set(CHILD(0)[0].number());
						}
					}
				}
				if(xvar) {
					bool factorable = false;
					Number nr1, nr0;
					if(SIZE == 2 && CHILD(1).isNumber()) {
						factorable = true;
						nr0 = CHILD(1).number();
					} else if(SIZE == 3 && CHILD(2).isNumber()) {
						nr0 = CHILD(2).number();
						if(CHILD(1).isMultiplication()) {
							if(CHILD(1).size() == 2 && CHILD(1)[0].isNumber() && xvar->equals(CHILD(1)[1])) {
								nr1 = CHILD(1)[0].number();
								factorable = true;
							}
						} else if(xvar->equals(CHILD(1))) {
							nr1.set(1, 1);
							factorable = true;
						}
					}
					if(factorable) {
						Number nr4ac(4, 1);
						nr4ac *= nr2;
						nr4ac *= nr0;
						Number nrtwo(2, 1);
						Number nr2a(2, 1);
						nr2a *= nr2;
						Number sqrtb24ac(nr1);
						sqrtb24ac.raise(nrtwo);
						sqrtb24ac -= nr4ac;
						if(!sqrtb24ac.isNegative()) {
							nrtwo.set(1, 2);
							MathStructure mstructb24(sqrtb24ac);
							if(eo.approximation == APPROXIMATION_EXACT && !sqrtb24ac.isApproximate()) {
								sqrtb24ac.raise(nrtwo);
								if(sqrtb24ac.isApproximate()) {
									mstructb24.raise(nrtwo);
								} else {
									mstructb24.set(sqrtb24ac);
								}
							} else {
								mstructb24.number().raise(nrtwo);
							}
							MathStructure m1(nr1), m2(nr1);
							Number mul1(1, 1), mul2(1, 1);
							if(mstructb24.isNumber()) {
								m1.number() += mstructb24.number();
								m1.number() /= nr2a;
								if(m1.number().isRational() && !m1.number().isInteger()) {
									mul1 = m1.number().denominator();
									m1.number() *= mul1;
								}
								m2.number() -= mstructb24.number();
								m2.number() /= nr2a;
								if(m2.number().isRational() && !m2.number().isInteger()) {
									mul2 = m2.number().denominator();
									m2.number() *= mul2;
								}
							} else {
								EvaluationOptions eo2 = eo;
								eo2.calculate_functions = false;
								eo2.sync_units = false;
								m1 += mstructb24;
								m1 /= nr2a;
								m1.eval(eo2);
								if(m1.isNumber()) {
									if(m1.number().isRational() && !m1.number().isInteger()) {
										mul1 = m1.number().denominator();
										m1.number() *= mul1;
									}
								} else {
									bool bint = false, bfrac = false;
									idm1(m1, bfrac, bint);
									if(bfrac) {
										idm2(m1, bfrac, bint, mul1);
										idm3(m1, mul1);
									}
								}
								m2 -= mstructb24;
								m2 /= nr2a;
								m2.eval(eo2);
								if(m2.isNumber()) {
									if(m2.number().isRational() && !m2.number().isInteger()) {
										mul2 = m2.number().denominator();
										m2.number() *= mul2;
									}
								} else {
									bool bint = false, bfrac = false;
									idm1(m2, bfrac, bint);
									if(bfrac) {
										idm2(m2, bfrac, bint, mul2);
										idm3(m2, mul2);
									}
								}
							}
							nr2 /= mul1;
							nr2 /= mul2;
							if(m1 == m2 && mul1 == mul2) {
								MathStructure xvar2(*xvar);
								if(!mul1.isOne()) xvar2 *= mul1;
								set(m1);
								add(xvar2, true);
								raise(MathStructure(2, 1));
								if(!nr2.isOne()) {
									multiply(nr2);
								}
							} else {
								m1.add(*xvar, true);
								if(!mul1.isOne()) m1[m1.size() - 1] *= mul1;
								m2.add(*xvar, true);
								if(!mul2.isOne()) m2[m2.size() - 1] *= mul2;
								clear();
								m_type = STRUCT_MULTIPLICATION;
								if(!nr2.isOne()) {
									APPEND(nr2);
								}
								APPEND(m1);
								APPEND(m2);
							}							
							return true;
						}
					}
				}
			}
			MathStructure factor_mstruct(1, 1);
			MathStructure mnew;
			if(factorize_find_multiplier(*this, mnew, factor_mstruct)) {
				mnew.factorize(eo);
				if(mnew.isMultiplication()) {
					set(mnew);
					bool b = false;
					for(unsigned int i = 0; i < SIZE; i++) {
						if(!CHILD(i).isAddition()) {
							int ret = CHILD(i).merge_multiplication(factor_mstruct, eo);
							if(ret == 0) {
								ret = factor_mstruct.merge_multiplication(CHILD(i), eo);
								if(ret > 0) {
									CHILD(i) = factor_mstruct;
								}
							}
							if(ret > 0) b = true;
							break;
						}
					}
					if(!b) {
						PREPEND(factor_mstruct);
					}
				} else {
					clear();
					m_type = STRUCT_MULTIPLICATION;
					APPEND(factor_mstruct);
					APPEND(mnew);
				}
				return true;
			}
			if(SIZE > 1 && CHILD(SIZE - 1).isNumber() && CHILD(SIZE - 1).number().isInteger()) {
				MathStructure *xvar = NULL;
				Number qnr(1, 1);
				int degree = 1;
				bool overflow = false;
				int qcof = 1;
				if(CHILD(0).isPower() && CHILD(0)[0].size() == 0 && CHILD(0)[1].isNumber() && CHILD(0)[1].number().isInteger() && CHILD(0)[1].number().isPositive()) {
					xvar = &CHILD(0)[0];
					degree = CHILD(0)[1].number().intValue(&overflow);
				} else if(CHILD(0).isMultiplication() && CHILD(0).size() == 2 && CHILD(0)[0].isNumber() && CHILD(0)[0].number().isInteger()) {
					if(CHILD(0)[1].isPower()) {
						if(CHILD(0)[1][0].size() == 0 && CHILD(0)[1][1].isNumber() && CHILD(0)[1][1].number().isInteger() && CHILD(0)[1][1].number().isPositive()) {
							xvar = &CHILD(0)[1][0];
							qcof = CHILD(0)[0].number().intValue(&overflow);
							if(!overflow) {
								if(qcof < 0) qcof = -qcof;
								degree = CHILD(0)[1][1].number().intValue(&overflow);
							}
						}
					}
				}
				int pcof = 1;
				if(!overflow) {
					pcof = CHILD(SIZE - 1).number().intValue(&overflow);
					if(pcof < 0) pcof = -pcof;
				}
				if(xvar && !overflow && degree <= 1000 && degree > 2) {
					bool b = true;
					for(unsigned int i = 1; b && i < SIZE - 1; i++) {
						switch(CHILD(i).type()) {
							case STRUCT_NUMBER: {
								b = false;
								break;
							}
							case STRUCT_POWER: {
								if(!CHILD(i)[1].isNumber() || !xvar->equals(CHILD(i)[0]) || !CHILD(i)[1].number().isInteger() || !CHILD(i)[1].number().isPositive()) {
									b = false;
								}
								break;
							}
							case STRUCT_MULTIPLICATION: {
								if(!CHILD(i).size() == 2 || !CHILD(i)[0].isNumber()) {
									b = false;
								} else if(CHILD(i)[1].isPower()) {
									if(!CHILD(i)[1][1].isNumber() || !xvar->equals(CHILD(i)[1][0]) || !CHILD(i)[1][1].number().isInteger() || !CHILD(i)[1][1].number().isPositive()) {
										b = false;
									}
								} else if(!xvar->equals(CHILD(i)[1])) {
									b = false;
								}
								break;
							}
							default: {
								if(!xvar->equals(CHILD(i))) {
									b = false;
								}
							}
						}
					}
					if(b) {
						Number factors[degree + 1];
						factors[0] = CHILD(SIZE - 1).number();
						vector<int> ps;
						vector<int> qs;
						vector<Number> zeroes;
						int curdeg = 1, prevdeg = 0;
						for(unsigned int i = 0; b && i < SIZE - 1; i++) {
							switch(CHILD(i).type()) {
								case STRUCT_POWER: {
									curdeg = CHILD(i)[1].number().intValue(&overflow);
									if(curdeg == prevdeg || curdeg > degree || prevdeg > 0 && curdeg > prevdeg || overflow) {
										b = false;
									} else {
										factors[curdeg].set(1, 1);
									}
									break;
								}
								case STRUCT_MULTIPLICATION: {
									if(CHILD(i)[1].isPower()) {
										curdeg = CHILD(i)[1][1].number().intValue(&overflow);
									} else {
										curdeg = 1;
									}
									if(curdeg == prevdeg || curdeg > degree || prevdeg > 0 && curdeg > prevdeg || overflow) {
										b = false;
									} else {
										factors[curdeg] = CHILD(i)[0].number();
									}
									break;
								}
								default: {
									curdeg = 1;
									factors[curdeg].set(1, 1);
								}
							}
							prevdeg = curdeg;
						}
						while(b && degree > 2) {
							for(int i = 1; i <= 1000; i++) {
								if(i > pcof) break;
								if(pcof % i == 0) ps.push_back(i);
							}
							for(int i = 1; i <= 1000; i++) {
								if(i > qcof) break;
								if(qcof % i == 0) qs.push_back(i);
							}
							Number itest;
							int i2;
							unsigned int pi = 0, qi = 0;
							Number nrtest(ps[0], qs[0]);
							while(true) {
								itest.clear(); i2 = degree;
								while(true) {
									itest += factors[i2];
									if(i2 == 0) break;
									itest *= nrtest;
									i2--;
								}
								if(itest.isZero()) {
									break;
								}
								if(nrtest.isPositive()) {
									nrtest.negate();
								} else {
									qi++;
									if(qi == qs.size()) {
										qi = 0;
										pi++;
										if(pi == ps.size()) {
											break;
										}
									}
									nrtest.set(ps[pi], qs[qi]);
								}
							}
							if(itest.isZero()) {
								itest.clear(); i2 = degree;
								Number ntmp(factors[i2]);
								for(; i2 > 0; i2--) {
									itest += ntmp;
									ntmp = factors[i2 - 1];
									factors[i2 - 1] = itest;
									itest *= nrtest;
								}
								degree--;
								nrtest.negate();
								zeroes.push_back(nrtest);
								if(degree == 2) {
									break;
								}
								qcof = factors[degree].intValue(&overflow);
								if(!overflow) {
									if(qcof < 0) qcof = -qcof;
									pcof = factors[0].intValue(&overflow);
									if(!overflow) {
										if(pcof < 0) pcof = -pcof;
									}
								}
								if(overflow) {
									break;
								}
							} else {
								break;
							}
							ps.clear();
							qs.clear();
						}
						if(zeroes.size() > 0) {
							MathStructure mleft;
							MathStructure mtmp;
							MathStructure *mcur;
							for(int i = degree; i >= 0; i--) {
								if(!factors[i].isZero()) {
									if(mleft.isZero()) {
										mcur = &mleft;
									} else {
										mleft.add(m_zero, true);
										mcur = &mleft[mleft.size() - 1];
									}
									if(i > 1) {
										if(!factors[i].isOne()) {
											mcur->multiply(*xvar);
											(*mcur)[0].set(factors[i]);
											mcur = &(*mcur)[1];
										} else {
											mcur->set(*xvar);
										}
										mtmp.set(i, 1);
										mcur->raise(mtmp);
									} else if(i == 1) {
										if(!factors[i].isOne()) {
											mcur->multiply(*xvar);
											(*mcur)[0].set(factors[i]);
										} else {
											mcur->set(*xvar);
										}
									} else {
										mcur->set(factors[i]);
									}
								}
							}
							mleft.factorize(eo);
							vector<int> powers;
							vector<unsigned int> powers_i;
							int dupsfound = 0;
							for(unsigned int i = 0; i < zeroes.size() - 1; i++) {
								while(i + 1 < zeroes.size() && zeroes[i] == zeroes[i + 1]) {
									dupsfound++;
									zeroes.erase(zeroes.begin() + (i + 1));
								}
								if(dupsfound > 0) {
									powers_i.push_back(i);
									powers.push_back(dupsfound + 1);
									dupsfound = 0;
								}
							}
							MathStructure xvar2(*xvar);
							Number *nrmul;
							if(mleft.isMultiplication()) {
								set(mleft);
								evalSort();
								if(CHILD(0).isNumber()) {
									nrmul = &CHILD(0).number();
								} else if(CHILD(0).isMultiplication() && CHILD(0).size() > 0 && CHILD(0)[0].isNumber()) {
									nrmul = &CHILD(0)[0].number();
								} else {
									PREPEND(m_one);
									nrmul = &CHILD(0).number();
								}
							} else {
								clear();
								m_type = STRUCT_MULTIPLICATION;
								APPEND(m_one);
								APPEND(mleft);
								nrmul = &CHILD(0).number();
							}
							unsigned int pi = 0;
							for(unsigned int i = 0; i < zeroes.size(); i++) {
								if(zeroes[i].isInteger()) {
									APPEND(xvar2);
								} else {
									APPEND(m_zero);
								}
								mcur = &CHILD(SIZE - 1);
								if(pi < powers_i.size() && powers_i[pi] == i) {
									mcur->raise(MathStructure(powers[pi], 1));
									mcur = &(*mcur)[0];
									if(zeroes[i].isInteger()) {
										mcur->add(zeroes[i]);
									} else {
										Number nr(zeroes[i].denominator());
										mcur->add(zeroes[i].numerator());
										(*mcur)[0] *= xvar2;
										(*mcur)[0][0].number() = nr;
										nr.raise(powers[pi]);
										nrmul->divide(nr);										
									}
									pi++;
								} else {
									if(zeroes[i].isInteger()) {
										mcur->add(zeroes[i]);
									} else {
										nrmul->divide(zeroes[i].denominator());
										mcur->add(zeroes[i].numerator());
										(*mcur)[0] *= xvar2;
										(*mcur)[0][0].number() = zeroes[i].denominator();
									}
								}
							}
							if(CHILD(0).isNumber() && CHILD(0).number().isOne()) {
								ERASE(0);
							} else if(CHILD(0).isMultiplication() && CHILD(0).size() > 0 && CHILD(0)[0].isNumber() && CHILD(0)[0].number().isOne()) {
								if(CHILD(0).size() == 1) {
									ERASE(0);
								} else if(CHILD(0).size() == 2) {
									MathStructure msave(CHILD(0)[1]);
									CHILD(0) = msave;
								} else {
									CHILD(0).delChild(1);
								}
							}
							evalSort(true);
							Number dupspow;
							for(unsigned int i = 0; i < SIZE - 1; i++) {
								mcur = NULL;
								if(CHILD(i).isPower()) {
									if(CHILD(i)[0].isAddition() && CHILD(i)[1].isNumber()) {
										mcur = &CHILD(i)[0];
									}
								} else if(CHILD(i).isAddition()) {
									mcur = &CHILD(i);
								}
								while(mcur && i + 1 < SIZE) {
									if(CHILD(i + 1).isPower()) {
										if(CHILD(i + 1)[0].isAddition() && CHILD(i + 1)[1].isNumber() && mcur->equals(CHILD(i + 1)[0])) {
											dupspow += CHILD(i + 1)[1].number();
										} else {
											mcur = NULL;
										}
									} else if(CHILD(i + 1).isAddition() && mcur->equals(CHILD(i + 1))) {
										dupspow++;
									} else {
										mcur = NULL;
									}
									if(mcur) {
										ERASE(i + 1);
									}
								}
								if(!dupspow.isZero()) {
									if(CHILD(i).isPower()) {
										CHILD(i)[1].number() += dupspow;
									} else {
										dupspow++;
										CHILD(i) ^= dupspow;
									}
									dupspow.clear();
								}
							}
							if(SIZE == 1) {
								MathStructure msave(CHILD(0));
								set(msave);
							}
							return true;
						}
					}
				}
			}
		}
		default: {
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).factorize(eo)) {
					childUpdated(i + 1);
				}
			}
		}
	}
	return false;
}


#ifdef HAVE_GIAC
giac::gen MathStructure::toGiac() const {
	switch(type()) {
		case STRUCT_NUMBER: {
			return o_number.internalNumber();
		} 
		case STRUCT_SYMBOLIC: {
			string id = "!";
			id += s_sym;
			return giac::identificateur(id);
		}
		case STRUCT_INVERSE: {
			return giac::symbolic(giac::at_inv, CHILD(0).toGiac());
		} 
		case STRUCT_NEGATE: {
			return giac::symbolic(giac::at_neg, CHILD(0).toGiac());
		} 
		case STRUCT_NOT: {
			return giac::symbolic(giac::at_not, CHILD(0).toGiac());
		} 
		case STRUCT_AND: {}
		case STRUCT_OR: {}
		case STRUCT_XOR: {}
		case STRUCT_COMPARISON: {}
		case STRUCT_POWER: {}
		case STRUCT_MULTIPLICATION: {} 
		case STRUCT_ADDITION: {}
		case STRUCT_VECTOR: {
			giac::gen v_g;
			giac::vecteur v;
			for(unsigned int i = 0; i < SIZE; i++) {
				giac::gen f = CHILD(i).toGiac();
				v.push_back(f);
			}
			v_g = v;
			switch(m_type) {
				case STRUCT_POWER: return giac::symbolic(giac::at_pow, v_g);
				case STRUCT_ADDITION: return giac::symbolic(giac::at_plus, v_g);
				case STRUCT_MULTIPLICATION: return giac::symbolic(giac::at_prod, v_g);
				case STRUCT_AND: return giac::symbolic(giac::at_and, v_g);
				case STRUCT_OR: return giac::symbolic(giac::at_ou, v_g);
				case STRUCT_XOR: return giac::symbolic(giac::at_xor, v_g);
				case STRUCT_COMPARISON: {
					switch(ct_comp) {
						case COMPARISON_EQUALS: return giac::symbolic(giac::at_equal, v_g);
						case COMPARISON_NOT_EQUALS: return giac::symbolic(giac::at_different, v_g);
						case COMPARISON_LESS: return giac::symbolic(giac::at_inferieur_strict, v_g);
						case COMPARISON_GREATER: return giac::symbolic(giac::at_superieur_strict, v_g);
						case COMPARISON_EQUALS_LESS: return giac::symbolic(giac::at_inferieur_egal, v_g);
						case COMPARISON_EQUALS_GREATER: return giac::symbolic(giac::at_superieur_egal, v_g);
					}
				}
				case STRUCT_VECTOR: {
					return v_g;
				}
			}
		}
		case STRUCT_FUNCTION: {
			return o_function->toGiac(*this);
		}
		case STRUCT_VARIABLE: {
			if(variable() == CALCULATOR->getPI()) {
				return giac::_IDNT_pi;
			} else if(variable() == CALCULATOR->getE()) {
				return giac::e__IDNT;
			} else {
				string id = "v!";
				id += o_variable->name();
				return giac::identificateur(id, o_variable->get().toGiac());
			}
		}
		case STRUCT_UNIT: {
			string id = "u!";
			id += unit()->name();
			giac::identificateur ident(id);
			return ident;
		}
		case STRUCT_UNKNOWN: {
			return *giac_unknown;
		}
		case STRUCT_UNDEFINED: {
			return giac::_IDNT_undef;
		}
		default: {
			giac::gen g;
			return g;
		}
	}
}
void MathStructure::set(const giac::gen &giac_gen, bool in_retry) {
	clear();
	printf("%i %i\n", giac_gen.type, giac_gen.subtype);
	switch(giac_gen.type) {
		case giac::_INT_: {}
		case giac::_DOUBLE_: {}
		case giac::_ZINT: {}
		case giac::_REAL: {}
		case giac::_CPLX: {
			m_type = STRUCT_NUMBER;
			o_number.setInternal(giac_gen);
			setApproximate(o_number.isApproximate());
			setPrecision(o_number.precision());
			break;
		}
		case giac::_POLY: {
			clear();
			giac_unknown = new giac::gen(giac_gen);
			m_type = STRUCT_UNKNOWN;
			break;
		}
		case giac::_IDNT: {
			if(*giac_gen._IDNTptr == giac::_IDNT_pi) {
				set(CALCULATOR->getPI());
			} else if(*giac_gen._IDNTptr == giac::e__IDNT) {
				set(CALCULATOR->getE());
			} else if(*giac_gen._IDNTptr == giac::_IDNT_undef) {
				setUndefined();
			} else if(*giac_gen._IDNTptr == giac::_IDNT_infinity) {
				setInfinity();
			} else {
				if(giac_gen._IDNTptr->name->length() > 0 && (*giac_gen._IDNTptr->name)[0] == '!') {
					set(giac_gen._IDNTptr->name->substr(1, giac_gen._IDNTptr->name->length() - 1));
				} else if(giac_gen._IDNTptr->name->length() > 1 && (*giac_gen._IDNTptr->name)[1] == '!') {
					if((*giac_gen._IDNTptr->name)[0] == 'v') {
						set(CALCULATOR->getActiveVariable(giac_gen._IDNTptr->name->substr(2, giac_gen._IDNTptr->name->length() - 2)));
					} else if((*giac_gen._IDNTptr->name)[0] == 'u') {
						set(CALCULATOR->getActiveUnit(giac_gen._IDNTptr->name->substr(2, giac_gen._IDNTptr->name->length() - 2)));
					}
				} else {
					set(*giac_gen._IDNTptr->name);
				}
			}
			break;
		}
		case giac::_SYMB: {
			bool b_two = false;
			bool qf = false;
			Function *f = NULL;
			int t = -1;
			if(giac_gen._SYMBptr->sommet == giac::at_prod) {
				b_two = true;
				t = STRUCT_MULTIPLICATION;
			} else if(giac_gen._SYMBptr->sommet == giac::at_pow) {
				if(giac_gen._SYMBptr->feuille.type == giac::_VECT  && !ckmatrix(giac_gen._SYMBptr->feuille) && giac_gen._SYMBptr->feuille._VECTptr->size() == 2) {
					m_type = STRUCT_POWER;
					APPEND(MathStructure((*giac_gen._SYMBptr->feuille._VECTptr)[0]));
					APPEND(MathStructure((*giac_gen._SYMBptr->feuille._VECTptr)[1]));
				} else {
					set(giac_gen._SYMBptr->feuille);
				}
			} else if(giac_gen._SYMBptr->sommet == giac::at_plus) {
				b_two = true;
				t = STRUCT_ADDITION;
			} else if(giac_gen._SYMBptr->sommet == giac::at_division) {
				if(giac_gen._SYMBptr->feuille.type == giac::_VECT  && !ckmatrix(giac_gen._SYMBptr->feuille) && giac_gen._SYMBptr->feuille._VECTptr->size() == 2) {
					t = STRUCT_DIVISION;
				} else {
					set(giac_gen._SYMBptr->feuille);
				}
			} else if(giac_gen._SYMBptr->sommet == giac::at_neg) {
				m_type = STRUCT_NEGATE;
				APPEND(MathStructure(giac_gen._SYMBptr->feuille));
			} else if(giac_gen._SYMBptr->sommet == giac::at_inv) {
				m_type = STRUCT_INVERSE;
				APPEND(MathStructure(giac_gen._SYMBptr->feuille));
			} else if(giac_gen._SYMBptr->sommet == giac::at_not) {
				m_type = STRUCT_NOT;
				APPEND(MathStructure(giac_gen._SYMBptr->feuille));
			} else if(giac_gen._SYMBptr->sommet == giac::at_and) {
				b_two = true;
				t = STRUCT_AND;
			} else if(giac_gen._SYMBptr->sommet == giac::at_ou) {
				b_two = true;
				t = STRUCT_OR;
			} else if(giac_gen._SYMBptr->sommet == giac::at_xor) {
				b_two = true;
				t = STRUCT_XOR;
			} else if(giac_gen._SYMBptr->sommet == giac::at_equal) {
				ct_comp = COMPARISON_EQUALS;
				b_two = true;
				t = STRUCT_COMPARISON;
			} else if(giac_gen._SYMBptr->sommet == giac::at_different) {
				ct_comp = COMPARISON_NOT_EQUALS;
				b_two = true;
				t = STRUCT_COMPARISON;
			} else if(giac_gen._SYMBptr->sommet == giac::at_inferieur_strict) {
				ct_comp = COMPARISON_LESS;
				b_two = true;
				t = STRUCT_COMPARISON;
			} else if(giac_gen._SYMBptr->sommet == giac::at_superieur_strict) {
				ct_comp = COMPARISON_GREATER;
				b_two = true;
				t = STRUCT_COMPARISON;
			} else if(giac_gen._SYMBptr->sommet == giac::at_superieur_egal) {
				ct_comp = COMPARISON_EQUALS_GREATER;
				b_two = true;
				t = STRUCT_COMPARISON;
			} else if(giac_gen._SYMBptr->sommet == giac::at_inferieur_egal) {
				ct_comp = COMPARISON_EQUALS_LESS;
				b_two = true;
				t = STRUCT_COMPARISON;
			} else if(giac_gen._SYMBptr->sommet == giac::at_not) {
				m_type = STRUCT_NOT;
				APPEND(MathStructure(giac_gen._SYMBptr->feuille));
			} else if(giac_gen._SYMBptr->sommet == giac::at_factorial) {
				f = CALCULATOR->f_factorial;
			} else if(giac_gen._SYMBptr->sommet == giac::at_abs) {
				f = CALCULATOR->f_abs;
			} else if(giac_gen._SYMBptr->sommet == giac::at_round) {
				f = CALCULATOR->f_round;
			} else if(giac_gen._SYMBptr->sommet == giac::at_floor) {
				f = CALCULATOR->f_floor;
			} else if(giac_gen._SYMBptr->sommet == giac::at_ceil) {
				f = CALCULATOR->f_ceil;
			} else if(giac_gen._SYMBptr->sommet == giac::at_rem) {
				f = CALCULATOR->f_rem;
			} else if(giac_gen._SYMBptr->sommet == giac::at_re) {
				f = CALCULATOR->f_re;
			} else if(giac_gen._SYMBptr->sommet == giac::at_im) {
				f = CALCULATOR->f_im;
			} else if(giac_gen._SYMBptr->sommet == giac::at_arg) {
				f = CALCULATOR->f_arg;
			} else if(giac_gen._SYMBptr->sommet == giac::at_sqrt) {
				f = CALCULATOR->f_sqrt;
			} else if(giac_gen._SYMBptr->sommet == giac::at_sq) {
				f = CALCULATOR->f_sq;
			} else if(giac_gen._SYMBptr->sommet == giac::at_exp) {
				f = CALCULATOR->f_exp;
			} else if(giac_gen._SYMBptr->sommet == giac::at_ln) {
				f = CALCULATOR->f_ln;
			} else if(giac_gen._SYMBptr->sommet == giac::at_log10) {
				f = CALCULATOR->f_log10;
			} else if(giac_gen._SYMBptr->sommet == giac::at_alog10) {
				f = CALCULATOR->f_alog10;
			} else if(giac_gen._SYMBptr->sommet == giac::at_sin) {
				f = CALCULATOR->f_sin;
			} else if(giac_gen._SYMBptr->sommet == giac::at_cos) {
				f = CALCULATOR->f_cos;
			} else if(giac_gen._SYMBptr->sommet == giac::at_tan) {
				f = CALCULATOR->f_tan;
			} else if(giac_gen._SYMBptr->sommet == giac::at_asin) {
				f = CALCULATOR->f_asin;
			} else if(giac_gen._SYMBptr->sommet == giac::at_acos) {
				f = CALCULATOR->f_acos;
			} else if(giac_gen._SYMBptr->sommet == giac::at_atan) {
				f = CALCULATOR->f_atan;
			} else if(giac_gen._SYMBptr->sommet == giac::at_sinh) {
				f = CALCULATOR->f_sinh;
			} else if(giac_gen._SYMBptr->sommet == giac::at_cosh) {
				f = CALCULATOR->f_cosh;
			} else if(giac_gen._SYMBptr->sommet == giac::at_tanh) {
				f = CALCULATOR->f_tanh;
			} else if(giac_gen._SYMBptr->sommet == giac::at_asinh) {
				f = CALCULATOR->f_asinh;
			} else if(giac_gen._SYMBptr->sommet == giac::at_acosh) {
				f = CALCULATOR->f_acosh;
			} else if(giac_gen._SYMBptr->sommet == giac::at_atanh) {
				f = CALCULATOR->f_atanh;
			} else if(giac_gen._SYMBptr->sommet == giac::at_zeta) {
				f = CALCULATOR->f_zeta;
			} else if(giac_gen._SYMBptr->sommet == giac::at_gamma) {
				f = CALCULATOR->f_gamma;
			} else if(giac_gen._SYMBptr->sommet == giac::at_beta) {
				f = CALCULATOR->f_beta;
			} else if(giac_gen._SYMBptr->sommet == giac::at_psi) {
				f = CALCULATOR->f_psi;
			} else if(giac_gen._SYMBptr->sommet == giac::at_solve) {
				f = CALCULATOR->f_solve;
			} else if(giac_gen._SYMBptr->sommet == giac::at_derive) {
				f = CALCULATOR->f_diff;
			} else if(giac_gen._SYMBptr->sommet == giac::at_integrate) {
				f = CALCULATOR->f_integrate;
			} else if(giac_gen._SYMBptr->sommet == giac::at_limit) {
				f = CALCULATOR->f_limit;
			} else if(giac_gen._SYMBptr->sommet == giac::at_bernoulli) {
				f = CALCULATOR->f_bernoulli;
			} else if(giac_gen._SYMBptr->sommet == giac::at_romberg) {
				f = CALCULATOR->f_romberg;
			} else if(giac_gen._SYMBptr->sommet == giac::at_fourier_an) {
				f = CALCULATOR->f_fourier_an;
			} else if(giac_gen._SYMBptr->sommet == giac::at_fourier_bn) {
				f = CALCULATOR->f_fourier_bn;
			} else if(giac_gen._SYMBptr->sommet == giac::at_fourier_cn) {
				f = CALCULATOR->f_fourier_cn;
			} else if(giac_gen._SYMBptr->sommet == giac::at_rand) {
				f = CALCULATOR->f_rand;
			} else if(giac_gen._SYMBptr->sommet == giac::at_qalculate_function) {
				qf = true;
				t = STRUCT_FUNCTION;
			} else {
				giac_unknown = new giac::gen(giac_gen);
				m_type = STRUCT_UNKNOWN;
			}
			if(t >= 0 || f) {
				if(f) {
					m_type = STRUCT_FUNCTION;
					o_function = f;
				} else {
					m_type = t;
				}
				if(giac_gen._SYMBptr->feuille.type == giac::_VECT && !ckmatrix(giac_gen._SYMBptr->feuille)) {
					if(giac_gen._SYMBptr->feuille._VECTptr->size() == 1 && b_two) {
						set((*giac_gen._SYMBptr->feuille._VECTptr)[0]);
					} else {
						for(unsigned int i = 0; i < giac_gen._SYMBptr->feuille._VECTptr->size(); i++) {
							if(i == 0 && qf) {
								if((*giac_gen._SYMBptr->feuille._VECTptr)[i].type == giac::_IDNT) {
									o_function = (Function*) s2p(*(*giac_gen._SYMBptr->feuille._VECTptr)[i]._IDNTptr->name);
								} else {
									giac_unknown = new giac::gen(giac_gen);
									m_type = STRUCT_UNKNOWN;
									break;
								}
							} else {
								APPEND(MathStructure((*giac_gen._SYMBptr->feuille._VECTptr)[i]));
							}
						}
					}
				} else {
					if(b_two) {
						set(giac_gen._SYMBptr->feuille);
					} else if(qf) {
						if(giac_gen.type == giac::_POINTER_) {
							o_function = (Function*) giac_gen._POINTER_val;
						} else {
							giac_unknown = new giac::gen(giac_gen);
							m_type = STRUCT_UNKNOWN;
							break;
						}
					} else {
						APPEND(MathStructure(giac_gen._SYMBptr->feuille));
					}
				}
			}
			break;
		}
		case giac::_FRAC: {
			set(giac_gen._FRACptr->num);
			MathStructure mstruct(giac_gen._FRACptr->den);
			if(isNumber() && mstruct.isNumber()) {
				o_number.setInternal(giac_gen);
				setApproximate(o_number.isApproximate());
				setPrecision(o_number.precision());
			} else {
				add(mstruct, OPERATION_DIVIDE);
			}
			break;
		}
		case giac::_STRNG: {
			set(*giac_gen._STRNGptr);
			break;
		}
		case giac::_VECT: {
			if(giac_gen._VECTptr->size() == 1) {
				set((*giac_gen._VECTptr)[0]);
				break;
			}
			m_type = STRUCT_VECTOR;
			for(unsigned int i = 0; i < giac_gen._VECTptr->size(); i++) {
				APPEND(MathStructure((*giac_gen._VECTptr)[i]));
			}
			break;
		}
		case giac::_USER: {}
		case giac::_EXT: {}
		case giac::_FUNC: {}
		case giac::_ROOT: {}
		case giac::_MOD: {}
		case giac::_SPOL1: {}
		default: {
			clear();
			giac_unknown = new giac::gen(giac_gen);
			m_type = STRUCT_UNKNOWN;
			break;
		}
	}	
}
MathStructure::MathStructure(const giac::gen &giac_gen) {
	init();
	set(giac_gen, false);
}
#endif

void MathStructure::addChild(const MathStructure &o) {
	APPEND(o);
}
void MathStructure::delChild(unsigned int index) {
	if(index > 0 && index <= SIZE) {
		ERASE(index - 1);
	}
}
void MathStructure::insertChild(const MathStructure &o, unsigned int index) {
	if(index > 0 && index <= v_subs.size()) {
		v_order.insert(v_order.begin() + (index - 1), v_subs.size());
		v_subs.push_back(o);
	} else {
		addChild(o);
	}
}
void MathStructure::setChild(const MathStructure &o, unsigned int index) {
	if(index > 0 && index <= SIZE) {
		CHILD(index - 1) = o;
	}
}
const MathStructure *MathStructure::getChild(unsigned int index) const {
	if(index > 0 && index <= v_order.size()) {
		return &CHILD(index - 1);
	}
	return NULL;
}
MathStructure *MathStructure::getChild(unsigned int index) {
	if(index > 0 && index <= v_order.size()) {
		return &CHILD(index - 1);
	}
	return NULL;
}
unsigned int MathStructure::countChilds() const {
	return SIZE;
}
unsigned int MathStructure::size() const {
	return SIZE;
}
const MathStructure *MathStructure::base() const {
	if(m_type == STRUCT_POWER && SIZE >= 1) {
		return &CHILD(0);
	}
	return NULL;
}
const MathStructure *MathStructure::exponent() const {
	if(m_type == STRUCT_POWER && SIZE >= 2) {
		return &CHILD(1);
	}
	return NULL;
}
MathStructure *MathStructure::base() {
	if(m_type == STRUCT_POWER && SIZE >= 1) {
		return &CHILD(0);
	}
	return NULL;
}
MathStructure *MathStructure::exponent() {
	if(m_type == STRUCT_POWER && SIZE >= 2) {
		return &CHILD(1);
	}
	return NULL;
}
void MathStructure::addAlternative(const MathStructure &o) {
	if(m_type != STRUCT_ALTERNATIVES) {
		MathStructure copy_this(*this);
		clear();
		m_type = STRUCT_ALTERNATIVES;
		APPEND(copy_this);
	}
	APPEND(o);
}

int MathStructure::type() const {
	return m_type;
}
void MathStructure::unformat(const EvaluationOptions &eo) {
	for(unsigned int i = 0; i < SIZE; i++) {
		CHILD(i).unformat(eo);
	}
	switch(m_type) {
		case STRUCT_INVERSE: {
			APPEND(m_minus_one);
			m_type = STRUCT_POWER;	
		}
		case STRUCT_NEGATE: {
			PREPEND(m_minus_one);
			m_type = STRUCT_MULTIPLICATION;
		}
		case STRUCT_DIVISION: {
			CHILD(1).raise(m_minus_one);
			m_type = STRUCT_MULTIPLICATION;
		}
		case STRUCT_UNIT: {
			if(o_prefix && !eo.keep_prefixes) {
				if(o_prefix == CALCULATOR->null_prefix) {
					o_prefix = NULL;
				} else {
					Unit *u = o_unit;
					Prefix *p = o_prefix;
					set(10);
					raise(p->exponent());
					multiply(u);
				}
			}
			b_plural = false;
		}
	}
}

void idm1(const MathStructure &mnum, bool &bfrac, bool &bint) {
	switch(mnum.type()) {
		case STRUCT_NUMBER: {
			if((!bfrac || bint) && mnum.number().isRational()) {
				if(!mnum.number().isInteger()) {
					bint = false;
					bfrac = true;
				}
			} else {
				bint = false;
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if((!bfrac || bint) && mnum.size() > 0 && mnum[0].isNumber() && mnum[0].number().isRational()) {
				if(!mnum[0].number().isInteger()) {
					bint = false;
					bfrac = true;
				}
				
			} else {
				bint = false;
			}
			break;
		}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < mnum.size() && (!bfrac || bint); i++) {
				idm1(mnum[i], bfrac, bint);
			}
			break;
		}
		default: {
			bint = false;
		}
	}
}
void idm2(const MathStructure &mnum, bool &bfrac, bool &bint, Number &nr) {
	switch(mnum.type()) {
		case STRUCT_NUMBER: {
			if(mnum.number().isRational()) {
				if(mnum.number().isInteger()) {
					if(bint) {
						if(mnum.number().isOne()) {
							bint = false;
						} else if(nr.isOne()) {
							nr = mnum.number();
						} else if(nr != mnum.number()) {
							nr.gcd(mnum.number());
							if(nr.isOne()) bint = false;
						}
					}
				} else {
					if(nr.isOne()) {
						nr = mnum.number().denominator();
					} else {
						Number nden(mnum.number().denominator());
						if(nr != nden) {
							Number ngcd(nden);
							ngcd.gcd(nr);
							nden /= ngcd;
							nr *= nden;
						}
					}
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if(mnum.size() > 0 && mnum[0].isNumber() && mnum[0].number().isRational()) {
				if(mnum[0].number().isInteger()) {
					if(bint) {
						if(mnum[0].number().isOne()) {
							bint = false;
						} else if(nr.isOne()) {
							nr = mnum[0].number();
						} else if(nr != mnum[0].number()) {
							nr.gcd(mnum[0].number());
							if(nr.isOne()) bint = false;
						}
					}
				} else {
					if(nr.isOne()) {
						nr = mnum[0].number().denominator();
					} else {
						Number nden(mnum[0].number().denominator());
						if(nr != nden) {
							Number ngcd(nden);
							ngcd.gcd(nr);
							nden /= ngcd;
							nr *= nden;
						}
					}
				}
			}
			break;
		}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < mnum.size() && (bfrac || bint); i++) {
				idm2(mnum[i], bfrac, bint, nr);
			}
			break;
		}
	}
}
void idm3(MathStructure &mnum, Number &nr) {
	switch(mnum.type()) {
		case STRUCT_NUMBER: {
			mnum.number() *= nr;
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if(mnum.size() > 0 && mnum[0].isNumber()) {
				mnum[0].number() *= nr;
			} else {
				mnum.insertChild(nr, 1);
			}
			break;
		}
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < mnum.size(); i++) {
				idm3(mnum[i], nr);
			}
			break;
		}
		default: {
			mnum.transform(STRUCT_MULTIPLICATION);
			mnum.insertChild(nr, 1);
		}
	}
}

bool MathStructure::improve_division_multipliers(const PrintOptions &po) {
	switch(m_type) {
		case STRUCT_MULTIPLICATION: {
			unsigned int inum = 0, iden = 0;
			bool bfrac = false, bint = true, bdiv = false;
			unsigned int index1, index2;
			bool dofrac = !po.negative_exponents;
			for(unsigned int i2 = 0; i2 < SIZE; i2++) {
				if(CHILD(i2).isPower() && CHILD(i2)[1].isMinusOne()) {
					if(!po.place_units_separately || !CHILD(i2)[0].isUnit()) {
						if(iden == 0) index1 = i2;
						iden++;
						bdiv = true;
						if(CHILD(i2)[0].contains(STRUCT_ADDITION)) {
							dofrac = true;
						}
					}
				} else if(!bdiv && !po.negative_exponents && CHILD(i2).isPower() && CHILD(i2)[1].hasNegativeSign()) {
					if(!po.place_units_separately || !CHILD(i2)[0].isUnit()) {
						if(!bdiv) index1 = i2;
						bdiv = true;
					}
				} else {
					if(!po.place_units_separately || !CHILD(i2).isUnit_exp()) {
						if(inum == 0) index2 = i2;
						inum++;
					}
				}
			}
			if(!bdiv) break;
			if(iden > 1 && !po.negative_exponents) {
				for(int i2 = index1 + 1; i2 < (int) SIZE; i2++) {
					if(CHILD(i2).isPower() && CHILD(i2)[1].isMinusOne()) {
						CHILD(index1)[0].multiply(CHILD(i2)[0], true);
						ERASE(i2);
						i2--;
					}
				}
				iden = 1;
			}
			if(bint) bint = inum > 0 && iden == 1;
			if(inum > 0) idm1(CHILD(index2), bfrac, bint);
			if(iden > 0) idm1(CHILD(index1)[0], bfrac, bint);
			bool b = false;
			if(!dofrac) bfrac = false;
			if(bint || bfrac) {
				Number nr(1, 1);
				if(inum > 0) idm2(CHILD(index2), bfrac, bint, nr);
				if(iden > 0) idm2(CHILD(index1)[0], bfrac, bint, nr);
				if((bint || bfrac) && !nr.isOne()) {
					if(bint) {
						nr.recip();
					}
					if(inum == 0) {
						PREPEND(MathStructure(nr));
					} else if(inum > 1 && !CHILD(index2).isNumber()) {
						idm3(*this, nr);
					} else {
						idm3(CHILD(index2), nr);
					}
					if(iden > 0) {
						idm3(CHILD(index1)[0], nr);
					} else {
						MathStructure mstruct(nr);
						mstruct.raise(m_minus_one);
						insertChild(mstruct, index1);
					}
					b = true;
				}
			}
			/*if(!po.negative_exponents && SIZE == 2 && CHILD(1).isAddition()) {
				MathStructure factor_mstruct(1, 1);
				if(factorize_find_multiplier(CHILD(1), CHILD(1), factor_mstruct)) {
					transform(STRUCT_MULTIPLICATION);
					PREPEND(factor_mstruct);
				}
			}*/
			return b;
		}
		case STRUCT_DIVISION: {
			bool bint = true, bfrac = false;
			idm1(CHILD(0), bfrac, bint);
			idm1(CHILD(1), bfrac, bint);
			if(bint || bfrac) {
				Number nr(1, 1);
				idm2(CHILD(0), bfrac, bint, nr);
				idm2(CHILD(1), bfrac, bint, nr);
				if((bint || bfrac) && !nr.isOne()) {
					if(bint) {
						nr.recip();
					}
					idm3(CHILD(0), nr);
					idm3(CHILD(1), nr);
					return true;
				}
			}
			break;
		}
		case STRUCT_INVERSE: {
			bool bint = false, bfrac = false;
			idm1(CHILD(0), bfrac, bint);
			if(bint || bfrac) {
				Number nr(1, 1);
				idm2(CHILD(0), bfrac, bint, nr);
				if((bint || bfrac) && !nr.isOne()) {
					MathStructure mdiv(CHILD(0));
					clear();
					m_type = STRUCT_DIVISION;
					idm3(mdiv, nr);
					APPEND(nr);
					APPEND(mdiv);
					return true;
				}
			}
			break;
		}
		case STRUCT_POWER: {
			if(CHILD(1).isMinusOne()) {
				bool bint = false, bfrac = false;
				idm1(CHILD(0), bfrac, bint);
				if(bint || bfrac) {
					Number nr(1, 1);
					idm2(CHILD(0), bfrac, bint, nr);
					if((bint || bfrac) && !nr.isOne()) {
						idm3(CHILD(0), nr);
						transform(STRUCT_MULTIPLICATION);
						PREPEND(MathStructure(nr));
						return true;
					}
				}
				break;
			}
		}
		default: {
			bool b = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).improve_division_multipliers()) b = true;
			}
			return b;
		}
	}
	return false;
}

void MathStructure::setPrefixes(const PrintOptions &po, MathStructure *parent, unsigned int pindex) {
	switch(m_type) {
		case STRUCT_MULTIPLICATION: {
			bool b = false;
			unsigned int i = SIZE;
			for(unsigned int i2 = 0; i2 < SIZE; i2++) {
				if(CHILD(i2).isUnit_exp()) {
					if((CHILD(i2).isUnit() && CHILD(i2).prefix()) || (CHILD(i2).isPower() && CHILD(i2)[0].prefix())) {
						b = false;
						return;
					}
					if(po.use_prefixes_for_currencies || (CHILD(i2).isPower() && !CHILD(i2)[0].unit()->isCurrency()) || (CHILD(i2).isUnit() && !CHILD(i2).unit()->isCurrency())) {
						b = true;
						if(i > i2) i = i2;
						break;
					} else if(i < i2) {
						i = i2;
					}
				}
			}
			if(b) {
				Number exp(1, 1);
				Number exp2(1, 1);
				bool b2 = false;
				unsigned int i2 = i;
				if(CHILD(i).isPower()) {
					if(CHILD(i)[1].isNumber() && CHILD(i)[1].number().isInteger()) {
						exp = CHILD(i)[1].number();
					} else {
						b = false;
					}
				}
				if(po.use_denominator_prefix && !exp.isNegative()) {
					for(; i2 < SIZE; i2++) {
						if(CHILD(i2).isPower() && CHILD(i2)[0].isUnit() && CHILD(i2)[1].isNumber() && CHILD(i2)[1].number().isNegative()) {
							if(CHILD(i2)[1].prefix() || !CHILD(i2)[1].number().isInteger()) {
								break;
							}
							if(!b) {
								b = true;
								exp = CHILD(i2)[1].number();
								i = i2;
							} else {
								b2 = true;
								exp2 = CHILD(i2)[1].number();
							}
							break;
						}
					}
				} else if(exp.isNegative() && b) {
					for(; i2 < SIZE; i2++) {
						if(CHILD(i2).isPower() && CHILD(i2)[0].isUnit() && !(CHILD(i2)[1].isNumber() && CHILD(i2)[1].number().isNegative())) {
							if(CHILD(i2)[1].prefix() || !CHILD(i2)[1].number().isInteger()) {
								if(!po.use_denominator_prefix) {
									b = false;
								}
								break;
							}
							if(po.use_denominator_prefix) {
								b2 = true;
								exp2 = exp;
								exp = CHILD(i2)[1].number();
								unsigned int i3 = i;
								i = i2;
								i2 = i3;
							} else {
								i = i2;
								exp = CHILD(i2)[1].number();
							}
							break;
						} else if(CHILD(i2).isUnit()) {
							if(po.use_denominator_prefix) {
								b2 = true;
								exp2 = exp;
								exp.set(1, 1);
								unsigned int i3 = i;
								i = i2;
								i2 = i3;
							} else {
								i = i2;
								exp.set(1, 1);
							}
							break;
						}
					}
				}
				Number exp10;
				if(b) {
					if(po.prefix) {
						if(po.prefix != CALCULATOR->null_prefix)  {
							if(CHILD(i).isUnit()) CHILD(i).setPrefix(po.prefix);
							else CHILD(i)[0].setPrefix(po.prefix);
							if(CHILD(0).isNumber()) {
								CHILD(0).number() /= po.prefix->value(exp);
							} else {
								PREPEND(po.prefix->value(exp));
								CHILD(0).number().recip();
							}
							if(b2) {
								exp10 = CHILD(0).number();
								exp10.log(10);
								exp10.floor();
							}
						}
					} else if(po.use_unit_prefixes && CHILD(0).isNumber() && exp.isInteger()) {
						exp10 = CHILD(0).number();
						exp10.log(10);
						exp10.floor();
						if(b2) {	
							Number tmp_exp(exp10);
							tmp_exp.setNegative(false);
							Number e1(3, 1);
							e1 *= exp;
							Number e2(3, 1);
							e2 *= exp2;
							e2.setNegative(false);
							int i = 0;
							while(true) {
								tmp_exp -= e1;
								if(!tmp_exp.isPositive()) {
									break;
								}
								if(exp10.isNegative()) i++;
								tmp_exp -= e2;
								if(tmp_exp.isNegative()) {
									break;
								}
								if(!exp10.isNegative())	i++;
							}
							e2.setNegative(exp10.isNegative());
							e2 *= i;
							exp10 -= e2;
						}
						Prefix *p = CALCULATOR->getBestPrefix(exp10, exp, po.use_all_prefixes);
						if(p) {
							Number test_exp(exp10);
							test_exp -= p->exponent(exp);
							if(test_exp.isInteger()) {
								if((exp10.isPositive() && exp10.compare(test_exp) == COMPARISON_RESULT_LESS) || (exp10.isNegative() && exp10.compare(test_exp) == COMPARISON_RESULT_GREATER)) {
									CHILD(0).number() /= p->value(exp);
									if(CHILD(i).isUnit()) CHILD(i).setPrefix(p);
									else CHILD(i)[0].setPrefix(p);
								}
							}
						}
					}
					if(b2 && CHILD(0).isNumber() && ((po.prefix && po.prefix != CALCULATOR->null_prefix) || po.use_unit_prefixes)) {
						exp10 = CHILD(0).number();
						exp10.log(10);
						exp10.floor();
						Prefix *p = CALCULATOR->getBestPrefix(exp10, exp2, po.use_all_prefixes);
						if(p) {
							Number test_exp(exp10);
							test_exp -= p->exponent(exp2);
							if(test_exp.isInteger()) {
								if((exp10.isPositive() && exp10.compare(test_exp) == COMPARISON_RESULT_LESS) || (exp10.isNegative() && exp10.compare(test_exp) == COMPARISON_RESULT_GREATER)) {
									CHILD(0).number() /= p->value(exp2);
									CHILD(i2)[0].setPrefix(p);
								}
							}
						}
					}	
				}
				break;
			}
		}
		case STRUCT_UNIT: {
			if(!o_prefix && (po.prefix && po.prefix != CALCULATOR->null_prefix)) {
				Unit *u = o_unit;
				clear();
				APPEND(MathStructure(1));
				APPEND(MathStructure(u));
				m_type = STRUCT_MULTIPLICATION;
				setPrefixes(po, parent, pindex);
			}
			break;
		}
		case STRUCT_POWER: {
			if(CHILD(0).isUnit()) {
				if(CHILD(1).isNumber() && CHILD(1).number().isReal() && !o_prefix && (po.prefix && po.prefix != CALCULATOR->null_prefix)) {
					MathStructure msave(*this);
					clear();
					APPEND(MathStructure(1));
					APPEND(msave);
					m_type = STRUCT_MULTIPLICATION;
					setPrefixes(po, parent, pindex);
				}
				break;
			}
		}
		default: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).setPrefixes(po, this, i + 1);
			}
		}
	}
}
void MathStructure::postFormatUnits(const PrintOptions &po, MathStructure *parent, unsigned int pindex) {
	switch(m_type) {
		case STRUCT_DIVISION: {
			if(po.place_units_separately) {
				vector<unsigned int> nums;
				bool b1 = false, b2 = false;
				if(CHILD(0).isMultiplication()) {
					for(unsigned int i = 0; i < CHILD(0).size(); i++) {
						if(CHILD(0)[i].isUnit_exp()) {
							nums.push_back(i);
						} else {
							b1 = true;
						}
					}
					b1 = b1 && !nums.empty();
				} else if(CHILD(0).isUnit_exp()) {
					b1 = true;
				}
				vector<unsigned int> dens;
				if(CHILD(1).isMultiplication()) {
					for(unsigned int i = 0; i < CHILD(1).size(); i++) {
						if(CHILD(1)[i].isUnit_exp()) {
							dens.push_back(i);
						} else {
							b2 = true;
						}
					}
					b2 = b2 && !dens.empty();
				} else if(CHILD(1).isUnit_exp()) {
					if(CHILD(0).isUnit_exp()) {
						b1 = false;
					} else {
						b2 = true;
					}
				}
				if(b2 && !b1) b1 = true;
				if(b1) {
					MathStructure num = m_undefined;
					if(CHILD(0).isUnit_exp()) {
						num = CHILD(0);
						CHILD(0) = 1;
					} else if(nums.size() > 0) {
						num = CHILD(0)[nums[0]];
						for(unsigned int i = 1; i < nums.size(); i++) {
							num.multiply(CHILD(0)[nums[i]], i > 1);
						}
						for(unsigned int i = 0; i < nums.size(); i++) {
							CHILD(0).delChild(nums[i] + 1 - i);
						}
						if(CHILD(0).size() == 1) {
							MathStructure msave(CHILD(0)[0]);
							CHILD(0) = msave;
						}
					}
					MathStructure den = m_undefined;
					if(CHILD(1).isUnit_exp()) {
						den = CHILD(1);
						MathStructure msave(CHILD(0));
						set(msave);
					} else if(dens.size() > 0) {
						den = CHILD(1)[dens[0]];
						for(unsigned int i = 1; i < dens.size(); i++) {
							den.multiply(CHILD(1)[dens[i]], i > 1);
						}
						for(unsigned int i = 0; i < dens.size(); i++) {
							CHILD(1).delChild(dens[i] + 1 - i);
						}
						if(CHILD(1).size() == 1) {
							MathStructure msave(CHILD(1)[0]);
							CHILD(1) = msave;
						}
					}
					if(num.isUndefined()) {
						transform(STRUCT_DIVISION, den);
					} else {
						if(!den.isUndefined()) {
							num.transform(STRUCT_DIVISION, den);
						}
						multiply(num, false);
					}
					if(CHILD(0).isDivision()) {
						if(CHILD(0)[0].isMultiplication()) {
							if(CHILD(0)[0].size() == 1) {
								MathStructure msave(CHILD(0)[0][0]);
								CHILD(0)[0] = msave;
							} else if(CHILD(0)[0].size() == 0) {
								CHILD(0)[0] = 1;
							}
						}
						if(CHILD(0)[1].isMultiplication()) {
							if(CHILD(0)[1].size() == 1) {
								MathStructure msave(CHILD(0)[1][0]);
								CHILD(0)[1] = msave;
							} else if(CHILD(0)[1].size() == 0) {
								MathStructure msave(CHILD(0)[0]);
								CHILD(0) = msave;
							}
						} else if(CHILD(0)[1].isOne()) {
							MathStructure msave(CHILD(0)[0]);
							CHILD(0) = msave;
						}
						if(CHILD(0).isDivision() && CHILD(0)[1].isNumber() && CHILD(0)[0].isMultiplication() && CHILD(0)[0].size() > 1 && CHILD(0)[0][0].isNumber()) {
							MathStructure msave;
							if(CHILD(0)[0].size() == 2) {
								MathStructure msavenum(CHILD(0)[0][0]);
								msave = CHILD(0)[0][1];
								CHILD(0)[0] = msavenum;	
							} else {
								msave = CHILD(0)[0];
								CHILD(0)[0] = msave[0];
								msave.delChild(1);
							}
							if(isMultiplication()) {
								insertChild(msave, 2);
							} else {
								CHILD(0) *= msave;
							}
						}
					}
					bool do_plural = true;
					switch(CHILD(0).type()) {
						case STRUCT_NUMBER: {
							if(CHILD(0).isZero() || CHILD(0).number().isOne() || CHILD(0).number().isMinusOne() || CHILD(0).number().isFraction()) {
								do_plural = false;
							}
							break;
						}
						case STRUCT_DIVISION: {
							if(CHILD(0)[0].isNumber() && CHILD(0)[1].isNumber()) {
								if(CHILD(0)[0].number().isLessThanOrEqualTo(CHILD(0)[1].number())) {
									do_plural = false;
								}
							}
							break;
						}
						case STRUCT_INVERSE: {
							if(CHILD(0)[0].isNumber() && CHILD(0)[0].number().isGreaterThanOrEqualTo(1)) {
								do_plural = false;
							}
							break;
						}
					}
					switch(CHILD(1).type()) {
						case STRUCT_UNIT: {
							CHILD(1).setPlural(do_plural);
							break;
						}
						case STRUCT_POWER: {
							CHILD(1)[0].setPlural(do_plural);
							break;
						}
						case STRUCT_MULTIPLICATION: {
							CHILD(1)[CHILD(1).size() - 1].setPlural(do_plural);
							break;
						}
						case STRUCT_DIVISION: {
							switch(CHILD(1)[0].type()) {
								case STRUCT_UNIT: {
									CHILD(1)[0].setPlural(do_plural);
									break;
								}
								case STRUCT_POWER: {
									CHILD(1)[0][0].setPlural(do_plural);
									break;
								}
								case STRUCT_MULTIPLICATION: {
									CHILD(1)[0][CHILD(1)[0].size() - 1].setPlural(do_plural);
									break;
								}
							}
							break;
						}
						
					}
				}
			} else {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).postFormatUnits(po, this, i + 1);
				}
			}
			break;
		}
		case STRUCT_UNIT: {
			b_plural = false;
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if(SIZE > 1 && CHILD(1).isUnit_exp() && CHILD(0).isNumber()) {
				bool do_plural = !(CHILD(0).isZero() || CHILD(0).number().isOne() || CHILD(0).number().isMinusOne() || CHILD(0).number().isFraction());
				unsigned int i = 2;
				for(; i < SIZE; i++) {
					if(CHILD(i).isUnit()) {
						CHILD(i).setPlural(false);
					} else if(CHILD(i).isPower() && CHILD(i)[0].isUnit()) {
						CHILD(i)[0].setPlural(false);
					} else {
						break;
					}
				}
				if(do_plural) {
					i--;
					if(CHILD(i).isUnit()) {
						CHILD(i).setPlural(true);
					} else {
						CHILD(i)[0].setPlural(true);
					}
				}
			} else if(SIZE > 0) {
				int last_unit = -1;
				for(unsigned int i = 0; i < SIZE; i++) {
					if(CHILD(i).isUnit()) {
						CHILD(i).setPlural(false);
						last_unit = i;
					} else if(CHILD(i).isPower() && CHILD(i)[0].isUnit()) {
						CHILD(i)[0].setPlural(false);
						last_unit = i;
					} else if(last_unit >= 0) {
						break;
					}
				}
				if(last_unit > 0) {
					if(CHILD(last_unit).isUnit()) {
						CHILD(last_unit).setPlural(true);
					} else {
						CHILD(last_unit)[0].setPlural(true);
					}
				}
			}
			break;
		}
		case STRUCT_POWER: {
			if(CHILD(0).isUnit()) {
				CHILD(0).setPlural(false);
				break;
			}
		}
		default: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).postFormatUnits(po, this, i + 1);
			}
		}
	}
}
void MathStructure::prefixCurrencies() {
	if(isMultiplication()) {
		int index = -1;
		for(unsigned int i = 0; i < SIZE; i++) {
			if(CHILD(i).isUnit_exp()) {
				if(CHILD(i).isUnit() && CHILD(i).unit()->isCurrency()) {
					if(index >= 0) {
						index = -1;
						break;
					}
					index = i;
				} else {
					index = -1;
					break;
				}
			}
		}
		if(index >= 0) {
			v_order.insert(v_order.begin(), v_order[index]);
			v_order.erase(v_order.begin() + (index + 1));
		}
	} else {
		for(unsigned int i = 0; i < SIZE; i++) {
			CHILD(i).prefixCurrencies();
		}
	}
}
void MathStructure::format(const PrintOptions &po) {
	sort(po);
	if(po.improve_division_multipliers) {
		if(improve_division_multipliers(po)) sort(po);
	}
	setPrefixes(po);
	formatsub(po);
	postFormatUnits(po);
	if(po.sort_options.prefix_currencies && po.abbreviate_names && CALCULATOR->place_currency_code_before) {
		prefixCurrencies();
	}
}
void MathStructure::formatsub(const PrintOptions &po, MathStructure *parent, unsigned int pindex) {

	for(unsigned int i = 0; i < SIZE; i++) {
		CHILD(i).formatsub(po, this, i + 1);
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
/*			MathStructure *mi, *mi2;
			bool bm1, bm2;
			bool b = false, c = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).isNegate()) {bm1 = true; mi = &CHILD(i)[0];}
				else {bm1 = false; mi = &CHILD(i);}
				b = false;
				if(mi->isDivision()) {
					for(unsigned int i2 = i + 1; i2 < SIZE; i2++) {
						if(CHILD(i2).isNegate()) {bm2 = true; mi2 = &CHILD(i2)[0];}
						else {bm2 = false; mi2 = &CHILD(i2);}
						if(mi2->isDivision() && (*mi)[1] == (*mi2)[1]) {
							(*mi)[0].add((*mi2)[0], true);
							if(bm1 != bm2) {
								(*mi)[0][(*mi)[0].size() - 1].transform(STRUCT_NEGATE);
							}
							ERASE(i2);
							if(CHILD(i).isNegate()) {mi = &CHILD(i)[0];}
							else {mi = &CHILD(i);}
							b = true;
							i2--;
						} else if(mi2->isInverse() && (*mi)[1] == (*mi2)[0]) {
							(*mi)[0].add(m_one, true);
							if(bm1 != bm2) {
								(*mi)[0][(*mi)[0].size() - 1].transform(STRUCT_NEGATE);
							}
							ERASE(i2);
							if(CHILD(i).isNegate()) {mi = &CHILD(i)[0];}
							else {mi = &CHILD(i);}
							b = true;
							i2--;
						}
					}
				} else if(mi->isInverse()) {
					for(unsigned int i2 = i + 1; i2 < SIZE; i2++) {
						if(CHILD(i2).isNegate()) {bm2 = true; mi2 = &CHILD(i2)[0];}
						else {bm2 = false; mi2 = &CHILD(i2);}
						if(mi2->isDivision() && (*mi)[0] == (*mi2)[1]) {
							mi->set(1, 1);
							mi->transform(STRUCT_DIVISION, (*mi2)[1]);
							(*mi)[0].add((*mi2)[0], true);
							if(bm1 != bm2) {
								(*mi)[0][(*mi)[0].size() - 1].transform(STRUCT_NEGATE);
							}
							ERASE(i2);
							if(CHILD(i).isNegate()) {mi = &CHILD(i)[0];}
							else {mi = &CHILD(i);}
							b = true;
							i--;
							break;
						}
					}
				}
				if(b) {
					(*mi)[0].sort(po, false);
					c = true;
				}
			}
			if(c) {
				if(SIZE == 1) {
					MathStructure msave(CHILD(0));
					set(msave);
				} else {
					sort(po, false);
				}
			}*/
			break;
		}
		case STRUCT_DIVISION: {
			if(CHILD(0).isAddition() && CHILD(0).size() > 0) {
				bool b = true;
				for(unsigned int i = 0; i < CHILD(0).size(); i++) {
					if(!CHILD(0)[i].isNegate()) {
						b = false;
						break;
					}
				}
				if(b) {
					for(unsigned int i = 0; i < CHILD(0).size(); i++) {
						MathStructure msave(CHILD(0)[i][0]);
						CHILD(0)[i] = msave;
					}
					transform(STRUCT_NEGATE);
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {

			if(CHILD(0).isNegate()) {
				CHILD(0).number().negate();
				if(CHILD(0)[0].isOne()) {
					ERASE(0);
					if(SIZE == 1) {
						MathStructure mmove(CHILD(0));
						set(mmove);
					}
				} else {
					MathStructure mmove(CHILD(0)[0]);
					CHILD(0) = mmove;
				}
				transform(STRUCT_NEGATE);
				formatsub(po, parent, pindex);
				break;
			}
			
			bool b = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).isInverse()) {
					if(!po.negative_exponents || !CHILD(i)[0].isNumber()) {
						b = true;
						break;
					}
				} else if(CHILD(i).isDivision()) {
					if(!CHILD(i)[0].isNumber() || !CHILD(i)[1].isNumber() || (!po.negative_exponents && CHILD(i)[0].number().isOne())) {
						b = true;
						break;
					}
				}
			}

			if(b) {
				MathStructure den;
				MathStructure num = m_undefined;
				short ds = 0, ns = 0;
				MathStructure *mnum = NULL, *mden = NULL;
				for(unsigned int i = 0; i < SIZE; i++) {
					if(CHILD(i).isInverse()) {
						mden = &CHILD(i)[0];
					} else if(CHILD(i).isDivision()) {
						mnum = &CHILD(i)[0];
						mden = &CHILD(i)[1];
					} else {
						mnum = &CHILD(i);
					}
					if(mnum) {
						if(ns > 0) {
							if(mnum->isMultiplication() && num.isNumber()) {
								for(unsigned int i2 = 0; i2 < mnum->size(); i2++) {
									num.multiply((*mnum)[i2], true);
								}
							} else {
								num.multiply(*mnum, ns > 1);
							}
						} else {
							num = *mnum;
						}						
						ns++;
						mnum = NULL;
					}
					if(mden) {
						if(ds > 0) {	
							if(mden->isMultiplication() && den.isNumber()) {
								for(unsigned int i2 = 0; i2 < mden->size(); i2++) {
									den.multiply((*mden)[i2], true);
								}
							} else {
								den.multiply(*mden, ds > 1);
							}							
						} else {
							den = *mden;
						}
						ds++;
						mden = NULL;
					}
				}
				clear();
				m_type = STRUCT_DIVISION;
				if(num.isUndefined()) num.set(1, 1);
				APPEND(MathStructure(num));
				APPEND(MathStructure(den));
				formatsub(po, parent, pindex);
				break;
			}

			unsigned int index = 0;
			if(CHILD(0).isOne()) {
				index = 1;
			}

			switch(CHILD(index).type()) {
				case STRUCT_POWER: {
					if(!CHILD(index)[0].isUnit_exp()) {
						break;
					}
				}
				case STRUCT_UNIT: {
					if(index == 0) {
						if(parent && (!parent->isDivision() || pindex != 2)) {
							PREPEND(MathStructure(1));
						}
					}
					break;
				}
				default: {
					if(index == 1) {
						ERASE(0);
						if(SIZE == 1) {
							MathStructure msave(CHILD(0));
							set(msave);
						}
					}
				}
			}

			break;
		}
		case STRUCT_UNIT: {
			if(!parent || (!parent->isPower() && !parent->isMultiplication() && !parent->isInverse() && !(parent->isDivision() && pindex == 2))) {				MathStructure msave(*this);
				clear();
				APPEND(MathStructure(1));
				APPEND(msave);
				m_type = STRUCT_MULTIPLICATION;
			}
			break;
		}
		case STRUCT_POWER: {
			if((!po.negative_exponents || CHILD(0).isAddition()) && CHILD(1).isNegate() && (!CHILD(0).isVector() || !CHILD(1).isMinusOne())) {
				if(CHILD(1)[0].isOne()) {
					m_type = STRUCT_INVERSE;
					ERASE(1);
				} else {
					MathStructure mmove(CHILD(1)[0]);
					CHILD(1) = mmove;
					transform(STRUCT_INVERSE);
				}
				formatsub(po, parent, pindex);
			} else if(po.halfexp_to_sqrt && ((CHILD(1).isDivision() && CHILD(1)[0].isNumber() && CHILD(1)[0].number().isInteger() && CHILD(1)[1].isNumber() && CHILD(1)[1].number().isTwo()) || (CHILD(1).isNumber() && CHILD(1).number().denominatorIsTwo()) || (CHILD(1).isInverse() && CHILD(1)[0].isNumber() && CHILD(1)[0].number() == 2))) {
				if(CHILD(1).isInverse() || (CHILD(1).isDivision() && CHILD(1)[0].number().isOne()) || (CHILD(1).isNumber() && CHILD(1).number().numeratorIsOne())) {
					m_type = STRUCT_FUNCTION;
					ERASE(1)
					o_function = CALCULATOR->f_sqrt;
				} else {
					if(CHILD(1).isNumber()) {
						CHILD(1).number() -= Number(1, 2);
					} else {
						Number nr = CHILD(1)[0].number();
						nr /= CHILD(1)[1].number();
						nr.floor();
						CHILD(1).set(nr);
					}
					if(CHILD(1).number().isOne()) {
						MathStructure msave(CHILD(0));
						set(msave);
						if(parent && parent->isMultiplication()) {
							parent->insertChild(MathStructure(CALCULATOR->f_sqrt, this, NULL), pindex + 1);
						} else {
							multiply(MathStructure(CALCULATOR->f_sqrt, this, NULL));
						}
					} else {
						if(parent && parent->isMultiplication()) {
							parent->insertChild(MathStructure(CALCULATOR->f_sqrt, &CHILD(0), NULL), pindex + 1);
						} else {
							multiply(MathStructure(CALCULATOR->f_sqrt, &CHILD(0), NULL));
						}
					}
				}
				formatsub(po, parent, pindex);
			} else if(CHILD(0).isUnit_exp() && (!parent || (!parent->isPower() && !parent->isMultiplication() && !parent->isInverse() && !(parent->isDivision() && pindex == 2)))) {
				MathStructure msave(*this);
				clear();
				APPEND(MathStructure(1));
				APPEND(msave);
				m_type = STRUCT_MULTIPLICATION;
			}
			break;
		}
		case STRUCT_NUMBER: {
			if(o_number.isNegative()) {
				o_number.negate();
				transform(STRUCT_NEGATE);
				formatsub(po, parent, pindex);
			} else if(po.number_fraction_format == FRACTION_COMBINED && po.base != BASE_SEXAGESIMAL && po.base != BASE_TIME && o_number.isRational() && !o_number.isInteger()) {
				if(o_number.isFraction()) {
					Number num(o_number.numerator());
					Number den(o_number.denominator());
					clear();
					if(num.isOne()) {
						m_type = STRUCT_INVERSE;
					} else {
						m_type = STRUCT_DIVISION;
						APPEND(MathStructure(num));
					}
					APPEND(MathStructure(den));
				} else {
					Number frac(o_number);
					frac.frac();
					MathStructure num(frac.numerator());
					num.transform(STRUCT_DIVISION, frac.denominator());
					o_number.trunc();
					add(num);
				}
			} else if((po.number_fraction_format == FRACTION_FRACTIONAL || po.base == BASE_ROMAN_NUMERALS) && po.base != BASE_SEXAGESIMAL && po.base != BASE_TIME && o_number.isRational() && !o_number.isInteger()) {
				Number num(o_number.numerator());
				Number den(o_number.denominator());
				clear();
				if(num.isOne()) {
					m_type = STRUCT_INVERSE;
				} else {
					m_type = STRUCT_DIVISION;
					APPEND(MathStructure(num));
				}
				APPEND(MathStructure(den));
			} else if(o_number.isComplex()) {
				if(o_number.hasRealPart()) {
					Number re(o_number.realPart());
					Number im(o_number.imaginaryPart());
					MathStructure mstruct(im);
					if(im.isOne()) {
						mstruct = CALCULATOR->v_i;
					} else {
						mstruct.multiply(CALCULATOR->v_i);
					}
					o_number = re;
					add(mstruct);
					formatsub(po, parent, pindex);
				} else {
					Number im(o_number.imaginaryPart());
					if(im.isOne()) {
						set(CALCULATOR->v_i);
					} else if(im.isMinusOne()) {
						clear();
						APPEND(MathStructure(CALCULATOR->v_i));
						m_type = STRUCT_NEGATE;
					} else {
						o_number = im;
						multiply(CALCULATOR->v_i);
					}
					formatsub(po, parent, pindex);
				}
			}
			break;
		}
	}
}

int namelen(const MathStructure &mstruct, const PrintOptions &po, const InternalPrintStruct &ips, bool *abbreviated = NULL) {
	const string *str;
	switch(mstruct.type()) {
		case STRUCT_FUNCTION: {
			const ExpressionName *ename = &mstruct.function()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names);
			str = &ename->name;
			if(abbreviated) *abbreviated = ename->abbreviation;
			break;
		}
		case STRUCT_VARIABLE:  {
			const ExpressionName *ename = &mstruct.variable()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names);
			str = &ename->name;
			if(abbreviated) *abbreviated = ename->abbreviation;
			break;
		}
		case STRUCT_SYMBOLIC:  {
			str = &mstruct.symbol();
			if(abbreviated) *abbreviated = false;
			break;
		}
		case STRUCT_UNIT:  {
			const ExpressionName *ename = &mstruct.unit()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, mstruct.isPlural(), po.use_reference_names);
			str = &ename->name;
			if(abbreviated) *abbreviated = ename->abbreviation;
			break;
		}
		default: {if(abbreviated) *abbreviated = false; return 0;}
	}
	if(text_length_is_one(*str)) return 1;
	return str->length();
}

bool MathStructure::needsParenthesis(const PrintOptions &po, const InternalPrintStruct &ips, const MathStructure &parent, unsigned int index, bool flat_division, bool flat_power) const {
	switch(parent.type()) {
		case STRUCT_MULTIPLICATION: {
			switch(m_type) {
				case STRUCT_MULTIPLICATION: {return true;}
				case STRUCT_DIVISION: {return flat_division;}
				case STRUCT_INVERSE: {return flat_division;}
				case STRUCT_ADDITION: {return true;}
				case STRUCT_POWER: {return po.excessive_parenthesis;}
				case STRUCT_NEGATE: {return po.excessive_parenthesis;}
				case STRUCT_AND: {return true;}
				case STRUCT_OR: {return true;}
				case STRUCT_XOR: {return true;}
				case STRUCT_COMPARISON: {return true;}
				case STRUCT_NOT: {return po.excessive_parenthesis;}
				case STRUCT_FUNCTION: {return false;}
				case STRUCT_VECTOR: {return false;}
				case STRUCT_NUMBER: {return false;}
				case STRUCT_VARIABLE: {return false;}
				case STRUCT_SYMBOLIC: {return false;}
				case STRUCT_UNIT: {return false;}
				case STRUCT_UNDEFINED: {return po.excessive_parenthesis;}
				default: {return true;}
			}
		}
		case STRUCT_INVERSE: {}
		case STRUCT_DIVISION: {
			switch(m_type) {
				case STRUCT_MULTIPLICATION: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_DIVISION: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_INVERSE: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_ADDITION: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_POWER: {return flat_division && po.excessive_parenthesis;}
				case STRUCT_NEGATE: {return flat_division && po.excessive_parenthesis;}
				case STRUCT_AND: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_OR: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_XOR: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_COMPARISON: {return flat_division || po.excessive_parenthesis;}
				case STRUCT_NOT: {return flat_division && po.excessive_parenthesis;}
				case STRUCT_FUNCTION: {return false;}
				case STRUCT_VECTOR: {return false;}
				case STRUCT_NUMBER: {return false;}
				case STRUCT_VARIABLE: {return false;}
				case STRUCT_SYMBOLIC: {return false;}
				case STRUCT_UNIT: {return false;}
				case STRUCT_UNDEFINED: {return false;}
				default: {return true;}
			}
		}
		case STRUCT_ADDITION: {
			switch(m_type) {
				case STRUCT_MULTIPLICATION: {return po.excessive_parenthesis;}
				case STRUCT_DIVISION: {return flat_division && po.excessive_parenthesis;}
				case STRUCT_INVERSE: {return flat_division && po.excessive_parenthesis;}
				case STRUCT_ADDITION: {return true;}
				case STRUCT_POWER: {return po.excessive_parenthesis;}
				case STRUCT_NEGATE: {return index > 1;}
				case STRUCT_AND: {return true;}
				case STRUCT_OR: {return true;}
				case STRUCT_XOR: {return true;}
				case STRUCT_COMPARISON: {return true;}
				case STRUCT_NOT: {return false;}
				case STRUCT_FUNCTION: {return false;}
				case STRUCT_VECTOR: {return false;}
				case STRUCT_NUMBER: {return false;}
				case STRUCT_VARIABLE: {return false;}
				case STRUCT_SYMBOLIC: {return false;}
				case STRUCT_UNIT: {return false;}
				case STRUCT_UNDEFINED: {return false;}
				default: {return true;}
			}
		}
		case STRUCT_POWER: {
			switch(m_type) {
				case STRUCT_MULTIPLICATION: {return true;}
				case STRUCT_DIVISION: {return index == 1 || flat_division || po.excessive_parenthesis;}
				case STRUCT_INVERSE: {return index == 1 || flat_division || po.excessive_parenthesis;}
				case STRUCT_ADDITION: {return true;}
				case STRUCT_POWER: {return true;}
				case STRUCT_NEGATE: {return index == 1 || po.excessive_parenthesis;}
				case STRUCT_AND: {return true;}
				case STRUCT_OR: {return true;}
				case STRUCT_XOR: {return true;}
				case STRUCT_COMPARISON: {return true;}
				case STRUCT_NOT: {return index == 1 || po.excessive_parenthesis;}
				case STRUCT_FUNCTION: {return false;}
				case STRUCT_VECTOR: {return false;}
				case STRUCT_NUMBER: {return false;}
				case STRUCT_VARIABLE: {return false;}
				case STRUCT_SYMBOLIC: {return false;}
				case STRUCT_UNIT: {return false;}
				case STRUCT_UNDEFINED: {return false;}
				default: {return true;}
			}
		}
		case STRUCT_NEGATE: {
			switch(m_type) {
				case STRUCT_MULTIPLICATION: {return false;}
				case STRUCT_DIVISION: {return po.excessive_parenthesis;}
				case STRUCT_INVERSE: {return flat_division && po.excessive_parenthesis;}
				case STRUCT_ADDITION: {return true;}
				case STRUCT_POWER: {return true;}
				case STRUCT_NEGATE: {return true;}
				case STRUCT_AND: {return true;}
				case STRUCT_OR: {return true;}
				case STRUCT_XOR: {return true;}
				case STRUCT_COMPARISON: {return true;}
				case STRUCT_NOT: {return po.excessive_parenthesis;}
				case STRUCT_FUNCTION: {return false;}
				case STRUCT_VECTOR: {return false;}
				case STRUCT_NUMBER: {return false;}
				case STRUCT_VARIABLE: {return false;}
				case STRUCT_SYMBOLIC: {return false;}
				case STRUCT_UNIT: {return false;}
				case STRUCT_UNDEFINED: {return false;}
				default: {return true;}
			}
		}
		case STRUCT_AND: {}
		case STRUCT_OR: {}
		case STRUCT_XOR: {}
		case STRUCT_COMPARISON: {
			switch(m_type) {
				case STRUCT_MULTIPLICATION: {return true;}
				case STRUCT_DIVISION: {return flat_division;}
				case STRUCT_INVERSE: {return flat_division;}
				case STRUCT_ADDITION: {return true;}
				case STRUCT_POWER: {return po.excessive_parenthesis;}
				case STRUCT_NEGATE: {return po.excessive_parenthesis;}
				case STRUCT_AND: {return true;}
				case STRUCT_OR: {return true;}
				case STRUCT_XOR: {return true;}
				case STRUCT_COMPARISON: {return true;}
				case STRUCT_NOT: {return false;}
				case STRUCT_FUNCTION: {return false;}
				case STRUCT_VECTOR: {return false;}
				case STRUCT_NUMBER: {return false;}
				case STRUCT_VARIABLE: {return false;}
				case STRUCT_SYMBOLIC: {return false;}
				case STRUCT_UNIT: {return false;}
				case STRUCT_UNDEFINED: {return false;}
				default: {return true;}
			}
		}
		case STRUCT_NOT: {
			switch(m_type) {
				case STRUCT_MULTIPLICATION: {return true;}
				case STRUCT_DIVISION: {return true;}
				case STRUCT_INVERSE: {return true;}
				case STRUCT_ADDITION: {return true;}
				case STRUCT_POWER: {return po.excessive_parenthesis;}
				case STRUCT_NEGATE: {return po.excessive_parenthesis;}
				case STRUCT_AND: {return true;}
				case STRUCT_OR: {return true;}
				case STRUCT_XOR: {return true;}
				case STRUCT_COMPARISON: {return true;}
				case STRUCT_NOT: {return true;}
				case STRUCT_FUNCTION: {return po.excessive_parenthesis;}
				case STRUCT_VECTOR: {return po.excessive_parenthesis;}
				case STRUCT_NUMBER: {return po.excessive_parenthesis;}
				case STRUCT_VARIABLE: {return po.excessive_parenthesis;}
				case STRUCT_SYMBOLIC: {return po.excessive_parenthesis;}
				case STRUCT_UNIT: {return po.excessive_parenthesis;}
				case STRUCT_UNDEFINED: {return po.excessive_parenthesis;}
				default: {return true;}
			}
		}
		case STRUCT_FUNCTION: {
			return false;
		}
		case STRUCT_VECTOR: {
			return false;
		}
		default: {
			return true;
		}
	}
}

int MathStructure::neededMultiplicationSign(const PrintOptions &po, const InternalPrintStruct &ips, const MathStructure &parent, unsigned int index, bool par, bool par_prev, bool flat_division, bool flat_power) const {
	if(!po.short_multiplication) return MULTIPLICATION_SIGN_OPERATOR;
	if(index <= 1) return MULTIPLICATION_SIGN_NONE;
	if(par_prev && par) return MULTIPLICATION_SIGN_NONE;
	if(par_prev) return MULTIPLICATION_SIGN_OPERATOR;
	if(par) return MULTIPLICATION_SIGN_NONE;
	int t = parent[index - 2].type();
	bool abbr_prev = false, abbr_this = false;
	int namelen_prev = namelen(parent[index - 2], po, ips, &abbr_prev);
	int namelen_this = namelen(*this, po, ips, &abbr_this);	
	switch(t) {
		case STRUCT_MULTIPLICATION: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_INVERSE: {}
		case STRUCT_DIVISION: {if(flat_division) return MULTIPLICATION_SIGN_OPERATOR; return MULTIPLICATION_SIGN_SPACE;}
		case STRUCT_ADDITION: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_POWER: {if(flat_power) return MULTIPLICATION_SIGN_OPERATOR; break;}
		case STRUCT_NEGATE: {break;}
		case STRUCT_AND: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_OR: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_XOR: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_COMPARISON: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_NOT: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_FUNCTION: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_VECTOR: {break;}
		case STRUCT_NUMBER: {break;}
		case STRUCT_VARIABLE: {break;}
		case STRUCT_SYMBOLIC: {
			break;
		}
		case STRUCT_UNIT: {
			if(m_type == STRUCT_UNIT) {
				if(!abbr_prev && !abbr_this) {
					return MULTIPLICATION_SIGN_NONE;
				} 
				if(po.place_units_separately) {
					return MULTIPLICATION_SIGN_OPERATOR_SHORT;
				} else {
					return MULTIPLICATION_SIGN_OPERATOR;
				}
			} else if(m_type == STRUCT_NUMBER) {
				if(namelen_prev > 1) {
					return MULTIPLICATION_SIGN_SPACE;
				} else {
					return MULTIPLICATION_SIGN_NONE;
				}
			}
			//return MULTIPLICATION_SIGN_SPACE;
		}
		case STRUCT_UNDEFINED: {break;}
		default: {return MULTIPLICATION_SIGN_OPERATOR;}
	}
	switch(m_type) {
		case STRUCT_MULTIPLICATION: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_INVERSE: {}
		case STRUCT_DIVISION: {if(flat_division) return MULTIPLICATION_SIGN_OPERATOR; return MULTIPLICATION_SIGN_SPACE;}
		case STRUCT_ADDITION: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_POWER: {return CHILD(0).neededMultiplicationSign(po, ips, parent, index, par, par_prev, flat_division, flat_power);}
		case STRUCT_NEGATE: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_AND: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_OR: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_XOR: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_COMPARISON: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_NOT: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_FUNCTION: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_VECTOR: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_NUMBER: {return MULTIPLICATION_SIGN_OPERATOR;}
		case STRUCT_VARIABLE: {}
		case STRUCT_SYMBOLIC: {
			if(t != STRUCT_NUMBER && ((namelen_prev > 1 || namelen_this > 1) || equals(parent[index - 2]))) return MULTIPLICATION_SIGN_OPERATOR;
			if(namelen_this > 1 || (m_type == STRUCT_SYMBOLIC && !po.allow_non_usable)) return MULTIPLICATION_SIGN_SPACE;
			return MULTIPLICATION_SIGN_NONE;
		}
		case STRUCT_UNIT: {
			if(t == STRUCT_POWER && parent[index - 2][0].isUnit_exp()) {
				return MULTIPLICATION_SIGN_NONE;
			}			
			return MULTIPLICATION_SIGN_SPACE;
		}
		case STRUCT_UNDEFINED: {return MULTIPLICATION_SIGN_OPERATOR;}
		default: {return MULTIPLICATION_SIGN_OPERATOR;}
	}
}

string MathStructure::print(const PrintOptions &po, const InternalPrintStruct &ips) const {
	if(ips.depth == 0 && po.is_approximate) *po.is_approximate = false;
	string print_str;
	InternalPrintStruct ips_n = ips;
	if(isApproximate()) ips_n.parent_approximate = true;
	if(precision() > 0 && (ips_n.parent_precision < 1 || precision() < ips_n.parent_precision)) ips_n.parent_precision = precision();
	switch(m_type) {
		case STRUCT_NUMBER: {
			print_str = o_number.print(po, ips_n);
			break;
		}
		case STRUCT_SYMBOLIC: {
			if(po.allow_non_usable) {
				print_str = s_sym;
			} else {
				print_str = "\"";
				print_str += s_sym;
				print_str += "\"";
			}
			break;
		}
#ifdef HAVE_GIAC		
		case STRUCT_UNKNOWN: {
			print_str = giac_unknown->print();
			break;
		}
#endif
		case STRUCT_ADDITION: {
			ips_n.depth++;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(i > 0) {
					if(CHILD(i).type() == STRUCT_NEGATE) {
						if(po.spacious) print_str += " ";
						if(po.use_unicode_signs) print_str += SIGN_MINUS;
						else print_str += "-";
						if(po.spacious) print_str += " ";
						ips_n.wrap = CHILD(i)[0].needsParenthesis(po, ips_n, *this, i + 1, true, true);
						print_str += CHILD(i)[0].print(po, ips_n);
					} else {
						if(po.spacious) print_str += " ";
						print_str += "+";
						if(po.spacious) print_str += " ";
						ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
						print_str += CHILD(i).print(po, ips_n);
					}
				} else {
					ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
					print_str += CHILD(i).print(po, ips_n);
				}
			}
			break;
		}
		case STRUCT_NEGATE: {
			if(po.use_unicode_signs) print_str += SIGN_MINUS;
			else print_str = "-";
			ips_n.depth++;
			ips_n.wrap = CHILD(0).needsParenthesis(po, ips_n, *this, 1, true, true);
			print_str += CHILD(0).print(po, ips_n);
			break;
		}
		case STRUCT_MULTIPLICATION: {
			ips_n.depth++;
			bool par_prev = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
				if(!po.short_multiplication && i > 0) {
					if(po.spacious) print_str += " ";
					if(po.use_unicode_signs) print_str += SIGN_MULTIDOT;
					else print_str += "*";
					if(po.spacious) print_str += " ";
				} else if(i > 0) {
					switch(CHILD(i).neededMultiplicationSign(po, ips_n, *this, i + 1, ips_n.wrap, par_prev, true, true)) {
						case MULTIPLICATION_SIGN_SPACE: {print_str += " "; break;}
						case MULTIPLICATION_SIGN_OPERATOR: {
							if(po.spacious) {
								if(po.use_unicode_signs) print_str += " " SIGN_MULTIDOT " ";
								else print_str += " * "; 
								break;
							}
						}
						case MULTIPLICATION_SIGN_OPERATOR_SHORT: {
							if(po.use_unicode_signs) print_str += SIGN_MULTIDOT;
							else print_str += "*"; 
							break;
						}
					}
				}
				print_str += CHILD(i).print(po, ips_n);
				par_prev = ips_n.wrap;
			}
			break;
		}
		case STRUCT_INVERSE: {
			print_str = "1";
			if(po.spacious) print_str += " ";
			if(po.use_unicode_signs) print_str += SIGN_DIVISION;
			else print_str += "/";
			if(po.spacious) print_str += " ";
			ips_n.depth++;
			ips_n.division_depth++;
			ips_n.wrap = CHILD(0).needsParenthesis(po, ips_n, *this, 1, true, true);
			print_str += CHILD(0).print(po, ips_n);
			break;
		}
		case STRUCT_DIVISION: {
			ips_n.depth++;
			ips_n.division_depth++;
			ips_n.wrap = CHILD(0).needsParenthesis(po, ips_n, *this, 1, true, true);
			print_str = CHILD(0).print(po, ips_n);
			if(po.spacious) print_str += " ";
			if(po.use_unicode_signs) print_str += SIGN_DIVISION;
			else print_str += "/";
			if(po.spacious) print_str += " ";
			ips_n.wrap = CHILD(1).needsParenthesis(po, ips_n, *this, 2, true, true);
			print_str += CHILD(1).print(po, ips_n);
			break;
		}
		case STRUCT_POWER: {
			ips_n.depth++;
			ips_n.power_depth++;
			ips_n.wrap = CHILD(0).needsParenthesis(po, ips_n, *this, 1, true, true);
			print_str = CHILD(0).print(po, ips_n);
			print_str += "^";
			ips_n.wrap = CHILD(1).needsParenthesis(po, ips_n, *this, 2, true, true);
			PrintOptions po2 = po;
			po2.show_ending_zeroes = false;
			print_str += CHILD(1).print(po2, ips_n);
			break;
		}
		case STRUCT_COMPARISON: {
			ips_n.depth++;
			ips_n.wrap = CHILD(0).needsParenthesis(po, ips_n, *this, 1, true, true);
			print_str = CHILD(0).print(po, ips_n);
			if(po.spacious) print_str += " ";
			switch(ct_comp) {
				case COMPARISON_EQUALS: {print_str += "="; break;}
				case COMPARISON_NOT_EQUALS: {
					if(po.use_unicode_signs) print_str += SIGN_NOT_EQUAL;
					else print_str += "!="; 
					break;
				}
				case COMPARISON_GREATER: {print_str += ">"; break;}
				case COMPARISON_LESS: {print_str += "<"; break;}
				case COMPARISON_EQUALS_GREATER: {
					if(po.use_unicode_signs) print_str += SIGN_GREATER_OR_EQUAL;
					else print_str += ">="; 
					break;
				}
				case COMPARISON_EQUALS_LESS: {
					if(po.use_unicode_signs) print_str += SIGN_LESS_OR_EQUAL;
					else print_str += "<="; 
					break;
				}
			}
			if(po.spacious) print_str += " ";
			ips_n.wrap = CHILD(1).needsParenthesis(po, ips_n, *this, 2, true, true);
			print_str += CHILD(1).print(po, ips_n);
			break;
		}
		case STRUCT_AND: {
			ips_n.depth++;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(i > 0) {
					if(po.spacious) print_str += " ";
					print_str += "&&";
					if(po.spacious) print_str += " ";
				}
				ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
				print_str += CHILD(i).print(po, ips_n);
			}
			break;
		}
		case STRUCT_OR: {
			ips_n.depth++;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(i > 0) {
					if(po.spacious) print_str += " ";
					print_str += "||";
					if(po.spacious) print_str += " ";
				}
				ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
				print_str += CHILD(i).print(po, ips_n);
			}
			break;
		}
		case STRUCT_XOR: {
			ips_n.depth++;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(i > 0) {
					if(po.spacious) print_str += " ";
					print_str += "XOR";
					if(po.spacious) print_str += " ";
				}
				ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
				print_str += CHILD(i).print(po, ips_n);
			}
			break;
		}
		case STRUCT_NOT: {
			print_str = "!";
			ips_n.depth++;
			ips_n.wrap = CHILD(0).needsParenthesis(po, ips_n, *this, 1, true, true);
			print_str += CHILD(0).print(po, ips_n);
			break;
		}
		case STRUCT_VECTOR: {
			ips_n.depth++;
			print_str = "[";
			for(unsigned int i = 0; i < SIZE; i++) {
				if(i > 0) {
					print_str += po.comma();
					if(po.spacious) print_str += " ";
				}
				ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
				print_str += CHILD(i).print(po, ips_n);
			}
			print_str += "]";
			break;
		}
		case STRUCT_UNIT: {
			const ExpressionName *ename = &o_unit->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, b_plural, po.use_reference_names);
			if(o_prefix) print_str += o_prefix->name(po.abbreviate_names && ename->abbreviation, po.use_unicode_signs);
			print_str += ename->name;
			break;
		}
		case STRUCT_VARIABLE: {
			print_str = o_variable->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name;
			break;
		}
		case STRUCT_FUNCTION: {
			ips_n.depth++;
			print_str += o_function->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names).name;
			print_str += "(";
			for(unsigned int i = 0; i < SIZE; i++) {
				if(i > 0) {
					print_str += po.comma();
					if(po.spacious) print_str += " ";
				}
				ips_n.wrap = CHILD(i).needsParenthesis(po, ips_n, *this, i + 1, true, true);
				print_str += CHILD(i).print(po, ips_n);
			}
			print_str += ")";
			break;
		}
		case STRUCT_UNDEFINED: {
			print_str = _("undefined");
			break;
		}
	}
	if(ips.wrap) {
		print_str.insert(0, "(");
		print_str += ")";
	}
	return print_str;
}

MathStructure &MathStructure::flattenVector(MathStructure &mstruct) const {
	if(!isVector()) {
		mstruct = *this;
		return mstruct;
	}
	MathStructure mstruct2;
	mstruct.clearVector();
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).isVector()) {
			CHILD(i).flattenVector(mstruct2);
			for(unsigned int i2 = 0; i2 < mstruct2.size(); i2++) {
				mstruct.addItem(mstruct2[i2]);
			}
		} else {
			mstruct.addItem(CHILD(i));
		}
	}
	return mstruct;
}
bool MathStructure::rankVector(bool ascending) {
	vector<int> ranked;
	vector<bool> ranked_equals_prev;	
	bool b;
	for(unsigned int index = 0; index < SIZE; index++) {
		b = false;
		for(unsigned int i = 0; i < ranked.size(); i++) {
			ComparisonResult cmp = CHILD(index).compare(CHILD(ranked[i]));
			if(COMPARISON_NOT_FULLY_KNOWN(cmp)) {
				CALCULATOR->error(true, _("Unsolvable comparison at component %s when trying to rank vector."), i2s(index).c_str(), NULL);
				return false;
			}
			if((ascending && cmp == COMPARISON_RESULT_GREATER) || cmp == COMPARISON_RESULT_EQUAL || (!ascending && cmp == COMPARISON_RESULT_LESS)) {
				if(cmp == COMPARISON_RESULT_EQUAL) {
					ranked.insert(ranked.begin() + i + 1, index);
					ranked_equals_prev.insert(ranked_equals_prev.begin() + i + 1, true);
				} else {
					ranked.insert(ranked.begin() + i, index);
					ranked_equals_prev.insert(ranked_equals_prev.begin() + i, false);
				}
				b = true;
				break;
			}
		}
		if(!b) {
			ranked.push_back(index);
			ranked_equals_prev.push_back(false);
		}
	}	
	int n_rep = 0;
	for(int i = (int) ranked.size() - 1; i >= 0; i--) {
		if(ranked_equals_prev[i]) {
			n_rep++;
		} else {
			if(n_rep) {
				MathStructure v(i + 1 + n_rep, 1);
				v += i + 1;
				v *= MathStructure(1, 2);
				for(; n_rep >= 0; n_rep--) {
					CHILD(ranked[i + n_rep]) = v;
				}
			} else {
				CHILD(ranked[i]).set(i + 1, 1);
			}
			n_rep = 0;
		}
	}
	return true;
}
bool MathStructure::sortVector(bool ascending) {
	vector<unsigned int> ranked_mstructs;
	bool b;
	for(unsigned int index = 0; index < SIZE; index++) {
		b = false;
		for(unsigned int i = 0; i < ranked_mstructs.size(); i++) {
			ComparisonResult cmp = CHILD(index).compare(v_subs[ranked_mstructs[i]]);
			if(COMPARISON_MIGHT_BE_LESS_OR_GREATER(cmp)) {
				CALCULATOR->error(true, _("Unsolvable comparison at component %s when trying to sort vector."), i2s(index).c_str(), NULL);
				return false;
			}
			if((ascending && COMPARISON_IS_EQUAL_OR_GREATER(cmp)) || (!ascending && COMPARISON_IS_EQUAL_OR_LESS(cmp))) {
				ranked_mstructs.insert(ranked_mstructs.begin() + i, v_order[index]);
				b = true;
				break;
			}
		}
		if(!b) {
			ranked_mstructs.push_back(v_order[index]);
		}
	}	
	v_order = ranked_mstructs;
	return true;
}
MathStructure &MathStructure::getRange(int start, int end, MathStructure &mstruct) const {
	if(!isVector()) {
		if(start > 1) {
			mstruct.clear();
			return mstruct;
		} else {
			mstruct = *this;
			return mstruct;
		}
	}
	if(start < 1) start = 1;
	else if(start > (int) SIZE) {
		mstruct.clear();
		return mstruct;
	}
	if(end < (int) 1 || end > (int) SIZE) end = SIZE;
	else if(end < start) end = start;	
	mstruct.clearVector();
	for(; start <= end; start++) {
		mstruct.addItem(CHILD(start - 1));
	}
	return mstruct;
}

void MathStructure::resizeVector(unsigned int i, const MathStructure &mfill) {
	if(i > SIZE) {
		while(i > SIZE) {
			APPEND(mfill);
		}
	} else if(i > SIZE) {
		REDUCE(i)
	}
}

unsigned int MathStructure::rows() const {
	if(m_type != STRUCT_VECTOR || SIZE == 0 || (SIZE == 1 && (!CHILD(0).isVector() || CHILD(0).size() == 0))) return 0;
	return SIZE;
}
unsigned int MathStructure::columns() const {
	if(m_type != STRUCT_VECTOR || SIZE == 0 || !CHILD(0).isVector()) return 0;
	return CHILD(0).size();
}
const MathStructure *MathStructure::getElement(unsigned int row, unsigned int column) const {
	if(row == 0 || column == 0 || row > rows() || column > columns()) return NULL;
	if(CHILD(row - 1).size() < column) return NULL;
	return &CHILD(row - 1)[column - 1];
}
MathStructure &MathStructure::getArea(unsigned int r1, unsigned int c1, unsigned int r2, unsigned int c2, MathStructure &mstruct) const {
	unsigned int r = rows();
	unsigned int c = columns();
	if(r1 < 1) r1 = 1;
	else if(r1 > r) r1 = r;
	if(c1 < 1) c1 = 1;
	else if(c1 > c) c1 = c;
	if(r2 < 1 || r2 > r) r2 = r;
	else if(r2 < r1) r2 = r1;
	if(c2 < 1 || c2 > c) c2 = c;
	else if(c2 < c1) c2 = c1;
	mstruct.clearMatrix(); mstruct.resizeMatrix(r2 - r1 + 1, c2 - c1 + 1, m_undefined);
	for(unsigned int index_r = r1; index_r <= r2; index_r++) {
		for(unsigned int index_c = c1; index_c <= c2; index_c++) {
			mstruct[index_r - r1][index_c - c1] = CHILD(index_r - 1)[index_c - 1];
		}			
	}
	return mstruct;
}
MathStructure &MathStructure::rowToVector(unsigned int r, MathStructure &mstruct) const {
	if(r > rows()) return m_undefined;
	if(r < 1) r = 1;
	mstruct = CHILD(r - 1);
	return mstruct;
}
MathStructure &MathStructure::columnToVector(unsigned int c, MathStructure &mstruct) const {
	if(c > columns()) {
		mstruct = m_undefined;
		return mstruct;
	}
	if(c < 1) c = 1;
	mstruct.clearVector();
	for(unsigned int i = 0; i < SIZE; i++) {
		mstruct.addItem(CHILD(i)[c - 1]);
	}
	return mstruct;
}
MathStructure &MathStructure::matrixToVector(MathStructure &mstruct) const {
	if(!isVector()) {
		mstruct = *this;
		return mstruct;
	}
	mstruct.clearVector();
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).isVector()) {
			for(unsigned int i2 = 0; i2 < CHILD(i).size(); i2++) {
				mstruct.addItem(CHILD(i)[i2]);
			}
		} else {
			mstruct.addItem(CHILD(i));
		}
	}
	return mstruct;
}
void MathStructure::setElement(const MathStructure &mstruct, unsigned int row, unsigned int column) {
	if(row > rows() || column > columns() || row < 1 || column < 1) return;
	CHILD(row - 1)[column - 1] = mstruct;
}
void MathStructure::addRows(unsigned int r, const MathStructure &mfill) {
	if(r == 0) return;
	unsigned int cols = columns();
	MathStructure mstruct; mstruct.clearVector();
	mstruct.resizeVector(cols, mfill);
	for(unsigned int i = 0; i < r; i++) {
		APPEND(mstruct);
	}
}
void MathStructure::addColumns(unsigned int c, const MathStructure &mfill) {
	if(c == 0) return;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).isVector()) {
			for(unsigned int i2 = 0; i2 < c; i2++) {
				CHILD(i).addItem(mfill);
			}
		}
	}
}
void MathStructure::addRow(const MathStructure &mfill) {
	addRows(1, mfill);
}
void MathStructure::addColumn(const MathStructure &mfill) {
	addColumns(1, mfill);
}
void MathStructure::resizeMatrix(unsigned int r, unsigned int c, const MathStructure &mfill) {
	if(r > SIZE) {
		addRows(r - SIZE, mfill);
	} else if(r != SIZE) {
		REDUCE(r);
	}
	unsigned int cols = columns();
	if(c > cols) {
		addColumns(c - cols, mfill);
	} else if(c != cols) {
		for(unsigned i = 0; i < SIZE; i++) {
			CHILD(i).resizeVector(c, mfill);
		}
	}
}
bool MathStructure::matrixIsSymmetric() const {
	return rows() == columns();
}
MathStructure &MathStructure::determinant(MathStructure &mstruct, const EvaluationOptions &eo) const {
	if(!matrixIsSymmetric()) {
		CALCULATOR->error(true, _("The determinant can only be calculated for symmetric matrices."), NULL);
		mstruct = m_undefined;
		return mstruct;
	}
	EvaluationOptions eo2 = eo;
	eo2.calculate_functions = false;
	eo2.sync_units = false;
	if(SIZE == 1) {
		if(CHILD(0).size() >= 1) {	
			mstruct = CHILD(0)[0];
		}
		mstruct.eval(eo2);
	} else if(SIZE == 2) {
		mstruct = CHILD(0)[0];
		mstruct.multiply(CHILD(1)[1], true);
		mstruct.eval(eo2);
		MathStructure mtmp = CHILD(1)[0];
		mtmp.multiply(CHILD(0)[1], true);
		mtmp.eval(eo2);
		mstruct.subtract(mtmp, true);
		mstruct.eval(eo2);
	} else {
		MathStructure mtrx;
		mtrx.clearMatrix();
		mtrx.resizeMatrix(SIZE - 1, CHILD(0).size() - 1, m_undefined);
		for(unsigned int index_c = 0; index_c < CHILD(0).size(); index_c++) {
			for(unsigned int index_r2 = 1; index_r2 < SIZE; index_r2++) {
				for(unsigned int index_c2 = 0; index_c2 < CHILD(index_r2).size(); index_c2++) {
					if(index_c2 > index_c) {
						mtrx.setElement(CHILD(index_r2)[index_c2], index_r2, index_c2);
					} else if(index_c2 < index_c) {
						mtrx.setElement(CHILD(index_r2)[index_c2], index_r2, index_c2 + 1);
					}
				}
			}
			
			MathStructure mdet;
			mtrx.determinant(mdet, eo);
			
			if(index_c % 2 == 1) {
				mdet.negate();
			}
			
			mdet.multiply(CHILD(0)[index_c], true);
			mdet.eval(eo2);

			mstruct.add(mdet, true);
			mstruct.eval(eo2);

		}
	}
	return mstruct;
}
MathStructure &MathStructure::permanent(MathStructure &mstruct, const EvaluationOptions &eo) const {
	if(!matrixIsSymmetric()) {
		CALCULATOR->error(true, _("The permanent can only be calculated for symmetric matrices."), NULL);
		mstruct = m_undefined;
		return mstruct;
	}
	EvaluationOptions eo2 = eo;
	eo2.calculate_functions = false;
	eo2.sync_units = false;
	if(SIZE == 1) {
		if(CHILD(0).size() >= 1) {	
			mstruct == CHILD(0)[0];
		}
		mstruct.eval(eo2);
	} else if(SIZE == 2) {
		mstruct = CHILD(0)[0];
		mstruct *= CHILD(1)[1];
		MathStructure mtmp = CHILD(1)[0];
		mtmp *= CHILD(0)[1];
		mstruct += mtmp;
		mstruct.eval(eo2);
	} else {
		MathStructure mtrx;
		mtrx.clearMatrix();
		mtrx.resizeMatrix(SIZE - 1, CHILD(0).size() - 1, m_undefined);
		for(unsigned int index_c = 0; index_c < CHILD(0).size(); index_c++) {
			for(unsigned int index_r2 = 1; index_r2 < SIZE; index_r2++) {
				for(unsigned int index_c2 = 0; index_c2 < CHILD(index_r2).size(); index_c2++) {
					if(index_c2 > index_c) {
						mtrx.setElement(CHILD(index_r2)[index_c2], index_r2, index_c2);
					} else if(index_c2 < index_c) {
						mtrx.setElement(CHILD(index_r2)[index_c2], index_r2, index_c2 + 1);
					}
				}
			}
			MathStructure mdet;
			mtrx.permanent(mdet, eo);
			
			mdet *= CHILD(0)[index_c];
			mstruct += mdet;
		}
		mstruct.eval(eo2);
	}
	return mstruct;
}
void MathStructure::setToIdentityMatrix(unsigned int n) {
	clearMatrix();
	resizeMatrix(n, n, m_zero);
	for(unsigned int i = 0; i < n; i++) {
		CHILD(i)[i] = m_one;
	}
}
MathStructure &MathStructure::getIdentityMatrix(MathStructure &mstruct) const {
	mstruct.setToIdentityMatrix(columns());
	return mstruct;
}
bool MathStructure::invertMatrix(const EvaluationOptions &eo) {
	if(!matrixIsSymmetric()) return false;
	MathStructure mstruct;
	determinant(mstruct, eo);
	mstruct.raise(m_minus_one);
	adjointMatrix(eo);
	multiply(mstruct);
	eval(eo);
	return true;
}
bool MathStructure::adjointMatrix(const EvaluationOptions &eo) {
	if(!matrixIsSymmetric()) return false;
	MathStructure msave(*this);
	for(unsigned int index_r = 0; index_r < SIZE; index_r++) {
		for(unsigned int index_c = 0; index_c < CHILD(0).size(); index_c++) {
			msave.cofactor(index_r + 1, index_c + 1, CHILD(index_r)[index_c], eo);
		}
	}
	transposeMatrix();
	eval(eo);
	return true;
}
bool MathStructure::transposeMatrix() {
	MathStructure msave(*this);
	resizeMatrix(CHILD(0).size(), SIZE, m_undefined);
	for(unsigned int index_r = 0; index_r < SIZE; index_r++) {
		for(unsigned int index_c = 0; index_c < CHILD(0).size(); index_c++) {
			CHILD(index_r)[index_c] = msave[index_c][index_r];
		}
	}
	return true;
}	
MathStructure &MathStructure::cofactor(unsigned int r, unsigned int c, MathStructure &mstruct, const EvaluationOptions &eo) const {
	if(r < 1) r = 1;
	if(c < 1) c = 1;
	if(r > SIZE || c > CHILD(0).size()) {
		mstruct = m_undefined;
		return mstruct;
	}
	r--; c--;
	mstruct.clearMatrix(); mstruct.resizeMatrix(SIZE - 1, CHILD(0).size() - 1, m_undefined);
	for(unsigned int index_r = 0; index_r < SIZE; index_r++) {
		if(index_r != r) {
			for(unsigned int index_c = 0; index_c < CHILD(0).size(); index_c++) {
				if(index_c > c) {
					if(index_r > r) {
						mstruct[index_r - 1][index_c - 1] = CHILD(index_r)[index_c];
					} else {
						mstruct[index_r][index_c - 1] = CHILD(index_r)[index_c];
					}
				} else if(index_c < c) {
					if(index_r > r) {
						mstruct[index_r - 1][index_c] = CHILD(index_r)[index_c];
					} else {
						mstruct[index_r][index_c] = CHILD(index_r)[index_c];
					}
				}
			}
		}
	}	
	MathStructure mstruct2;
	mstruct = mstruct.determinant(mstruct2, eo);
	if((r + c) % 2 == 1) {
		mstruct.negate();
	}
	mstruct.eval(eo);
	return mstruct;
}

void gatherInformation(const MathStructure &mstruct, vector<Unit*> &base_units, vector<AliasUnit*> &alias_units) {
	switch(mstruct.type()) {
		case STRUCT_UNIT: {
			switch(mstruct.unit()->subtype()) {
				case SUBTYPE_BASE_UNIT: {
					for(unsigned int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == mstruct.unit()) {
							return;
						}
					}
					base_units.push_back(mstruct.unit());
					break;
				}
				case SUBTYPE_ALIAS_UNIT: {
					for(unsigned int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == mstruct.unit()) {
							return;
						}
					}
					alias_units.push_back((AliasUnit*) (mstruct.unit()));
					break;
				}
				case SUBTYPE_COMPOSITE_UNIT: {
					gatherInformation(((CompositeUnit*) (mstruct.unit()))->generateMathStructure(), base_units, alias_units);
					break;
				}				
			}
			break;
		}
		default: {
			for(unsigned int i = 0; i < mstruct.size(); i++) {
				gatherInformation(mstruct[i], base_units, alias_units);
			}
			break;
		}
	}

}

bool MathStructure::isUnitCompatible(const MathStructure &mstruct) {
	bool b1 = mstruct.containsType(STRUCT_UNIT);
	bool b2 = containsType(STRUCT_UNIT);
	if(b1 != b2) return false;
	if(!b1) return true;
	if(mstruct.isMultiplication()) {
		unsigned int i2 = 0;
		for(unsigned int i = 0; i < SIZE; i++) {
			if(CHILD(i).containsType(STRUCT_UNIT)) {
				bool b = false;
				for(; i2 < mstruct.size(); i2++) {
					if(mstruct[i2].containsType(STRUCT_UNIT)) {
						if(!CHILD(i).isUnitCompatible(mstruct[i2])) {
							return false;
						}
						i2++;
						b = true;
						break;
					}
				}
				if(!b) return false;
			}
		}
		for(; i2 < mstruct.size(); i2++) {
			if(mstruct[i2].containsType(STRUCT_UNIT)) {
				return false;
			}	
		}
	}
	if(isUnit()) {
		return equals(mstruct);
	} else if(isPower()) {
		return equals(mstruct);
	}
	return true;
}

bool MathStructure::syncUnits(bool sync_complex_relations) {
	vector<Unit*> base_units;
	vector<AliasUnit*> alias_units;
	vector<CompositeUnit*> composite_units;	
	gatherInformation(*this, base_units, alias_units);
	CompositeUnit *cu;
	bool b = false;
	for(int i = 0; i < (int) alias_units.size(); i++) {
		if(alias_units[i]->baseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
			b = false;
			cu = (CompositeUnit*) alias_units[i]->baseUnit();
			for(unsigned int i2 = 0; i2 < base_units.size(); i2++) {
				if(cu->containsRelativeTo(base_units[i2])) {
					for(unsigned int i = 0; i < composite_units.size(); i++) {
						if(composite_units[i] == cu) {
							b = true;
							break;
						}
					}
					if(!b) composite_units.push_back(cu);					
					goto erase_alias_unit_1;
				}
			}
			for(unsigned int i2 = 0; i2 < alias_units.size(); i2++) {
				if(cu->containsRelativeTo(alias_units[i2])) {
					for(unsigned int i = 0; i < composite_units.size(); i++) {
						if(composite_units[i] == cu) {
							b = true;
							break;
						}
					}
					if(!b) composite_units.push_back(cu);				
					goto erase_alias_unit_1;
				}
			}					
		}
		goto dont_erase_alias_unit_1;
		erase_alias_unit_1:
		alias_units.erase(alias_units.begin() + i);
		for(int i2 = 0; i2 < (int) cu->units.size(); i2++) {
			b = false;
			switch(cu->units[i2]->firstBaseUnit()->subtype()) {
				case SUBTYPE_BASE_UNIT: {
					for(unsigned int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) base_units.push_back((Unit*) cu->units[i2]->firstBaseUnit());
					break;
				}
				case SUBTYPE_ALIAS_UNIT: {
					for(unsigned int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) alias_units.push_back((AliasUnit*) cu->units[i2]->firstBaseUnit());				
					break;
				}
				case SUBTYPE_COMPOSITE_UNIT: {
					gatherInformation(((CompositeUnit*) cu->units[i2]->firstBaseUnit())->generateMathStructure(), base_units, alias_units);
					break;
				}
			}
		}
		i = -1;
		dont_erase_alias_unit_1:
		;
	}
	for(int i = 0; i < (int) alias_units.size(); i++) {
		for(int i2 = 0; i2 < (int) alias_units.size(); i2++) {
			if(i != i2 && alias_units[i]->baseUnit() == alias_units[i2]->baseUnit()) { 
				if(alias_units[i2]->isParentOf(alias_units[i])) {
					goto erase_alias_unit_2;
				}
				if(!alias_units[i]->isParentOf(alias_units[i2])) {
					b = false;
					for(unsigned int i3 = 0; i3 < base_units.size(); i3++) {
						if(base_units[i3] == alias_units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) base_units.push_back((Unit*) alias_units[i]->baseUnit());
					goto erase_alias_unit_2;
				}
			}
		} 
		goto dont_erase_alias_unit_2;
		erase_alias_unit_2:
		alias_units.erase(alias_units.begin() + i);
		i--;
		dont_erase_alias_unit_2:
		;
	}	
	for(int i = 0; i < (int) alias_units.size(); i++) {
		if(alias_units[i]->baseUnit()->subtype() == SUBTYPE_BASE_UNIT) {
			for(unsigned int i2 = 0; i2 < base_units.size(); i2++) {
				if(alias_units[i]->baseUnit() == base_units[i2]) {
					goto erase_alias_unit_3;
				}
			}
		} 
		goto dont_erase_alias_unit_3;
		erase_alias_unit_3:
		alias_units.erase(alias_units.begin() + i);
		i--;
		dont_erase_alias_unit_3:
		;
	}
	b = false;
	for(unsigned int i = 0; i < composite_units.size(); i++) {	
		if(convert(composite_units[i], sync_complex_relations)) b = true;
	}	
	if(dissolveAllCompositeUnits()) b = true;
	for(unsigned int i = 0; i < base_units.size(); i++) {	
		if(convert(base_units[i], sync_complex_relations)) b = true;
	}
	for(unsigned int i = 0; i < alias_units.size(); i++) {	
		if(convert(alias_units[i], sync_complex_relations)) b = true;
	}
	return b;
}
bool MathStructure::testDissolveCompositeUnit(Unit *u) {
	if(m_type == STRUCT_UNIT) {
		if(o_unit->subtype() == SUBTYPE_COMPOSITE_UNIT) {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				set(((CompositeUnit*) o_unit)->generateMathStructure());
				return true;
			}
		} else if(o_unit->subtype() == SUBTYPE_ALIAS_UNIT && o_unit->baseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
			if(((CompositeUnit*) (o_unit->baseUnit()))->containsRelativeTo(u)) {
				convert(o_unit->baseUnit());
				convert(u);
				return true;
			}		
		}
	}
	return false; 
}
bool MathStructure::testCompositeUnit(Unit *u) {
	if(m_type == STRUCT_UNIT) {
		if(o_unit->subtype() == SUBTYPE_COMPOSITE_UNIT) {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				return true;
			}
		} else if(o_unit->subtype() == SUBTYPE_ALIAS_UNIT && o_unit->baseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
			if(((CompositeUnit*) (o_unit->baseUnit()))->containsRelativeTo(u)) {
				return true;
			}		
		}
	}
	return false; 
}
bool MathStructure::dissolveAllCompositeUnits() {
	switch(m_type) {
		case STRUCT_UNIT: {
			if(o_unit->subtype() == SUBTYPE_COMPOSITE_UNIT) {
				set(((CompositeUnit*) o_unit)->generateMathStructure());
				return true;
			}
			break;
		}
		default: {
			bool b = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).dissolveAllCompositeUnits()) b = true;
			}
			return b;
		}		
	}		
	return false;
}
bool MathStructure::convert(Unit *u, bool convert_complex_relations) {
	bool b = false;	
	if(m_type == STRUCT_UNIT && o_unit == u) return false;
	if(u->subtype() == SUBTYPE_COMPOSITE_UNIT && !(m_type == STRUCT_UNIT && o_unit->baseUnit() == u)) {
		return convert(((CompositeUnit*) u)->generateMathStructure());
	}
	if(m_type == STRUCT_UNIT) {
		if(!convert_complex_relations && u->hasComplexRelationTo(o_unit)) return false;
		if(testDissolveCompositeUnit(u)) {
			convert(u, convert_complex_relations);
			return true;
		}
		MathStructure exp(1, 1);
		MathStructure mstruct(1, 1);
		u->convert(o_unit, mstruct, exp, &b);
		if(b) {
			o_unit = u;
			if(!exp.isOne()) {
				raise(exp);
			}
			if(!mstruct.isOne()) {
				multiply(mstruct);
			}
		}
		return b;
	} else {
		if(convert_complex_relations) {
			if(m_type == STRUCT_MULTIPLICATION) {
				int b_c = -1;
				for(unsigned int i = 0; i < SIZE; i++) {
					if(CHILD(i).isUnit_exp()) {
						if((CHILD(i).isUnit() && u->hasComplexRelationTo(CHILD(i).unit())) || (CHILD(i).isPower() && u->hasComplexRelationTo(CHILD(i)[0].unit()))) {
							if(b_c >= 0) {
								b_c = -1;
								break;
							}
							b_c = i;
						}
					}
				}
				if(b_c >= 0) {
					MathStructure mstruct(1, 1);
					if(SIZE == 2) {
						if(b_c == 0) mstruct = CHILD(1);
						else mstruct = CHILD(0);
					} else if(SIZE > 2) {
						mstruct = *this;
						mstruct.delChild(b_c + 1);
					}
					MathStructure exp(1, 1);
					Unit *u2;
					if(CHILD(b_c).isPower()) {
						if(CHILD(b_c)[0].testDissolveCompositeUnit(u)) {
							convert(u, convert_complex_relations);
							return true;
						}	
						exp = CHILD(b_c)[1];
						u2 = CHILD(b_c)[0].unit();
					} else {
						if(CHILD(b_c).testDissolveCompositeUnit(u)) {
							convert(u, convert_complex_relations);
							return true;
						}
						u2 = CHILD(b_c).unit();
					}
					u->convert(u2, mstruct, exp, &b);
					if(b) {
						set(u);
						if(!exp.isOne()) {
							raise(exp);
						}
						if(!mstruct.isOne()) {
							multiply(mstruct);
						}
					}
					return b;
				}
			} else if(m_type == STRUCT_POWER) {
				if(CHILD(0).isUnit() && u->hasComplexRelationTo(CHILD(0).unit())) {
					if(CHILD(0).testDissolveCompositeUnit(u)) {
						convert(u, convert_complex_relations);
						return true;
					}	
					MathStructure exp(CHILD(1));
					MathStructure mstruct(1, 1);
					u->convert(CHILD(0).unit(), mstruct, exp, &b);
					if(b) {
						set(u);
						if(!exp.isOne()) {
							raise(exp);
						}
						if(!mstruct.isOne()) {
							multiply(mstruct);
						}
					}
					return b;
				}
			}
			if(m_type == STRUCT_MULTIPLICATION || m_type == STRUCT_POWER) {
				for(unsigned int i = 0; i < SIZE; i++) {
					if(CHILD(i).convert(u, false)) b = true;
				}
				return b;
			}
		}
		for(unsigned int i = 0; i < SIZE; i++) {
			if(CHILD(i).convert(u, convert_complex_relations)) b = true;
		}
		return b;		
	}	
	return b;
}
bool MathStructure::convert(const MathStructure unit_mstruct, bool convert_complex_relations) {
	bool b = false;
	if(unit_mstruct.type() == STRUCT_UNIT) {
		if(convert(unit_mstruct.unit(), convert_complex_relations)) b = true;
	} else {
		for(unsigned int i = 0; i < unit_mstruct.size(); i++) {
			if(convert(unit_mstruct[i], convert_complex_relations)) b = true;
		}
	}	
	return b;
}

bool MathStructure::contains(const MathStructure &mstruct, bool check_variables, bool check_functions) const {
	if(equals(mstruct)) return true;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).contains(mstruct, check_variables, check_functions)) return true;
	}
	if(check_variables && m_type == STRUCT_VARIABLE && o_variable->isKnown()) {
		return ((KnownVariable*) o_variable)->get().contains(mstruct, check_variables, check_functions);
	} else if(check_functions && m_type == STRUCT_FUNCTION) {
		EvaluationOptions eo;
		eo.approximation = APPROXIMATION_APPROXIMATE;
		MathStructure mstruct2(*this);
		mstruct2.calculateFunctions(eo);
		return mstruct2.contains(mstruct, check_variables, check_functions);
	}
	return false;
}
bool MathStructure::containsType(int mtype) const {
	if(m_type == mtype) return true;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).containsType(mtype)) return true;
	}
	return false;
}
bool MathStructure::containsUnknowns() const {
	if(m_type == STRUCT_SYMBOLIC || (m_type == STRUCT_VARIABLE && !o_variable->isKnown())) return true;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).containsUnknowns()) return true;
	}
	return false;
}
bool MathStructure::containsDivision() const {
	if(m_type == STRUCT_DIVISION || m_type == STRUCT_INVERSE || (m_type == STRUCT_POWER && CHILD(1).hasNegativeSign())) return true;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).containsDivision()) return true;
	}
	return false;
}
void MathStructure::findAllUnknowns(MathStructure &unknowns_vector) {
	if(!unknowns_vector.isVector()) unknowns_vector.clearVector();
	switch(m_type) {
		case STRUCT_VARIABLE: {
			if(o_variable->isKnown()) {
				break;
			}
		}
		case STRUCT_SYMBOLIC: {
			bool b = false;
			for(unsigned int i = 0; i < unknowns_vector.size(); i++) {
				if(equals(unknowns_vector[i])) {
					b = true;
					break;
				}
			}
			if(!b) unknowns_vector.addItem(*this);
			break;
		}
		default: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).findAllUnknowns(unknowns_vector);
			}
		}
	}
}
bool MathStructure::replace(const MathStructure &mfrom, const MathStructure &mto) {
	bool b = false;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).replace(mfrom, mto)) {
			b = true;
			childUpdated(i + 1);
		}
	}
	if(equals(mfrom)) {
		set(mto);
		b = true;
	}
	return b;
}

MathStructure MathStructure::generateVector(MathStructure x_mstruct, const MathStructure &min, const MathStructure &max, int steps, MathStructure *x_vector, const EvaluationOptions &eo) {
	if(steps < 1) {
		steps = 1;
	}
	MathStructure x_value(min);
	MathStructure y_value;
	MathStructure y_vector;
	y_vector.clearVector();
	MathStructure step(max);
	step -= min;
	step /= steps;
	step.eval(eo);
	for(int i = 0; i <= steps; i++) {
		if(x_vector) {
			x_vector->addComponent(x_value);
		}
		y_value = *this;
		y_value.replace(x_mstruct, x_value);
		y_value.eval(eo);
		y_vector.addComponent(y_value);
		x_value += step;
		x_value.eval(eo);
	}
	return y_vector;
}
MathStructure MathStructure::generateVector(MathStructure x_mstruct, const MathStructure &min, const MathStructure &max, const MathStructure &step, MathStructure *x_vector, const EvaluationOptions &eo) {
	MathStructure x_value(min);
	MathStructure y_value;
	MathStructure y_vector;
	y_vector.clearVector();
	ComparisonResult cr = max.compare(x_value);
	while(COMPARISON_IS_EQUAL_OR_LESS(cr)) {
		if(x_vector) {
			x_vector->addComponent(x_value);
		}
		y_value = *this;
		y_value.replace(x_mstruct, x_value);
		y_value.eval(eo);
		y_vector.addComponent(y_value);
		x_value += step;
		x_value.eval(eo);
		cr = max.compare(x_value);
	}
	return y_vector;
}
MathStructure MathStructure::generateVector(MathStructure x_mstruct, const MathStructure &x_vector, const EvaluationOptions &eo) {
	MathStructure y_value;
	MathStructure y_vector;
	y_vector.clearVector();
	for(unsigned int i = 1; i <= x_vector.components(); i++) {
		y_value = *this;
		y_value.replace(x_mstruct, x_vector.getComponent(i));
		y_value.eval(eo);
		y_vector.addComponent(y_value);
	}
	return y_vector;
}

bool MathStructure::differentiate(const MathStructure &x_var, const EvaluationOptions &eo) {
	if(equals(x_var)) {
		set(1, 1);
		return true;
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).differentiate(x_var, eo);
			}
			break;
		}
		case STRUCT_ALTERNATIVES: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).differentiate(x_var, eo);
			}
			break;
		}
		case STRUCT_AND: {}
		case STRUCT_OR: {}
		case STRUCT_XOR: {}
		case STRUCT_COMPARISON: {}
		case STRUCT_UNIT: {}
		case STRUCT_NUMBER: {
			clear();
			break;
		}
		case STRUCT_POWER: {
			if(SIZE < 1) {
				clear();
				break;
			} else if(SIZE < 2) {
				MathStructure base_copy(CHILD(0));
				set(base_copy);
				return differentiate(x_var, eo);
			}
			bool x_in_base = CHILD(0).contains(x_var, true, true);
			bool x_in_exp = CHILD(1).contains(x_var, true, true);
			if(x_in_base && !x_in_exp) {
				MathStructure exp_mstruct(CHILD(1));
				MathStructure base_mstruct(CHILD(0));
				CHILD(1) += m_minus_one;
				multiply(exp_mstruct);
				base_mstruct.differentiate(x_var, eo);
				multiply(base_mstruct);
			} else if(!x_in_base && x_in_exp) {
				MathStructure exp_mstruct(CHILD(1));
				MathStructure mstruct(CALCULATOR->f_ln, &CHILD(0), NULL);
				multiply(mstruct);
				exp_mstruct.differentiate(x_var, eo);
				multiply(exp_mstruct);
			} else if(x_in_base && x_in_exp) {
				MathStructure exp_mstruct(CHILD(1));
				MathStructure base_mstruct(CHILD(0));
				exp_mstruct.differentiate(x_var, eo);
				base_mstruct.differentiate(x_var, eo);
				base_mstruct /= CHILD(0);
				base_mstruct *= CHILD(1);
				MathStructure mstruct(CALCULATOR->f_ln, &CHILD(0), NULL);
				mstruct *= exp_mstruct;
				mstruct += base_mstruct;
				multiply(mstruct);
			} else {
				clear();
			}
			break;
		}
		case STRUCT_FUNCTION: {
			if(o_function == CALCULATOR->f_ln && SIZE == 1) {
				MathStructure mstruct(CHILD(0));
				set(mstruct);
				inverse();
				mstruct.differentiate(x_var, eo);
				multiply(mstruct);
			} else if(o_function == CALCULATOR->f_sin && SIZE == 1) {
				o_function = CALCULATOR->f_cos;
				MathStructure mstruct(CHILD(0));
				mstruct.differentiate(x_var, eo);
				multiply(mstruct);
			} else if(o_function == CALCULATOR->f_cos && SIZE == 1) {
				o_function = CALCULATOR->f_sin;
				MathStructure mstruct(CHILD(0));
				negate();
				mstruct.differentiate(x_var, eo);
				multiply(mstruct, true);
			} else if(o_function == CALCULATOR->f_integrate && SIZE == 2 && CHILD(1) == x_var) {
				MathStructure mstruct(CHILD(0));
				set(mstruct);
			} else if(o_function == CALCULATOR->f_diff && SIZE == 3 && CHILD(1) == x_var) {
				CHILD(2) += m_one;
			} else if(o_function == CALCULATOR->f_diff && SIZE == 2 && CHILD(1) == x_var) {
				APPEND(MathStructure(2));
			} else if(o_function == CALCULATOR->f_diff && SIZE == 1 && x_var == (Variable*) CALCULATOR->v_x) {
				APPEND(x_var);
				APPEND(MathStructure(2));
			} else {
				if(!eo.calculate_functions || !calculateFunctions(eo)) {
					MathStructure mstruct3(1);
					MathStructure mstruct(CALCULATOR->f_diff, this, &x_var, &mstruct3, NULL);
					set(mstruct);
					return false;
				} else {
					EvaluationOptions eo2 = eo;
					eo2.calculate_functions = false;
					return differentiate(x_var, eo2);
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if(SIZE > 2) {
				MathStructure mstruct = CHILD(0);
				mstruct.transform(STRUCT_MULTIPLICATION, CHILD(1));
				for(unsigned int i = 2; i < SIZE - 1; i++) {
					mstruct.addChild(CHILD(i));
				}
				MathStructure mstruct2(LAST);
				MathStructure mstruct3(mstruct);
				mstruct.differentiate(x_var, eo);
				mstruct2.differentiate(x_var, eo);			
				mstruct *= LAST;
				mstruct2 *= mstruct3;
				set(mstruct);
				add(mstruct2);
			} else {
				MathStructure mstruct(CHILD(0));
				MathStructure mstruct2(CHILD(1));
				mstruct.differentiate(x_var, eo);
				mstruct2.differentiate(x_var, eo);			
				mstruct *= CHILD(1);
				mstruct2 *= CHILD(0);
				set(mstruct);
				add(mstruct2);
			}
			break;
		}
		case STRUCT_SYMBOLIC: {
			if(representsNumber()) {
				clear();
			} else {
				MathStructure mstruct3(1);
				MathStructure mstruct(CALCULATOR->f_diff, this, &x_var, &mstruct3, NULL);
				set(mstruct);
				return false;
			}
			break;
		}
		case STRUCT_VARIABLE: {
			if(eo.calculate_variables && o_variable->isKnown()) {
				if(eo.approximation != APPROXIMATION_EXACT || !o_variable->isApproximate()) {
					set(((KnownVariable*) o_variable)->get());
					return differentiate(x_var, eo);
				} else if(contains(x_var, true, true)) {
					MathStructure mstruct3(1);
					MathStructure mstruct(CALCULATOR->f_diff, this, &x_var, &mstruct3, NULL);
					set(mstruct);
					return false;
				}
			}
			if(representsNumber()) {
				clear();
				break;
			}
		}		
		default: {
			MathStructure mstruct3(1);
			MathStructure mstruct(CALCULATOR->f_diff, this, &x_var, &mstruct3, NULL);
			set(mstruct);
			return false;
		}	
	}
	childrenUpdated();
	return true;
}

bool MathStructure::integrate(const MathStructure &x_var, const EvaluationOptions &eo) {
	if(equals(x_var)) {
		raise(2);
		multiply(MathStructure(1, 2));
		return true;
	}
	if(!contains(x_var, true, true)) {
		multiply(x_var);
		return true;
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).integrate(x_var, eo);
			}
			break;
		}
		case STRUCT_ALTERNATIVES: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).integrate(x_var, eo);
			}
			break;
		}
		case STRUCT_AND: {}
		case STRUCT_OR: {}
		case STRUCT_XOR: {}
		case STRUCT_COMPARISON: {}
		case STRUCT_UNIT: {}
		case STRUCT_NUMBER: {
			multiply(x_var);
			break;
		}
		case STRUCT_POWER: {
			if(CHILD(0).equals(x_var)) {
				if(CHILD(1).isNumber() && CHILD(1).number().isMinusOne()) {
					MathStructure mstruct(CALCULATOR->f_abs, &x_var, NULL);
					set(CALCULATOR->f_ln, &mstruct, NULL);
					break;
				} else if(CHILD(1).isNumber() || (!CHILD(1).contains(x_var, true, true) && CHILD(1).representsNonNegative())) {
					CHILD(1) += m_one;
					MathStructure mstruct(CHILD(1));
					divide(mstruct);
					break;
				}
			} else if(CHILD(0).isVariable() && CHILD(0).variable() == CALCULATOR->v_e) {
				if(CHILD(1).equals(x_var)) {
					break;
				} else if(CHILD(1).isMultiplication()) {
					bool b = false;
					unsigned int i = 0;
					for(; i < CHILD(1).size(); i++) {
						if(CHILD(1)[i].equals(x_var)) {
							b = true;
							break;
						}
					}
					if(b) {
						MathStructure mstruct;
						if(CHILD(1).size() == 2) {
							if(i == 0) {
								mstruct = CHILD(1)[1];
							} else {
								mstruct = CHILD(1)[0];
							}
						} else {
							mstruct = CHILD(1);
							mstruct.delChild(i + 1);
						}
						if(mstruct.representsNonZero() && !mstruct.contains(x_var, true, true)) {
							divide(mstruct);
							break;
						}
					}
				}
			} else if(CHILD(1).equals(x_var) && !CHILD(0).contains(x_var, true, true) && CHILD(0).representsPositive()) {
				MathStructure mstruct(CALCULATOR->f_ln, &CHILD(0), NULL);
				CHILD(1) *= mstruct;
				CHILD(0) = CALCULATOR->v_e;
				return integrate(x_var, eo);
			}
			MathStructure mstruct(CALCULATOR->f_integrate, this, &x_var, NULL);
			set(mstruct);
			return false;
		}
		case STRUCT_FUNCTION: {
			if(o_function == CALCULATOR->f_ln && SIZE == 1 && CHILD(0) == x_var) {
				multiply(x_var);
				subtract(x_var);
			} else if(o_function == CALCULATOR->f_sin && SIZE == 1 && CHILD(0) == x_var) {
				o_function = CALCULATOR->f_cos;
				multiply(m_minus_one);
			} else if(o_function == CALCULATOR->f_cos && SIZE == 1 && CHILD(0) == x_var) {
				o_function = CALCULATOR->f_sin;
			} else if(o_function == CALCULATOR->f_diff && SIZE == 3 && CHILD(1) == x_var) {
				if(CHILD(2).isOne()) {
					MathStructure mstruct(CHILD(0));
					set(mstruct);
				} else {
					CHILD(2) += m_minus_one;
				}
			} else if(o_function == CALCULATOR->f_diff && SIZE == 2 && CHILD(1) == x_var) {
				MathStructure mstruct(CHILD(0));
				set(mstruct);
			} else {
				if(!eo.calculate_functions || !calculateFunctions(eo)) {
					MathStructure mstruct(CALCULATOR->f_integrate, this, &x_var, NULL);
					set(mstruct);
					return false;
				} else {
					EvaluationOptions eo2 = eo;
					eo2.calculate_functions = false;
					return integrate(x_var, eo2);
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			MathStructure mstruct;
			bool b = false;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(!CHILD(i).contains(x_var)) {
					if(b) {
						mstruct *= CHILD(i);
					} else {
						mstruct = CHILD(i);
						b = true;
					}
					ERASE(i);
				}
			}
			if(b) {
				if(SIZE == 1) {
					MathStructure msave(CHILD(0));
					set(msave);
				} else if(SIZE == 0) {
					set(mstruct);
					break;	
				}
				integrate(x_var, eo);
				multiply(mstruct);
			} else {
				MathStructure mstruct(CALCULATOR->f_integrate, this, &x_var, NULL);
				set(mstruct);
				return false;
			}
			break;
		}
		case STRUCT_SYMBOLIC: {
			if(representsNumber()) {
				multiply(x_var);
			} else {
				MathStructure mstruct(CALCULATOR->f_integrate, this, &x_var, NULL);
				set(mstruct);
				return false;
			}
			break;
		}
		case STRUCT_VARIABLE: {
			if(eo.calculate_variables && o_variable->isKnown()) {
				if(eo.approximation != APPROXIMATION_EXACT || !o_variable->isApproximate()) {
					set(((KnownVariable*) o_variable)->get());
					return integrate(x_var, eo);
				} else if(contains(x_var, true, true)) {
					MathStructure mstruct3(1);
					MathStructure mstruct(CALCULATOR->f_integrate, this, &x_var, &mstruct3, NULL);
					set(mstruct);
					return false;
				}
			}
			if(representsNumber()) {
				multiply(x_var);
				break;
			}
		}		
		default: {
			MathStructure mstruct(CALCULATOR->f_integrate, this, &x_var, NULL);
			set(mstruct);
			return false;
		}	
	}
	childrenUpdated();
	return true;
}


const MathStructure &MathStructure::find_x_var() const {
	if(isSymbolic()) {
		return *this;
	} else if(isVariable()) {
		if(o_variable->isKnown()) return m_undefined;
		return *this;
	}
	const MathStructure *mstruct;
	const MathStructure *x_mstruct = &m_undefined;
	for(unsigned int i = 0; i < SIZE; i++) {
		mstruct = &CHILD(i).find_x_var();
		if(mstruct->isVariable()) {
			if(mstruct->variable() == CALCULATOR->v_x) {
				return *mstruct;
			} else if(!x_mstruct->isVariable()) {
				x_mstruct = mstruct;
			} else if(mstruct->variable() == CALCULATOR->v_y) {
				x_mstruct = mstruct;
			} else if(mstruct->variable() == CALCULATOR->v_z && x_mstruct->variable() != CALCULATOR->v_y) {
				x_mstruct = mstruct;
			}
		} else if(mstruct->isSymbolic()) {
			if(!x_mstruct->isVariable() && !x_mstruct->isSymbolic()) {
				x_mstruct = mstruct;
			}
		}
	}
	return *x_mstruct;
}
bool MathStructure::isolate_x(const EvaluationOptions &eo, const MathStructure &x_varp) {
	if(!isComparison()) {
		bool b = false;
		for(unsigned int i = 0; i < SIZE; i++) {
			if(CHILD(i).isolate_x(eo, x_varp)) b = true;
		}
		return b;
	}
	if(x_varp.isUndefined()) {
		const MathStructure *x_var2;
		if(eo.isolate_var) x_var2 = eo.isolate_var;
		else x_var2 = &find_x_var();
		if(x_var2->isUndefined()) return false;
		if(ct_comp != COMPARISON_EQUALS && ct_comp != COMPARISON_NOT_EQUALS) {
			return isolate_x(eo, *x_var2);
		}
		if(CHILD(1).isZero() && CHILD(0).isAddition()) {
			bool found_1x = false;
			for(unsigned int i = 0; i < CHILD(0).size(); i++) {
				if(CHILD(0)[i] == *x_var2) {
					found_1x = true;
				} else if(CHILD(0)[i].contains(*x_var2)) {
					found_1x = false;
					break;
				}
			}
			if(found_1x) return isolate_x(eo, *x_var2);
		}
		if(containsType(STRUCT_VECTOR)) return false;
		MathStructure x_var(*x_var2);
		MathStructure msave(*this);
		if(isolate_x(eo, x_var)) {	
			if(isComparison() && CHILD(0) == x_var) {
				bool b = false;
				MathStructure mtest;
				EvaluationOptions eo2 = eo;
				eo2.calculate_functions = false;
				eo2.isolate_x = false;
				eo2.test_comparisons = true;
				if(eo2.approximation == APPROXIMATION_EXACT) eo2.approximation = APPROXIMATION_TRY_EXACT;
				if(CHILD(1).isVector()) {
					MathStructure mtest2;
					for(int i2 = 0; i2 < (int) CHILD(1).size(); i2++) {
						mtest = x_var;
						mtest.transform(STRUCT_COMPARISON, CHILD(1)[i2]);
						mtest.setComparisonType(comparisonType());
						mtest.eval(eo2);
						if(mtest.isComparison()) {
							mtest = msave;
							mtest.replace(x_var, CHILD(1)[i2]);
							CALCULATOR->beginTemporaryStopErrors();
							mtest.eval(eo2);
							CALCULATOR->endTemporaryStopErrors();
							if(!mtest.isNumber() || (ct_comp == COMPARISON_EQUALS && mtest.number().getBoolean() < 1) || (ct_comp == COMPARISON_NOT_EQUALS && mtest.number().getBoolean() > 0)) {
								CHILD(1).delChild(i2 + 1);
								i2--;
							}
						} else {
							CHILD(1).delChild(i2 + 1);
							i2--;
							mtest2 = mtest;
							b = true;
						}
					}
					if(CHILD(1).isVector() && CHILD(1).size() > 0) {
						if(CHILD(1).size() == 1) {
							msave = CHILD(1)[0];
							CHILD(1) = msave;
						}
						return true;
					} else if(b) {
						set(mtest2);
						return true;
					}
				} else {
					mtest = *this;
					mtest.eval(eo2);
					if(mtest.isComparison()) {
						mtest = msave;
						mtest.replace(x_var, CHILD(1));
						CALCULATOR->beginTemporaryStopErrors();
						mtest.eval(eo2);
						CALCULATOR->endTemporaryStopErrors();
						if(mtest.isNumber() && ((ct_comp == COMPARISON_EQUALS && mtest.number().getBoolean() > 0) || (ct_comp == COMPARISON_NOT_EQUALS && mtest.number().getBoolean() < 1))) {
							return true;
						}
					} else {
						set(mtest);
						return true;
					}
					set(msave);
					return false;
				}
				set(msave);
			}
		}
		return false;
	}
	MathStructure x_var(x_varp);
	EvaluationOptions eo2 = eo;
	eo2.calculate_functions = false;
	eo2.test_comparisons = false;
	eo2.isolate_x = false;
	switch(CHILD(0).type()) {
		case STRUCT_ADDITION: {
			bool b = false;
			for(unsigned int i = 0; i < CHILD(0).size(); i++) {
				if(!CHILD(0)[i].contains(x_var)) {
					CHILD(1).subtract(CHILD(0)[i], true);
					CHILD(0).delChild(i + 1);
					b = true;
				}
			}
			if(b) {
				CHILD(1).eval(eo2);
				if(CHILD(0).size() == 1) {
					MathStructure msave(CHILD(0)[0]);
					CHILD(0) = msave;
				}
				isolate_x(eo, x_var);
				childrenUpdated();
				return true;
			}
			if(CHILD(0).size() == 2) {
				bool sqpow = false;
				bool nopow = false;
				MathStructure mstruct_a, mstruct_b;
				for(unsigned int i = 0; i < CHILD(0).size(); i++) {
					if(!sqpow && CHILD(0)[i].isPower() && CHILD(0)[i][0] == x_var && CHILD(0)[i][1].isNumber() && CHILD(0)[i][1].number() == 2) {
						sqpow = true;
						mstruct_a.set(1, 1);
					} else if(!nopow && CHILD(0)[i] == x_var) {
						nopow = true;
						mstruct_b.set(1, 1);
					} else if(CHILD(0)[i].isMultiplication()) {
						for(unsigned int i2 = 0; i2 < CHILD(0)[i].size(); i2++) {
							if(CHILD(0)[i][i2] == x_var) {
								if(nopow) break;
								nopow = true;
								mstruct_b = CHILD(0)[i];
								mstruct_b.delChild(i2 + 1);
							} else if(CHILD(0)[i][i2].isPower() && CHILD(0)[i][i2][0] == x_var && CHILD(0)[i][i2][1].isNumber() && CHILD(0)[i][i2][1].number() == 2) {
								if(sqpow) break;
								sqpow = true;
								mstruct_a = CHILD(0)[i];
								mstruct_a.delChild(i2 + 1);
							} else if(CHILD(0)[i][i2].contains(x_var)) {
								sqpow = false;
								nopow = false;
								break;
							}
						}
					}
				}	
				if(sqpow && nopow) {
					MathStructure b2(mstruct_b);
					b2 ^= 2;
					MathStructure ac(4, 1);
					ac *= mstruct_a;
					ac *= CHILD(1);
					b2 += ac;
					b2 ^= MathStructure(1, 2);
					mstruct_b.negate();
					MathStructure mstruct_1(mstruct_b);
					mstruct_1 += b2;
					MathStructure mstruct_2(mstruct_b);
					mstruct_2 -= b2;
					mstruct_a *= 2;
					mstruct_1 /= mstruct_a;
					mstruct_2 /= mstruct_a;
					mstruct_1.eval(eo2);
					mstruct_2.eval(eo2);
					CHILD(0) = x_var;
					if(mstruct_1 != mstruct_2) {
						CHILD(1).clearVector();
						CHILD(1).addItem(mstruct_1);
						CHILD(1).addItem(mstruct_2);
					} else {
						CHILD(1) = mstruct_1;
					}
					return true;
				} 
			}
			for(unsigned int i2 = 0; i2 < CHILD(0).size(); i2++) {
				if(CHILD(0)[i2].isMultiplication()) {
					int index = -1;
					for(unsigned int i = 0; i < CHILD(0)[i2].size(); i++) {
						if(CHILD(0)[i2][i].contains(x_var)) {
  							if(CHILD(0)[i2][i].isPower() && CHILD(0)[i2][i][1].isNumber() && CHILD(0)[i2][i][1].number().isMinusOne() && (eo.assume_denominators_nonzero || CHILD(0)[i2][i][0].representsNonZero())) {
								index = i;
								break;
							}
						}
					}
					if(index >= 0) {
						MathStructure msave(CHILD(0)[i2]);
						CHILD(0).delChild(i2 + 1);
						CHILD(0) -= CHILD(1);
						CHILD(1).clear();
						CHILD(0) *= msave[index][0];
						msave.delChild(index + 1);
						if(msave.size() == 1) {
							MathStructure msave2(msave[0]);
							msave = msave2;
						}
						CHILD(0) += msave;
						CHILD(0).eval(eo2);
						isolate_x(eo, x_var);
						childrenUpdated();
						return true;
					}
				}
			}
			if(CHILD(1).isNumber()) {
				MathStructure mtest(*this);
				if(!CHILD(1).isZero()) {
					mtest[0].add(CHILD(1), true);
					mtest[0][mtest[0].size() - 1].number().negate();
					mtest[1].clear();
				}
				if(mtest[0].factorize(eo)) {
					childUpdated(1);
					if(mtest.isolate_x(eo, x_var)) {
						set(mtest);
						return true;
					}
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			bool b = false;
			for(unsigned int i = 0; i < CHILD(0).size(); i++) {
				if(!CHILD(0)[i].contains(x_var)) {
					if(eo.assume_denominators_nonzero || CHILD(0)[i].representsNonZero()) {
						bool b2 = false;
						if(ct_comp != COMPARISON_EQUALS && ct_comp != COMPARISON_NOT_EQUALS) {
							if(CHILD(0)[i].representsNegative()) {
								switch(ct_comp) {
									case COMPARISON_LESS: {ct_comp = COMPARISON_GREATER; break;}
									case COMPARISON_GREATER: {ct_comp = COMPARISON_LESS; break;}
									case COMPARISON_EQUALS_LESS: {ct_comp = COMPARISON_EQUALS_GREATER; break;}
									case COMPARISON_EQUALS_GREATER: {ct_comp = COMPARISON_EQUALS_LESS; break;}
									default: {}
								}
								b2 = true;
							} else if(CHILD(0)[i].representsNonNegative()) {
								b2 = true;
							}
						} else {
							b2 = true;
						}
						if(b2) {
							CHILD(1).divide(CHILD(0)[i], true);
							CHILD(0).delChild(i + 1);
							b = true;
						}
					}
				}
			}
			CHILD(1).eval(eo2);
			childUpdated(2);
			if(b) {
				if(CHILD(0).size() == 1) {
					MathStructure msave(CHILD(0)[0]);
					CHILD(0) = msave;
				}
				if(CHILD(1).contains(x_var)) {
					CHILD(0) -= CHILD(1);
					CHILD(1).clear();
				}
				isolate_x(eo, x_var);
				childrenUpdated();
				return true;
			} else if(CHILD(1).isZero() && (ct_comp == COMPARISON_EQUALS || ct_comp == COMPARISON_NOT_EQUALS)) {
				MathStructure mtest, mtest2, msol;
				msol.clearVector();
				for(unsigned int i = 0; i < CHILD(0).size(); i++) {
					if(CHILD(0)[i].contains(x_var)) {
						mtest.clear();
						mtest.transform(STRUCT_COMPARISON, CHILD(1));
						mtest.setComparisonType(ct_comp);
						mtest[0] = CHILD(0)[i];
						mtest.isolate_x(eo, x_var);
						if(mtest[0] == x_var) {
							eo2.test_comparisons = true;
							if(eo2.approximation == APPROXIMATION_EXACT) eo2.approximation = APPROXIMATION_TRY_EXACT;eo2.approximation = APPROXIMATION_TRY_EXACT;
							if(mtest[1].isVector()) {
								for(unsigned int i2 = 0; i2 < mtest[1].size(); i2++) {
									mtest2 = x_var;
									mtest2.transform(STRUCT_COMPARISON, mtest[1][i2]);
									mtest2.setComparisonType(mtest.comparisonType());
									mtest2.eval(eo2);
									if(mtest2.isComparison()) {
										mtest2 = *this;
										mtest2[0].replace(x_var, mtest[1][i2]);
										CALCULATOR->beginTemporaryStopErrors();
										mtest2.eval(eo2);
										CALCULATOR->endTemporaryStopErrors();
										if(mtest2.isNumber() && ((ct_comp == COMPARISON_EQUALS && mtest2.number().getBoolean() > 0) || (ct_comp == COMPARISON_NOT_EQUALS && mtest2.number().getBoolean() < 1))) {
											msol.addChild(mtest[1][i2]);
										}
									}
								}
							} else {
								mtest2 = mtest;
								mtest2.eval(eo2);
								if(mtest2.isComparison()) {
									mtest2 = *this;
									mtest2[0].replace(x_var, mtest[1]);
									CALCULATOR->beginTemporaryStopErrors();
									mtest2.eval(eo2);
									CALCULATOR->endTemporaryStopErrors();
									if(mtest2.isNumber() && ((ct_comp == COMPARISON_EQUALS && mtest2.number().getBoolean() > 0) || (ct_comp == COMPARISON_NOT_EQUALS && mtest2.number().getBoolean() < 1))) {
										msol.addChild(mtest[1]);
									}
								}
							}
						}
					}
				}
				if(msol.size() > 0) {
					CHILD(0) = x_var;
					if(msol.size() == 1) {
						CHILD(1) = msol[0];
					} else {
						CHILD(1) = msol;
					}
					childrenUpdated();
					return true;
				}
			}
			break;
		} 
		case STRUCT_POWER: {
			if(CHILD(0)[0].contains(x_var)) {
				if(!CHILD(0)[1].contains(x_var)) {
					MathStructure mnonzero, mbefore;
					bool test_nonzero = false;
					if(!CHILD(0)[1].representsNonNegative() && !CHILD(0)[0].representsNonZero()) {
						if(ct_comp != COMPARISON_EQUALS && ct_comp != COMPARISON_NOT_EQUALS) return false;
						test_nonzero = true;
						if(CHILD(0)[1].isNumber()) {
							if(CHILD(0)[1].isMinusOne()) {
								mnonzero = CHILD(0)[0];
							} else {
								mnonzero = CHILD(0);
								mnonzero[1].number().setNegative(false);
							}
						} else {
							mnonzero = CHILD(0);
							mnonzero[1].negate();
						}
					}
					if(CHILD(0)[1].representsEven()) {

						MathStructure exp(1, 1);
						exp /= CHILD(0)[1];
						
						bool b1 = true, b2 = true;
						
						MathStructure mstruct(*this);
						mstruct[1] ^= exp;
						mstruct[1].eval(eo2);
						mstruct[0] = CHILD(0)[0];
						
						MathStructure mtest1(mstruct);
						mtest1.isolate_x(eo, x_var);
						mtest1.childrenUpdated();
						MathStructure mtestC1(mtest1);
						eo2.test_comparisons = true;
						mtestC1.eval(eo2);
						b1 = mtestC1.isComparison();
						eo2.test_comparisons = false;
						
						if(test_nonzero && b1) {
							if(mtestC1[0] != x_var) {
								return false;
							}
							mnonzero.replace(x_var, mtestC1[1]);
							CALCULATOR->beginTemporaryStopErrors();
							mnonzero.eval(eo2);
							CALCULATOR->endTemporaryStopErrors();
							if(!((eo.assume_denominators_nonzero && !mnonzero.isZero()) || mnonzero.representsNonZero())) {
								if(eo.assume_denominators_nonzero) {
									b1 = false;
								} else {
									return false;
								}
							}	
						}
						
						MathStructure mtest2(mstruct);
						mtest2[1].negate();
						mtest2[1].eval(eo2);
						switch(ct_comp) {
							case COMPARISON_LESS: {mtest2.setComparisonType(COMPARISON_GREATER); break;}
							case COMPARISON_GREATER: {mtest2.setComparisonType(COMPARISON_LESS); break;}
							case COMPARISON_EQUALS_LESS: {mtest2.setComparisonType(COMPARISON_EQUALS_GREATER); break;}
							case COMPARISON_EQUALS_GREATER: {mtest2.setComparisonType(COMPARISON_EQUALS_LESS); break;}
							default: {}
						}
						mtest2.isolate_x(eo, x_var);
						mtest2.childrenUpdated();
						MathStructure mtestC2(mtest2);
						if(mtest2 == mtest1) {
							b2 = false;
							mtestC2 = mtestC1;	
						}
						if(b2) {
							eo2.test_comparisons = true;
							mtestC2.eval(eo2);
							b2 = mtestC2.isComparison();
							eo2.test_comparisons = false;
						}
						if(test_nonzero && b2) {
							if(mtestC2[0] != x_var) {
								return false;
							}
							mnonzero.replace(x_var, mtestC2[1]);
							CALCULATOR->beginTemporaryStopErrors();
							mnonzero.eval(eo2);
							CALCULATOR->endTemporaryStopErrors();
							if(!((eo.assume_denominators_nonzero && !mnonzero.isZero()) || mnonzero.representsNonZero())) {
								if(b1 && eo.assume_denominators_nonzero) {
									b2 = false;
								} else {
									return false;
								}
							}	
						}
					
						if(b1 && !b2) {
							set(mtest1);	
						} else if(b2 && !b1) {
							set(mtest2);	
						} else if(!b1 && !b2) {
							if(mtestC1 == mtestC2) {
								set(mtestC1);
							} else {
								clearVector();
								APPEND(mtestC1);
								APPEND(mtestC2);
							}
						} else if(mtest1[0] == mtest2[0] && mtest1.comparisonType() == mtest2.comparisonType()) {
							CHILD(0) = mtest1[0];
							CHILD(1).clearVector();
							CHILD(1).addItem(mtest1[1]);
							CHILD(1).addItem(mtest2[1]);
						} else {
							clearVector();
							APPEND(mtest1);
							APPEND(mtest2);
						}
						childrenUpdated();						
					} else {
						if(test_nonzero) mbefore = *this;
						MathStructure exp(1, 1);
						exp /= CHILD(0)[1];
						CHILD(1) ^= exp;
						CHILD(1).eval(eo2);
						MathStructure msave(CHILD(0)[0]);
						CHILD(0) = msave;
						isolate_x(eo, x_var);
						childrenUpdated();
						if(test_nonzero) {
							if(isComparison() && CHILD(0) == x_var) {
								mnonzero.replace(x_var, CHILD(1));
								CALCULATOR->beginTemporaryStopErrors();
								mnonzero.eval(eo2);
								CALCULATOR->endTemporaryStopErrors();
								if((eo.assume_denominators_nonzero && !mnonzero.isZero()) || mnonzero.representsNonZero()) {
									return true;
								}	
							}
							set(mbefore);
							return false;
						}
					}
					return true;
				}
			} else if(CHILD(0)[1].contains(x_var)) {
				MathStructure msave(CHILD(1));
				CHILD(1).set(CALCULATOR->f_logn, &msave, &CHILD(0)[0], NULL);
				eo2.calculate_functions = true;
				CHILD(1).eval(eo2);
				MathStructure msave2(CHILD(0)[1]);
				CHILD(0) = msave2;
				isolate_x(eo, x_var);
				childrenUpdated();
				return true;
			}
			break;
		}
		case STRUCT_FUNCTION: {
			if(CHILD(0).function() == CALCULATOR->f_ln && CHILD(0).size() == 1) {
				if(ct_comp != COMPARISON_EQUALS && ct_comp != COMPARISON_NOT_EQUALS) {
					break;
				}
				if(CHILD(0)[0].contains(x_var)) {
					MathStructure msave(CHILD(1));
					CHILD(1).set(CALCULATOR->v_e);
					CHILD(1) ^= msave;
					CHILD(1).eval(eo2);
					MathStructure msave2(CHILD(0)[0]);
					CHILD(0) = msave2;
					isolate_x(eo, x_var);
					childrenUpdated();
					return true;
				}
			} else if(CHILD(0).function() == CALCULATOR->f_logn && CHILD(0).size() == 2) {
				if(ct_comp != COMPARISON_EQUALS && ct_comp != COMPARISON_NOT_EQUALS) {
					break;
				}
				if(CHILD(0)[0].contains(x_var)) {
					MathStructure msave(CHILD(1));
					CHILD(1) = CHILD(0)[1];
					CHILD(1) ^= msave;
					CHILD(1).eval(eo2);
					MathStructure msave2(CHILD(0)[0]);
					CHILD(0) = msave2;
					isolate_x(eo, x_var);
					childrenUpdated();
					return true;
				}
			} else if(CHILD(0).function() == CALCULATOR->f_abs && CHILD(0).size() == 1) {
				if(CHILD(0)[0].contains(x_var)) {
					
					bool b1 = true, b2 = true;
									
					MathStructure mtest1(*this);
					mtest1[0] = CHILD(0)[0];
					mtest1.isolate_x(eo, x_var);
					mtest1.childrenUpdated();
					MathStructure mtestC1(mtest1);
					eo2.test_comparisons = true;
					mtestC1.eval(eo2);
					b1 = mtestC1.isComparison();
					eo2.test_comparisons = false;
						
					MathStructure mtest2(*this);
					mtest2[0] = CHILD(0)[0];
					mtest2[1].negate();
					mtest2[1].eval(eo2);
					switch(ct_comp) {
						case COMPARISON_LESS: {mtest2.setComparisonType(COMPARISON_GREATER); break;}
						case COMPARISON_GREATER: {mtest2.setComparisonType(COMPARISON_LESS); break;}
						case COMPARISON_EQUALS_LESS: {mtest2.setComparisonType(COMPARISON_EQUALS_GREATER); break;}
						case COMPARISON_EQUALS_GREATER: {mtest2.setComparisonType(COMPARISON_EQUALS_LESS); break;}
						default: {}
					}
					mtest2.isolate_x(eo, x_var);
					mtest2.childrenUpdated();
					MathStructure mtestC2(mtest2);
					if(mtest2 == mtest1) {
						b2 = false;
						mtestC2 = mtestC1;	
					}
					if(b2) {
						eo2.test_comparisons = true;
						mtestC2.eval(eo2);
						b2 = mtestC2.isComparison();
						eo2.test_comparisons = false;
					}
					
					
					if(b1 && !b2) {
						set(mtest1);	
					} else if(b2 && !b1) {
						set(mtest2);	
					} else if(!b1 && !b2) {
						if(mtestC1 == mtestC2) {
							set(mtestC1);
						} else {
							clearVector();
							APPEND(mtestC1);
							APPEND(mtestC2);
						}
					} else if(mtest1[0] == mtest2[0] && mtest1.comparisonType() == mtest2.comparisonType()) {
						CHILD(0) = mtest1[0];
						CHILD(1).clearVector();
						CHILD(1).addItem(mtest1[1]);
						CHILD(1).addItem(mtest2[1]);
					} else {
						clearVector();
						APPEND(mtest1);
						APPEND(mtest2);
					}
					childrenUpdated();
					return true;
					
				}
			}
			break;
		}
	}
	return false;
}


