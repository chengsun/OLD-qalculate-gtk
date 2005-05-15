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
	SUBTYPE_BASE_UNIT,
	SUBTYPE_ALIAS_UNIT,
	SUBTYPE_COMPOSITE_UNIT
};	

#include <libqalculate/ExpressionItem.h>
#include <libqalculate/includes.h>

class Unit : public ExpressionItem {

  protected:

	string ssystem;
	bool b_si;

  public:

	Unit(string cat_, string name_, string plural_ = "", string singular_ = "", string title_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	Unit();	
	Unit(const Unit *unit);	
	virtual ~Unit();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);

	bool isSIUnit() const;
	void setAsSIUnit();
	void setSystem(string s_system);
	const string &system() const;
	bool isCurrency() const;
	void setPlural(string name_, bool force = true);
	void setSingular(string name_, bool force = true);
	virtual string print(bool plural_, bool short_, bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual const string &plural(bool return_singular_if_no_plural = true, bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual const string &singular(bool return_short_if_no_singular = true, bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual const string &shortName(bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual bool isUsedByOtherUnits() const;
	virtual Unit* baseUnit() const;
	virtual MathStructure &baseValue(MathStructure &mvalue, MathStructure &mexp) const;
	virtual MathStructure &convertToBase(MathStructure &mvalue, MathStructure &mexp) const;
	virtual MathStructure &baseValue(MathStructure &mvalue) const;
	virtual MathStructure &convertToBase(MathStructure &mvalue) const;
	virtual MathStructure baseValue() const;
	virtual MathStructure convertToBase() const;
	virtual int baseExp(int exp_ = 1) const;
	virtual int type() const;
	virtual int subtype() const;
	virtual bool isChildOf(Unit *u) const;
	virtual bool isParentOf(Unit *u) const;
	virtual bool hasComplexRelationTo(Unit *u) const;	
	MathStructure &convert(Unit *u, MathStructure &mvalue, MathStructure &exp, bool *converted = NULL) const;
	MathStructure &convert(Unit *u, MathStructure &mvalue, bool *converted = NULL) const;
	MathStructure convert(Unit *u, bool *converted = NULL) const;

};

class AliasUnit : public Unit {

  protected:

	string value, rvalue;
	int exp;
	Unit *unit;

  public:

	AliasUnit(string cat_, string name_, string plural_, string singular_, string title_, Unit *alias, string relation = "1", int exp_ = 1, string reverse = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
	AliasUnit(const AliasUnit *unit);		
	AliasUnit();			
	virtual ~AliasUnit();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	
	virtual Unit* baseUnit() const;
	virtual Unit* firstBaseUnit() const;
	virtual void setBaseUnit(Unit *alias);
	virtual string expression() const;
	virtual string reverseExpression() const;
	virtual void setExpression(string relation);
	virtual void setReverseExpression(string reverse);
	virtual MathStructure &firstBaseValue(MathStructure &mvalue, MathStructure &mexp) const;
	virtual MathStructure &convertToFirstBase(MathStructure &mvalue, MathStructure &mexp) const;
	virtual MathStructure &baseValue(MathStructure &mvalue, MathStructure &mexp) const;
	virtual MathStructure &convertToBase(MathStructure &mvalue, MathStructure &mexp) const;
	virtual MathStructure &baseValue(MathStructure &mvalue) const;
	virtual MathStructure &convertToBase(MathStructure &mvalue) const;
	virtual MathStructure baseValue() const;
	virtual MathStructure convertToBase() const;
	virtual int baseExp(int exp_ = 1) const;
	virtual void setExponent(int exp_);
	virtual int firstBaseExp() const;
	virtual int subtype() const;
	virtual bool isChildOf(Unit *u) const;
	virtual bool isParentOf(Unit *u) const;
	virtual bool hasComplexExpression() const;
	virtual bool hasComplexRelationTo(Unit *u) const;

};

class AliasUnit_Composite : public AliasUnit {

  protected:

	Prefix *prefixv;

  public:
	AliasUnit_Composite(Unit *alias, int exp_ = 1, Prefix *prefix_ = NULL);
	AliasUnit_Composite(const AliasUnit_Composite *unit);			
	virtual ~AliasUnit_Composite();

	virtual ExpressionItem *copy() const;
	virtual void set(const ExpressionItem *item);
	
	virtual string print(bool plural_, bool short_, bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
	virtual Prefix *prefix() const;
	virtual int prefixExponent() const;	
	virtual void set(Unit *u, int exp_ = 1, Prefix *prefix_ = NULL);
	virtual MathStructure &firstBaseValue(MathStructure &mvalue, MathStructure &mexp) const;
	virtual MathStructure &convertToFirstBase(MathStructure &mvalue, MathStructure &mexp) const;

};

class CompositeUnit : public Unit {
	
	protected:
	
		string sshort;

	public:
		//--------internal-------------//
		vector<AliasUnit_Composite*> units;
		//-----------------------------//

		CompositeUnit(string cat_, string name_, string title_ = "", string base_expression_ = "", bool is_local = true, bool is_builtin = false, bool is_active = true);
		CompositeUnit(const CompositeUnit *unit);
		virtual ~CompositeUnit();
		virtual ExpressionItem *copy() const;
		virtual void set(const ExpressionItem *item);		
		virtual void add(Unit *u, int exp_ = 1, Prefix *prefix = NULL);
		virtual Unit *get(size_t index, int *exp_ = NULL, Prefix **prefix = NULL) const;
		virtual void setExponent(size_t index, int exp_);
		virtual void setPrefix(size_t index, Prefix *prefix);
		virtual size_t countUnits() const;
		virtual void del(Unit *u);
		virtual string print(bool plural_, bool short_, bool use_unicode = false, bool (*can_display_unicode_string_function) (const char*, void*) = NULL, void *can_display_unicode_string_arg = NULL) const;
		virtual int subtype() const;
		virtual bool containsRelativeTo(Unit *u) const;
		virtual MathStructure generateMathStructure() const;
		virtual void setBaseExpression(string base_expression_);		
		virtual void updateNames();
		virtual void clear();
};


#endif
