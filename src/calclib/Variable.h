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

class Variable;

#include "Calculator.h"

class Variable {
  protected:
	Calculator *calc;  
	string sname, scat, stitle;
	Manager *mngr;
	bool buservariable;
  public:
	Variable(Calculator *calc_, string cat_, string name_, Manager *mngr_, string title_ = "", bool uservariable_ = true);
	Variable(Calculator *calc_, string cat_, string name_, long double value_, string title_ = "", bool uservariable_ = true);	
	~Variable(void);
	void set(Manager *mngr_);
	Manager *get(void);
	void name(string name_, bool force = true);
	string name(void);
	string title(void);
	void title(string title_);		
	void value(long double value_);
	string category(void);
	void category(string cat_);		
	bool isUserVariable(void);
};

#endif
