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

	string sname, uname, scat, stitle, sdescr;
	bool b_local, b_changed, b_builtin, b_approx, b_active, b_registered, b_hidden, b_destroyed;
	int i_ref;
	vector<ExpressionItem*> v_refs;

  public:

	ExpressionItem(string cat_, string name_, string title_ = "", string descr_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true, string unicode_name = "");
	ExpressionItem();
	virtual ~ExpressionItem();
	
	virtual ExpressionItem *copy() const = 0;
	virtual void set(const ExpressionItem *item);
	
	virtual bool destroy();

	bool isRegistered() const;
	void setRegistered(bool is_registered);

	virtual void setName(string name_, bool force = true);
	virtual void setUnicodeName(string name_, bool force = true);
	
	virtual const string &name(bool use_unicode = false) const;
	virtual const string &unicodeName(bool return_name_if_no_unicode = true) const;
	virtual const string &referenceName() const;
	
	virtual const string &title(bool return_name_if_no_title = true, bool use_unicode = false) const;
	
	virtual void setTitle(string title_);		
	
	virtual const string &description() const;
	virtual void setDescription(string descr_);

	virtual const string &category() const;
	
	virtual void setCategory(string cat_);		

	virtual bool hasChanged() const;
	virtual void setChanged(bool has_changed);
	
	virtual bool isLocal() const;
	virtual bool setLocal(bool is_local = true, int will_be_active = -1);
	
	virtual bool isBuiltin() const;
	
	virtual bool isApproximate() const;
	virtual void setApproximate(bool is_approx = true);
	
	virtual bool isActive() const;
	virtual void setActive(bool is_active);
	
	virtual bool isHidden() const;
	virtual void setHidden(bool is_hidden);
	
	virtual int refcount() const;
	virtual void ref();
	virtual void unref();
	virtual void ref(ExpressionItem *o);
	virtual void unref(ExpressionItem *o);
	virtual ExpressionItem *getReferencer(unsigned int index = 1) const;
	virtual bool changeReference(ExpressionItem *o_from, ExpressionItem *o_to);
	
	virtual int type() const = 0;
};

#endif
