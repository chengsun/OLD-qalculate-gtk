/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef FUNCTION_H
#define FUNCTION_H

#include "ExpressionItem.h"
#include "includes.h"

enum {
	ARGUMENT_TYPE_FREE,
	ARGUMENT_TYPE_SYMBOLIC,
	ARGUMENT_TYPE_TEXT,
	ARGUMENT_TYPE_DATE,
	ARGUMENT_TYPE_FILE,
	ARGUMENT_TYPE_INTEGER,	
	ARGUMENT_TYPE_NUMBER,
	ARGUMENT_TYPE_VECTOR,	
	ARGUMENT_TYPE_MATRIX,
	ARGUMENT_TYPE_EXPRESSION_ITEM,
	ARGUMENT_TYPE_FUNCTION,	
	ARGUMENT_TYPE_UNIT,
	ARGUMENT_TYPE_BOOLEAN,
	ARGUMENT_TYPE_VARIABLE,
	ARGUMENT_TYPE_ANGLE,
	ARGUMENT_TYPE_GIAC,
	ARGUMENT_TYPE_SET
};

typedef enum {
	ARGUMENT_MIN_MAX_NONE,
	ARGUMENT_MIN_MAX_POSITIVE,
	ARGUMENT_MIN_MAX_NONZERO,
	ARGUMENT_MIN_MAX_NONNEGATIVE,
	ARGUMENT_MIN_MAX_NEGATIVE	
} ArgumentMinMaxPreDefinition;


class Function : public ExpressionItem {

  protected:

	int argc;
	int max_argc;
	vector<string> default_values;
	Sgi::hash_map<unsigned int, Argument*> argdefs;
	Sgi::hash_map<unsigned int, int> argoccs;
	unsigned int last_argdef_index;		
	bool testArguments(MathStructure &vargs);
	virtual MathStructure createFunctionMathStructureFromVArgs(const MathStructure &vargs);
	virtual MathStructure createFunctionMathStructureFromSVArgs(vector<string> &svargs);	
	string scondition;
	
  public:
  
	Function(string name_, int argc_, int max_argc_ = 0, string unicode_name = "", string cat_ = "", string title_ = "", string descr_ = "", bool is_active = true);
	Function(const Function *function);
	Function();
	virtual ~Function();	

	virtual ExpressionItem *copy() const = 0;
	virtual void set(const ExpressionItem *item);
	virtual int type() const;

	bool testArgumentCount(int itmp);
	virtual MathStructure calculate(const string &eq, const EvaluationOptions &eo = default_evaluation_options);
	virtual MathStructure parse(const string &eq, const ParseOptions &po = default_parse_options);
	virtual MathStructure calculate(MathStructure &vargs, const EvaluationOptions &eo = default_evaluation_options);	
	virtual int calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo);	
	string condition() const;
	void setCondition(string expression);
	bool testCondition(const MathStructure &vargs);
	int countArgOccurence(unsigned int arg_);
	int args() const;
	int minargs() const;	
	int maxargs() const;		
	int args(const string &str, MathStructure &vargs, const ParseOptions &po = default_parse_options);
	unsigned int lastArgumentDefinitionIndex() const;
	Argument *getArgumentDefinition(unsigned int index);
	void clearArgumentDefinitions();
	void setArgumentDefinition(unsigned int index, Argument *argdef);
	int stringArgs(const string &str, vector<string> &svargs);		
	void setDefaultValue(unsigned int arg_, string value_);
	string getDefaultValue(unsigned int arg_) const;	
	MathStructure produceVector(const MathStructure &vargs, int begin = -1, int end = -1);
	MathStructure produceArgumentsVector(const MathStructure &vargs, int begin = -1, int end = -1);
	
	virtual bool representsPositive(const MathStructure &vargs) const {return false;}
	virtual bool representsNegative(const MathStructure &vargs) const {return false;}
	virtual bool representsNonNegative(const MathStructure &vargs) const {return false;}
	virtual bool representsNonPositive(const MathStructure &vargs) const {return false;}
	virtual bool representsInteger(const MathStructure &vargs) const {return false;}
	virtual bool representsNumber(const MathStructure &vargs) const {return false;}
	virtual bool representsRational(const MathStructure &vargs) const {return false;}
	virtual bool representsReal(const MathStructure &vargs) const {return false;}
	virtual bool representsComplex(const MathStructure &vargs) const {return false;}
	virtual bool representsNonZero(const MathStructure &vargs) const {return false;}
	virtual bool representsEven(const MathStructure &vargs) const {return false;}
	virtual bool representsOdd(const MathStructure &vargs) const {return false;}
	virtual bool representsUndefined(const MathStructure &vargs) const {return false;}
	
};

