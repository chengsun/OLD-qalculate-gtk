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
#include "Number.h"
#include "Calculator.h"
#include "Variable.h"

#include <sstream>

#define TRIG_FUNCTION(FUNC)	mngr->set(vargs[0]); mngr->recalculateVariables(); if(!mngr->isNumber() || !mngr->number()->FUNC()) {mngr->set(this, vargs[0], NULL);} else {mngr->setPrecise(!mngr->number()->isApproximate());}
#define FR_FUNCTION(FUNC)	mngr->set(vargs[0]); if(!mngr->number()->FUNC()) {mngr->set(this, vargs[0], NULL);} else {mngr->setPrecise(!mngr->number()->isApproximate());}
#define FR_FUNCTION_2(FUNC)	mngr->set(vargs[0]); if(!mngr->number()->FUNC(vargs[1]->number())) {mngr->set(this, vargs[0], vargs[1], NULL);} else {mngr->setPrecise(!mngr->number()->isApproximate());}

#define NON_COMPLEX_NUMBER_ARGUMENT(i)				NumberArgument *arg_non_complex##i = new NumberArgument(); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);
#define NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(i)			NumberArgument *arg_non_complex##i = new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);
#define NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(i)		NumberArgument *arg_non_complex##i = new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, true, false); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);



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

