/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef FUNCTION_H
#define FUNCTION_H

#include "ExpressionItem.h"
#include "includes.h"

enum {
	ARGUMENT_TYPE_FREE,
	ARGUMENT_TYPE_TEXT,
	ARGUMENT_TYPE_DATE,
	ARGUMENT_TYPE_INTEGER,	
	ARGUMENT_TYPE_FRACTION,
	ARGUMENT_TYPE_VECTOR,	
	ARGUMENT_TYPE_MATRIX,
	ARGUMENT_TYPE_EXPRESSION_ITEM,
	ARGUMENT_TYPE_FUNCTION,	
	ARGUMENT_TYPE_UNIT,
	ARGUMENT_TYPE_BOOLEAN,
	ARGUMENT_TYPE_VARIABLE,
	ARGUMENT_TYPE_ANGLE
};

typedef enum {
	ARGUMENT_MIN_MAX_NONE,
	ARGUMENT_MIN_MAX_POSITIVE,
	ARGUMENT_MIN_MAX_NONZERO,
	ARGUMENT_MIN_MAX_NONNEGATIVE,
	ARGUMENT_MIN_MAX_NEGATIVE	
} ArgumentMinMaxPreDefinition;

class Function : public ExpressionItem {

  protected:

	int argc;
	int max_argc;
	vector<string> default_values;
	hash_map<int, Argument*> argdefs;
	int last_argdef_index;
	virtual void calculate(Manager *mngr, vector<Manager*> &vargs);			
	bool testArgumentCount(int itmp);
	bool testArguments(vector<Manager*> &vargs);
	virtual Manager *createFunctionManagerFromVArgs(vector<Manager*> &vargs);
	virtual Manager *createFunctionManagerFromSVArgs(vector<string> &svargs);	
	string scondition;
	
  public:
	Function(string cat_, string name_, int argc_, string title_ = "", string descr_ = "", int max_argc_ = 0, bool is_active = true);
	Function(const Function *function);
	Function();
	virtual ~Function();	

	virtual ExpressionItem *copy() const = 0;
	virtual void set(const ExpressionItem *item);
	virtual int type() const;	
	
	virtual Manager *calculate(const string &eq);
	virtual Manager *calculate(vector<Manager*> &vargs, int counted_args = -1);	
	string condition() const;
	void setCondition(string expression);
	bool testCondition(vector<Manager*> &vargs);
	int args() const;
	int minargs() const;	
	int maxargs() const;		
	int args(const string &str, vector<Manager*> &vargs);
	int lastArgumentDefinitionIndex() const;
	Argument *getArgumentDefinition(int index);
	void clearArgumentDefinitions();
	void setArgumentDefinition(int index, Argument *argdef);
	int stringArgs(const string &str, vector<string> &svargs);		
	void setDefaultValue(int arg_, string value_);
	string getDefaultValue(int arg_) const;	
	Vector *produceVector(vector<Manager*> &vargs, int begin = -1, int end = -1);
	Vector *produceArgumentsVector(vector<Manager*> &vargs, int begin = -1, int end = -1);
};

class UserFunction : public Function {
  protected:
  
	string eq, eq_calc;	
	
  public:
  
	UserFunction(string cat_, string name_, string eq_, bool is_local = true, int argc_ = -1, string title_ = "", string descr_ = "", int max_argc_ = 0, bool is_active = true);
	UserFunction(const UserFunction *function);
	void set(const ExpressionItem *item);
	ExpressionItem *copy() const;
	string equation() const;
	string internalEquation() const;
	void calculate(Manager *mngr, vector<Manager*> &vargs);	
	Manager *calculate(vector<Manager*> &vargs);	
	Manager *calculate(const string &eq);	
	void setEquation(string new_eq, int argc_ = -1, int max_argc_ = 0);	
};

class Argument {

  protected:
  
  	string sname, scondition;
	bool b_zero, b_test, b_matrix;
	virtual bool subtest(const Manager *value) const;
	virtual string subprintlong() const;
	
