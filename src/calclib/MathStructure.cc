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
	va_list ap;
	va_start(ap, o); 
	if(o) {
		v_subs.push_back(*o);
		while(true) {
			o = va_arg(ap, const MathStructure*);
			if(!o) break;
			v_subs.push_back(*o);
		}
	}
	va_end(ap);	
	m_type = STRUCT_VECTOR;
}
MathStructure::MathStructure(Function *o, ...) {
	init();
	va_list ap;
	va_start(ap, o); 
	o_function = o;
	while(true) {
		const MathStructure *mstruct = va_arg(ap, const MathStructure*);
		if(!mstruct) break;
		v_subs.push_back(*mstruct);
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
			break;
		}
		case STRUCT_COMPARISON: {
			ct_comp = o.comparisonType();
			break;
		}
	}
	for(unsigned int i = 0; i < o.size(); i++) {
		v_subs.push_back(o[i]);
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
		v_subs.push_back(*o);
		while(true) {
			o = va_arg(ap, const MathStructure*);
			if(!o) break;
			v_subs.push_back(*o);
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
		v_subs.push_back(*mstruct);
	}
	va_end(ap);	
	m_type = STRUCT_FUNCTION;
}
void MathStructure::set(Unit *u, Prefix *p) {
	clear();
	if(p) {
		set(10);
		raise(p->exponent());
		multiply(u);
	} else {
		o_unit = u;
		m_type = STRUCT_UNIT;
	}
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

const MathStructure &MathStructure::operator [] (unsigned int index) const {return v_subs[index];}
MathStructure &MathStructure::operator [] (unsigned int index) {return v_subs[index];}

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
	v_subs.clear();
}
void MathStructure::clearVector() {
	clear();
	m_type = STRUCT_VECTOR;
}
void MathStructure::clearMatrix() {
	clearVector();
	v_subs.resize(1);
	v_subs[0].clearVector();
}

const Number &MathStructure::number() const {
	return o_number;
}
Number &MathStructure::number() {
	return o_number;
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
Unit *MathStructure::unit() const {
	return o_unit;
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
	if(m_type != STRUCT_VECTOR) return false;
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(!v_subs[i].isVector() || (i > 0 && v_subs[i].size() != v_subs[i - 1].size())) return false;
	}
	return true;
}
bool MathStructure::isFunction() const {return m_type == STRUCT_FUNCTION;}
bool MathStructure::isUnit() const {return m_type == STRUCT_UNIT;}
bool MathStructure::isUnit_exp() const {return m_type == STRUCT_UNIT || (m_type == STRUCT_POWER && v_subs[0].isUnit());}
bool MathStructure::isVariable() const {return m_type == STRUCT_VARIABLE;}
bool MathStructure::isComparison() const {return m_type == STRUCT_COMPARISON;}
bool MathStructure::isAND() const {return m_type == STRUCT_AND;}
bool MathStructure::isOR() const {return m_type == STRUCT_OR;}
bool MathStructure::isXOR() const {return m_type == STRUCT_XOR;}
bool MathStructure::isNOT() const {return m_type == STRUCT_NOT;}
bool MathStructure::isInverse() const {return m_type == STRUCT_INVERSE;}
bool MathStructure::isNegate() const {return m_type == STRUCT_NEGATE;}
bool MathStructure::isInfinity() const {return m_type == STRUCT_NUMBER && o_number.isInfinite();}
bool MathStructure::isUndefined() const {return m_type == STRUCT_UNDEFINED || (m_type == STRUCT_NUMBER && o_number.isUndefined());}
bool MathStructure::isNumber() const {return m_type == STRUCT_NUMBER;}
bool MathStructure::isZero() const {return m_type == STRUCT_NUMBER && o_number.isZero();}
bool MathStructure::isOne() const {return m_type == STRUCT_NUMBER && o_number.isOne();}

bool MathStructure::representsNumber() const {return (m_type == STRUCT_NUMBER) || (m_type == STRUCT_VARIABLE && o_variable->isNumber());}
bool MathStructure::representsInteger() const {return (m_type == STRUCT_NUMBER && o_number.isInteger()) || (m_type == STRUCT_VARIABLE && o_variable->isInteger());}
bool MathStructure::representsPositive() const {return (m_type == STRUCT_NUMBER && o_number.isPositive()) || (m_type == STRUCT_VARIABLE && o_variable->isPositive());}
bool MathStructure::representsNegative() const {return (m_type == STRUCT_NUMBER && o_number.isNegative()) || (m_type == STRUCT_VARIABLE && o_variable->isNegative());}
bool MathStructure::representsNonNegative() const {return (m_type == STRUCT_NUMBER && o_number.isNonNegative()) || (m_type == STRUCT_VARIABLE && o_variable->isNonNegative());}
bool MathStructure::representsRational() const {return (m_type == STRUCT_NUMBER && o_number.isRational()) || (m_type == STRUCT_VARIABLE && o_variable->isRational());}
bool MathStructure::representsReal() const {return (m_type == STRUCT_NUMBER && o_number.isReal()) || (m_type == STRUCT_VARIABLE && o_variable->isReal());}
bool MathStructure::representsNonZero() const {return (m_type == STRUCT_NUMBER && !o_number.isZero()) || (m_type == STRUCT_VARIABLE && o_variable->isNonZero());}


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
	v_subs.push_back(struct_this);
	v_subs.push_back(o);
}
void MathStructure::transform(int mtype) {
	MathStructure struct_this(*this);
	clear();
	m_type = mtype;
	v_subs.push_back(struct_this);
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
		case OPERATION_AND: {
			if(m_type == STRUCT_AND && append) {
				v_subs.push_back(o);
			} else {
				transform(STRUCT_AND, o);
			}
			break;
		}
		case OPERATION_OR: {
			if(m_type == STRUCT_OR && append) {
				v_subs.push_back(o);
			} else {
				transform(STRUCT_OR, o);
			}
			break;
		}
		case OPERATION_XOR: {
			if(m_type == STRUCT_XOR && append) {
				v_subs.push_back(o);
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
				MathStructure o2(v_subs[1]);
				o2.add(o, op);
				transform(STRUCT_AND, o2);
			} else if(append && m_type == STRUCT_AND && v_subs[v_subs.size() - 1].type() == STRUCT_COMPARISON) {
				MathStructure o2(v_subs[v_subs.size() - 1][1]);
				o2.add(o, op);
				v_subs.push_back(o2);
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
		v_subs.push_back(o);
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
		v_subs.push_back(o);
	} else {
		transform(STRUCT_MULTIPLICATION, o);
	}
}
void MathStructure::divide(const MathStructure &o, bool append) {
	transform(STRUCT_DIVISION, o);
//	MathStructure o2(o);
//	o2.inverse();
//	multiply(o2, append);
}
void MathStructure::raise(const MathStructure &o) {
	transform(STRUCT_POWER, o);
}
void MathStructure::negate() {
	transform(STRUCT_NEGATE);
}
void MathStructure::inverse() {
	transform(STRUCT_INVERSE);
}
void MathStructure::setNOT() {
	transform(STRUCT_NOT);
}		

bool MathStructure::equals(const MathStructure &o) const {
	if(m_type != o.type()) return false;
	if(v_subs.size() != o.size()) return false;
	switch(m_type) {
		case STRUCT_SYMBOLIC: {return s_sym == o.symbol();}
		case STRUCT_NUMBER: {return o_number.equals(o.number());}
		case STRUCT_COMPARISON: {return false;}
		case STRUCT_FUNCTION: {return false;}
	}
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(!v_subs[i].equals(o[i])) return false;
	}
	return true;
}		
int MathStructure::compare(const MathStructure &o) const {
	if(m_type != o.type()) return -2;
	if(v_subs.size() != o.size()) return -2;
	switch(m_type) {
		case STRUCT_SYMBOLIC: {if(s_sym == o.symbol()) return 0; return -2;}
		case STRUCT_NUMBER: {return o_number.compare(o.number());}
		case STRUCT_COMPARISON: {return -2;}
		case STRUCT_FUNCTION: {return -2;}
	}
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(!v_subs[i].equals(o[i])) return -2;
	}
	return -2;
}		

int MathStructure::merge_addition(const MathStructure &mstruct) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		if(o_number.add(mstruct.number())) return 1;
		return -1;
	}
	if(isZero()) {
		set(mstruct);
		return 1;
	}
	if(mstruct.isZero()) {
		return 1;
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
			switch(mstruct.type()) {
				case STRUCT_ADDITION: {
					for(unsigned int i = 0; i < mstruct.size(); i++) {
						v_subs.push_back(mstruct[i]);
					}
					return 1;
				}
				default: {
					v_subs.push_back(mstruct);
					return 1;
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			switch(mstruct.type()) {
				case STRUCT_ADDITION: {
					return 0;
				}
				case STRUCT_MULTIPLICATION: {
					unsigned int i1 = 0, i2 = 0;
					bool b = true;
					if(v_subs[0].isNumber()) i1 = 1;
					if(mstruct[0].isNumber()) i2 = 1;
					if(v_subs.size() - i1 == mstruct.size() - i2) {
						for(unsigned int i = i1; i < v_subs.size(); i++) {
							if(v_subs[i] != mstruct[i + i2 - i1]) {
								b = false;
								break;
							}
						}
						if(b) {
							if(i1 == 0) {
								v_subs.push_back(1);
							}
							if(i2 == 0) {
								v_subs[0].number() += 1;
							} else {
								v_subs[0].number() += mstruct[0].number();
							}
							return 1;
						}
					}
					break;
				}
				default: {
					if(v_subs.size() == 2 && v_subs[0].isNumber() && v_subs[1] == mstruct) {
						v_subs[0].number() += 1;
						return 1;
					}
				}					
			}
			break;
		}
		default: {
			switch(mstruct.type()) {
				case STRUCT_ADDITION: {}
				case STRUCT_MULTIPLICATION: {
					return 0;
				}
				default: {
					if(equals(mstruct)) {
						multiply(2);
						return 1;
					}
				}
			}	
		}		
	}
	return -1;
}

