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

/**
* Contains a known variable.
*/

class Variable : public ExpressionItem {

  protected:

	Manager *mngr;

  public:
  
	Variable(string cat_, string name_, Manager *mngr_, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	Variable(string cat_, string name_, string expression_, string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);	
	Variable(const Variable *variable);
	~Variable();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	virtual int type() const;

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
	virtual const Manager *get() const;	

};

#endif