ForFunction::ForFunction() : Function("Logical", "for", 5, "for...do") {
	argoccs[1] = -1;
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());	
	argoccs[4] = -1;
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
	Manager *action_mngr_pre = CALCULATOR->calculate_sub(action, false);
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
	
	mngr->clear();

	Manager *action_mngr;
	Manager *cur_mngr = mngr;
	unsigned int count = 1;
	if(action_mngr_pre->isAlternatives()) {
		count = action_mngr_pre->countChilds();
	}
	for(unsigned int i2 = 0; i2 < count; i2++) {
		if(action_mngr_pre->isAlternatives()) {
			if(i2 > 0) {
				Manager mngr2;
				mngr->addAlternative(&mngr2);
				cur_mngr = mngr->getChild(i2);
				i_mngr.set(1, 1);
				x_mngr.set(vargs[0]);
				y_mngr.set(vargs[3]);
			}
			action_mngr = action_mngr_pre->getChild(i2);
		} else {
			action_mngr = action_mngr_pre;
		}

		
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
			calced = CALCULATOR->calculate_sub(counter, false);
			x_mngr.set(calced);
			calced->unref();
			i++;
			i_mngr.set(i, 1);
		}
		cur_mngr->set(&y_mngr);
		
	}
	action_mngr_pre->unref();
	CALCULATOR->delId(x_id, true);
	CALCULATOR->delId(y_id, true);
	CALCULATOR->delId(i_id, true);

}
SumFunction::SumFunction() : Function("Algebra", "sum", 3, "Sum") {
	setArgumentDefinition(1, new IntegerArgument());
	setArgumentDefinition(2, new IntegerArgument());	
	setArgumentDefinition(3, new TextArgument());
	setCondition("\\y >= \\x");
}
void SumFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	string action = vargs[2]->text();

	Manager mngr_i("\\i");
	Manager mngr_j("\\j");
	Manager mngr_k("\\k");

	int i_id = CALCULATOR->addId(&mngr_i, true);
	string str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(i_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\i", str, action);		
	int j_id = CALCULATOR->addId(&mngr_j, true);
	str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(j_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\j", str, action);
	int k_id = CALCULATOR->addId(&mngr_k, true);
	str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(k_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\k", str, action);
	
	
	CALCULATOR->beginTemporaryStopErrors();
	Manager *action_mngr_pre = CALCULATOR->calculate_sub(action, false);
	CALCULATOR->endTemporaryStopErrors();	

	mngr->clear();

	Manager *action_mngr;
	Manager *cur_mngr = mngr;
	unsigned int count = 1;
	if(action_mngr_pre->isAlternatives()) {
		count = action_mngr_pre->countChilds();
	}
	for(unsigned int i = 0; i < count; i++) {
		if(action_mngr_pre->isAlternatives()) {
			if(i > 0) {
				Manager mngr2;
				mngr->addAlternative(&mngr2);
				cur_mngr = mngr->getChild(i);
			}
			action_mngr = action_mngr_pre->getChild(i);
		} else {
			action_mngr = action_mngr_pre;
		}

		Manager i_mngr(vargs[0]->number());
		Manager mngr_calc;
	
		while(i_mngr.number()->isLessThanOrEqualTo(vargs[1]->number())) {	
			mngr_calc.set(action_mngr);
			mngr_calc.replace(&mngr_i, &i_mngr);
			mngr_calc.replace(&mngr_j, &mngr_i);
			mngr_calc.replace(&mngr_k, &mngr_j);
			mngr_calc.recalculateFunctions();
			mngr_calc.clean();
			cur_mngr->add(&mngr_calc, OPERATION_ADD);		
			i_mngr.number()->add(1, 1);
		}
	}
	action_mngr_pre->unref();
	CALCULATOR->delId(i_id, true);
	CALCULATOR->delId(j_id, true);
	CALCULATOR->delId(k_id, true);
}
ProductFunction::ProductFunction() : Function("Algebra", "product", 3, "Product") {
	setArgumentDefinition(1, new IntegerArgument());
	setArgumentDefinition(2, new IntegerArgument());	
	setArgumentDefinition(3, new TextArgument());
	setCondition("\\y >= \\x");
}
void ProductFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	string action = vargs[2]->text();

	Manager mngr_i("\\i");
	Manager mngr_j("\\j");
	Manager mngr_k("\\k");

	int i_id = CALCULATOR->addId(&mngr_i, true);
	string str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(i_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\i", str, action);		
	int j_id = CALCULATOR->addId(&mngr_j, true);
	str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(j_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\j", str, action);
	int k_id = CALCULATOR->addId(&mngr_k, true);
	str = LEFT_PARENTHESIS;
	str += ID_WRAP_LEFT;
	str += i2s(k_id);
	str += ID_WRAP_RIGHT;
	str += RIGHT_PARENTHESIS;
	gsub("\\k", str, action);
	
	
	CALCULATOR->beginTemporaryStopErrors();
	Manager *action_mngr_pre = CALCULATOR->calculate_sub(action, false);
	CALCULATOR->endTemporaryStopErrors();	

	mngr->clear();

	Manager *action_mngr;
	Manager *cur_mngr = mngr;
	unsigned int count = 1;
	if(action_mngr_pre->isAlternatives()) {
		count = action_mngr_pre->countChilds();
	}
	for(unsigned int i = 0; i < count; i++) {
		if(action_mngr_pre->isAlternatives()) {
			if(i > 0) {
				Manager mngr2;
				mngr->addAlternative(&mngr2);
				cur_mngr = mngr->getChild(i);
			}
			action_mngr = action_mngr_pre->getChild(i);
		} else {
			action_mngr = action_mngr_pre;
		}
		Manager i_mngr(vargs[0]->number());
		Manager mngr_calc;
		bool started = false;
		while(i_mngr.number()->isLessThanOrEqualTo(vargs[1]->number())) {	
			mngr_calc.set(action_mngr);
			mngr_calc.replace(&mngr_i, &i_mngr);
			mngr_calc.replace(&mngr_j, &mngr_i);
			mngr_calc.replace(&mngr_k, &mngr_j);
			mngr_calc.recalculateFunctions();
			mngr_calc.clean();
			if(started) {
				cur_mngr->add(&mngr_calc, OPERATION_MULTIPLY);
			} else {
				cur_mngr->add(&mngr_calc, OPERATION_ADD);
				started = true;
			}
			i_mngr.number()->add(1, 1);
		}
	}
	action_mngr_pre->unref();
	CALCULATOR->delId(i_id, true);
	CALCULATOR->delId(j_id, true);
	CALCULATOR->delId(k_id, true);
	
}

ProcessFunction::ProcessFunction() : Function("Utilities", "process", 1, "Process components", "", -1) {
	setArgumentDefinition(1, new TextArgument("", false));
	setArgumentDefinition(2, new MatrixArgument("", false));
	argoccs[2] = -1;
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
	Manager *sarg_mngr_pre = CALCULATOR->calculate_sub(sarg, false);
	CALCULATOR->endTemporaryStopErrors();	

	Manager *sarg_mngr;
	Manager *cur_mngr = mngr;
	unsigned int count = 1;
	if(sarg_mngr_pre->isAlternatives()) {
		count = sarg_mngr_pre->countChilds();
	}
	for(unsigned int i2 = 0; i2 < count; i2++) {
		if(sarg_mngr_pre->isAlternatives()) {
			if(i2 > 0) {
				Manager mngr2;
				mngr->addAlternative(&mngr2);
				cur_mngr = mngr->getChild(i2);
			}
			sarg_mngr = sarg_mngr_pre->getChild(i2);
		} else {
			sarg_mngr = sarg_mngr_pre;
		}

	
		if(vargs.size() == 1) {
			break;
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
			cur_mngr->set(v);
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
			cur_mngr->set(mtrx);
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
			cur_mngr->set(sarg_mngr);				
		}
	}
	sarg_mngr_pre->unref();
}

CustomSumFunction::CustomSumFunction() : Function("Utilities", "csum", 4, "Custom sum of components", "", -1) {
	setArgumentDefinition(1, new IntegerArgument());
	setArgumentDefinition(2, new IntegerArgument());
	setArgumentDefinition(4, new TextArgument());
	setArgumentDefinition(5, new VectorArgument("", false));
	argoccs[3] = -1;
	argoccs[5] = -1;
}
void CustomSumFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	int start = 1;
	int end = -1;
	start = vargs[0]->number()->intValue();
	if(start < 1) start = 1;
	end = vargs[1]->number()->intValue();

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
	Manager *sarg_mngr_pre = CALCULATOR->calculate_sub(sarg, false);
	CALCULATOR->endTemporaryStopErrors();	

	Manager *sarg_mngr;
	Manager *cur_mngr = mngr;
	unsigned int count = 1;
	if(sarg_mngr_pre->isAlternatives()) {
		count = sarg_mngr_pre->countChilds();
	}
	for(unsigned int i2 = 0; i2 < count; i2++) {
		if(sarg_mngr_pre->isAlternatives()) {
			if(i2 > 0) {
				Manager mngr2;
				mngr->addAlternative(&mngr2);
				cur_mngr = mngr->getChild(i2);
			}
			sarg_mngr = sarg_mngr_pre->getChild(i2);
		} else {
			sarg_mngr = sarg_mngr_pre;
		}


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
		cur_mngr->set(y_mngr);
		y_mngr->unref();	
	}
	sarg_mngr_pre->unref();
}

FunctionFunction::FunctionFunction() : Function("Utilities", "function", 1, "Function", "", -1) {
	setArgumentDefinition(1, new TextArgument());
	argoccs[2] = -1;
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
	Matrix mtrx(vargs[0]->number()->intValue(), vargs[1]->number()->intValue());
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
	argoccs[1] = -1;
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
	argoccs[1] = -1;
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
	Vector *v = vargs[1]->matrix()->rowToVector(vargs[0]->number()->intValue());
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
	Vector *v = vargs[1]->matrix()->columnToVector(vargs[0]->number()->intValue());
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
		mngr->set(vargs[2]->matrix()->get(vargs[0]->number()->intValue(), vargs[1]->number()->intValue()));
	} else if(vargs[0]->number()->isOne() && vargs[1]->number()->isOne()) {
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
		mngr->set(vargs[1]->matrix()->get((vargs[0]->number()->intValue() - 1) / vargs[1]->matrix()->columns() + 1, (vargs[0]->number()->intValue() - 1) % vargs[1]->matrix()->columns() + 1));
	} else if(vargs[0]->number()->isOne()) {
		mngr->set(vargs[1]);
	}
}
RangeFunction::RangeFunction() : Function("Matrices", "range", 3, "Range") {
	argoccs[1] = -1;
	argoccs[2] = -1;
	argoccs[3] = -1;
}
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
	setArgumentDefinition(1, new IntegerArgument(""));
	setArgumentDefinition(2, new IntegerArgument(""));	
	setArgumentDefinition(3, new VectorArgument("", false));	
}
void LimitsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int i = vargs[0]->number()->intValue(), n = vargs[1]->number()->intValue();	
	Vector *v = produceVector(vargs);
	Vector *vctr = v->getRange(i, n);
	mngr->set(vctr);
	delete vctr;
	delete v;
}
AreaFunction::AreaFunction() : Function("Matrices", "area", 5, "Area") {
	setArgumentDefinition(1, new IntegerArgument(""));
	setArgumentDefinition(2, new IntegerArgument(""));	
	setArgumentDefinition(3, new IntegerArgument(""));
	setArgumentDefinition(4, new IntegerArgument(""));	
	setArgumentDefinition(5, new MatrixArgument(""));	
}
void AreaFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Matrix *mtrx = vargs[4]->matrix()->getArea(vargs[0]->number()->intValue(), vargs[1]->number()->intValue(), vargs[2]->number()->intValue(), vargs[3]->number()->intValue());
	mngr->set(mtrx);
	delete mtrx;
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
	} else if(vargs[0]->isNumber() && vargs[0]->number()->isInteger() && vargs[0]->number()->isPositive()) {
		Matrix mtrx;
		mtrx.setToIdentityMatrix(vargs[0]->number()->intValue());
		mngr->set(&mtrx);
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
DeterminantFunction::DeterminantFunction() : Function("Matrices", "det", 1, "Determinant") {
	setArgumentDefinition(1, new MatrixArgument());
	argoccs[1] = -1;
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
PermanentFunction::PermanentFunction() : Function("Matrices", "permanent", 1, "Permanent") {
	setArgumentDefinition(1, new MatrixArgument());
	argoccs[1] = -1;
}
void PermanentFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Manager *per = vargs[0]->matrix()->permanent();
	if(!per) {
		mngr->set(this, vargs[0], NULL);
		return;
	}
	mngr->set(per);
	per->unref();	
}
CofactorFunction::CofactorFunction() : Function("Matrices", "cofactor", 3, "Cofactor") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(3, new MatrixArgument());
}
void CofactorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Manager *mngr2 = vargs[2]->matrix()->cofactor(vargs[0]->number()->intValue(), vargs[1]->number()->intValue());
	if(!mngr2) {
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
		return;
	}
	mngr->set(mngr2);
	mngr2->unref();	
}
AdjointFunction::AdjointFunction() : Function("Matrices", "adj", 1, "Adjoint") {
	setArgumentDefinition(1, new MatrixArgument());
	argoccs[1] = -1;
}
void AdjointFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->matrix()->adjoint()) {
		mngr->set(this, vargs[0], NULL);
	}
}
InverseFunction::InverseFunction() : Function("Matrices", "inverse", 1, "Inverse") {
	setArgumentDefinition(1, new MatrixArgument());
	argoccs[1] = -1;
}
void InverseFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->matrix()->inverse()) {
		mngr->set(this, vargs[0], NULL);
	}
}
IFFunction::IFFunction() : Function("Logical", "if", 3, "If...Then...Else") {
	NON_COMPLEX_NUMBER_ARGUMENT(1)
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());
}
void IFFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int result = vargs[0]->number()->getBoolean();
	if(result) {			
		Manager *mngr2 = CALCULATOR->calculate_sub(vargs[1]->text());
		mngr->set(mngr2);
		mngr2->unref();		
	} else if(result == 0) {			
		Manager *mngr2 = CALCULATOR->calculate_sub(vargs[2]->text());		
		mngr->set(mngr2);
		mngr2->unref();		
	} else {
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
	}	
}
GCDFunction::GCDFunction() : Function("Arithmetics", "gcd", 2, "Greatest Common Divisor") {
	setArgumentDefinition(1, new IntegerArgument());
	setArgumentDefinition(2, new IntegerArgument());
}
void GCDFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->number()->gcd(vargs[1]->number());
	mngr->setPrecise(!mngr->number()->isApproximate());
}
DaysFunction::DaysFunction() : Function("Date & Time", "days", 2, "Days between two dates", "", 4) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new DateArgument());	
	IntegerArgument *arg = new IntegerArgument();
	Number integ;
	arg->setMin(&integ);
	integ.set(4, 1);
	arg->setMax(&integ);
	setArgumentDefinition(3, arg);	
	setArgumentDefinition(4, new BooleanArgument());				
	setDefaultValue(3, "1"); 
}
void DaysFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int days = daysBetweenDates(vargs[0]->text(), vargs[1]->text(), vargs[2]->number()->intValue(), vargs[3]->number()->isZero());
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
	Number integ;
	arg->setMin(&integ);
	integ.set(4, 1);
	arg->setMax(&integ);
	setArgumentDefinition(3, arg);	
	setArgumentDefinition(4, new BooleanArgument());		
	setDefaultValue(3, "1");
}
void YearFracFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Number *fr = yearsBetweenDates(vargs[0]->text(), vargs[1]->text(), vargs[2]->number()->intValue(), vargs[3]->number()->isZero());
	if(!fr) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], vargs[3], NULL);			
	} else {
		mngr->set(fr);
	}
	delete fr;
}
WeekFunction::WeekFunction() : Function("Date & Time", "week", 0, "Week of Year", "", 2) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new BooleanArgument());	
	setDefaultValue(1, "\"today\"");
}
void WeekFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int w = week(vargs[0]->text(), vargs[1]->number()->getBoolean());
	if(w < 0) {
		mngr->set(this, vargs[0], vargs[1], NULL);
	} else {
		mngr->set(w, 1);
	}
}
WeekdayFunction::WeekdayFunction() : Function("Date & Time", "weekday", 0, "Day of Week", "", 2) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new BooleanArgument());
	setDefaultValue(1, "\"today\"");
}
void WeekdayFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int w = weekday(vargs[0]->text());
	if(w < 0) {
		mngr->set(this, vargs[0], vargs[1], NULL);
	} else {
		if(vargs[1]->number()->getBoolean()) {
			if(w == 7) w = 1;
			else w++;
		}
		mngr->set(w, 1);
	}
}
YeardayFunction::YeardayFunction() : Function("Date & Time", "yearday", 0, "Day of Year", "", 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
void YeardayFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int d = yearday(vargs[0]->text());
	if(d < 0) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->set(d, 1);
	}
}
MonthFunction::MonthFunction() : Function("Date & Time", "month", 0, "Month", "", 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
void MonthFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int year, month, day;
	bool b = s2date(vargs[0]->text(), year, month, day);
	if(!b) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->set(month, 1);
	}
}
DayFunction::DayFunction() : Function("Date & Time", "day", 0, "Day of Month", "", 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
void DayFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int year, month, day;
	bool b = s2date(vargs[0]->text(), year, month, day);
	if(!b) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->set(day, 1);
	}
}
YearFunction::YearFunction() : Function("Date & Time", "year", 0, "Year", "", 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
void YearFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int year, month, day;
	bool b = s2date(vargs[0]->text(), year, month, day);
	if(!b) {
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->set(year, 1);
	}
}
TimeFunction::TimeFunction() : Function("Date & Time", "time", 0, "Current Time", "") {
}
void TimeFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int hour, min, sec;
	now(hour, min, sec);
	mngr->set(sec, 1);
	mngr->number()->divide(60);
	mngr->number()->add(min);
	mngr->number()->divide(60);
	mngr->number()->add(hour);
}
FactorialFunction::FactorialFunction() : Function("Arithmetics", "factorial", 1, "Factorial") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE, true, false));
}
void FactorialFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->number()->factorial()) {
		mngr->set(this, vargs[0], NULL);
	}
}
BinomialFunction::BinomialFunction() : Function("Arithmetics", "binomial", 2, "Binomial") {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE, true, true));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE, true, true));
	setCondition("\\x>=\\y");
}
void BinomialFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->clear();
	if(!mngr->number()->binomial(vargs[0]->number(), vargs[1]->number())) {
		mngr->set(this, vargs[0], vargs[1], NULL);
	}
}
AbsFunction::AbsFunction() : Function("Arithmetics", "abs", 1, "Absolute Value") {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void AbsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->number()->abs()) {
		mngr->set(this, vargs[0], NULL);	
	}
	mngr->setPrecise(!mngr->number()->isApproximate());
}
SignumFunction::SignumFunction() : Function("Arithmetics", "sgn", 1, "Signum") {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void SignumFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	if(!mngr->number()->signum()) {
		mngr->set(this, vargs[0], NULL);	
	}
	mngr->setPrecise(!mngr->number()->isApproximate());
}
CeilFunction::CeilFunction() : Function("Arithmetics", "ceil", 1, "Round upwards") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
void CeilFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->number()->ceil();
	mngr->setPrecise(!mngr->number()->isApproximate());
}
FloorFunction::FloorFunction() : Function("Arithmetics", "floor", 1, "Round downwards") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
void FloorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->number()->floor();
	mngr->setPrecise(!mngr->number()->isApproximate());
}
TruncFunction::TruncFunction() : Function("Arithmetics", "trunc", 1, "Round towards zero") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
void TruncFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->number()->trunc();
	mngr->setPrecise(!mngr->number()->isApproximate());
}
RoundFunction::RoundFunction() : Function("Arithmetics", "round", 1, "Round") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
void RoundFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->number()->round();
	mngr->setPrecise(!mngr->number()->isApproximate());
}
FracFunction::FracFunction() : Function("Arithmetics", "frac", 1, "Extract numberal part") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
void FracFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->number()->frac();
	mngr->setPrecise(!mngr->number()->isApproximate());
}
ImaginaryPartFunction::ImaginaryPartFunction() : Function("Analysis", "im", 1, "Imaginary Part") {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void ImaginaryPartFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Number *imag = vargs[0]->number()->imaginaryPart();
	mngr->set(imag);
	delete imag;
	mngr->setPrecise(!mngr->number()->isApproximate());
}
RealPartFunction::RealPartFunction() : Function("Analysis", "re", 1, "Real Part") {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
void RealPartFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Number *real = vargs[0]->number()->realPart();
	mngr->set(real);
	delete real;
	mngr->setPrecise(!mngr->number()->isApproximate());
}
IntFunction::IntFunction() : Function("Arithmetics", "int", 1, "Extract integer part") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
void IntFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->number()->trunc();
	mngr->setPrecise(!mngr->number()->isApproximate());
}
RemFunction::RemFunction() : Function("Arithmetics", "rem", 2, "Reminder (rem)") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(2)
}
void RemFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);	
	mngr->number()->rem(vargs[1]->number());
	mngr->setPrecise(!mngr->number()->isApproximate());
}
ModFunction::ModFunction() : Function("Arithmetics", "mod", 2, "Reminder (mod)") {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(2)
}
void ModFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);	
	mngr->number()->mod(vargs[1]->number());
	mngr->setPrecise(!mngr->number()->isApproximate());
}