int MathStructure::merge_multiplication(const MathStructure &mstruct) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		if(o_number.multiply(mstruct.number())) return 1;
		return -1;
	}
	if(isZero() || mstruct.isZero()) {
		clear(); 
		return 1;
	} else if(mstruct.isOne()) {
		return 1;
	} else if(isOne()) {
		set(mstruct);
		return 1;
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
			switch(mstruct.type()) {
				case STRUCT_ADDITION: {
					MathStructure msave(*this);
					v_subs.clear();
					for(unsigned int i = 0; i < mstruct.size(); i++) {
						v_subs.push_back(msave);
						v_subs[i] *= mstruct[i];
					}
					return 1;
				}
				default: {
					for(unsigned int i = 0; i < v_subs.size(); i++) {
						v_subs[i].multiply(mstruct);
					}
					return 1;
				}
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			switch(mstruct.type()) {
				case STRUCT_ADDITION: {
					return 0;
				}
				case STRUCT_MULTIPLICATION: {
					for(unsigned int i = 0; i < mstruct.size(); i++) {
						v_subs.push_back(mstruct[i]);
					}
					return 1;
				}
				default: {
					break;
				}
			}
			break;
		}
		case STRUCT_POWER: {
			switch(mstruct.type()) {
				case STRUCT_ADDITION: {}
				case STRUCT_MULTIPLICATION: {
					return 0;
				}
				case STRUCT_POWER: {
					if(v_subs[1].isNumber() && mstruct[1].isNumber() && mstruct[0] == v_subs[0]) {
						v_subs[1] += mstruct[1];
					} else if(v_subs[0].isNumber() && mstruct.v_subs[0].isNumber() && mstruct.v_subs[1].equals(v_subs[1])) {
						v_subs[0] *= mstruct[0];
					}
					break;
				}
				default: {
					if(v_subs[1].isNumber() && v_subs[0] == mstruct) {
						v_subs[1] += 1;
						return 1;
					}
					break;
				}
			}
			break;
		}
		default: {
			switch(mstruct.type()) {
				case STRUCT_ADDITION: {}
				case STRUCT_MULTIPLICATION: {}
				case STRUCT_POWER: {
					return 0;
				}
				default: {
					if(equals(mstruct)) {
						raise(2);
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

int MathStructure::merge_power(const MathStructure &mstruct) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		if(o_number.raise(mstruct.number())) return 1;
		return -1;
	}
	if(mstruct.isOne()) {
		return 1;
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
			if(mstruct.isNumber() && mstruct.number().isInteger()) {			
				MathStructure mstruct1(v_subs[0]);
				MathStructure mstruct2(v_subs[1]);
				for(unsigned int i = 2; i < v_subs.size(); i++) {
					mstruct2.add(v_subs[i], true);
				}
				Number m(mstruct.number());
				Number k(1);
				Number p1(m);
				Number p2(1);
				p1 -= 1;
				Number bn;
				v_subs.clear();
				v_subs.push_back(mstruct1);
				v_subs[0].raise(mstruct);
				while(k.isLessThan(m)) {
					bn.binomial(m, k);
					v_subs.push_back(bn);
					v_subs[v_subs.size() - 1].multiply(mstruct1);
					if(!p1.isOne()) {
						v_subs[v_subs.size() - 1][1].raise(p1);
					}
					v_subs[v_subs.size() - 1].multiply(mstruct2, true);
					if(!p2.isOne()) {
						v_subs[v_subs.size() - 1][2].raise(p2);
					}
					k += 1;
					p1 -= 1;
					p2 += 1;
				}
				v_subs.push_back(mstruct2);
				v_subs[v_subs.size() - 1].raise(mstruct);
				return 1;
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if(mstruct.representsInteger()) {
				for(unsigned int i = 0; i < v_subs.size(); i++) {
					v_subs[i].raise(mstruct);	
				}
				return 1;
			}
			goto default_power_merge;
		}
		case STRUCT_POWER: {
			if(mstruct.representsInteger() || v_subs[0].representsReal()) {
				if(v_subs[1].isNumber() && v_subs[1].number().isRational()) {
					if(v_subs[1].number().numeratorIsEven()) {
						MathStructure mstruct_base(v_subs[0]);
						v_subs[0].set(CALCULATOR->f_abs, &mstruct_base, NULL);
						v_subs[0].calculateFunctions(default_evaluation_options);
					}
					v_subs[1] *= mstruct;
					return 1;
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
					v_subs.push_back(msave);
					v_subs[v_subs.size() - 1].raise(mstruct[i]);
				}
				return 1;
			}
			break;
		}
	}
	return -1;
}

/*bool MathStructure::merge(const MathStructure &mstruct, MathOperation op) {
	if(mstruct.type() == STRUCT_NUMBER && m_type == STRUCT_NUMBER) {
		return o_number->add(op, mstruct.number());
	}
	if(op == OPERATION_ADD) {
		if(mstruct.isNull()) {
			return true;
		}
		if(isNull()) {
			set(mstruct);
			return true;
		}		
		switch(m_type) {
			case STRUCT_ADDITION: {
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {
						for(unsigned int i = 0; i < mstruct.size(); i++) {
							v_subs.push_back(mstruct[i]);
						}
						return true;
					}
					default: {
						v_subs.push_back(mstruct);
						return true;
					}
				}
				break;
			}
			case STRUCT_MULTIPLICATION: {
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {
						if(!reverseadd(mstruct, op, translate_)) {
							return false;
						}
						break;
					}
					case STRUCT_MULTIPLICATION: {
						bool b = false;
						if(CALCULATOR->multipleRootsEnabled() && containsType(STRUCT_POWER) && mstruct.containsType(STRUCT_POWER)) {
							if(compatible(mstruct)) {
								MathStructure num1(1, 1);
								bool even_den = false;
								bool nonnum = false;
								for(unsigned int i = 0; i < mstructs.size(); i++) {
									if(mstructs[i]->isNumber()) {
										num1.add(mstructs[i], OPERATION_MULTIPLY);
									} else if(mstructs[i]->isPower()) {
										if(!mstructs[i]->exponent()->isNumber() || mstructs[i]->exponent()->number()->isComplex()) {
											nonnum = true;
											break;
										} else if(!even_den && mstructs[i]->exponent()->number()->denominatorIsEven()) {
											even_den = true;
										}
									}
								}
								if(nonnum) {
									if(!translate_) {
										return false;
									}
									transform(mstruct, STRUCT_ADDITION, op);
									break;
								}
								MathStructure num2(1, 1);
								for(unsigned int i = 0; i < mstructs.size(); i++) {
									if(mstruct.mstructs[i]->isNumber()) {
										num2.add(mstruct.mstructs[i], OPERATION_MULTIPLY);
									}
								}
								add(&num1, OPERATION_DIVIDE);
								if(even_den) {	
									MathStructure mstruct_bak(this);
									num1.number()->setNegative(false);
									num2.number()->setNegative(false);
									if(num1.equals(&num2)) {
										num1.add(&num2, OPERATION_ADD);
										add(&num1, OPERATION_MULTIPLY);
										MathStructure mstruct2(&mstruct_bak);
										num1.addInteger(-1, OPERATION_MULTIPLY);
										mstruct2.add(&num1, OPERATION_MULTIPLY);
										addAlternative(&mstruct2);
										mstruct2.clear();
										addAlternative(&mstruct2);
									} else {
										MathStructure num_temp(&num1);
										num_temp.add(&num2, OPERATION_ADD);
										add(&num_temp, OPERATION_MULTIPLY);
										MathStructure mstruct2(&mstruct_bak);
										num_temp.addInteger(-1, OPERATION_MULTIPLY);
										mstruct2.add(&num_temp, OPERATION_MULTIPLY);
										addAlternative(&mstruct2);
										mstruct2.set(&mstruct_bak);
										num_temp.set(&num1);
										num_temp.addInteger(-1, OPERATION_MULTIPLY);
										num_temp.add(&num2, OPERATION_ADD);
										mstruct2.add(&num_temp, OPERATION_MULTIPLY);
										addAlternative(&mstruct2);
										mstruct2.set(&mstruct_bak);
										num_temp.addInteger(-1, OPERATION_MULTIPLY);
										mstruct2.add(&num_temp, OPERATION_MULTIPLY);
										addAlternative(&mstruct2);
									}
								} else {
									num1.add(&num2, OPERATION_ADD);
									add(&num1, OPERATION_MULTIPLY);
								}
								break;
							}
						} else if(compatible(mstruct)) {
							for(unsigned int i = 0; i < mstructs.size(); i++) {
								if(mstructs[i]->isNumber()) {
									for(unsigned int i2 = 0; i2 < mstruct.mstructs.size(); i2++) {
										if(mstruct.mstructs[i2]->isNumber()) {
											mstructs[i]->add(mstruct.mstructs[i2], op);
											b = true;
											break;
										}
									}
									if(!b) {
										mstructs[i]->addInteger(1, op);
										b = true;
									}
									if(mstructs[i]->isNull()) {
										clear();
									} else if(mstructs[i]->isOne()) {
										mstructs[i]->unref();
										mstructs.erase(mstructs.begin() + i);
										if(mstructs.size() == 1) {
											moveto(mstructs[0]);
										}
									}
									break;
								}
							}
							if(!b) {
								for(unsigned int i2 = 0; i2 < mstruct.mstructs.size(); i2++) {
									if(mstruct.mstructs[i2]->isNumber()) {
										if(mstruct.mstructs[i2]->number()->isMinusOne()) {
											clear();
										} else {
											push_back(new MathStructure(1, 1));
											mstructs[mstructs.size() - 1]->add(mstruct.mstructs[i2], op);
										}
										b = true;
										break;
									}
								}
							}
							if(!b) {
								push_back(new MathStructure(2, 1));
							}
							break;							
						}
						if(!translate_) {
							return false;
						}
						transform(mstruct, STRUCT_ADDITION, op);
						break;
					}
					case STRUCT_POWER: {
						if(CALCULATOR->multipleRootsEnabled() && containsType(STRUCT_POWER)) {
							if(mstruct.mstructs[1]->isNumber() && !mstruct.mstructs[1]->number()->isComplex() && compatible(mstruct)) {
								if(mstruct.mstructs[1]->number()->denominatorIsEven()) {
									bool b = false;
									for(unsigned int i = 0; i < mstructs.size(); i++) {
										if(mstructs[i]->isNumber()) {
											MathStructure mstruct_num(mstructs[i]);
											mstructs[i]->unref();
											mstructs.erase(mstructs.begin());
											if(mstructs.size() == 1) {
												moveto(mstructs[0]);
											}
											MathStructure mstruct_bak(this);
											MathStructure mstruct_num2(&mstruct_num);
											mstruct_num2.addInteger(1, op);
											if(mstruct_num2.isZero()) {
												clear();
											} else if(!mstruct_num2.isOne()) {
												add(&mstruct_num2, OPERATION_MULTIPLY);
											}
											MathStructure mstruct2(&mstruct_bak);
											mstruct_num2.set(&mstruct_num);
											mstruct_num2.addInteger(-1, op);
											if(mstruct_num2.isZero()) {
												mstruct2.clear();
											} else if(!mstruct_num2.isOne()) {
												mstruct2.add(&mstruct_num2, OPERATION_MULTIPLY);	
											}
											addAlternative(&mstruct2);
											mstruct2.set(&mstruct_bak);
											mstruct_num.addInteger(-1, OPERATION_MULTIPLY);
											mstruct_num2.set(&mstruct_num);
											mstruct_num2.addInteger(1, op);
											if(mstruct_num2.isZero()) {
												mstruct2.clear();
											} else if(!mstruct_num2.isOne()) {
												mstruct2.add(&mstruct_num2, OPERATION_MULTIPLY);	
											}
											addAlternative(&mstruct2);
											mstruct2.set(&mstruct_bak);
											mstruct_num2.set(&mstruct_num);
											mstruct_num2.addInteger(-1, op);
											if(mstruct_num2.isZero()) {
												mstruct2.clear();
											} else if(!mstruct_num2.isOne()) {
												mstruct2.add(&mstruct_num2, OPERATION_MULTIPLY);	
											}
											addAlternative(&mstruct2);
											b = true;
											break;
										}
									}
									if(!b) {
										if(!translate_) {
											return false;
										}
										transform(mstruct, STRUCT_ADDITION, op);
									}
									break;
								}
							} else {
								if(!translate_) {
									return false;
								}
								transform(mstruct, STRUCT_ADDITION, op);
								break;
							}
						}
					}
					case UNIT_MANAGER: {
					}
					case STRUCT_FUNCTION: {
					}
					case STRUCT_VARIABLE: {
					}
					case STRING_MANAGER: {
						if(compatible(mstruct)) {
							bool b = false;
							for(unsigned int i = 0; i < mstructs.size(); i++) {
								if(mstructs[i]->isNumber()) {
									mstructs[i]->addInteger(1, op);
									if(mstructs[i]->number()->isZero()) {
										clear();
									} else if(mstructs[i]->number()->isOne()) {
										mstructs[i]->unref();
										mstructs.erase(mstructs.begin());
										if(mstructs.size() == 1) {
											moveto(mstructs[0]);
										}
									}
									b = true;
									break;
								}
							}
							if(!b) {
								push_back(new MathStructure(2, 1));
							}
							break;
						}					
					}					
					default: {
						if(!translate_) {
							return false;
						}
						transform(mstruct, STRUCT_ADDITION, op);
						break;
					}
				}
				break;
			}
			case STRUCT_NUMBER: {
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {
						if(!reverseadd(mstruct, op, translate_)) {
							return false;
						}
						break;
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mstruct, STRUCT_ADDITION, op, true);
						break;
					}					
				}
				break;
			}
			case STRUCT_POWER: {
				if(CALCULATOR->multipleRootsEnabled() && mstruct.isPower()) {
					if(mstructs[1]->isNumber() && !mstructs[1]->number()->isComplex() && equals(mstruct)) {
						if(mstructs[1]->number()->denominatorIsEven()) {
							MathStructure *mstruct2 = new MathStructure(this);
							clear();
							push_back(new MathStructure(2, 1));
							push_back(mstruct2);
							m_type = STRUCT_MULTIPLICATION;
							MathStructure mstruct3(this);
							mstruct3.addInteger(-1, OPERATION_MULTIPLY);
							addAlternative(&mstruct3);
							mstruct3.clear();
							addAlternative(&mstruct3);
							break;
						} else {
							MathStructure *mstruct2 = new MathStructure(this);
							clear();
							push_back(new MathStructure(2, 1));
							push_back(mstruct2);
							m_type = STRUCT_MULTIPLICATION;
							break;
						}
					}
					if(!translate_) {
						return false;
					}
					transform(mstruct, STRUCT_ADDITION, op);
					break;
				}
			}
			default: {
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {}
					case STRUCT_MULTIPLICATION: {
						if(!reverseadd(mstruct, op, translate_)) {
							return false;
						}
						break;
					}
					default: {
						if(equals(mstruct)) {
							MathStructure *mstruct2 = new MathStructure(this);
							clear();
							push_back(new MathStructure(2, 1));
							push_back(mstruct2);
							m_type = STRUCT_MULTIPLICATION;
							break;
						}
						if(!translate_) {
							return false;
						}
						transform(mstruct, STRUCT_ADDITION, op);
					}
				}	
			}		
		}
	} else if(op == OPERATION_MULTIPLY) {
		if(isNull() || mstruct.isNull()) {
			clear(); 
			return true;
		} else if(mstruct.isOne()) {
			return true;
		} else if(isOne()) {
			set(mstruct);
			return true;
		}
		if(mstruct.isPower() && !isNumber()) {
			if(mstruct.base()->isAddition() && mstruct.exponent()->isNumber() && mstruct.exponent()->number()->isMinusOne()) {
				MathStructure *mstruct2 = mstruct.base();
				MathStructure *prev_base = NULL, *this_base = NULL;
				bool poly = true;
				if(mstruct2->countChilds() < 2 || mstruct2->getChild(0)->isNumber()) {
					poly = false;
				} else {
					for(unsigned int i = 0; i < mstruct2->countChilds(); i++) {
						if(mstruct2->getChild(i)->isNumber()) {
							prev_base = NULL;
							this_base = NULL;
						} else if(mstruct2->getChild(i)->isPower()) {
							this_base = mstruct2->getChild(i)->base();
						} else if(mstruct2->getChild(i)->isMultiplication() && mstruct2->getChild(i)->countChilds() > 1 && mstruct2->getChild(i)->getChild(0)->isNumber()) {
							if(mstruct2->getChild(i)->countChilds() == 2) {
								if(mstruct2->getChild(i)->getChild(1)->isPower()) {
									this_base = mstruct2->getChild(i)->getChild(1)->base();
								} else {
									this_base = mstruct2->getChild(i)->getChild(1);
								}
							} else {
								this_base = mstruct2->getChild(i);
							}
						} else {
							this_base = mstruct2->getChild(i);
						}
						if(prev_base && prev_base->isMultiplication() && this_base->isMultiplication()) {
							int first1 = 0, first2 = 0;
							if(prev_base->countChilds() > 0 && prev_base->getChild(0)->isNumber()) {
								first1 = 1;
							}
							if(this_base->countChilds() > 0 && this_base->getChild(0)->isNumber()) {
								first2 = 1;
							}
							if(prev_base->countChilds() - first1 != this_base->countChilds() - first2) {
								poly = false;
								break;
							}
							for(unsigned int i2 = first1; i2 < prev_base->countChilds(); i2++) {
								if(prev_base->getChild(i2)->isPower()) {
									if(this_base->getChild(i2 - first1 + first2)->isPower()) {
										if(!prev_base->getChild(i2)->base()->equals(this_base->getChild(i2 - first1 + first2)->base())) {
											poly = false;
											break;
										}
									} else {
										if(!prev_base->getChild(i2)->base()->equals(this_base->getChild(i2 - first1 + first2))) {
											poly = false;
											break;
										}
									}
								} else {
									if(this_base->getChild(i2 - first1 + first2)->isPower()) {
										if(!prev_base->getChild(i2)->equals(this_base->getChild(i2 - first1 + first2)->base())) {
											poly = false;
											break;
										}
									} else {
										if(!prev_base->getChild(i2)->equals(this_base->getChild(i2 - first1 + first2))) {
											poly = false;
											break;
										}
									}
								}
							}
							if(!poly) break;
						} else if(prev_base && !this_base->compatible(prev_base)) {
							poly = false;
							break;
						}
						prev_base = this_base;
					}
				}
				//polynomial division
				MathStructure div;
				MathStructure ans;
				MathStructure rem;
				MathStructure *cur_mstruct;
				bool b2 = false;
				unsigned int i = 0;
				while(poly) {
					if(isAddition()) {
						if(i < mstructs.size()) {
							cur_mstruct = mstructs[i];
							i++;
						} else {
							break;
						}
					} else {
						if(i > 0) {
							break;
						}
						i++;
						cur_mstruct = this;
					}
					bool b = false;
					if(rem.isAddition()) {
						for(unsigned int i2 = 0; i2 < rem.countChilds(); i2++) {
							div.set(cur_mstruct);
							div.add(rem.getChild(i2), OPERATION_SUBTRACT);
							if(!div.isAddition()) {
								cur_mstruct.set(&div);
								rem.getChild(i2)->clear();
							}
						}
						rem.typeclean();
					} else if(!rem.isZero()) {
						div.set(cur_mstruct);
						div.add(&rem, OPERATION_SUBTRACT);
						if(!div.isAddition()) {
							cur_mstruct.set(&div);
							rem.clear();
						}
					}
					for(unsigned int i2 = 0; i2 < mstruct2->countChilds(); i2++) {
						b = true;
						if(!mstruct2->getChild(i2)->isNumber()) {
							div.set(cur_mstruct);
							div.add(mstruct2->getChild(i2), OPERATION_DIVIDE);
							if(div.isMultiplication()) {
								for(unsigned int i3 = 0; i3 < div.countChilds(); i3++) {
									if(div.getChild(i3)->isPower() && (!div.getChild(i3)->exponent()->isNumber() || !div.getChild(i3)->exponent()->number()->isPositive() || !div.getChild(i3)->exponent()->number()->isInteger())) {
										b = false;
										break;
									}
								}
							} else if(div.isPower()) {
							 	if(!div.exponent()->isNumber() || !div.exponent()->number()->isPositive() || !div.exponent()->number()->isInteger()) {
									b = false;
								}
							} else if(div.isAddition()) {
								b = false;
							}
							if(b) {
								b2 = true;
								ans.add(&div, OPERATION_ADD);
								div.add(mstruct2, OPERATION_MULTIPLY);
								div.add(cur_mstruct, OPERATION_SUBTRACT);
								rem.add(&div, OPERATION_ADD);
								break;
							} else {
								break;
							}
						}
					}
					if(!b) {
						rem.add(cur_mstruct, OPERATION_SUBTRACT);
					}
				}
				if(b2) {
					rem.add(mstruct2, OPERATION_DIVIDE);
					set(&ans);
					add(&rem, OPERATION_SUBTRACT);
					sort();
					return true;
				}
			}
		}
		switch(m_type) {
			case STRUCT_ADDITION: {
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {
						MathStructure *mstruct3 = new MathStructure(this);
						clear();
						for(unsigned int i = 0; i < mstruct.mstructs.size(); i++) {
							MathStructure *mstruct2 = new MathStructure(mstruct3);
							mstruct2->add(mstruct.mstructs[i], op);
							add(mstruct2, OPERATION_ADD);
							mstruct2->unref();
						}
						mstruct3->unref();
						break;
					}
					default: {
						for(unsigned int i = 0; i < mstructs.size(); i++) {
							mstructs[i]->add(mstruct, op);
						}
						break;
					}
				}
				typeclean();
				break;
			}
			case STRUCT_MULTIPLICATION: {
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {
						if(!reverseadd(mstruct, op, translate_)) {
							return false;
						}
						break;
					}
					case STRUCT_MULTIPLICATION: {
						for(unsigned int i = 0; i < mstruct.mstructs.size(); i++) {
							add(mstruct.mstructs[i], op);
						}
						typeclean();
						break;
					}
					default: {
						MathStructure *mstruct2 = new MathStructure(mstruct);
						push_back(mstruct2);
						typeclean();
						break;
					}
				}
				break;
			}
			case STRUCT_POWER: {
				switch(mstruct.type()) {
					case STRUCT_MULTIPLICATION: {}
					case STRUCT_ADDITION: {
						if(!reverseadd(mstruct, op, translate_)) {
							return false;
						}
						break;
					}
					case STRUCT_POWER: {
						if(CALCULATOR->multipleRootsEnabled() && mstructs[1]->isNumber() && mstruct.mstructs[1]->isNumber() && mstructs[1]->number()->denominatorIsEven() && mstruct.mstructs[1]->number()->denominatorIsEven() && mstruct.mstructs[0]->equals(mstructs[0])) {
							mstructs[1]->add(mstruct.mstructs[1], OPERATION_ADD);
							if(mstructs[1]->isNull()) {
								set(1, 1);
							} else if(mstructs[1]->isOne()) {
								moveto(mstructs[0]);
							}
							if(!isPower() || !mstructs[1]->number()->denominatorIsEven()) {
								MathStructure mstruct2(this);
								mstruct2.addInteger(-1, OPERATION_MULTIPLY);
								addAlternative(&mstruct2);
							}
						} else if((!CALCULATOR->multipleRootsEnabled() || (mstructs[1]->isNumber() && !mstructs[1]->number()->isComplex() && mstruct.mstructs[1]->isNumber() && !mstruct.mstructs[1]->number()->isComplex())) && mstruct.mstructs[0]->equals(mstructs[0])) {
							mstructs[1]->add(mstruct.mstructs[1], OPERATION_ADD);
							if(mstructs[1]->isNull()) {
								set(1, 1);
							} else if(mstructs[1]->isOne()) {
								moveto(mstructs[0]);
							}
						} else if(mstructs[0]->isNumber() && mstruct.mstructs[0]->isNumber() && mstruct.mstructs[1]->equals(mstructs[1])) {
							mstructs[0]->add(mstruct.mstructs[0], OPERATION_MULTIPLY);
						} else {
							if(!translate_) {
								return false;
							}
							transform(mstruct, STRUCT_MULTIPLICATION, op);
						}
						break;
					}
					default: {
						if(mstruct.equals(mstructs[0])) {
							mstructs[1]->addInteger(1, OPERATION_ADD);
							if(mstructs[1]->isNull()) {
								set(1, 1);
							} else if(mstructs[1]->isOne()) {
								moveto(mstructs[0]);
							}
						} else {
							if(!translate_) {
								return false;
							}
							transform(mstruct, STRUCT_MULTIPLICATION, op);
						}
						break;
					}
				}
				break;
			}
			case STRUCT_NUMBER: {
				if(o_number->isOne()) {set(mstruct); return true;}
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {}
					case STRUCT_MULTIPLICATION: {}
					case STRUCT_POWER: {
						if(!reverseadd(mstruct, op, translate_)) {
							return false;
						}
						break;
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mstruct, STRUCT_MULTIPLICATION, op, true);
						break;
					}
				}
				break;
			}
			default: {
				switch(mstruct.type()) {
					case STRUCT_ADDITION: {}
					case STRUCT_MULTIPLICATION: {}
					case STRUCT_POWER: {
						if(!reverseadd(mstruct, op, translate_)) {
							return false;
						}
						break;
					}
					default: {
						if(equals(mstruct)) {
							MathStructure *mstruct2 = new MathStructure(this);
							clear();
							push_back(mstruct2);
							push_back(new MathStructure(2, 1));
							m_type = STRUCT_POWER;
							break;
						}
						if(!translate_) {
							return false;
						}
						transform(mstruct, STRUCT_MULTIPLICATION, op);
						break;
					}
				}
			}			
		}
	} else if(op == OPERATION_RAISE) {
		if(mstruct.isNull()) {
			set(1, 1);
			return true;
		} else if(mstruct.isOne()) {
			return true;
		} else if(isZero() && mstruct.isNumber() && mstruct.number()->isNegative()) {
			if(translate_) {CALCULATOR->error(true, _("Division by zero."), NULL);}
		}
		bool b_trans = false;
		if((m_type == STRUCT_ADDITION || CALCULATOR->multipleRootsEnabled()) && mstruct.isNumber() && !mstruct.number()->isInteger() && !mstruct.number()->isComplex()) {
			Number *num = mstruct.number()->numerator();
			if(!num->isOne() && (CALCULATOR->multipleRootsEnabled() || !num->isMinusOne())) {
				MathStructure mstruct2(num);
				bool b = add(&mstruct2, op, translate_);
				Number *den = mstruct.number()->denominator();
				den->recip();
				mstruct2.set(den);
				if(!b) {
					b = add(&mstruct2, op, translate_);
				} else {
					add(&mstruct2, op);
				}
				
				delete den;
				return b;
			}
			delete num;
		}
		switch(m_type) {
			case STRUCT_MULTIPLICATION: {
				for(unsigned int i = 0; i < mstructs.size(); i++) {
					mstructs[i]->add(mstruct, op);
				}
				typeclean();
				break;
			}		
			case STRUCT_ADDITION: {
				switch(mstruct.type()) {
					case STRUCT_NUMBER: {
						if(mstruct.number()->isInteger() && !mstruct.number()->isMinusOne()) {
							Number n(mstruct.number());
							n.setNegative(false);*/
							/*MathStructure *mstruct2 = new MathStructure(this);
							n->add(-1);
							for(; n->isPositive(); n->add(-1)) {
								add(mstruct2, OPERATION_MULTIPLY);
							}*/
/*							MathStructure n_mstruct(&n);
							Number bn;
							MathStructure bn_mstruct;
							Number i(&n);
							bool b_even = i.isEven();
							Number n_two(2, 1);
							i.trunc(&n_two);
							MathStructure second_mstruct;
							if(mstructs.size() == 2) {
								second_mstruct.set(mstructs[1]);
							} else {
								second_mstruct.setType(STRUCT_ADDITION);
								for(unsigned int i = 1; i < mstructs.size(); i++) {
									mstructs[i]->ref();
									second_mstruct.push_back(mstructs[i]);
								}
							}
							MathStructure *mstruct_new = new MathStructure(mstructs[0]);
							mstruct_new->add(&n_mstruct, OPERATION_RAISE);
							MathStructure *mstruct2 = new MathStructure(&second_mstruct);
							mstruct2->add(&n_mstruct, OPERATION_RAISE);
							mstruct_new->add(mstruct2, OPERATION_ADD);
							Number i2(1, 1);
							Number n2(&n);
							n2.add(-1);
							n_mstruct.set(&n2);
							MathStructure mstruct_a, mstruct_b, i_mstruct(&i2);
							int cmp = i2.compare(&i);
							while(cmp >= 0) {
								bn.binomial(&n, &i2);
								bn_mstruct.set(&bn);
								mstruct_a.set(mstructs[0]);
								mstruct_a.add(&n_mstruct, OPERATION_RAISE);
								bn_mstruct.add(&mstruct_a, OPERATION_MULTIPLY);
								mstruct_b.set(&second_mstruct);
								if(!i2.isOne()) {
									mstruct_b.add(&i_mstruct, OPERATION_RAISE);
								}
								bn_mstruct.add(&mstruct_b, OPERATION_MULTIPLY);
								mstruct_new->add(&bn_mstruct, OPERATION_ADD);
								
								if(!b_even || cmp != 0) {
									bn_mstruct.set(&bn);
									mstruct_a.set(&second_mstruct);
									mstruct_a.add(&n_mstruct, OPERATION_RAISE);
									bn_mstruct.add(&mstruct_a, OPERATION_MULTIPLY);
									mstruct_b.set(mstructs[0]);
									if(!i2.isOne()) {
										mstruct_b.add(&i_mstruct, OPERATION_RAISE);
									}
									bn_mstruct.add(&mstruct_b, OPERATION_MULTIPLY);
									mstruct_new->add(&bn_mstruct, OPERATION_ADD);
								}
								i2.add(1);
								cmp = i2.compare(&i);
								if(cmp >= 0) {
									i_mstruct.set(&i2);
									n2.add(-1);
									n_mstruct.set(&n2);
								}
							}
							moveto(mstruct_new);
							mstruct_new->unref();
							if(mstruct.number()->isNegative()) {
								//mstruct2->unref();
								mstruct2 = new MathStructure(1, 1);
								mstruct2->add(this, OPERATION_DIVIDE);
								moveto(mstruct2);
								mstruct2->unref();
							}
							break;
						}
						b_trans = true;
						break;						
					}
					default: {
						b_trans = true;
						break;
					}
				}
				break;
			}
			case STRUCT_POWER: {
				if(!CALCULATOR->multipleRootsEnabled() || (mstruct.isNumber() && mstruct.number()->isMinusOne()) || (mstructs[1]->isNumber() && mstructs[1]->number()->isInteger() && mstruct.isNumber() && mstruct.number()->isInteger())) {
					if(mstructs[0]->add(mstruct, OPERATION_RAISE, false)) {
						mstructs[0]->add(mstructs[1], OPERATION_RAISE);
						moveto(mstructs[0]);
						break;
					}
				}
				mstructs[1]->add(mstruct, OPERATION_MULTIPLY);
				if(mstructs[1]->isNull()) {
					set(1, 1);
				} else if(mstructs[1]->isNumber() && mstructs[0]->isNumber()) {
					mstructs[0]->add(mstructs[1], OPERATION_RAISE);
					moveto(mstructs[0]);
				} else if(mstructs[1]->isOne()) {
					moveto(mstructs[0]);
				}
				break;
			}
			case STRUCT_NUMBER: {
				if(isNull()) {
					if(mstruct.isNumber() && mstruct.number()->isNegative()) {
						if(!translate_) {
							return false;
						}
						if(mstruct.number()->isMinusOne()) {
							transform(mstruct, STRUCT_POWER, op);
						} else {
							MathStructure *mstruct2 = new MathStructure(-1, 1);
							transform(mstruct2, STRUCT_POWER, op);
							mstruct2->unref();
						}
					}
					break;
				}
				if(o_number->isOne()) {
					return true;
				}
				b_trans = true;
				break;
			}
			case STRUCT_VARIABLE: {
				if(o_variable == CALCULATOR->getE()) {
					if(mstruct.isMultiplication() && mstruct.countChilds() == 2 && mstruct.getChild(1)->isVariable() && mstruct.getChild(1)->variable() == CALCULATOR->getPI()) {
						if(mstruct.getChild(0)->isNumber()) {
							if(mstruct.getChild(0)->number()->isI() || mstruct.getChild(0)->number()->isMinusI()) {
								set(-1, 1);
								break;
							} else if(mstruct.getChild(0)->number()->isComplex() && !mstruct.getChild(0)->number()->hasRealPart()) {
								Number *img = mstruct.getChild(0)->number()->imaginaryPart();
								if(img->isInteger()) {
									set(-1, 1);
									if(img->isEven()) {
										set(1, 1);
									}
									delete img;
									break;
								} else if(img->equals(1, 2) || img->equals(-1, 2)) {
									clear();
									img->set(1, 1);
									o_number->setImaginaryPart(img);
									delete img;
									break;
								}
								delete img;
							}
						}
					} else if(mstruct.isFunction() && mstruct.function() == CALCULATOR->getLnFunction() && mstruct.countChilds() == 1 && mstruct.getChild(0)->isNumber()) {
						if(mstruct.getChild(0)->number()->isPositive()) {
							set(mstruct.getChild(0));
							break;
						} else if(!mstruct.getChild(0)->number()->isZero() && mstruct.getChild(0)->number()->isComplex()) {
							set(mstruct.getChild(0));
							break;
						}
					}
				}
			}	
			default: {
				b_trans = true;
			}
		}
		if(b_trans) {
//			if(!CALCULATOR->multipleRootsEnabled() && mstruct.isMultiplication()) {
			if(mstruct.isMultiplication()) {
				bool b = false;
				unsigned int i;
				for(i = 0; i < mstruct.mstructs.size(); i++) {
					if(add(mstruct.mstructs[i], OPERATION_RAISE, false)) {
						b = true;
						break;
					}
				}
				if(b) {
					sort();
					if(mstruct.mstructs.size() == 2) {
						if(i == 0) {
							add(mstruct.mstructs[1], OPERATION_RAISE);
						} else {
							add(mstruct.mstructs[0], OPERATION_RAISE);
						}
					} else if(mstruct.mstructs.size() > 2) {
						MathStructure mstruct2;
						mstruct2.setType(STRUCT_MULTIPLICATION);
						for(unsigned int i2 = 0; i2 < mstruct.mstructs.size(); i2++) {
							if(i2 != i) {
								mstruct.mstructs[i2]->ref();
								mstruct2.push_back(mstruct.mstructs[i2]);
							}
						}
						add(&mstruct2, OPERATION_RAISE);
					}
					b_trans = false;
				}
//			} else if(!CALCULATOR->multipleRootsEnabled() && mstruct.isAddition()) {
			} else if(mstruct.isAddition()) {
				bool b = false;
				unsigned int i;
				MathStructure mstruct3(this);
				for(i = 0; i < mstruct.mstructs.size(); i++) {
					if(add(mstruct.mstructs[i], OPERATION_RAISE, false)) {
						b = true;
						break;
					}
				}
				if(b) {
					sort();
					if(mstruct.mstructs.size() == 2) {
						if(i == 0) {
							mstruct3.add(mstruct.mstructs[1], OPERATION_RAISE);
						} else {
							mstruct3.add(mstruct.mstructs[0], OPERATION_RAISE);
						}
					} else if(mstruct.mstructs.size() > 2) {
						MathStructure mstruct2;
						mstruct2.setType(STRUCT_ADDITION);
						for(unsigned int i2 = 0; i2 < mstruct.mstructs.size(); i2++) {
							if(i2 != i) {
								mstruct.mstructs[i2]->ref();
								mstruct2.push_back(mstruct.mstructs[i2]);
							}
						}
						mstruct3.add(&mstruct2, OPERATION_RAISE);
					} else {
						mstruct3.set(1, 1);
					}
					add(&mstruct3, OPERATION_MULTIPLY);
					b_trans = false;
				}
			}
			if(b_trans) {
				if(!translate_) {
					return false;
				}
				transform(mstruct, STRUCT_POWER, op);
			}
		}
	} else if(op == OPERATION_EQUALS || op == OPERATION_LESS || op == OPERATION_GREATER || op == OPERATION_NOT_EQUALS || op == OPERATION_EQUALS_LESS || op == OPERATION_EQUALS_GREATER) {
		int s = compare(mstruct);
		if(s > -2) {
			clear();
			switch(op) {
				case OPERATION_EQUALS: {o_number->setTrue(s == 0); break;}
				case OPERATION_LESS: {o_number->setTrue(s > 0); break;}
				case OPERATION_GREATER: {o_number->setTrue(s < 0); break;}
				case OPERATION_EQUALS_LESS: {o_number->setTrue(s >= 0); break;}
				case OPERATION_EQUALS_GREATER: {o_number->setTrue(s <= 0); break;}
				case OPERATION_NOT_EQUALS: {o_number->setTrue(s != 0); break;}
				default: {}
			}
		} else {
		
			if(!translate_) {
				return false;
			}
			
			MathStructure *mstruct2 = new MathStructure(this);	
			mstruct2->add(mstruct, OPERATION_SUBTRACT);
			clear();
			push_back(mstruct2);
			mstruct2 = new MathStructure();
			push_back(mstruct2);
			switch(op) {
				case OPERATION_EQUALS: {comparison_type = COMPARISON_EQUALS; break;}
				case OPERATION_LESS: {comparison_type = COMPARISON_LESS; break;}
				case OPERATION_GREATER: {comparison_type = COMPARISON_GREATER; break;}
				case OPERATION_EQUALS_LESS: {comparison_type = COMPARISON_EQUALS_LESS; break;}
				case OPERATION_EQUALS_GREATER: {comparison_type = COMPARISON_EQUALS_GREATER; break;}
				case OPERATION_NOT_EQUALS: {comparison_type = COMPARISON_NOT_EQUALS; break;}
				default: {}
			}
			m_type = STRUCT_COMPARISON;
		}
	} else if(op == OPERATION_OR || OPERATION_AND) {
		int p; 
		if((p = isPositive()) >= 0) {
			if(p && op == OPERATION_OR) {
				set(1, 1);
			} else if(!p && op == OPERATION_AND) {
				clear();
			} else if(!mstruct.isComparison() || !mstruct.getChild(1)->isZero() || mstruct.comparisonType() != COMPARISON_GREATER) {
				MathStructure *mstruct2 = new MathStructure(mstruct);
				clear();
				push_back(mstruct2);
				mstruct2 = new MathStructure();
				push_back(mstruct2);
				m_type = STRUCT_COMPARISON;
				comparison_type = COMPARISON_GREATER;
			}
		} else if((p = mstruct.isPositive()) >= 0) {
			if(p && op == OPERATION_OR) {
				set(1, 1);
			} else if(!p && op == OPERATION_AND) {
				clear();
			} else if(!isComparison() || !mstructs[1]->isZero() || comparison_type != COMPARISON_GREATER) {
				MathStructure *mstruct2 = new MathStructure(this);
				clear();
				push_back(mstruct2);
				mstruct2 = new MathStructure();
				push_back(mstruct2);
				m_type = STRUCT_COMPARISON;
				comparison_type = COMPARISON_GREATER;
			}		
		} else {
			bool b = false;
			if(isComparison() && mstructs[1]->isZero() && comparison_type == COMPARISON_GREATER) {
				moveto(mstructs[0]);
				b = true;
			} 
			if(mstruct.isComparison() && mstruct.getChild(1)->isZero() && mstruct.comparisonType() == COMPARISON_GREATER) {
				b = true;
				mstruct = mstruct.getChild(0);
			}
			if(!b && !translate_) {
				return false;
			}
			if((m_type == OR_MANAGER && op == OPERATION_OR) || (m_type == STRUCT_AND && op == OPERATION_AND)) {
				push_back(new MathStructure(mstruct));	
			} else if(op == OPERATION_OR) {
				if(mstruct.type() == OR_MANAGER) {
					transform(mstruct.getChild(0), OR_MANAGER, op);
					for(unsigned int i = 1; i < mstruct.countChilds(); i++) {
						push_back(new MathStructure(mstruct.getChild(i)));
					}
				} else {
					if(!translate_) {
						return false;
					}
					transform(mstruct, OR_MANAGER, op);
				}
			} else {
				if(mstruct.type() == STRUCT_AND) {
					transform(mstruct.getChild(0), STRUCT_AND, op);
					for(unsigned int i = 1; i < mstruct.countChilds(); i++) {
						push_back(new MathStructure(mstruct.getChild(i)));
					}
				} else {
					if(!translate_) {
						return false;
					}
					transform(mstruct, STRUCT_AND, op);
				}
			}
		}
	}
//	printf("PRESORT [%s]\n", print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION | DISPLAY_FORMAT_SCIENTIFIC).c_str());	
	sort();
//	printf("POSTSORT [%s]\n", print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION | DISPLAY_FORMAT_SCIENTIFIC).c_str());	
//	CALCULATOR->checkFPExceptions();
	return true;
}*/

void MathStructure::calculate() {
}
void MathStructure::calculatesub(int level) {
	bool b = true;
	while(b) {
		b = false;
		switch(m_type) {
			case STRUCT_VARIABLE: {
				if(o_variable->isKnown()) {
					set(((KnownVariable*) o_variable)->get());
					b = true;
				}
				break;
			}
			case STRUCT_POWER: {
				v_subs[0].calculatesub(level);
				v_subs[1].calculatesub(level);
				if(v_subs[0].merge_power(v_subs[1]) == 1) {
					MathStructure new_this(v_subs[0]);
					set(new_this);
					b = true;
				}
				break;
			}
			case STRUCT_NEGATE: {
				v_subs[0].calculatesub(level);
				if(v_subs[0].isNegate()) {
					MathStructure new_this(v_subs[0][0]);
					set(new_this);
					b = true;
				} else if(v_subs[0].isNumber()) {
					if(v_subs[0].number().negate()) {
						MathStructure new_this(v_subs[0]);
						set(new_this);
						b = true;
					}
					break;
				}
				break;
			}
			case STRUCT_INVERSE: {
				v_subs[0].calculatesub(level);
				if(v_subs[0].isInverse()) {
					MathStructure new_this(v_subs[0][0]);
					set(new_this);
					b = true;
				} else if(v_subs[0].isNumber()) {
					if(v_subs[0].number().recip()) {
						MathStructure new_this(v_subs[0]);
						set(new_this);
						b = true;
					}
					break;
				}
				break;
			}
			case STRUCT_ADDITION: {
				for(unsigned int i = 0; i < v_subs.size(); i++) {
					v_subs[i].calculatesub(level);
				}
				for(int i = 0; i < (int) v_subs.size() - 1; i++) {
					for(int i2 = i + 1; i2 < (int) v_subs.size(); i2++) {
						int r = v_subs[i].merge_addition(v_subs[i2]);
						if(r == 0) {
							r = v_subs[i2].merge_addition(v_subs[i]);
							if(r == 1) {
								v_subs[i] = v_subs[i2];
							}
						}
						if(r == 1) {
							v_subs.erase(v_subs.begin() + i2);
							i2--;
							b = true;
						}
					}
				}
				if(v_subs.size() == 1) {
					MathStructure new_this(v_subs[0]);
					set(new_this);
				} else if(v_subs.size() == 0) {
					clear();
				}
				break;
			}
			case STRUCT_MULTIPLICATION: {
				for(unsigned int i = 0; i < v_subs.size(); i++) {
					v_subs[i].calculatesub(level);
				}
				for(int i = 0; i < (int) v_subs.size() - 1; i++) {
					for(int i2 = i + 1; i2 < (int) v_subs.size(); i2++) {
						int r = v_subs[i].merge_multiplication(v_subs[i2]);
						if(r == 0) {
							r = v_subs[i2].merge_multiplication(v_subs[i]);
							if(r == 1) {
								v_subs[i] = v_subs[i2];
							}
						}
						if(r == 1) {
							v_subs.erase(v_subs.begin() + i2);
							i2--;
							b = true;
						}
					}
				}
				if(v_subs.size() == 1) {
					MathStructure new_this(v_subs[0]);
					set(new_this);
				} else if(v_subs.size() == 0) {
					clear();
				}
				break;
			}
			case STRUCT_DIVISION: {
				v_subs[0].calculatesub(level);
				for(unsigned int i = 1; i < v_subs.size(); i++) {
					v_subs[i].calculatesub(level);
					if(v_subs[i - 1].isNumber() && v_subs[i].isNumber()) {
						if(v_subs[i - 1].number().divide(v_subs[i].number())) {
							v_subs.erase(v_subs.begin() + i);
							i--;
							b = true;
						}
					}
				}
				if(v_subs.size() == 1) {
					MathStructure new_this(v_subs[0]);
					set(new_this);
				}
				break;
			}
			default: {
				for(unsigned int i = 0; i < v_subs.size(); i++) {
					v_subs[i].calculatesub(level);
				}
			}
		}
	}
}
void MathStructure::calculateFunctions(const EvaluationOptions &eo) {
	if(m_type == STRUCT_FUNCTION) {
		if(!o_function->testArgumentCount(v_subs.size())) {
			return;
		}
		if((int) v_subs.size() > o_function->maxargs()) {
			v_subs.resize(o_function->maxargs());
		}
		m_type = STRUCT_VECTOR;
		Argument *arg = NULL, *last_arg = NULL;
		int last_i = 0;
		for(unsigned int i = 0; i < v_subs.size(); i++) {
			arg = o_function->getArgumentDefinition(i + 1);
			if(arg) {
				last_arg = arg;
				last_i = i;
				if(!arg->test(v_subs[i], i + 1, o_function, eo)) {
					m_type = STRUCT_FUNCTION;
					return;
				}
			}
		}
		if(last_arg && o_function->maxargs() < 0 && last_i >= o_function->minargs()) {
			for(unsigned int i = last_i + 1; i < v_subs.size(); i++) {
				if(!last_arg->test(v_subs[i], i + 1, o_function, eo)) {
					m_type = STRUCT_FUNCTION;
					return;
				}
			}
		}
		if(!o_function->testCondition(*this)) {
			m_type = STRUCT_FUNCTION;
			return;
		}
		MathStructure mstruct;
		if(o_function->calculate(mstruct, *this, eo)) {
			set(mstruct);
			calculateFunctions(eo);
		} else {
			m_type = STRUCT_FUNCTION;
		}
	} else {
		for(unsigned int i = 0; i < v_subs.size(); i++) {
			v_subs[i].calculateFunctions(eo);
		}
	}
}

#ifdef HAVE_GIAC

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif

gen evalf_try_exact(const gen &g, int level=DEFAULT_EVAL_LEVEL,const context * contextptr = 0);

  gen check_not_assume_try_exact(const gen & not_evaled,const gen & evaled, bool evalf_after,int level,const context * contextptr){
    if ( (evaled.type==_VECT) && (evaled.subtype==_ASSUME__VECT) ){
      if ( evalf_after && evaled._VECTptr->size()==2 && evaled._VECTptr->back().type<=_CPLX )
	  return evalf_try_exact(evaled._VECTptr->back(),level,contextptr);
      if (not_evaled.type==_IDNT && evaled._VECTptr->size()==1 && evaled._VECTptr->front().type==_INT_){
	gen tmp=not_evaled;
	tmp.subtype=evaled._VECTptr->front().val;
	return tmp;
      }
      return not_evaled;
    }
    else {
      if (evalf_after && (evaled.type!=_IDNT))
	return evalf_try_exact(evaled,level,contextptr);
      else {
	if (not_evaled.subtype==evaled.subtype && not_evaled==evaled) // don't return a copy of the _IDNT
	  return not_evaled;
	else
	  return evaled;
      }
    }
  }

  gen evalf_try_exact_VECT(const vecteur & v,int subtype,int level,const context * contextptr){
    // bool save_is_inevalf=is_inevalf;
    // is_inevalf=true;
    vecteur w;
    vecteur::const_iterator it=v.begin(), itend=v.end();
    w.reserve(itend-it);
    for (;it!=itend;++it){
      gen tmp=evalf_try_exact(*it,level,contextptr);
      if (subtype){
	if ((subtype==_SEQ__VECT)&&(tmp.type==_VECT) && (tmp.subtype==_SEQ__VECT)){
	  const_iterateur jt=tmp._VECTptr->begin(),jtend=tmp._VECTptr->end();
	  for (;jt!=jtend;++jt)
	    w.push_back(*jt);
	}
	else {
	  if ((subtype==_SEQ__VECT) || (!equalposcomp(w,tmp)))
	    w.push_back(tmp);
	}
      }
      else
	w.push_back(tmp);
    }
    // is_inevalf=save_is_inevalf;
    return gen(w,subtype);
  }


gen evalf_try_exact(const gen &g, int level, const context *contextptr) {
    switch (g.type) {
    case _DOUBLE_: case _REAL: case _STRNG: case _MAP: case _EQW: case _GROB: case _INT_: case _ZINT:
      return g;
    case _CPLX: 
      return gen(evalf_try_exact(*g._CPLXptr,level,contextptr),evalf_try_exact(*(g._CPLXptr + 1),level,contextptr));
    case _USER:
      return g._USERptr->evalf(level,contextptr);
    case _IDNT:
      if (*g._IDNTptr==_IDNT_pi || *g._IDNTptr==e__IDNT )
	return g._IDNTptr->eval(level,contextptr);
      if (!contextptr && g.subtype==_GLOBAL__EVAL)
	return check_not_assume_try_exact(g,global_eval(g,100),true,level,contextptr);
      return check_not_assume_try_exact(g,g._IDNTptr->eval(level,contextptr),true,level,contextptr);
    case _VECT:
      return gen(evalf_try_exact_VECT(*g._VECTptr,g.subtype,level,contextptr));
    case _SYMB:
      if ( (g._SYMBptr->sommet==at_pow) && (g._SYMBptr->feuille._VECTptr->back().type==_INT_))
	return pow(g._SYMBptr->feuille._VECTptr->front().evalf(level,contextptr),g._SYMBptr->feuille._VECTptr->back());
      if (g._SYMBptr->sommet==at_integrate)
	return _romberg(g._SYMBptr->feuille,contextptr);
      if (g._SYMBptr->sommet==at_rootof)
	return approx_rootof(g._SYMBptr->feuille.evalf(level,contextptr));
      if (g._SYMBptr->sommet==at_cell)
	return g;
      return g._SYMBptr->evalf(level,contextptr);
    case _FRAC:
      return rdiv(evalf_try_exact(g._FRACptr->num,level,contextptr),evalf_try_exact(g._FRACptr->den,level,contextptr));
    case _FUNC: case _MOD: case _ROOT:
      return g; // replace in RPN mode
    case _EXT:
      return alg_evalf(evalf_try_exact(*g._EXTptr,level,contextptr),evalf_try_exact(*(g._EXTptr+1),level,contextptr));
    case _POLY:
      return apply(*g._POLYptr,no_context_evalf);
    default: 
      return g.evalf(level, contextptr);
    }
}
#ifndef NO_NAMESPACE_GIAC
}
#endif

void MathStructure::evalf() {
	if(m_type == STRUCT_FUNCTION) {
		MathStructure mstruct(toGiac().evalf());
		if(!mstruct.isFunction()) {
			set(mstruct);
			return evalf();
		}
	} else if(m_type == STRUCT_POWER) {
		MathStructure mstruct(giac::simplify(toGiac()));
		if(!mstruct.isPower()) {
			set(mstruct);
			return evalf();
		}
		mstruct.set(toGiac().evalf());
		if(!mstruct.isPower()) {
			set(mstruct);
			return evalf();
		}
	} else if(m_type == STRUCT_UNKNOWN) {
		set(giac_unknown->evalf());
	}
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		v_subs[i].evalf();
	}
	set(toGiac().eval());
	if(m_type == STRUCT_FUNCTION) {
		MathStructure mstruct(toGiac().evalf());
		if(!mstruct.isFunction()) {
			set(mstruct);
		}
	} else if(m_type == STRUCT_POWER) {
		MathStructure mstruct(giac::simplify(toGiac()));
		if(!mstruct.isPower()) {
			set(mstruct);
			return;
		}
		mstruct.set(toGiac().evalf());
		if(!mstruct.isPower()) {
			set(mstruct);
			return;
		}
	} else if(m_type == STRUCT_UNKNOWN) {
		set(giac_unknown->evalf());
	}
}

void MathStructure::evalQalculateFunctions(const EvaluationOptions &eo) {
	if(m_type == STRUCT_FUNCTION && !o_function->isGiacFunction()) {
		for(unsigned int i = 0; i < v_subs.size(); i++) {
			v_subs[i].eval(eo);
		}
		m_type = STRUCT_VECTOR;
		MathStructure vargs(*this);
		m_type = STRUCT_FUNCTION;
		set(o_function->calculate(vargs));
	} else {
		for(unsigned int i = 0; i < v_subs.size(); i++) {
			v_subs[i].evalQalculateFunctions(eo);
		}
	}
}
#endif

MathStructure &MathStructure::eval(const EvaluationOptions &eo) {
#ifdef HAVE_GIAC
	if(eo.sync_units) {
		syncUnits();
	}
	evalQalculateFunctions(eo);
	try {
		if(eo.approximation == APPROXIMATION_APPROXIMATE) {
			set(toGiac().evalf());
		} else if(eo.approximation == APPROXIMATION_TRY_EXACT) {
			//g = giac::evalf_try_exact(g);
			set(toGiac().eval());
			evalf();
		} else {
			set(toGiac().eval());
		}
		if(eo.structuring == STRUCTURING_SIMPLIFY) {
			set(giac::simplify(toGiac()));
		} else if(eo.structuring == STRUCTURING_FACTORIZE) {
			set(giac::factor(toGiac()));
		}
	} catch(std::runtime_error & err){
		printf("Giac error: %s.\n", err.what());
		CALCULATOR->error(true, _("Giac error: %s.\n"), err.what(), NULL);
	}
#else
	syncUnits();
	calculateFunctions(eo);
	calculatesub(1);
	calculatesub(2);
#endif
	return *this;
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
			return giac::symbolic(giac::at_inv, v_subs[0].toGiac());
		} 
		case STRUCT_NEGATE: {
			return giac::symbolic(giac::at_neg, v_subs[0].toGiac());
		} 
		case STRUCT_NOT: {
			return giac::symbolic(giac::at_not, v_subs[0].toGiac());
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
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				giac::gen f = v_subs[i].toGiac();
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
					v_subs.push_back(MathStructure((*giac_gen._SYMBptr->feuille._VECTptr)[0]));
					v_subs.push_back(MathStructure((*giac_gen._SYMBptr->feuille._VECTptr)[1]));
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
				v_subs.push_back(MathStructure(giac_gen._SYMBptr->feuille));
			} else if(giac_gen._SYMBptr->sommet == giac::at_inv) {
				m_type = STRUCT_INVERSE;
				v_subs.push_back(MathStructure(giac_gen._SYMBptr->feuille));
			} else if(giac_gen._SYMBptr->sommet == giac::at_not) {
				m_type = STRUCT_NOT;
				v_subs.push_back(MathStructure(giac_gen._SYMBptr->feuille));
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
				v_subs.push_back(MathStructure(giac_gen._SYMBptr->feuille));
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
								v_subs.push_back(MathStructure((*giac_gen._SYMBptr->feuille._VECTptr)[i]));
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
						v_subs.push_back(MathStructure(giac_gen._SYMBptr->feuille));
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
				v_subs.push_back((*giac_gen._VECTptr)[i]);
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
	v_subs.push_back(o);
}
void MathStructure::insertChild(const MathStructure &o, unsigned int index) {
	if(index > 0 && index <= v_subs.size()) {
		v_subs.insert(v_subs.begin() + (index - 1), o);
	} else {
		addChild(o);
	}
}
void MathStructure::setChild(const MathStructure &o, unsigned int index) {
	if(index > 0 && index <= v_subs.size()) {
		v_subs[index - 1] = o;
	}
}
const MathStructure *MathStructure::getChild(unsigned int index) const {
	if(index > 0 && index <= v_subs.size()) {
		return &v_subs[index - 1];
	}
	return NULL;
}
MathStructure *MathStructure::getChild(unsigned int index) {
	if(index > 0 && index <= v_subs.size()) {
		return &v_subs[index - 1];
	}
	return NULL;
}
unsigned int MathStructure::countChilds() const {
	return v_subs.size();
}
unsigned int MathStructure::size() const {
	return v_subs.size();
}
const MathStructure *MathStructure::base() const {
	if(m_type == STRUCT_POWER && v_subs.size() >= 1) {
		return &v_subs[0];
	}
	return NULL;
}
const MathStructure *MathStructure::exponent() const {
	if(m_type == STRUCT_POWER && v_subs.size() >= 2) {
		return &v_subs[1];
	}
	return NULL;
}
MathStructure *MathStructure::base() {
	if(m_type == STRUCT_POWER && v_subs.size() >= 1) {
		return &v_subs[0];
	}
	return NULL;
}
MathStructure *MathStructure::exponent() {
	if(m_type == STRUCT_POWER && v_subs.size() >= 2) {
		return &v_subs[1];
	}
	return NULL;
}
void MathStructure::addAlternative(const MathStructure &o) {
	if(m_type != STRUCT_ALTERNATIVES) {
		MathStructure copy_this(*this);
		clear();
		m_type = STRUCT_ALTERNATIVES;
		v_subs.push_back(copy_this);
	}
	v_subs.push_back(o);
}

int MathStructure::type() const {
	return m_type;
}

void MathStructure::format() {
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		v_subs[i].format();
	}
	switch(m_type) {
		case STRUCT_MULTIPLICATION: {
			bool b = false;
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(v_subs[i].type() == STRUCT_INVERSE) {
					b = true;
				}
			}
			if(b) {
				MathStructure den;
				MathStructure num;
				bool ds = false, ns = false;
				for(unsigned int i = 0; i < v_subs.size(); i++) {
					if(v_subs[i].type() == STRUCT_INVERSE) {
						if(ds) {	
							den.multiply(v_subs[i][0]);
						} else {
							den.set(v_subs[i][0]);
						}
					} else {
						if(ns) {
							num.multiply(v_subs[i]);
						} else {
							ns = true;
							num.set(v_subs[i]);
						}
					}
				}
				clear();
				m_type = STRUCT_DIVISION;
				v_subs.push_back(num);
				v_subs.push_back(den);
			}
		}
	}
}

