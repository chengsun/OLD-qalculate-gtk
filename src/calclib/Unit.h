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

enum {
	BASE_UNIT,
	ALIAS_UNIT,
	COMPOSITE_UNIT
};	

#include "ExpressionItem.h"
#include "includes.h"

class Unit : public ExpressionItem {

  protected:

	string sshortname, splural;

  public:

	Unit(string cat_, string name_, string plural_ = "", string short_name_ = "", string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	Unit();	
	Unit(const Unit *unit);	
	virtual ~Unit();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	
	void setPlural(string name_, bool force = true);
	void setShortName(string name_, bool force = true);
	virtual string referenceName() const;
	virtual string name() const;
	virtual string plural(bool return_name_if_no_plural = true) const;
	virtual string shortName(bool return_name_if_no_short = true, bool plural_ = false) const;
	virtual bool isUsedByOtherUnits() const;
	virtual const Unit* baseUnit() const;
	virtual string baseName() const;
	virtual string baseExpName() const;
	virtual string shortBaseName() const;
	virtual string shortBaseExpName() const;
	virtual Manager *baseValue(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *convertToBase(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual long int baseExp(long int exp_ = 1) const;
	virtual int type() const;
	virtual int unitType() const;
	virtual bool isChildOf(const Unit *u) const;
	virtual bool isParentOf(const Unit *u) const;
	virtual bool hasComplexRelationTo(const Unit *u) const;	
	Manager *convert(const Unit *u, Manager *value_ = NULL, Manager *exp_ = NULL, bool *converted = NULL) const;

};

class AliasUnit : public Unit {

  protected:

	string value, rvalue;
	long int exp;
	Manager *exp_mngr;
	Unit *unit;

  public:

	AliasUnit(string cat_, string name_, string plural_, string short_name_, string title_, const Unit *alias, string relation = "1", long int exp_ = 1, string reverse = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	AliasUnit(const AliasUnit *unit);		
	AliasUnit();			
	virtual ~AliasUnit();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	
	virtual string baseName() const;
	virtual string baseExpName() const;
	virtual string shortBaseName() const;
	virtual string shortBaseExpName() const;
	virtual string firstBaseName() const;
	virtual string firstBaseExpName() const;
	virtual string firstShortBaseName() const;
	virtual string firstShortBaseExpName() const;
	virtual const Unit* baseUnit() const;
	virtual const Unit* firstBaseUnit() const;
	virtual void setBaseUnit(const Unit *alias);
	virtual string expression() const;
	virtual string reverseExpression() const;
	virtual void setExpression(string relation);
	virtual void setReverseExpression(string reverse);
	virtual Manager *baseValue(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *convertToBase(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *firstBaseValue(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual Manager *convertToFirstBase(Manager *value_ = NULL, Manager *exp_ = NULL) const;
	virtual long int baseExp(long int exp_ = 1) const;
	virtual void setExponent(long int exp_);
	virtual long int firstBaseExp() const;
	virtual int unitType() const;
	virtual bool isChildOf(const Unit *u) const;
	virtual bool isParentOf(const Unit *u) const;
	virtual bool hasComplexExpression() const;
	virtual bool hasComplexRelationTo(const Unit *u) const;

};

class AliasUnit_Composite : public AliasUnit {

  protected:

	Prefix *prefixv;

  public:
	AliasUnit_Composite(const Unit *alias, long int exp_ = 1, const Prefix *prefix_ = NULL);
	AliasUnit_Composite(const AliasUnit_Composite *unit);			
	virtual ~AliasUnit_Composite();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	
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

		CompositeUnit(string cat_, string name_, string title_ = "", string base_expression_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
		CompositeUnit(const CompositeUnit *unit);
		virtual ~CompositeUnit();
		virtual ExpressionItem *copy() const;
		virtual void set(const ExpressionItem *item);		
		virtual void add(const Unit *u, long int exp_ = 1, const Prefix *prefix = NULL);
		virtual Unit *get(int index, long int *exp_ = NULL, Prefix **prefix = NULL) const;
		virtual int countUnits() const;
		virtual void del(Unit *u);
		virtual string print(bool plural_, bool short_) const;
		virtual string name() const;
		virtual string plural(bool return_name_if_no_plural = true) const;
		virtual string shortName(bool return_name_if_no_short = true, bool plural_ = false) const;
		virtual int unitType() const;
		virtual bool containsRelativeTo(const Unit *u) const;
		virtual Manager *generateManager(bool cleaned = true) const;		
		virtual string referenceName() const;
		virtual void setBaseExpression(string base_expression_);		
};

#endif
