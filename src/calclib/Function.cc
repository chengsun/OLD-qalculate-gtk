/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Function.h"
#include "util.h"
#include "Calculator.h"
#include "Matrix.h"
#include "Manager.h"

Function::Function(string cat_, string name_, int argc_, string title_, string descr_, int max_argc_, bool is_active) : ExpressionItem(cat_, name_, title_, descr_, false, true, is_active) {
	argc = argc_;
	if(max_argc_ < 0 || argc < 0) {
		max_argc = -1;
	} else if(max_argc_ < argc) {
		max_argc = argc;
	} else {
		max_argc = max_argc_;
		for(int i = 0; i < max_argc - argc; i++) {
			default_values.push_back("0");
		}
	}
	last_arg_name_index = 0;
	last_arg_type_index = 0;
}
Function::Function(const Function *function) {
	set(function);
}
Function::Function() {
	argc = 0;
	max_argc = 0;
	last_arg_name_index = 0;
	last_arg_type_index = 0;
}
Function::~Function() {}

ExpressionItem *Function::copy() const {
	return new Function(this);
}
void Function::set(const ExpressionItem *item) {
	if(item->type() == TYPE_FUNCTION) {
		Function *f = (Function*) item;
		argc = f->minargs();
		max_argc = f->maxargs();
		default_values.clear();
		for(int i = argc + 1; i <= max_argc; i++) {
			setDefaultValue(i, f->getDefaultValue(i));
		}
		last_arg_name_index = f->lastArgumentNameIndex();
		last_arg_type_index = f->lastArgumentTypeIndex();
		argnames.clear();
		argtypes.clear();
		for(int i = 1; i <= f->lastArgumentNameIndex(); i++) {
			setArgumentName(f->argumentName(i), i);
		}
		for(int i = 1; i <= f->lastArgumentTypeIndex(); i++) {
			setArgumentType(f->argumentType(i), i);
		}		
	}
	ExpressionItem::set(item);
}
int Function::type() const {
	return TYPE_FUNCTION;
}

