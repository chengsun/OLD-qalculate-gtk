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
#define VALUE_MANAGER		'v'
#define UNIT_MANAGER		'u'
#define NULL_MANAGER		0
#define STRING_MANAGER		's'
#define UNSOLVEABLE_FUNCTION_MANAGER	'f'

#include "Calculator.h"
#include "Unit.h"
#include "Function.h"

class Manager {

	protected:
	
		void plusclean();		
		void multiclean();		
		void powerclean();				
		bool reverseadd(Manager *mngr, char sign, bool translate_ = true);
		void transform(Manager *mngr, char type_, char sign, bool reverse_ = false);
		bool mergable(Manager *mngr, char type_);
		bool compatible(Manager *mngr);
	
	public:
	
		Calculator *calc;
		Unit* o_unit;
		long double d_value;
		vector<Manager*> mngrs;
		char c_type;
		int refcount;
		string s_var;
		Function *function;
		
	
		void moveto(Manager *mngr);			
		
		Manager(Calculator *calc_);
		Manager(Calculator *calc_, long double value_);		
		Manager(Calculator *calc_, string var_);				
		Manager(Calculator *calc_, Unit *u, long double value_ = 1);				
		Manager(const Manager *mngr);	
		~Manager(void);
		void set(const Manager *mngr);
		void set(long double value_);		
		void set(string var_);				
		void set(Unit *u, long double value_ = 1);				
		bool add(Manager *mngr, char sign = MULTIPLICATION_CH, bool translate_ = true);	
		void add(Unit *u, char sign = MULTIPLICATION_CH);		
		void add(long double value_, char sign = MULTIPLICATION_CH);			
		int compare(Manager *mngr);
		void sort(void);					
		void clear(void);
		bool equal(Manager *mngr);
		long double value(void);
		void value(long double value_);				
		Unit *unit(void);
		void unit(Unit *u, long double value_ = 1);		
		bool negative();
		void finalize();
		void syncUnits();		
		bool testDissolveCompositeUnit(Unit *u);
		bool testCompositeUnit(Unit *u);	
		bool dissolveAllCompositeUnits();			
		bool convert(Unit*);
		bool convert(string unit_str);		
		bool convert(Manager *unit_mngr);				
		string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int unitflags = UNIT_FORMAT_DEFAULT, int precision = PRECISION, int decimals_to_keep = 0, bool decimals_expand = true, bool decimals_decrease = false, bool *usable = NULL, bool toplevel = true, bool *plural = NULL);
		void ref(void);
		void unref(void);
		char type(void) const;

};


#endif