/*void SinFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]); 
	if(mngr->isVariable() && mngr->variable() == CALCULATOR->getPI()) {
		mngr->clear();
		return;
	} else if(mngr->isMultiplication() && mngr->countChilds() == 2 && mngr->getChild(0)->isNumber() && mngr->getChild(1)->isVariable() && mngr->getChild(1)->variable() == CALCULATOR->getPI()) {
		if(mngr->getChild(0)->number()->isInteger()) {
			mngr->clear();
			return;
		} else if(!mngr->getChild(0)->number()->isComplex()) {
			if(mngr->getChild(0)->number()->equals(1, 2)) {
				mngr->set(1, 1);
				return;
			}
			if(mngr->getChild(0)->number()->equals(-1, 2)) {
				mngr->set(-1, 1);
				return;
			}
			if(mngr->getChild(0)->number()->equals(1, 6)) {
				mngr->set(1, 2);
				return;
			}
			if(mngr->getChild(0)->number()->equals(-1, 6)) {
				mngr->set(-1, 2);
				return;
			}
		}
	}
	mngr->recalculateVariables();
	if(!mngr->isNumber() || !mngr->number()->sin()) {
		vargs[0]->recalculateVariables();
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->setPrecise(!mngr->number()->isApproximate());
	}
}*/
LogFunction::LogFunction() : Function("Exponents and Logarithms", "ln", 1, "Natural Logarithm") {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false, false));
}
void LogFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isNumber()) {
		if(vargs[0]->number()->isMinusOne()) {
			mngr->clear();
			Number cmplx(1, 1);
			mngr->number()->setImaginaryPart(&cmplx);
			Manager mngr2(CALCULATOR->getPI());
			mngr->add(&mngr2, OPERATION_MULTIPLY);
			return;
		}
		if(vargs[0]->number()->isComplex() && !vargs[0]->number()->hasRealPart()) {
			if(vargs[0]->number()->isI() || vargs[0]->number()->isMinusI()) {
				mngr->set(vargs[0]);
				mngr->addInteger(2, OPERATION_DIVIDE);
				Manager mngr2(CALCULATOR->getPI());
				mngr->add(&mngr2, OPERATION_MULTIPLY);
				return;
			}
		}
		mngr->set(vargs[0]);
		if(mngr->number()->ln()) {
			mngr->setPrecise(!mngr->number()->isApproximate());
			return;
		}
	}
	if(vargs[0]->isVariable() && vargs[0]->variable() == CALCULATOR->getE()) {
		mngr->set(1, 1);
		return;
	} else if(vargs[0]->isPower() && vargs[0]->base()->isVariable() && vargs[0]->base()->variable() == CALCULATOR->getE() && vargs[0]->exponent()->isNumber()) {
		if(!vargs[0]->exponent()->number()->isComplex()) {
			mngr->set(vargs[0]->exponent());
			return;
		} else {
			Number pi_nr(CALCULATOR->getPI()->get()->number());
			Number *img_part = vargs[0]->exponent()->number()->imaginaryPart();
			if(img_part->isLessThanOrEqualTo(&pi_nr)) {
				pi_nr.negate();
				if(img_part->isGreaterThan(&pi_nr)) {
					mngr->set(vargs[0]->exponent());
					delete img_part;
					return;
				}
			}
			delete img_part;
		}
	}
	mngr->set(this, vargs[0], NULL);			
}
LognFunction::LognFunction() : Function("Exponents and Logarithms", "log", 1, "Base-N Logarithm", "", 2) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false, false));
	setArgumentDefinition(2, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false, false));
	setDefaultValue(2, "e");
}
void LognFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isNumber() && vargs[1]->isNumber()) {
		mngr->set(vargs[0]);
		if(mngr->number()->log(vargs[1]->number())) {
			mngr->setPrecise(!mngr->number()->isApproximate());
			return;
		}
	} else if(vargs[1]->isVariable() && vargs[1]->variable() == CALCULATOR->getE()) {
		if(vargs[0]->isNumber()) {
			if(vargs[0]->number()->isMinusOne()) {
				mngr->clear();
				Number cmplx(1, 1);
				mngr->number()->setImaginaryPart(&cmplx);
				Manager mngr2(CALCULATOR->getPI());
				mngr->add(&mngr2, OPERATION_MULTIPLY);
				return;
			}
			if(vargs[0]->number()->isComplex() && !vargs[0]->number()->hasRealPart()) {
				if(vargs[0]->number()->isI() || vargs[0]->number()->isMinusI()) {
					mngr->set(vargs[0]);
					mngr->addInteger(2, OPERATION_DIVIDE);
					Manager mngr2(CALCULATOR->getPI());
					mngr->add(&mngr2, OPERATION_MULTIPLY);
					return;
				}
			}
			mngr->set(vargs[0]);
			if(mngr->number()->ln()) {
				mngr->setPrecise(!mngr->number()->isApproximate());
				return;
			}
		}
		if(vargs[0]->isVariable() && vargs[0]->variable() == CALCULATOR->getE()) {
			mngr->set(1, 1);
			return;
		} else if(vargs[0]->isPower() && vargs[0]->base()->isVariable() && vargs[0]->base()->variable() == CALCULATOR->getE() && vargs[0]->exponent()->isNumber()) {
			if(!vargs[0]->exponent()->number()->isComplex()) {
				mngr->set(vargs[0]->exponent());
				return;
			} else {
				Number pi_nr(CALCULATOR->getPI()->get()->number());
				Number *img_part = vargs[0]->exponent()->number()->imaginaryPart();
				if(img_part->isLessThanOrEqualTo(&pi_nr)) {
					pi_nr.negate();
					if(img_part->isGreaterThan(&pi_nr)) {
						mngr->set(vargs[0]->exponent());
						delete img_part;
						return;
					}
				}
				delete img_part;
			}
		}
		mngr->set(CALCULATOR->getLnFunction(), vargs[0], NULL);
	}
	mngr->set(CALCULATOR->getLnFunction(), vargs[0], NULL);
	Manager mngr2(CALCULATOR->getLnFunction(), vargs[1], NULL);
	mngr->add(&mngr2, OPERATION_DIVIDE);		
}
TotalFunction::TotalFunction() : Function("Statistics", "total", -1, "Sum (total)") {
	setArgumentDefinition(1, new VectorArgument("", false));
}
void TotalFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	Vector *v = produceVector(vargs);
	for(unsigned int index = 1; index <= v->components(); index++) {
		mngr->add(v->get(index), OPERATION_ADD);
	}
}
PercentileFunction::PercentileFunction() : Function("Statistics", "percentile", 1, "Percentile", "", -1) {
	NumberArgument *arg = new NumberArgument();
	Number fr;
	arg->setMin(&fr);
	fr.set(99, 1);
	arg->setMax(&fr);
	arg->setIncludeEqualsMin(false);
	arg->setIncludeEqualsMax(false);
	setArgumentDefinition(1, arg);
	setArgumentDefinition(2, new VectorArgument("", false));
	argoccs[1] = -1;
	argoccs[2] = -1;
}
void PercentileFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() < 1) {
		return;
	}
	Number fr100(100);
	Vector *v = produceVector(vargs);	
	if(!v->sort()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);
		mngr->set(mngr2);
		mngr2->unref();	
	} else {
		Number nfr(v->components() + 1);		
		Number pfr(vargs[0]->number());		
		pfr.divide(&fr100);
		pfr.multiply(&nfr);
/*		Number cfr(v->components());		
		if(pfr.isZero() || pfr.numerator()->isLessThan(pfr.denominator()) || pfr.isGreaterThan(&cfr)) {
			CALCULATOR->error(true, _("Not enough samples."), NULL);
		}*/
		if(pfr.isInteger()) {
			mngr->set(v->get(pfr.intValue()));
		} else {
			Number ufr(&pfr);
			ufr.ceil();
			Number lfr(&pfr);
			lfr.floor();
			pfr.subtract(&lfr);
			Manager gap(v->get(ufr.intValue()));
			gap.add(v->get(lfr.intValue()), OPERATION_SUBTRACT);
			Manager pfr_mngr(&pfr);
			gap.add(&pfr_mngr, OPERATION_MULTIPLY);
			mngr->set(v->get(lfr.intValue()));
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
				if(CALCULATOR->showArgumentErrors()) {
					CALCULATOR->error(true, _("Unsolvable comparison in %s()."), name().c_str(), NULL);
				}
				Manager mngr_v(v);
				mngr->set(this, &mngr_v, NULL);
				delete v;
				return;
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
				if(CALCULATOR->showArgumentErrors()) {
					CALCULATOR->error(true, _("Unsolvable comparison in %s()."), name().c_str(), NULL);
				}
				Manager mngr_v(v);
				mngr->set(this, &mngr_v, NULL);
				delete v;
				return;
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

BASEFunction::BASEFunction() : Function("General", "base", 2, "Number Base") {
	setArgumentDefinition(1, new TextArgument());
	IntegerArgument *arg = new IntegerArgument();
	Number integ(2, 1);
	arg->setMin(&integ);
	integ.set(36, 1);
	arg->setMax(&integ);
	setArgumentDefinition(2, arg);
}
void BASEFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str = vargs[0]->text();
	remove_blanks(str);
	mngr->set(strtol(str.c_str(), NULL, vargs[1]->number()->intValue()), 1);
}
BINFunction::BINFunction() : Function("General", "bin", 1, "Binary") {
	setArgumentDefinition(1, new TextArgument());
}
void BINFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str = vargs[0]->text();
	remove_blanks(str);
	mngr->set(strtol(str.c_str(), NULL, 2), 1);
}
OCTFunction::OCTFunction() : Function("General", "oct", 1, "Octal") {
	setArgumentDefinition(1, new TextArgument());
}
void OCTFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str = vargs[0]->text();
	remove_blanks(str);
	mngr->set(strtol(str.c_str(), NULL, 8), 1);
}
HEXFunction::HEXFunction() : Function("General", "hex", 1, "Hexadecimal") {
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
RomanFunction::RomanFunction() : Function("General", "roman", 1, "Roman Number") {
	setArgumentDefinition(1, new TextArgument());
}
void RomanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str = vargs[0]->text();
	remove_blanks(str);
	Number nr;
	Number cur;
	bool large = false;
	vector<Number> numbers;
	bool capital = false;
	for(unsigned int i = 0; i < str.length(); i++) {
		switch(str[i]) {
			case 'I': {
				if(!capital && i == str.length() - 1) {
					cur.set(2);
					CALCULATOR->error(false, _("Assuming the unusual practice of letting a last capital I mean 2 in a roman numeral."), NULL);
					break;
				}
			}
			case 'J': {capital = true;}
			case 'i': {}
			case 'j': {
				cur.set(1);
				break;
			}
			case 'V': {capital = true;}
			case 'v': {
				cur.set(5);
				break;
			}
			case 'X': {capital = true;}
			case 'x': {
				cur.set(10);
				break;
			}
			case 'L': {capital = true;}
			case 'l': {
				cur.set(50);
				break;
			}
			case 'C': {capital = true;}
			case 'c': {
				cur.set(100);
				break;
			}
			case 'D': {capital = true;}
			case 'd': {
				cur.set(500);
				break;
			}
			case 'M': {capital = true;}
			case 'm': {
				cur.set(1000);
				break;
			}
			case '(': {
				int multi = 1, multi2;
				bool turn = false;
				bool error = false;
				i++;
				for(; i < str.length(); i++) {
					if(str[i] == '|') {
						if(!turn) {
							turn = true;
							multi2 = multi;
						} else {
							error = true;
							break;
						}
					} else if(str[i] == ')') {
						if(turn) {
							multi2--;
							if(multi2 < 1) {
								break;
							}	
						} else {
							error = true;
							break;
						}
					} else if(str[i] == '(') {
						if(!turn) {
							multi++;	
						} else {
							error = true;
							break;
						}
					} else {
						error = true;
						i--;
						break;
					}
				}
				if(error | !turn) {
					CALCULATOR->error(true, _("Error in roman numerals: %s."), str.c_str(), NULL);
				} else {
					cur.set(10);
					cur.raise(multi);
					cur.multiply(100);
				}
				break;
			}
			case '|': {
				if(large) {
					cur.clear();
					large = false;
					break;
				} else if(str.length() > i + 1 && str[i + 2] == ')') {
					i++;
					int multi = 1;
					for(; i < str.length(); i++) {
						if(str[i] != ')') {
							i--;
							break;
						}
						multi++;
					}
					cur.set(10);
					cur.raise(multi);
					cur.multiply(50);
					break;
				} else if(str.length() > i + 2 && str[i + 2] == '|') {
					cur.clear();
					large = true;
					break;
				}
			}
			default: {
				cur.clear();
				CALCULATOR->error(true, _("Unknown roman numeral: %s."), str.substr(i, 1).c_str(), NULL);
			}
		}
		if(!cur.isZero()) {
			if(large) {
				cur.multiply(100000);
			}
			numbers.resize(numbers.size() + 1);
			numbers[numbers.size() - 1].set(&cur);
		}
	}
	vector<Number> values;
	values.resize(numbers.size());
	bool error = false;
	int rep = 1;
	for(unsigned int i = 0; i < numbers.size(); i++) {
		if(i == 0 || numbers[i].isLessThanOrEqualTo(&numbers[i - 1])) {
			nr.add(&numbers[i]);
			if(i > 0 && numbers[i].equals(&numbers[i - 1])) {
				rep++;
				if(rep > 3 && numbers[i].isLessThan(1000)) {
					error = true;
				} else if(rep > 1 && (numbers[i].equals(5) || numbers[i].equals(50) || numbers[i].equals(500))) {
					error = true;
				}
			} else {
				rep = 1;
			}
		} else {
			numbers[i - 1].multiply(10);
			if(numbers[i - 1].isLessThan(&numbers[i])) {
				error = true;
			}
			numbers[i - 1].divide(10);
			for(int i2 = i - 2; ; i2--) {
				if(i2 < 0) {
					nr.negate();
					nr.add(&numbers[i]);
					break;
				} else if(numbers[i2].isGreaterThan(&numbers[i2 + 1])) {
					Number nr2(&nr);
					nr2.subtract(&values[i2]);
					nr.subtract(&nr2);
					nr.subtract(&nr2);
					nr.add(&numbers[i]);
					if(numbers[i2].isLessThan(&numbers[i])) {
						error = true;
					}
					break;
				}
				error = true;
			}
		}
		values[i].set(&nr);
	}
	if(error) {
		CALCULATOR->error(false, _("Errors in roman numerals: \"%s\". Interpreted as %s, which should be written as %s."), str.c_str(), nr.print().c_str(), nr.print(NUMBER_FORMAT_ROMAN).c_str(), NULL);
	}
	values.clear();
	numbers.clear();
	mngr->set(&nr);
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
AsciiFunction::AsciiFunction() : Function("Utilities", "code", 1, "ASCII Value") {
	TextArgument *arg = new TextArgument();
	arg->setCustomCondition("len(\\x) = 1");
	setArgumentDefinition(1, arg);
}
void AsciiFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	unsigned char c = (unsigned char) vargs[0]->text()[0];
	mngr->set(c, 1);
}
CharFunction::CharFunction() : Function("Utilities", "char", 1, "ASCII Char") {
	IntegerArgument *arg = new IntegerArgument();
	Number fr(32, 0);
	arg->setMin(&fr);
	fr.set(0x7f, 1);
	arg->setMax(&fr);
	setArgumentDefinition(1, arg);
}
void CharFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	string str;
	str += vargs[0]->number()->intValue();
	mngr->set(str);
}
ReplaceFunction::ReplaceFunction() : Function("Utilities", "replace", 3, "Replace") {
}
void ReplaceFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->replace(vargs[1], vargs[2]);
}

