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
#include "MathStructure.h"
#include "Variable.h"
#include "Number.h"
#include "Unit.h"

#ifdef HAVE_GIAC
#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

/*	gen qalculate_function(const gen &a, const gen &b){
		if(is_integer(a) && is_integer(b)) return (a + b) / (a * b);
		return symbolic(at_qalculate_function, makevecteur(a, b));
	}*/

	gen _qalculate_function(const gen &args){
		return symbolic(at_qalculate_function, args);
		/*if((args.type != _VECT) || (args._VECTptr->size() < 1)) setsizeerr();
		vecteur &v = *args._VECTptr;
		if(v[0].type != _POINTER_) {
			return symbolic(at_qalculate_function, v);
		}
		Function *f = (Function*) v[0]._POINTER_val;
		MathStructure vargs;
		vargs.clearVector();
		for(unsigned int i = 1; i < v.size(); i++) {
			vargs.addComponent(v[i]);
		}
		MathStructure mstruct = f->calculate(vargs);
		return mstruct.toGiac();*/
	}
	const string _qalculate_function_s("qalculate_function");
	unary_function_unary __qalculate_function(&_qalculate_function, _qalculate_function_s);
	unary_function_ptr at_qalculate_function (&__qalculate_function, 0, true);
     
#ifndef NO_NAMESPACE_GIAC
}
#endif // ndef NO_NAMESPACE_GIAC
#endif

