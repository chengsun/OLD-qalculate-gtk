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

/**
* Contains a known variable.
*/

class Variable {
  protected:
	string sname, scat, stitle;
	Manager *mngr;
	bool b_user, b_changed, b_builtin;
  public:
	Variable(string cat_, string name_, Manager *mngr_, string title_ = "", bool uservariable_ = true, bool is_builtin = false);
	Variable(string cat_, string name_, long double value_, string title_ = "", bool uservariable_ = true, bool is_builtin = false);	
	~Variable(void);

	/**
	* Sets the value of the variable.
	*
	* @see #value
	*/
	void set(Manager *mngr_);
	/**
	* Returns the value of the variable.
	*/	
	Manager *get(void);
	/**
	* Sets the name of the variable.
	*/	
	void setName(string name_, bool force = true);
	/**
	* Returns the name of the variable.
	*/		
	string name(void);
	/**
	* Returns the title/descriptive name of the variable.
	*/		
	string title(bool return_name_if_no_title = true);
	/**
	* Sets the title/descriptive name of the variable.
	*/			
	void setTitle(string title_);		
	/**
	* Sets the value of the variable.
	*
	* @see #set
	*/			
	void setValue(long double value_);
	/**
	* Returns the category of the variable.
	*/			
	string category(void);
	/**
	* Sets the category of the variable.
	*/			
	void setCategory(string cat_);		
	/**
	* Tells if the variable is editable for the end user.
	*/			
	bool isUserVariable(void);
	bool hasChanged();
	bool setUserVariable(bool is_user_var);
	bool setChanged(bool has_changed);
	bool isBuiltinVariable();

};

#endif