string MathStructure::print(const PrintOptions &po, const InternalPrintStruct &ips) const {
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
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(i > 0) {
					if(v_subs[i].type() == STRUCT_NEGATE) {
						print_str += " - ";
						ips_n.wrap = v_subs[i][0].size() > 1;
						print_str += v_subs[i][0].print(po, ips_n);
					} else {
						print_str += " + ";
						ips_n.wrap = v_subs[i].size() > 1;
						print_str += v_subs[i].print(po, ips_n);
					}
				} else {
					ips_n.wrap = v_subs[i].size() > 1;
					print_str += v_subs[i].print(po, ips_n);
				}
			}
			break;
		}
		case STRUCT_NEGATE: {
			print_str = "-";
			ips_n.depth++;
			ips_n.wrap = v_subs[0].size() > 1;
			print_str += v_subs[0].print(po, ips_n);
			break;
		}
		case STRUCT_MULTIPLICATION: {
			ips_n.depth++;
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				ips_n.wrap = v_subs[i].isMultiplication() || v_subs[i].isAddition();
				if(!po.short_multiplication || (i > 0 && !ips_n.wrap && (v_subs[i - 1].size() > 1 || (v_subs[i].isNumber() && v_subs[i - 1].isNumber())))) {
					print_str += " * ";
				}
				print_str += v_subs[i].print(po, ips_n);
			}
			break;
		}
		case STRUCT_INVERSE: {
			print_str = "1";
			print_str += " / ";
			ips_n.depth++;
			ips_n.wrap = v_subs[0].size() > 1;
			print_str += v_subs[0].print(po, ips_n);
			break;
		}
		case STRUCT_DIVISION: {
			ips_n.depth++;
			ips_n.wrap = v_subs[0].size() > 1;
			print_str = v_subs[0].print(po, ips_n);
			print_str += " / ";
			ips_n.wrap = v_subs[1].size() > 1;
			print_str += v_subs[1].print(po, ips_n);
			break;
		}
		case STRUCT_POWER: {
			ips_n.depth++;
			ips_n.wrap = v_subs[0].size() > 1;
			print_str = v_subs[0].print(po, ips_n);
			print_str += "^";
			ips_n.wrap = v_subs[1].size() > 1;
			print_str += v_subs[1].print(po, ips_n);
			break;
		}
		case STRUCT_COMPARISON: {
			ips_n.depth++;
			ips_n.wrap = v_subs[0].size() > 1;
			print_str = v_subs[0].print(po, ips_n);
			print_str += " ";
			switch(ct_comp) {
				case COMPARISON_EQUALS: {print_str += "="; break;}
				case COMPARISON_NOT_EQUALS: {print_str += "!="; break;}
				case COMPARISON_GREATER: {print_str += ">"; break;}
				case COMPARISON_LESS: {print_str += "<"; break;}
				case COMPARISON_EQUALS_GREATER: {print_str += ">="; break;}
				case COMPARISON_EQUALS_LESS: {print_str += "<="; break;}
			}
			print_str += " ";
			ips_n.wrap = v_subs[1].size() > 1;
			print_str += v_subs[1].print(po, ips_n);
			break;
		}
		case STRUCT_AND: {
			ips_n.depth++;
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(i > 0) {
					print_str += " && ";
				}
				ips_n.wrap = v_subs[i].size() > 1;
				print_str += v_subs[i].print(po, ips_n);
			}
			break;
		}
		case STRUCT_OR: {
			ips_n.depth++;
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(i > 0) {
					print_str += " || ";
				}
				ips_n.wrap = v_subs[i].size() > 1;
				print_str += v_subs[i].print(po, ips_n);
			}
			break;
		}
		case STRUCT_XOR: {
			ips_n.depth++;
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(i > 0) {
					print_str += " XOR ";
				}
				ips_n.wrap = v_subs[i].size() > 1;
				print_str += v_subs[i].print(po, ips_n);
			}
			break;
		}
		case STRUCT_NOT: {
			print_str = "!";
			ips_n.depth++;
			ips_n.wrap = true;
			print_str += v_subs[0].print(po, ips_n);
			break;
		}
		case STRUCT_VECTOR: {
			ips_n.depth++;
			ips_n.wrap = false;
			print_str = "[";
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(i > 0) {
					print_str += ", ";
				}
				print_str += v_subs[i].print(po, ips_n);
			}
			print_str += "]";
			break;
		}
		case STRUCT_UNIT: {
			if(po.abbreviate_units) {
				print_str = o_unit->shortName();
			} else {
				print_str = o_unit->singular();
			}
			break;
		}
		case STRUCT_VARIABLE: {
			print_str = o_variable->name();
			break;
		}
		case STRUCT_FUNCTION: {
			ips_n.depth++;
			ips_n.wrap = false;
			print_str += o_function->name();
			print_str += "(";
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(i > 0) {
					print_str += ", ";
				}
				print_str += v_subs[i].print(po, ips_n);
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

MathStructure MathStructure::range(int start, int end) const {
	if(!isVector()) {
		if(start > 1) {
			return MathStructure();
		} else {
			return *this;
		}
	}
	if(start < 1) start = 1;
	else if(start > (int) v_subs.size()) return MathStructure();
	if(end < (int) v_subs.size() || end > (int) v_subs.size()) end = v_subs.size();
	else if(end < start) end = start;	
	MathStructure mstruct;
	mstruct.clearVector();
	for(; start < end; start++) {
		mstruct.addItem(v_subs[start - 1]);
	}
	return mstruct;
}

MathStructure MathStructure::flattenVector() const {
	if(!isVector()) {
		return *this;
	}
	MathStructure mstruct;
	mstruct.clearVector();
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(v_subs[i].isVector()) {
			for(unsigned int i2 = 0; i2 < v_subs[i].size(); i2++) {
				mstruct.addItem(v_subs[i][i2]);
			}
		} else {
			mstruct.addItem(v_subs[i]);
		}
	}
	return mstruct;
}
bool MathStructure::rank(bool ascending) {
	return false;
}
bool MathStructure::sort(bool ascending) {
	return false;
}
void MathStructure::resizeVector(unsigned int i, const MathStructure &mfill) {
	if(i > v_subs.size()) {
		while(i > v_subs.size()) {
			v_subs.push_back(mfill);
		}
	} else {
		v_subs.resize(i);
	}
}

