/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "BuiltinFunctions.h"
#include "util.h"
#include "Matrix.h"
#include "Manager.h"
#include "Fraction.h"
#include "Calculator.h"
#include "Variable.h"

#include <sstream>

#define TRIG_FUNCTION(FUNC)	mngr->set(vargs[0]); mngr->recalculateVariables(); if(!mngr->isFraction() || !mngr->fraction()->FUNC()) {mngr->set(this, vargs[0], NULL);} else {mngr->setPrecise(mngr->fraction()->isPrecise());}
#define FR_FUNCTION(FUNC)	mngr->set(vargs[0]); if(!mngr->fraction()->FUNC()) {mngr->set(this, vargs[0], NULL);} else {mngr->setPrecise(mngr->fraction()->isPrecise());}
#define FR_FUNCTION_2(FUNC)	mngr->set(vargs[0]); if(!mngr->fraction()->FUNC(vargs[1]->fraction())) {mngr->set(this, vargs[0], vargs[1], NULL);} else {mngr->setPrecise(mngr->fraction()->isPrecise());}




PiFunction::PiFunction() : Function("Constants", "PI", 0, "Archimede's Constant (pi)") {}
void PiFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
//		mngr->set(name());
		mngr->set(this, NULL);
	} else {
		Fraction fr; fr.pi(); mngr->set(&fr);
	}
}
EFunction::EFunction() : Function("Constants", "EXP0", 0, "The Base of Natural Logarithms (e)") {}
void EFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
//		mngr->set(name());
		mngr->set(this, NULL);		
	} else {
		Fraction fr; fr.e(); mngr->set(&fr);
	}
}
PythagorasFunction::PythagorasFunction() : Function("Constants", "PYTHAGORAS", 0, "Pythagora's Constant (sqrt 2)") {}
void PythagorasFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
//		mngr->set(name());
		mngr->set(this, NULL);		
	} else {
		Fraction fr; fr.pythagoras(); mngr->set(&fr);
	}
}
EulerFunction::EulerFunction() : Function("Constants", "EULER", 0, "Euler's Constant") {}
void EulerFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
//		mngr->set(name());
		mngr->set(this, NULL);		
	} else {
		Fraction fr; fr.euler(); mngr->set(&fr);
	}
}
GoldenFunction::GoldenFunction() : Function("Constants", "GOLDEN", 0, "The Golden Ratio") {}
void GoldenFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
//		mngr->set(name());
		mngr->set(this, NULL);		
	} else {
		Fraction fr; fr.golden(); mngr->set(&fr);
	}
}
AperyFunction::AperyFunction() : Function("Constants", "APERY", 0, "Apery's Constant") {}
void AperyFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
//		mngr->set(name());
		mngr->set(this, NULL);		
	} else {
		Fraction fr; fr.apery(); mngr->set(&fr);
	}
}
CatalanFunction::CatalanFunction() : Function("Constants", "CATALAN", 0, "Catalan's Constant") {}
void CatalanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
//		mngr->set(name());
		mngr->set(this, NULL);		
	} else {
		Fraction fr; fr.catalan(); mngr->set(&fr);
	}
}


#ifdef HAVE_LIBCLN
ZetaFunction::ZetaFunction() : Function("", "zeta", 1, "Riemann Zeta") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
}
void ZetaFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	FR_FUNCTION(zeta)
}
#endif

ErrorFunction::ErrorFunction() : Function("Utilities", "error", 1, "Display error") {
	setArgumentDefinition(1, new TextArgument());
}
void ErrorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	CALCULATOR->error(true, vargs[0]->text().c_str(), NULL);
}
WarningFunction::WarningFunction() : Function("Utilities", "warning", 1, "Display warning") {
	setArgumentDefinition(1, new TextArgument());
}
void WarningFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	CALCULATOR->error(false, vargs[0]->text().c_str(), NULL);
}
MessageFunction::MessageFunction() : Function("Utilities", "message", 1, "Display a message", "", -1) {
	setArgumentDefinition(1, new TextArgument());
}
void MessageFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	CALCULATOR->error(false, vargs[0]->text().c_str(), NULL);
}

