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

typedef enum {
	ARGUMENT_TYPE_FREE,
	ARGUMENT_TYPE_TEXT,
	ARGUMENT_TYPE_DATE,
	ARGUMENT_TYPE_POSITIVE,
	ARGUMENT_TYPE_NONNEGATIVE,
	ARGUMENT_TYPE_NONZERO,	
	ARGUMENT_TYPE_INTEGER,	
	ARGUMENT_TYPE_POSITIVE_INTEGER,
	ARGUMENT_TYPE_NONNEGATIVE_INTEGER,	
	ARGUMENT_TYPE_NONZERO_INTEGER,		
	ARGUMENT_TYPE_FRACTION,
	ARGUMENT_TYPE_VECTOR,	
	ARGUMENT_TYPE_MATRIX,
	ARGUMENT_TYPE_FUNCTION,	
	ARGUMENT_TYPE_UNIT,
	ARGUMENT_TYPE_BOOLEAN
} ArgumentType;


class Function : public ExpressionItem {

  protected:

	int argc;
	int max_argc;
	vector<string> default_values;
	hash_map<int, string> argnames;
	hash_map<int, ArgumentType> argtypes;	
	int last_arg_name_index, last_arg_type_index;
	virtual void calculate(Manager *mngr, vector<Manager*> &vargs);			
	bool testArgumentCount(int itmp);
	virtual Manager *createFunctionManagerFromVArgs(vector<Manager*> &vargs);
	virtual Manager *createFunctionManagerFromSVArgs(vector<string> &svargs);	
	
  public:
	Function(string cat_, string name_, int argc_, string title_ = "", string descr_ = "", int max_argc_ = 0, bool is_active = true);
	Function(const Function *function);
	Function();
	virtual ~Function();	

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	virtual int type() const;	
	
	virtual Manager *calculate(const string &eq);
	virtual Manager *calculate(vector<Manager*> &vargs, int counted_args = -1);	
	int args() const;
	int minargs() const;	
	int maxargs() const;		
	int args(const string &str, vector<Manager*> &vargs);
	int lastArgumentNameIndex() const;
	int lastArgumentTypeIndex() const;	
	string argumentName(int index);
	ArgumentType argumentType(int index);	
	const char *argumentTypeString(int index);		
	void clearArgumentNames();
	void clearArgumentTypes();	
	void setArgumentName(string name_, int index);
	void setArgumentType(ArgumentType type_, int index);	
	int stringArgs(const string &str, vector<string> &svargs);		
	void setDefaultValue(int arg_, string value_);
	string getDefaultValue(int arg_) const;	
	Vector *produceVector(vector<Manager*> &vargs, int begin = -1, int end = -1);
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

#endif
