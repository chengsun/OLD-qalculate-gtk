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

Function::Function(Calculator *calc_, string cat_, string name_, int argc_, string title_, string descr_, bool priviliged_) {
	calc = calc_;
	bpriv = priviliged_;
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	remove_blank_ends(descr_);
	remove_blank_ends(name_);
	sname = name_;
	stitle = title_;
	description(descr_);
	category(cat_);
	argc = argc_;
	sargs.clear();
}
Function::~Function(void) {}
bool Function::priviliged(void) {
	return bpriv;
}
int Function::args(void) {
	return argc;
}
string Function::name(void) {
	return sname;
}
void Function::name(string new_name, bool force) {
	remove_blank_ends(new_name);
	if(new_name != sname) {
		sname = calc->getName(new_name, (void*) this, force);
	}
	calc->functionNameChanged(this, bpriv);
}
int Function::args(const string &str) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0, i4 = 0;
	vargs.clear();
	printf("ARGS1 %s\n", str.c_str());
	if(!str.empty()) {
		itmp = 1;
		while(1) {
			if((i = str.find_first_of(COMMA_S, i)) != (int) string::npos) {
				if((i3 = str.find_first_of(LEFT_BRACKET_S, i4)) < i && i3 >= 0) {
					printf("ARGS(\n");
					i = str.find_first_of(RIGHT_BRACKET_S, i3);
					i4 = i;
					if(i == (int) string::npos)
						break;
				} else {
					printf("ARGS2 %s\n", str.substr(i2, i - i2).c_str());
					if(itmp <= args() || args() < 0) {
						Manager *mngr = calc->calculate(str.substr(i2, i - i2));
						vargs.push_back(mngr);
					}
					i++;
					i2 = i;
					i4 = i;
					itmp++;
				}
			} else {
				printf("ARGS3 %s\n", str.substr(i2, str.length() - i2).c_str());
				if(itmp <= args() || args() < 0) {
					Manager *mngr = calc->calculate(str.substr(i2, str.length() - i2));
					vargs.push_back(mngr);			
				}
				break;
			}
		}
	}
	return itmp;
}
int Function::args(const string &str, string *buffer) {
	int itmp = 0, i = 0, i2 = 0, i3 = 0;
	if(!str.empty()) {
		itmp = 1;
		while(1) {
			if((i = str.find_first_of(COMMA_S, i)) != (int) string::npos) {
				if((i3 = str.find_first_of(LEFT_BRACKET_S, i2)) < i && i3 >= 0) {
					i = str.find_first_of(RIGHT_BRACKET_S, i3);
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
	return itmp;
}
string Function::category(void) {
	return scat;
}
void Function::category(string cat_) {
	remove_blank_ends(cat_);
	scat = cat_;
}
string Function::description(void) {
	return sdescr;
}
void Function::description(string descr_) {
	remove_blank_ends(descr_);
	sdescr = descr_;
}
string Function::title(void) {
	return stitle;
}
void Function::title(string title_) {
	remove_blank_ends(title_);
	stitle = title_;
}
string Function::argName(int index) {
	if(index > 0 && index <= sargs.size())
		return sargs[index - 1];
	return "";
}
void Function::clearArgNames(void) {
	sargs.clear();
}
void Function::addArgName(string name_) {
	sargs.push_back(name_);
}
bool Function::setArgName(string name_, int index) {
	if(index > 0 && index <= sargs.size()) {
		sargs[index - 1] = name_;
		return true;
	}
	return false;
}
Manager *Function::calculate(const string &argv) {
	Manager *mngr = NULL;
	int itmp = args(argv);
	if(itmp >= args()) {
		if(itmp > args() && args() >= 0)
			calc->error(false, 3, "To many arguments for ", name().c_str(), "() (ignored)");
		mngr = new Manager(calc);
		calculate2(mngr);
		calc->checkFPExceptions(sname.c_str());
	} else {
		calc->error(true, 4, "You need ", i2s(args()).c_str(), " arguments in function ", name().c_str());
	}
	for(unsigned int i = 0; i < vargs.size(); i++) {
		vargs[i]->unref();
	}
	vargs.clear();
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
bool Function::isUserFunction(void) {
	return false;
}

UserFunction::UserFunction(Calculator *calc_, string cat_, string name_, string eq_, int argc_, string title_, string descr_) : Function(calc_, cat_, name_, argc_, title_, descr_) {
	equation(eq_, argc_);
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
			if((i = str.find_first_of(COMMA_S, i)) != (int) string::npos) {
				if((i3 = str.find_first_of(LEFT_BRACKET_S, i4)) < i && i3 >= 0) {
					i = str.find_first_of(RIGHT_BRACKET_S, i3);
					i4 = i;
					if(i == (int) string::npos)
						break;
				} else {
					if(itmp <= args() || args() < 0) {
						stmp = LEFT_BRACKET_CH;
						stmp += str.substr(i2, i - i2);
						stmp += RIGHT_BRACKET_CH;
						svargs.push_back(stmp);
					}
					i++;
					i2 = i;
					i4 = i;
					itmp++;
				}
			} else {
				if(itmp <= args() || args() < 0) {
					stmp = LEFT_BRACKET_CH;
					stmp += str.substr(i2, str.length() - i2);
					stmp += RIGHT_BRACKET_CH;
					svargs.push_back(stmp);
				}
				break;
			}
		}
	}
	return itmp;
}
Manager *UserFunction::calculate(const string &argv) {
	if(args() > 0) {
		int itmp;
		if((itmp = stringArgs(argv)) >= args()) {
			if(itmp > args())
				calc->error(false, 3, "To many arguments for ", name().c_str(), "() (ignored)");
			string stmp = equation();
			string svar;
			int i2 = 0;
			printf("UserFunction 1: %s\n", stmp.c_str());
			for(int i = 0; i < args(); i++) {
				svar = '\\';
				if('x' + i > 'z')
					svar += (char) ('a' + i - 3);
				else
					svar += 'x' + i;
				while(1) {
					if((i2 = stmp.find(svar)) != (int) string::npos) {
						printf("UserFunction 2: %s\n", svargs[i].c_str());
						stmp.replace(i2, 2, svargs[i]);
						printf("UserFunction 3: %s\n", stmp.c_str());
					} else {
						break;
					}
				}
			}
			svargs.clear();
			printf("UserFunction 4: %s\n", stmp.c_str());
			Manager *mngr = calc->calculate(stmp);
			return mngr;
		} else {
			calc->error(true, 4, "You need ", i2s(args()).c_str(), " arguments in function ", name().c_str());
			svargs.clear();
			return NULL;
		}
	} else {
		Manager *mngr = calc->calculate(equation());
		return mngr;
	}
}
void UserFunction::equation(string new_eq, int argc_) {
	if(argc_ < 0) {
		argc_ = 0;
		string svar;
		int i2 = 0;
		for(int i = 0; i < 26; i++) {
			svar = '\\';
			if('x' + i > 'z')
				svar += (char) ('a' + i - 3);
			else
				svar += 'x' + i;
			if((i2 = new_eq.find(svar)) == (int) string::npos) {
				break;
			}
			argc_++;
		}
	}
	if(argc_ > 26)
		argc_ = 26;

	eq = new_eq;
	argc = argc_;
}
bool UserFunction::isUserFunction(void) {
	return true;
}
