/*
    Qalculate    

    Copyright (C) 2004  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "MathStructure.h"
#include "Number.h"
#include "Function.h"
#include "Variable.h"
#include "Unit.h"
#include "Prefix.h"


#define APPEND(o)	v_order.push_back(v_subs.size()); v_subs.push_back(o); if(!b_approx && o.isApproximate()) b_approx = true;
#define PREPEND(o)	v_order.insert(v_order.begin(), v_subs.size()); v_subs.push_back(o); if(!b_approx && o.isApproximate()) b_approx = true;
#define CLEAR		v_order.clear(); v_subs.clear();
#define REDUCE(v_size)	for(unsigned int v_index = v_size; v_index < v_order.size(); v_index++) {v_subs.erase(v_subs.begin() + v_order[v_index]);} v_order.resize(v_size);
#define CHILD(v_index)	v_subs[v_order[v_index]]
#define SIZE		v_order.size()
#define LAST		v_subs[v_order[v_order.size() - 1]]
#define ERASE(v_index)	v_subs.erase(v_subs.begin() + v_order[v_index]); for(unsigned int v_index2 = 0; v_index2 < v_order.size(); v_index2++) {if(v_order[v_index2] > v_order[v_index]) v_order[v_index2]--;} v_order.erase(v_order.begin() + v_index);


inline void MathStructure::init() {
#ifdef HAVE_GIAC
	giac_unknown = NULL;
#endif
	m_type = STRUCT_NUMBER;
	b_approx = false;
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
	m_type = o.type();
}
void MathStructure::set(int num, int den, int exp10) {
	clear();
	o_number.set(num, den, exp10);
	b_approx = o_number.isApproximate();
	m_type = STRUCT_NUMBER;
}
void MathStructure::set(double float_value) {
	clear();
	o_number.setFloat(float_value);
	b_approx = o_number.isApproximate();
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
	if(b_approx) return;
	if(m_type == STRUCT_NUMBER && o_number.isApproximate()) {
		b_approx = true;
	}
}
void MathStructure::childUpdated(unsigned int index) {
	if(b_approx) return;
	if(index <= SIZE && index > 0 && CHILD(index - 1).isApproximate()) {
		b_approx = true;
	}
}
void MathStructure::childrenUpdated() {
	if(b_approx) return;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).isApproximate()) {
			b_approx = true;
			break;
		}	
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
bool MathStructure::representsUndefined() const {
	switch(m_type) {
		case STRUCT_UNDEFINED: {return true;}
		case STRUCT_POWER: {return (CHILD(0).isZero() && CHILD(1).representsNegative()) || (CHILD(0).isInfinity() && CHILD(1).isZero());}
		case STRUCT_FUNCTION: {return o_function->representsUndefined(*this);}
		default: {return false;}
	}
}


void MathStructure::setApproximate(bool is_approx) {
	b_approx = is_approx;
}
bool MathStructure::isApproximate() const {
	return b_approx;
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
	APPEND(MathStructure(-1));
	APPEND(struct_this);
}
void MathStructure::inverse() {
	//transform(STRUCT_INVERSE);
	raise(-1);
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
			set(mstruct);
			b_approx = true;
		} else {
			set(mstruct);
		}
		return 1;
	}
	if(mstruct.isZero()) {
		if(mstruct.isApproximate()) b_approx = true;
		return 1;
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
						if(mstruct.isApproximate()) b_approx = true;
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
					if(mstruct.isApproximate()) b_approx = true;
					return 1;
				}
				default: {
					APPEND(mstruct);
					if(mstruct.isApproximate()) b_approx = true;
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
								APPEND(MathStructure(1));
							}
							if(i2 == 0) {
								CHILD(0).number() += 1;
							} else {
								CHILD(0).number() += mstruct[0].number();
							}
							if(mstruct.isApproximate()) b_approx = true;
							return 1;
						}
					}
					break;
				}
				default: {
					if(SIZE == 2 && CHILD(0).isNumber() && CHILD(1) == mstruct) {
						CHILD(0).number() += 1;
						if(mstruct.isApproximate()) b_approx = true;
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
						if(mstruct.isApproximate()) b_approx = true;
						return 1;
					}
				}
			}	
		}		
	}
	return -1;
}

int MathStructure::merge_multiplication(const MathStructure &mstruct, const EvaluationOptions &eo) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		Number nr(o_number);
		if(nr.multiply(mstruct.number()) && (eo.approximation == APPROXIMATION_APPROXIMATE || !nr.isApproximate() || o_number.isApproximate() || mstruct.number().isApproximate())) {
			o_number = nr;
			numberUpdated();
			return 1;
		}
		return -1;
	}
	if(mstruct.isOne()) {
		if(mstruct.isApproximate()) b_approx = true;
		return 1;
	} else if(isOne()) {
		if(b_approx) {
			set(mstruct);
			b_approx = true;
		} else {
			set(mstruct);
		}
		return 1;
	}
	if(representsUndefined() || mstruct.representsUndefined()) return -1;
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
								for(unsigned int index = 0; index <= msave[0].size(); index++) {
									mtmp = msave[index_r][index];
									mtmp *= mstruct[index][index_c];
									CHILD(index_r)[index_c] += mtmp;
								}			
							}		
						}
						if(mstruct.isApproximate()) b_approx = true;
						return 1;
					} else {
						if(SIZE == mstruct.size()) {
							for(unsigned int i = 0; i < SIZE; i++) {
								CHILD(i) *= mstruct[i];
							}
							m_type = STRUCT_ADDITION;
							if(mstruct.isApproximate()) b_approx = true;
							return 1;
						}
					}
					return -1;
				}
				default: {
					for(unsigned int i = 0; i < SIZE; i++) {
						CHILD(i) *= mstruct;
					}
					if(mstruct.isApproximate()) b_approx = true;
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
					if(mstruct.isApproximate()) b_approx = true;
					return 1;
				}
				case STRUCT_POWER: {
					if(mstruct[1].isNumber() && *this == mstruct[0]) {
						if(representsNonNegative() || mstruct[1].representsPositive()) {
							set(mstruct);
							CHILD(1) += 1;
							return 1;
						}
					}
				}
				default: {
					for(unsigned int i = 0; i < SIZE; i++) {
						CHILD(i).multiply(mstruct);
					}
					if(mstruct.isApproximate()) b_approx = true;
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
					if(mstruct.isApproximate()) b_approx = true;
					return 1;
				}
				case STRUCT_POWER: {
					if(mstruct[1].isNumber() && *this == mstruct[0]) {
						if(representsNonNegative() || mstruct[1].representsPositive()) {
							set(mstruct);
							CHILD(1) += 1;
							return 1;
						}
					}
				}
				default: {
					APPEND(mstruct);
					if(mstruct.isApproximate()) b_approx = true;
					return 1;
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
						if(CHILD(0).representsNonZero() || (CHILD(1).representsPositive() && mstruct[1].representsPositive())) {
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
							if(mstruct.isApproximate()) b_approx = true;
							return 1;
						}
					}
					break;
				}
				default: {
					if(CHILD(1).isNumber() && CHILD(0) == mstruct) {
						if(CHILD(0).representsNonZero() || CHILD(1).representsPositive()) {
							CHILD(1) += 1;
							if(mstruct.isApproximate()) b_approx = true;
							return 1;
						}
					}
					if(mstruct.isZero()) {
						clear();
						if(mstruct.isApproximate()) b_approx = true; 
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
					if(mstruct.isZero() || isZero()) {
						clear(); 
						if(mstruct.isApproximate()) b_approx = true;
						return 1;
					}
					if(equals(mstruct)) {
						raise(2);
						if(mstruct.isApproximate()) b_approx = true;
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
		if(nr.raise(mstruct.number()) && (eo.approximation == APPROXIMATION_APPROXIMATE || !nr.isApproximate() || o_number.isApproximate() || mstruct.number().isApproximate())) {
			o_number = nr;
			numberUpdated();
			return 1;
		} else if(mstruct.number().isRational() && !mstruct.number().numerator().isOne()) {
			nr = o_number;
			if(nr.raise(mstruct.number().numerator()) && (eo.approximation == APPROXIMATION_APPROXIMATE || !nr.isApproximate() || o_number.isApproximate() || mstruct.number().isApproximate())) {
				o_number = nr;
				numberUpdated();
				nr.set(mstruct.number().denominator());
				nr.recip();
				raise(nr);
				return 1;
			}
		}
		return -1;
	}
	if(mstruct.isOne()) {
		if(mstruct.isApproximate()) b_approx = true;
		return 1;
	}
	if(representsUndefined() || mstruct.representsUndefined()) return -1;
	if(isZero() && mstruct.representsPositive()) {
		return 1;
	}
	if(mstruct.isZero()) {
		set(1, 1);
		if(mstruct.isApproximate()) b_approx = true;
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
						if(mstruct.isApproximate()) b_approx = true;
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
					if(mstruct.isApproximate()) b_approx = true;
					return 1;
				}
				
			}
			goto default_power_merge;
		}
		case STRUCT_ADDITION: {
			if(eo.simplify_addition_powers && mstruct.isNumber() && mstruct.number().isInteger() && !mstruct.number().isMinusOne()) {
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
				if(mstruct.isApproximate()) b_approx = true;
				return 1;
			}
			goto default_power_merge;
		}
		case STRUCT_MULTIPLICATION: {
			if(mstruct.representsInteger()) {
				for(unsigned int i = 0; i < SIZE; i++) {
					CHILD(i).raise(mstruct);	
				}
				if(mstruct.isApproximate()) b_approx = true;
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
					if(mstruct.isApproximate()) b_approx = true;
					return 1;
				}
			}
			goto default_power_merge;
		}
		case STRUCT_POWER: {
			if(mstruct.representsInteger() || CHILD(0).representsReal()) {
				if(CHILD(1).isNumber() && CHILD(1).number().isRational()) {
					if(CHILD(1).number().numeratorIsEven()) {
						MathStructure mstruct_base(CHILD(0));
						CHILD(0).set(CALCULATOR->f_abs, &mstruct_base, NULL);
					}
					CHILD(1) *= mstruct;
					if(mstruct.isApproximate()) b_approx = true;
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
								if(mstruct.isApproximate()) b_approx = true;
								return 1;
							} else if(img == Number(1, 2)) {
								clear();
								img.set(1, 1);
								o_number.setImaginaryPart(img);
								if(mstruct.isApproximate()) b_approx = true;
								return 1;
							} else if(img == Number(-1, 2)) {
								clear();
								img.set(-1, 1);
								o_number.setImaginaryPart(img);
								if(mstruct.isApproximate()) b_approx = true;
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
				if(mstruct.isApproximate()) b_approx = true;
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
						if(mstruct.isApproximate()) b_approx = true;
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
						if(eo.calculate_functions) calculateFunctions(feo);
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
				if(CHILD(0).representsNonPositive()) {
					clear();
					b = true;
				} else if(CHILD(0).representsPositive()) {
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
							set(1, 1);
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
							set(1, 1);
							b = true;	
						} else if(CHILD(0).representsNonZero()) {
							clear();
							b = true;
						}
						break;
					}
					case COMPARISON_NOT_EQUALS:  {
						if(incomp) {
							set(1, 1);
							b = true;
						} else if(CHILD(0).representsNonZero()) {
							set(1, 1);
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
							set(1, 1);
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
							set(1, 1);
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
							set(1, 1);
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
							set(1, 1);
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

int evalSortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2);
int evalSortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2) {
	if(mstruct1.type() != mstruct2.type()) {
		if(mstruct2.isAddition()) return -evalSortCompare(mstruct2, mstruct1);	
		if(mstruct2.isMultiplication() && !mstruct1.isAddition()) return -evalSortCompare(mstruct2, mstruct1);		
		if(mstruct2.isPower() && !mstruct1.isAddition() && !mstruct1.isMultiplication()) return -evalSortCompare(mstruct2, mstruct1);
	}
	switch(mstruct1.type()) {
		case STRUCT_NUMBER: {
			if(mstruct2.isNumber()) {
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
			}
			return -1;
		} 
		case STRUCT_UNIT: {
			if(mstruct2.isUnit()) {
				if(mstruct1.unit()->name() < mstruct2.unit()->name()) return -1;
				if(mstruct1.unit()->name() == mstruct2.unit()->name()) return 0;
				return 1;
			}
			if(mstruct2.isFunction()) return -1;
			return 1;
		}
		case STRUCT_SYMBOLIC: {
			if(mstruct2.isSymbolic()) {
				if(mstruct1.symbol() < mstruct2.symbol()) return -1;
				else if(mstruct1.symbol() == mstruct2.symbol()) return 0;
				else return 1;
			} else if(mstruct2.isVariable()) {
				if(mstruct1.symbol() < mstruct2.variable()->name()) return -1;
				else return 1;
			}
			if(mstruct2.isUnit() || mstruct2.isFunction()) return -1;
			return 1;
		}
		case STRUCT_VARIABLE: {
			if(mstruct2.isVariable()) {
				if(mstruct1.variable()->name() < mstruct2.variable()->name()) return -1;
				else if(mstruct1.variable()->name() == mstruct2.variable()->name()) return 0;
				else return 1;
			} else if(mstruct2.isSymbolic()) {
				if(mstruct1.variable()->name() <= mstruct2.symbol()) return -1;
				else return 1;
			}
			if(mstruct2.isUnit() || mstruct2.isFunction()) return -1;
			return 1;
		}
		case STRUCT_FUNCTION: {
			if(mstruct2.isFunction()) {
				if(mstruct1.function()->name() < mstruct2.function()->name()) return -1;
				if(mstruct1.function()->name() == mstruct2.function()->name()) {
					for(unsigned int i = 0; i < mstruct2.size(); i++) {
						if(i >= mstruct1.size()) {
							return -1;	
						}
						int i2 = evalSortCompare(mstruct1[i], mstruct2[i]);
						if(i2 != 0) return i2;
					}
					return 0;
				} else return 1;
			}
			return 1;
		}
		case STRUCT_POWER: {
			if(mstruct2.isPower()) {
				int i = evalSortCompare(mstruct1[0], mstruct2[0]);
				if(i == 0) {
					return evalSortCompare(mstruct1[1], mstruct2[1]);
				} else {
					return i;
				}
			} else if(!mstruct2.isAddition()) {
				int i = evalSortCompare(mstruct1[0], mstruct2);
				if(i == 0) {
					if(mstruct1[1].hasNegativeSign()) return 1;
					return -1;
				}
				return i;
			}
			return 1;
		}
		case STRUCT_MULTIPLICATION: {
			if(mstruct1.size() < 1) return 1;
			if(mstruct2.isNumber()) return -1;
			unsigned int start = 0;
			if(mstruct1[0].isNumber() && mstruct1.size() > 1) start = 1;
			if(mstruct2.size() < 1 || mstruct2.isPower()) return evalSortCompare(mstruct1[start], mstruct2);
			if(mstruct2.isMultiplication()) {
				if(mstruct2.size() < 1) return -1;
				unsigned int mstruct_start = 0;
				if(mstruct2[0].isNumber() && mstruct2.size() > 1) mstruct_start = 1;			
				for(unsigned int i = 0; ; i++) {
					if(i >= mstruct2.size() - mstruct_start && i >= mstruct1.size() - start) return 0;
					if(i >= mstruct2.size() - mstruct_start) return 1;
					if(i >= mstruct1.size() - start) return -1;
					int i2 = evalSortCompare(mstruct1[i + start], mstruct2[i + mstruct_start]);
					if(i2 != 0) return i2;
				}
			} 
			if(mstruct2.isAddition()) return 1;
			return -1;
		} 
		case STRUCT_ADDITION: {		
			if(mstruct1.size() < 1) return 1;
			if(mstruct2.isNumber()) return -1;
			if(mstruct2.isAddition()) {
				if(mstruct2.size() < 1) return -1;
				for(unsigned int i = 0; ; i++) {
					if(i >= mstruct2.size() && i >= mstruct1.size()) return 0;
					if(i >= mstruct2.size()) return 1;
					if(i >= mstruct1.size()) return -1;
					int i2 = evalSortCompare(mstruct1[i], mstruct2[i]);
					if(i2 != 0) return i2;
				}
			} 
			return evalSortCompare(mstruct1[0], mstruct2);
		}
		default: {
			return 1;
		}
	}
	return 1;
}

void MathStructure::evalSort() {
	if(m_type != STRUCT_ADDITION && m_type != STRUCT_MULTIPLICATION) return;
	vector<unsigned int> sorted;
	bool b;
	for(unsigned int i = 0; i < SIZE; i++) {
		b = false;
		if(m_type == STRUCT_MULTIPLICATION && CHILD(i).isNumber()) {
			sorted.insert(sorted.begin(), v_order[i]);
		} else if(m_type == STRUCT_MULTIPLICATION && CHILD(i).isPower() && CHILD(i)[0].isNumber()) {	
			for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
				if(v_subs[sorted[i2]].isNumber()) {
				} else if(v_subs[sorted[i2]].isPower() && v_subs[sorted[i2]][0].isNumber()) {
					if(evalSortCompare(CHILD(i), v_subs[sorted[i2]]) < 0) {
						b = true;
					}
				} else {
					b = true;
				}
				if(b) {
					sorted.insert(sorted.begin() + i2, v_order[i]);
					break;
				}
			}
			if(!b) sorted.push_back(v_order[i]);
		} else {		
			for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
				if(!(m_type == STRUCT_MULTIPLICATION && v_subs[sorted[i2]].isNumber_exp()) && evalSortCompare(CHILD(i), v_subs[sorted[i2]]) < 0) {
					sorted.insert(sorted.begin() + i2, v_order[i]);
					b = true;
					break;
				}
			}
			if(!b) sorted.push_back(v_order[i]);
		}
	}
	for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
		v_order[i2] = sorted[i2];
	}
}

int sortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2, const MathStructure &parent, const SortOptions &so);
int sortCompare(const MathStructure &mstruct1, const MathStructure &mstruct2, const MathStructure &parent, const SortOptions &so) {

	if(mstruct1.type() != mstruct2.type() && !mstruct1.isUnit()) {
		if(parent.isAddition() && so.minus_last) {
			bool m1 = mstruct1.hasNegativeSign(), m2 = mstruct2.hasNegativeSign();
			if(m1 && !m2) {
				return 1;
			} else if(m2 && !m1) {
				return -1;
			}
		}
		if(mstruct1.isNumber()) {
			if(parent.isMultiplication()) return -1;
			return 1;
		}
		if(mstruct2.isNumber()) {
			if(parent.isMultiplication()) return 1;
			return -1;
		}
		if(mstruct2.isUnit()) return -sortCompare(mstruct2, mstruct1, parent, so);
		else if(mstruct2.isAddition()) return -sortCompare(mstruct2, mstruct1, parent, so);	
		else if(mstruct2.isMultiplication() && !mstruct1.isAddition()) return -sortCompare(mstruct2, mstruct1, parent, so);
		else if(mstruct2.isPower() && !mstruct1.isAddition() && !mstruct1.isMultiplication()) return -sortCompare(mstruct2, mstruct1, parent, so);
	}

	switch(mstruct1.type()) {
		case STRUCT_NUMBER: {
			if(mstruct2.isNumber()) {
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
			}
			if(parent.isMultiplication()) return -1;
			return 1;
		} 
		case STRUCT_UNIT: {
			if(mstruct2.isUnit()) {
				if(mstruct1.unit()->name() < mstruct2.unit()->name()) return -1;
				if(mstruct1.unit()->name() == mstruct2.unit()->name()) return 0;
				return 1;
			}
			return 1;
		}
		case STRUCT_SYMBOLIC: {
			if(mstruct2.isSymbolic()) {
				if(mstruct1.symbol() < mstruct2.symbol()) return -1;
				else if(mstruct1.symbol() == mstruct2.symbol()) return 0;
				else return 1;
			} else if(mstruct2.isVariable()) {
				if(mstruct1.symbol() < mstruct2.variable()->name()) return -1;
				else return 1;
			}
			if(mstruct2.isUnit() || mstruct2.isFunction()) return -1;
			return 1;
		}
		case STRUCT_VARIABLE: {
			if(mstruct2.isVariable()) {
				if(mstruct1.variable()->name() < mstruct2.variable()->name()) return -1;
				else if(mstruct1.variable()->name() == mstruct2.variable()->name()) return 0;
				else return 1;
			} else if(mstruct2.isSymbolic()) {
				if(mstruct1.variable()->name() <= mstruct2.symbol()) return -1;
				else return 1;
			}
			if(mstruct2.isUnit() || mstruct2.isFunction()) return -1;
			return 1;
		}
		case STRUCT_FUNCTION: {
			if(mstruct2.isFunction()) {
				if(mstruct1.function()->name() < mstruct2.function()->name()) return -1;
				if(mstruct1.function()->name() == mstruct2.function()->name()) {
					for(unsigned int i = 0; i < mstruct2.size(); i++) {
						if(i >= mstruct1.size()) {
							return -1;	
						}
						int i2 = sortCompare(mstruct1[i], mstruct2[i], parent, so);
						if(i2 != 0) return i2;
					}
					return 0;
				} else return 1;
			}
			return 1;
		}
		case STRUCT_POWER: {
			if(mstruct2.isPower()) {
				int i = sortCompare(mstruct1[0], mstruct2[0], parent, so);
				if(i == 0) {
					return sortCompare(mstruct1[1], mstruct2[1], parent, so);
				} else {
					return i;
				}
			} else if(!mstruct2.isAddition()) {
				int i = sortCompare(mstruct1[0], mstruct2, parent, so);
				if(i == 0) {
					if(mstruct1[1].hasNegativeSign()) return 1;
					return -1;
				}
				return i;
			}
			return 1;
		}
		case STRUCT_MULTIPLICATION: {
			if(mstruct1.size() < 1) return 1;
			if(mstruct2.isNumber()) return -1;
			unsigned int start = 0;
			if(mstruct1[0].isNumber() && mstruct1.size() > 1) start = 1;
			if(mstruct2.size() < 1 || mstruct2.isPower()) return sortCompare(mstruct1[start], mstruct2, parent, so);
			if(mstruct2.isMultiplication()) {
				if(mstruct2.size() < 1) return -1;
				unsigned int mstruct_start = 0;
				if(mstruct2[0].isNumber() && mstruct2.size() > 1) mstruct_start = 1;			
				for(unsigned int i = 0; ; i++) {
					if(i >= mstruct2.size() - mstruct_start && i >= mstruct1.size() - start) return 0;
					if(i >= mstruct2.size() - mstruct_start) return 1;
					if(i >= mstruct1.size() - start) return -1;
					int i2 = sortCompare(mstruct1[i + start], mstruct2[i + mstruct_start], parent, so);
					if(i2 != 0) return i2;
				}
			} 
			if(mstruct2.isAddition()) return 1;
			return -1;
		} 
		case STRUCT_ADDITION: {		
			if(mstruct1.size() < 1) return 1;
			if(mstruct2.isNumber()) return -1;
			if(mstruct2.isAddition()) {
				if(mstruct2.size() < 1) return -1;
				for(unsigned int i = 0; ; i++) {
					if(i >= mstruct2.size() && i >= mstruct1.size()) return 0;
					if(i >= mstruct2.size()) return 1;
					if(i >= mstruct1.size()) return -1;
					int i2 = sortCompare(mstruct1[i], mstruct2[i], parent, so);
					if(i2 != 0) return i2;
				}
			} 
			return sortCompare(mstruct1[0], mstruct2, parent, so);
		}
		default: {
			return 1;
		}
	}
	return 1;
}

void MathStructure::sort(const SortOptions &so, bool recursive) {
	if(recursive) {
		for(unsigned int i = 0; i < SIZE; i++) {
			CHILD(i).sort(so);
		}
	}
	if(m_type != STRUCT_ADDITION && m_type != STRUCT_MULTIPLICATION) return;
	vector<unsigned int> sorted;
	bool b;
	for(unsigned int i = 0; i < SIZE; i++) {
		b = false;
		for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
			if(sortCompare(CHILD(i), v_subs[sorted[i2]], *this, so) < 0) {
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
MathStructure &MathStructure::eval(const EvaluationOptions &eo) {
	if(eo.structuring == STRUCTURING_NONE) return *this;
	unformat(eo);
	if(eo.sync_units) {
		syncUnits();
		unformat(eo);
	}
	if(eo.calculate_functions) {
		calculateFunctions(eo);
	}
	EvaluationOptions eo2 = eo;
	eo2.simplify_addition_powers = false;
	if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
		EvaluationOptions eo3 = eo2;
		eo3.approximation = APPROXIMATION_EXACT;
		calculatesub(eo3, eo);
		eo3.approximation = APPROXIMATION_APPROXIMATE;
		calculatesub(eo3, eo);
	} else {
		calculatesub(eo2, eo);
	}
	if(eo2.isolate_x) {
		isolate_x(eo2);
	}
	if(eo.simplify_addition_powers) {
		eo2.simplify_addition_powers = true;
		if(containsAdditionPower()) {
			if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
				EvaluationOptions eo3 = eo2;
				eo3.approximation = APPROXIMATION_APPROXIMATE;
				calculatesub(eo3, eo);
			} else {
				calculatesub(eo2, eo);
			}
			if(eo2.isolate_x) {
				isolate_x(eo2);
			}
		}
	}
	if(eo2.sync_units && eo2.sync_complex_unit_relations) {
		if(syncUnits(true)) {
			unformat(eo2);
			if(eo2.approximation == APPROXIMATION_TRY_EXACT) {
				EvaluationOptions eo3 = eo2;
				eo3.approximation = APPROXIMATION_EXACT;
				calculatesub(eo3, eo);
				eo3.approximation = APPROXIMATION_APPROXIMATE;
				calculatesub(eo3, eo);
			} else {
				calculatesub(eo2, eo);
			}
			if(eo2.isolate_x) {
				isolate_x(eo2);
			}
		}
	}
	return *this;
}
void MathStructure::factorize(const EvaluationOptions &eo) {
	switch(type()) {
		case STRUCT_ADDITION: {
			MathStructure factor_mstruct(1, 1);
			Number gcd;
			for(unsigned int i = 0; i < SIZE; i++) {
				if(CHILD(i).isNumber()) {
					if(gcd.isZero()) {
						gcd = CHILD(i).number();
					} else {
						if(!gcd.gcd(CHILD(i).number())) {
							gcd.clear();
							break;
						}
					}	
				} else if(CHILD(i).isMultiplication() && CHILD(i)[0].isNumber()) {	
					if(gcd.isZero()) {
						gcd.set(CHILD(i)[0].number());
					} else {
						if(!gcd.gcd(CHILD(i)[0].number())) {
							gcd.clear();
							break;
						}
					}
				} else {
					gcd.clear();
					break;
				}
				if(gcd.isOne() || gcd.isZero()) {
					gcd.clear();
					break;
				}
			}
			if(!gcd.isZero()) {
				factor_mstruct.set(gcd);
			}
			if(SIZE > 0) {
				unsigned int i = 0;
				MathStructure *cur_mstruct;
				while(true) {
					if(CHILD(0).isMultiplication()) {
						if(i >= CHILD(0).size()) {
							break;
						}
						cur_mstruct = &CHILD(0)[i];
					} else {
						cur_mstruct = &CHILD(0);
					}
					if(!cur_mstruct->isNumber() && (!cur_mstruct->isPower() || cur_mstruct->exponent()->isNumber())) {
						MathStructure *exp = NULL;
						MathStructure *bas;
						if(cur_mstruct->isPower()) {
							exp = cur_mstruct->exponent();
							bas = cur_mstruct->base();
						} else {
							bas = cur_mstruct;
						}
						bool b = true;
						for(unsigned int i2 = 1; i2 < SIZE; i2++) {
							b = false;
							unsigned int i3 = 0;
							MathStructure *cmp_mstruct;
							while(true) {
								if(CHILD(i2).isMultiplication()) {
									if(i3 >= CHILD(i2).size()) {
										break;
									}
									cmp_mstruct = &CHILD(i2)[i3];
								} else {
									cmp_mstruct = &CHILD(i2);
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
								if(!CHILD(i2).isMultiplication()) {
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
								factor_mstruct.multiply(mstruct, true);
							} else {
								factor_mstruct.multiply(*bas, true);
							}
						}
					}
					if(!CHILD(0).isMultiplication()) {
						break;
					}
					i++;
				}
			}
			if(!factor_mstruct.isOne()) {
				MathStructure msave(*this);
				msave /= factor_mstruct;
				EvaluationOptions eo2 = eo;
				eo2.calculate_functions = false;
				eo2.sync_units = false;
				msave.eval(eo2);
				clear();
				m_type = STRUCT_MULTIPLICATION;
				APPEND(factor_mstruct);
				APPEND(msave);
			}
			break;
		}
		default: {
			for(unsigned int i = 0; i < SIZE; i++) {
				CHILD(i).factorize();
			}
		}
	}
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
			APPEND(MathStructure(-1));
			m_type = STRUCT_POWER;	
		}
		case STRUCT_NEGATE: {
			PREPEND(MathStructure(-1));
			m_type = STRUCT_MULTIPLICATION;
		}
		case STRUCT_DIVISION: {
			CHILD(1).raise(-1);
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

void MathStructure::setPrefixes(const PrintOptions &po, const MathStructure *parent, unsigned int pindex) {
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
					if(po.use_prefixes_for_currencies || (CHILD(i2).isPower() && CHILD(i2)[0].unit()->baseUnit() != CALCULATOR->u_euro) || (CHILD(i2).isUnit() && CHILD(i2).unit()->baseUnit() != CALCULATOR->u_euro)) {
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
							exp10.trunc();
						}
					} else if(po.use_unit_prefixes && CHILD(0).isNumber() && exp.isInteger()) {
						exp10 = CHILD(0).number();
						exp10.log(10);
						exp10.trunc();
						if(b2) {	
							Number tmp_exp(exp10);
							Number texp(exp);
							texp -= exp2;
							tmp_exp.trunc(texp);
							tmp_exp.setNegative(false);
							if(tmp_exp.isGreaterThan(2)) {
								tmp_exp = exp10;
								tmp_exp /= texp;
								tmp_exp *= exp2;
								tmp_exp.setNegative(false);
								texp = exp2;
								texp *= 3;
								tmp_exp.trunc(texp);
								tmp_exp *= texp;
								exp10 -= tmp_exp;
								if(exp10.isEven()) {
									if(exp10.isNegative()) exp10--;
									else exp10++;
								}
							}
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
					if(b2 && CHILD(0).isNumber() && (po.prefix || po.use_unit_prefixes)) {
						exp10 = CHILD(0).number();
						exp10.log(10);
						exp10.trunc();
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
			if(!o_prefix && po.prefix) {
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
				if(CHILD(1).isNumber() && CHILD(1).number().isReal() && !o_prefix && po.prefix) {
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
void MathStructure::postFormatUnits(const PrintOptions &po, const MathStructure *parent, unsigned int pindex) {
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
						multiply(den, false);
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
					if(!CHILD(i).isUnit_exp()) {
						break;
					}
				}
				i--;
				if(CHILD(i).isUnit()) {
					CHILD(i).setPlural(do_plural);
				} else {
					CHILD(i)[0].setPlural(do_plural);
				}
				break;
			}
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
				if(CHILD(i).isUnit() && CHILD(i).unit()->baseUnit() == CALCULATOR->u_euro) {
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
	sort(po.sort_options);
	setPrefixes(po);
	formatsub(po);
	postFormatUnits(po);
	if(po.sort_options.prefix_currencies && po.abbreviate_units) {
		prefixCurrencies();
	}
}
void MathStructure::formatsub(const PrintOptions &po, const MathStructure *parent, unsigned int pindex) {

	for(unsigned int i = 0; i < SIZE; i++) {
		CHILD(i).formatsub(po, this, i + 1);
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
			MathStructure *mi, *mi2;
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
							(*mi)[0].add(MathStructure(1, 1), true);
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
					(*mi)[0].sort(po.sort_options, false);
					c = true;
				}
			}
			if(c) {
				if(SIZE == 1) {
					MathStructure msave(CHILD(0));
					set(msave);
				} else {
					sort(po.sort_options, false);
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
				if(CHILD(i).isInverse() || CHILD(i).isDivision()) {
					b = true;
					break;
				}
			}

			if(b) {
				MathStructure den;
				MathStructure num = m_undefined;
				short ds = 0, ns = 0;
				for(unsigned int i = 0; i < SIZE; i++) {
					if(CHILD(i).isInverse()) {
						if(ds > 0) {	
							den.multiply(CHILD(i)[0], ds > 1);
						} else {
							den = CHILD(i)[0];
						}
						ds++;
					} else if(CHILD(i).isDivision()) {
						if(ns > 0) {
							num.multiply(CHILD(i)[0], ns > 1);
						} else {
							num = CHILD(i)[0];
						}
						ns++;
						if(ds > 0) {	
							den.multiply(CHILD(i)[1], ds > 1);
						} else {
							den = CHILD(i)[1];
						}
						ds++;
					} else {
						if(ns > 0) {
							num.multiply(CHILD(i), ns > 1);
						} else {
							num = CHILD(i);
						}
						ns++;
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
			if(!po.negative_exponents && CHILD(1).isNegate()) {
				if(CHILD(1)[0].isOne()) {
					m_type = STRUCT_INVERSE;
					ERASE(1);
				} else {
					MathStructure mmove(CHILD(1)[0]);
					CHILD(1) = mmove;
					transform(STRUCT_INVERSE);
				}
				formatsub(po, parent, pindex);
			} else if(po.halfexp_to_sqrt && ((CHILD(1).isNumber() && CHILD(1).number() == Number(1, 2)) || (CHILD(1).isInverse() && CHILD(1)[0].isNumber() && CHILD(1)[0].number() == 2))) {
				m_type = STRUCT_FUNCTION;
				ERASE(1)
				o_function = CALCULATOR->f_sqrt;
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
					}
					mstruct.multiply(CALCULATOR->v_i);
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

int namelen(const MathStructure &mstruct, const PrintOptions &po, const InternalPrintStruct &ips) {
	const string *str;
	switch(mstruct.type()) {
		case STRUCT_FUNCTION: {
			str = &mstruct.function()->name(po.use_unicode_signs);
			break;
		}
		case STRUCT_VARIABLE:  {
			str = &mstruct.variable()->name(po.use_unicode_signs);
			break;
		}
		case STRUCT_SYMBOLIC:  {
			str = &mstruct.symbol();
			break;
		}
		case STRUCT_UNIT:  {
			if(po.abbreviate_units) {
				str = &mstruct.unit()->shortName(po.use_unicode_signs);
			} else if(mstruct.isPlural()) {
				str = &mstruct.unit()->plural(true, po.use_unicode_signs);
			} else {
				str = &mstruct.unit()->singular(true, po.use_unicode_signs);
			}
			break;
		}
		default: {return 0;}
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
				case STRUCT_MULTIPLICATION: {return false;}
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
	int namelen_prev = namelen(parent[index - 2], po, ips);
	int namelen_this = namelen(*this, po, ips);
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
		case STRUCT_VARIABLE: {}
		case STRUCT_SYMBOLIC: {
			break;
		}
		case STRUCT_UNIT: {
			if(m_type == STRUCT_UNIT) {
				if(namelen_prev > 1 || namelen_this > 1) {
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
			if(namelen_prev > 1 || namelen_this > 1) return MULTIPLICATION_SIGN_OPERATOR;
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
	switch(m_type) {
		case STRUCT_NUMBER: {
			print_str = o_number.print(po, ips_n);
			break;
		}
		case STRUCT_SYMBOLIC: {
			print_str = s_sym;
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
						print_str += "-";
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
			print_str = "-";
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
					print_str += "*";
					if(po.spacious) print_str += " ";
				} else if(i > 0) {
					switch(CHILD(i).neededMultiplicationSign(po, ips_n, *this, i + 1, ips_n.wrap, par_prev, true, true)) {
						case MULTIPLICATION_SIGN_SPACE: {print_str += " "; break;}
						case MULTIPLICATION_SIGN_OPERATOR: {if(po.spacious) {print_str += " * "; break;}}
						case MULTIPLICATION_SIGN_OPERATOR_SHORT: {print_str += "*"; break;}
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
			print_str += "/";
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
			print_str += "/";
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
			print_str += CHILD(1).print(po, ips_n);
			break;
		}
		case STRUCT_COMPARISON: {
			ips_n.depth++;
			ips_n.wrap = CHILD(0).needsParenthesis(po, ips_n, *this, 1, true, true);
			print_str = CHILD(0).print(po, ips_n);
			if(po.spacious) print_str += " ";
			switch(ct_comp) {
				case COMPARISON_EQUALS: {print_str += "="; break;}
				case COMPARISON_NOT_EQUALS: {print_str += "!="; break;}
				case COMPARISON_GREATER: {print_str += ">"; break;}
				case COMPARISON_LESS: {print_str += "<"; break;}
				case COMPARISON_EQUALS_GREATER: {print_str += ">="; break;}
				case COMPARISON_EQUALS_LESS: {print_str += "<="; break;}
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
			if(po.abbreviate_units) {
				if(o_prefix) {
					print_str = o_prefix->shortName(true, po.use_unicode_signs);
				}
				print_str += o_unit->shortName(po.use_unicode_signs);
			} else {
				if(o_prefix && o_prefix) {
					print_str = o_prefix->longName(true, po.use_unicode_signs);
				}
				if(b_plural) {
					print_str += o_unit->plural(true, po.use_unicode_signs);
				} else {
					print_str += o_unit->singular(true, po.use_unicode_signs);
				}
			}
			break;
		}
		case STRUCT_VARIABLE: {
			print_str = o_variable->name(po.use_unicode_signs);
			break;
		}
		case STRUCT_FUNCTION: {
			ips_n.depth++;
			print_str += o_function->name(po.use_unicode_signs);
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
	mstruct.raise(-1);
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
			switch(mstruct.unit()->unitType()) {
				case BASE_UNIT: {
					for(unsigned int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == mstruct.unit()) {
							return;
						}
					}
					base_units.push_back(mstruct.unit());
					break;
				}
				case ALIAS_UNIT: {
					for(unsigned int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == mstruct.unit()) {
							return;
						}
					}
					alias_units.push_back((AliasUnit*) (mstruct.unit()));
					break;
				}
				case COMPOSITE_UNIT: {
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
		if(alias_units[i]->baseUnit()->unitType() == COMPOSITE_UNIT) {
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
			switch(cu->units[i2]->firstBaseUnit()->unitType()) {
				case BASE_UNIT: {
					for(unsigned int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) base_units.push_back((Unit*) cu->units[i2]->firstBaseUnit());
					break;
				}
				case ALIAS_UNIT: {
					for(unsigned int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) alias_units.push_back((AliasUnit*) cu->units[i2]->firstBaseUnit());				
					break;
				}
				case COMPOSITE_UNIT: {
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
		if(alias_units[i]->baseUnit()->unitType() == BASE_UNIT) {
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
		if(o_unit->unitType() == COMPOSITE_UNIT) {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				set(((CompositeUnit*) o_unit)->generateMathStructure());
				return true;
			}
		} else if(o_unit->unitType() == ALIAS_UNIT && o_unit->baseUnit()->unitType() == COMPOSITE_UNIT) {
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
		if(o_unit->unitType() == COMPOSITE_UNIT) {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				return true;
			}
		} else if(o_unit->unitType() == ALIAS_UNIT && o_unit->baseUnit()->unitType() == COMPOSITE_UNIT) {
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
			if(o_unit->unitType() == COMPOSITE_UNIT) {
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
	if(u->unitType() == COMPOSITE_UNIT && !(m_type == STRUCT_UNIT && o_unit->baseUnit() == u)) {
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

bool MathStructure::contains(const MathStructure &mstruct) const {
	if(equals(mstruct)) return true;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).contains(mstruct)) return true;
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
bool MathStructure::replace(const MathStructure &mfrom, const MathStructure &mto) {
	bool b = false;
	for(unsigned int i = 0; i < SIZE; i++) {
		if(CHILD(i).replace(mfrom, mto)) b = true;
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
			bool x_in_base = CHILD(0).contains(x_var);
			bool x_in_exp = CHILD(1).contains(x_var);
			if(x_in_base && !x_in_exp) {
				MathStructure exp_mstruct(CHILD(1));
				MathStructure base_mstruct(CHILD(0));
				CHILD(1) += -1;
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
			} else if(o_function == CALCULATOR->f_diff && SIZE == 3 && CHILD(1) == x_var) {
				CHILD(2) += 1;
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
			clear();
			break;
		}
		case STRUCT_VARIABLE: {
			if(representsNumber()) {
				clear();
			} else {
				bool b = false;
				if(eo.calculate_variables && o_variable->isKnown()) {
					if(eo.approximation == APPROXIMATION_APPROXIMATE || !o_variable->isApproximate()) {
						set(((KnownVariable*) o_variable)->get());
						b = true;
					}
				}
				if(!b) {
					MathStructure mstruct3(1);
					MathStructure mstruct(CALCULATOR->f_diff, this, &x_var, &mstruct3, NULL);
					set(mstruct);
					return false;
				} else {
					EvaluationOptions eo2 = eo;
					eo2.calculate_variables = false;
					return differentiate(x_var, eo2);
				}
			}
			break;
		}		
		default: {
			MathStructure mstruct3(1);
			MathStructure mstruct(CALCULATOR->f_diff, this, &x_var, &mstruct3, NULL);
			set(mstruct);
			return false;
		}	
	}
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
		const MathStructure &x_var2 = find_x_var();
		if(x_var2.isUndefined()) return false;
		return isolate_x(eo, x_var2);
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
			break;
		}
		case STRUCT_MULTIPLICATION: {
			bool b = false;
			for(unsigned int i = 0; i < CHILD(0).size(); i++) {
				if(!CHILD(0)[i].contains(x_var) && CHILD(0)[i].representsNonZero()) {
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
			break;
		} 
		case STRUCT_POWER: {
			if(CHILD(0)[0].contains(x_var)) {
				if(!CHILD(0)[1].contains(x_var)) {
					if(!CHILD(0)[1].representsNonNegative() && !CHILD(0)[0].representsNonZero()) {
						return false;
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
						MathStructure exp(1, 1);
						exp /= CHILD(0)[1];
						CHILD(1) ^= exp;
						CHILD(1).eval(eo2);	
						MathStructure msave(CHILD(0)[0]);
						CHILD(0) = msave;
						isolate_x(eo, x_var);
						childrenUpdated();
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


