/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef UNIT_H
#define UNIT_H

class Unit;
class AliasUnit;
class AliasUnit_Composite;
class CompositeUnit;

#include "Calculator.h"
#include "Manager.h"
#include "Prefix.h"

class Unit {
  protected:
	string scategory, sname, sshortname, stitle, splural;
	bool b_user, b_changed;
  public:
	Unit(string cat_, string name_, string plural_ = "", string short_name_ = "", string title_ = "", bool is_user_unit = true);
	virtual ~Unit(void);
	void setTitle(string title_);
	string title(bool return_name_if_no_title = true);
	void setCategory(string cat_);
	void setName(string name_, bool force = true);
	void setPlural(string name_, bool force = true);
	void setShortName(string name_, bool force = true);
	string category(void);
	virtual string name(void);
	virtual string plural(bool return_name_if_no_plural = true);
	virtual string shortName(bool return_name_if_no_short = true, bool plural_ = false);
	virtual bool isUsedByOtherUnits(void);
	virtual Unit* baseUnit(void);
	virtual string baseName(void);
	virtual string baseExpName(void);
	virtual string shortBaseName(void);
	virtual string shortBaseExpName(void);
	virtual Manager *baseValue(Manager *value_ = NULL, Manager *exp_ = NULL);
	virtual Manager *convertToBase(Manager *value_ = NULL, Manager *exp_ = NULL);
	virtual long int baseExp(long int exp_ = 1);
	virtual char type() const;
	virtual bool isChildOf(Unit *u);
	virtual bool isParentOf(Unit *u);
	virtual bool hasComplexRelationTo(Unit *u);	
	Manager *convert(Unit *u, Manager *value_ = NULL, Manager *exp_ = NULL, bool *converted = NULL);
	bool isUserUnit();
	bool hasChanged();
	void setUserUnit(bool is_user_unit);
	void setChanged(bool has_changed);	
};

class AliasUnit : public Unit {
  protected:
	string value, rvalue;
	long int exp;
	Manager *exp_mngr;
	Unit *unit;
	bool b_exact;
  public:
	AliasUnit(string cat_, string name_, string plural_, string short_name_, string title_, Unit *alias, string relation = "1", long int exp_ = 1, string reverse = "", bool is_user_unit = true);
	~AliasUnit(void);
	virtual string baseName(void);
	virtual string baseExpName(void);
	virtual string shortBaseName(void);
	virtual string shortBaseExpName(void);
	virtual string firstBaseName(void);
	virtual string firstBaseExpName(void);
	virtual string firstShortBaseName(void);
	virtual string firstShortBaseExpName(void);
	virtual Unit* baseUnit(void);
	virtual Unit* firstBaseUnit(void);
	virtual void setBaseUnit(Unit *alias);
	virtual string expression(void);
	virtual string reverseExpression(void);
	virtual void setExpression(string relation);
	virtual void setReverseExpression(string reverse);
	virtual Manager *baseValue(Manager *value_ = NULL, Manager *exp_ = NULL);
	virtual Manager *convertToBase(Manager *value_ = NULL, Manager *exp_ = NULL);
	virtual Manager *firstBaseValue(Manager *value_ = NULL, Manager *exp_ = NULL);
	virtual Manager *convertToFirstBase(Manager *value_ = NULL, Manager *exp_ = NULL);
	virtual long int baseExp(long int exp_ = 1);
	virtual void setExponent(long int exp_);
	virtual long int firstBaseExp(void);
	virtual char type() const;
	virtual bool isChildOf(Unit *u);
	virtual bool isParentOf(Unit *u);
	virtual bool hasComplexExpression(void);
	virtual bool hasComplexRelationTo(Unit *u);
	virtual bool isPrecise() const;
	virtual void setPrecise(bool is_precise);	
//	virtual Manager *convert(Unit *u, Manager *value_ = NULL, long double exp_ = 1.0L);
};

class AliasUnit_Composite : public AliasUnit {
  protected:
	Prefix *prefixv;
  public:
	AliasUnit_Composite(Unit *alias, long int exp_ = 1, Prefix *prefix_ = NULL);
	virtual ~AliasUnit_Composite(void);
	virtual string print(bool plural_);
	virtual string printShort(bool plural_);
	virtual Prefix *prefix() const;
	virtual long int prefixExponent() const;	
	virtual void set(Unit *u, long int exp_ = 1, Prefix *prefix_ = NULL);
	virtual Manager *firstBaseValue(Manager *value_ = NULL, Manager *exp_ = NULL);
	virtual Manager *convertToFirstBase(Manager *value_ = NULL, Manager *exp_ = NULL);
};

class CompositeUnit : public Unit {

	public:
		//--------internal-------------//
		vector<AliasUnit_Composite*> units;
		//-----------------------------//

		CompositeUnit(string cat_, string name_, string title_ = "", string base_expression_ = "", bool is_user_unit = true);
		virtual ~CompositeUnit(void);
		virtual void add(Unit *u, long int exp_ = 1, Prefix *prefix = NULL);
		virtual Unit *get(int index, long int *exp_ = NULL, Prefix **prefix = NULL);
		virtual void del(Unit *u);
		virtual string print(bool plural_, bool short_);
		virtual string name(void);
		virtual string plural(bool return_name_if_no_plural = true);
		virtual string shortName(bool return_name_if_no_short = true, bool plural_ = false);
		virtual char type() const;
		virtual bool containsRelativeTo(Unit *u);
		virtual Manager *generateManager(bool cleaned = true);		
		virtual string internalName();
		virtual void setBaseExpression(string base_expression_);		
};

#endif
