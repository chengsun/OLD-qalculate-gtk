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


/**
* Contains a known variable.
*/

class Variable : public ExpressionItem {

  protected:

	Manager *mngr;
	bool b_expression;
 	int calculated_precision;
	string sexpression;

  public:
  
	Variable(string cat_, string name_, Manager *mngr_, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	Variable(string cat_, string name_, string expression_, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);	
	Variable();
	Variable(const Variable *variable);
	~Variable();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	virtual int type() const;
	virtual bool isExpression() const;
	virtual string expression() const;

	/**
	* Sets the value of the variable.
	*
	* @see #value
	*/
	virtual void set(Manager *mngr_);
	virtual void set(string expression_);	

	/**
	* Returns the value of the variable.
	*/	
	virtual Manager *get();

	/**
	* Returns the value of the variable.
	*/	
	virtual Manager *copyManager() const;	

};

class DynamicVariable : public Variable {

  protected:
  
  	virtual void calculate() const = 0;
  	
  public:

	DynamicVariable(string cat_, string name_, string title_ = "", bool is_local = false, bool is_builtin = true, bool is_active = true);
	DynamicVariable(const DynamicVariable *variable);
	DynamicVariable();
	virtual ~DynamicVariable();

	ExpressionItem *copy() const = 0;
	void set(const ExpressionItem *item);

	/**
	* Returns the value of the variable.
	*/	
	virtual Manager *get();
	
	virtual Manager *copyManager() const;		

	void set(Manager *mngr_);
	void set(string expression_);	
	
	int calculatedPrecision() const;

};

DECLARE_BUILTIN_VARIABLE(PiVariable);
DECLARE_BUILTIN_VARIABLE(EVariable);
DECLARE_BUILTIN_VARIABLE(EulerVariable);
DECLARE_BUILTIN_VARIABLE(CatalanVariable);
DECLARE_BUILTIN_VARIABLE(AperyVariable);
DECLARE_BUILTIN_VARIABLE(PythagorasVariable);
DECLARE_BUILTIN_VARIABLE(GoldenVariable);

#endif
