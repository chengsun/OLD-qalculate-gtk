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

#include <libqalculate/ExpressionItem.h>
#include <libqalculate/includes.h>

#define DECLARE_BUILTIN_VARIABLE(x)	class x : public DynamicVariable { \
					  private: \
						void calculate() const;	\
 					  public: \
						x(); \
						x(const x *variable) {set(variable);} \
						ExpressionItem *copy() const {return new x(this);} \
					};

typedef enum {
	ASSUMPTION_NUMBER_NONE = 0,
	ASSUMPTION_NUMBER_NUMBER = 1,
	ASSUMPTION_NUMBER_COMPLEX = 2,
	ASSUMPTION_NUMBER_REAL = 3,
	ASSUMPTION_NUMBER_RATIONAL = 4,
	ASSUMPTION_NUMBER_INTEGER = 5
} AssumptionNumberType;

typedef enum {
	ASSUMPTION_SIGN_UNKNOWN,
	ASSUMPTION_SIGN_POSITIVE,
	ASSUMPTION_SIGN_NONNEGATIVE,
	ASSUMPTION_SIGN_NEGATIVE,
	ASSUMPTION_SIGN_NONPOSITIVE,
	ASSUMPTION_SIGN_NONZERO
} AssumptionSign;

enum {
	SUBTYPE_VARIABLE,
	SUBTYPE_UNKNOWN_VARIABLE,
	SUBTYPE_KNOWN_VARIABLE
};

class Assumptions {

  protected:
  
	AssumptionNumberType i_type;
	AssumptionSign i_sign;
	Number *fmin, *fmax;
	bool b_incl_min, b_incl_max;

  public:

	Assumptions();
	~Assumptions();

	bool isPositive();
	bool isNegative();
	bool isNonNegative();
	bool isNonPositive();
	bool isInteger();
	bool isNumber();
	bool isRational();
	bool isReal();
	bool isComplex();
	bool isNonZero();
	
	AssumptionNumberType numberType();
	AssumptionSign sign();
	void setNumberType(AssumptionNumberType ant);
	void setSign(AssumptionSign as);
	
	void setMin(const Number *nmin);	
	void setIncludeEqualsMin(bool include_equals);
	bool includeEqualsMin() const;	
	const Number *min() const;
	void setMax(const Number *nmax);	
	void setIncludeEqualsMax(bool include_equals);
	bool includeEqualsMax() const;	
	const Number *max() const;
	
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
	virtual int subtype() const {return SUBTYPE_VARIABLE;}
	virtual bool isKnown() const = 0;

	virtual bool representsPositive(bool allow_units = false) {return false;}
	virtual bool representsNegative(bool allow_units = false) {return false;}
	virtual bool representsNonNegative(bool allow_units = false) {return false;}
	virtual bool representsNonPositive(bool allow_units = false) {return false;}
	virtual bool representsInteger(bool allow_units = false) {return false;}
	virtual bool representsNumber(bool allow_units = false) {return false;}
	virtual bool representsRational(bool allow_units = false) {return false;}
	virtual bool representsReal(bool allow_units = false) {return false;}
	virtual bool representsComplex(bool allow_units = false) {return false;}
	virtual bool representsNonZero(bool allow_units = false) {return false;}
	virtual bool representsEven(bool allow_units = false) {return false;}
	virtual bool representsOdd(bool allow_units = false) {return false;}
	virtual bool representsUndefined(bool include_childs = false, bool include_infinite = false) {return false;}
	
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
	int subtype() const {return SUBTYPE_UNKNOWN_VARIABLE;}

	virtual bool representsPositive(bool allow_units = false);
	virtual bool representsNegative(bool allow_units = false);
	virtual bool representsNonNegative(bool allow_units = false);
	virtual bool representsNonPositive(bool allow_units = false);
	virtual bool representsInteger(bool allow_units = false);
	virtual bool representsNumber(bool allow_units = false);
	virtual bool representsRational(bool allow_units = false);
	virtual bool representsReal(bool allow_units = false);
	virtual bool representsComplex(bool allow_units = false);
	virtual bool representsNonZero(bool allow_units = false);
	
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
	int subtype() const {return SUBTYPE_KNOWN_VARIABLE;}

	virtual void set(const MathStructure &o);
	virtual void set(string expression_);	

	virtual const MathStructure &get();
	
	virtual bool representsPositive(bool allow_units = false);
	virtual bool representsNegative(bool allow_units = false);
	virtual bool representsNonNegative(bool allow_units = false);
	virtual bool representsNonPositive(bool allow_units = false);
	virtual bool representsInteger(bool allow_units = false);
	virtual bool representsNumber(bool allow_units = false);
	virtual bool representsRational(bool allow_units = false);
	virtual bool representsReal(bool allow_units = false);
	virtual bool representsComplex(bool allow_units = false);
	virtual bool representsNonZero(bool allow_units = false);
	virtual bool representsEven(bool allow_units = false);
	virtual bool representsOdd(bool allow_units = false);
	virtual bool representsUndefined(bool include_childs = false, bool include_infinite = false);

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
	
	virtual bool representsPositive(bool allow_units = false) {return true;}
	virtual bool representsNegative(bool allow_units = false) {return false;}
	virtual bool representsNonNegative(bool allow_units = false) {return true;}
	virtual bool representsNonPositive(bool allow_units = false) {return false;}
	virtual bool representsInteger(bool allow_units = false) {return false;}
	virtual bool representsNumber(bool allow_units = false) {return true;}
	virtual bool representsRational(bool allow_units = false) {return false;}
	virtual bool representsReal(bool allow_units = false) {return true;}
	virtual bool representsComplex(bool allow_units = false) {return false;}
	virtual bool representsNonZero(bool allow_units = false) {return true;}
	virtual bool representsEven(bool allow_units = false) {return false;}
	virtual bool representsOdd(bool allow_units = false) {return false;}
	virtual bool representsUndefined(bool include_childs = false, bool include_infinite = false) {return false;}

};

DECLARE_BUILTIN_VARIABLE(PiVariable);
DECLARE_BUILTIN_VARIABLE(EVariable);
DECLARE_BUILTIN_VARIABLE(EulerVariable);
DECLARE_BUILTIN_VARIABLE(CatalanVariable);

#endif
