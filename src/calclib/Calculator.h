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

class Calculator {
  protected:
	vector<Error*> errors;
	int error_id;
	int ianglemode;
	int i_precision;
	char vbuffer[200];
	bool b_functions, b_variables, b_units, b_unknown, b_calcvars, b_always_exact, b_rpn, b_use_all_prefixes, b_multiple_roots, b_den_prefix;
	vector<void*> ufv;
	vector<char> ufv_t;	
	Sgi::hash_map<unsigned int, Manager*> ids;
	Sgi::hash_map<unsigned int, bool> ids_p;	
	vector<string> signs;	
	vector<string> real_signs;
	vector<string> default_signs;	
	vector<string> default_real_signs;	
	char *saved_locale;
	int disable_errors_ref;
	pthread_t calculate_thread;
	pthread_attr_t calculate_thread_attr;
	bool b_functions_was, b_variables_was, b_units_was, b_unknown_was, b_calcvars_was, b_always_exact_was, b_rpn_was;
	string NAME_NUMBER_PRE_S, NAME_NUMBER_PRE_STR, DOT_STR, DOT_S, COMMA_S, COMMA_STR, ILLEGAL_IN_NAMES, ILLEGAL_IN_UNITNAMES, ILLEGAL_IN_NAMES_MINUS_SPACE_STR;

	bool b_gnuplot_open;
	string gnuplot_cmdline;
	FILE *gnuplot_pipe;
	
	bool local_to;
	
	Variable *pi_var, *e_var;
	Function *ln_func, *vector_func, *matrix_func, *abs_func, *diff_func, *bin_func, *oct_func, *hex_func, *base_func, *integrate_func;

  public:
  
  	bool b_busy;
	string expression_to_calculate, tmp_print_result;
	NumberFormat tmp_nrformat; int tmp_displayflags; int tmp_min_decimals; 
	int tmp_max_decimals; bool *tmp_in_exact; bool *tmp_usable; Prefix *tmp_prefix;	
  
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

	bool alwaysExact() const;
	void setAlwaysExact(bool always_exact);
	
	bool multipleRootsEnabled() const;
	void setMultipleRootsEnabled(bool enable_multiple_roots);

	void beginTemporaryStopErrors();
	void endTemporaryStopErrors();	

	Variable *getPI() const;
	Variable *getE() const;
	Function *getLnFunction() const;
	Function *getOctalFunction() const;
	Function *getHexadecimalFunction() const;
	Function *getBinaryFunction() const;
	Function *getBaseFunction() const;
	Function *getAbsFunction() const;
	Function *getDiffFunction() const;
	Function *getIntegrateFunction() const;
	Variable *getVariable(unsigned int index) const;
	Unit *getUnit(unsigned int index) const;	
	Function *getFunction(unsigned int index) const;	

	Prefix *getPrefix(unsigned int index) const;	
	Prefix *getPrefix(string name_) const;		
	Prefix *getExactPrefix(long int exp10, long int exp = 1) const;			
	Prefix *getExactPrefix(const Number *fr, long int exp = 1) const;				
	Prefix *getNearestPrefix(long int exp10, long int exp = 1) const;		
	Prefix *getBestPrefix(long int exp10, long int exp = 1) const;		
	Prefix *getBestPrefix(const Number *exp10, const Number *exp) const;				
	Prefix *addPrefix(Prefix *p);
	void prefixNameChanged(Prefix *p);	

	void setPrecision(int precision = DEFAULT_PRECISION);
	int getPrecision() const;

