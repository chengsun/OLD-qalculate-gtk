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

Function::Function(string cat_, string name_, int argc_, string title_, string descr_, bool priviliged_, int max_argc_) {
	b_user = false;
	bpriv = priviliged_;
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	remove_blank_ends(descr_);
	remove_blank_ends(name_);
	sname = name_;
	stitle = title_;
	setDescription(descr_);
	setCategory(cat_);
	argc = argc_;
	if(max_argc_ < argc) max_argc = argc;
	else {
		max_argc = max_argc_;
		for(int i = 0; i < max_argc - argc; i++) {
			default_values.push_back("0");
		}
	}
	sargs.clear();
	b_changed = false;
}
Function::~Function(void) {}
bool Function::priviliged(void) {
	return bpriv;
}
int Function::args(void) {
	return max_argc;
}
int Function::minargs(void) {
	return argc;
}
int Function::maxargs(void) {
	return max_argc;
}
string Function::name(void) {
	return sname;
}
void Function::setName(string new_name, bool force) {
	remove_blank_ends(new_name);
	if(new_name != sname) {
		b_changed = true;
		sname = CALCULATOR->getName(new_name, (void*) this, force);
	}
	CALCULATOR->functionNameChanged(this, bpriv);
}
int Function::args(const string &str) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0, i4 = 0;
	vargs.clear();
	if(!str.empty()) {
		itmp = 1;
		while(1) {
			if((i = str.find(COMMA_CH, i)) != (int) string::npos) {
				if((i3 = str.find(LEFT_BRACKET_CH, i4)) < i && i3 >= 0) {
					i = str.find(RIGHT_BRACKET_CH, i3);
					i4 = i;
					if(i == (int) string::npos)
						break;
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
int Function::args(const string &str, string *buffer) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0;
	if(!str.empty()) {
		itmp = 1;
		while(1) {
			if((i = str.find(COMMA_CH, i)) != (int) string::npos) {
				if((i3 = str.find(LEFT_BRACKET_CH, i2)) < i && i3 >= 0) {
					i = str.find(RIGHT_BRACKET_CH, i3);
					if(i == (int) string::npos)
						break;
				} else {
					if(itmp <= args()) {
						buffer[itmp - 1] = str.substr(i2, i - i2);
					}
					i++;
					i2 = i;
					itmp++;
				}
			} else {
				if(itmp <= args()) {
					buffer[itmp - 1] = str.substr(i2, str.length() - i2);
				}
				break;
			}
		}
	}
	if(itmp < maxargs() && itmp >= minargs()) {
		int itmp2 = itmp;
		while(itmp2 < maxargs()) {
			buffer[itmp2] = default_values[itmp2 - minargs()];
			itmp2++;
		}
	}	
	return itmp;
}
string Function::category(void) {
	return scat;
}
void Function::setCategory(string cat_) {
	remove_blank_ends(cat_);
	scat = cat_;
	b_changed = true;
}
string Function::description(void) {
	return sdescr;
}
void Function::setDescription(string descr_) {
	remove_blank_ends(descr_);
	sdescr = descr_;
	b_changed = true;
}
string Function::title(bool return_name_if_no_title) {
	if(return_name_if_no_title && stitle.empty()) {
		return name();
	}
	return stitle;
}
void Function::setTitle(string title_) {
	remove_blank_ends(title_);
	stitle = title_;
	b_changed = true;
}
string Function::argName(int index) {
	if(index > 0 && index <= sargs.size())
		return sargs[index - 1];
	return "";
}
void Function::clearArgNames(void) {
	sargs.clear();
	b_changed = true;
}
void Function::addArgName(string name_) {
	sargs.push_back(name_);
	b_changed = true;
}
bool Function::setArgName(string name_, int index) {
	if(index > 0 && index <= sargs.size()) {
		sargs[index - 1] = name_;
		b_changed = true;
		return true;
	}
	return false;
}
bool Function::testArgCount(int itmp) {
	if(itmp >= minargs()) {
		if(itmp > maxargs() && maxargs() >= 0)
			CALCULATOR->error(false, _("Additional arguments for function %s() was ignored. Function can only use %s arguments."), name().c_str(), i2s(maxargs()).c_str());						
		return true;	
	}
	CALCULATOR->error(true, _("You need at least %s arguments in function %s()."), i2s(minargs()).c_str(), name().c_str());
	return false;
}
Manager *Function::createFunctionManagerFromVArgs(int itmp) {
	Manager *mngr = new Manager(this, NULL);
	for(int i = 0; i < itmp; i++) {
		mngr->addFunctionArg(vargs[i]);
	}
	return mngr;
}
Manager *Function::createFunctionManagerFromSVArgs(int itmp) {
	Manager *mngr = new Manager(this, NULL); 
	for(int i = 0; i < itmp; i++) {
		Manager *mngr2 = new Manager(svargs[i]);
		mngr->addFunctionArg(mngr2);
		mngr2->unref();
	}
	return mngr;
}
void Function::clearVArgs() {
	for(unsigned int i = 0; i < vargs.size(); i++) {
		vargs[i]->unref();
	}
	vargs.clear();
}
void Function::clearSVArgs() {
	svargs.clear();
}
Manager *Function::calculate(const string &argv) {
	Manager *mngr = NULL;
	int itmp = args(argv);
	if(testArgCount(itmp)) {
		mngr = new Manager();
		calculate2(mngr);
		CALCULATOR->checkFPExceptions(sname.c_str());	
	} else {
		mngr = createFunctionManagerFromVArgs(itmp);
	}
	clearVArgs();
	return mngr;
}
void Function::calculate2(Manager *mngr) {
/*	if(u)
		u->add
		(uargs[0]);*/
	mngr->value(calculate3());	
}
long double Function::calculate3() {
	return 0;
}
bool Function::isBuiltinFunction(void) {
	return true;
}
bool Function::isUserFunction(void) {
	return b_user;
}
bool Function::hasChanged(void) {
	return b_changed;
}
void Function::setUserFunction(bool is_user_function) {
	b_user = is_user_function;
}
void Function::setChanged(bool has_changed) {
	b_changed = has_changed;
}
void Function::setDefaultValue(int arg_, string value_) {
	if(arg_ > argc && arg_ <= max_argc && default_values.size() >= arg_ - argc) {
		default_values[arg_ - argc - 1] = value_;
	}
}
string Function::getDefaultValue(int arg_) {
	if(arg_ > argc && arg_ <= max_argc && default_values.size() >= arg_ - argc) {
		return default_values[arg_ - argc - 1];
	}
	return "";
}

UserFunction::UserFunction(string cat_, string name_, string eq_, bool is_user_function, int argc_, string title_, string descr_, int max_argc_) : Function(cat_, name_, argc_, title_, descr_, false, max_argc_) {
	b_user = is_user_function;
	setEquation(eq_, argc_, max_argc_);
	b_changed = false;	
}
string UserFunction::equation(void) {
	return eq;
}
int Function::stringArgs(const string &str) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0, i4 = 0;
	string stmp;
	svargs.clear();
	if(!str.empty()) {
		itmp = 1;
		while(1) {
			if((i = str.find(COMMA_CH, i)) != (int) string::npos) {
				if((i3 = str.find(LEFT_BRACKET_CH, i4)) < i && i3 >= 0) {
					i = str.find(RIGHT_BRACKET_CH, i3);
					i4 = i;
					if(i == (int) string::npos)
						break;
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
Manager *UserFunction::calculate(const string &argv) {
	if(args() > 0) {
		int itmp;
		if((itmp = stringArgs(argv)) >= minargs()) {
			if(itmp > maxargs())
				CALCULATOR->error(false, _("Additional arguments for function %s() was ignored. Function can only use %s arguments."), name().c_str(), i2s(maxargs()).c_str());						
			string stmp = eq_calc;
			string svar;
			int i2 = 0;
			for(int i = 0; i < args(); i++) {
				svar = '\\';
				if('x' + i > 'z')
					svar += (char) ('a' + i - 3);
				else
					svar += 'x' + i;
				while(1) {
					if((i2 = stmp.find(svar)) != (int) string::npos) {
						svargs[i].insert(0, LEFT_BRACKET_STR);
						svargs[i] += RIGHT_BRACKET_STR;						
						stmp.replace(i2, 2, svargs[i]);
					} else {
						break;
					}
				}
			}
			svargs.clear();
			Manager *mngr = CALCULATOR->calculate(stmp);
			return mngr;
		} else {
			CALCULATOR->error(true, _("You need at least %s arguments in function %s()."), i2s(minargs()).c_str(), name().c_str());		
			Manager *mngr = new Manager(this, NULL);
			for(int i = 0; i < itmp; i++) {
				Manager *mngr2 = CALCULATOR->calculate(svargs[i]);
				mngr->addFunctionArg(mngr2);
				mngr2->unref();
			}
			svargs.clear();
			return mngr;
		}
	} else {
		Manager *mngr = CALCULATOR->calculate(eq_calc);
		return mngr;
	}
}
void UserFunction::setEquation(string new_eq, int argc_, int max_argc_) {
	b_changed = true;
	eq = new_eq;
	default_values.clear();
	if(argc_ < 0) {
		argc_ = 0, max_argc_ = 0;
		string svar, svar_o, svar_v;
		bool optionals = false;
		int i2 = 0;
		unsigned int i3 = 0;
		for(int i = 0; i < 26; i++) {
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
				
			if((i2 = new_eq.find(svar_o)) != (int) string::npos) {				
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
			} else if((i2 = new_eq.find(svar)) == (int) string::npos) {
				break;
			}
			if(optionals) {
				max_argc_++;
			} else {
				max_argc_++;
				argc_++;
			}
		}
	}
	if(argc_ > 26)
		argc_ = 26;
	if(max_argc_ > 26)
		max_argc_ = 26;
	while(default_values.size() < max_argc_ - argc_) {
		default_values.push_back("0");
	}
	default_values.resize(max_argc_ - argc_);

	eq_calc = new_eq;
	argc = argc_;
	max_argc = max_argc_;	
}
bool UserFunction::isBuiltinFunction(void) {
	return false;
}