Function::Function(string name_, int argc_, int max_argc_, string cat_, string title_, string descr_, bool is_active) : ExpressionItem(cat_, name_, title_, descr_, false, true, is_active) {
	argc = argc_;
	if(max_argc_ < 0 || argc < 0) {
		if(argc < 0) argc = 0;
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
		for(unsigned int i = 1; i <= f->lastArgumentDefinitionIndex(); i++) {
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

#ifdef HAVE_GIAC
giac::gen Function::toGiac(const MathStructure &vargs) const {
	giac::vecteur v;
	v.push_back(giac::identificateur(p2s((void*) this)));
	for(unsigned int i = 0; i < vargs.size(); i++) {
		v.push_back(vargs[i].toGiac());
	}
	return giac::symbolic(giac::at_qalculate_function, v);
}
giac::gen Function::argsToGiac(const MathStructure &vargs) const {
	giac::vecteur v;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		v.push_back(vargs[i].toGiac());
	}
	return v;
}
bool Function::isGiacFunction() const {return false;}
#endif

int Function::countArgOccurence(unsigned int arg_) {
	if((int) arg_ > argc && max_argc < 0) {
		arg_ = argc + 1;
	}
	if(argoccs.count(arg_) > 0) {
		return argoccs[arg_];
	}
	return 1;
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
bool Function::testCondition(const MathStructure &vargs) {
	if(scondition.empty()) {
		return true;
	}
	UserFunction test_function("", "CONDITION_TEST_FUNCTION", scondition, false, argc, "", "", max_argc);
	MathStructure vargs2(vargs);
	MathStructure mstruct(test_function.Function::calculate(vargs2));
	EvaluationOptions eo;
	eo.approximation = APPROXIMATION_APPROXIMATE;
	mstruct.eval(eo);
	if(!mstruct.isNumber() || !mstruct.number().isPositive()) {
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
			unsigned int i2 = 0;
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
		if(CALCULATOR->showArgumentErrors()) {
			CALCULATOR->error(true, _("%s() requires that %s"), name().c_str(), str.c_str(), NULL);
		}
		return false;
	}
	return true;
}
int Function::args(const string &argstr, MathStructure &vargs) {
	vargs.clearVector();
	int start_pos = 0;
	bool in_cit1 = false, in_cit2 = false;
	int pars = 0;
	int itmp = 0;
	string str = argstr, stmp;
	remove_blank_ends(str);
	Argument *arg;
	bool last_is_vctr = false, vctr_started = false;
	if(maxargs() > 0) {
		arg = getArgumentDefinition(maxargs());
		last_is_vctr = arg && arg->type() == ARGUMENT_TYPE_VECTOR;
	}
	for(unsigned int str_index = 0; str_index < str.length(); str_index++) {
		switch(str[str_index]) {
			case LEFT_VECTOR_WRAP_CH: {}
			case LEFT_PARENTHESIS_CH: {
				if(!in_cit1 && !in_cit2) {
					pars++;
				}
				break;
			}
			case RIGHT_VECTOR_WRAP_CH: {}
			case RIGHT_PARENTHESIS_CH: {
				if(!in_cit1 && !in_cit2 && pars > 0) {
					pars--;
				}
				break;
			}
			case '\"': {
				if(in_cit1) {
					in_cit1 = false;
				} else if(!in_cit2) {
					in_cit1 = true;
				}
				break;
			}
			case '\'': {
				if(in_cit2) {
					in_cit2 = false;
				} else if(!in_cit1) {
					in_cit1 = true;
				}
				break;
			}
			case COMMA_CH: {
				if(pars == 0 && !in_cit1 && !in_cit2) {
					itmp++;
					if(itmp <= maxargs() || args() < 0) {
						stmp = str.substr(start_pos, str_index - start_pos);
						remove_blank_ends(stmp);
						arg = getArgumentDefinition(itmp);
						if(stmp.empty()) {
							if(arg) {
								vargs.addItem(arg->parse(getDefaultValue(itmp)));
							} else {
								vargs.addItem(CALCULATOR->parse(getDefaultValue(itmp)));
							}
						} else {
							if(arg) {
								vargs.addItem(arg->parse(stmp));
							} else {
								vargs.addItem(CALCULATOR->parse(stmp));
							}
						}
					} else if(last_is_vctr) {
						if(!vctr_started) {
							vargs[vargs.size() - 1].transform(STRUCT_VECTOR);
							vctr_started = true;
						}
						stmp = str.substr(start_pos, str_index - start_pos);
						remove_blank_ends(stmp);
						if(stmp.empty()) {
							vargs[vargs.size() - 1].addItem(getArgumentDefinition(maxargs())->parse(getDefaultValue(itmp)));
						} else {
							vargs[vargs.size() - 1].addItem(getArgumentDefinition(maxargs())->parse(stmp));
						}
					}
					start_pos = str_index + 1;
				}
				break;
			}
		}
	}
	if(!str.empty()) {
		itmp++;
		if(itmp <= maxargs() || args() < 0) {
			stmp = str.substr(start_pos, str.length() - start_pos);
			remove_blank_ends(stmp);
			arg = getArgumentDefinition(itmp);
			if(stmp.empty()) {
				if(arg) {
					vargs.addItem(arg->parse(getDefaultValue(itmp)));
				} else {
					vargs.addItem(CALCULATOR->parse(getDefaultValue(itmp)));
				}
			} else {
				if(arg) {
					vargs.addItem(arg->parse(stmp));
				} else {
					vargs.addItem(CALCULATOR->parse(stmp));
				}
			}
		} else if(last_is_vctr) {
			if(!vctr_started) {
				vargs[vargs.size() - 1].transform(STRUCT_VECTOR);
				vctr_started = true;
			}
			stmp = str.substr(start_pos, str.length() - start_pos);
			remove_blank_ends(stmp);
			if(stmp.empty()) {
				vargs[vargs.size() - 1].addItem(getArgumentDefinition(maxargs())->parse(getDefaultValue(itmp)));
			} else {
				vargs[vargs.size() - 1].addItem(getArgumentDefinition(maxargs())->parse(stmp));
			}
		}	
	}
	if(itmp < maxargs() && itmp >= minargs()) {
		int itmp2 = itmp;
		while(itmp2 < maxargs()) {
			vargs.addItem(CALCULATOR->parse(default_values[itmp2 - minargs()]));
			itmp2++;
		}
	}
	return itmp;
}
unsigned int Function::lastArgumentDefinitionIndex() const {
	return last_argdef_index;
}
Argument *Function::getArgumentDefinition(unsigned int index) {
	if(argdefs.count(index)) {
		return argdefs[index];
	}
	return NULL;
}
void Function::clearArgumentDefinitions() {
	for(Sgi::hash_map<unsigned int, Argument*>::iterator it = argdefs.begin(); it != argdefs.end(); ++it) {
		delete it->second;
	}
	argdefs.clear();
	last_argdef_index = 0;
	setChanged(true);
}
void Function::setArgumentDefinition(unsigned int index, Argument *argdef) {
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
		if(itmp > maxargs() && maxargs() >= 0) {
			CALCULATOR->error(false, _("Additional arguments for function %s() was ignored. Function can only use %s argument(s)."), name().c_str(), i2s(maxargs()).c_str());
		}
		return true;	
	}
	string str;
	Argument *arg;
	bool b = false;
	for(int i = 1; i <= minargs(); i++) {
		arg = getArgumentDefinition(i);
		if(i > 1) {
			str += CALCULATOR->getComma();
			str += " ";
		}
		if(arg && !arg->name().empty()) {
			str += arg->name();
			b = true;
		} else {
			str += "?";
		}
	}
	if(b) {
		CALCULATOR->error(true, _("You need at least %s argument(s) (%s) in function %s()."), i2s(minargs()).c_str(), str.c_str(), name().c_str());
	} else {
		CALCULATOR->error(true, _("You need at least %s argument(s) in function %s()."), i2s(minargs()).c_str(), name().c_str());
	}
	return false;
}
MathStructure Function::createFunctionMathStructureFromVArgs(const MathStructure &vargs) {
	MathStructure mstruct(this, NULL);
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mstruct.addChild(vargs[i]);
	}
	return mstruct;
}
MathStructure Function::createFunctionMathStructureFromSVArgs(vector<string> &svargs) {
	MathStructure mstruct(this, NULL); 
	for(unsigned int i = 0; i < svargs.size(); i++) {
		mstruct.addChild(svargs[i]);
	}
	return mstruct;
}
MathStructure Function::calculate(const string &argv, const EvaluationOptions &eo) {
	MathStructure vargs;
	args(argv, vargs);	
	return calculate(vargs, eo);
}
MathStructure Function::parse(const string &argv) {
	MathStructure vargs;
	args(argv, vargs);	
	return createFunctionMathStructureFromVArgs(vargs);
}
bool Function::testArguments(MathStructure &vargs) {
	unsigned int last = 0;
	for(Sgi::hash_map<unsigned int, Argument*>::iterator it = argdefs.begin(); it != argdefs.end(); ++it) {
		if(it->first > last) {
			last = it->first;
		}
		if(it->second && it->first > 0 && it->first <= vargs.size() && !it->second->test(vargs[it->first - 1], it->first, this)) {
			return false;
		}
	}
	if(max_argc < 0 && (int) last > argc && argdefs.count(last)) {
		for(unsigned int i = last + 1; i <= vargs.size(); i++) {
			if(!argdefs[last]->test(vargs[i - 1], i, this)) {
				return false;
			}
		}
	}
	return testCondition(vargs);
}
MathStructure Function::calculate(MathStructure &vargs, const EvaluationOptions &eo) {
	int itmp = vargs.size();
	if(testArgumentCount(itmp)) {
		while(itmp < maxargs()) {
			vargs.addItem(CALCULATOR->parse(default_values[itmp - minargs()]));
			itmp++;
		}
		MathStructure mstruct;
		bool b = false;
		for(unsigned int i = 0; i < vargs.size(); i++) {
			if(vargs[i].type() == STRUCT_ALTERNATIVES) {
				b = true;
				break;
			}
		} 
		if(b) {
			vector<unsigned int> solutions;
			solutions.reserve(vargs.size());
			for(unsigned int i = 0; i < vargs.size(); i++) {
				solutions.push_back(0);
			}
			b = true;
			while(true) {
				MathStructure vargs_copy(vargs);
				for(unsigned int i = 0; i < vargs_copy.size(); i++) {
					if(vargs_copy[i].type() == STRUCT_ALTERNATIVES) {
						if(!b && solutions[i] < vargs_copy[i].countChilds()) {
							vargs_copy[i] = vargs_copy[i].getChild(solutions[i] + 1);
							solutions[i]++;
							b = true;
						} else {
							solutions[i] = 0;
							vargs_copy[i] = vargs_copy[i].getChild(solutions[i] + 1);
						}
					}
				}
				if(!b) break;
				MathStructure mstruct2;
				if(!testArguments(vargs_copy) || !calculate(mstruct2, vargs_copy, eo)) {
					mstruct2 = createFunctionMathStructureFromVArgs(vargs_copy);
				} else {
					if(isApproximate()) mstruct2.setApproximate();
				}
				if(mstruct.isZero()) {
					mstruct = mstruct2;
				} else {
					mstruct.addAlternative(mstruct2);
				}
				b = false;	 			
			}
		} else {
			if(!testArguments(vargs) || !calculate(mstruct, vargs, eo)) {
				return createFunctionMathStructureFromVArgs(vargs);
			}
		}
		if(isApproximate()) mstruct.setApproximate();
		return mstruct;
	} else {
		return createFunctionMathStructureFromVArgs(vargs);
	}
}
bool Function::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	//mstruct = createFunctionMathStructureFromVArgs(vargs);
	return false;
}
void Function::setDefaultValue(unsigned int arg_, string value_) {
	if((int) arg_ > argc && (int) arg_ <= max_argc && (int) default_values.size() >= (int) arg_ - argc) {
		default_values[arg_ - argc - 1] = value_;
	}
}
string Function::getDefaultValue(unsigned int arg_) const {
	if((int) arg_ > argc && (int) arg_ <= max_argc && (int) default_values.size() >= (int) arg_ - argc) {
		return default_values[arg_ - argc - 1];
	}
	return "";
}
int Function::stringArgs(const string &argstr, vector<string> &svargs) {
	svargs.clear();
	int start_pos = 0;
	bool in_cit1 = false, in_cit2 = false;
	int pars = 0;
	int itmp = 0;
	string str = argstr, stmp;
	remove_blank_ends(str);
	for(unsigned int str_index = 0; str_index < str.length(); str_index++) {
		switch(str[str_index]) {
			case LEFT_PARENTHESIS_CH: {
				if(!in_cit1 && !in_cit2) {
					pars++;
				}
				break;
			}
			case RIGHT_PARENTHESIS_CH: {
				if(!in_cit1 && !in_cit2 && pars > 0) {
					pars--;
				}
				break;
			}
			case '\"': {
				if(in_cit1) {
					in_cit1 = false;
				} else if(!in_cit2) {
					in_cit1 = true;
				}
				break;
			}
			case '\'': {
				if(in_cit2) {
					in_cit2 = false;
				} else if(!in_cit1) {
					in_cit1 = true;
				}
				break;
			}
			case COMMA_CH: {
				if(pars == 0 && !in_cit1 && !in_cit2) {
					itmp++;
					if(itmp <= maxargs() || args() < 0) {
						stmp = str.substr(start_pos, str_index - start_pos);
						remove_blank_ends(stmp);																				
						remove_parenthesis(stmp);						
						remove_blank_ends(stmp);
						if(stmp.empty()) {
							stmp = getDefaultValue(itmp);
						}
						svargs.push_back(stmp);
					}
					start_pos = str_index + 1;
				}
				break;
			}
		}
	}
	if(!str.empty()) {
		itmp++;
		if(itmp <= maxargs() || args() < 0) {
			stmp = str.substr(start_pos, str.length() - start_pos);
			remove_blank_ends(stmp);																				
			remove_parenthesis(stmp);						
			remove_blank_ends(stmp);
			if(stmp.empty()) {
				stmp = getDefaultValue(itmp);
			}
			svargs.push_back(stmp);
		}	
	}
	if(itmp < maxargs() && itmp >= minargs()) {
		int itmp2 = itmp;
		while(itmp2 < maxargs()) {
			svargs.push_back(default_values[itmp2 - minargs()]);	
			itmp2++;
		}
	}
	return itmp;
}

