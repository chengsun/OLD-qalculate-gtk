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

#include "includes.h"

enum {
	MULTIPLICATION_MANAGER,
	ADDITION_MANAGER,
	POWER_MANAGER,
	NUMBER_MANAGER,
	UNIT_MANAGER,
	NULL_MANAGER,
	STRING_MANAGER,
	FUNCTION_MANAGER,
	VARIABLE_MANAGER,	
	MATRIX_MANAGER,
	ALTERNATIVE_MANAGER,
	AND_MANAGER,
	OR_MANAGER,
	NOT_MANAGER,
	COMPARISON_MANAGER,
	GIAC_MANAGER	
};

class Manager {

	protected:

		Unit* o_unit;
		vector<Manager*> mngrs;
		int c_type;
		int i_refcount;
		string s_var;
		Function *o_function;
		Variable *o_variable;
		Number *o_number;
		Matrix *mtrx;
		bool b_exact;
		bool b_protect;
		ComparisonType comparison_type;
		Prefix *o_prefix;
		
		int sync_id;

		void init();
	
	public:

#ifdef HAVE_GIAC
		giac::gen *g_gen;
#endif
		
		//dangerous
		void setType(int mngr_type);
		void setComparisonType(ComparisonType comp_type);
		
		void setPrefix(Prefix *p);
		Prefix *prefix() const;
		bool clearPrefixes();
		
		void transform(const Manager *mngr, char type_, MathOperation op, bool reverse_ = false);		
		bool typeclean();
		bool reverseadd(const Manager *mngr, MathOperation op, bool translate_ = true);
		bool compatible(const Manager *mngr);
		void moveto(Manager *mngr);			
		void push_back(Manager *mngr);
		
		
		Manager();
		Manager(double value_);		
		Manager(long int numerator_, long int denominator_, long int number_exp_ = 0);		
		Manager(string var_);	
		Manager(const Variable *v);
		Manager(const Function *f, ...);							
		Manager(const Unit *u, long int exp10 = 0);				
		Manager(const Manager *mngr);	
		Manager(const Number *o);
		Manager(const Matrix *matrix_);					
		Manager(const Vector *vector_);
		~Manager();
		void setNull();
		void set(const Manager *mngr);
		void set(const Function *f, ...);		
		void set(const Number *o);
		void set(const Matrix *matrix_);
		void set(const Vector *vector_);
		void set(double value_);		
		void set(long int numerator_, long int denominator_, long int number_exp_ = 0);		
		void set(string var_);		
		void set(const Variable *v);		
		void set(const Unit *u, long int exp10 = 0);				
		void addFunctionArg(const Manager *mngr);
		bool add(const Manager *mngr, MathOperation op = OPERATION_MULTIPLY, bool translate_ = true);	
		bool setNOT(bool translate_ = true);
		void addUnit(const Unit *u, MathOperation op = OPERATION_MULTIPLY);		
		void addFloat(double value_, MathOperation op = OPERATION_MULTIPLY);			
		void addInteger(long int value_, MathOperation op = OPERATION_MULTIPLY);					
		void addAlternative(const Manager *mngr);
		int compare(const Manager *mngr) const;
		int sortCompare(const Manager *mngr, int sortflags = SORT_SCIENTIFIC) const;
		void sort(int sortflags = SORT_SCIENTIFIC);					
		void clear();
		bool equals(const Manager *mngr) const;
		double value() const;
		Number *number() const;
		Matrix *matrix() const;		
		const string &text() const;
		Unit *unit() const;
		Manager *getChild(unsigned int index) const;
		unsigned int countChilds() const;
		Manager *base() const;
		Manager *exponent() const;
		Function *function() const;
		Variable *variable() const;
		void recalculateFunctions();
		void recalculateVariables();
		ComparisonType comparisonType() const;
		bool isAlternatives() const;
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
		bool isVariable() const;
		bool isPower() const;
		bool isNumber() const;
		bool isNumber_exp() const;
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
		string print(NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, bool *in_exact = NULL, bool *usable = NULL, Prefix *set_prefix = NULL, bool toplevel = true, bool *plural = NULL, Number *l_exp = NULL, bool in_composite = false, bool in_power = false, bool draw_minus = false, bool print_equals = false, bool in_multiplication = false, bool wrap = true, bool wrap_all = false, bool *has_parenthesis = NULL, bool in_div = false, bool no_add_one = false, Number *l_exp2 = NULL, Prefix **prefix1 = NULL, Prefix **prefix2 = NULL, string string_fr = "") const;
		int refcount() const;
		void ref();
		void unref();
		void protect(bool do_protect = true);
		bool isProtected() const;
		int type() const;
		
#ifdef HAVE_GIAC
		giac::gen toGiac(bool *isnull = NULL) const;
		Manager(const giac::gen &giac_gen);
		void set(const giac::gen &giac_gen, bool in_retry = false);
		void simplify();
#endif
		
		bool contains(Manager *mngr) const;
		bool containsType(int mtype) const;
		void replace(Manager *replace_this, Manager *replace_with);
		void replace_no_copy(Manager *replace_this, Manager *replace_with);
		void differentiate(string x_var);
		void integrate(string x_var);
		void factorize();
		
		Vector *generateVector(string x_var, const Manager *min, const Manager *max, int steps, Vector **x_vector = NULL);
		Vector *generateVector(string x_var, Vector *x_vector);
		
		string find_x_var();
		void move_x_to_one_side(string x_var = "");
		void solve(string x_var);

};


#endif
