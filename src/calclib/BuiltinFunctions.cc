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

#include <sstream>

#define TRIG_FUNCTION(FUNC)	if(vargs[0]->isFraction()) {mngr->set(vargs[0]); CALCULATOR->setAngleValue(mngr); if(!mngr->fraction()->FUNC()) {mngr->set(this, vargs[0], NULL);} } else {mngr->set(this, vargs[0], NULL);}
#define FR_FUNCTION(FUNC)	if(vargs[0]->isFraction()) {mngr->set(vargs[0]); if(!mngr->fraction()->FUNC()) {mngr->set(this, vargs[0], NULL);} } else {mngr->set(this, vargs[0], NULL);}
#define FR_FUNCTION_2(FUNC)	if(vargs[0]->isFraction() && vargs[1]->isFraction()) {mngr->set(vargs[0]); if(!mngr->fraction()->FUNC(vargs[1]->fraction())) {mngr->set(this, vargs[0], vargs[1], NULL);} } else {mngr->set(this, vargs[0], vargs[1], NULL);}

#define TEST_TEXT(i)		if(!vargs[i]->isText()) {CALCULATOR->error(true, _("You need to put expression in quotes for %s()."), name().c_str(), NULL); Manager *mngr2 = createFunctionManagerFromVArgs(vargs); mngr->set(mngr2); mngr2->unref(); return;}



PiFunction::PiFunction() : Function("Constants", "pi", 0, "Archimede's Constant (pi)") {}
void PiFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
		mngr->set(name());
	} else {
		Fraction fr; fr.pi(); mngr->set(&fr);
	}
}
EFunction::EFunction() : Function("Constants", "e", 0, "The Base of Natural Logarithms (e)") {}
void EFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
		mngr->set(name());
	} else {
		Fraction fr; fr.e(); mngr->set(&fr);
	}
}
PythagorasFunction::PythagorasFunction() : Function("Constants", "pythagoras", 0, "Pythagora's Constant (sqrt 2)") {}
void PythagorasFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
		mngr->set(name());
	} else {
		Fraction fr; fr.pythagoras(); mngr->set(&fr);
	}
}
EulerFunction::EulerFunction() : Function("Constants", "euler", 0, "Euler's Constant") {}
void EulerFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
		mngr->set(name());
	} else {
		Fraction fr; fr.euler(); mngr->set(&fr);
	}
}
GoldenFunction::GoldenFunction() : Function("Constants", "golden", 0, "The Golden Ratio") {}
void GoldenFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
		mngr->set(name());
	} else {
		Fraction fr; fr.golden(); mngr->set(&fr);
	}
}
AperyFunction::AperyFunction() : Function("Constants", "apery", 0, "Apery's Constant") {}
void AperyFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
		mngr->set(name());
	} else {
		Fraction fr; fr.apery(); mngr->set(&fr);
	}
}
CatalanFunction::CatalanFunction() : Function("Constants", "catalan", 0, "Catalan's Constant") {}
void CatalanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(CALCULATOR->alwaysExact()) {
		mngr->set(name());
	} else {
		Fraction fr; fr.catalan(); mngr->set(&fr);
	}
}