ForFunction::ForFunction() : Function("Logical", "for", 5, "for...do") {
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());	
	setArgumentDefinition(5, new TextArgument());
}
void ForFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	string condition = vargs[1]->text();
	string counter = vargs[2]->text();
	string action = vargs[4]->text();

	gsub("\\x", "\"\\x\"", action);	
	gsub("\\y", "\"\\y\"", action);		
	gsub("\\i", "\"\\i\"", action);		
	Manager mngr_x("\\x");
	Manager mngr_y("\\y");
	Manager mngr_i("\\i");
	
	CALCULATOR->beginTemporaryStopErrors();
	Manager *action_mngr = CALCULATOR->calculate(action);
	CALCULATOR->endTemporaryStopErrors();	

	Manager x_mngr(vargs[0]);
	Manager y_mngr(vargs[3]);
	Manager i_mngr(1, 1);

	x_mngr.protect();
	y_mngr.protect();
	i_mngr.protect();
	int x_id = CALCULATOR->addId(&x_mngr, true);
	string str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(x_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\x", str, condition);
	gsub("\\x", str, counter);
	int y_id = CALCULATOR->addId(&y_mngr, true);
	str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(y_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\y", str, condition);	
	gsub("\\y", str, counter);
	int i_id = CALCULATOR->addId(&i_mngr, true);
	str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(i_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\i", str, condition);	
	gsub("\\i", str, counter);	
	Manager mngr_calc;
	Manager *calced = NULL;
	int i = 1;
	while(CALCULATOR->testCondition(condition)) {	
		mngr_calc.set(action_mngr);
		mngr_calc.replace(&mngr_x, &x_mngr);
		mngr_calc.replace(&mngr_y, &y_mngr);		
		mngr_calc.replace(&mngr_i, &i_mngr);
		mngr_calc.recalculateFunctions();
		mngr_calc.clean();
		y_mngr.set(&mngr_calc);		
		calced = CALCULATOR->calculate(counter);
		x_mngr.set(calced);
		calced->unref();
		i++;
		i_mngr.set(i, 1);
	}
	CALCULATOR->delId(x_id, true);
	CALCULATOR->delId(y_id, true);
	CALCULATOR->delId(i_id, true);
	action_mngr->unref();
	mngr->set(&y_mngr);
}

ProcessFunction::ProcessFunction() : Function("Utilities", "process", 1, "Process components", "", -1) {
	setArgumentDefinition(1, new TextArgument("", false));
	setArgumentDefinition(2, new MatrixArgument("", false));
}
void ProcessFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	string sarg = vargs[0]->text();
	int i = sarg.find("\\x");
	int i_length;
	while(i != (int) string::npos) {
		if(i + 2 < (int) sarg.length() && sarg[i + 2] == '_' && i + 3 < (int) sarg.length()) {
			string index_str = "component(";
			if(sarg[i + 3] == LEFT_PARENTHESIS_CH) {
				int missing = 0;
				int i2 = find_ending_bracket(sarg, i + 4, &missing);
				if(i2 == (int) string::npos) {
					for(int i3 = 1; i3 < missing; i3++) {
						sarg += RIGHT_PARENTHESIS;
					}
					index_str += sarg.substr(i + 4, sarg.length() - (i + 4));
					i_length += sarg.length() - i;
				} else {
					index_str += sarg.substr(i + 4, i2 - (i + 4));
					i_length = i2 + 1 - i;					
				}
			} else {
				int i2 = sarg.find_first_of(OPERATORS, i + 3);
				if(i2 == (int) string::npos) {
					index_str += sarg.substr(i + 3, sarg.length() - (i + 3));
					i_length = sarg.length() - i;
				} else {
					index_str += sarg.substr(i + 3, i2 - (i + 3));
					i_length += i2 - i;
				}
			}
			index_str += ",\\x)";
			sarg.replace(i, i_length, index_str);
			i += index_str.length();
		} else {
			sarg.replace(i, 2, "\\z");
			i += 2;
		}
		i = sarg.find("\\x", i);
	}	

	gsub("\\x", "\"\\x\"", sarg);	
	gsub("\\z", "\"\\z\"", sarg);		
	gsub("\\i", "\"\\i\"", sarg);
	gsub("\\c", "\"\\c\"", sarg);		
	gsub("\\r", "\"\\r\"", sarg);		
	gsub("\\n", "\"\\n\"", sarg);	
	Manager mngr_x("\\x");
	Manager mngr_z("\\z");		
	Manager mngr_i("\\i");
	Manager mngr_n("\\n");
	Manager mngr_c("\\c");
	Manager mngr_r("\\r");			
	
	CALCULATOR->beginTemporaryStopErrors();
	Manager *sarg_mngr = CALCULATOR->calculate(sarg);
	CALCULATOR->endTemporaryStopErrors();	
	
	if(vargs.size() == 1) {
	} else if(vargs.size() > 2 || (vargs[1]->isMatrix() && vargs[1]->matrix()->isVector())) {

		Vector *v = produceVector(vargs);

		Manager x_mngr(v);
		Manager i_mngr;
		Manager z_mngr;		
		Manager n_mngr(v->components(), 1);
		Manager r_mngr(1, 1);
		Manager mngr_calc;
		x_mngr.protect();
		sarg_mngr->replace(&mngr_n, &n_mngr);
		sarg_mngr->replace(&mngr_r, &r_mngr);
		for(unsigned int index = 1; index <= v->components(); index++) {
			i_mngr.set(index, 1);
			z_mngr.set(v->get(index));
			mngr_calc.set(sarg_mngr);
			mngr_calc.replace_no_copy(&mngr_x, &x_mngr);
			mngr_calc.replace(&mngr_z, &z_mngr);
			mngr_calc.replace(&mngr_i, &i_mngr);
			mngr_calc.replace(&mngr_c, &i_mngr);
			mngr_calc.recalculateFunctions();
			mngr_calc.finalize();
			v->set(&mngr_calc, index);		
		}		
		mngr->set(v);
		delete v;		
	} else if(vargs[1]->isMatrix()) {

		Matrix *mtrx = new Matrix(vargs[1]->matrix());	
		
		Manager x_mngr(mtrx);
		Manager z_mngr;
		Manager i_mngr;
		Manager r_mngr;
		Manager c_mngr;
		Manager n_mngr(mtrx->rows() * mtrx->columns(), 1);
		x_mngr.protect();
		sarg_mngr->replace(&mngr_n, &n_mngr);
		Manager mngr_calc;
		for(unsigned int index_r = 1; index_r <= mtrx->rows(); index_r++) {
			r_mngr.set(index_r, 1);						
			for(unsigned int index_c = 1; index_c <= mtrx->columns(); index_c++) {		
				z_mngr.set(mtrx->get(index_r, index_c));
				i_mngr.set((index_r - 1) * mtrx->columns() + index_c, 1);
				c_mngr.set(index_c, 1);				
				mngr_calc.set(sarg_mngr);
				mngr_calc.replace_no_copy(&mngr_x, &x_mngr);
				mngr_calc.replace(&mngr_z, &z_mngr);
				mngr_calc.replace(&mngr_i, &i_mngr);
				mngr_calc.replace(&mngr_c, &c_mngr);
				mngr_calc.replace(&mngr_r, &r_mngr);					
				mngr_calc.recalculateFunctions();
				mngr_calc.finalize();
				mtrx->set(&mngr_calc, index_r, index_c);					
			}
		}	
		mngr->set(mtrx);
		delete mtrx;			
	} else {
		Manager x_mngr(vargs[1]);
		Manager i_mngr(1, 1);
		Manager mngr_calc;
		sarg_mngr->replace(&mngr_n, &i_mngr);
		sarg_mngr->replace(&mngr_r, &i_mngr);
		sarg_mngr->replace(&mngr_c, &i_mngr);
		sarg_mngr->replace(&mngr_i, &i_mngr);		
		sarg_mngr->replace(&mngr_x, &x_mngr);
		sarg_mngr->replace(&mngr_z, &x_mngr);
		sarg_mngr->recalculateFunctions();
		sarg_mngr->finalize();
		mngr->set(sarg_mngr);				
	}
	sarg_mngr->unref();
}

CustomSumFunction::CustomSumFunction() : Function("Utilities", "csum", 4, "Custom sum of components", "", -1) {
	setArgumentDefinition(1, new IntegerArgument());
	setArgumentDefinition(2, new IntegerArgument());
	setArgumentDefinition(4, new TextArgument());
	setArgumentDefinition(5, new VectorArgument("", false));
}
void CustomSumFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	int start = 1;
	int end = -1;
	start = vargs[0]->fraction()->numerator()->getInt();
	if(start < 1) start = 1;
	end = vargs[1]->fraction()->numerator()->getInt();

	string sarg = vargs[3]->text();
	int i = sarg.find("\\x");
	int i_length;
	while(i != (int) string::npos) {
		if(i + 2 < (int) sarg.length() && sarg[i + 2] == '_' && i + 3 < (int) sarg.length()) {
			string index_str = "component(";
			if(sarg[i + 3] == LEFT_PARENTHESIS_CH) {
				int missing = 0;
				int i2 = find_ending_bracket(sarg, i + 4, &missing);
				if(i2 == (int) string::npos) {
					for(int i3 = 1; i3 < missing; i3++) {
						sarg += RIGHT_PARENTHESIS;
					}
					index_str += sarg.substr(i + 4, sarg.length() - (i + 4));
					i_length += sarg.length() - i;
				} else {
					index_str += sarg.substr(i + 4, i2 - (i + 4));
					i_length = i2 + 1 - i;					
				}
			} else {
				int i2 = sarg.find_first_of(OPERATORS, i + 3);
				if(i2 == (int) string::npos) {
					index_str += sarg.substr(i + 3, sarg.length() - (i + 3));
					i_length = sarg.length() - i;
				} else {
					index_str += sarg.substr(i + 3, i2 - (i + 3));
					i_length += i2 - i;
				}
			}
			index_str += ",\\x)";
			sarg.replace(i, i_length, index_str);
			i += index_str.length();
		} else {
			sarg.replace(i, 2, "\\z");
			i += 2;
		}
		i = sarg.find("\\x", i);
	}	

	gsub("\\x", "\"\\x\"", sarg);	
	gsub("\\y", "\"\\y\"", sarg);	
	gsub("\\z", "\"\\z\"", sarg);		
	gsub("\\i", "\"\\i\"", sarg);
	gsub("\\c", "\"\\c\"", sarg);		
	gsub("\\r", "\"\\r\"", sarg);		
	gsub("\\n", "\"\\n\"", sarg);	
	Manager mngr_x("\\x");
	Manager mngr_y("\\y");
	Manager mngr_z("\\z");		
	Manager mngr_i("\\i");
	Manager mngr_n("\\n");
	Manager mngr_c("\\c");
	Manager mngr_r("\\r");			
	
	CALCULATOR->beginTemporaryStopErrors();
	Manager *sarg_mngr = CALCULATOR->calculate(sarg);
	CALCULATOR->endTemporaryStopErrors();	

	Manager *y_mngr = new Manager(vargs[2]);
	if(vargs.size() == 4) {
	} else if(vargs.size() > 5 || (vargs[4]->isMatrix() && vargs[4]->matrix()->isVector())) {

		Vector *v = produceVector(vargs);

		int n = v->components();
		if(start > n) start = n;
		if(end < 1 || end > n) end = n;
		else if(end < start) end = start;	

		Manager x_mngr(v);
		Manager i_mngr;
		Manager z_mngr;		
		Manager n_mngr(v->components(), 1);
		Manager r_mngr(1, 1);
		Manager mngr_calc;
		x_mngr.protect();
		sarg_mngr->replace(&mngr_n, &n_mngr);
		sarg_mngr->replace(&mngr_r, &r_mngr);
		for(int index = start; index <= end; index++) {	
			i_mngr.set(index, 1);
			z_mngr.set(v->get(index));
			mngr_calc.set(sarg_mngr);
			mngr_calc.replace_no_copy(&mngr_x, &x_mngr);
			mngr_calc.replace(&mngr_y, y_mngr);
			mngr_calc.replace(&mngr_z, &z_mngr);
			mngr_calc.replace(&mngr_i, &i_mngr);
			mngr_calc.replace(&mngr_c, &i_mngr);
			mngr_calc.recalculateFunctions();
			mngr_calc.clean();
			y_mngr->set(&mngr_calc);
		}		
		delete v;		
	} else if(vargs[4]->isMatrix()) {
		Matrix *mtrx = new Matrix(vargs[4]->matrix());	
		
		int n = mtrx->columns() * mtrx->rows();
		if(start > n) start = n;
		if(end < 1) end = n;
		else if(end > n) end = n;
		else if(end < start) end = start;		
		
		Manager x_mngr(mtrx);
		Manager z_mngr;
		Manager i_mngr;
		Manager r_mngr;
		Manager c_mngr;
		Manager n_mngr(mtrx->rows() * mtrx->columns(), 1);
		x_mngr.protect();
		sarg_mngr->replace(&mngr_n, &n_mngr);
		Manager mngr_calc;
		int i;
		for(unsigned int index_r = 1; index_r <= mtrx->rows(); index_r++) {
			r_mngr.set(index_r, 1);						
			for(unsigned int index_c = 1; index_c <= mtrx->columns(); index_c++) {		
				i = (index_r - 1) * mtrx->columns() + index_c;
				if(i >= start && i <= end) {
					z_mngr.set(mtrx->get(index_r, index_c));
					i_mngr.set(i, 1);
					c_mngr.set(index_c, 1);				
					mngr_calc.set(sarg_mngr);
					mngr_calc.replace_no_copy(&mngr_x, &x_mngr);
					mngr_calc.replace(&mngr_y, y_mngr);
					mngr_calc.replace(&mngr_z, &z_mngr);
					mngr_calc.replace(&mngr_i, &i_mngr);
					mngr_calc.replace(&mngr_c, &c_mngr);
					mngr_calc.replace(&mngr_r, &r_mngr);					
					mngr_calc.recalculateFunctions();
					mngr_calc.clean();
					y_mngr->set(&mngr_calc);					
				}
			}
		}	
		delete mtrx;			
	} else {
		Manager x_mngr(vargs[4]);
		Manager i_mngr(1, 1);
		Manager mngr_calc;
		sarg_mngr->replace(&mngr_n, &i_mngr);
		sarg_mngr->replace(&mngr_r, &i_mngr);
		sarg_mngr->replace(&mngr_c, &i_mngr);
		sarg_mngr->replace(&mngr_i, &i_mngr);		
		sarg_mngr->replace(&mngr_y, y_mngr);
		sarg_mngr->replace(&mngr_x, &x_mngr);
		sarg_mngr->replace(&mngr_z, &x_mngr);
		sarg_mngr->recalculateFunctions();
		sarg_mngr->clean();
		y_mngr->set(sarg_mngr);				
	}
	sarg_mngr->unref();
	mngr->set(y_mngr);
	y_mngr->unref();	
}

