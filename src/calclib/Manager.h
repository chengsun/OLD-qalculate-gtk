/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef MANAGER_H
#define MANAGER_H

enum {
	MULTIPLICATION_MANAGER,
	ADDITION_MANAGER,
	POWER_MANAGER,
	FRACTION_MANAGER,
	UNIT_MANAGER,
	NULL_MANAGER,
	STRING_MANAGER,
	FUNCTION_MANAGER,
	MATRIX_MANAGER,
	ALTERNATIVE_MANAGER,
	AND_MANAGER,
	OR_MANAGER,
	NOT_MANAGER,
	COMPARISON_MANAGER
};

#include "includes.h"

class Manager {

	
	public:
	
		Unit* o_unit;
		vector<Manager*> mngrs;
		int c_type;
		int refcount;
		string s_var;
		Function *o_function;
		Fraction *fr;
		Matrix *mtrx;
		bool b_exact;
		bool b_protect;
		ComparisonType comparison_type;
		
		void transform(const Manager *mngr, char type_, MathOperation op, bool reverse_ = false);		
		bool typeclean();
		bool reverseadd(const Manager *mngr, MathOperation op, bool translate_ = true);
		bool compatible(const Manager *mngr);
		void moveto(Manager *mngr);			
		void init();
		void push_back(Manager *mngr);
		
		Manager();
		Manager(long double value_);		
		Manager(long int numerator_, long int denominator_, long int fraction_exp_ = 0);		
		Manager(string var_);	
		Manager(const Function *f, ...);							
		Manager(const Unit *u, long int exp10 = 0);				
		Manager(const Manager *mngr);	
		Manager(const Fraction *fraction_);
		Manager(const Matrix *matrix_);					
		Manager(const Vector *vector_);
		~Manager();
		void setNull();
		void set(const Manager *mngr);
		void set(const Function *f, ...);		
		void set(const Fraction *fraction_);				
		void set(const Matrix *matrix_);
		void set(const Vector *vector_);
		void set(long double value_);		
		void set(long int numerator_, long int denominator_, long int fraction_exp_ = 0);		
		void set(string var_);				
		void set(const Unit *u, long int exp10 = 0);				
		void addFunctionArg(const Manager *mngr);
		bool add(const Manager *mngr, MathOperation op = OPERATION_MULTIPLY, bool translate_ = true);	
		bool setNOT(bool translate_ = true);
		void addUnit(const Unit *u, MathOperation op = OPERATION_MULTIPLY);		
		void addFloat(long double value_, MathOperation op = OPERATION_MULTIPLY);			
		void addInteger(long int value_, MathOperation op = OPERATION_MULTIPLY);					
		void addAlternative(const Manager *mngr);
		int compare(const Manager *mngr, int sortflags = SORT_SCIENTIFIC) const;
		void sort(int sortflags = SORT_SCIENTIFIC);					
		void clear();
		bool equals(const Manager *mngr) const;
		long double value() const;
		Fraction *fraction() const;
		Matrix *matrix() const;		
		const string &text() const;
		Unit *unit() const;
		Manager *getChild(int index) const;
		int countChilds() const;
		Manager *base() const;
		Manager *exponent() const;
		Function *function() const;
		void recalculateFunctions();
		ComparisonType comparisonType() const;
		bool isComparison() const;
		bool isOR() const;
		bool isAND() const;
		bool isNOT() const;
		bool isText() const;
		bool isUnit() const;
		bool isUnit_exp() const;
		bool isAddition() const;
		bool isMultiplication() const;
		bool isFunction() const;
		bool isPower() const;
		bool isNumber() const;
		bool isFraction() const;		
		bool isMatrix() const;				
		bool isNull() const;		
		bool isZero() const;				
		bool isOne() const;
		bool isPrecise() const;
		void setPrecise(bool is_precise);
		bool hasNegativeSign() const;
		int signedness() const;
		int isPositive() const;
		bool negative() const;
		void finalize();
		void clean();
		void syncUnits();		
		bool testDissolveCompositeUnit(const Unit *u);
		bool testCompositeUnit(const Unit *u);	
		bool dissolveAllCompositeUnits();			
		bool convert(const Unit*);
		bool convert(string unit_str);		
		bool convert(const Manager *unit_mngr);				
		string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, bool *in_exact = NULL, bool *usable = NULL, Prefix *prefix = NULL, bool toplevel = true, bool *plural = NULL, Integer *l_exp = NULL, bool in_composite = false, bool in_power = false) const;
		void ref();
		void unref();
		void protect(bool do_protect = true);
		bool isProtected() const;
		int type() const;
		
		void replace(Manager *replace_this, Manager *replace_with);
		void replace_no_copy(Manager *replace_this, Manager *replace_with);
		void differentiate(string x_var);

};


#endif