#ifdef HAVE_LIBCLN
ZetaFunction::ZetaFunction() : Function("", "zeta", 1, "Riemann Zeta") {
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 1);
}
void ZetaFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	FR_FUNCTION(zeta)
}
#endif
ProcessFunction::ProcessFunction() : Function("Utilities", "process", 1, "Process components", "", -1) {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void ProcessFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	string sarg = vargs[0]->text();
	int i = sarg.find("\\x");
	int i_length;
	while(i != string::npos) {
		if(i + 2 < sarg.length() && sarg[i + 2] == '_' && i + 3 < sarg.length()) {
			string index_str = "component(";
			if(sarg[i + 3] == LEFT_BRACKET_CH) {
				int missing = 0;
				int i2 = find_ending_bracket(sarg, i + 4, &missing);
				if(i2 == string::npos) {
					for(int i3 = 1; i3 < missing; i3++) {
						sarg += RIGHT_BRACKET;
					}
					index_str += sarg.substr(i + 4, sarg.length() - (i + 4));
					i_length += sarg.length() - i;
				} else {
					index_str += sarg.substr(i + 4, i2 - (i + 4));
					i_length = i2 + 1 - i;					
				}
			} else {
				int i2 = sarg.find_first_of(OPERATORS, i + 3);
				if(i2 = string::npos) {
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
		for(int index = 1; index <= v->components(); index++) {
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
		for(int index_r = 1; index_r <= mtrx->rows(); index_r++) {
			r_mngr.set(index_r, 1);						
			for(int index_c = 1; index_c <= mtrx->columns(); index_c++) {		
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
	setArgumentType(ARGUMENT_TYPE_INTEGER, 1);
	setArgumentType(ARGUMENT_TYPE_INTEGER, 2);
	setArgumentType(ARGUMENT_TYPE_TEXT, 4);
	setArgumentType(ARGUMENT_TYPE_VECTOR, 5);
}
void CustomSumFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {

	int start = 1;
	int end = -1;
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger()) {
		CALCULATOR->error(true, _("The upper and lower limits must be integers."), NULL);
	} else {
		start = vargs[0]->fraction()->numerator()->getInt();
		if(start < 1) start = 1;
		end = vargs[1]->fraction()->numerator()->getInt();
	}

	string sarg = vargs[3]->text();
	int i = sarg.find("\\x");
	int i_length;
	while(i != string::npos) {
		if(i + 2 < sarg.length() && sarg[i + 2] == '_' && i + 3 < sarg.length()) {
			string index_str = "component(";
			if(sarg[i + 3] == LEFT_BRACKET_CH) {
				int missing = 0;
				int i2 = find_ending_bracket(sarg, i + 4, &missing);
				if(i2 == string::npos) {
					for(int i3 = 1; i3 < missing; i3++) {
						sarg += RIGHT_BRACKET;
					}
					index_str += sarg.substr(i + 4, sarg.length() - (i + 4));
					i_length += sarg.length() - i;
				} else {
					index_str += sarg.substr(i + 4, i2 - (i + 4));
					i_length = i2 + 1 - i;					
				}
			} else {
				int i2 = sarg.find_first_of(OPERATORS, i + 3);
				if(i2 = string::npos) {
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
		for(int index_r = 1; index_r <= mtrx->rows(); index_r++) {
			r_mngr.set(index_r, 1);						
			for(int index_c = 1; index_c <= mtrx->columns(); index_c++) {		
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
	setArgumentType(ARGUMENT_TYPE_TEXT, 1);
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
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 1);
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 2);	
}
void MatrixFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[0]->fraction()->isPositive() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger() || !vargs[1]->fraction()->isPositive()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);	
		mngr->set(mngr2);
		mngr2->unref();
		CALCULATOR->error(true, _("The number of rows and columns in matrix must be positive integers."), NULL);
		return;
	}
	Matrix mtrx(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt());
	int r = 1, c = 1;
	for(int i = 2; i < vargs.size(); i++) {
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
	for(int i = 0; i < vargs.size(); i++) {
		vctr.set(vargs[i], i + 1);	
	}
	mngr->set(&vctr);
}
RankFunction::RankFunction() : Function("Matrices", "rank", -1, "Rank") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
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
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
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
MatrixToVectorFunction::MatrixToVectorFunction() : Function("Matrices", "matrixtovector", 1, "Convert Matrix to Vector") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
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
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 1);
	setArgumentType(ARGUMENT_TYPE_MATRIX, 2);	
}
void RowFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction() && vargs[0]->fraction()->isPositive() && vargs[0]->fraction()->isInteger() && vargs[1]->isMatrix()) {
		Vector *v = vargs[1]->matrix()->rowToVector(vargs[0]->fraction()->numerator()->getInt());
		if(!v) {
			CALCULATOR->error(true, "Row %s does not exist in matrix.", vargs[0]->print().c_str(), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);
		} else {
			mngr->set(v);
			delete v;
		}
	} else {
		mngr->set(this, vargs[0], vargs[1], NULL);
	}
}
ColumnFunction::ColumnFunction() : Function("Matrices", "column", 2, "Extract Column as Vector") {
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 1);
	setArgumentType(ARGUMENT_TYPE_MATRIX, 2);	
}
void ColumnFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction() && vargs[0]->fraction()->isPositive() && vargs[0]->fraction()->isInteger() && vargs[1]->isMatrix()) {
		Vector *v = vargs[1]->matrix()->columnToVector(vargs[0]->fraction()->numerator()->getInt());
		if(!v) {
			CALCULATOR->error(true, "Column %s does not exist in matrix.", vargs[0]->print().c_str(), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);
		} else {
			mngr->set(v);
			delete v;
		}
	} else {
		mngr->set(this, vargs[0], vargs[1], NULL);
	}
}
RowsFunction::RowsFunction() : Function("Matrices", "rows", 1, "Rows") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void RowsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->rows(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ColumnsFunction::ColumnsFunction() : Function("Matrices", "columns", 1, "Columns") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void ColumnsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->columns(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ElementsFunction::ElementsFunction() : Function("Matrices", "elements", 1, "Elements") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void ElementsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->columns() * vargs[0]->matrix()->rows(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ElementFunction::ElementFunction() : Function("Matrices", "element", 3, "Element") {
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 1);
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 2);
	setArgumentType(ARGUMENT_TYPE_MATRIX, 3);
}
void ElementFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[0]->fraction()->isPositive() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger() || !vargs[1]->fraction()->isPositive()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);	
		mngr->set(mngr2);
		mngr2->unref();
		CALCULATOR->error(true, _("Row and column in matrix must be positive integers."), NULL);
		return;
	}
	if(vargs[2]->isMatrix()) {
		mngr->set(vargs[2]->matrix()->get(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt()));
	} else if(vargs[0]->fraction()->isOne() && vargs[1]->fraction()->isOne()) {
		mngr->set(vargs[2]);
	}
}
ComponentsFunction::ComponentsFunction() : Function("Matrices", "components", 1, "Components") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void ComponentsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->columns() * vargs[0]->matrix()->rows(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ComponentFunction::ComponentFunction() : Function("Matrices", "component", 2, "Component") {
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 1);
	setArgumentType(ARGUMENT_TYPE_VECTOR, 2);
}
void ComponentFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[0]->fraction()->isPositive()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);	
		mngr->set(mngr2);
		mngr2->unref();
		CALCULATOR->error(true, _("Component index in vector must be a positive integer."), NULL);
		return;
	}
	if(vargs[1]->isMatrix()) {
		mngr->set(vargs[1]->matrix()->get((vargs[0]->fraction()->numerator()->getInt() - 1) / vargs[1]->matrix()->columns() + 1, (vargs[0]->fraction()->numerator()->getInt() - 1) % vargs[1]->matrix()->columns() + 1));
	} else if(vargs[0]->fraction()->isOne()) {
		mngr->set(vargs[1]);
	}
}
LimitsFunction::LimitsFunction() : Function("Matrices", "limits", 2, "Limits", "", -1) {
	setArgumentType(ARGUMENT_TYPE_INTEGER, 1);
	setArgumentType(ARGUMENT_TYPE_INTEGER, 2);	
	setArgumentType(ARGUMENT_TYPE_VECTOR, 3);	
}
void LimitsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);	
		mngr->set(mngr2);
		mngr2->unref();
		CALCULATOR->error(true, _("The upper and lower limits must be integers."), NULL);
		return;
	}
	int i = vargs[0]->fraction()->numerator()->getInt(), n = vargs[1]->fraction()->numerator()->getInt();	
	Vector *v = produceVector(vargs);
	Vector *vctr = v->getRange(i, n);
	mngr->set(vctr);
	delete vctr;
	delete v;
}
TransposeFunction::TransposeFunction() : Function("Matrices", "transpose", 1, "Transpose") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void TransposeFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]);
		mngr->matrix()->transpose();
	} else {
		mngr->set(this, vargs[0], NULL);
	}
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
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void DeterminantFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		Manager *det = vargs[0]->matrix()->determinant();
		if(det) {
			mngr->set(det);
			det->unref();	
			return;
		}
	}
	mngr->set(this, vargs[0], NULL);
}
CofactorFunction::CofactorFunction() : Function("Matrices", "cofactor", 3, "Cofactor") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void CofactorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[0]->fraction()->isPositive() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger() || !vargs[1]->fraction()->isPositive()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);	
		mngr->set(mngr2);
		mngr2->unref();
		CALCULATOR->error(true, _("Row and column in matrix must be positive integers."), NULL);
		return;
	}
	if(vargs[2]->isMatrix()) {
		Manager *mngr2 = vargs[2]->matrix()->cofactor(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt());
		if(mngr2) {
			mngr->set(mngr2);
			mngr2->unref();
			return;
		}
	}
	mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
}
AdjointFunction::AdjointFunction() : Function("Matrices", "adj", 1, "Adjoint") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void AdjointFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]);
		if(!mngr->matrix()->adjoint()) {
			mngr->set(this, vargs[0], NULL);
		}
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
InverseFunction::InverseFunction() : Function("Matrices", "inverse", 1, "Inverse") {
	setArgumentType(ARGUMENT_TYPE_MATRIX, 1);
}
void InverseFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]);
		if(!mngr->matrix()->inverse()) {
			mngr->set(this, vargs[0], NULL);
		}
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
IFFunction::IFFunction() : Function("Logical", "if", 3, "If...Then...Else") {
	setArgumentType(ARGUMENT_TYPE_TEXT, 1);
}
Manager *IFFunction::calculate(vector<Manager*> &vargs) {
	string argv = "";
	for(int i = 0; i < vargs.size(); i++) {
		if(i != 0) {
			argv += COMMA;
		}
		argv += vargs[i]->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTIONAL_ONLY | DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT | DISPLAY_FORMAT_SCIENTIFIC);
	}
	return calculate(argv);
}
Manager *IFFunction::calculate(const string &argv) {
	Manager *mngr = NULL;
	argc = 1;
	max_argc = 1;
	vector<Manager*> vargs;
	args(argv, vargs);	
	argc = 3;
	max_argc = 3;
	vector<string> svargs;
	int itmp = stringArgs(argv, svargs);	
	if(testArgumentCount(itmp)) {
		string expr;
		if(!vargs[0]->isText()) {
			CALCULATOR->error(true, _("You need to put expression in quotes for %s()."), name().c_str(), NULL);		
			mngr = createFunctionManagerFromSVArgs(svargs);
			for(int i = 0; i < vargs.size(); i++) {
				vargs[i]->unref();
			}
			svargs.clear();
			return mngr;	
		}
		expr = vargs[0]->text();
		unsigned int i = expr.find_first_of("<=>", 0);
		bool result = false;
		int com = 0;
		if(i == string::npos) {
//			CALCULATOR->error(false, _("Condition contains no comparison, interpreting as \"%s > 0\"."), expr.c_str(), NULL);
			expr += " > 0";
			i = expr.find_first_of("<=>", 0);
		} 
		string str1 = expr.substr(0, i);
		string str2 = expr.substr(i + 1, expr.length() - i + 1);			
		remove_blank_ends(str2);
		char sign1 = expr[i], sign2 = 0;
		if(str2[0] == '>' || str2[0] == '=' || str2[0] == '<') {
			if(str2[0] != sign1) sign2 = str2[0];
			str2.erase(str2.begin());
		}
		Manager *mngr1 = CALCULATOR->calculate(str1);
		Manager *mngr2 = CALCULATOR->calculate(str2);			
		mngr1->add(mngr2, SUBTRACT);
		if(mngr1->isFraction()) {
			if(sign1 == '=') {
				if(sign2 == 0) 
					result = mngr1->fraction()->isZero();
				else if(sign2 == '>') 
					result = !mngr1->fraction()->isNegative();
				else if(sign2 == '<') 
					result = !mngr1->fraction()->isPositive();
			} else if(sign1 == '>') {
				if(sign2 == 0) 
					result = mngr1->fraction()->isPositive();
				else if(sign2 == '=') 
					result = !mngr1->fraction()->isNegative();
				else if(sign2 == '<') 
					result = !mngr1->fraction()->isZero(); 
			} else if(sign1 == '<') {
				if(sign2 == 0) 
					result = mngr1->fraction()->isNegative();
				else if(sign2 == '>') 
					result = !mngr1->fraction()->isZero();
				else if(sign2 == '=') 
					result = !mngr1->fraction()->isPositive();
			}
		} else {
			CALCULATOR->error(true, _("Comparison is not solvable, treating as FALSE."), NULL);
		}
		mngr1->unref();
		mngr2->unref();
		
		if(result) {			
			mngr = CALCULATOR->calculate(svargs[1]);
		} else {			
			mngr = CALCULATOR->calculate(svargs[2]);		
		}			
	} else {
		mngr = createFunctionManagerFromSVArgs(svargs);
	}
	for(int i = 0; i < vargs.size(); i++) {
		vargs[i]->unref();
	}
	svargs.clear();
	return mngr;
}
GCDFunction::GCDFunction() : Function("Arithmetics", "gcd", 2, "Greatest Common Divisor") {
}
void GCDFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(!vargs[0]->isFraction()) mngr->set(this, vargs[0], vargs[1], NULL);
	if(!vargs[1]->isFraction()) mngr->set(this, vargs[0], vargs[1], NULL);	
	mngr->set(vargs[0]);
	mngr->fraction()->gcd(vargs[1]->fraction());
}
DaysFunction::DaysFunction() : Function("Date & Time", "daysto", 2, "Days to date") {
	setArgumentType(ARGUMENT_TYPE_DATE, 1);
	setArgumentType(ARGUMENT_TYPE_DATE, 2);	
}
void DaysFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	TEST_TEXT(1)	
	int days = daysBetweenDates(vargs[0]->text(), vargs[1]->text(), 1);
	if(days < 0) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
	} else {
		mngr->set(days, 1, 0);
	}	
}
DaysBetweenDatesFunction::DaysBetweenDatesFunction() : Function("Date & Time", "days_between_dates", 2, "Days between two dates", "", 3) {
	setArgumentType(ARGUMENT_TYPE_DATE, 1);
	setArgumentType(ARGUMENT_TYPE_DATE, 2);	
	setArgumentType(ARGUMENT_TYPE_INTEGER, 3);	
}
void DaysBetweenDatesFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	TEST_TEXT(1)	
	if(!vargs[2]->isFraction() || !vargs[2]->fraction()->isInteger() || vargs[2]->fraction()->isNegative()) {
		CALCULATOR->error(true, _("Basis for %s() must be a non-negative integer."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
		return;
	}
	int days = daysBetweenDates(vargs[0]->text(), vargs[1]->text(), vargs[2]->fraction()->numerator()->getInt());
	if(days < 0) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);			
	} else {
		mngr->set(days, 1, 0);
	}		
}
YearsBetweenDatesFunction::YearsBetweenDatesFunction() : Function("Date & Time", "years_between_dates", 2, "Years between two dates", "", 3) {
	setArgumentType(ARGUMENT_TYPE_DATE, 1);
	setArgumentType(ARGUMENT_TYPE_DATE, 2);	
	setArgumentType(ARGUMENT_TYPE_INTEGER, 3);	
}
void YearsBetweenDatesFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	TEST_TEXT(1)	
	if(!vargs[2]->isFraction() || !vargs[2]->fraction()->isInteger() || vargs[2]->fraction()->isNegative()) {
		CALCULATOR->error(true, _("Basis for %s() must be a non-negative integer."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
		return;
	}
	Fraction *fr = yearsBetweenDates(vargs[0]->text(), vargs[1]->text(), vargs[2]->fraction()->numerator()->getInt());
	if(!fr) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);			
	} else {
		mngr->set(fr);
	}
	delete fr;
}
DifferentiateFunction::DifferentiateFunction() : Function("Experimental", "diff", 2, "Differentiate") {
	setArgumentType(ARGUMENT_TYPE_TEXT, 2);
}
void DifferentiateFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	CALCULATOR->error(true, _("%s() is an experimental unfinished function!"), name().c_str(), NULL);
	TEST_TEXT(1)
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
	setArgumentType(ARGUMENT_TYPE_NONNEGATIVE_INTEGER, 1);
}
void FactorialFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction() && !vargs[0]->fraction()->isNegative() && vargs[0]->fraction()->isInteger()) {
		if(vargs[0]->fraction()->isZero()) mngr->set(1, 1);
		mngr->set(vargs[0]);
		while(!vargs[0]->fraction()->isOne()) {
			vargs[0]->addInteger(-1, ADD);
			mngr->add(vargs[0], MULTIPLY);
		}
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
AbsFunction::AbsFunction() : Function("Arithmetics", "abs", 1, "Absolute Value") {

}
void AbsFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->setNegative(false);		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
CeilFunction::CeilFunction() : Function("Arithmetics", "ceil", 1, "Round upwards") {

}
void CeilFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->ceil();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
FloorFunction::FloorFunction() : Function("Arithmetics", "floor", 1, "Round downwards") {

}
void FloorFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->floor();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
TruncFunction::TruncFunction() : Function("Arithmetics", "trunc", 1, "Round towards zero") {

}
void TruncFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->trunc();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
RoundFunction::RoundFunction() : Function("Arithmetics", "round", 1, "Round") {

}
void RoundFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->round();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
FracFunction::FracFunction() : Function("Arithmetics", "frac", 1, "Extract fractional part") {

}
void FracFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->mod();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
IntFunction::IntFunction() : Function("Arithmetics", "int", 1, "Extract integer part") {

}
void IntFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->trunc();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
RemFunction::RemFunction() : Function("Arithmetics", "rem", 2, "Reminder (rem)") {
	setArgumentType(ARGUMENT_TYPE_NONZERO, 2);
}
void RemFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		if(vargs[1]->isZero()) {
			CALCULATOR->error(true, _("The denominator in rem function cannot be zero"), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);		
		} else {
			mngr->set(vargs[0]);	
			mngr->fraction()->divide(vargs[1]->fraction());
			mngr->fraction()->rem();		
		}
	} else {
		mngr->set(this, vargs[0], vargs[1], NULL);
	}
}
ModFunction::ModFunction() : Function("Arithmetics", "mod", 2, "Reminder (mod)") {
	setArgumentType(ARGUMENT_TYPE_NONZERO, 2);
}
void ModFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		if(vargs[1]->isZero()) {
			CALCULATOR->error(true, _("The denominator in rem function cannot be zero"), NULL);
			mngr->set(this, vargs[0], vargs[1], NULL);		
		} else {
			mngr->set(vargs[0]);	
			mngr->fraction()->divide(vargs[1]->fraction());
			mngr->fraction()->mod();		
		}
	} else {
		mngr->set(this, vargs[0], vargs[1], NULL);
	}
}