RadiansToDefaultAngleUnitFunction::RadiansToDefaultAngleUnitFunction() : Function("Trigonometry", "radtodef", 1, "Radians To Default Angle Unit") {
}
void RadiansToDefaultAngleUnitFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	switch(CALCULATOR->angleMode()) {
		case DEGREES: {
			Manager mngr_pi;
			if(CALCULATOR->alwaysExact()) {
				mngr_pi.set(CALCULATOR->getPI());
			} else {
				mngr_pi.set(CALCULATOR->getPI()->get());
			}
			mngr->addInteger(180, OPERATION_MULTIPLY);
	    		mngr->add(&mngr_pi, OPERATION_DIVIDE);
			break;
		}
		case GRADIANS: {
			Manager mngr_pi;
			if(CALCULATOR->alwaysExact()) {
				mngr_pi.set(CALCULATOR->getPI());
			} else {
				mngr_pi.set(CALCULATOR->getPI()->get());
			}
			mngr->addInteger(200, OPERATION_MULTIPLY);
	    		mngr->add(&mngr_pi, OPERATION_DIVIDE);
			break;
		}
	}
}

#ifdef HAVE_GIAC
GiacFunction::GiacFunction() : Function("Calculus", "giac", 1, "Giac expression") {
	setArgumentDefinition(1, new TextArgument());
}
void GiacFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	try {
		giac::gen v1(vargs[0]->text());
		mngr->set(simplify(v1));
		mngr->clean();
	} catch(std::runtime_error & err){
		CALCULATOR->error(true, _("Giac error: %s."), err.what(), NULL);
		mngr->set(this, vargs[0], NULL);
		return;
	}
}
GiacDeriveFunction::GiacDeriveFunction() : Function("Calculus", "giac_diff", 1, "Derive (giac)", "", 3) {
	setArgumentDefinition(1, new GiacArgument());
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");
	setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(3, "1");		
}
void GiacDeriveFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
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
		CALCULATOR->error(true, _("Giac error: %s."), err.what(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
		return;
	}
}
GiacSolveFunction::GiacSolveFunction() : Function("Calculus", "giac_solve", 1, "Solve equation (giac)", "", 2) {
	setArgumentDefinition(1, new GiacArgument());
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");		
}
void GiacSolveFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
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
		CALCULATOR->error(true, _("Giac error: %s."), err.what(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
}
GiacIntegrateFunction::GiacIntegrateFunction() : Function("Calculus", "integrate", 1, "Integrate", "", 2) {
	setArgumentDefinition(1, new GiacArgument());
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");
}
void GiacIntegrateFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	bool failed = false;
	giac::gen v1 = vargs[0]->toGiac(&failed);
	if(failed) {
		CALCULATOR->error(true, _("Conversion to Giac failed."), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
	giac::identificateur id(vargs[1]->text());
	try {
#ifdef OLD_GIAC_API
    		giac::gen ans = giac::integrate(v1, id);
#else
		giac::gen ans = giac::integrate(v1, id, NULL);
#endif
		ans = simplify(ans);
		mngr->set(ans);
		mngr->clean();
	} catch(std::runtime_error & err){
		CALCULATOR->error(true, _("Giac error: %s."), err.what(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
}
#endif
DeriveFunction::DeriveFunction() : Function("Calculus", "diff", 1, "Derive", "", 3) {
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");
	setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(3, "1");		
}
void DeriveFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	int i = vargs[2]->number()->intValue();
	mngr->set(vargs[0]);
	while(i) {
		mngr->differentiate(vargs[1]->text());
		i--;
	}
}
IntegrateFunction::IntegrateFunction() : Function("Calculus", "integrate", 1, "Integrate", "", 2) {
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");
}
void IntegrateFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->integrate(vargs[1]->text());
}
SolveFunction::SolveFunction() : Function("Calculus", "solve", 1, "Solve equation", "", 2) {
	setArgumentDefinition(2, new TextArgument());
	setDefaultValue(2, "\"x\"");		
}
void SolveFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	Manager mngr_solve(vargs[0]);
	mngr_solve.solve(vargs[1]->text());
	if(vargs[0]->isComparison()) {
		if(vargs[0]->comparisonType() == COMPARISON_EQUALS) {
			if(mngr_solve.getChild(0)->equals(vargs[1])) {
				mngr->set(mngr_solve.getChild(1));
				return;
			} else if(mngr_solve.getChild(1)->equals(vargs[1])) {
				mngr->set(mngr_solve.getChild(0));
				return;
			}
		} else {
			mngr->set(&mngr_solve);
			return;
		}
	}
	CALCULATOR->error(false, _("No solution was found."), NULL);
	mngr->set(this, &mngr_solve, vargs[1], NULL);
}

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
	Matrix *mtrx = CALCULATOR->importCSV(vargs[0]->text().c_str(), vargs[1]->number()->intValue(), vargs[2]->text());
	if(!mtrx) {
		CALCULATOR->error(true, "Failed to load %s.", vargs[0]->text().c_str(), NULL);
	}
	mngr->set(mtrx);
}