  public:
  
	Argument(string name_ = "", bool does_test = true);	
	Argument(const Argument *arg);

	virtual void set(const Argument *arg);
	virtual Argument *copy() const;

	virtual string print() const;
	string printlong() const;
	
	bool test(const Manager *value, int index, Function *f) const;
	
	string name() const;
	void setName(string name_);

	void setCustomCondition(string condition);
	string getCustomCondition() const;
	
	bool tests() const;
	void setTests(bool does_test);

	bool zeroForbidden() const;
	void setZeroForbidden(bool forbid_zero);
	
	bool matrixAllowed() const;
	void setMatrixAllowed(bool allow_matrix);
	
	virtual bool needQuotes() const;
	
	virtual int type() const;

};

class FractionArgument : public Argument {

  protected:
  
	Fraction *fmin, *fmax;
	bool b_incl_min, b_incl_max;

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	FractionArgument(string name_ = "", ArgumentMinMaxPreDefinition minmax = ARGUMENT_MIN_MAX_NONE, bool does_test = true);
	FractionArgument(const FractionArgument *arg);
	~FractionArgument();
	
	virtual void set(const Argument *arg);
	virtual Argument *copy() const;

	virtual string print() const;	
	
	void setMin(const Fraction *min_);	
	void setIncludeEqualsMin(bool include_equals);
	bool includeEqualsMin() const;	
	const Fraction *min() const;
	void setMax(const Fraction *max_);	
	void setIncludeEqualsMax(bool include_equals);
	bool includeEqualsMax() const;	
	const Fraction *max() const;	

	virtual int type() const;

};

class IntegerArgument : public Argument {

  protected:
  
	Integer *imin, *imax;

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	IntegerArgument(string name_ = "", ArgumentMinMaxPreDefinition minmax = ARGUMENT_MIN_MAX_NONE, bool does_test = true);
	IntegerArgument(const IntegerArgument *arg);
	~IntegerArgument();

	virtual void set(const Argument *arg);
	virtual Argument *copy() const;

	virtual string print() const;	

	void setMin(const Integer *min_);	
	const Integer *min() const;
	void setMax(const Integer *max_);	
	const Integer *max() const;
	
	virtual int type() const;

};

class TextArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	TextArgument(string name_ = "", bool does_test = true);
	TextArgument(const TextArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	virtual bool needQuotes() const;
	
};
class DateArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	DateArgument(string name_ = "", bool does_test = true);
	DateArgument(const DateArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	virtual bool needQuotes() const;
};
class VectorArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	VectorArgument(string name_ = "", bool does_test = true, bool allow_matrix = false);
	VectorArgument(const VectorArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class MatrixArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	MatrixArgument(string name_ = "", bool does_test = true);
	MatrixArgument(const MatrixArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class ExpressionItemArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	ExpressionItemArgument(string name_ = "", bool does_test = true);
	ExpressionItemArgument(const ExpressionItemArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	virtual bool needQuotes() const;
};
class FunctionArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	FunctionArgument(string name_ = "", bool does_test = true);
	FunctionArgument(const FunctionArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	virtual bool needQuotes() const;
};
class BooleanArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	BooleanArgument(string name_ = "", bool does_test = true);
	BooleanArgument(const BooleanArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class UnitArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	UnitArgument(string name_ = "", bool does_test = true);
	UnitArgument(const UnitArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	virtual bool needQuotes() const;
};
class AngleArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	AngleArgument(string name_ = "", bool does_test = true);
	AngleArgument(const AngleArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class VariableArgument : public Argument {

  protected:
  
	virtual bool subtest(const Manager *value) const;  
	virtual string subprintlong() const;

  public:
  
  	VariableArgument(string name_ = "", bool does_test = true);
	VariableArgument(const VariableArgument *arg);
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	virtual bool needQuotes() const;
};

#endif