FunctionFunction::FunctionFunction() : Function("Utilities", "function", 1, "Function", "", -1) {
	setArgumentDefinition(1, new TextArgument());
}
void FunctionFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	UserFunction f("", "Generated Function", vargs[0]->text());
	vector<Manager*> vargs2(vargs);
	vargs2.erase(vargs2.begin());
	Manager *mngr2 = f.calculate(vargs2);	
	mngr->set(mngr2);
	mngr2->unref();
}
MatrixFunction::MatrixFunction() : Function("Matrices", "matrix", 2, "Construct Matrix", "", -1) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
}
void MatrixFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Matrix mtrx(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt());
	unsigned int r = 1, c = 1;
	for(unsigned int i = 2; i < vargs.size(); i++) {
		if(r > mtrx.rows()) {
			CALCULATOR->error(false, _("Too many elements (%s) for the order (%sx%s) of the matrix."), i2s(vargs.size() - 2).c_str(), i2s(mtrx.rows()).c_str(), i2s(mtrx.columns()).c_str(), NULL);
			break;
		}
		mtrx.set(vargs[i], r, c);	
		if(c == mtrx.columns()) {
			c = 1;
			r++;
		} else {
			c++;
		}
	}
	mngr->set(&mtrx);
}
VectorFunction::VectorFunction() : Function("Matrices", "vector", -1, "Construct Vector") {}
void VectorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Vector vctr(vargs.size());
	for(unsigned int i = 0; i < vargs.size(); i++) {
		vctr.set(vargs[i], i + 1);	
	}
	mngr->set(&vctr);
}
RankFunction::RankFunction() : Function("Matrices", "rank", -1, "Rank") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void RankFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() > 1) {
		Vector *v = produceVector(vargs);
		if(v->rank()) {
			mngr->set(v);
		} else {
			Manager *mngr2 = createFunctionManagerFromVArgs(vargs);
			mngr->set(mngr2);
			mngr2->unref();
		}
		delete v;
	} else if(vargs.size() == 1) {
		if(vargs[0]->isMatrix()) {
			mngr->set(vargs[0]);
			if(!mngr->matrix()->rank()) {
				mngr->set(this, vargs[0], NULL);
			}
		} else {
			mngr->set(1, 1);
		}
	}
}
SortFunction::SortFunction() : Function("Matrices", "sort", -1, "Sort") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void SortFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() > 1) {
		Vector *v = produceVector(vargs);
		if(v->sort()) {
			mngr->set(v);
		} else {
			Manager *mngr2 = createFunctionManagerFromVArgs(vargs);
			mngr->set(mngr2);
			mngr2->unref();
		}
		delete v;
	} else if(vargs.size() == 1) {
		if(vargs[0]->isMatrix()) {
			mngr->set(vargs[0]);
			if(!mngr->matrix()->sort()) {
				mngr->set(this, vargs[0], NULL);
			}
		} else {
			mngr->set(vargs[0]);
		}
	}
}
MatrixToVectorFunction::MatrixToVectorFunction() : Function("Matrices", "matrix2vector", 1, "Convert Matrix to Vector") {
	setArgumentDefinition(1, new MatrixArgument("", false));
}
void MatrixToVectorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		Vector *v = vargs[0]->matrix()->toVector();
		mngr->set(v);
		delete v;
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
RowFunction::RowFunction() : Function("Matrices", "row", 2, "Extract Row as Vector") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new MatrixArgument());	
}
void RowFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Vector *v = vargs[1]->matrix()->rowToVector(vargs[0]->fraction()->numerator()->getInt());
	if(!v) {
		CALCULATOR->error(true, _("Row %s does not exist in matrix."), vargs[0]->print().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
	} else {
		mngr->set(v);
		delete v;
	}
}
ColumnFunction::ColumnFunction() : Function("Matrices", "column", 2, "Extract Column as Vector") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new MatrixArgument());	
}
void ColumnFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Vector *v = vargs[1]->matrix()->columnToVector(vargs[0]->fraction()->numerator()->getInt());
	if(!v) {
		CALCULATOR->error(true, _("Column %s does not exist in matrix."), vargs[0]->print().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
	} else {
		mngr->set(v);
		delete v;
	}
}
RowsFunction::RowsFunction() : Function("Matrices", "rows", 1, "Rows") {
	setArgumentDefinition(1, new MatrixArgument("", false));
}
void RowsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->rows(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ColumnsFunction::ColumnsFunction() : Function("Matrices", "columns", 1, "Columns") {
	setArgumentDefinition(1, new MatrixArgument("", false));
}
void ColumnsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->columns(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ElementsFunction::ElementsFunction() : Function("Matrices", "elements", 1, "Elements") {
	setArgumentDefinition(1, new MatrixArgument("", false));
}
void ElementsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->columns() * vargs[0]->matrix()->rows(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ElementFunction::ElementFunction() : Function("Matrices", "element", 3, "Element") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(3, new MatrixArgument("", false));
}
void ElementFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[2]->isMatrix()) {
		mngr->set(vargs[2]->matrix()->get(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt()));
	} else if(vargs[0]->fraction()->isOne() && vargs[1]->fraction()->isOne()) {
		mngr->set(vargs[2]);
	}
}
ComponentsFunction::ComponentsFunction() : Function("Matrices", "components", 1, "Components") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void ComponentsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->columns() * vargs[0]->matrix()->rows(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ComponentFunction::ComponentFunction() : Function("Matrices", "component", 2, "Component") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new VectorArgument("", false));
}
void ComponentFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[1]->isMatrix()) {
		mngr->set(vargs[1]->matrix()->get((vargs[0]->fraction()->numerator()->getInt() - 1) / vargs[1]->matrix()->columns() + 1, (vargs[0]->fraction()->numerator()->getInt() - 1) % vargs[1]->matrix()->columns() + 1));
	} else if(vargs[0]->fraction()->isOne()) {
		mngr->set(vargs[1]);
	}
}
RangeFunction::RangeFunction() : Function("Matrices", "range", 3, "Range") {}
void RangeFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Vector v;
	Manager x_value(vargs[0]);
	bool b;
	while(x_value.compare(vargs[1]) <= 0) {
		if(b) v.addComponent();
		v.set(&x_value, v.components());
		x_value.add(vargs[2], OPERATION_ADD);
	}
	mngr->set(&v);
}
LimitsFunction::LimitsFunction() : Function("Matrices", "limits", 2, "Limits", "", -1) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(3, new VectorArgument("", false));	
}
void LimitsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int i = vargs[0]->fraction()->numerator()->getInt(), n = vargs[1]->fraction()->numerator()->getInt();	
	Vector *v = produceVector(vargs);
	Vector *vctr = v->getRange(i, n);
	mngr->set(vctr);
	delete vctr;
	delete v;
}
TransposeFunction::TransposeFunction() : Function("Matrices", "transpose", 1, "Transpose") {
	setArgumentDefinition(1, new MatrixArgument());
}
void TransposeFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->matrix()->transpose();
}
IdentityFunction::IdentityFunction() : Function("Matrices", "identity", 1, "Identity") {}
void IdentityFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		Matrix *mtrx = vargs[0]->matrix()->getIdentityMatrix();
		mngr->set(mtrx);
		delete mtrx;
	} else if(vargs[0]->isFraction() && vargs[0]->fraction()->isInteger() && vargs[0]->fraction()->isPositive()) {
		Matrix mtrx;
		mtrx.setToIdentityMatrix(vargs[0]->fraction()->numerator()->getInt());
		mngr->set(&mtrx);
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
DeterminantFunction::DeterminantFunction() : Function("Matrices", "det", 1, "Determinant") {
	setArgumentDefinition(1, new MatrixArgument());
}
void DeterminantFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Manager *det = vargs[0]->matrix()->determinant();
	if(!det) {
		mngr->set(this, vargs[0], NULL);
		return;
	}
	mngr->set(det);
	det->unref();	
}
CofactorFunction::CofactorFunction() : Function("Matrices", "cofactor", 3, "Cofactor") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(3, new MatrixArgument());
}
void CofactorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Manager *mngr2 = vargs[2]->matrix()->cofactor(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt());
	if(!mngr2) {
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
		return;
	}
	mngr->set(mngr2);
	mngr2->unref();	
}
AdjointFunction::AdjointFunction() : Function("Matrices", "adj", 1, "Adjoint") {
	setArgumentDefinition(1, new MatrixArgument());
}
void AdjointFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->matrix()->adjoint()) {
		mngr->set(this, vargs[0], NULL);
	}
}
InverseFunction::InverseFunction() : Function("Matrices", "inverse", 1, "Inverse") {
	setArgumentDefinition(1, new MatrixArgument());
}
void InverseFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->matrix()->inverse()) {
		mngr->set(this, vargs[0], NULL);
	}
}
IFFunction::IFFunction() : Function("Logical", "if", 3, "If...Then...Else") {
	setArgumentDefinition(1, new FractionArgument());
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());
}
void IFFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int result = vargs[0]->fraction()->getBoolean();
	if(result) {			
		Manager *mngr2 = CALCULATOR->calculate(vargs[1]->text());
		mngr->set(mngr2);
		mngr2->unref();		
	} else if(result == 0) {			
		Manager *mngr2 = CALCULATOR->calculate(vargs[2]->text());		
		mngr->set(mngr2);
		mngr2->unref();		
	} else {
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
	}	
}
GCDFunction::GCDFunction() : Function("Arithmetics", "gcd", 2, "Greatest Common Divisor") {
	setArgumentDefinition(1, new FractionArgument());
	setArgumentDefinition(2, new FractionArgument());
}
void GCDFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->gcd(vargs[1]->fraction());
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
DaysFunction::DaysFunction() : Function("Date & Time", "days", 2, "Days between two dates", "", 4) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new DateArgument());	
	IntegerArgument *arg = new IntegerArgument();
	Integer integ;
	arg->setMin(&integ);
	integ.set(4);
	arg->setMax(&integ);
	setArgumentDefinition(3, arg);	
	setArgumentDefinition(4, new BooleanArgument());				
	setDefaultValue(3, "1"); 
}
void DaysFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int days = daysBetweenDates(vargs[0]->text(), vargs[1]->text(), vargs[2]->fraction()->numerator()->getInt(), vargs[3]->fraction()->isZero());
	if(days < 0) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], vargs[3], NULL);			
	} else {
		mngr->set(days, 1, 0);
	}			
}
YearFracFunction::YearFracFunction() : Function("Date & Time", "yearfrac", 2, "Years between two dates", "", 4) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new DateArgument());	
	IntegerArgument *arg = new IntegerArgument();
	Integer integ;
	arg->setMin(&integ);
	integ.set(4);
	arg->setMax(&integ);
	setArgumentDefinition(3, arg);	
	setArgumentDefinition(4, new BooleanArgument());		
	setDefaultValue(3, "1");
}
void YearFracFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Fraction *fr = yearsBetweenDates(vargs[0]->text(), vargs[1]->text(), vargs[2]->fraction()->numerator()->getInt(), vargs[3]->fraction()->isZero());
	if(!fr) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], vargs[3], NULL);			
	} else {
		mngr->set(fr);
	}
	delete fr;
}
DifferentiateFunction::DifferentiateFunction() : Function("Experimental", "diff", 2, "Differentiate") {
	setArgumentDefinition(2, new TextArgument());
}
void DifferentiateFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	CALCULATOR->error(true, _("%s() is an experimental unfinished function!"), name().c_str(), NULL);
	if(vargs[0]->isText()) {
		Manager *mngr2 = CALCULATOR->calculate(vargs[0]->text());
		mngr->set(mngr2);
		mngr2->unref();
	} else {
		mngr->set(vargs[0]);
	}
	mngr->differentiate(vargs[1]->text());
}
FactorialFunction::FactorialFunction() : Function("Arithmetics", "factorial", 1, "Factorial") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE, true, false));
}
void FactorialFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->fraction()->isZero()) mngr->set(1, 1);
	Integer integ(vargs[0]->fraction()->numerator());
	integ.factorial();
	mngr->set(&integ);
}
BinomialFunction::BinomialFunction() : Function("Arithmetics", "binomial", 2, "Binomial") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE, true, true));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE, true, true));
	setCondition("\\x>=\\y");
}
void BinomialFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Integer integ;
	integ.binomial(vargs[0]->fraction()->numerator(), vargs[1]->fraction()->numerator());
	mngr->set(&integ);
}
AbsFunction::AbsFunction() : Function("Arithmetics", "abs", 1, "Absolute Value") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void AbsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->setNegative(false);
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
CeilFunction::CeilFunction() : Function("Arithmetics", "ceil", 1, "Round upwards") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void CeilFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->ceil();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
FloorFunction::FloorFunction() : Function("Arithmetics", "floor", 1, "Round downwards") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void FloorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->floor();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
TruncFunction::TruncFunction() : Function("Arithmetics", "trunc", 1, "Round towards zero") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void TruncFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->trunc();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
RoundFunction::RoundFunction() : Function("Arithmetics", "round", 1, "Round") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void RoundFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->round();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
FracFunction::FracFunction() : Function("Arithmetics", "frac", 1, "Extract fractional part") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void FracFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->frac();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
IntFunction::IntFunction() : Function("Arithmetics", "int", 1, "Extract integer part") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void IntFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->fraction()->trunc();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
RemFunction::RemFunction() : Function("Arithmetics", "rem", 2, "Reminder (rem)") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
	setArgumentDefinition(2, new FractionArgument("", ARGUMENT_MIN_MAX_NONZERO, true, false));
}
void RemFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);	
	mngr->fraction()->divide(vargs[1]->fraction());
	mngr->fraction()->rem();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}