MathStructure Function::produceVector(const MathStructure &vargs, int begin, int end) {	
	if(begin < 1) {
		begin = minargs() + 1;
		if(begin < 1) begin = 1;
	}
	if(end < 1 || end >= (int) vargs.size()) {
		end = vargs.size();
	}
	if(begin == 1 && vargs.size() == 1) {
		if(vargs.getComponent(1)->isVector()) {
			return *vargs.getComponent(1);
		} else {
			return vargs;
		}
	}
	return vargs.range(begin, end).flattenVector();
}
MathStructure Function::produceArgumentsVector(const MathStructure &vargs, int begin, int end) {	
	if(begin < 1) {
		begin = minargs() + 1;
		if(begin < 1) begin = 1;
	}
	if(end < 1 || end >= (int) vargs.size()) {
		end = vargs.size();
	}
	if(begin == 1 && vargs.size() == 1) {
		return vargs;
	}
	return vargs.range(begin, end);
}

UserFunction::UserFunction(string cat_, string name_, string eq_, bool is_local, int argc_, string title_, string descr_, int max_argc_, bool is_active) : Function(name_, argc_, max_argc_, cat_, title_, descr_, is_active) {
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

bool UserFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(args() != 0) {
		string stmp = eq_calc;
		string svar;
		string v_str;
		vector<int> v_id;
		unsigned int i2 = 0;
		int i_args = maxargs();
		if(i_args < 0) {
			i_args = minargs();
		}
		for(int i = 0; i < i_args; i++) {
			svar = '\\';
			if('x' + i > 'z') {
				svar += (char) ('a' + i - 3);
			} else {
				svar += 'x' + i;
			}
			i2 = 0;	
			v_id.push_back(CALCULATOR->addId(vargs[i], true));
			v_str = LEFT_PARENTHESIS ID_WRAP_LEFT;
			v_str += i2s(v_id[v_id.size() - 1]);
			v_str += ID_WRAP_RIGHT RIGHT_PARENTHESIS;			
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
			string w_str;
			if(stmp.find("\\v") != string::npos) {
				v_id.push_back(CALCULATOR->addId(produceVector(vargs), true));
				v_str = LEFT_PARENTHESIS ID_WRAP_LEFT;
				v_str += i2s(v_id[v_id.size() - 1]);
				v_str += ID_WRAP_RIGHT RIGHT_PARENTHESIS;					
			}
			if(stmp.find("\\w") != string::npos) {
				v_id.push_back(CALCULATOR->addId(produceArgumentsVector(vargs), true));
				w_str = LEFT_PARENTHESIS ID_WRAP_LEFT;
				w_str += i2s(v_id[v_id.size() - 1]);
				w_str += ID_WRAP_RIGHT RIGHT_PARENTHESIS;							
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
		mstruct = CALCULATOR->parse(stmp);
		CALCULATOR->setRPNMode(was_rpn);

		for(unsigned int i = 0; i < v_id.size(); i++) {
			CALCULATOR->delId(v_id[i], true);
		}
		if(isApproximate()) mstruct.setApproximate();
	} else {
		bool was_rpn = CALCULATOR->inRPNMode();
		CALCULATOR->setRPNMode(false);
		mstruct = CALCULATOR->parse(eq_calc);
		CALCULATOR->setRPNMode(was_rpn);
		if(isApproximate()) mstruct.setApproximate();
	}
	return true;
}
void UserFunction::setEquation(string new_eq, int argc_, int max_argc_) {
	setChanged(true);
	eq = new_eq;
	default_values.clear();
	if(argc_ < 0) {
		argc_ = 0, max_argc_ = 0;
		string svar, svar_o, svar_v;
		bool optionals = false;
		int i3 = 0, i4 = 0, i5 = 0;
		unsigned int i2 = 0;
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
					if((i3 = new_eq.find(ID_WRAP_RIGHT_CH, i2 + 2)) != (int) string::npos) {
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
				argoccs[i + 1] = 1;
				while((i2 = new_eq.find(svar, i2 + 2)) != string::npos) {
					if(new_eq[i2 - 1] != '\\') {
						argoccs[i + 1]++;
					}
				}
			} else if((i2 = new_eq.find(svar, i5)) != string::npos) {
				if(i2 > 0 && new_eq[i2 - 1] == '\\') {
					i5 = i2 + 2;
					goto before_find_in_set_equation;
				}
				argoccs[i + 1] = 1;
				while((i2 = new_eq.find(svar, i2 + 2)) != string::npos) {
					if(new_eq[i5 - 1] != '\\') {
						argoccs[i + 1]++;
					}
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
		if(argc_ < 0) argc_ = 0;
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

Argument::Argument(string name_, bool does_test, bool does_error) {
	sname = name_;
	remove_blank_ends(sname);
	scondition = "";
	b_zero = true;
	b_test = does_test;
	b_matrix = false;
	b_text = false;
	b_error = does_error;
}
Argument::Argument(const Argument *arg) {
	b_text = false;
	set(arg);
}
Argument::~Argument() {}
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
bool Argument::test(MathStructure &value, int index, Function *f, const EvaluationOptions &eo) const {
	if(!b_test) {
		return true;
	}
	bool evaled = false;
	bool b = subtest(value, eo);
	if(b && !b_zero) {
		if(!value.isNumber()) {
			value.eval(eo);	
			evaled = true;
		}
		b = !value.isZero();
	}
	if(!b && b_matrix) {
		if(!evaled && !value.isMatrix()) {
			value.eval(eo);
			evaled = true;
		}
		b = value.isMatrix();
	}
	if(b && !scondition.empty()) {
		string expression = scondition;
		int id = CALCULATOR->addId(value, true);
		string ids = LEFT_PARENTHESIS;
		ids += ID_WRAP_LEFT;
		ids += i2s(id);
		ids += ID_WRAP_RIGHT;
		ids += RIGHT_PARENTHESIS;
		gsub("\\x", ids, expression);
		b = CALCULATOR->testCondition(expression);
		CALCULATOR->delId(id, true);
	}
	if(!b) {
		if(b_error) {
			if(sname.empty()) {
				CALCULATOR->error(true, _("Argument %s in %s() must be %s."), i2s(index).c_str(), f->name().c_str(), printlong().c_str(), NULL);
			} else {
				CALCULATOR->error(true, _("Argument %s, %s, in %s() must be %s."), i2s(index).c_str(), sname.c_str(), f->name().c_str(), printlong().c_str(), NULL);
			}
		}
		return false;
	}
	return true;
}
MathStructure Argument::evaluate(const string &str, bool keep_exact) const {
	if(b_text) {
		int pars = 0;
		while(true) {
			int pars2 = 1;
			unsigned int i = pars;
			if((int) str.length() >= 2 + pars * 2 && str[pars] == LEFT_PARENTHESIS_CH && str[str.length() - 1 - pars] == RIGHT_PARENTHESIS_CH) {
				while(true) {
					i = str.find_first_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, i + 1);
					if(i >= str.length() - 1 - pars) {
						break;
					} else if(str[i] == LEFT_PARENTHESIS_CH) {
						pars2++;
					} else if(str[i] == RIGHT_PARENTHESIS_CH) {
						pars2--;
						if(pars2 == 0) {
							break;
						}
					}
				}
				if(pars2 > 0) {
					pars++;
				}
			} else {
				break;
			}
			if(pars2 == 0) break;
		}
		if((int) str.length() >= 2 + pars * 2) {
			if(str[pars] == ID_WRAP_LEFT_CH && str[str.length() - 1 - pars] == ID_WRAP_RIGHT_CH && str.find(ID_WRAP_RIGHT, pars + 1) == str.length() - 1 - pars) {
				return CALCULATOR->parse(str.substr(pars, str.length() - pars * 2));
			}
			if(str[pars] == '\\' && str[str.length() - 1 - pars] == '\\') {
				return CALCULATOR->parse(str.substr(1 + pars, str.length() - 2 - pars * 2));
			}	
			if((str[pars] == '\"' && str[str.length() - 1 - pars] == '\"') || (str[pars] == '\'' && str[str.length() - 1 - pars] == '\'')) {
				unsigned int i = pars + 1, cits = 0;
				while(i < str.length() - 1 - pars) {
					i = str.find(str[pars], i);
					if(i >= str.length() - 1 - pars) {
						break;
					}
					cits++;
					i++;
				}
				if((cits / 2) % 2 == 0) {
					return str.substr(1 + pars, str.length() - 2 - pars * 2);
				}
			}
		}
		return str.substr(pars, str.length() - pars * 2);
	} else {
		return CALCULATOR->parse(str);
	}
}
void Argument::evaluate(MathStructure &mstruct, const EvaluationOptions &eo) const {
	if(type() != ARGUMENT_TYPE_FREE) {
		mstruct.eval(eo);
	}
}
MathStructure Argument::parse(const string &str) const {
	if(b_text) {
		int pars = 0;
		while(true) {
			int pars2 = 1;
			unsigned int i = pars;
			if((int) str.length() >= 2 + pars * 2 && str[pars] == LEFT_PARENTHESIS_CH && str[str.length() - 1 - pars] == RIGHT_PARENTHESIS_CH) {
				while(true) {
					i = str.find_first_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, i + 1);
					if(i >= str.length() - 1 - pars) {
						break;
					} else if(str[i] == LEFT_PARENTHESIS_CH) {
						pars2++;
					} else if(str[i] == RIGHT_PARENTHESIS_CH) {
						pars2--;
						if(pars2 == 0) {
							break;
						}
					}
				}
				if(pars2 > 0) {
					pars++;
				}
			} else {
				break;
			}
			if(pars2 == 0) break;
		}
		if((int) str.length() >= 2 + pars * 2) {
			if(str[pars] == ID_WRAP_LEFT_CH && str[str.length() - 1 - pars] == ID_WRAP_RIGHT_CH && str.find(ID_WRAP_RIGHT, pars + 1) == str.length() - 1 - pars) {
				return CALCULATOR->parse(str.substr(pars, str.length() - pars * 2));
			}
			if(str[pars] == '\\' && str[str.length() - 1 - pars] == '\\') {
				return CALCULATOR->parse(str.substr(1 + pars, str.length() - 2 - pars * 2));
			}	
			if((str[pars] == '\"' && str[str.length() - 1 - pars] == '\"') || (str[pars] == '\'' && str[str.length() - 1 - pars] == '\'')) {
				unsigned int i = pars + 1, cits = 0;
				while(i < str.length() - 1 - pars) {
					i = str.find(str[pars], i);
					if(i >= str.length() - 1 - pars) {
						break;
					}
					cits++;
					i++;
				}
				if((cits / 2) % 2 == 0) {
					return str.substr(1 + pars, str.length() - 2 - pars * 2);
				}
			}
		}
		return str.substr(pars, str.length() - pars * 2);
	} else {
		return CALCULATOR->parse(str);
	}
}

bool Argument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
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
bool Argument::alerts() const {
	return b_error;
}
void Argument::setAlerts(bool does_error) {
	b_error = does_error;
}
bool Argument::suggestsQuotes() const {return false;}
int Argument::type() const {
	return ARGUMENT_TYPE_FREE;
}
bool Argument::matrixAllowed() const {return b_matrix;}
void Argument::setMatrixAllowed(bool allow_matrix) {b_matrix = allow_matrix;}

NumberArgument::NumberArgument(string name_, ArgumentMinMaxPreDefinition minmax, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {
	fmin = NULL;
	fmax = NULL;
	b_incl_min = true;
	b_incl_max = true;
	b_complex = true;
	switch(minmax) {
		case ARGUMENT_MIN_MAX_POSITIVE: {
			fmin = new Number();
			b_incl_min = false;
			break;
		}
		case ARGUMENT_MIN_MAX_NEGATIVE: {
			fmax = new Number();
			b_incl_max = false;
			break;
		}
		case ARGUMENT_MIN_MAX_NONNEGATIVE: {
			fmin = new Number();
			break;
		}
		case ARGUMENT_MIN_MAX_NONZERO: {
			setZeroForbidden(true);
			break;
		}
		default: {}
	}
}
NumberArgument::NumberArgument(const NumberArgument *arg) {
	fmin = NULL;
	fmax = NULL;
	set(arg);
}
NumberArgument::~NumberArgument() {
	if(fmin) {
		delete fmin;
	}
	if(fmax) {
		delete fmax;
	}
}
	
void NumberArgument::setMin(const Number *nmin) {
	if(!nmin) {
		if(fmin) {
			delete fmin;
		}
		return;
	}
	if(!fmin) {
		fmin = new Number(*nmin);
	} else {
		fmin->set(*nmin);
	}
}
void NumberArgument::setIncludeEqualsMin(bool include_equals) {
	b_incl_min = include_equals;
}
bool NumberArgument::includeEqualsMin() const {
	return b_incl_min;
}
const Number *NumberArgument::min() const {
	return fmin;
}
void NumberArgument::setMax(const Number *nmax) {
	if(!nmax) {
		if(fmax) {
			delete fmax;
		}
		return;
	}
	if(!fmax) {
		fmax = new Number(*nmax);
	} else {
		fmax->set(*nmax);
	}
}
void NumberArgument::setIncludeEqualsMax(bool include_equals) {
	b_incl_max = include_equals;
}
bool NumberArgument::includeEqualsMax() const {
	return b_incl_max;
}
const Number *NumberArgument::max() const {
	return fmax;
}
bool NumberArgument::complexAllowed() const {
	return b_complex;
}
void NumberArgument::setComplexAllowed(bool allow_complex) {
	b_complex = allow_complex;
}
bool NumberArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isNumber()) {
		value.eval(eo);
	}
	if(!value.isNumber() || (!b_complex && value.number().isComplex())) {
		return false;
	}
	if(fmin) {
		ComparisonResult cmpr = fmin->compare(value.number());
		if(!(cmpr == COMPARISON_RESULT_GREATER || (b_incl_min && COMPARISON_IS_EQUAL_OR_GREATER(cmpr)))) {
			return false;
		}
	}
	if(fmax) {
		ComparisonResult cmpr = fmax->compare(value.number());
		if(!(cmpr == COMPARISON_RESULT_LESS || (b_incl_max && COMPARISON_IS_EQUAL_OR_LESS(cmpr)))) {
			return false;
		}
	}	
	return true;
}
int NumberArgument::type() const {
	return ARGUMENT_TYPE_NUMBER;
}
Argument *NumberArgument::copy() const {
	return new NumberArgument(this);
}
void NumberArgument::set(const Argument *arg) {
	if(arg->type() == ARGUMENT_TYPE_NUMBER) {
		const NumberArgument *farg = (const NumberArgument*) arg;
		b_incl_min = farg->includeEqualsMin();
		b_incl_max = farg->includeEqualsMax();
		b_complex = farg->complexAllowed();
		if(fmin) {
			delete fmin;
			fmin = NULL;
		}
		if(fmax) {
			delete fmax;
			fmax = NULL;
		}
		if(farg->min()) {
			fmin = new Number(*farg->min());
		}
		if(farg->max()) {
			fmax = new Number(*farg->max());
		}		
	}
	Argument::set(arg);
}
string NumberArgument::print() const {
	return _("number");
}
string NumberArgument::subprintlong() const {
	string str;
	if(b_complex) {
		str += _("a number");
	} else {
		str += _("a real number");
	}
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

IntegerArgument::IntegerArgument(string name_, ArgumentMinMaxPreDefinition minmax, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {
	imin = NULL;
	imax = NULL;
	switch(minmax) {
		case ARGUMENT_MIN_MAX_POSITIVE: {
			imin = new Number(1, 1);
			break;
		}
		case ARGUMENT_MIN_MAX_NEGATIVE: {
			imax = new Number(-1, 1);
			break;
		}
		case ARGUMENT_MIN_MAX_NONNEGATIVE: {
			imin = new Number();
			break;
		}
		case ARGUMENT_MIN_MAX_NONZERO: {
			setZeroForbidden(true);
			break;
		}	
		default: {}	
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
	
void IntegerArgument::setMin(const Number *nmin) {
	if(!nmin) {
		if(imin) {
			delete imin;
		}
		return;
	}
	if(!imin) {
		imin = new Number(*nmin);
	} else {
		imin->set(*nmin);
	}
}
const Number *IntegerArgument::min() const {
	return imin;
}
void IntegerArgument::setMax(const Number *nmax) {
	if(!nmax) {
		if(imax) {
			delete imax;
		}
		return;
	}
	if(!imax) {
		imax = new Number(*nmax);
	} else {
		imax->set(*nmax);
	}
}
const Number *IntegerArgument::max() const {
	return imax;
}
bool IntegerArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isNumber()) {
		value.eval(eo);
	}
	if(!value.isNumber() || !value.number().isInteger()) {
		return false;
	}
	if(imin) {
		ComparisonResult cmpr = imin->compare(value.number());
		if(!(COMPARISON_IS_EQUAL_OR_GREATER(cmpr))) {
			return false;
		}
	}
	if(imax) {
		ComparisonResult cmpr = imax->compare(value.number());
		if(!(COMPARISON_IS_EQUAL_OR_LESS(cmpr))) {
			return false;
		}
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
			imin = new Number(*iarg->min());
		}
		if(iarg->max()) {
			imax = new Number(*iarg->max());
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

GiacArgument::GiacArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {}
GiacArgument::GiacArgument(const GiacArgument *arg) {set(arg);}
GiacArgument::~GiacArgument() {}
int GiacArgument::type() const {return ARGUMENT_TYPE_GIAC;}
Argument *GiacArgument::copy() const {return new GiacArgument(this);}
string GiacArgument::subprintlong() const {return _("a free value (giac adjusted)");}
MathStructure GiacArgument::evaluate(const string &str, bool keep_exact) const {
	MathStructure mstruct = CALCULATOR->parse(str);
	return mstruct;
}
MathStructure GiacArgument::parse(const string &str) const {
	return CALCULATOR->parse(str);
}

SymbolicArgument::SymbolicArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {b_text = true;}
SymbolicArgument::SymbolicArgument(const SymbolicArgument *arg) {set(arg);}
SymbolicArgument::~SymbolicArgument() {}
bool SymbolicArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic() && (!value.isVariable() || value.variable()->isKnown())) {
		value.eval(eo);
	}
	return value.isSymbolic() || value.isVariable();
}
int SymbolicArgument::type() const {return ARGUMENT_TYPE_SYMBOLIC;}
Argument *SymbolicArgument::copy() const {return new SymbolicArgument(this);}
string SymbolicArgument::print() const {return _("symbol");}
string SymbolicArgument::subprintlong() const {return _("an unknown variable/symbol");}

TextArgument::TextArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {b_text = true;}
TextArgument::TextArgument(const TextArgument *arg) {set(arg); b_text = true;}
TextArgument::~TextArgument() {}
bool TextArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	return value.isSymbolic();
}
int TextArgument::type() const {return ARGUMENT_TYPE_TEXT;}
Argument *TextArgument::copy() const {return new TextArgument(this);}
string TextArgument::print() const {return _("text");}
string TextArgument::subprintlong() const {return _("a text string");}
bool TextArgument::suggestsQuotes() const {return false;}

DateArgument::DateArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) { b_text = true;}
DateArgument::DateArgument(const DateArgument *arg) {set(arg); b_text = true;}
DateArgument::~DateArgument() {}
bool DateArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	int day = 0, year = 0, month = 0;
	return value.isSymbolic() && s2date(value.symbol(), day, year, month);
}
int DateArgument::type() const {return ARGUMENT_TYPE_DATE;}
Argument *DateArgument::copy() const {return new DateArgument(this);}
string DateArgument::print() const {return _("date");}
string DateArgument::subprintlong() const {return _("a date");}

VectorArgument::VectorArgument(string name_, bool does_test, bool allow_matrix, bool does_error) : Argument(name_, does_test, does_error) {
	setMatrixAllowed(allow_matrix);
	b_argloop = true;
}
VectorArgument::VectorArgument(const VectorArgument *arg) {
	set(arg);
	b_argloop = arg->reoccuringArguments();
	unsigned int i = 1; 
	while(true) {
		if(!arg->getArgument(i)) break;
		subargs.push_back(arg->getArgument(i)->copy());
		i++;
	}	
}
VectorArgument::~VectorArgument() {
	for(unsigned int i = 0; i < subargs.size(); i++) {
		delete subargs[i];
	}
}
bool VectorArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isVector()) {
		value.eval(eo);
	}
	if(!value.isVector()) return false;
	if(b_argloop && subargs.size() > 0) {
		for(unsigned int i = 0; i < value.components(); i++) {
			if(!subargs[i % subargs.size()]->test(value[i], 1, NULL, eo)) {
				return false;
			}
		}
	} else {
		for(unsigned int i = 0; i < subargs.size() && i < value.components(); i++) {
			if(!subargs[i]->test(value[i], 1, NULL, eo)) {
				return false;
			}
		}
	}
	return true;
}
int VectorArgument::type() const {return ARGUMENT_TYPE_VECTOR;}
Argument *VectorArgument::copy() const {return new VectorArgument(this);}
string VectorArgument::print() const {return _("vector");}
string VectorArgument::subprintlong() const {
	if(subargs.size() > 0) {
		string str = _("a vector with");
		for(unsigned int i = 0; i < subargs.size(); i++) {
			if(i > 0) {
				str += ", ";
			}
			str += subargs[i]->printlong();
		}
		if(b_argloop) {
			str += ", ...";
		}
		return str;
	} else {
		return _("a vector");
	}
}
bool VectorArgument::reoccuringArguments() const {
	return b_argloop;
}
void VectorArgument::setReoccuringArguments(bool reocc) {
	b_argloop = reocc;
}
void VectorArgument::addArgument(Argument *arg) {
	arg->setAlerts(false);
	subargs.push_back(arg);
}
void VectorArgument::delArgument(unsigned int index) {
	if(index > 0 && index <= subargs.size()) {
		subargs.erase(subargs.begin() + (index - 1));
	}
}
unsigned int VectorArgument::countArguments() const {
	return subargs.size();
}
Argument *VectorArgument::getArgument(unsigned int index) const {
	if(index > 0 && index <= subargs.size()) {
		return subargs[index - 1];
	}
	return NULL;
}