class UserFunction : public Function {
  protected:
  
	string eq, eq_calc;	
	
  public:
  
	UserFunction(string cat_, string name_, string eq_, bool is_local = true, int argc_ = -1, string title_ = "", string descr_ = "", int max_argc_ = 0, bool is_active = true, string unicode_name = "");
	UserFunction(const UserFunction *function);
	void set(const ExpressionItem *item);
	ExpressionItem *copy() const;
	string equation() const;
	string internalEquation() const;
	int calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo);	
	void setEquation(string new_eq, int argc_ = -1, int max_argc_ = 0);	
};

class Argument {

  protected:
  
  	string sname, scondition;
	bool b_zero, b_test, b_matrix, b_text, b_error;
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;
	virtual string subprintlong() const;
	
  public:
  
	Argument(string name_ = "", bool does_test = true, bool does_error = true);	
	Argument(const Argument *arg);
	virtual ~Argument();

	virtual void set(const Argument *arg);
	virtual Argument *copy() const;

	virtual string print() const;
	string printlong() const;
	
	bool test(MathStructure &value, int index, Function *f, const EvaluationOptions &eo = default_evaluation_options) const;
	//virtual MathStructure evaluate(const string &str, bool keep_exact = true) const;
	virtual void evaluate(MathStructure &mstruct, const EvaluationOptions &eo) const;
	virtual MathStructure parse(const string &str, const ParseOptions &po = default_parse_options) const;
	
	string name() const;
	void setName(string name_);

	void setCustomCondition(string condition);
	string getCustomCondition() const;
	
	bool tests() const;
	void setTests(bool does_error);
	
	bool alerts() const;
	void setAlerts(bool does_error);

	bool zeroForbidden() const;
	void setZeroForbidden(bool forbid_zero);
	
	bool matrixAllowed() const;
	void setMatrixAllowed(bool allow_matrix);
	
	virtual bool suggestsQuotes() const;
	
	virtual int type() const;

};

class NumberArgument : public Argument {

  protected:
  
	Number *fmin, *fmax;
	bool b_incl_min, b_incl_max;
	bool b_complex;

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	NumberArgument(string name_ = "", ArgumentMinMaxPreDefinition minmax = ARGUMENT_MIN_MAX_NONE, bool does_test = true, bool does_error = true);
	NumberArgument(const NumberArgument *arg);
	virtual ~NumberArgument();
	
	virtual void set(const Argument *arg);
	virtual Argument *copy() const;

	virtual string print() const;	
	
	void setMin(const Number *nmin);	
	void setIncludeEqualsMin(bool include_equals);
	bool includeEqualsMin() const;	
	const Number *min() const;
	void setMax(const Number *nmax);	
	void setIncludeEqualsMax(bool include_equals);
	bool includeEqualsMax() const;	
	const Number *max() const;	
	
	bool complexAllowed() const;
	void setComplexAllowed(bool allow_complex);

	virtual int type() const;

};

class IntegerArgument : public Argument {

  protected:
  
	Number *imin, *imax;

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	IntegerArgument(string name_ = "", ArgumentMinMaxPreDefinition minmax = ARGUMENT_MIN_MAX_NONE, bool does_test = true, bool does_error = true);
	IntegerArgument(const IntegerArgument *arg);
	virtual ~IntegerArgument();

	virtual void set(const Argument *arg);
	virtual Argument *copy() const;

	virtual string print() const;	

	void setMin(const Number *nmin);	
	const Number *min() const;
	void setMax(const Number *nmax);	
	const Number *max() const;
	
