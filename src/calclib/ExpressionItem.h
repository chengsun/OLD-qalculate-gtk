/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef EXPRESSIONITEM_H
#define EXPRESSIONITEM_H

#include "includes.h"

/**
* Base class for functions, variables and units.
*/

class ExpressionItem {

  protected:

	string sname, scat, stitle, sdescr;
	bool b_local, b_changed, b_builtin, b_exact, b_active, b_registered;

  public:

	ExpressionItem(string cat_, string name_, string title_ = "", string descr_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	ExpressionItem();
	virtual ~ExpressionItem();
	
	virtual ExpressionItem *copy() const = 0;
	virtual void set(const ExpressionItem *item);
	
	virtual bool destroy();

	bool isRegistered() const;
	void setRegistered(bool is_registered);

	/**
	* Sets the name of the ExpressionItem.
	*/	
	virtual void setName(string name_, bool force = true);
	
	/**
	* Returns the name of the ExpressionItem.
	*/		
	virtual string name() const;
	virtual string referenceName() const;
	
	/**
	* Returns the title/descriptive name of the ExpressionItem.
	*/		
	virtual string title(bool return_name_if_no_title = true) const;
	
	/**
	* Sets the title/descriptive name of the ExpressionItem.
	*/			
	virtual void setTitle(string title_);		
	
	virtual string description() const;
	virtual void setDescription(string descr_);

	/**
	* Returns the category of the ExpressionItem.
	*/			
	virtual string category() const;
	
	/**
	* Sets the category of the ExpressionItem.
	*/			
	virtual void setCategory(string cat_);		

	virtual bool hasChanged() const;
	virtual bool setChanged(bool has_changed);
	
	/**
	* Tells if the ExpressionItem is edited/created bt the end user.
	*/				
	virtual bool isLocal() const;
	virtual bool setLocal(bool is_local = true, int will_be_active = -1);
	
	virtual bool isBuiltin() const;
	
	virtual bool isPrecise() const;
	virtual void setPrecise(bool is_precise);
	
	virtual bool isActive() const;
	virtual void setActive(bool is_active);
	
	virtual int type() const = 0;
};

#endif