MatrixArgument::MatrixArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {
	b_sym = false;
}
MatrixArgument::MatrixArgument(const MatrixArgument *arg) {
	set(arg);
	b_sym = arg->symmetricDemanded();
}
MatrixArgument::~MatrixArgument() {}
bool MatrixArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isMatrix()) {
		value.eval(eo);
	}
	return value.isMatrix() && (!b_sym || value.matrixIsSymmetric());
}
bool MatrixArgument::symmetricDemanded() const {return b_sym;}
void MatrixArgument::setSymmetricDemanded(bool sym) {b_sym = sym;}
int MatrixArgument::type() const {return ARGUMENT_TYPE_MATRIX;}
Argument *MatrixArgument::copy() const {return new MatrixArgument(this);}
string MatrixArgument::print() const {return _("matrix");}
string MatrixArgument::subprintlong() const {
	if(b_sym) {
		return _("a symmetric matrix");
	} else {
		return _("a matrix");
	}
}

ExpressionItemArgument::ExpressionItemArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {b_text = true;}
ExpressionItemArgument::ExpressionItemArgument(const ExpressionItemArgument *arg) {set(arg); b_text = true;}
ExpressionItemArgument::~ExpressionItemArgument() {}
bool ExpressionItemArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	return value.isSymbolic() && CALCULATOR->getExpressionItem(value.symbol());
}
int ExpressionItemArgument::type() const {return ARGUMENT_TYPE_EXPRESSION_ITEM;}
Argument *ExpressionItemArgument::copy() const {return new ExpressionItemArgument(this);}
string ExpressionItemArgument::print() const {return _("object");}
string ExpressionItemArgument::subprintlong() const {return _("a valid function, unit or variable name");}

