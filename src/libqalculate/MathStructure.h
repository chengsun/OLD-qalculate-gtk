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

#include <libqalculate/includes.h>
#include <libqalculate/Number.h>

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
	
		size_t i_ref;
	
		int m_type;
		bool b_approx;
		int i_precision;
	
		vector<MathStructure*> v_subs;
		vector<size_t> v_order;
		string s_sym;
		Number o_number;
		Variable *o_variable;

		Unit *o_unit;
		Prefix *o_prefix;
		bool b_plural;
		
		MathFunction *o_function;
		MathStructure *function_value;
		
#ifdef HAVE_GIAC		
		giac::gen *giac_unknown;
#endif
		ComparisonType ct_comp;
	
	public:

		void ref();
		void unref();
		size_t refcount() const;
		void init();
		
		MathStructure();
		MathStructure(const MathStructure &o);
		MathStructure(int num, int den = 1, int exp10 = 0);
		MathStructure(string sym);
		MathStructure(double float_value);
		MathStructure(const MathStructure *o, ...);
		MathStructure(MathFunction *o, ...);
		MathStructure(Unit *u, Prefix *p = NULL);
		MathStructure(Variable *o);
		MathStructure(const Number &o);
		~MathStructure();
		
		void set(const MathStructure &o, bool merge_precision = false);
		void set_nocopy(MathStructure &o, bool merge_precision = false);
		void setToChild(size_t index, bool merge_precision = false);
		void set(int num, int den = 1, int exp10 = 0, bool preserve_precision = false);
		void set(string sym, bool preserve_precision = false);
		void set(double float_value, bool preserve_precision = false);
		void setVector(const MathStructure *o, ...);
		void set(MathFunction *o, ...);
		void set(Unit *u, Prefix *p = NULL, bool preserve_precision = false);
		void set(Variable *o, bool preserve_precision = false);
		void set(const Number &o, bool preserve_precision = false);
		void setInfinity(bool preserve_precision = false);
		void setUndefined(bool preserve_precision = false);
		void clear(bool preserve_precision = false);
		void clearVector(bool preserve_precision = false);
		void clearMatrix(bool preserve_precision = false);
		
		void operator = (const MathStructure &o);
		void operator = (const Number &o);
		void operator = (int i);
		void operator = (Unit *u);
		void operator = (Variable *v);
		void operator = (string sym);
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
		
		void operator *= (const Number &o);
		void operator /= (const Number &o);
		void operator += (const Number &o);
		void operator -= (const Number &o);
		void operator ^= (const Number &o);
		
		void operator *= (int i);
		void operator /= (int i);
		void operator += (int i);
		void operator -= (int i);
		void operator ^= (int i);
		
		void operator *= (Unit *u);
		void operator /= (Unit *u);
		void operator += (Unit *u);
		void operator -= (Unit *u);
		void operator ^= (Unit *u);
		
		void operator *= (Variable *v);
		void operator /= (Variable *v);
		void operator += (Variable *v);
		void operator -= (Variable *v);
		void operator ^= (Variable *v);
		
		void operator *= (string sym);
		void operator /= (string sym);
		void operator += (string sym);
		void operator -= (string sym);
		void operator ^= (string sym);
		
		bool operator == (const MathStructure &o) const;
		bool operator == (const Number &o) const;
		bool operator == (int i) const;
		bool operator == (Unit *u) const;
		bool operator == (Variable *v) const;
		bool operator == (string sym) const;
		
		bool operator != (const MathStructure &o) const;
		
		const MathStructure &operator [] (size_t index) const;
		MathStructure &operator [] (size_t index);
		
		const MathStructure *functionValue() const;
		
		const Number &number() const;
		Number &number();
		void numberUpdated();
		void childUpdated(size_t index, bool recursive = false);
		void childrenUpdated(bool recursive = false);
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
		void setFunction(MathFunction *f);
		void setUnit(Unit *u);
		void setVariable(Variable *v);
		MathFunction *function() const;
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
		void transform(int mtype, const Number &o);
		void transform(int mtype, int i);
		void transform(int mtype, Unit *u);
		void transform(int mtype, Variable *v);
		void transform(int mtype, string sym);
		void transform_nocopy(int mtype, MathStructure *o);
		void transform(int mtype);

		void add(const MathStructure &o, MathOperation op, bool append = false);
		void add(const MathStructure &o, bool append = false);
		void subtract(const MathStructure &o, bool append = false);
		void multiply(const MathStructure &o, bool append = false);
		void divide(const MathStructure &o, bool append = false);
		void raise(const MathStructure &o);
		void add(const Number &o, bool append = false);
		void subtract(const Number &o, bool append = false);
		void multiply(const Number &o, bool append = false);
		void divide(const Number &o, bool append = false);
		void raise(const Number &o);
		void add(int i, bool append = false);
		void subtract(int i, bool append = false);
		void multiply(int i, bool append = false);
		void divide(int i, bool append = false);
		void raise(int i);
		void add(Variable *v, bool append = false);
		void subtract(Variable *v, bool append = false);
		void multiply(Variable *v, bool append = false);
		void divide(Variable *v, bool append = false);
		void raise(Variable *v);
		void add(Unit *u, bool append = false);
		void subtract(Unit *u, bool append = false);
		void multiply(Unit *u, bool append = false);
		void divide(Unit *u, bool append = false);
		void raise(Unit *u);
		void add(string sym, bool append = false);
		void subtract(string sym, bool append = false);
		void multiply(string sym, bool append = false);
		void divide(string sym, bool append = false);
		void raise(string sym);
		void add_nocopy(MathStructure *o, MathOperation op, bool append = false);
		void add_nocopy(MathStructure *o, bool append = false);
		void subtract_nocopy(MathStructure *o, bool append = false);
		void multiply_nocopy(MathStructure *o, bool append = false);
		void divide_nocopy(MathStructure *o, bool append = false);
		void raise_nocopy(MathStructure *o);
		void inverse();
		void negate();
		void setNOT();
		
		bool equals(const MathStructure &o) const;
		bool equals(const Number &o) const;
		bool equals(int i) const;
		bool equals(Unit *u) const;
		bool equals(Variable *v) const;
		bool equals(string sym) const;
		
		ComparisonResult compare(const MathStructure &o) const;
		
		void mergePrecision(const MathStructure &o);
		
		int merge_addition(MathStructure &mstruct, const EvaluationOptions &eo);
		int merge_multiplication(MathStructure &mstruct, const EvaluationOptions &eo, bool do_append = true);
		int merge_power(MathStructure &mstruct, const EvaluationOptions &eo);
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
		void addChild_nocopy(MathStructure *o);
		void delChild(size_t index);
		void insertChild(const MathStructure &o, size_t index);
		void insertChild_nocopy(MathStructure *o, size_t index);
		void setChild(const MathStructure &o, size_t index = 1);
		void setChild_nocopy(MathStructure *o, size_t index = 1);
		const MathStructure *getChild(size_t index) const;
		MathStructure *getChild(size_t index);
		size_t countChilds() const;
		size_t countTotalChilds(bool count_function_as_one = true) const;
		size_t size() const;
		
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
		void setPrefixes(const PrintOptions &po = default_print_options, MathStructure *parent = NULL, size_t pindex = 0);
		void prefixCurrencies();
		void format(const PrintOptions &po = default_print_options);
		void formatsub(const PrintOptions &po = default_print_options, MathStructure *parent = NULL, size_t pindex = 0);
		void postFormatUnits(const PrintOptions &po = default_print_options, MathStructure *parent = NULL, size_t pindex = 0);
		void unformat(const EvaluationOptions &eo = default_evaluation_options);
		bool needsParenthesis(const PrintOptions &po, const InternalPrintStruct &ips, const MathStructure &parent, size_t index, bool flat_division = true, bool flat_power = true) const;

		int neededMultiplicationSign(const PrintOptions &po, const InternalPrintStruct &ips, const MathStructure &parent, size_t index, bool par, bool par_prev, bool flat_division = true, bool flat_power = true) const;
		
		string print(const PrintOptions &po = default_print_options, const InternalPrintStruct &ips = top_ips) const;
		
