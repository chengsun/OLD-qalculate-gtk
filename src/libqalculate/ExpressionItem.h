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

#include <libqalculate/includes.h>

/**
* Base class for functions, variables and units.
*/

struct ExpressionName {
	
	bool abbreviation, suffix, unicode, plural, reference, avoid_input, case_sensitive;
	string name;
	
	ExpressionName();
	ExpressionName(string sname);
	
	void operator = (const ExpressionName &ename);
	bool operator == (const ExpressionName &ename) const;
	bool operator != (const ExpressionName &ename) const;
	
};

class ExpressionItem {

  protected:

	string scat, stitle, sdescr;
	bool b_local, b_changed, b_builtin, b_approx, b_active, b_registered, b_hidden, b_destroyed;
	int i_ref, i_precision;
	vector<ExpressionItem*> v_refs;
	vector<ExpressionName> names;

  public:

	ExpressionItem(string cat_, string name_, string title_ = "", string descr_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	ExpressionItem();
	virtual ~ExpressionItem();
	
	virtual ExpressionItem *copy() const = 0;
	virtual void set(const ExpressionItem *item);
	
	virtual bool destroy();

	bool isRegistered() const;
	void setRegistered(bool is_registered);

	virtual const string &name(bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual const string &referenceName() const;
	
	virtual const ExpressionName &preferredName(bool abbreviation = false, bool use_unicode = false, bool plural = false, bool reference = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual const ExpressionName &preferredInputName(bool abbreviation = false, bool use_unicode = false, bool plural = false, bool reference = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual const ExpressionName &preferredDisplayName(bool abbreviation = false, bool use_unicode = false, bool plural = false, bool reference = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual const ExpressionName &getName(size_t index) const;
	virtual void setName(const ExpressionName &ename, size_t index = 1, bool force = true);
	virtual void setName(string sname, size_t index, bool force = true);
	virtual void addName(const ExpressionName &ename, size_t index = 0, bool force = true);
	virtual void addName(string sname, size_t index = 0, bool force = true);
	virtual size_t countNames() const;
	virtual void clearNames();
	virtual void clearNonReferenceNames();
	virtual void removeName(size_t index);
	virtual bool hasName(const string &sname) const;
	virtual bool hasNameCaseSensitive(const string &sname) const;
	virtual const ExpressionName &findName(int abbreviation = -1, int use_unicode = -1, int plural = -1, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	
	virtual const string &title(bool return_name_if_no_title = true, bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	
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
	
	virtual int precision() const;
	virtual void setPrecision(int prec);
	
	virtual bool isActive() const;
	virtual void setActive(bool is_active);
	
	virtual bool isHidden() const;
	virtual void setHidden(bool is_hidden);
	
	virtual int refcount() const;
	virtual void ref();
	virtual void unref();
	virtual void ref(ExpressionItem *o);
	virtual void unref(ExpressionItem *o);
	virtual ExpressionItem *getReferencer(size_t index = 1) const;
	virtual bool changeReference(ExpressionItem *o_from, ExpressionItem *o_to);
	
	virtual int type() const = 0;
	virtual int subtype() const = 0;
};

#endif
