/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef VARIABLE_H
#define VARIABLE_H

#include "ExpressionItem.h"
#include "includes.h"

#define DECLARE_BUILTIN_VARIABLE(x)	class x : public DynamicVariable { \
					  private: \
						void calculate() const;	\
 					  public: \
						x(); \
						x(const x *variable) {set(variable);} \
						ExpressionItem *copy() const {return new x(this);} \
					};

typedef enum {
	ASSUMPTION_NUMBER_INTEGER = 0,
	ASSUMPTION_NUMBER_RATIONAL = 1,
	ASSUMPTION_NUMBER_REAL = 2,
	ASSUMPTION_NUMBER_COMPLEX = 3,
	ASSUMPTION_NUMBER_NUMBER = 4,
	ASSUMPTION_NUMBER_NONE = 5
} AssumptionNumberType;

typedef enum {
	ASSUMPTION_SIGN_UNKNOWN,
	ASSUMPTION_SIGN_POSITIVE,
	ASSUMPTION_SIGN_NONNEGATIVE,
	ASSUMPTION_SIGN_NEGATIVE,
	ASSUMPTION_SIGN_NONZERO
} AssumptionSign;

class Assumptions {

  public:

	AssumptionNumberType number_type;
	AssumptionSign sign;

	Assumptions();
	virtual ~Assumptions();

	virtual bool isPositive();
	virtual bool isNegative();
	virtual bool isNonNegative();
	virtual bool isInteger();
	virtual bool isNumber();
	virtual bool isRational();
	virtual bool isReal();
	virtual bool isNonZero();
};


class Variable : public ExpressionItem {

  public:

	Variable(string cat_, string name_, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	Variable();
	Variable(const Variable *variable);
	virtual ~Variable();
	virtual ExpressionItem *copy() const = 0;
	virtual void set(const ExpressionItem *item);
	virtual int type() const {return TYPE_VARIABLE;}
	virtual bool isKnown() const = 0;

	virtual bool isPositive() {return false;}
	virtual bool isNegative() {return false;}
	virtual bool isNonNegative() {return false;}
	virtual bool isInteger() {return false;}
	virtual bool isNumber() {return false;}
	virtual bool isRational() {return false;}
	virtual bool isReal() {return false;}
	virtual bool isNonZero() {return false;}
	
};

class UnknownVariable : public Variable {

  protected:
  
  	Assumptions *o_assumption;
  
  public:

	UnknownVariable(string cat_, string name_, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	UnknownVariable();
	UnknownVariable(const UnknownVariable *variable);
	virtual ~UnknownVariable();
	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	bool isKnown() const {return false;}
	void setAssumptions(Assumptions *ass);
	Assumptions *assumptions();

	virtual bool isPositive();
	virtual bool isNegative();
	virtual bool isNonNegative();
	virtual bool isInteger();
	virtual bool isNumber();
	virtual bool isRational();
	virtual bool isReal();
	virtual bool isNonZero();
	
};

class KnownVariable : public Variable {

  protected:

	MathStructure *mstruct;
	bool b_expression;
 	int calculated_precision;
	string sexpression;

  public:
  
	KnownVariable(string cat_, string name_, const MathStructure &o, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	KnownVariable(string cat_, string name_, string expression_, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);	
	KnownVariable();
	KnownVariable(const KnownVariable *variable);
	virtual ~KnownVariable();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	bool isKnown() const {return true;}
	virtual bool isExpression() const;
	virtual string expression() const;

	virtual void set(const MathStructure &o);
	virtual void set(string expression_);	

	virtual const MathStructure &get();
	
	virtual bool isPositive();
	virtual bool isNegative();
	virtual bool isNonNegative();
	virtual bool isInteger();
	virtual bool isNumber();
	virtual bool isRational();
	virtual bool isReal();
	virtual bool isNonZero();

};

class DynamicVariable : public KnownVariable {

  protected:
  
  	virtual void calculate() const = 0;
  	
  public:

	DynamicVariable(string cat_, string name_, string title_ = "", bool is_local = false, bool is_builtin = true, bool is_active = true);
	DynamicVariable(const DynamicVariable *variable);
	DynamicVariable();
	virtual ~DynamicVariable();

	ExpressionItem *copy() const = 0;
	void set(const ExpressionItem *item);

	const MathStructure &get();
	
	void set(const MathStructure &o);
	void set(string expression);	
	
	int calculatedPrecision() const;
	
	virtual bool isPositive() {return true;}
	virtual bool isNegative() {return false;}
	virtual bool isNonNegative() {return true;}
	virtual bool isInteger() {return false;}
	virtual bool isNumber() {return true;}
	virtual bool isRational() {return false;}
	virtual bool isReal() {return true;}
	virtual bool isNonZero() {return true;}

};

DECLARE_BUILTIN_VARIABLE(PiVariable);
DECLARE_BUILTIN_VARIABLE(EVariable);
DECLARE_BUILTIN_VARIABLE(EulerVariable);
DECLARE_BUILTIN_VARIABLE(CatalanVariable);

#endif