ModFunction::ModFunction() : Function("Arithmetics", "mod", 2, "Reminder (mod)") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
	setArgumentDefinition(2, new FractionArgument("", ARGUMENT_MIN_MAX_NONZERO, true, false));
}
void ModFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);	
	mngr->fraction()->divide(vargs[1]->fraction());
	mngr->fraction()->mod();
	mngr->setPrecise(mngr->fraction()->isPrecise());
}

SinFunction::SinFunction() : Function("Trigonometry", "sin", 1, "Sine") {setArgumentDefinition(1, new AngleArgument("", false));}
void SinFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]); 
	if(mngr->isVariable() && mngr->variable() == CALCULATOR->getPI()) {
		mngr->clear();
		return;
	} else if(mngr->isMultiplication() && mngr->countChilds() == 2 && mngr->getChild(0)->isFraction() && mngr->getChild(1)->isVariable() && mngr->getChild(1)->variable() == CALCULATOR->getPI()) {
		if(mngr->getChild(0)->fraction()->isInteger()) {
			mngr->clear();
			return;
		} else if(!mngr->getChild(0)->fraction()->isComplex()) {
			Fraction fr(mngr->getChild(0)->fraction());
			fr.frac();
			if((fr.numerator()->isOne() || fr.numerator()->isMinusOne()) && fr.denominator()->equals(2)) {
				fr.set(mngr->getChild(0)->fraction());
				fr.floor();
				if(fr.numerator()->isEven()) {
					mngr->set(1, 1);
				} else {
					mngr->set(-1, 1);
				}
				return;
			}
		}
	}
	mngr->recalculateVariables();
	if(!mngr->isFraction() || !mngr->fraction()->sin()) {
		vargs[0]->recalculateVariables();
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->setPrecise(mngr->fraction()->isPrecise());
	}
}
CosFunction::CosFunction() : Function("Trigonometry", "cos", 1, "Cosine") {setArgumentDefinition(1, new AngleArgument("", false));}
void CosFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]); 
	if(mngr->isVariable() && mngr->variable() == CALCULATOR->getPI()) {
		mngr->set(1, 1);
		return;
	} else if(mngr->isMultiplication() && mngr->countChilds() == 2 && mngr->getChild(0)->isFraction() && mngr->getChild(1)->isVariable() && mngr->getChild(1)->variable() == CALCULATOR->getPI()) {
		if(mngr->getChild(0)->fraction()->isInteger()) {
			if(mngr->getChild(0)->fraction()->numerator()->isEven()) {
				mngr->set(-1, 1);
			} else {
				mngr->set(1, 1);
			}
			return;
		} else if(!mngr->getChild(0)->fraction()->isComplex()) {
			Fraction fr(mngr->getChild(0)->fraction());
			fr.frac();
			if((fr.numerator()->isOne() || fr.numerator()->isMinusOne()) && fr.denominator()->equals(2)) {
				mngr->clear();
				return;
			}
		}
	}
	mngr->recalculateVariables();
	if(!mngr->isFraction() || !mngr->fraction()->cos()) {
		vargs[0]->recalculateVariables();
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->setPrecise(mngr->fraction()->isPrecise());
	}
}
TanFunction::TanFunction() : Function("Trigonometry", "tan", 1, "Tangent") {setArgumentDefinition(1, new AngleArgument("", false));}
void TanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(tan)
}
SinhFunction::SinhFunction() : Function("Trigonometry", "sinh", 1, "Hyperbolic sine") {setArgumentDefinition(1, new AngleArgument("", false));}
void SinhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(sinh)
}
CoshFunction::CoshFunction() : Function("Trigonometry", "cosh", 1, "Hyperbolic cosine") {setArgumentDefinition(1, new AngleArgument("", false));}
void CoshFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(cosh)
}
TanhFunction::TanhFunction() : Function("Trigonometry", "tanh", 1, "Hyperbolic tangent") {setArgumentDefinition(1, new AngleArgument("", false));}
void TanhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(tanh)
}
AsinFunction::AsinFunction() : Function("Trigonometry", "asin", 1, "Arcsine") {setArgumentDefinition(1, new AngleArgument("", false));}
void AsinFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]); mngr->recalculateVariables();
	if(mngr->isFraction() && mngr->fraction()->isOne()) {
		mngr->set(1, 2);
		Manager mngr2(CALCULATOR->getPI());
		mngr->add(&mngr2, OPERATION_MULTIPLY);
	} else if(mngr->isFraction() && mngr->fraction()->isMinusOne()) {
		mngr->set(-1, 2);
		Manager mngr2(CALCULATOR->getPI());
		mngr->add(&mngr2, OPERATION_MULTIPLY);
	} else if(!mngr->isFraction() || !mngr->fraction()->asin()) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->setPrecise(mngr->fraction()->isPrecise());
	}
}
AcosFunction::AcosFunction() : Function("Trigonometry", "acos", 1, "Arccosine") {setArgumentDefinition(1, new AngleArgument("", false));}
void AcosFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]); mngr->recalculateVariables();
	if(mngr->isZero()) {
		mngr->set(1, 2);
		Manager mngr2(CALCULATOR->getPI());
		mngr->add(&mngr2, OPERATION_MULTIPLY);
		return;
	} else if(mngr->isFraction() && mngr->fraction()->isMinusOne()) {
		mngr->set(CALCULATOR->getPI());
		return;
	} else if(!mngr->isFraction() || !mngr->fraction()->acos()) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->setPrecise(mngr->fraction()->isPrecise());
	}
}
AtanFunction::AtanFunction() : Function("Trigonometry", "atan", 1, "Arctangent") {setArgumentDefinition(1, new AngleArgument("", false));}
void AtanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(atan)
}
AsinhFunction::AsinhFunction() : Function("Trigonometry", "asinh", 1, "Hyperbolic arcsine") {setArgumentDefinition(1, new AngleArgument("", false));}
void AsinhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(asinh)
}
AcoshFunction::AcoshFunction() : Function("Trigonometry", "acosh", 1, "Hyperbolic arccosine") {setArgumentDefinition(1, new AngleArgument("", false));}
void AcoshFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(acosh)
}
AtanhFunction::AtanhFunction() : Function("Trigonometry", "atanh", 1, "Hyperbolic arctangent") {setArgumentDefinition(1, new AngleArgument("", false));}
void AtanhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(atanh)
}
LogFunction::LogFunction() : Function("Exponents and Logarithms", "ln", 1, "Natural Logarithm") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONZERO, true, false));
}
void LogFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(mngr->fraction()->log()) {
			mngr->setPrecise(mngr->fraction()->isPrecise());
			return;
		}
	}
	if(vargs[0]->isVariable() && vargs[0]->variable() == CALCULATOR->getE()) {
		mngr->set(1, 1);
	} else {
		mngr->set(this, vargs[0], NULL);	
	}		
}
LognFunction::LognFunction() : Function("Exponents and Logarithms", "log", 2, "Base-N Logarithm") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONZERO, false, false));
	setArgumentDefinition(2, new FractionArgument("", ARGUMENT_MIN_MAX_POSITIVE, false, false));
}
void LognFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction() && vargs[1]->isFraction()) {
		mngr->set(vargs[0]);
		if(mngr->fraction()->log(vargs[1]->fraction())) {
			mngr->setPrecise(mngr->fraction()->isPrecise());
			return;
		}
	}
	mngr->set(CALCULATOR->getLnFunction(), vargs[0], NULL);
	Manager mngr2(CALCULATOR->getLnFunction(), vargs[1], NULL);
	mngr->add(&mngr2, OPERATION_DIVIDE);		
}
Log10Function::Log10Function() : Function("Exponents and Logarithms", "log10", 1, "Base-10 Logarithm") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONZERO, false, false));
}
void Log10Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->isFraction() || !mngr->fraction()->log10()) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->setPrecise(mngr->fraction()->isPrecise());
	}		
}
Log2Function::Log2Function() : Function("Exponents and Logarithms", "log2", 1, "Base-2 Logarithm") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_POSITIVE, true, false));
}
void Log2Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->isFraction() || !mngr->fraction()->log2()) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->setPrecise(mngr->fraction()->isPrecise());
	}		
}
ExpFunction::ExpFunction() : Function("Exponents and Logarithms", "exp", 1, "e raised to the power X") {}
void ExpFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->exp()) {
			mngr->set(this, vargs[0], NULL);
		} else {
			mngr->setPrecise(mngr->fraction()->isPrecise());
		}		
	} else {
		mngr->fraction()->e();
		mngr->setPrecise(mngr->fraction()->isPrecise());
		mngr->add(vargs[0], OPERATION_RAISE);	
	}
}
Exp10Function::Exp10Function() : Function("Exponents and Logarithms", "exp10", 1, "10 raised the to power X") {}
void Exp10Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->exp10()) {
			mngr->set(this, vargs[0], NULL);
		} else {
			mngr->setPrecise(mngr->fraction()->isPrecise());
		}		
	} else {
		mngr->set(10, 1);
		mngr->add(vargs[0], OPERATION_RAISE);	
	}
}
Exp2Function::Exp2Function() : Function("Exponents and Logarithms", "exp2", 1, "2 raised the to power X") {}
void Exp2Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->exp2()) {
			mngr->set(this, vargs[0], NULL);
		} else {
			mngr->setPrecise(mngr->fraction()->isPrecise());
		}		
	} else {
		mngr->set(2, 1);
		mngr->add(vargs[0], OPERATION_RAISE);	
	}
}
SqrtFunction::SqrtFunction() : Function("Exponents and Logarithms", "sqrt", 1, "Square Root") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, false));
}
void SqrtFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	Manager *mngr2 = new Manager(1, 2);
	mngr->add(mngr2, OPERATION_RAISE);	
	mngr2->unref();
}
AbsSqrtFunction::AbsSqrtFunction() : Function("Exponents and Logarithms", "abssqrt", 1, "Square Root (abs)") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, false));
}
void AbsSqrtFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->sqrt()) {
			mngr->set(this, vargs[0], NULL);
		} else {
			mngr->setPrecise(mngr->fraction()->isPrecise());
		}
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
CbrtFunction::CbrtFunction() : Function("Exponents and Logarithms", "cbrt", 1, "Cube Root") {
	setArgumentDefinition(1, new FractionArgument("", ARGUMENT_MIN_MAX_NONE, false));
}
void CbrtFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->cbrt()) {
			mngr->set(this, vargs[0], NULL);
		} else {
			mngr->setPrecise(mngr->fraction()->isPrecise());
		}		
	} else {
		mngr->set(vargs[0]);
		Manager *mngr2 = new Manager(1, 3);		
		mngr->add(mngr2, OPERATION_RAISE);
		mngr2->unref();	
	}
}
RootFunction::RootFunction() : Function("Exponents and Logarithms", "root", 2, "Nth Root") {}
void RootFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	Manager *mngr2 = new Manager(1, 1);		
	mngr2->add(vargs[1], OPERATION_DIVIDE);
	mngr->add(mngr2, OPERATION_RAISE);
	mngr2->unref();	
}
AbsRootFunction::AbsRootFunction() : Function("Exponents and Logarithms", "absroot", 2, "Nth Root (abs)") {}
void AbsRootFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction() && vargs[1]->isFraction()) {
		mngr->set(vargs[0]);
		Fraction fr(1);
		fr.divide(vargs[1]->fraction());
		if(mngr->fraction()->pow(&fr)) {
			mngr->setPrecise(mngr->fraction()->isPrecise());
			return;
		}		
	} 
	mngr->set(this, vargs[0], vargs[1], NULL);
}
PowFunction::PowFunction() : Function("Exponents and Logarithms", "pow", 2, "Power") {}
void PowFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
/*	if(vargs[0]->isFraction() && vargs[1]->isFraction()) {
		mngr->set(vargs[0]);
		if(mngr->fraction()->pow(vargs[1]->fraction())) {
			mngr->setPrecise(mngr->fraction()->isPrecise());
			return;
		}		
	}*/
	mngr->set(vargs[0]);
	mngr->add(vargs[1], OPERATION_RAISE);
}
HypotFunction::HypotFunction() : Function("Geometry", "hypot", 2, "Hypotenuse") {
}
void HypotFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->addInteger(2, OPERATION_RAISE);
	Manager *mngr2 = new Manager(vargs[1]);
	mngr2->addInteger(2, OPERATION_RAISE);		
	mngr->add(mngr2, OPERATION_RAISE);
	mngr2->unref();
	mngr2 = new Manager(1, 2);
	mngr->add(mngr2, OPERATION_RAISE);
	mngr2->unref();
}
SumFunction::SumFunction() : Function("Statistics", "sum", -1, "Sum") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void SumFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	Vector *v = produceVector(vargs);
	for(unsigned int index = 1; index <= v->components(); index++) {
		mngr->add(v->get(index), OPERATION_ADD);
	}
}
PercentileFunction::PercentileFunction() : Function("Statistics", "percentile", 1, "Percentile", "", -1) {
	FractionArgument *arg = new FractionArgument();
	Fraction fr;
	arg->setMin(&fr);
	fr.set(99, 1);
	arg->setMax(&fr);
	arg->setIncludeEqualsMin(false);
	arg->setIncludeEqualsMax(false);
	setArgumentDefinition(1, arg);
	setArgumentDefinition(2, new VectorArgument("", false));
}
void PercentileFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() < 1) {
		return;
	}
	Fraction fr100(100);
	Vector *v = produceVector(vargs);	
	if(!v->sort()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);
		mngr->set(mngr2);
		mngr2->unref();	
	} else {
		Fraction nfr(v->components() + 1);		
		Fraction pfr(vargs[0]->fraction());		
		pfr.divide(&fr100);
		pfr.multiply(&nfr);
/*		Fraction cfr(v->components());		
		if(pfr.isZero() || pfr.numerator()->isLessThan(pfr.denominator()) || pfr.isGreaterThan(&cfr)) {
			CALCULATOR->error(true, _("Not enough samples."), NULL);
		}*/
		if(pfr.isInteger()) {
			mngr->set(v->get(pfr.numerator()->getInt()));
		} else {
			Fraction ufr(&pfr);
			ufr.ceil();
			Fraction lfr(&pfr);
			lfr.floor();
			pfr.subtract(&lfr);
			Manager gap(v->get(ufr.numerator()->getInt()));
			gap.add(v->get(lfr.numerator()->getInt()), OPERATION_SUBTRACT);
			Manager pfr_mngr(&pfr);
			gap.add(&pfr_mngr, OPERATION_MULTIPLY);
			mngr->set(v->get(lfr.numerator()->getInt()));
			mngr->add(&gap, OPERATION_ADD);
		}
	}
	delete v;
}
MinFunction::MinFunction() : Function("Statistics", "min", -1, "Min") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void MinFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	Vector *v = produceVector(vargs);		
	int cmp;
	Manager *min = NULL;
	for(unsigned int index = 1; index <= v->components(); index++) {
		if(min == NULL) {
			min = v->get(index);
		} else {
			cmp = min->compare(v->get(index));
			if(cmp == -1) {
				min = v->get(index);
			} else if(cmp < -1) {
				CALCULATOR->error(true, _("Unsolvable comparison in %s() ignored."), name().c_str(), NULL);
			}
		}
	}
	mngr->set(min);
	delete v;
}
MaxFunction::MaxFunction() : Function("Statistics", "max", -1, "Max") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void MaxFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	Vector *v = produceVector(vargs);		
	int cmp;
	Manager *max = NULL;
	for(unsigned int index = 1; index <= v->components(); index++) {
		if(max == NULL) {
			max = v->get(index);
		} else {
			cmp = max->compare(v->get(index));
			if(cmp == 1) {
				max = v->get(index);
			} else if(cmp < -1) {
				CALCULATOR->error(true, _("Unsolvable comparison in %s() ignored."), name().c_str(), NULL);
			}
		}
	}
	mngr->set(max);
	delete v;
}
ModeFunction::ModeFunction() : Function("Statistics", "mode", -1, "Mode") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void ModeFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0) {
		return;
	}
	Vector *v = produceVector(vargs);
	int n = 0;
	bool b;
	vector<Manager*> vargs_nodup;
	vector<int> is;
	Manager *value = NULL;
	for(unsigned int index_c = 1; index_c <= v->components(); index_c++) {
		b = true;
		for(unsigned int index = 0; index < vargs_nodup.size(); index++) {
			if(vargs_nodup[index]->equals(v->get(index_c))) {
				is[index]++;
				b = false;
				break;
			}
		}
		if(b) {
			vargs_nodup.push_back(v->get(index_c));
			is.push_back(1);
		}
	}
	for(unsigned int index = 0; index < is.size(); index++) {
		if(is[index] > n) {
			n = is[index];
			value = vargs_nodup[index];
		}
	}
	mngr->set(value);
	delete v;
}
RandomFunction::RandomFunction() : Function("General", "rand", 0, "Random Number") {}
void RandomFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(drand48());
}