SinFunction::SinFunction() : Function("Trigonometry", "sin", 1, "Sine") {}
void SinFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(sin)
}
CosFunction::CosFunction() : Function("Trigonometry", "cos", 1, "Cosine") {}
void CosFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(cos)
}
TanFunction::TanFunction() : Function("Trigonometry", "tan", 1, "Tangent") {}
void TanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(tan)
}
SinhFunction::SinhFunction() : Function("Trigonometry", "sinh", 1, "Hyperbolic sine") {}
void SinhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(sinh)
}
CoshFunction::CoshFunction() : Function("Trigonometry", "cosh", 1, "Hyperbolic cosine") {}
void CoshFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(cosh)
}
TanhFunction::TanhFunction() : Function("Trigonometry", "tanh", 1, "Hyperbolic tangent") {}
void TanhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(tanh)
}
AsinFunction::AsinFunction() : Function("Trigonometry", "asin", 1, "Arcsine") {}
void AsinFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(asin)
}
AcosFunction::AcosFunction() : Function("Trigonometry", "acos", 1, "Arccosine") {}
void AcosFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(acos)
}
AtanFunction::AtanFunction() : Function("Trigonometry", "atan", 1, "Arctangent") {}
void AtanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(atan)
}
AsinhFunction::AsinhFunction() : Function("Trigonometry", "asinh", 1, "Hyperbolic arcsine") {}
void AsinhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(asinh)
}
AcoshFunction::AcoshFunction() : Function("Trigonometry", "acosh", 1, "Hyperbolic arccosine") {}
void AcoshFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(acosh)
}
AtanhFunction::AtanhFunction() : Function("Trigonometry", "atanh", 1, "Hyperbolic arctangent") {}
void AtanhFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TRIG_FUNCTION(atanh)
}
LogFunction::LogFunction() : Function("Exponents and Logarithms", "ln", 1, "Natural Logarithm") {
	setArgumentType(ARGUMENT_TYPE_POSITIVE, 1);
}
void LogFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	FR_FUNCTION(log)
}
Log10Function::Log10Function() : Function("Exponents and Logarithms", "log", 1, "Base-10 Logarithm") {
	setArgumentType(ARGUMENT_TYPE_POSITIVE, 1);
}
void Log10Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	FR_FUNCTION(log10)
}
Log2Function::Log2Function() : Function("Exponents and Logarithms", "log2", 1, "Base-2 Logarithm") {
	setArgumentType(ARGUMENT_TYPE_POSITIVE, 1);
}
void Log2Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	FR_FUNCTION(log2)
}
ExpFunction::ExpFunction() : Function("Exponents and Logarithms", "exp", 1, "e raised to the power X") {}
void ExpFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->exp()) {
			mngr->set(this, vargs[0], NULL);
		}		
	} else {
		mngr->set(E_VALUE);
		mngr->add(vargs[0], RAISE);	
	}
}
Exp10Function::Exp10Function() : Function("Exponents and Logarithms", "exp10", 1, "10 raised the to power X") {}
void Exp10Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->exp10()) {
			mngr->set(this, vargs[0], NULL);
		}		
	} else {
		mngr->set(10);
		mngr->add(vargs[0], RAISE);	
	}
}
Exp2Function::Exp2Function() : Function("Exponents and Logarithms", "exp2", 1, "2 raised the to power X") {}
void Exp2Function::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->exp2()) {
			mngr->set(this, vargs[0], NULL);
		}		
	} else {
		mngr->set(2);
		mngr->add(vargs[0], RAISE);	
	}
}
SqrtFunction::SqrtFunction() : Function("Exponents and Logarithms", "sqrt", 1, "Square Root") {
	setArgumentType(ARGUMENT_TYPE_NONNEGATIVE, 1);
}
void SqrtFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->sqrt()) {
			mngr->set(this, vargs[0], NULL);
		}
	} else {
		mngr->set(vargs[0]);
		Manager *mngr2 = new Manager(1, 2);
		mngr->add(mngr2, RAISE);
		mngr2->unref();		
	}
}
CbrtFunction::CbrtFunction() : Function("Exponents and Logarithms", "cbrt", 1, "Cube Root") {}
void CbrtFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		if(!mngr->fraction()->cbrt()) {
			mngr->set(this, vargs[0], NULL);
		}		
	} else {
		mngr->set(vargs[0]);
		Manager *mngr2 = new Manager(1, 3);		
		mngr->add(mngr2, RAISE);
		mngr2->unref();	
	}
}
RootFunction::RootFunction() : Function("Exponents and Logarithms", "root", 2, "Nth Root") {}
void RootFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction() && vargs[1]->isFraction()) {
		mngr->set(vargs[0]);
		Fraction fr(1);
		fr.divide(vargs[1]->fraction());
		if(mngr->fraction()->pow(&fr)) {
			return;
		}		
	} 
	mngr->set(vargs[0]);
	Manager *mngr2 = new Manager(1, 1);		
	mngr2->add(vargs[1], DIVIDE);
	mngr->add(mngr2, RAISE);
	mngr2->unref();	
}
PowFunction::PowFunction() : Function("Exponents and Logarithms", "pow", 2, "Power") {}
void PowFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs[0]->isFraction() && vargs[1]->isFraction()) {
		mngr->set(vargs[0]);
		if(mngr->fraction()->pow(vargs[1]->fraction())) {
			return;
		}		
	}
	mngr->set(vargs[0]);
	mngr->add(vargs[1], RAISE);
}
HypotFunction::HypotFunction() : Function("Geometry", "hypot", 2, "Hypotenuse") {
}
void HypotFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs[0]);
	mngr->addInteger(2, RAISE);
	Manager *mngr2 = new Manager(vargs[1]);
	mngr2->addInteger(2, RAISE);		
	mngr->add(mngr2, RAISE);
	mngr2->unref();
	mngr2 = new Manager(1, 2);
	mngr->add(mngr2, RAISE);
	mngr2->unref();
}
SumFunction::SumFunction() : Function("Statistics", "sum", -1, "Sum") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void SumFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], ADD);
	}
}
MeanFunction::MeanFunction() : Function("Statistics", "mean", -1, "Mean") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void MeanFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], ADD);
	}
	mngr->addInteger(vargs.size(), DIVIDE);	
}
MedianFunction::MedianFunction() : Function("Statistics", "median", -1, "Median") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void MedianFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	Vector *v = produceVector(vargs);	
	if(!v->sort()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);
		mngr->set(mngr2);
		mngr2->unref();	
	} else if(v->components() % 2 == 0) {
		mngr->set(v->get(v->components() / 2));
		mngr->add(v->get(v->components() / 2 + 1), ADD);		
		mngr->addInteger(2, DIVIDE);
	} else {
		mngr->set(v->get(v->components() / 2 + 1));
	}
	delete v;
}
PercentileFunction::PercentileFunction() : Function("Statistics", "percentile", 1, "Percentile", "", -1) {
	setArgumentType(ARGUMENT_TYPE_POSITIVE, 1);
	setArgumentType(ARGUMENT_TYPE_VECTOR, 2);	
}
void PercentileFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 1) {
		return;
	}
	Fraction fr100(100);
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isPositive() || !vargs[0]->fraction()->isLessThan(&fr100)) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs);	
		mngr->set(mngr2);
		mngr2->unref();	
		CALCULATOR->error(true, _("Percentile must be supplied a positive value lower than 100."), NULL);
		return;
	}
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
			gap.add(v->get(lfr.numerator()->getInt()), SUBTRACT);
			Manager pfr_mngr(&pfr);
			gap.add(&pfr_mngr, MULTIPLY);
			mngr->set(v->get(lfr.numerator()->getInt()));
			mngr->add(&gap, ADD);
		}
	}
	delete v;
}
MinFunction::MinFunction() : Function("Statistics", "min", -1, "Min") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void MinFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	Vector *v = produceVector(vargs);		
	Fraction *fr = NULL;
	for(int index = 1; index <= v->components(); index++) {
		if(v->get(index)->isFraction()) {
			if(!fr || v->get(index)->fraction()->isLessThan(fr)) {
				fr = v->get(index)->fraction();
			}
		} else {
			CALCULATOR->error(true, _("%s() can only compare numbers."), name().c_str(), NULL);
			Manager *mngr2 = createFunctionManagerFromVArgs(vargs);
			mngr->set(mngr2);
			mngr2->unref();
			fr = NULL;
			break;
		}
	}
	if(fr) mngr->set(fr);
	delete v;
}
MaxFunction::MaxFunction() : Function("Statistics", "max", -1, "Max") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void MaxFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0)
		return;
	Vector *v = produceVector(vargs);		
	Fraction *fr = NULL;
	for(int index = 1; index <= v->components(); index++) {
		if(v->get(index)->isFraction()) {
			if(!fr || v->get(index)->fraction()->isGreaterThan(fr)) {
				fr = v->get(index)->fraction();
			}
		} else {
			CALCULATOR->error(true, _("%s() function can only compare numbers."), name().c_str(), NULL);
			Manager *mngr2 = createFunctionManagerFromVArgs(vargs);
			mngr->set(mngr2);
			mngr2->unref();
			fr = NULL;
			break;
		}
	}
	if(fr) mngr->set(fr);
	delete v;
}
ModeFunction::ModeFunction() : Function("Statistics", "mode", -1, "Mode") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
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
	for(int index_c = 1; index_c <= v->components(); index_c++) {
		b = true;
		for(int index = 0; index < vargs_nodup.size(); index++) {
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
	for(int index = 0; index < is.size(); index++) {
		if(is[index] > n) {
			n = is[index];
			value = vargs_nodup[index];
		}
	}
	mngr->set(value);
	delete v;
}
NumberFunction::NumberFunction() : Function("Statistics", "number", -1, "Number") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void NumberFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(vargs.size(), 1);
}
StdDevFunction::StdDevFunction() : Function("Statistics", "stddev", -1, "Standard Deviation") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void StdDevFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0) {
		return;
	}
	Manager mean, value;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mean.add(vargs[i], ADD);
	}
	mean.addInteger(vargs.size(), DIVIDE);
	for(unsigned int i = 0; i < vargs.size(); i++) {
		vargs[i]->add(&mean, SUBTRACT);
		vargs[i]->addInteger(2, RAISE);
		value.add(vargs[i], ADD);
	}
	value.addInteger(vargs.size(), DIVIDE);
	Manager mngr2(1, 2);
	value.add(&mngr2, RAISE);
	mngr->set(&value);
}
StdDevSFunction::StdDevSFunction() : Function("Statistics", "stddevs", -1, "Standard Deviation (random sampling)") {
	setArgumentType(ARGUMENT_TYPE_VECTOR, 1);
}
void StdDevSFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	if(vargs.size() <= 0) {
		return;
	}
	Manager mean, value;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mean.add(vargs[i], ADD);
	}
	mean.addInteger(vargs.size(), DIVIDE);
	for(unsigned int i = 0; i < vargs.size(); i++) {
		vargs[i]->add(&mean, SUBTRACT);
		vargs[i]->addInteger(2, RAISE);
		value.add(vargs[i], ADD);
	}
	value.addInteger(vargs.size() - 1, DIVIDE);
	Manager mngr2(1, 2);
	value.add(&mngr2, RAISE);
	mngr->set(&value);
}
RandomFunction::RandomFunction() : Function("General", "rand", 0, "Random Number") {}
void RandomFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	mngr->set(drand48());
}

