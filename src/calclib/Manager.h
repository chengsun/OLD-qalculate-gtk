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

class Manager;

#define MULTIPLICATION_MANAGER	MULTIPLICATION_CH
#define ADDITION_MANAGER	PLUS_CH
#define POWER_MANAGER		POWER_CH
#define FRACTION_MANAGER	'v'
#define UNIT_MANAGER		'u'
#define NULL_MANAGER		0
#define STRING_MANAGER		's'
#define FUNCTION_MANAGER	'f'
#define MATRIX_MANAGER		'm'
#define ALTERNATIVE_MANAGER	'a'

#include "Calculator.h"
#include "Unit.h"
#include "Function.h"
#include "Fraction.h"
#include "Integer.h"
#include "Matrix.h"

class Manager {

	
	public:
	
		Unit* o_unit;
		vector<Manager*> mngrs;
		char c_type;
		int refcount;
		string s_var;
		Function *o_function;
		Fraction *fr;
		Matrix *mtrx;
		bool b_exact;
		
		void transform(const Manager *mngr, char type_, MathOperation op, bool reverse_ = false);		
		void altclean();
		void plusclean();		
		void multiclean();		
		void powerclean();				
		bool reverseadd(const Manager *mngr, MathOperation op, bool translate_ = true);
		bool compatible(const Manager *mngr);
		void moveto(Manager *mngr);			
		void init();
		void push_back(Manager *mngr);
		
		Manager(void);
		Manager(long double value_);		
		Manager(long int numerator_, long int denominator_, long int fraction_exp_ = 0);		
		Manager(string var_);	
		Manager(const Function *f, ...);							
		Manager(const Unit *u, long int exp10 = 0);				
		Manager(const Manager *mngr);	
		Manager(const Fraction *fraction_);
		Manager(const Matrix *matrix_);					
		~Manager(void);
		void setNull();
		void set(const Manager *mngr);
		void set(const Function *f, ...);		
		void set(const Fraction *fraction_);				
		void set(const Matrix *matrix_);
		void set(long double value_);		
		void set(long int numerator_, long int denominator_, long int fraction_exp_ = 0);		
		void set(string var_);				
		void set(const Unit *u, long int exp10 = 0);				
		void addFunctionArg(const Manager *mngr);
		bool add(const Manager *mngr, MathOperation op = MULTIPLY, bool translate_ = true);	
		void addUnit(const Unit *u, MathOperation op = MULTIPLY);		
		void addFloat(long double value_, MathOperation op = MULTIPLY);			
		void addInteger(long int value_, MathOperation op = MULTIPLY);					
		void addAlternative(const Manager *mngr);
		int compare(const Manager *mngr, int sortflags = SORT_SCIENTIFIC) const;
		void sort(int sortflags = SORT_SCIENTIFIC);					
		void clear(void);
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
		void ref(void);
		void unref(void);
		char type(void) const;
		
		void replace(Manager *replace_this, Manager *replace_with);
		void differentiate(string x_var);

};


#endif