FunctionArgument::FunctionArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {b_text = true;}
FunctionArgument::FunctionArgument(const FunctionArgument *arg) {set(arg); b_text = true;}
FunctionArgument::~FunctionArgument() {}
bool FunctionArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	return value.isSymbolic() && CALCULATOR->getFunction(value.symbol());
}
int FunctionArgument::type() const {return ARGUMENT_TYPE_FUNCTION;}
Argument *FunctionArgument::copy() const {return new FunctionArgument(this);}
string FunctionArgument::print() const {return _("function");}
string FunctionArgument::subprintlong() const {return _("a valid function name");}

UnitArgument::UnitArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {b_text = true;}
UnitArgument::UnitArgument(const UnitArgument *arg) {set(arg); b_text = true;}
UnitArgument::~UnitArgument() {}
bool UnitArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	return value.isSymbolic() && CALCULATOR->getUnit(value.symbol());
}
int UnitArgument::type() const {return ARGUMENT_TYPE_UNIT;}
Argument *UnitArgument::copy() const {return new UnitArgument(this);}
string UnitArgument::print() const {return _("unit");}
string UnitArgument::subprintlong() const {return _("a valid unit name");}

VariableArgument::VariableArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {b_text = true;}
VariableArgument::VariableArgument(const VariableArgument *arg) {set(arg); b_text = true;}
VariableArgument::~VariableArgument() {}
bool VariableArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	return value.isSymbolic() && CALCULATOR->getVariable(value.symbol());
}
int VariableArgument::type() const {return ARGUMENT_TYPE_VARIABLE;}
Argument *VariableArgument::copy() const {return new VariableArgument(this);}
string VariableArgument::print() const {return _("variable");}
string VariableArgument::subprintlong() const {return _("a valid variable name");}