int Function::args() const {
	return max_argc;
}
int Function::minargs() const {
	return argc;
}
int Function::maxargs() const {
	return max_argc;
}
int Function::args(const string &str, vector<Manager*> &vargs) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0, i4 = 0;
	vargs.clear();
	if(!str.empty()) {
		itmp = 1;
		while(1) {
			if((i = str.find(COMMA_CH, i)) != (int) string::npos) {
				if((i3 = str.find(LEFT_BRACKET_CH, i4)) < i && i3 >= 0) {
/*					i = str.find(RIGHT_BRACKET_CH, i3);
					i4 = i;
					if(i == (int) string::npos)
						break;*/
					i4 = i3;
					while(1) {
						i = str.find(RIGHT_BRACKET_CH, i4);
						if(i == string::npos) {
							i4 = i;
							break;	
						}
						i3 = str.find(LEFT_BRACKET_CH, i3 + 1);
						if(i3 == string::npos || i3 > i) {		
							i4 = i;
							break;
						}
						i4 = i + 1;			
					}
					if(i4 == string::npos)
						break;
					i = i4;						
				} else {
					if(itmp <= maxargs() || args() < 0) {
						Manager *mngr = CALCULATOR->calculate(str.substr(i2, i - i2));
						vargs.push_back(mngr);
					}
					i++;
					i2 = i;
					i4 = i;
					itmp++;
				}
			} else {
				if(itmp <= args() || args() < 0) {
					Manager *mngr = CALCULATOR->calculate(str.substr(i2, str.length() - i2));
					vargs.push_back(mngr);			
				}
				break;
			}
		}
	}
	if(itmp < maxargs() && itmp >= minargs()) {
		int itmp2 = itmp;
		while(itmp2 < maxargs()) {
			Manager *mngr = CALCULATOR->calculate(default_values[itmp2 - minargs()]);
			vargs.push_back(mngr);
			itmp2++;
		}
	}
	return itmp;
}
int Function::lastArgumentNameIndex() const {
	return last_arg_name_index;
}
int Function::lastArgumentTypeIndex() const {	
	return last_arg_type_index;
}
string Function::argumentName(int index) {
	if(argnames.count(index)) {
		return argnames[index];
	}
	return "";
}
ArgumentType Function::argumentType(int index) {
	if(argtypes.count(index)) {
		return argtypes[index];
	}
	return ARGUMENT_TYPE_FREE;
}
const char *Function::argumentTypeString(int index) {
	if(argtypes.count(index)) {
		switch(argtypes[index]) {
			case ARGUMENT_TYPE_TEXT: {return "text";}
			case ARGUMENT_TYPE_DATE: {return "date";}
			case ARGUMENT_TYPE_POSITIVE: {return "positive";}
			case ARGUMENT_TYPE_NONNEGATIVE: {return "non-negative";}
			case ARGUMENT_TYPE_NONZERO: {return "non-zero";}			
			case ARGUMENT_TYPE_INTEGER: {return "integer";}
			case ARGUMENT_TYPE_POSITIVE_INTEGER: {return "positive integer";}
			case ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {return "non-negative integer";}
			case ARGUMENT_TYPE_NONZERO_INTEGER: {return "non-zero integer";}			
			case ARGUMENT_TYPE_FRACTION: {return "number";}
			case ARGUMENT_TYPE_VECTOR: {return "vector";}
			case ARGUMENT_TYPE_MATRIX: {return "matrix";}
			case ARGUMENT_TYPE_FUNCTION: {return "function";}
			case ARGUMENT_TYPE_UNIT: {return "unit";}
			case ARGUMENT_TYPE_BOOLEAN: {return "boolean";}
		}
	}
	return NULL;
}
void Function::clearArgumentNames() {
	argnames.clear();
	last_arg_name_index = 0;
	setChanged(true);
}
void Function::setArgumentName(string name_, int index) {
	argnames[index] = name_;
	if(index > last_arg_name_index) {
		last_arg_name_index = index;
	}
	setChanged(true);
}
void Function::clearArgumentTypes() {
	argtypes.clear();
	last_arg_type_index = 0;
	setChanged(true);
}
void Function::setArgumentType(ArgumentType type_, int index) {
	argtypes[index] = type_;
	if(index > last_arg_type_index) {
		last_arg_type_index = index;
	}		
	setChanged(true);
}
bool Function::testArgumentCount(int itmp) {
	if(itmp >= minargs()) {
		if(itmp > maxargs() && maxargs() >= 0)
			CALCULATOR->error(false, _("Additional arguments for function %s() was ignored. Function can only use %s arguments."), name().c_str(), i2s(maxargs()).c_str());						
		return true;	
	}
	CALCULATOR->error(true, _("You need at least %s arguments in function %s()."), i2s(minargs()).c_str(), name().c_str());
	return false;
}
Manager *Function::createFunctionManagerFromVArgs(vector<Manager*> &vargs) {
	Manager *mngr = new Manager(this, NULL);
	for(int i = 0; i < vargs.size(); i++) {
		mngr->addFunctionArg(vargs[i]);
	}
	return mngr;
}
Manager *Function::createFunctionManagerFromSVArgs(vector<string> &svargs) {
	Manager *mngr = new Manager(this, NULL); 
	for(int i = 0; i < svargs.size(); i++) {
		Manager *mngr2 = new Manager(svargs[i]);
		mngr->addFunctionArg(mngr2);
		mngr2->unref();
	}
	return mngr;
}
Manager *Function::calculate(const string &argv) {
	vector<Manager*> vargs;
	args(argv, vargs);	
	Manager *mngr = calculate(vargs);
	for(int i = 0; i < vargs.size(); i++) {
		vargs[i]->unref();
	}	
	return mngr;
}
Manager *Function::calculate(vector<Manager*> &vargs) {
	Manager *mngr = NULL;
	if(testArgumentCount(vargs.size())) {
		mngr = new Manager();
		bool b = false;
		for(int i = 0; i < vargs.size(); i++) {
			if(vargs[i]->type() == ALTERNATIVE_MANAGER) {
				b = true;
				break;
			}
		} 
		if(b) {
			vector<Manager*> vargs_copy(vargs);
			vector<int> solutions;
			solutions.reserve(vargs.size());
			for(int i = 0; i < vargs.size(); i++) {
				solutions.push_back(0);
			}
			b = true;
			while(true) {
				for(int i = 0; i < vargs.size(); i++) {
					vargs[i] = vargs_copy[i];
				}
				for(int i = 0; i < vargs.size(); i++) {
					if(vargs[i]->type() == ALTERNATIVE_MANAGER) {
						if(!b && solutions[i] < vargs[i]->mngrs.size()) {
							vargs[i] = vargs[i]->mngrs[solutions[i]];
							solutions[i]++;
							b = true;
						} else {
							solutions[i] = 0;
							vargs[i] = vargs[i]->mngrs[solutions[i]];
						}
					}
				}
				if(!b) break;
				Manager *mngr2 = new Manager();
				calculate(mngr2, vargs);
				if(!isPrecise()) mngr2->setPrecise(false);
				if(mngr->isNull()) {
					mngr->moveto(mngr2);
				} else {
					mngr->addAlternative(mngr2);
				}
				b = false;	 			
			}
			mngr->altclean();
		} else {
			calculate(mngr, vargs);
		}
		if(!isPrecise()) mngr->setPrecise(false);
		CALCULATOR->checkFPExceptions(sname.c_str());	
	} else {
		mngr = createFunctionManagerFromVArgs(vargs);
	}	
	return mngr;
}
void Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
}
void Function::setDefaultValue(int arg_, string value_) {
	if(arg_ > argc && arg_ <= max_argc && default_values.size() >= arg_ - argc) {
		default_values[arg_ - argc - 1] = value_;
	}
}
string Function::getDefaultValue(int arg_) const {
	if(arg_ > argc && arg_ <= max_argc && default_values.size() >= arg_ - argc) {
		return default_values[arg_ - argc - 1];
	}
	return "";
}
int Function::stringArgs(const string &str, vector<string> &svargs) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0, i4 = 0;
	string stmp;
	svargs.clear();
	if(!str.empty()) {
		itmp = 1;
		while(1) {
			if((i = str.find(COMMA_CH, i)) != (int) string::npos) {
				if((i3 = str.find(LEFT_BRACKET_CH, i4)) < i && i3 != string::npos) {
					i4 = i3;
					while(1) {
						i = str.find(RIGHT_BRACKET_CH, i4);
						if(i == string::npos) {
							i4 = i;
							break;	
						}
						i3 = str.find(LEFT_BRACKET_CH, i3 + 1);
						if(i3 == string::npos || i3 > i) {		
							i4 = i;
							break;
						}
						i4 = i + 1;			
					}
					if(i4 == string::npos)
						break;
					i = i4;
				} else {
					if(itmp <= args() || args() < 0) {
/*						stmp = LEFT_BRACKET_STR;
						stmp += str.substr(i2, i - i2);
						stmp += RIGHT_BRACKET_STR;
						svargs.push_back(stmp);*/
						stmp = str.substr(i2, i - i2);
						remove_blank_ends(stmp);																				
						remove_brackets(stmp);						
						remove_blank_ends(stmp);
						svargs.push_back(stmp);
					}
					i++;
					i2 = i;
					i4 = i;
					itmp++;
				}
			} else {
				if(itmp <= args() || args() < 0) {
/*					stmp = LEFT_BRACKET_STR;
					stmp += str.substr(i2, str.length() - i2);
					stmp += RIGHT_BRACKET_STR;
					svargs.push_back(stmp);*/
					stmp = str.substr(i2, str.length() - i2);
					remove_blank_ends(stmp);					
					remove_brackets(stmp);									
					remove_blank_ends(stmp);
					svargs.push_back(stmp);
				}
				break;
			}
		}
	}
	if(itmp < maxargs() && itmp >= minargs()) {
		int itmp2 = itmp;
		while(itmp2 < maxargs()) {
/*			stmp = LEFT_BRACKET_STR;
			stmp += default_values[itmp2 - minargs()];
			stmp += RIGHT_BRACKET_STR;
			svargs.push_back(stmp);*/
			svargs.push_back(default_values[itmp - minargs()]);			
			itmp2++;
		}
	}	
	return itmp;
}

