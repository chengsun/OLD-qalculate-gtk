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
#include <ext/hash_map>

typedef hash_map<const char*, long double, hash<const char*>, eqstr> l_type;
typedef hash_map<char, long double> s_type;

class Calculator {
  protected:
	stack<Error*> errors;
	int error_id;
	int ianglemode;
	char vbuffer[200];
	bool b_functions, b_variables, b_units;
	vector<void*> ufv;
	vector<char> ufv_t;	
	hash_map<int, Manager*> ids;
  public:
	vector<Variable*> variables;
	vector<Function*> functions;	
	vector<Unit*> units;	
	hash_map<const char*, long double, hash<const char*>, eqstr> l_prefix;
	hash_map<char, long double> s_prefix;		
  
	Calculator(void);
	~Calculator(void);
	int addId(Manager *mngr);
	Manager *getId(int id);	
	void delId(int id);
	bool functionsEnabled(void);
	void setFunctionsEnabled(bool enable);
	bool variablesEnabled(void);
	void setVariablesEnabled(bool enable);	
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
	void error(bool critical, int count,...);
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
	bool load(const char* file_name);
	void addPrefix(const string &ntmp, long double value);
	bool getPrefix(const char *str, long double *value); 
	bool getPrefix(const string &str, long double *value); 	
	bool getPrefix(char c, long double *value);	
	char getSPrefix(long double value);		
	const char *getLPrefix(long double value);		
	bool save(const char* file_name);	
	string value2str(long double &value, int precision = PRECISION);	
	string value2str_decimals(long double &value, int precision = PRECISION);		
	string value2str_octal(long double &value, int precision = PRECISION);		
	string value2str_hex(long double &value, int precision = PRECISION);			
	string value2str_prefix(long double &value, int precision = PRECISION, bool use_short_prefixes = true, long double *new_value = NULL);
	string value2str_exp(long double &value, int precision = PRECISION);
	string value2str_exp_pure(long double &value, int precision = PRECISION);	
	long double getAngleValue(long double value);
	
};

#endif