FileArgument::FileArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {b_text = true;}
FileArgument::FileArgument(const FileArgument *arg) {set(arg); b_text = true;}
FileArgument::~FileArgument() {}
bool FileArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	return value.isSymbolic();
}
int FileArgument::type() const {return ARGUMENT_TYPE_FILE;}
Argument *FileArgument::copy() const {return new FileArgument(this);}
string FileArgument::print() const {return _("file");}
string FileArgument::subprintlong() const {return _("a valid file name");}

BooleanArgument::BooleanArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {}
BooleanArgument::BooleanArgument(const BooleanArgument *arg) {set(arg);}
BooleanArgument::~BooleanArgument() {}
bool BooleanArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isNumber()) {
		value.eval(eo);
	}
	return value.isZero() || value.isOne();
}
int BooleanArgument::type() const {return ARGUMENT_TYPE_BOOLEAN;}
Argument *BooleanArgument::copy() const {return new BooleanArgument(this);}
string BooleanArgument::print() const {return _("boolean");}
string BooleanArgument::subprintlong() const {return _("a boolean (0 or 1)");}

AngleArgument::AngleArgument(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {}
AngleArgument::AngleArgument(const AngleArgument *arg) {set(arg);}
AngleArgument::~AngleArgument() {}
bool AngleArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(CALCULATOR->u_rad && value.convert(CALCULATOR->u_rad) && value.contains(CALCULATOR->u_rad)) {
		value /= CALCULATOR->u_rad;
	} else {
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
		    		value *= (Variable*) CALCULATOR->v_pi;
	    			value /= 180;
				break;
			}
			case GRADIANS: {
				value *= (Variable*) CALCULATOR->v_pi;
	    			value /= 200;
				break;
			}
		}
	}
	return true;
}
int AngleArgument::type() const {return ARGUMENT_TYPE_ANGLE;}
Argument *AngleArgument::copy() const {return new AngleArgument(this);}
string AngleArgument::print() const {return _("angle");}
string AngleArgument::subprintlong() const {return _("an angle or a number (using the default angle unit)");}
MathStructure AngleArgument::evaluate(const string &str, bool keep_exact) const {
	bool was_cv = CALCULATOR->donotCalculateVariables();
	CALCULATOR->setDonotCalculateVariables(true);
	MathStructure mstruct = CALCULATOR->parse(str);
	CALCULATOR->setAngleValue(mstruct);
	CALCULATOR->setDonotCalculateVariables(was_cv);
	return mstruct;
}
MathStructure AngleArgument::parse(const string &str) const {
	return CALCULATOR->parse(str);
}