BASEFunction::BASEFunction() : Function("General", "BASE", 2, "Number Base") {
	setArgumentType(ARGUMENT_TYPE_TEXT, 1);
	setArgumentType(ARGUMENT_TYPE_POSITIVE_INTEGER, 2);
}
void BASEFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	if(!vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger() || !vargs[1]->fraction()->isPositive() || vargs[1]->fraction()->isOne() || vargs[1]->fraction()->numerator()->isGreaterThan(36)) {
		CALCULATOR->error(true, _("Base must be an integer between 2 and 36 for %s()."), name().c_str(), NULL);
		mngr->set(this, vargs[0], vargs[1], NULL);
		return;
	}
	mngr->set(strtol(vargs[0]->text().c_str(), NULL, vargs[1]->fraction()->numerator()->getInt()), 1);
}
BINFunction::BINFunction() : Function("General", "BIN", 1, "Binary") {
	setArgumentType(ARGUMENT_TYPE_TEXT, 1);
}
void BINFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	mngr->set(strtol(vargs[0]->text().c_str(), NULL, 2), 1);
}
OCTFunction::OCTFunction() : Function("General", "OCT", 1, "Octal") {
	setArgumentType(ARGUMENT_TYPE_TEXT, 1);
}
void OCTFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	mngr->set(strtol(vargs[0]->text().c_str(), NULL, 8), 1);
}
HEXFunction::HEXFunction() : Function("General", "HEX", 1, "Hexadecimal") {
	setArgumentType(ARGUMENT_TYPE_TEXT, 1);
}
void HEXFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	string expr = vargs[0]->text();
	if(expr.length() >= 2 && expr[0] == '0' && (expr[1] == 'x' || expr[1] == 'X')) {
	} else {
		expr.insert(0, "0x");
	}
	mngr->set(strtold(expr.c_str(), NULL));
}
TitleFunction::TitleFunction() : Function("Utilities", "title", 1, "Function Title") {
	setArgumentType(ARGUMENT_TYPE_FUNCTION, 1);
}
void TitleFunction::calculate(Manager *mngr, vector<Manager*> &vargs) {
	TEST_TEXT(0)
	Function *f = CALCULATOR->getFunction(vargs[0]->text());
	if(!f) {
		CALCULATOR->error(true, _("Function %s() does not exist."), vargs[0]->text().c_str(), NULL);
		mngr->set(this, vargs[0], NULL);
	} else {
		mngr->set(f->title());
	}
}