	const char *getDecimalPoint() const;
	const char *getComma() const;	
	void setLocale();
	void unsetLocale();
	int addId(Manager *mngr, bool persistent = false);
	Manager *getId(unsigned int id);	
	void delId(unsigned int id, bool force = false);
	bool allPrefixesEnabled() const;
	void setAllPrefixesEnabled(bool enable);
	bool functionsEnabled() const;
	void setFunctionsEnabled(bool enable);
	bool variablesEnabled() const;
	void setVariablesEnabled(bool enable);	
	bool unknownVariablesEnabled() const;
	void setUnknownVariablesEnabled(bool enable);		
	bool donotCalculateVariables() const;	
	void setDonotCalculateVariables(bool enable);			
	bool unitsEnabled() const;
	void setUnitsEnabled(bool enable);
	bool denominatorPrefixEnabled() const;
	void setDenominatorPrefixEnabled(bool enable);	
	void setRPNMode(bool enable);
	bool inRPNMode() const;
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
	string localizeExpression(string str) const;
	string unlocalizeExpression(string str) const;
	Manager *calculate(string str, bool enable_abort = false, int usecs = -1);
	string printManagerTimeOut(Manager *mngr, int usecs = 100000, NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, bool *in_exact = NULL, bool *usable = NULL, Prefix *prefix = NULL);
	Manager *convert(double value, Unit *from_unit, Unit *to_unit);
	Manager *convert(string str, Unit *from_unit, Unit *to_unit);	
	Manager *convert(Manager *mngr, Unit *to_unit, bool always_convert = true);		
	Manager *convert(Manager *mngr, string composite_);
	Manager *convertToBaseUnits(Manager *mngr);
	Unit *getBestUnit(Unit *u, bool allow_only_div = false);
	Manager *convertToBestUnit(Manager *mngr);
	Manager *convertToCompositeUnit(Manager *mngr, CompositeUnit *cu, bool always_convert = true);		
	void expressionItemActivated(ExpressionItem *item);
	void expressionItemDeactivated(ExpressionItem *item);
	void expressionItemDeleted(ExpressionItem *item);
	void nameChanged(ExpressionItem *item);
	void checkFPExceptions();
	void checkFPExceptions(const char *str);	
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
	void setFunctionsAndVariables(string &str);
	Variable* getVariable(string name_);
	Variable* getActiveVariable(string name_);
	ExpressionItem *addExpressionItem(ExpressionItem *item, bool force = true);
	Function* addFunction(Function *f, bool force = true);
	Function* getFunction(string name_);	
	Function* getActiveFunction(string name_);	
	void error(bool critical, const char *TEMPLATE,...);
	Error* error();
	Error* nextError();
	bool variableNameIsValid(string name_);
	string convertToValidVariableName(string name_);	
	bool functionNameIsValid(string name_);
	string convertToValidFunctionName(string name_);		
	bool unitNameIsValid(string name_);
	string convertToValidUnitName(string name_);		
	bool nameTaken(string name, ExpressionItem *object = NULL);
	bool unitIsUsedByOtherUnits(const Unit *u) const;	
	string getName(string name = "", ExpressionItem *object = NULL, bool force = false, bool always_append = false);
	//bool load(const char* file_name, bool is_user_defs = true);
	bool loadGlobalDefinitions();
	bool loadLocalDefinitions();
	int loadDefinitions(const char* file_name, bool is_user_defs = true);
	bool saveDefinitions();	
	int savePrefixes(const char* file_name, bool save_global = false);	
	int saveVariables(const char* file_name, bool save_global = false);	
	int saveUnits(const char* file_name, bool save_global = false);	
	int saveFunctions(const char* file_name, bool save_global = false);	
	Manager *setAngleValue(Manager *mngr);	
	Matrix *importCSV(const char *file_name, int first_row = 1, string delimiter = ",", vector<string> *headers = NULL);
	bool importCSV(const char *file_name, int first_row = 1, bool headers = true, string delimiter = ",", bool to_matrix = false, string name = "", string title = "", string category = "");
	int testCondition(string expression);
	
	bool canFetch();
	bool loadExchangeRates();
	bool fetchExchangeRates();
	
	bool canPlot();
	Vector *expressionToVector(string expression, const Manager *min, const Manager *max, int steps, Vector **x_vector = NULL, string x_var = "\\x");
	Vector *expressionToVector(string expression, float min, float max, int steps, Vector **x_vector = NULL, string x_var = "\\x");
	Vector *expressionToVector(string expression, Vector *x_vector, string x_var = "\\x");
	bool plotVectors(plot_parameters *param, Vector *y_vector, ...);
	bool plotVectors(plot_parameters *param, vector<Vector*> &y_vectors, vector<Vector*> &x_vectors, vector<plot_data_parameters*> &pdps, bool persistent = false);
	bool invokeGnuplot(string commands, string commandline_extra = "", bool persistent = false);
	bool closeGnuplot();
	bool gnuplotOpen();
		
};

#endif
