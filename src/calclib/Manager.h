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

#include "Calculator.h"
#include "Unit.h"
#include "Function.h"
#include "Fraction.h"
#include "Integer.h"

class Manager {

	
	public:
	
		Unit* o_unit;
		vector<Manager*> mngrs;
		char c_type;
		int refcount;
		string s_var;
		Function *o_function;
		Fraction *fr;
		bool b_exact;
		
		void transform(Manager *mngr, char type_, MathOperation op, bool reverse_ = false);		
		void plusclean();		
		void multiclean();		
		void powerclean();				
		bool reverseadd(Manager *mngr, MathOperation op, bool translate_ = true);
		bool compatible(Manager *mngr);
		void moveto(Manager *mngr);			
		void init();
		void push_back(Manager *mngr);
		
		Manager(void);
		Manager(long double value_);		
		Manager(long int numerator_, long int denominator_, long int fraction_exp_ = 0);		
		Manager(string var_);	
		Manager(Function *f, ...);							
		Manager(Unit *u, long int exp10 = 0);				
		Manager(const Manager *mngr);	
		Manager(const Fraction *fraction_);			
		~Manager(void);
		void setNull();
		void set(const Manager *mngr);
		void set(Function *f, ...);		
		void set(const Fraction *fraction_);				
		void set(long double value_);		
		void set(long int numerator_, long int denominator_, long int fraction_exp_ = 0);		
		void set(string var_);				
		void set(Unit *u, long int exp10 = 0);				
		void addFunctionArg(Manager *mngr);
		bool add(Manager *mngr, MathOperation op = MULTIPLY, bool translate_ = true);	
		void addUnit(Unit *u, MathOperation op = MULTIPLY);		
		void addFloat(long double value_, MathOperation op = MULTIPLY);			
		void addInteger(long int value_, MathOperation op = MULTIPLY);					
		int compare(Manager *mngr);
		void sort(void);					
		void clear(void);
		bool equals(Manager *mngr);
		long double value(void);
		void value(long double value_);				
		Fraction *fraction() const;
		bool isNumber();
		bool isFraction();		
		bool isNull();		
		bool isZero();				
		bool isOne();
		bool isPrecise() const;
		void setPrecise(bool is_precise);
		Unit *unit(void);
		void unit(Unit *u, long int value_ = 1);		
		bool negative();
		void finalize();
		void clean();
		void syncUnits();		
		bool testDissolveCompositeUnit(Unit *u);
		bool testCompositeUnit(Unit *u);	
		bool dissolveAllCompositeUnits();			
		bool convert(Unit*);
		bool convert(string unit_str);		
		bool convert(Manager *unit_mngr);				
		string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int decimals_to_keep = 0, bool decimals_expand = true, bool decimals_decrease = false, bool *in_exact = NULL, bool *usable = NULL, Prefix *prefix = NULL, bool toplevel = true, bool *plural = NULL, Integer *l_exp = NULL, bool in_composite = false, bool in_power = false);
		void ref(void);
		void unref(void);
		char type(void) const;
		
		void replace(Manager *replace_this, Manager *replace_with);
		void differentiate(string x_var);

};


#endif
