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

class Calculator;

#include "includes.h"
#include "util.h"
#include "Manager.h"
#include "Unit.h"
#include "EqItem.h"
#include "Variable.h"
#include "Function.h"
#include "Error.h"
#include "Prefix.h"
#include <ext/hash_map>

extern Calculator *calculator;

#define CALCULATOR	calculator

typedef vector<Prefix*> p_type;

class Calculator {
  protected:
	stack<Error*> errors;
	int error_id;
	int ianglemode;
	char vbuffer[200];
	bool b_functions, b_variables, b_units, b_unknown, b_calcvars;
	vector<void*> ufv;
	vector<char> ufv_t;	
	hash_map<int, Manager*> ids;
	vector<string> signs;	
	vector<string> real_signs;
	vector<string> default_signs;	
	vector<string> default_real_signs;	
  public:
  
	vector<Variable*> variables;
	vector<Function*> functions;	
	vector<Unit*> units;	
	vector<Prefix*> prefixes;
  
	Calculator(void);
	~Calculator(void);

	void addStringAlternative(string replacement, string standard);
	void addDefauktStringAlternative(string replacement, string standard);

	Variable *getVariable(int index) const;
	Unit *getUnit(int index) const;	
	Function *getFunction(int index) const;	

	Prefix *getPrefix(int index) const;	
	Prefix *getPrefix(string name_) const;		
	Prefix *getExactPrefix(long int exp10, long int exp = 1) const;			
	Prefix *getNearestPrefix(long int exp10, long int exp = 1) const;		
	Prefix *getBestPrefix(long int exp10, long int exp = 1) const;		
	Prefix *addPrefix(Prefix *p);
	void prefixNameChanged(Prefix *p);	

	const char *getDecimalPoint() const;
	const char *getComma() const;	
	void setLocale();
	void unsetLocale();
	string &remove_trailing_zeros(string &str, int decimals_to_keep = 0, bool expand = false, bool decrease = false);	
	int addId(Manager *mngr);
	Manager *getId(int id);	
	void delId(int id);
	bool functionsEnabled(void);
	void setFunctionsEnabled(bool enable);
	bool variablesEnabled(void);
	void setVariablesEnabled(bool enable);	
	bool unknownVariablesEnabled(void);
	void setUnknownVariablesEnabled(bool enable);		
	bool donotCalculateVariables(void);	
	void setDonotCalculateVariables(bool enable);			
	bool unitsEnabled(void);
	void setUnitsEnabled(bool enable);	
	int angleMode(void);
	void angleMode(int mode_);
	void resetVariables(void);
	void resetFunctions(void);	
	void resetUnits(void);		
	void reset(void);		
	void addBuiltinVariables(void);	
	void addBuiltinFunctions(void);
	void addBuiltinUnits(void);	
	Manager *calculate(string str);
	Manager *convert(long double value, Unit *from_unit, Unit *to_unit);
	Manager *convert(string str, Unit *from_unit, Unit *to_unit);	
	Manager *convert(Manager *mngr, Unit *to_unit, bool always_convert = true);		
	Manager *convert(Manager *mngr, string composite_);	
	Manager *convertToCompositeUnit(Manager *mngr, CompositeUnit *cu, bool always_convert = true);		
	void checkFPExceptions(void);
	void checkFPExceptions(const char *str);	
	void deleteName(string name_, void *object = NULL);
	void deleteUnitName(string name_, Unit *object = NULL);	
	Unit* addUnit(Unit *u, bool force = true);
	void delUnit(Unit *u);	
	void delUFV(void *object);		
	Unit* getUnit(string name_);
	Unit* getCompositeUnit(string internal_name_);	
	Variable* addVariable(Variable *v, bool force = true);
	void variableNameChanged(Variable *v);
	void functionNameChanged(Function *f, bool priviliged = false);	
	void unitNameChanged(Unit *u);	
	void unitShortNameChanged(Unit *u);	
	void unitPluralChanged(Unit *u);		
	void setFunctionsAndVariables(string &str);
	void setVariables(string &str);
	void delVariable(Variable *v);	
	Variable* getVariable(string name_);
	Function* addFunction(Function *f, bool force = true);
	void setFunctions(string &str);
	void delFunction(Function *f);		
	Function* getFunction(string name_);	
	void error(bool critical, const char *TEMPLATE,...);
	Error* error(void);
	Error* nextError(void);
	bool variableNameIsValid(string name_);
	string convertToValidVariableName(string name_);	
	bool functionNameIsValid(string name_);
	string convertToValidFunctionName(string name_);		
	bool unitNameIsValid(string name_);
	string convertToValidUnitName(string name_);		
	bool nameTaken(string name_, void *object = NULL);
	bool unitNameTaken(string name_, Unit *u = NULL);
	bool unitIsUsedByOtherUnits(Unit *u);	
	string getName(string name = "", void *object = NULL, bool force = false, bool always_append = false);
	string getUnitName(string name = "", Unit *object = NULL, bool force = false, bool always_append = false);	
	bool load(const char* file_name, bool is_user_defs = true);
	bool save(const char* file_name);	
	string value2str(long double &value, int precision = PRECISION);	
	string value2str_decimals(long double &value, int precision = PRECISION);	
	string value2str_bin(long double &value, int precision = PRECISION);				
	string value2str_octal(long double &value, int precision = PRECISION);		
	string value2str_hex(long double &value, int precision = PRECISION);			
	string value2str_prefix(long double &value, long int &exp, int precision = PRECISION, bool use_short_prefixes = true, long double *new_value = NULL, Prefix *prefix = NULL, bool print_one = true);
	string value2str_exp(long double &value, int precision = PRECISION);
	string value2str_exp_pure(long double &value, int precision = PRECISION);	
	long double getAngleValue(long double value);
	Manager *setAngleValue(Manager *mngr);	
	
};

#endif
