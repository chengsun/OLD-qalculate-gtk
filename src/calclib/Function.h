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
	Calculator *calc;
	int argc;
	int max_argc;
	vector<string> default_values;
	string sname, stitle, sdescr, scat;
	bool bpriv;
	vector<string> sargs;
	vector<Manager*> vargs;
	vector<string> svargs;	
	virtual void calculate2(Manager *mngr);	
	virtual long double calculate3(void);		
  public:
	Function(Calculator *calc_, string cat_, string name_, int argc_, string title_ = "", string descr_ = "", bool priviliged_ = false, int max_argc_ = -1);
	virtual ~Function(void);	
	virtual Manager *calculate(const string &eq);
	bool priviliged(void);
	int args(void);
	int minargs(void);	
	int maxargs(void);		
	string name(void);
	void name(string new_name, bool force = true);
	int args(const string &str);
	int args(const string &str, string *buffer);	
	string category(void);
	void category(string cat_);	
	string description(void);
	void description(string descr_);
	string title(void);
	void title(string title_);	
	string argName(int index);
	void clearArgNames(void);
	void addArgName(string name_);
	bool setArgName(string name_, int index);
	virtual bool isUserFunction(void);	
	int stringArgs(const string &str);		
	void setDefaultValue(int arg_, string value_);
	string getDefaultValue(int arg_);	
};

class UserFunction : public Function {
  protected:
	string eq, eq_calc;	
  public:
	UserFunction(Calculator *calc_, string cat_, string name_, string eq_, int argc_ = -1, string title_ = "", string descr_ = "", int max_argc_ = -1);
	string equation(void);
	Manager *calculate(const string &argv);	
	void equation(string new_eq, int argc_ = -1, int max_argc_ = -1);	
	bool isUserFunction(void);
};

#endif