//vector
	
		MathStructure &flattenVector(MathStructure &mstruct) const;
		
		bool rankVector(bool ascending = true);
		bool sortVector(bool ascending = true);
		
		MathStructure &getRange(int start, int end, MathStructure &mstruct) const;
		
		void resizeVector(size_t i, const MathStructure &mfill);
		
//matrix

		size_t rows() const;
		size_t columns() const;
		const MathStructure *getElement(size_t row, size_t column) const;
		MathStructure &getArea(size_t r1, size_t c1, size_t r2, size_t c2, MathStructure &mstruct) const;
		MathStructure &rowToVector(size_t r, MathStructure &mstruct) const;
		MathStructure &columnToVector(size_t c, MathStructure &mstruct) const;
		MathStructure &matrixToVector(MathStructure &mstruct) const;
		void setElement(const MathStructure &mstruct, size_t row, size_t column);
		void addRows(size_t r, const MathStructure &mfill);
		void addColumns(size_t c, const MathStructure &mfill);
		void addRow(const MathStructure &mfill);
		void addColumn(const MathStructure &mfill);
		void resizeMatrix(size_t r, size_t c, const MathStructure &mfill);
		bool matrixIsSymmetric() const;
		MathStructure &determinant(MathStructure &mstruct, const EvaluationOptions &eo) const;
		MathStructure &permanent(MathStructure &mstruct, const EvaluationOptions &eo) const;
		void setToIdentityMatrix(size_t n);
		MathStructure &getIdentityMatrix(MathStructure &mstruct) const;
		bool invertMatrix(const EvaluationOptions &eo);
		bool adjointMatrix(const EvaluationOptions &eo);
		bool transposeMatrix();
		MathStructure &cofactor(size_t r, size_t c, MathStructure &mstruct, const EvaluationOptions &eo) const;
		
