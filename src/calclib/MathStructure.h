/*
    Qalculate    

    Copyright (C) 2004  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef MATH_STRUCTURE_H
#define MATH_STRUCTURE_H

#include "includes.h"
#include "Number.h"

enum {
	STRUCT_MULTIPLICATION,
	STRUCT_INVERSE,
	STRUCT_DIVISION,
	STRUCT_ADDITION,
	STRUCT_NEGATE,
	STRUCT_POWER,
	STRUCT_NUMBER,
	STRUCT_UNIT,
	STRUCT_SYMBOLIC,
	STRUCT_FUNCTION,
	STRUCT_VARIABLE,
	STRUCT_VECTOR,
	STRUCT_ALTERNATIVES,
	STRUCT_AND,
	STRUCT_OR,
	STRUCT_XOR,
	STRUCT_NOT,
	STRUCT_COMPARISON,
	STRUCT_UNKNOWN,
	STRUCT_UNDEFINED
};

enum {
	MULTIPLICATION_SIGN_NONE,
	MULTIPLICATION_SIGN_SPACE,
	MULTIPLICATION_SIGN_OPERATOR,
	MULTIPLICATION_SIGN_OPERATOR_SHORT
};

class MathStructure {

	protected:
	
		int m_type;
		bool b_approx;
		int i_precision;
	
		vector<MathStructure> v_subs;
		vector<unsigned int> v_order;
		string s_sym;
		Number o_number;
		Variable *o_variable;

		Unit *o_unit;
		Prefix *o_prefix;
		bool b_plural;
		
		Function *o_function;
		
#ifdef HAVE_GIAC		
		giac::gen *giac_unknown;
#endif
		ComparisonType ct_comp;
	
	public:

		void init();
		
		MathStructure();
		MathStructure(const MathStructure &o);
		MathStructure(int num, int den = 1, int exp10 = 0);
		MathStructure(string sym);
		MathStructure(double float_value);
		MathStructure(const MathStructure *o, ...);
		MathStructure(Function *o, ...);
		MathStructure(Unit *u, Prefix *p = NULL);
		MathStructure(Variable *o);
		MathStructure(const Number &o);
		~MathStructure();
		
		void set(const MathStructure &o);
		void set(int num, int den = 1, int exp10 = 0);
		void set(string sym);
		void set(double float_value);
		void set(const MathStructure *o, ...);
		void set(Function *o, ...);
		void set(Unit *u, Prefix *p = NULL);
		void set(Variable *o);
		void set(const Number &o);
		void setInfinity();
		void setUndefined();
		void clear();
		void clearVector();
		void clearMatrix();
		
		void operator = (const MathStructure &o);
		MathStructure operator - () const;
		MathStructure operator * (const MathStructure &o) const;
		MathStructure operator / (const MathStructure &o) const;
		MathStructure operator + (const MathStructure &o) const;
		MathStructure operator - (const MathStructure &o) const;
		MathStructure operator ^ (const MathStructure &o) const;
		MathStructure operator && (const MathStructure &o) const;
		MathStructure operator || (const MathStructure &o) const;
		MathStructure operator ! () const;
		
		void operator *= (const MathStructure &o);
		void operator /= (const MathStructure &o);
		void operator += (const MathStructure &o);
		void operator -= (const MathStructure &o);
		void operator ^= (const MathStructure &o);
		
		bool operator == (const MathStructure &o) const;
		bool operator != (const MathStructure &o) const;
		
		const MathStructure &operator [] (unsigned int index) const;
		MathStructure &operator [] (unsigned int index);
		
		const Number &number() const;
		Number &number();
		void numberUpdated();
		void childUpdated(unsigned int index);
		void childrenUpdated();
		const string &symbol() const;
#ifdef HAVE_GIAC
		const giac::gen *unknown() const;
#endif
		ComparisonType comparisonType() const;
		void setComparisonType(ComparisonType comparison_type);
		//dangerous
		void setType(int mtype);
		Unit *unit() const;
		Prefix *prefix() const;
		void setPrefix(Prefix *p);
		bool isPlural() const;
		void setPlural(bool is_plural);
		void setFunction(Function *f);
		void setUnit(Unit *u);
		void setVariable(Variable *v);
		Function *function() const;
		Variable *variable() const;
		
		bool isAddition() const;
		bool isMultiplication() const;
		bool isPower() const;
		bool isSymbolic() const;
		bool isEmptySymbol() const;
		bool isVector() const;
		bool isMatrix() const;
		bool isFunction() const;
		bool isUnit() const;
		bool isUnit_exp() const;
		bool isNumber_exp() const;
		bool isVariable() const;
		bool isComparison() const;
		bool isAND() const;
		bool isOR() const;
		bool isXOR() const;
		bool isNOT() const;
		bool isInverse() const;
		bool isDivision() const;
		bool isNegate() const;
		bool isInfinity() const;
		bool isUndefined() const;
		bool isInteger() const;
		bool isNumber() const;
		bool isZero() const;
		bool isOne() const;
		bool isMinusOne() const;
		
		bool hasNegativeSign() const;
		
		bool representsPositive() const;
		bool representsNegative() const;
		bool representsNonNegative() const;
		bool representsNonPositive() const;
		bool representsInteger() const;
		bool representsNumber() const;
		bool representsRational() const;
		bool representsReal() const;
		bool representsComplex() const;
		bool representsNonZero() const;
		bool representsEven() const;
		bool representsOdd() const;
		bool representsUndefined(bool include_childs = false, bool include_infinite = false) const;
	
		void setApproximate(bool is_approx = true);	
		bool isApproximate() const;
		
		void setPrecision(int prec);
		int precision() const;
		
		void transform(int mtype, const MathStructure &o);
		void transform(int mtype);

		void add(const MathStructure &o, MathOperation op, bool append = false);
		void add(const MathStructure &o, bool append = false);
		void subtract(const MathStructure &o, bool append = false);
		void multiply(const MathStructure &o, bool append = false);
		void divide(const MathStructure &o, bool append = false);
		void raise(const MathStructure &o);
		void inverse();
		void negate();
		void setNOT();
		
		bool equals(const MathStructure &o) const;
		ComparisonResult compare(const MathStructure &o) const;
		
		int merge_addition(const MathStructure &mstruct, const EvaluationOptions &eo);
		int merge_multiplication(const MathStructure &mstruct, const EvaluationOptions &eo, bool do_append = true);
		int merge_power(const MathStructure &mstruct, const EvaluationOptions &eo);
		bool calculatesub(const EvaluationOptions &eo, const EvaluationOptions &feo);
		bool calculateFunctions(const EvaluationOptions &eo);			
		MathStructure &eval(const EvaluationOptions &eo = default_evaluation_options);
		bool factorize(const EvaluationOptions &eo = default_evaluation_options);

#ifdef HAVE_GIAC		
		giac::gen toGiac() const;
		void set(const giac::gen &giac_gen, bool in_retry = false);
		MathStructure(const giac::gen &giac_gen);
#endif
		
		void addChild(const MathStructure &o);
		void delChild(unsigned int index);
		void insertChild(const MathStructure &o, unsigned int index);
		void setChild(const MathStructure &o, unsigned int index = 1);
		const MathStructure *getChild(unsigned int index) const;
		MathStructure *getChild(unsigned int index);
		unsigned int countChilds() const;
		unsigned int countTotalChilds(bool count_function_as_one = true) const;
		unsigned int size() const;
		
#define		addItem(o)		addChild(o)
#define		insertItem(o, i)	insertChild(o, i)
#define		setItem(o, i)		setChild(o, i)
#define		items()			countChilds()
#define		getItem(i)		getChild(i)

#define		addComponent(o)		addChild(o)
#define		insertComponent(o, i)	insertChild(o, i)
#define		setComponent(o, i)	setChild(o, i)
#define		components()		countChilds()
#define		getComponent(i)		getChild(i)

		const MathStructure *base() const;
		const MathStructure *exponent() const;
		MathStructure *base();
		MathStructure *exponent();
		
		void addAlternative(const MathStructure &o);		

		int type() const;
		
		void sort(const PrintOptions &po = default_print_options, bool recursive = true);
		void evalSort(bool recursive = false);
		bool improve_division_multipliers(const PrintOptions &po = default_print_options);
		void setPrefixes(const PrintOptions &po = default_print_options, MathStructure *parent = NULL, unsigned int pindex = 0);
		void prefixCurrencies();
		void format(const PrintOptions &po = default_print_options);
		void formatsub(const PrintOptions &po = default_print_options, MathStructure *parent = NULL, unsigned int pindex = 0);
		void postFormatUnits(const PrintOptions &po = default_print_options, MathStructure *parent = NULL, unsigned int pindex = 0);
		void unformat(const EvaluationOptions &eo = default_evaluation_options);
		bool needsParenthesis(const PrintOptions &po, const InternalPrintStruct &ips, const MathStructure &parent, unsigned int index, bool flat_division = true, bool flat_power = true) const;

		int neededMultiplicationSign(const PrintOptions &po, const InternalPrintStruct &ips, const MathStructure &parent, unsigned int index, bool par, bool par_prev, bool flat_division = true, bool flat_power = true) const;
		
		string print(const PrintOptions &po = default_print_options, const InternalPrintStruct &ips = top_ips) const;
		
//vector
	
		MathStructure &flattenVector(MathStructure &mstruct) const;
		
		bool rankVector(bool ascending = true);
		bool sortVector(bool ascending = true);
		
		MathStructure &getRange(int start, int end, MathStructure &mstruct) const;
		
		void resizeVector(unsigned int i, const MathStructure &mfill);
		
//matrix

		unsigned int rows() const;
		unsigned int columns() const;
		const MathStructure *getElement(unsigned int row, unsigned int column) const;
		MathStructure &getArea(unsigned int r1, unsigned int c1, unsigned int r2, unsigned int c2, MathStructure &mstruct) const;
		MathStructure &rowToVector(unsigned int r, MathStructure &mstruct) const;
		MathStructure &columnToVector(unsigned int c, MathStructure &mstruct) const;
		MathStructure &matrixToVector(MathStructure &mstruct) const;
		void setElement(const MathStructure &mstruct, unsigned int row, unsigned int column);
		void addRows(unsigned int r, const MathStructure &mfill);
		void addColumns(unsigned int c, const MathStructure &mfill);
		void addRow(const MathStructure &mfill);
		void addColumn(const MathStructure &mfill);
		void resizeMatrix(unsigned int r, unsigned int c, const MathStructure &mfill);
		bool matrixIsSymmetric() const;
		MathStructure &determinant(MathStructure &mstruct, const EvaluationOptions &eo) const;
		MathStructure &permanent(MathStructure &mstruct, const EvaluationOptions &eo) const;
		void setToIdentityMatrix(unsigned int n);
		MathStructure &getIdentityMatrix(MathStructure &mstruct) const;
		bool invertMatrix(const EvaluationOptions &eo);
		bool adjointMatrix(const EvaluationOptions &eo);
		bool transposeMatrix();
		MathStructure &cofactor(unsigned int r, unsigned int c, MathStructure &mstruct, const EvaluationOptions &eo) const;
		
//units

		bool isUnitCompatible(const MathStructure &mstruct);
		bool syncUnits(bool sync_complex_relations = false);
		bool testDissolveCompositeUnit(Unit *u);
		bool testCompositeUnit(Unit *u);	
		bool dissolveAllCompositeUnits();			
		bool convert(Unit *u, bool convert_complex_relations = false);
		bool convert(const MathStructure unit_mstruct, bool convert_complex_relations = false);	
		
		
		bool contains(const MathStructure &mstruct, bool check_variables = false, bool check_functions = false) const;
		bool containsType(int mtype) const;
		bool containsAdditionPower() const;
		bool containsUnknowns() const;
		bool containsDivision() const;
		void findAllUnknowns(MathStructure &unknowns_vector);
		bool replace(const MathStructure &mfrom, const MathStructure &mto);
		
		MathStructure generateVector(MathStructure x_mstruct, const MathStructure &min, const MathStructure &max, int steps, MathStructure *x_vector = NULL, const EvaluationOptions &eo = default_evaluation_options);
		MathStructure generateVector(MathStructure x_mstruct, const MathStructure &x_vector, const EvaluationOptions &eo = default_evaluation_options);
		
		bool differentiate(const MathStructure &x_var, const EvaluationOptions &eo);
		bool integrate(const MathStructure &x_var, const EvaluationOptions &eo);
		
		const MathStructure &find_x_var() const;
		bool isolate_x(const EvaluationOptions &eo, const MathStructure &x_var = m_undefined);

};

#endif
