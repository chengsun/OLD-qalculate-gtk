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
	b_exact = true;
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
	sargs.clear();
	b_changed = false;
}
Function::~Function(void) {}
bool Function::priviliged(void) const {
	return bpriv;
}
int Function::args(void) const {
	return max_argc;
}
int Function::minargs(void) const {
	return argc;
}
int Function::maxargs(void) const {
	return max_argc;
}
string Function::name(void) const {
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
string Function::category(void) const {
	return scat;
}
void Function::setCategory(string cat_) {
	remove_blank_ends(cat_);
	scat = cat_;
	b_changed = true;
}
string Function::description(void) const {
	return sdescr;
}
void Function::setDescription(string descr_) {
	remove_blank_ends(descr_);
	sdescr = descr_;
	b_changed = true;
}
string Function::title(bool return_name_if_no_title) const {
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
string Function::argName(int index) const {
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
				calculate2(mngr2);
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
			calculate2(mngr);
		}
		if(!isPrecise()) mngr->setPrecise(false);
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
	mngr->set(calculate3());	
}
long double Function::calculate3() {
	return 0;
}
bool Function::isBuiltinFunction(void) const {
	return true;
}
bool Function::isUserFunction(void) const {
	return b_user;
}
bool Function::hasChanged(void) const {
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
string Function::getDefaultValue(int arg_) const {
	if(arg_ > argc && arg_ <= max_argc && default_values.size() >= arg_ - argc) {
		return default_values[arg_ - argc - 1];
	}
	return "";
}
bool Function::isPrecise() const {
	return b_exact;
}
void Function::setPrecise(bool is_precise) {
	b_exact = is_precise;
}
Vector *Function::produceVector(int begin, int end) {	
	if(begin < 0) {
		begin = minargs();
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

UserFunction::UserFunction(string cat_, string name_, string eq_, bool is_user_function, int argc_, string title_, string descr_, int max_argc_) : Function(cat_, name_, argc_, title_, descr_, false, max_argc_) {
	b_user = is_user_function;
	setEquation(eq_, argc_, max_argc_);
	b_changed = false;	
}
string UserFunction::equation(void) const {
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
Manager *UserFunction::calculate(const string &argv) {
	if(args() != 0) {
		int itmp;
		if((itmp = stringArgs(argv)) >= minargs()) {
			if(itmp > maxargs() && maxargs() >= 0)
				CALCULATOR->error(false, _("Additional arguments for function %s() was ignored. Function can only use %s arguments."), name().c_str(), i2s(maxargs()).c_str());						
			string stmp = eq_calc;
			string svar;
			int i2 = 0;
			int i_args = maxargs();
			if(i_args < 0) {
				i_args = minargs();
			}
			for(int i = 0; i < i_args; i++) {
				svar = '\\';
				if('x' + i > 'z')
					svar += (char) ('a' + i - 3);
				else
					svar += 'x' + i;
				while(1) {
					if((i2 = stmp.find(svar)) != (int) string::npos) {
						if(i2 != 0 && stmp[i2 - 1] == '\\') {
							i2 += 2;
						} else {
							svargs[i].insert(0, LEFT_BRACKET_STR);
							svargs[i] += RIGHT_BRACKET_STR;						
							stmp.replace(i2, 2, svargs[i]);
						}
					} else {
						break;
					}
				}
			}
			clearSVArgs();
			vector<Manager*> mngr_v;
			vector<int> v_id;
			if(maxargs() < 0) {
				args(argv);				
				Vector *v = produceVector();			
				clearVArgs();
				string v_str;
				while(1) {
					if((i2 = stmp.find("\\v")) != (int) string::npos) {					
						if(i2 != 0 && stmp[i2 - 1] == '\\') {
							i2 += 2;
						} else {
							mngr_v.push_back(new Manager(v));	
							v_id.push_back(CALCULATOR->addId(mngr_v[mngr_v.size() - 1], true));
							v_str = LEFT_BRACKET ID_WRAP_LEFT;
							v_str += i2s(v_id[v_id.size() - 1]);
							v_str += ID_WRAP_RIGHT RIGHT_BRACKET;	
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
			Manager *mngr = CALCULATOR->calculate(stmp);
			for(int i = 0; i < mngr_v.size(); i++) {
				CALCULATOR->delId(v_id[i], true);
				mngr_v[i]->unref();
			}
			if(!isPrecise()) mngr->setPrecise(false);
			return mngr;
		} else {
			CALCULATOR->error(true, _("You need at least %s arguments in function %s()."), i2s(minargs()).c_str(), name().c_str());		
			Manager *mngr = new Manager(this, NULL);
			for(int i = 0; i < itmp; i++) {
				Manager *mngr2 = CALCULATOR->calculate(svargs[i]);
				mngr->addFunctionArg(mngr2);
				mngr2->unref();
			}
			clearSVArgs();
			return mngr;
		}
	} else {
		Manager *mngr = CALCULATOR->calculate(eq_calc);
		if(!isPrecise()) mngr->setPrecise(false);
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
bool UserFunction::isBuiltinFunction(void) const {
	return false;
}