BASEFunction::BASEFunction() : Function("General", "BASE", 2, "Number Base") {
	setArgumentDefinition(1, new TextArgument());
	IntegerArgument *arg = new IntegerArgument();
	Integer integ(2);
	arg->setMin(&integ);
	integ.set(36);
	arg->setMax(&integ);
	setArgumentDefinition(2, arg);
}
void BASEFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str = vargs[0]->text();
	remove_blanks(str);
	mngr->set(strtol(str.c_str(), NULL, vargs[1]->fraction()->numerator()->getInt()), 1);
}
BINFunction::BINFunction() : Function("General", "BIN", 1, "Binary") {
	setArgumentDefinition(1, new TextArgument());
}
void BINFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str = vargs[0]->text();
	remove_blanks(str);
	mngr->set(strtol(str.c_str(), NULL, 2), 1);
}
OCTFunction::OCTFunction() : Function("General", "OCT", 1, "Octal") {
	setArgumentDefinition(1, new TextArgument());
}
void OCTFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str = vargs[0]->text();
	remove_blanks(str);
	mngr->set(strtol(str.c_str(), NULL, 8), 1);
}
HEXFunction::HEXFunction() : Function("General", "HEX", 1, "Hexadecimal") {
	setArgumentDefinition(1, new TextArgument());
}
void HEXFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
/*	string expr = vargs[0]->text();
	remove_blanks(expr);
	if(!(expr.length() >= 2 && expr[0] == '0' && (expr[1] == 'x' || expr[1] == 'X'))) {
		expr.insert(0, "0x");
	}
	mngr->set(strtold(expr.c_str(), NULL));*/
	string str = vargs[0]->text();
	remove_blanks(str);
	mngr->set(strtol(str.c_str(), NULL, 16), 1);
}
TitleFunction::TitleFunction() : Function("Utilities", "title", 1, "Object title") {
	setArgumentDefinition(1, new ExpressionItemArgument());
}
void TitleFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	ExpressionItem *item = CALCULATOR->getExpressionItem(vargs[0]->text());
	if(!item) {
		CALCULATOR->error(true, _("Object %s does not exist."), vargs[0]->text().c_str(), NULL);
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->set(item->title());
	}
}
SaveFunction::SaveFunction() : Function("Utilities", "save", 2, "Save as variable", "", 4) {
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());	
	setArgumentDefinition(4, new TextArgument());		
	setDefaultValue(3, "\"Temporary\"");
	setDefaultValue(4, "\"\"");	
}
void SaveFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	CALCULATOR->addVariable(new Variable(vargs[2]->text(), vargs[1]->text(), vargs[0], vargs[3]->text()));
}
ConcatenateFunction::ConcatenateFunction() : Function("Utilities", "concatenate", 1, "Concatenate strings", "", -1) {
	setArgumentDefinition(1, new TextArgument());
	setArgumentDefinition(2, new TextArgument());	
}
void ConcatenateFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		str += vargs[i]->text();
	}
	mngr->set(str);
}
LengthFunction::LengthFunction() : Function("Utilities", "len", 1, "Length of string") {
	setArgumentDefinition(1, new TextArgument());
}
void LengthFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]->text().length());
}