//units

		int isUnitCompatible(const MathStructure &mstruct);
		bool syncUnits(bool sync_complex_relations = false);
		bool testDissolveCompositeUnit(Unit *u);
		bool testCompositeUnit(Unit *u);	
		bool dissolveAllCompositeUnits();			
		bool convert(Unit *u, bool convert_complex_relations = false);
		bool convert(const MathStructure unit_mstruct, bool convert_complex_relations = false);	
		
		
		int contains(const MathStructure &mstruct, bool structural_only = true, bool check_variables = false, bool check_functions = false) const;
		int containsRepresentativeOf(const MathStructure &mstruct, bool check_variables = false, bool check_functions = false) const;
		int containsType(int mtype, bool structural_only = true, bool check_variables = false, bool check_functions = false) const;
		int containsRepresentativeOfType(int mtype, bool check_variables = false, bool check_functions = false) const;
		bool containsAdditionPower() const;
		bool containsUnknowns() const;
		bool containsDivision() const;
		void findAllUnknowns(MathStructure &unknowns_vector);
		bool replace(const MathStructure &mfrom, const MathStructure &mto);
		bool replace(const MathStructure &mfrom1, const MathStructure &mto1, const MathStructure &mfrom2, const MathStructure &mto2);
		
		MathStructure generateVector(MathStructure x_mstruct, const MathStructure &min, const MathStructure &max, int steps, MathStructure *x_vector = NULL, const EvaluationOptions &eo = default_evaluation_options);
		MathStructure generateVector(MathStructure x_mstruct, const MathStructure &min, const MathStructure &max, const MathStructure &step, MathStructure *x_vector = NULL, const EvaluationOptions &eo = default_evaluation_options);
		MathStructure generateVector(MathStructure x_mstruct, const MathStructure &x_vector, const EvaluationOptions &eo = default_evaluation_options);
		
		bool differentiate(const MathStructure &x_var, const EvaluationOptions &eo);
		bool integrate(const MathStructure &x_var, const EvaluationOptions &eo);
		
		const MathStructure &find_x_var() const;
		bool isolate_x(const EvaluationOptions &eo, const MathStructure &x_var = m_undefined);
		
		

};

#endif
