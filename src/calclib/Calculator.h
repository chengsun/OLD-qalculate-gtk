/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "includes.h"
#include "util.h"
#include <pthread.h>

extern Calculator *calculator;

#define CALCULATOR	calculator

typedef vector<Prefix*> p_type;

struct plot_parameters {
	string title;
	string y_label, x_label;
	string filename;
	PlotFileType filetype;
	string font;
	bool color;
	bool auto_y_min, auto_x_min;
	bool auto_y_max, auto_x_max;
	float y_min, x_min;
	float y_max, x_max;
	bool y_log, x_log;
	int y_log_base, x_log_base;
	bool grid;
	int linewidth;
	bool show_all_borders;
	PlotLegendPlacement legend_placement;
	plot_parameters();
};

struct plot_data_parameters {
	string title;
	PlotSmoothing smoothing;
	PlotStyle style;
	bool yaxis2;
	bool xaxis2;
	plot_data_parameters();
};

class CalculatorError {
  protected:
	string smessage;
	bool bcritical;
  public:
	CalculatorError(string message_, bool critical_);
	CalculatorError(const CalculatorError &e);
	string message(void) const;
	const char* c_message(void) const;	
	bool critical(void) const;
};

extern MathStructure m_undefined, m_empty_vector, m_empty_matrix, m_zero, m_one;
extern EvaluationOptions no_evaluation;

class Calculator {
  protected:
	vector<CalculatorError> errors;
	int error_id;
	int ianglemode;
	int i_precision;
	char vbuffer[200];
	vector<void*> ufv;
	vector<char> ufv_t;	
	
	vector<MathStructure> id_structs;
	vector<unsigned int> ids;
	vector<bool> ids_p;
	vector<unsigned int> freed_ids;	
	unsigned int ids_i;
	
	vector<string> signs;	
	vector<string> real_signs;
	vector<string> default_signs;	
	vector<string> default_real_signs;	
	char *saved_locale;
	int disable_errors_ref;
	pthread_t calculate_thread;
	pthread_attr_t calculate_thread_attr;
	pthread_t print_thread;
	pthread_attr_t print_thread_attr;
	bool b_functions_was, b_variables_was, b_units_was, b_unknown_was, b_calcvars_was, b_rpn_was;
	string NAME_NUMBER_PRE_S, NAME_NUMBER_PRE_STR, DOT_STR, DOT_S, COMMA_S, COMMA_STR, ILLEGAL_IN_NAMES, ILLEGAL_IN_UNITNAMES, ILLEGAL_IN_NAMES_MINUS_SPACE_STR;

	bool b_argument_errors;
	bool exchange_rates_warning_issued;

	bool b_gnuplot_open;
	string gnuplot_cmdline;
	FILE *gnuplot_pipe, *calculate_pipe_r, *calculate_pipe_w, *print_pipe_r, *print_pipe_w;
	
	bool local_to;
	
	Assumptions *default_assumptions;
	
  public:

	KnownVariable *v_pi, *v_e, *v_i, *v_inf, *v_pinf, *v_minf;
	UnknownVariable *v_x, *v_y, *v_z;
	Function *f_vector, *f_sort, *f_rank, *f_limits, *f_component, *f_components, *f_merge_vectors;
	Function *f_matrix, *f_matrix_to_vector, *f_area, *f_rows, *f_columns, *f_row, *f_column, *f_elements, *f_element, *f_transpose, *f_identity, *f_determinant, *f_permanent, *f_adjoint, *f_cofactor, *f_inverse; 
	Function *f_factorial, *f_binomial;
	Function *f_abs, *f_gcd, *f_signum, *f_round, *f_floor, *f_ceil, *f_trunc, *f_int, *f_frac, *f_rem, *f_mod;
	Function *f_re, *f_im, *f_arg;
  	Function *f_sqrt, *f_sq;
	Function *f_exp;
	Function *f_ln, *f_logn;
	Function *f_sin, *f_cos, *f_tan, *f_asin, *f_acos, *f_atan, *f_sinh, *f_cosh, *f_tanh, *f_asinh, *f_acosh, *f_atanh, *f_radians_to_default_angle_unit;
	Function *f_zeta, *f_gamma, *f_beta;
	Function *f_total, *f_percentile, *f_min, *f_max, *f_mode, *f_rand;
	Function *f_days, *f_yearfrac, *f_week, *f_weekday, *f_month, *f_day, *f_year, *f_yearday, *f_time;
	Function *f_bin, *f_oct, *f_hex, *f_base, *f_roman;
	Function *f_ascii, *f_char;
	Function *f_length, *f_concatenate;
	Function *f_replace;
	Function *f_for, *f_sum, *f_product, *f_process, *f_process_matrix, *f_csum;
	Function *f_diff, *f_solve;
	Unit *u_rad, *u_euro;
	Prefix *null_prefix;
  