	virtual int type() const;

};

class SymbolicArgument : public Argument {

  protected:
  
  	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	SymbolicArgument(string name_ = "", bool does_test = true, bool does_error = true);
	SymbolicArgument(const SymbolicArgument *arg);
	virtual ~SymbolicArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};

class TextArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	TextArgument(string name_ = "", bool does_test = true, bool does_error = true);
	TextArgument(const TextArgument *arg);
	virtual ~TextArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	virtual bool suggestsQuotes() const;
};

class GiacArgument : public Argument {

  protected:
  
	virtual string subprintlong() const;

  public:
  
  	GiacArgument(string name_ = "", bool does_test = true, bool does_error = true);
	GiacArgument(const GiacArgument *arg);
	virtual ~GiacArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	//virtual MathStructure evaluate(const string &str, bool keep_exact = true) const;
	virtual MathStructure parse(const string &str, const ParseOptions &po = default_parse_options) const;
};

class DateArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	DateArgument(string name_ = "", bool does_test = true, bool does_error = true);
	DateArgument(const DateArgument *arg);
	virtual ~DateArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class VectorArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;
	vector<Argument*> subargs;
	bool b_argloop;

  public:
  
  	VectorArgument(string name_ = "", bool does_test = true, bool allow_matrix = false, bool does_error = true);
	VectorArgument(const VectorArgument *arg);
	virtual ~VectorArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	bool reoccuringArguments() const;
	void setReoccuringArguments(bool reocc);
	void addArgument(Argument *arg);
	void delArgument(unsigned int index);
	unsigned int countArguments() const;
	Argument *getArgument(unsigned int index) const;
};
class MatrixArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;
	bool b_sym;

  public:
  
  	MatrixArgument(string name_ = "", bool does_test = true, bool does_error = true);
	MatrixArgument(const MatrixArgument *arg);
	virtual bool symmetricDemanded() const;
	virtual void setSymmetricDemanded(bool sym);
	virtual ~MatrixArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class ExpressionItemArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	ExpressionItemArgument(string name_ = "", bool does_test = true, bool does_error = true);
	ExpressionItemArgument(const ExpressionItemArgument *arg);
	virtual ~ExpressionItemArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class FunctionArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	FunctionArgument(string name_ = "", bool does_test = true, bool does_error = true);
	FunctionArgument(const FunctionArgument *arg);
	virtual ~FunctionArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class BooleanArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	BooleanArgument(string name_ = "", bool does_test = true, bool does_error = true);
	BooleanArgument(const BooleanArgument *arg);
	virtual ~BooleanArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class UnitArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	UnitArgument(string name_ = "", bool does_test = true, bool does_error = true);
	UnitArgument(const UnitArgument *arg);
	virtual ~UnitArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class AngleArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	AngleArgument(string name_ = "", bool does_test = true, bool does_error = true);
	AngleArgument(const AngleArgument *arg);
	virtual ~AngleArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	//virtual MathStructure evaluate(const string &str, bool keep_exact = true) const;
	virtual MathStructure parse(const string &str, const ParseOptions &po = default_parse_options) const;
};
class VariableArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	VariableArgument(string name_ = "", bool does_test = true, bool does_error = true);
	VariableArgument(const VariableArgument *arg);
	virtual ~VariableArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};
class FileArgument : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	FileArgument(string name_ = "", bool does_test = true, bool does_error = true);
	FileArgument(const FileArgument *arg);
	virtual ~FileArgument();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
};

class ArgumentSet : public Argument {

  protected:
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;
	vector<Argument*> subargs;

  public:
  
  	ArgumentSet(string name_ = "", bool does_test = true, bool does_error = true);
	ArgumentSet(const ArgumentSet *arg);
	virtual ~ArgumentSet();
	virtual int type() const;
	virtual Argument *copy() const;
	virtual string print() const;
	void addArgument(Argument *arg);
	void delArgument(unsigned int index);
	unsigned int countArguments() const;
	Argument *getArgument(unsigned int index) const;
	
};

#endif
