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

class Unit {
  protected:
	string scategory, sname, sshortname, stitle, splural;
	Calculator *calc;
  public:
	Unit(Calculator *calc_, string cat_, string name_, string plural_ = "", string short_name_ = "", string title_ = "");
	virtual ~Unit(void);
	void title(string title_);
	string title(void);
	void category(string cat_);
	void name(string name_, bool force = true);
	void plural(string name_, bool force = true);
	void shortName(string name_, bool force = true);
	string category(void);
	virtual string name(void);
	virtual string plural(void);
	virtual string shortName(bool plural_ = false);
	virtual bool hasShortName(void);
	virtual bool hasPlural(void);
	virtual bool isUsedByOtherUnits(void);
	virtual Unit* baseUnit(void);
	virtual string baseName(void);
	virtual string baseExpName(void);
	virtual string shortBaseName(void);
	virtual string shortBaseExpName(void);
	virtual Manager *baseValue(Manager *value_ = NULL, long double exp_ = 1.0L);
	virtual Manager *convertToBase(Manager *value_ = NULL, long double exp_ = 1.0L);
	virtual long double baseExp(long double exp_ = 1.0L);
	virtual char type() const;
	virtual bool isChildOf(Unit *u);
	virtual bool isParentOf(Unit *u);
	Manager *convert(Unit *u, Manager *value_ = NULL, long double exp_ = 1, bool *converted = NULL);
};

class AliasUnit : public Unit {
  protected:
	string value, rvalue;
	long double d_exp;
	Unit *unit;
  public:
	AliasUnit(Calculator *calc_, string cat_, string name_, string plural_, string short_name_, string title_, Unit *alias, string relation = "1", long double exp_ = 1.0L, string reverse = "");
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
	virtual void baseUnit(Unit *alias);
	virtual string expression(void);
	virtual string reverseExpression(void);
	virtual void expression(string relation);
	virtual void reverseExpression(string reverse);
	virtual Manager *baseValue(Manager *value_ = NULL, long double exp_ = 1.0L);
	virtual Manager *convertToBase(Manager *value_ = NULL, long double exp_ = 1.0L);
	virtual Manager *firstBaseValue(Manager *value_ = NULL, long double exp_ = 1.0L);
	virtual Manager *convertToFirstBase(Manager *value_ = NULL, long double exp_ = 1.0L);
	virtual long double baseExp(long double exp_ = 1.0L);
	virtual void exp(long double exp_);
	virtual long double firstBaseExp(void);
	virtual char type() const;
	virtual bool isChildOf(Unit *u);
	virtual bool isParentOf(Unit *u);
//	virtual Manager *convert(Unit *u, Manager *value_ = NULL, long double exp_ = 1.0L);
};

class AliasUnit_Composite : public AliasUnit {
  protected:
	long double prefixv;
  public:
	AliasUnit_Composite(Calculator *calc_, Unit *alias, long double exp_ = 1.0L, long double prefix = 1.0L);
	virtual ~AliasUnit_Composite(void);
	virtual string print(bool plural_);
	virtual string printShort(bool plural_);
	virtual long double prefixValue(void);
	virtual void set(Unit *u, long double exp_ = 1.0L, long double prefix = 1.0L);
	virtual Manager *firstBaseValue(Manager *value_ = NULL, long double exp_ = 1.0L);
	virtual Manager *convertToFirstBase(Manager *value_ = NULL, long double exp_ = 1.0L);
};

class CompositeUnit : public Unit {

	public:
		//--------internal-------------//
		vector<AliasUnit_Composite*> units;
		vector<int> sorted;
		bool bsorted;
		//-----------------------------//

		CompositeUnit(Calculator *calc_, string cat_, string title_ = "");
		virtual ~CompositeUnit(void);
		virtual void add(Unit *u, long double exp_ = 1.0L, long double prefix = 1.0L);
		virtual Unit *get(int index, long double *exp_ = NULL, long double *prefix = NULL);
		virtual void del(Unit *u);
		virtual string print(bool plural_, bool short_);
		virtual string name(void);
		virtual string plural(void);
		virtual string shortName(bool plural_ = false);
		virtual void sort(void);
		virtual bool hasShortName(void);
		virtual bool hasPlural(void);
		virtual char type() const;
};

#endif