  	bool b_busy, calculate_thread_stopped, print_thread_stopped;
	string expression_to_calculate, tmp_print_result;
	PrintOptions tmp_printoptions;
	EvaluationOptions tmp_evaluationoptions;
	
	PrintOptions save_printoptions;	
  
	vector<Variable*> variables;
	vector<Function*> functions;	
	vector<Unit*> units;	
	vector<Prefix*> prefixes;
  
	Calculator();
	virtual ~Calculator();

	bool utf8_pos_is_valid_in_name(char *pos);

	void addStringAlternative(string replacement, string standard);
	bool delStringAlternative(string replacement, string standard);
	void addDefaultStringAlternative(string replacement, string standard);
	bool delDefaultStringAlternative(string replacement, string standard);

	bool showArgumentErrors() const;
	void beginTemporaryStopErrors();
	void endTemporaryStopErrors();	
	
	unsigned int addId(const MathStructure &m_struct, bool persistent = false);
	const MathStructure *getId(unsigned int id);	
	void delId(unsigned int id, bool force = false);

	Variable *getVariable(unsigned int index) const;
	Unit *getUnit(unsigned int index) const;	
	Function *getFunction(unsigned int index) const;	
	
	void setDefaultAssumptions(Assumptions *ass);
	Assumptions *defaultAssumptions();

	Prefix *getPrefix(unsigned int index) const;	
	Prefix *getPrefix(string name_) const;		
	Prefix *getExactPrefix(int exp10, int exp = 1) const;			
	Prefix *getExactPrefix(const Number &o, int exp = 1) const;				
	Prefix *getNearestPrefix(int exp10, int exp = 1) const;		
	Prefix *getBestPrefix(int exp10, int exp = 1, bool all_prefixes = true) const;		
	Prefix *getBestPrefix(const Number &exp10, const Number &exp, bool all_prefixes = true) const;
	Prefix *addPrefix(Prefix *p);
	void prefixNameChanged(Prefix *p);	

	void setPrecision(int precision = DEFAULT_PRECISION);
	int getPrecision() const;

	const string &getDecimalPoint() const;
	const string &getComma() const;	
	void setLocale();
	void unsetLocale();
	
	int angleMode() const;
	void angleMode(int mode_);
	void resetVariables();
	void resetFunctions();	
	void resetUnits();		
	void reset();		
	void addBuiltinVariables();	
	void addBuiltinFunctions();
	void addBuiltinUnits();	
	void saveState();
	void restoreState();
	void clearBuffers();
	void abort();
	void abort_this();
	bool busy();
	void terminateThreads();
	
	string localizeExpression(string str) const;
	string unlocalizeExpression(string str) const;
	bool calculate(MathStructure &mstruct, string str, int usecs, const EvaluationOptions &eo = default_evaluation_options);
	MathStructure calculate(string str, const EvaluationOptions &eo = default_evaluation_options);
	string printMathStructureTimeOut(const MathStructure &mstruct, int usecs = 100000, const PrintOptions &op = default_print_options);
	
	MathStructure parse(string str, const ParseOptions &po = default_parse_options);
	MathStructure parseNumber(string str, const ParseOptions &po = default_parse_options);
	MathStructure parseOperators(string str, const ParseOptions &po = default_parse_options);
	void parseAdd(string &str, MathStructure &mstruct, const ParseOptions &po, MathOperation s);
	void parseAdd(string &str, MathStructure &mstruct, const ParseOptions &po);
	
	MathStructure convert(double value, Unit *from_unit, Unit *to_unit, const EvaluationOptions &eo = default_evaluation_options);
	MathStructure convert(string str, Unit *from_unit, Unit *to_unit, const EvaluationOptions &eo = default_evaluation_options);
	MathStructure convert(const MathStructure &mstruct, Unit *to_unit, const EvaluationOptions &eo = default_evaluation_options, bool always_convert = true);
	MathStructure convert(const MathStructure &mstruct, string composite_, const EvaluationOptions &eo = default_evaluation_options);
	MathStructure convertToBaseUnits(const MathStructure &mstruct, const EvaluationOptions &eo = default_evaluation_options);
	Unit *getBestUnit(Unit *u, bool allow_only_div = false);
	MathStructure convertToBestUnit(const MathStructure &mstruct, const EvaluationOptions &eo = default_evaluation_options);
	MathStructure convertToCompositeUnit(const MathStructure &mstruct, CompositeUnit *cu, const EvaluationOptions &eo = default_evaluation_options, bool always_convert = true);
	