Vector *Function::produceVector(vector<Manager*> &vargs, int begin, int end) {	
	if(begin < 0) {
		begin = minargs();
		if(begin < 0) begin = 0;
	}
	if(end < 0 || end >= vargs.size()) {
		end = vargs.size() - 1;
	}
	Vector *v = new Vector();
	for(int index = begin; index <= end; index++) {
		if(vargs[index]->isMatrix()) {
			for(int index_r = 1; index_r <= vargs[index]->matrix()->rows(); index_r++) {
				for(int index_c = 1; index_c <= vargs[index]->matrix()->columns(); index_c++) {
					if(!(index == begin && index_r == 1 && index_c == 1)) v->addComponent();
					v->set(vargs[index]->matrix()->get(index_r, index_c), v->components());
				}			
			}
		} else {
			if(index != begin) v->addComponent();
			v->set(vargs[index], v->components());
		}	
	}
	return v;
}

UserFunction::UserFunction(string cat_, string name_, string eq_, bool is_local, int argc_, string title_, string descr_, int max_argc_, bool is_active) : Function(cat_, name_, argc_, title_, descr_, max_argc_, is_active) {
	b_local = is_local;
	b_builtin = false;
	setEquation(eq_, argc_, max_argc_);
	setChanged(false);	
}
UserFunction::UserFunction(const UserFunction *function) {
	set(function);
}
string UserFunction::equation() const {
	return eq;
}
string UserFunction::internalEquation() const {
	return eq_calc;
}
ExpressionItem *UserFunction::copy() const {
	return new UserFunction(this);
}
void UserFunction::set(const ExpressionItem *item) {
	if(item->type() == TYPE_FUNCTION) {
		if(!item->isBuiltin()) {
			eq = ((UserFunction*) item)->equation();
			eq_calc = ((UserFunction*) item)->internalEquation();			
		}
		Function::set(item);
	} else {
		ExpressionItem::set(item);
	}
}

