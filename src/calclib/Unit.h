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
	string title(bool return_name_if_no_title = true) const;
	void setCategory(string cat_);
	void setName(string name_, bool force = true);
	void setPlural(string name_, bool force = true);
	void setShortName(string name_, bool force = true);
	string category(void) const;
	virtual string name(void) const;
	virtual string plural(bool return_name_if_no_plural = true) const;
	virtual string shortName(bool return_name_if_no_short = true, bool plural_ = false) const;
	virtual bool isUsedByOtherUnits(void) const;
	virtual const Unit* baseUnit(void) const;
	virtual string baseName(void) const;
	virtual string baseExpName(void) const;
	virtual string shortBaseName(void) const;
	virtual string shortBaseExpName(void) const;
	virtual Manager *baseValue(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *convertToBase(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual long int baseExp(long int exp_ = 1) const;
	virtual char type() const;
	virtual bool isChildOf(const Unit *u) const;
	virtual bool isParentOf(const Unit *u) const;
	virtual bool hasComplexRelationTo(const Unit *u) const;	
	Manager *convert(const Unit *u, Manager *value_ = NULL, Manager *exp_ = NULL, bool *converted = NULL) const;
	bool isUserUnit() const;
	bool hasChanged() const;
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
	AliasUnit(string cat_, string name_, string plural_, string short_name_, string title_, const Unit *alias, string relation = "1", long int exp_ = 1, string reverse = "", bool is_user_unit = true);
	~AliasUnit(void);
	virtual string baseName(void) const;
	virtual string baseExpName(void) const;
	virtual string shortBaseName(void) const;
	virtual string shortBaseExpName(void) const;
	virtual string firstBaseName(void) const;
	virtual string firstBaseExpName(void) const;
	virtual string firstShortBaseName(void) const;
	virtual string firstShortBaseExpName(void) const;
	virtual const Unit* baseUnit(void) const;
	virtual const Unit* firstBaseUnit(void) const;
	virtual void setBaseUnit(const Unit *alias);
	virtual string expression(void) const;
	virtual string reverseExpression(void) const;
	virtual void setExpression(string relation);
	virtual void setReverseExpression(string reverse);
	virtual Manager *baseValue(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *convertToBase(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *firstBaseValue(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *convertToFirstBase(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual long int baseExp(long int exp_ = 1) const;
	virtual void setExponent(long int exp_);
	virtual long int firstBaseExp(void) const;
	virtual char type() const;
	virtual bool isChildOf(const Unit *u) const;
	virtual bool isParentOf(const Unit *u) const;
	virtual bool hasComplexExpression(void) const;
	virtual bool hasComplexRelationTo(const Unit *u) const;
	virtual bool isPrecise() const;
	virtual void setPrecise(bool is_precise);	
//	virtual Manager *convert(Unit *u, Manager *value_ = NULL, long double exp_ = 1.0L);
};

class AliasUnit_Composite : public AliasUnit {
  protected:
	Prefix *prefixv;
  public:
	AliasUnit_Composite(const Unit *alias, long int exp_ = 1, const Prefix *prefix_ = NULL);
	virtual ~AliasUnit_Composite(void);
	virtual string print(bool plural_) const;
	virtual string printShort(bool plural_) const;
	virtual const Prefix *prefix() const;
	virtual long int prefixExponent() const;	
	virtual void set(const Unit *u, long int exp_ = 1, const Prefix *prefix_ = NULL);
	virtual Manager *firstBaseValue(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *convertToFirstBase(Manager *value_ = NULL, Manager *exp_ = NULL) const;
};

class CompositeUnit : public Unit {

	public:
		//--------internal-------------//
		vector<AliasUnit_Composite*> units;
		//-----------------------------//

		CompositeUnit(string cat_, string name_, string title_ = "", string base_expression_ = "", bool is_user_unit = true);
		virtual ~CompositeUnit(void);
		virtual void add(const Unit *u, long int exp_ = 1, const Prefix *prefix = NULL);
		virtual Unit *get(int index, long int *exp_ = NULL, Prefix **prefix = NULL) const;
		virtual int countUnits() const;
		virtual void del(Unit *u);
		virtual string print(bool plural_, bool short_) const;
		virtual string name(void) const;
		virtual string plural(bool return_name_if_no_plural = true) const;
		virtual string shortName(bool return_name_if_no_short = true, bool plural_ = false) const;
		virtual char type() const;
		virtual bool containsRelativeTo(const Unit *u) const;
		virtual Manager *generateManager(bool cleaned = true) const;		
		virtual string internalName() const;
		virtual void setBaseExpression(string base_expression_);		
};

#endif