	void expressionItemActivated(ExpressionItem *item);
	void expressionItemDeactivated(ExpressionItem *item);
	void expressionItemDeleted(ExpressionItem *item);
	void nameChanged(ExpressionItem *item);
	void deleteName(string name_, ExpressionItem *object = NULL);
	void deleteUnitName(string name_, Unit *object = NULL);	
	Unit* addUnit(Unit *u, bool force = true);
	void delUFV(void *object);		
	ExpressionItem *getActiveExpressionItem(string name, ExpressionItem *item = NULL);
	ExpressionItem *getInactiveExpressionItem(string name, ExpressionItem *item = NULL);	
	ExpressionItem *getActiveExpressionItem(ExpressionItem *item);
	ExpressionItem *getExpressionItem(string name, ExpressionItem *item = NULL);
	Unit* getUnit(string name_);
	Unit* getActiveUnit(string name_);
	Unit* getCompositeUnit(string internal_name_);	
	Variable* addVariable(Variable *v, bool force = true);
	void variableNameChanged(Variable *v);
	void functionNameChanged(Function *f, bool priviliged = false);	
	void unitNameChanged(Unit *u);	
	void unitSingularChanged(Unit *u);	
	void unitPluralChanged(Unit *u);	
	Variable* getVariable(string name_);
	Variable* getActiveVariable(string name_);
	ExpressionItem *addExpressionItem(ExpressionItem *item, bool force = true);
	Function* addFunction(Function *f, bool force = true);
	Function* getFunction(string name_);	
	Function* getActiveFunction(string name_);	
	void error(bool critical, const char *TEMPLATE,...);
	CalculatorError *error();
	CalculatorError *nextError();
	bool variableNameIsValid(string name_);
	string convertToValidVariableName(string name_);	
	bool functionNameIsValid(string name_);
	string convertToValidFunctionName(string name_);		
	bool unitNameIsValid(string name_);
	string convertToValidUnitName(string name_);		
	bool nameTaken(string name, ExpressionItem *object = NULL);
	bool unitIsUsedByOtherUnits(const Unit *u) const;	
	string getName(string name = "", ExpressionItem *object = NULL, bool force = false, bool always_append = false);
	bool loadGlobalDefinitions();
	bool loadLocalDefinitions();
	int loadDefinitions(const char* file_name, bool is_user_defs = true);
	bool saveDefinitions();	
	int savePrefixes(const char* file_name, bool save_global = false);	
	int saveVariables(const char* file_name, bool save_global = false);	
	int saveUnits(const char* file_name, bool save_global = false);	
	int saveFunctions(const char* file_name, bool save_global = false);	
	MathStructure setAngleValue(const MathStructure &mstruct);	
	bool importCSV(MathStructure &mstruct, const char *file_name, int first_row = 1, string delimiter = ",", vector<string> *headers = NULL);
	bool importCSV(const char *file_name, int first_row = 1, bool headers = true, string delimiter = ",", bool to_matrix = false, string name = "", string title = "", string category = "");
	int testCondition(string expression);
	
	bool canFetch();
	bool loadExchangeRates();
	bool fetchExchangeRates();
	bool checkExchangeRatesDate();
	
	bool canPlot();
	MathStructure expressionToPlotVector(string expression, const MathStructure &min, const MathStructure &max, int steps, MathStructure *x_vector = NULL, string x_var = "\\x");
	MathStructure expressionToPlotVector(string expression, float min, float max, int steps, MathStructure *x_vector = NULL, string x_var = "\\x");
	MathStructure expressionToPlotVector(string expression, const MathStructure &x_vector, string x_var = "\\x");
	//bool plotVectors(plot_parameters *param, const MathStructure *y_vector, ...);
	bool plotVectors(plot_parameters *param, const vector<MathStructure> &y_vectors, const vector<MathStructure> &x_vectors, vector<plot_data_parameters*> &pdps, bool persistent = false);
	bool invokeGnuplot(string commands, string commandline_extra = "", bool persistent = false);
	bool closeGnuplot();
	bool gnuplotOpen();
		
};

#endif