unsigned int MathStructure::rows() const {
	if(m_type != STRUCT_VECTOR || v_subs.size() == 0 || (v_subs.size() == 1 && (!v_subs[0].isVector() || v_subs[0].size() == 0))) return 0;
	return v_subs.size();
}
unsigned int MathStructure::columns() const {
	if(m_type != STRUCT_VECTOR || v_subs.size() == 0 || !v_subs[0].isVector()) return 0;
	return v_subs[0].size();
}
const MathStructure *MathStructure::getElement(unsigned int row, unsigned int column) const {
	if(row == 0 || column == 0 || row > rows() || column > columns()) return NULL;
	if(v_subs[row - 1].size() < column) return NULL;
	return &v_subs[row - 1][column - 1];
}
void MathStructure::setElement(const MathStructure &mstruct, unsigned int row, unsigned int column) {
	if(row == 0 || column == 0 || row > rows() || column > columns()) return;
	if(v_subs[row - 1].size() < column) return;
	v_subs[row - 1][column - 1] = mstruct;
}
void MathStructure::addRows(unsigned int r, const MathStructure &mfill) {
	if(r == 0) return;
	unsigned int cols = columns();
	MathStructure mstruct; mstruct.clearVector();
	mstruct.resizeVector(cols, mfill);
	for(unsigned int i = 0; i < r; i++) {
		v_subs.push_back(mstruct);
	}
}
void MathStructure::addColumns(unsigned int c, const MathStructure &mfill) {
	if(c == 0) return;
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(v_subs[i].isVector()) {
			for(unsigned int i = 0; i < c; i++) {
				v_subs[i].addItem(mfill);
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
	if(r > v_subs.size()) {
		addRows(r - v_subs.size(), mfill);
	} else if(r != v_subs.size()) {
		v_subs.resize(r);
	}
	unsigned int cols = columns();
	if(c > cols) {
		addColumns(c - cols, mfill);
	} else if(c != cols) {
		for(unsigned i = 0; i < v_subs.size(); i++) {
			v_subs[i].resizeVector(c, mfill);
		}
	}
}
bool MathStructure::matrixIsSymmetric() const {
	return rows() == columns();
}
MathStructure MathStructure::determinant() const {
	if(!matrixIsSymmetric()) {
		CALCULATOR->error(true, _("The determinant can only be calculated for symmetric matrices."), NULL);
		return m_undefined;
	}
	MathStructure mstruct;
	if(v_subs.size() == 1) {
		if(v_subs[0].size() >= 1) {	
			mstruct == v_subs[0][0];
		}
	} else if(v_subs.size() == 2) {
		mstruct = v_subs[0][0];
		mstruct *= v_subs[1][1];
		MathStructure mtmp = v_subs[1][0];
		mtmp *= v_subs[0][1];
		mstruct -= mtmp;
	} else {
		MathStructure mtrx;
		mtrx.clearMatrix();
		mtrx.resizeMatrix(v_subs.size() - 1, v_subs[0].size() - 1, m_undefined);
		for(unsigned int index_c = 0; index_c < v_subs[0].size(); index_c++) {
			for(unsigned int index_r2 = 1; index_r2 < v_subs.size(); index_r2++) {
				for(unsigned int index_c2 = 0; index_c2 < v_subs[index_r2].size(); index_c2++) {
					if(index_c2 > index_c) {
						mtrx.setElement(v_subs[index_r2][index_c2], index_r2, index_c2);
					} else if(index_c2 < index_c) {
						mtrx.setElement(v_subs[index_r2][index_c2], index_r2, index_c2 + 1);
					}
				}
			}
			MathStructure mdet = mtrx.determinant();	
			if(index_c % 2 == 1) {
				mdet.negate();
			}
			
			mdet *= v_subs[0][index_c];
			mstruct += mdet;
		}
	}
	return mstruct;
}
void MathStructure::setToIdentityMatrix(unsigned int n) {
	clearMatrix();
	resizeMatrix(n, n, m_zero);
	for(unsigned int i = 0; i < n; i++) {
		v_subs[i][i] = m_one;
	}
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

void MathStructure::syncUnits() {
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
	for(unsigned int i = 0; i < composite_units.size(); i++) {	
		convert(composite_units[i]);
	}	
	dissolveAllCompositeUnits();
	for(unsigned int i = 0; i < base_units.size(); i++) {	
		convert(base_units[i]);
	}
	for(unsigned int i = 0; i < alias_units.size(); i++) {	
		convert(alias_units[i]);
	}	
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
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				if(v_subs[i].dissolveAllCompositeUnits()) b = true;
			}
			return b;
		}		
	}		
	return false;
}
bool MathStructure::convert(Unit *u) {
	if(m_type == STRUCT_NUMBER || m_type == STRUCT_SYMBOLIC || m_type == STRUCT_VARIABLE || m_type == STRUCT_UNKNOWN) return false;
	bool b = false;	
	if(m_type == STRUCT_UNIT && o_unit == u) return false;
	if(u->unitType() == COMPOSITE_UNIT && !(m_type == STRUCT_UNIT && o_unit->baseUnit() == u)) {
		return convert(((CompositeUnit*) u)->generateMathStructure());
	}
	if(m_type == STRUCT_UNIT) {
		if(u->hasComplexRelationTo(o_unit)) return false;
		if(testDissolveCompositeUnit(u)) {
			convert(u);
			return true;
		}
		MathStructure exp(1, 1);
		MathStructure mstruct(1, 1);
		u->convert(o_unit, mstruct, exp, &b);
		if(b) {
			o_unit = u;
			if(!exp.isNumber() || !exp.number().isOne()) {
				raise(exp);
			}
			multiply(mstruct);
		}
		return b;
	} else {
		for(unsigned int i = 0; i < v_subs.size(); i++) {
			if(v_subs[i].convert(u)) b = true;
		}
		return b;		
	}	
	return b;
}
bool MathStructure::convert(string unit_str) {
	return convert(CALCULATOR->parse(unit_str));
}
bool MathStructure::convert(const MathStructure unit_mstruct) {
	bool b = false;
	if(unit_mstruct.type() == STRUCT_UNIT) {
		if(convert(unit_mstruct.unit())) b = true;
	} else {
		for(unsigned int i = 0; i < unit_mstruct.size(); i++) {
			if(convert(unit_mstruct[i])) b = true;
		}
	}	
	return b;
}