ArgumentSet::ArgumentSet(string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {
}
ArgumentSet::ArgumentSet(const ArgumentSet *arg) {
	set(arg); 
	unsigned int i = 1;
	while(true) {
		if(!arg->getArgument(i)) break;
		subargs.push_back(arg->getArgument(i)->copy());
		i++;
	}
}
ArgumentSet::~ArgumentSet() {
	for(unsigned int i = 0; i < subargs.size(); i++) {
		delete subargs[i];
	}
}
bool ArgumentSet::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	for(unsigned int i = 0; i < subargs.size(); i++) {
		if(subargs[i]->test(value, 1, NULL, eo)) {
			return true;
		}
	}
	return false;
}
int ArgumentSet::type() const {return ARGUMENT_TYPE_SET;}
Argument *ArgumentSet::copy() const {return new ArgumentSet(this);}
string ArgumentSet::print() const {
	string str = "";
	for(unsigned int i = 0; i < subargs.size(); i++) {
		if(i > 0) {
			if(i == subargs.size() - 1) {
				str += " ";
				str += _("or");
				str += " ";
			} else {
				str += ", ";
			}
		}
		str += subargs[i]->print();
	}
	return str;
}
string ArgumentSet::subprintlong() const {
	string str = "";
	for(unsigned int i = 0; i < subargs.size(); i++) {
		if(i > 0) {
			if(i == subargs.size() - 1) {
				str += " ";
				str += _("or");
				str += " ";
			} else {
				str += ", ";
			}
		}
		str += subargs[i]->printlong();
	}
	return str;
}
void ArgumentSet::addArgument(Argument *arg) {
	arg->setAlerts(false);
	subargs.push_back(arg);
}
void ArgumentSet::delArgument(unsigned int index) {
	if(index > 0 && index <= subargs.size()) {
		subargs.erase(subargs.begin() + (index - 1));
	}
}
unsigned int ArgumentSet::countArguments() const {
	return subargs.size();
}
Argument *ArgumentSet::getArgument(unsigned int index) const {
	if(index > 0 && index <= subargs.size()) {
		return subargs[index - 1];
	}
	return NULL;
}