#ifdef HAVE_GIAC
GiacFunction::GiacFunction() : Function("CAS", "giac", 1, "Giac expression") {
	setArgumentDefinition(1, new TextArgument());
}
void GiacFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	try {
		giac::gen v1(vargs[0]->text());
		mngr->set(simplify(v1));
		mngr->clean();
	} catch(std::runtime_error & err){
		CALCULATOR->error(true, "Giac error: %s.", err.what(), NULL);
		mngr->set(this, vargs[0], NULL);
		return;
	}
}
SolveFunction::SolveFunction() : Function("CAS", "solve", 1, "Solve equation", "", 2) {
	setArgumentDefinition(1, new GiacArgument());
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");		
}
void SolveFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	bool failed = false;
	giac::gen v1 = vargs[0]->toGiac(&failed);
	if(failed) {
		CALCULATOR->error(true, _("Conversion to Giac failed."), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
	giac::identificateur id(vargs[1]->text());
	try {
		giac::vecteur v = giac::solve(v1, id);
		if(v.size() < 1) {
			CALCULATOR->error(false, _("No solution could be found."), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);
		} else {
			v[0] = simplify(v[0]);
			mngr->set(v[0]);
			for(unsigned int i = 1; i < v.size(); i++) {
				v[1] = simplify(v[1]);
				Manager alt_mngr(v[1]);
				mngr->addAlternative(&alt_mngr);
			}
		}
		mngr->clean();
	} catch(std::runtime_error & err){
		CALCULATOR->error(true, "Giac error: %s.", err.what(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
}
DeriveFunction::DeriveFunction() : Function("CAS", "diff", 1, "Derive", "", 3) {
	setArgumentDefinition(1, new GiacArgument());
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");
	setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(3, "1");		
}
void DeriveFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	bool failed = false;
	giac::gen v1 = vargs[0]->toGiac(&failed);
	if(failed) {
		CALCULATOR->error(true, _("Conversion to Giac failed."), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
		return;
	}
	giac::identificateur id(vargs[1]->text());
	giac::gen vars = id;
	giac::gen nderiv = vargs[2]->toGiac();
	try {
		giac::gen ans = giac::derive(v1, vars, nderiv);
		ans = simplify(ans);
		mngr->set(ans);
		mngr->clean();
	} catch(std::runtime_error & err){
		CALCULATOR->error(true, "Giac error: %s.", err.what(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
		return;
	}
}
IntegrateFunction::IntegrateFunction() : Function("CAS", "integrate", 1, "Integrate", "", 2) {
	setArgumentDefinition(1, new GiacArgument());
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");
}
void IntegrateFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	bool failed = false;
	giac::gen v1 = vargs[0]->toGiac(&failed);
	if(failed) {
		CALCULATOR->error(true, _("Conversion to Giac failed."), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
	giac::identificateur id(vargs[1]->text());
	try {
    		giac::gen ans = giac::integrate(v1, id);
		ans = simplify(ans);
		mngr->set(ans);
		mngr->clean();
	} catch(std::runtime_error & err){
		CALCULATOR->error(true, "Giac error: %s.", err.what(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
}
#endif
LoadFunction::LoadFunction() : Function("Utilities", "load", 1, "Load CSV file", "", 3) {
	setArgumentDefinition(1, new FileArgument());
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(2, "1");
	setArgumentDefinition(3, new TextArgument());
	setDefaultValue(3, ",");	
}
void LoadFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[2]->text() == "tab") {
		vargs[2]->set("\t");
	}
	Matrix *mtrx = CALCULATOR->importCSV(vargs[0]->text().c_str(), vargs[1]->fraction()->numerator()->getInt(), vargs[2]->text());
	if(!mtrx) {
		CALCULATOR->error(true, "Failed to load %s.", vargs[0]->text().c_str(), NULL);
	}
	mngr->set(mtrx);
}