bool MathStructure::contains(const MathStructure &mstruct) const {
	if(equals(mstruct)) return true;
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(v_subs[i].contains(mstruct)) return true;
	}
	return false;
}
bool MathStructure::containsType(int mtype) const {
	if(m_type == mtype) return true;
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(v_subs[i].containsType(mtype)) return true;
	}
	return false;
}
bool MathStructure::replace(const MathStructure &mfrom, const MathStructure &mto) {
	bool b = false;
	for(unsigned int i = 0; i < v_subs.size(); i++) {
		if(v_subs[i].replace(mfrom, mto)) b = true;
	}
	if(equals(mfrom)) {
		set(mto);
		b = true;
	}
	return b;
}

MathStructure MathStructure::generateVector(string x_var, const MathStructure &min, const MathStructure &max, int steps, MathStructure *x_vector) {
	if(steps < 1) {
		steps = 1;
	}
	MathStructure x_value(min);
	MathStructure x_mstruct(x_var);
	MathStructure y_value;
	MathStructure y_vector;
	y_vector.clearVector();
	MathStructure step(max);
	step -= min;
	step /= steps;
	step.eval();
	for(int i = 0; i <= steps; i++) {
		if(x_vector) {
			x_vector->addComponent(x_value);
		}
		y_value = *this;
		y_value.replace(x_mstruct, x_value);
		y_value.eval();
		y_vector.addComponent(y_value);
		x_value += step;
	}
	return y_vector;
}
MathStructure MathStructure::generateVector(string x_var, const MathStructure &x_vector) {
	MathStructure y_value;
	MathStructure x_mstruct(x_var);
	MathStructure y_vector;
	y_vector.clearVector();
	for(unsigned int i = 1; i <= x_vector.components(); i++) {
		y_value = *this;
		y_value.replace(x_mstruct, x_vector.getComponent(i));
		y_value.eval();
		y_vector.addComponent(y_value);
	}
	return y_vector;
}