Manager *UserFunction::calculate(vector<Manager*> &vargs) {
	Function::calculate(vargs);
}
Manager *UserFunction::calculate(const string &eq) {
	Function::calculate(eq);
}
void UserFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	if(args() != 0) {
		string stmp = eq_calc;
		string svar;
		string v_str;
		vector<int> v_id;
		int i2 = 0;
		int i_args = maxargs();
		if(i_args < 0) {
			i_args = minargs();
		}
		vector<Manager*> mngr_v;										
		for(int i = 0; i < i_args; i++) {
			svar = '\\';
			if('x' + i > 'z') {
				svar += (char) ('a' + i - 3);
			} else {
				svar += 'x' + i;
			}
			i2 = 0;	
			mngr_v.push_back(new Manager(vargs[i]));
			v_id.push_back(CALCULATOR->addId(mngr_v[i], true));
			v_str = LEFT_BRACKET ID_WRAP_LEFT;
			v_str += i2s(v_id[v_id.size() - 1]);
			v_str += ID_WRAP_RIGHT RIGHT_BRACKET;			
			while(1) {
				if((i2 = stmp.find(svar, i2)) != (int) string::npos) {
					if(i2 != 0 && stmp[i2 - 1] == '\\') {
						i2 += 2;
					} else {
						stmp.replace(i2, 2, v_str);
					}
				} else {
					break;
				}
			}
		}

		if(maxargs() < 0) {
		
			Vector *v = produceVector(vargs);			
			mngr_v.push_back(new Manager(v));	
			v_id.push_back(CALCULATOR->addId(mngr_v[mngr_v.size() - 1], true));
			v_str = LEFT_BRACKET ID_WRAP_LEFT;
			v_str += i2s(v_id[v_id.size() - 1]);
			v_str += ID_WRAP_RIGHT RIGHT_BRACKET;	
			while(1) {
				if((i2 = stmp.find("\\v")) != (int) string::npos) {					
					if(i2 != 0 && stmp[i2 - 1] == '\\') {
						i2 += 2;
					} else {
						stmp.replace(i2, 2, v_str);
					}
				} else {
					break;
				}
			}
			delete v;
		} 		

		while(1) {
			if((i2 = stmp.find("\\\\")) != (int) string::npos) {
				stmp.replace(i2, 2, "\\");
			} else {
				break;
			}
		}
		Manager *mngr2 = CALCULATOR->calculate(stmp);
		mngr->set(mngr2);
		mngr2->unref();
		for(int i = 0; i < v_id.size(); i++) {
			CALCULATOR->delId(v_id[i], true);
			mngr_v[i]->unref();
		}
		if(!isPrecise()) mngr->setPrecise(false);
	} else {
		Manager *mngr2 = CALCULATOR->calculate(eq_calc);
		mngr->set(mngr2);
		mngr2->unref();
		if(!isPrecise()) mngr->setPrecise(false);
	}
}
void UserFunction::setEquation(string new_eq, int argc_, int max_argc_) {
	setChanged(true);
	eq = new_eq;
	default_values.clear();
	if(argc_ < 0) {
		argc_ = 0, max_argc_ = 0;
		string svar, svar_o, svar_v;
		bool optionals = false;
		int i2 = 0;
		unsigned int i3 = 0, i4 = 0, i5 = 0;
		for(int i = 0; i < 25; i++) {
			begin_loop_in_set_equation:
			i4 = 0; i5 = 0;
			svar = '\\';
			svar_o = '\\';
			if('x' + i > 'z')
				svar += (char) ('a' + i - 3);
			else
				svar += 'x' + i;
			if('X' + i > 'Z')
				svar_o += (char) ('A' + i - 3);
			else
				svar_o += 'X' + i;
				
			before_find_in_set_equation:	
			if(i != 24 && (i2 = new_eq.find(svar_o, i4)) != (int) string::npos) {
				if(i2 > 0 && new_eq[i2 - 1] == '\\') {
					i4 = i2 + 2;
					goto before_find_in_set_equation;
				}				
				i3 = 0;
				if(new_eq.length() > i2 + 2 && new_eq[i2 + 2] == ID_WRAP_LEFT_CH) {
					if((i3 = new_eq.find(ID_WRAP_RIGHT_CH, i2 + 2)) != string::npos) {
						svar_v = new_eq.substr(i2 + 3, i3 - (i2 + 3));	
						i3 -= i2 + 1;
					} else i3 = 0;
				}
				if(i3) {
					default_values.push_back(svar_v);
				} else {
					default_values.push_back("0");
				}
				new_eq.replace(i2, 2 + i3, svar);
				while((i2 = new_eq.find(svar_o, i2 + 1)) != (int) string::npos) {
					new_eq.replace(i2, 2, svar);
				}				
				optionals = true;
			} else if((i2 = new_eq.find(svar, i5)) != (int) string::npos) {
				if(i2 > 0 && new_eq[i2 - 1] == '\\') {
					i5 = i2 + 2;
					goto before_find_in_set_equation;
				}			
			} else {
				if(i < 24 && !optionals) {
					i = 24;
					goto begin_loop_in_set_equation;
				}
				break;
			}
			if(i == 24) {
				max_argc_ = -1;
			} else {
				if(optionals) {
					max_argc_++;
				} else {
					max_argc_++;
					argc_++;
				}			
			}
		}
	}
	if(argc_ > 25) {
		argc_ = 25;
	}
	if(max_argc_ > 25) {
		max_argc_ = 25;
	}
	if(max_argc_ < 0 || argc_ < 0) {
		max_argc_ = -1;
	} else if(max_argc_ < argc_) {
		max_argc_ = argc_;	
	}
	
	while((int) default_values.size() < max_argc_ - argc_) {
		default_values.push_back("0");
	}
	if(max_argc_ > 0) default_values.resize(max_argc_ - argc_);
	eq_calc = new_eq;
	argc = argc_;
	max_argc = max_argc_;	
}
