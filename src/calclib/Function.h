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

class Function;
class UserFunction;

#include "Calculator.h"

class Function {
  protected:
  	bool b_exact;
	int argc;
	int max_argc;
	vector<string> default_values;
	string sname, stitle, sdescr, scat;
	bool b_user, b_changed;
	bool bpriv;
	vector<string> sargs;
	vector<Manager*> vargs;
	vector<string> svargs;	
	virtual void calculate2(Manager *mngr);	
	virtual long double calculate3(void);		
	bool testArgCount(int itmp);
	virtual Manager *createFunctionManagerFromVArgs(int itmp);
	virtual Manager *createFunctionManagerFromSVArgs(int itmp);	
	virtual void clearVArgs();
	virtual void clearSVArgs();	
  public:
	Function(string cat_, string name_, int argc_, string title_ = "", string descr_ = "", bool priviliged_ = false, int max_argc_ = 0);
	virtual ~Function(void);	
	virtual Manager *calculate(const string &eq);
	bool priviliged(void) const;
	int args(void) const;
	int minargs(void) const;	
	int maxargs(void) const;		
	string name(void) const;
	void setName(string new_name, bool force = true);
	int args(const string &str);
	int args(const string &str, string *buffer);	
	string category(void) const;
	void setCategory(string cat_);	
	string description(void) const;
	void setDescription(string descr_);
	string title(bool return_name_if_no_title = true) const;
	void setTitle(string title_);	
	string argName(int index) const;
	void clearArgNames(void);
	void addArgName(string name_);
	bool setArgName(string name_, int index);
	virtual bool isUserFunction(void) const;	
	virtual bool isBuiltinFunction(void) const;		
	virtual bool hasChanged(void) const;
	virtual void setUserFunction(bool is_user_function);	
	virtual void setChanged(bool has_changed);	
	int stringArgs(const string &str);		
	void setDefaultValue(int arg_, string value_);
	string getDefaultValue(int arg_) const;	
	bool isPrecise() const;
	void setPrecise(bool is_precise);	
};

class UserFunction : public Function {
  protected:
	string eq, eq_calc;	
  public:
	UserFunction(string cat_, string name_, string eq_, bool is_user_function = true, int argc_ = -1, string title_ = "", string descr_ = "", int max_argc_ = 0);
	string equation(void) const;
	Manager *calculate(const string &argv);	
	void setEquation(string new_eq, int argc_ = -1, int max_argc_ = 0);	
	bool isBuiltinFunction(void) const;
};

#endif