bool MathStructure::differentiate(const MathStructure &x_var) {
	if(equals(x_var)) {
		set(1, 1);
		return true;
	}
	switch(m_type) {
		case STRUCT_ADDITION: {
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				v_subs[i].differentiate(x_var);
			}
			break;
		}
		case STRUCT_ALTERNATIVES: {
			for(unsigned int i = 0; i < v_subs.size(); i++) {
				v_subs[i].differentiate(x_var);
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
			if(v_subs.size() < 1) {
				clear();
				break;
			} else if(v_subs.size() < 2) {
				MathStructure base_copy(v_subs[0]);
				set(base_copy);
				return differentiate(x_var);
			}
			bool x_in_base = v_subs[0].contains(x_var);
			bool x_in_exp = v_subs[1].contains(x_var);
			if(x_in_base && !x_in_exp) {
				MathStructure exp_mstruct(v_subs[1]);
				MathStructure base_mstruct(v_subs[0]);
				v_subs[1] += -1;
				multiply(exp_mstruct);
				base_mstruct.differentiate(x_var);
				multiply(base_mstruct);
			} else if(!x_in_base && x_in_exp) {
				MathStructure exp_mstruct(v_subs[1]);
				MathStructure mstruct(CALCULATOR->f_ln, &v_subs[0], NULL);
				multiply(mstruct);
				exp_mstruct.differentiate(x_var);
				multiply(exp_mstruct);
			} else if(x_in_base && x_in_exp) {
				MathStructure exp_mstruct(v_subs[1]);
				MathStructure base_mstruct(v_subs[0]);
				exp_mstruct.differentiate(x_var);
				base_mstruct.differentiate(x_var);
				base_mstruct /= v_subs[0];
				base_mstruct *= v_subs[1];
				MathStructure mstruct(CALCULATOR->f_ln, &v_subs[0], NULL);
				mstruct *= exp_mstruct;
				mstruct += base_mstruct;
				multiply(mstruct);
			} else {
				clear();
			}
			break;
		}
		case STRUCT_FUNCTION: {
			if(o_function == CALCULATOR->f_ln && v_subs.size() == 1) {
				MathStructure mstruct(v_subs[0]);
				set(mstruct);
				inverse();
				mstruct.differentiate(x_var);
				multiply(mstruct);
			} else if(o_function == CALCULATOR->f_sin && v_subs.size() == 1) {
				o_function = CALCULATOR->f_cos;
				MathStructure mstruct(v_subs[0]);
				mstruct.differentiate(x_var);
				multiply(mstruct);
			} else if(o_function == CALCULATOR->f_cos && v_subs.size() == 1) {
				o_function = CALCULATOR->f_sin;
				MathStructure mstruct(v_subs[0]);
				multiply(-1);
				mstruct.differentiate(x_var);
				multiply(mstruct, true);
			} else if(o_function == CALCULATOR->f_diff && v_subs.size() == 3 && v_subs[1] == x_var) {
				v_subs[2] += 1;
			} else if(o_function == CALCULATOR->f_diff && v_subs.size() == 2 && v_subs[1] == x_var) {
				v_subs.push_back(2);
			} else if(o_function == CALCULATOR->f_diff && v_subs.size() == 1 && x_var == (Variable*) CALCULATOR->v_x) {
				v_subs.push_back(x_var);
				v_subs.push_back(2);
			} else {
				MathStructure mstruct3(1);
				MathStructure mstruct(CALCULATOR->f_diff, this, &x_var, &mstruct3, NULL);
				set(mstruct);
				return false;
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if(v_subs.size() > 2) {
				MathStructure mstruct = v_subs[0];
				mstruct.transform(STRUCT_MULTIPLICATION, v_subs[1]);
				for(unsigned int i = 2; i < v_subs.size() - 1; i++) {
					mstruct.addChild(v_subs[i]);
				}
				MathStructure mstruct2(v_subs[v_subs.size() - 1]);
				MathStructure mstruct3(mstruct);
				mstruct.differentiate(x_var);
				mstruct2.differentiate(x_var);			
				mstruct *= v_subs[v_subs.size() - 1];
				mstruct2 *= mstruct3;
				set(mstruct);
				add(mstruct2);
			} else {
				MathStructure mstruct(v_subs[0]);
				MathStructure mstruct2(v_subs[1]);
				mstruct.differentiate(x_var);
				mstruct2.differentiate(x_var);			
				mstruct *= v_subs[1];
				mstruct2 *= v_subs[0];
				set(mstruct);
				add(mstruct2);
			}
			break;
		}
		case STRUCT_SYMBOLIC: {
			clear();
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

