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
#include "Integer.h"
#include "Fraction.h"

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
	last_argdef_index = 0;
}
Function::Function(const Function *function) {
	set(function);
}
Function::Function() {
	argc = 0;
	max_argc = 0;
	last_argdef_index = 0;
}
Function::~Function() {
	clearArgumentDefinitions();
}

//ExpressionItem *Function::copy() const {return new Function(this);}
void Function::set(const ExpressionItem *item) {
	if(item->type() == TYPE_FUNCTION) {
		Function *f = (Function*) item;
		argc = f->minargs();
		max_argc = f->maxargs();
		default_values.clear();
		for(int i = argc + 1; i <= max_argc; i++) {
			setDefaultValue(i, f->getDefaultValue(i));
		}
		last_argdef_index = f->lastArgumentDefinitionIndex();
		scondition = f->condition();
		clearArgumentDefinitions();
		for(int i = 1; i <= f->lastArgumentDefinitionIndex(); i++) {
			if(f->getArgumentDefinition(i)) {
				setArgumentDefinition(i, f->getArgumentDefinition(i)->copy());
			}
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
string Function::condition() const {
	return scondition;
}
void Function::setCondition(string expression) {
	scondition = expression;
	remove_blank_ends(scondition);
}
bool Function::testCondition(vector<Manager*> &vargs) {
	if(scondition.empty()) {
		return true;
	}
	UserFunction test_function("", "CONDITION_TEST_FUNCTION", scondition, false, argc, "", "", max_argc);
	Manager *mngr = test_function.calculate(vargs);
	if(!mngr->isFraction() || !mngr->fraction()->isPositive()) {
		string str = scondition;
		string svar, argstr;
		Argument *arg;
		int i_args = maxargs();
		if(i_args < 0) {
			i_args = minargs() + 2;
		}
		for(int i = 0; i < i_args; i++) {
			svar = '\\';
			if(maxargs() < 0 && i >= minargs()) {
				svar += (char) ('v' + i - minargs());
			} else { 
				if('x' + i > 'z') {
					svar += (char) ('a' + i - 3);
				} else {
					svar += 'x' + i;
				}
			}
			int i2 = 0;
			while(true) {
				if((i2 = str.find(svar, i2)) != string::npos) {
					if(maxargs() < 0 && i > minargs()) {
						arg = getArgumentDefinition(i);
					} else {
						arg = getArgumentDefinition(i + 1);
					}
					argstr = "\"";
					if(!arg || arg->name().empty()) {
						argstr += _("argument");
						argstr += " ";
						if(maxargs() < 0 && i > minargs()) {
							argstr += i2s(i);
						} else {
							argstr += i2s(i + 1);
						}
					} else {
						argstr += arg->name();
					}
					argstr += "\"";
					str.replace(i2, 2, argstr);
				} else {
					break;
				}
			}
		}
		CALCULATOR->error(true, "%s() requires that %s", name().c_str(), str.c_str(), NULL);
		return false;
	}
	return true;
}
int Function::args(const string &str, vector<Manager*> &vargs) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0, i4 = 0;
	string str2;
	vargs.clear();
	Manager *mngr;
	if(!str.empty()) {
		itmp = 1;
		while(true) {
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
						str2 = str.substr(i2, i - i2);
						remove_blank_ends(str2);
						if(str2.empty()) {
							mngr = CALCULATOR->calculate(getDefaultValue(itmp));
						} else {
							mngr = CALCULATOR->calculate(str2);
						}
						vargs.push_back(mngr);
					}
					i++;
					i2 = i;
					i4 = i;
					itmp++;
				}
			} else {
				if(itmp <= args() || args() < 0) {
					str2 = str.substr(i2, str.length() - i2);
					remove_blank_ends(str2);
					if(str2.empty()) {
						mngr = CALCULATOR->calculate(getDefaultValue(itmp));
					} else {
						mngr = CALCULATOR->calculate(str2);
					}				
					vargs.push_back(mngr);			
				}
				break;
			}
		}
	}
	if(itmp < maxargs() && itmp >= minargs()) {
		int itmp2 = itmp;
		while(itmp2 < maxargs()) {
			mngr = CALCULATOR->calculate(default_values[itmp2 - minargs()]);
			vargs.push_back(mngr);
			itmp2++;
		}
	}
	return itmp;
}
int Function::lastArgumentDefinitionIndex() const {
	return last_argdef_index;
}
Argument *Function::getArgumentDefinition(int index) {
	if(argdefs.count(index)) {
		return argdefs[index];
	}
	return NULL;
}
void Function::clearArgumentDefinitions() {
	for(hash_map<int, Argument*>::iterator it = argdefs.begin(); it != argdefs.end(); ++it) {
		delete it->second;
	}
	argdefs.clear();
	last_argdef_index = 0;
	setChanged(true);
}
void Function::setArgumentDefinition(int index, Argument *argdef) {
	if(argdefs.count(index)) {
		delete argdefs[index];
	}
	argdefs[index] = argdef;
	if(index > last_argdef_index) {
		last_argdef_index = index;
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
	int itmp = args(argv, vargs);	
	Manager *mngr = calculate(vargs, itmp);
	for(int i = 0; i < vargs.size(); i++) {
		vargs[i]->unref();
	}	
	return mngr;
}
bool Function::testArguments(vector<Manager*> &vargs) {
	for(hash_map<int, Argument*>::iterator it = argdefs.begin(); it != argdefs.end(); ++it) {
		if(it->second && it->first > 0 && it->first <= vargs.size() && !it->second->test(vargs[it->first - 1], it->first, this)) {
			return false;
		}
	}
	return testCondition(vargs);
}
Manager *Function::calculate(vector<Manager*> &vargs, int itmp) {
	Manager *mngr = NULL;
	if(itmp < 0) itmp = vargs.size();
	if(testArgumentCount(itmp) && testArguments(vargs)) {
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
			mngr->typeclean();
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
						if(stmp.empty()) {
							stmp = getDefaultValue(itmp);
						}
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
					if(stmp.empty()) {
						stmp = getDefaultValue(itmp);
					}
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
			svargs.push_back(default_values[itmp2 - minargs()]);			
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
Vector *Function::produceArgumentsVector(vector<Manager*> &vargs, int begin, int end) {	
	if(begin < 0) {
		begin = minargs();
		if(begin < 0) begin = 0;
	}
	if(end < 0 || end >= vargs.size()) {
		end = vargs.size() - 1;
	}
	Vector *v = new Vector();
	for(int index = begin; index <= end; index++) {
		if(index != begin) v->addComponent();
		v->set(vargs[index], v->components());
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
			while(true) {
				if((i2 = stmp.find(svar, i2)) != string::npos) {
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
			Vector *v = NULL, *w = NULL;
			string w_str;
			if(stmp.find("\\v") != string::npos) {
				v = produceVector(vargs);			
				mngr_v.push_back(new Manager(v));	
				v_id.push_back(CALCULATOR->addId(mngr_v[mngr_v.size() - 1], true));
				v_str = LEFT_BRACKET ID_WRAP_LEFT;
				v_str += i2s(v_id[v_id.size() - 1]);
				v_str += ID_WRAP_RIGHT RIGHT_BRACKET;					
			}
			if(stmp.find("\\w") != string::npos) {
				w = produceArgumentsVector(vargs);	
				mngr_v.push_back(new Manager(w));	
				v_id.push_back(CALCULATOR->addId(mngr_v[mngr_v.size() - 1], true));
				w_str = LEFT_BRACKET ID_WRAP_LEFT;
				w_str += i2s(v_id[v_id.size() - 1]);
				w_str += ID_WRAP_RIGHT RIGHT_BRACKET;							
			}			
			while(true) {
				if((i2 = stmp.find("\\v")) != string::npos) {					
					if(i2 != 0 && stmp[i2 - 1] == '\\') {
						i2 += 2;
					} else {
						stmp.replace(i2, 2, v_str);
					}
				} else {
					break;
				}
			}
			while(true) {
				if((i2 = stmp.find("\\w")) != string::npos) {					
					if(i2 != 0 && stmp[i2 - 1] == '\\') {
						i2 += 2;
					} else {
						stmp.replace(i2, 2, w_str);
					}
				} else {
					break;
				}
			}			
			if(v) delete v;
			if(w) delete w;
		} 		

		while(true) {
			if((i2 = stmp.find("\\\\")) != string::npos) {
				stmp.replace(i2, 2, "\\");
			} else {
				break;
			}
		}
		bool was_rpn = CALCULATOR->inRPNMode();
		CALCULATOR->setRPNMode(false);
		Manager *mngr2 = CALCULATOR->calculate(stmp);
		CALCULATOR->setRPNMode(was_rpn);
		mngr->set(mngr2);
		mngr2->unref();
		for(int i = 0; i < v_id.size(); i++) {
			CALCULATOR->delId(v_id[i], true);
			mngr_v[i]->unref();
		}
		if(!isPrecise()) mngr->setPrecise(false);
	} else {
		bool was_rpn = CALCULATOR->inRPNMode();
		CALCULATOR->setRPNMode(false);
		Manager *mngr2 = CALCULATOR->calculate(eq_calc);
		CALCULATOR->setRPNMode(was_rpn);
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
		for(int i = 0; i < 26; i++) {
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
			if(i < 24 && (i2 = new_eq.find(svar_o, i4)) != string::npos) {
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
				while((i2 = new_eq.find(svar_o, i2 + 1)) != string::npos) {
					new_eq.replace(i2, 2, svar);
				}				
				optionals = true;
			} else if((i2 = new_eq.find(svar, i5)) != string::npos) {
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
			if(i >= 24) {
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
	if(argc_ > 24) {
		argc_ = 24;
	}
	if(max_argc_ > 24) {
		max_argc_ = 24;
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

Argument::Argument(string name_, bool does_test) {
	sname = name_;
	remove_blank_ends(sname);
	scondition = "";
	b_zero = true;
	b_test = does_test;
	b_matrix = false;
}
Argument::Argument(const Argument *arg) {set(arg);}
Argument *Argument::copy() const {
	return new Argument(this);
}
string Argument::print() const {return "";}
string Argument::subprintlong() const {return _("a free value");}
string Argument::printlong() const {
	string str = subprintlong();
	if(!b_zero) {
		str += " ";
		str += _("that is nonzero");
	}
	if(!scondition.empty()) {
		if(!b_zero) {
			str += " ";
			str += _("and");
		}
		str += " ";
		str += _("that fulfills the condition:");
		str += " \"";
		str += scondition;
		str += "\"";
	}
	return str;
}
void Argument::set(const Argument *arg) {
	sname = arg->name();
	scondition = arg->getCustomCondition();
	b_zero = !arg->zeroForbidden();
	b_test = arg->tests();
	b_matrix = arg->matrixAllowed();
}
bool Argument::test(const Manager *value, int index, Function *f) const {
	if(!b_test) {
		return true;
	}
	if(!b_zero && value->isZero()) {
		if(sname.empty()) {
			CALCULATOR->error(true, "Argument %s in %s() must be %s.", i2s(index).c_str(), f->name().c_str(), printlong().c_str(), NULL);
		} else {
			CALCULATOR->error(true, "Argument %s, %s, in %s() must be %s.", i2s(index).c_str(), sname.c_str(), f->name().c_str(), printlong().c_str(), NULL);
		}
		return false;
	}
	if(!(b_matrix && value->isMatrix()) && !subtest(value)) {
		if(sname.empty()) {
			CALCULATOR->error(true, "Argument %s in %s() must be %s.", i2s(index).c_str(), f->name().c_str(), printlong().c_str(), NULL);
		} else {
			CALCULATOR->error(true, "Argument %s, %s, in %s() must be %s.", i2s(index).c_str(), sname.c_str(), f->name().c_str(), printlong().c_str(), NULL);
		}
		return false;
	}
	if(!scondition.empty()) {
		string expression = scondition;
		Manager *mngr = (Manager*) value;
		mngr->protect(true);
		int id = CALCULATOR->addId(mngr, true);
		string ids = LEFT_BRACKET;
		ids += ID_WRAP_LEFT;
		ids += i2s(id);
		ids += ID_WRAP_RIGHT;
		ids += RIGHT_BRACKET;
		gsub("\\x", ids, expression);
		bool result = CALCULATOR->testCondition(expression);
		CALCULATOR->delId(id, true);
		mngr->protect(false);
		return result;
	}
	return true;
}
bool Argument::subtest(const Manager *value) const {
	return true;
}
string Argument::name() const {
	return sname;
}
void Argument::setName(string name_) {
	sname = name_;
	remove_blank_ends(sname);
}
void Argument::setCustomCondition(string condition) {
	scondition = condition;
	remove_blank_ends(scondition);
}
string Argument::getCustomCondition() const {
	return scondition;
}
bool Argument::zeroForbidden() const {
	return !b_zero;
}
void Argument::setZeroForbidden(bool forbid_zero) {
	b_zero = !forbid_zero;
}
bool Argument::tests() const {
	return b_test;
}
void Argument::setTests(bool does_test) {
	b_test = does_test;
}
bool Argument::needQuotes() const {return false;}
int Argument::type() const {
	return ARGUMENT_TYPE_FREE;
}
bool Argument::matrixAllowed() const {return b_matrix;}
void Argument::setMatrixAllowed(bool allow_matrix) {b_matrix = allow_matrix;}

FractionArgument::FractionArgument(string name_, ArgumentMinMaxPreDefinition minmax, bool does_test) : Argument(name_, does_test) {
	fmin = NULL;
	fmax = NULL;
	b_incl_min = true;
	b_incl_max = true;
	switch(minmax) {
		case ARGUMENT_MIN_MAX_POSITIVE: {
			fmin = new Fraction();
			b_incl_min = false;
			break;
		}
		case ARGUMENT_MIN_MAX_NEGATIVE: {
			fmax = new Fraction();
			b_incl_max = false;
			break;
		}
		case ARGUMENT_MIN_MAX_NONNEGATIVE: {
			fmin = new Fraction();
			break;
		}
		case ARGUMENT_MIN_MAX_NONZERO: {
			setZeroForbidden(true);
			break;
		}		
	}
}
FractionArgument::FractionArgument(const FractionArgument *arg) {
	fmin = NULL;
	fmax = NULL;
	set(arg);
}
FractionArgument::~FractionArgument() {
	if(fmin) {
		delete fmin;
	}
	if(fmax) {
		delete fmax;
	}
}
	
void FractionArgument::setMin(const Fraction *min_) {
	if(!min_) {
		if(fmin) {
			delete fmin;
		}
		return;
	}
	if(!fmin) {
		fmin = new Fraction(min_);
	} else {
		fmin->set(min_);
	}
}
void FractionArgument::setIncludeEqualsMin(bool include_equals) {
	b_incl_min = include_equals;
}
bool FractionArgument::includeEqualsMin() const {
	return b_incl_min;
}
const Fraction *FractionArgument::min() const {
	return fmin;
}
void FractionArgument::setMax(const Fraction *max_) {
	if(!max_) {
		if(fmax) {
			delete fmax;
		}
		return;
	}
	if(!fmax) {
		fmax = new Fraction(max_);
	} else {
		fmax->set(max_);
	}
}
void FractionArgument::setIncludeEqualsMax(bool include_equals) {
	b_incl_max = include_equals;
}
bool FractionArgument::includeEqualsMax() const {
	return b_incl_max;
}
const Fraction *FractionArgument::max() const {
	return fmax;
}
bool FractionArgument::subtest(const Manager *value) const {
	if(!value->isFraction()) {
		return false;
	}
	if(fmin && (b_incl_min && value->fraction()->compare(fmin) > 0) || (!b_incl_min && value->fraction()->compare(fmin) >= 0)) {
		return false;
	}
	if(fmax && (b_incl_max && value->fraction()->compare(fmax) < 0) || (!b_incl_max && value->fraction()->compare(fmax) <= 0)) {
		return false;
	}	
	return true;
}
int FractionArgument::type() const {
	return ARGUMENT_TYPE_FRACTION;
}
Argument *FractionArgument::copy() const {
	return new FractionArgument(this);
}
void FractionArgument::set(const Argument *arg) {
	if(arg->type() == ARGUMENT_TYPE_FRACTION) {
		const FractionArgument *farg = (const FractionArgument*) arg;
		b_incl_min = farg->includeEqualsMin();
		b_incl_max = farg->includeEqualsMax();
		if(fmin) {
			delete fmin;
			fmin = NULL;
		}
		if(fmax) {
			delete fmax;
			fmax = NULL;
		}
		if(farg->min()) {
			fmin = new Fraction(farg->min());
		}
		if(farg->max()) {
			fmax = new Fraction(farg->max());
		}		
	}
	Argument::set(arg);
}
string FractionArgument::print() const {
	return _("number");
}
string FractionArgument::subprintlong() const {
	string str = _("a number");
	if(fmin) {
		str += " ";
		if(b_incl_min) {
			str += _(">=");
		} else {
			str += _(">");
		}
		str += " ";
		str += fmin->print();
	}
	if(fmax) {
		if(fmin) {
			str += " ";
			str += _("and");
		}
		str += " ";
		if(b_incl_max) {
			str += _("<=");
		} else {
			str += _("<");
		}
		str += " ";
		str += fmax->print();
	}
	return str;
}

IntegerArgument::IntegerArgument(string name_, ArgumentMinMaxPreDefinition minmax, bool does_test) : Argument(name_, does_test) {
	imin = NULL;
	imax = NULL;
	switch(minmax) {
		case ARGUMENT_MIN_MAX_POSITIVE: {
			imin = new Integer(1);
			break;
		}
		case ARGUMENT_MIN_MAX_NEGATIVE: {
			imax = new Integer(-1);
			break;
		}
		case ARGUMENT_MIN_MAX_NONNEGATIVE: {
			imin = new Integer();
			break;
		}
		case ARGUMENT_MIN_MAX_NONZERO: {
			setZeroForbidden(true);
			break;
		}		
	}	
}
IntegerArgument::IntegerArgument(const IntegerArgument *arg) {
	imin = NULL;
	imax = NULL;
	set(arg);
}
IntegerArgument::~IntegerArgument() {
	if(imin) {
		delete imin;
	}
	if(imax) {
		delete imax;
	}
}
	
void IntegerArgument::setMin(const Integer *min_) {
	if(!min_) {
		if(imin) {
			delete imin;
		}
		return;
	}
	if(!imin) {
		imin = new Integer(min_);
	} else {
		imin->set(min_);
	}
}
const Integer *IntegerArgument::min() const {
	return imin;
}
void IntegerArgument::setMax(const Integer *max_) {
	if(!max_) {
		if(imax) {
			delete imax;
		}
		return;
	}
	if(!imax) {
		imax = new Integer(max_);
	} else {
		imax->set(max_);
	}
}
const Integer *IntegerArgument::max() const {
	return imax;
}
bool IntegerArgument::subtest(const Manager *value) const {
	if(!value->isFraction() || !value->fraction()->isInteger()) {
		return false;
	}
	if(imin && value->fraction()->numerator()->compare(imin) > 0) {
		return false;
	}
	if(imax && value->fraction()->numerator()->compare(imax) < 0) {
		return false;
	}	
	return true;
}
int IntegerArgument::type() const {
	return ARGUMENT_TYPE_INTEGER;
}
Argument *IntegerArgument::copy() const {
	return new IntegerArgument(this);
}
void IntegerArgument::set(const Argument *arg) {
	if(arg->type() == ARGUMENT_TYPE_INTEGER) {
		const IntegerArgument *iarg = (const IntegerArgument*) arg;
		if(imin) {
			delete imin;
			imin = NULL;
		}
		if(imax) {
			delete imax;
			imax = NULL;
		}
		if(iarg->min()) {
			imin = new Integer(iarg->min());
		}
		if(iarg->max()) {
			imax = new Integer(iarg->max());
		}		
	}
	Argument::set(arg);
}
string IntegerArgument::print() const {
	return _("integer");
}
string IntegerArgument::subprintlong() const {
	string str = _("an integer");
	if(imin) {
		str += " ";
		str += _(">=");
		str += " ";
		str += imin->print();
	}
	if(imax) {
		if(imin) {
			str += " ";
			str += _("and");
		}
		str += " ";
		str += _("<=");
		str += " ";
		str += imax->print();
	}
	return str;
}

TextArgument::TextArgument(string name_, bool does_test) : Argument(name_, does_test) {}
TextArgument::TextArgument(const TextArgument *arg) {set(arg);}
bool TextArgument::subtest(const Manager *value) const {return value->isText();}
int TextArgument::type() const {return ARGUMENT_TYPE_TEXT;}
Argument *TextArgument::copy() const {return new TextArgument(this);}
string TextArgument::print() const {return _("text");}
string TextArgument::subprintlong() const {return _("a text string");}
bool TextArgument::needQuotes() const {return true;}

DateArgument::DateArgument(string name_, bool does_test) : Argument(name_, does_test) {}
DateArgument::DateArgument(const DateArgument *arg) {set(arg);}
bool DateArgument::subtest(const Manager *value) const {
	int day = 0, year = 0, month = 0;
	return value->isText() && s2date(value->text(), day, year, month);
}
int DateArgument::type() const {return ARGUMENT_TYPE_DATE;}
Argument *DateArgument::copy() const {return new DateArgument(this);}
string DateArgument::print() const {return _("date");}
string DateArgument::subprintlong() const {return _("a quoted date");}
bool DateArgument::needQuotes() const {return true;}

VectorArgument::VectorArgument(string name_, bool does_test, bool allow_matrix) : Argument(name_, does_test) {
	setMatrixAllowed(allow_matrix);
}
VectorArgument::VectorArgument(const VectorArgument *arg) {set(arg);}
bool VectorArgument::subtest(const Manager *value) const {return value->isMatrix() && value->matrix()->isVector();}
int VectorArgument::type() const {return ARGUMENT_TYPE_VECTOR;}
Argument *VectorArgument::copy() const {return new VectorArgument(this);}
string VectorArgument::print() const {return _("vector");}
string VectorArgument::subprintlong() const {return _("a vector");}

MatrixArgument::MatrixArgument(string name_, bool does_test) : Argument(name_, does_test) {}
MatrixArgument::MatrixArgument(const MatrixArgument *arg) {set(arg);}
bool MatrixArgument::subtest(const Manager *value) const {return value->isMatrix();}
int MatrixArgument::type() const {return ARGUMENT_TYPE_TEXT;}
Argument *MatrixArgument::copy() const {return new MatrixArgument(this);}
string MatrixArgument::print() const {return _("matrix");}
string MatrixArgument::subprintlong() const {return _("a matrix");}

ExpressionItemArgument::ExpressionItemArgument(string name_, bool does_test) : Argument(name_, does_test) {}
ExpressionItemArgument::ExpressionItemArgument(const ExpressionItemArgument *arg) {set(arg);}
bool ExpressionItemArgument::subtest(const Manager *value) const {return value->isText() && CALCULATOR->getExpressionItem(value->text());}
int ExpressionItemArgument::type() const {return ARGUMENT_TYPE_EXPRESSION_ITEM;}
Argument *ExpressionItemArgument::copy() const {return new ExpressionItemArgument(this);}
string ExpressionItemArgument::print() const {return _("object");}
string ExpressionItemArgument::subprintlong() const {return _("a quoted valid function, unit or variable");}
bool ExpressionItemArgument::needQuotes() const {return true;}

FunctionArgument::FunctionArgument(string name_, bool does_test) : Argument(name_, does_test) {}
FunctionArgument::FunctionArgument(const FunctionArgument *arg) {set(arg);}
bool FunctionArgument::subtest(const Manager *value) const {return value->isText() && CALCULATOR->getFunction(value->text());}
int FunctionArgument::type() const {return ARGUMENT_TYPE_FUNCTION;}
Argument *FunctionArgument::copy() const {return new FunctionArgument(this);}
string FunctionArgument::print() const {return _("function");}
string FunctionArgument::subprintlong() const {return _("a quoted valid function");}
bool FunctionArgument::needQuotes() const {return true;}

UnitArgument::UnitArgument(string name_, bool does_test) : Argument(name_, does_test) {}
UnitArgument::UnitArgument(const UnitArgument *arg) {set(arg);}
bool UnitArgument::subtest(const Manager *value) const {return value->isText() && CALCULATOR->getUnit(value->text());}
int UnitArgument::type() const {return ARGUMENT_TYPE_UNIT;}
Argument *UnitArgument::copy() const {return new UnitArgument(this);}
string UnitArgument::print() const {return _("unit");}
string UnitArgument::subprintlong() const {return _("a quoted valid unit");}
bool UnitArgument::needQuotes() const {return true;}

VariableArgument::VariableArgument(string name_, bool does_test) : Argument(name_, does_test) {}
VariableArgument::VariableArgument(const VariableArgument *arg) {set(arg);}
bool VariableArgument::subtest(const Manager *value) const {return value->isText() && CALCULATOR->getVariable(value->text());}
int VariableArgument::type() const {return ARGUMENT_TYPE_VARIABLE;}
Argument *VariableArgument::copy() const {return new VariableArgument(this);}
string VariableArgument::print() const {return _("variable");}
string VariableArgument::subprintlong() const {return _("a quoted valid variable");}
bool VariableArgument::needQuotes() const {return true;}

BooleanArgument::BooleanArgument(string name_, bool does_test) : Argument(name_, does_test) {}
BooleanArgument::BooleanArgument(const BooleanArgument *arg) {set(arg);}
bool BooleanArgument::subtest(const Manager *value) const {return value->isZero() || value->isOne();}
int BooleanArgument::type() const {return ARGUMENT_TYPE_BOOLEAN;}
Argument *BooleanArgument::copy() const {return new BooleanArgument(this);}
string BooleanArgument::print() const {return _("boolean");}
string BooleanArgument::subprintlong() const {return _("a boolean (0 or 1)");}

