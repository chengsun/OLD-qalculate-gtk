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
#include "MathStructure.h"
#include "Number.h"
#include "Calculator.h"
#include "Variable.h"

#include <sstream>

#define FR_FUNCTION(FUNC)	Number nr = vargs[0].number(); if(!nr.FUNC() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {return false;} else {mstruct = nr; return true;}
#define FR_FUNCTION_2(FUNC)	Number nr = vargs[0].number(); if(!nr.FUNC(vargs[1].number()) || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {return false;} else {mstruct = nr; return true;}

#define NON_COMPLEX_NUMBER_ARGUMENT(i)				NumberArgument *arg_non_complex##i = new NumberArgument(); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);
#define NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(i)			NumberArgument *arg_non_complex##i = new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);
#define NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(i)		NumberArgument *arg_non_complex##i = new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, true, false); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);



VectorFunction::VectorFunction() : Function("vector", 0, 1) {
	setArgumentDefinition(1, new VectorArgument());
}
bool VectorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	return true;
}
MatrixFunction::MatrixFunction() : Function("matrix", 3) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(3, new VectorArgument());
}
bool MatrixFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	unsigned int rows = vargs[0].number().intValue();
	unsigned int columns = vargs[1].number().intValue();
	mstruct.clearMatrix(); mstruct.resizeMatrix(rows, columns, m_zero);
	unsigned int r = 1, c = 1;
	for(unsigned int i = 0; i < vargs[2].size(); i++) {
		if(r > rows || c > columns) {
			CALCULATOR->error(false, _("Too many elements (%s) for the order (%sx%s) of the matrix."), i2s(vargs[2].size()).c_str(), i2s(rows).c_str(), i2s(columns).c_str(), NULL);
			break;
		}
		mstruct[r - 1][c - 1] = vargs[i];	
		if(c == columns) {
			c = 1;
			r++;
		} else {
			c++;
		}
	}
	return true;
}
RankFunction::RankFunction() : Function("rank", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
bool RankFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	return mstruct.rankVector();
}
SortFunction::SortFunction() : Function("sort", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
bool SortFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	return mstruct.sortVector();
}
MergeVectorsFunction::MergeVectorsFunction() : Function("mergevectors", -1) {
}
bool MergeVectorsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct.clearVector();
	for(unsigned int i = 0; i < vargs.size(); i++) {
		if(vargs[i].isVector()) {
			for(unsigned int i2 = 0; i2 < vargs[i].size(); i2++) {
				mstruct.addItem(vargs[i][i2]);
			}
		} else {
			mstruct.addItem(vargs[i]);
		}
	}
	return true;
}
MatrixToVectorFunction::MatrixToVectorFunction() : Function("matrix2vector", 1) {
	setArgumentDefinition(1, new MatrixArgument());
}
bool MatrixToVectorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[0].matrixToVector(mstruct);
	return true;
}
RowFunction::RowFunction() : Function("row", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new MatrixArgument());	
}
bool RowFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int row = vargs[0].number().intValue();
	if(row > (int) vargs[1].rows()) {
		CALCULATOR->error(true, _("Row %s does not exist in matrix."), vargs[0].print().c_str(), NULL);
		return false;
	}
	vargs[1].rowToVector(row, mstruct);
	return true;
}
ColumnFunction::ColumnFunction() : Function("column", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new MatrixArgument());	
}
bool ColumnFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int col = vargs[0].number().intValue();
	if(col > (int) vargs[1].columns()) {
		CALCULATOR->error(true, _("Column %s does not exist in matrix."), vargs[0].print().c_str(), NULL);
		return false;
	}
	vargs[1].columnToVector(col, mstruct);
	return true;
}
RowsFunction::RowsFunction() : Function("rows", 1) {
	setArgumentDefinition(1, new MatrixArgument(""));
}
bool RowsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].rows();
	return true;
}
ColumnsFunction::ColumnsFunction() : Function("columns", 1) {
	setArgumentDefinition(1, new MatrixArgument(""));
}
bool ColumnsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].columns();
	return true;
}
ElementsFunction::ElementsFunction() : Function("elements", 1) {
	setArgumentDefinition(1, new MatrixArgument(""));
}
bool ElementsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) (vargs[0].rows() * vargs[0].columns());
	return true;
}
ElementFunction::ElementFunction() : Function("element", 3) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(3, new MatrixArgument(""));
}
bool ElementFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int row = vargs[0].number().intValue();
	int col = vargs[1].number().intValue();
	bool b = true;
	if(col > (int) vargs[2].columns()) {
		CALCULATOR->error(true, _("Column %s does not exist in matrix."), vargs[1].print().c_str(), NULL);
		b = false;
	}
	if(row > (int) vargs[2].rows()) {
		CALCULATOR->error(true, _("Row %s does not exist in matrix."), vargs[0].print().c_str(), NULL);
		b = false;
	}
	if(b) {
		const MathStructure *em = vargs[2].getElement(row, col);
		if(em) mstruct = *em;
		else b = false;
	}
	return b;
}
ComponentsFunction::ComponentsFunction() : Function("components", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
bool ComponentsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].components();
	return true;
}
ComponentFunction::ComponentFunction() : Function("component", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new VectorArgument(""));
}
bool ComponentFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int i = vargs[0].number().intValue();
	if(i > (int) vargs[1].components()) {
		CALCULATOR->error(true, _("Component %s does not exist in vector."), vargs[0].print().c_str(), NULL);
		return false;
	}
	mstruct = *vargs[1].getComponent(i);
	return true;
}
LimitsFunction::LimitsFunction() : Function("limits", 3) {
	setArgumentDefinition(1, new IntegerArgument(""));
	setArgumentDefinition(2, new IntegerArgument(""));	
	setArgumentDefinition(3, new VectorArgument(""));	
}
bool LimitsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[2].getRange(vargs[0].number().intValue(), vargs[1].number().intValue(), mstruct);
	return true;
}
AreaFunction::AreaFunction() : Function("area", 5) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(4, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(5, new MatrixArgument(""));	
}
bool AreaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[4].getArea(vargs[0].number().intValue(), vargs[1].number().intValue(), vargs[2].number().intValue(), vargs[3].number().intValue(), mstruct);
	return true;
}
TransposeFunction::TransposeFunction() : Function("transpose", 1) {
	setArgumentDefinition(1, new MatrixArgument());
}
bool TransposeFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	return mstruct.transposeMatrix();
}
IdentityFunction::IdentityFunction() : Function("identity", 1) {
	ArgumentSet *arg = new ArgumentSet();
	arg->addArgument(new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	arg->addArgument(marg);
	setArgumentDefinition(1, arg);
}
bool IdentityFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].isMatrix()) {
		if(vargs[0].rows() != vargs[0].columns()) {
			return false;
		}
		mstruct.setToIdentityMatrix(vargs[0].size());
	} else {
		mstruct.setToIdentityMatrix((unsigned int) vargs[0].number().intValue());
	}
	return true;
}
DeterminantFunction::DeterminantFunction() : Function("det", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
bool DeterminantFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[0].determinant(mstruct, eo);
	return !mstruct.isUndefined();
}
PermanentFunction::PermanentFunction() : Function("permanent", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
bool PermanentFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[0].permanent(mstruct, eo);
	return !mstruct.isUndefined();
}
CofactorFunction::CofactorFunction() : Function("cofactor", 3) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(3, new MatrixArgument());
}
bool CofactorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[2].cofactor(vargs[0].number().intValue(), vargs[1].number().intValue(), mstruct, eo);
	return !mstruct.isUndefined();
}
AdjointFunction::AdjointFunction() : Function("adj", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
bool AdjointFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct.adjointMatrix(eo);
	return !mstruct.isUndefined();
}
InverseFunction::InverseFunction() : Function("inverse", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
bool InverseFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct.invertMatrix(eo);
	return !mstruct.isUndefined();;
}

ZetaFunction::ZetaFunction() : Function("zeta", 1, 1, SIGN_ZETA) {
	setArgumentDefinition(1, new IntegerArgument());
}
bool ZetaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(zeta)
}
GammaFunction::GammaFunction() : Function("gamma", 1) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, false));
}
bool GammaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0]; 
	mstruct -= 1;
	mstruct.set(CALCULATOR->f_factorial, &vargs[0], NULL);
	mstruct[0] -= 1;
	return true;
}
BetaFunction::BetaFunction() : Function("beta", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, false));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, false));
}
bool BetaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0]; 
	mstruct.set(CALCULATOR->f_gamma, &vargs[0], NULL);
	MathStructure mstruct2(CALCULATOR->f_gamma, &vargs[1], NULL);
	mstruct *= mstruct2;
	mstruct2[0] += vargs[0];
	mstruct /= mstruct2;
	return true;
}

FactorialFunction::FactorialFunction() : Function("factorial", 1) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool FactorialFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(factorial)
}
BinomialFunction::BinomialFunction() : Function("binomial", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE, true, true));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE, true, true));
	setCondition("\\x>=\\y");
}
bool BinomialFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	Number nr;
	if(!nr.binomial(vargs[0].number(), vargs[1].number())) return false;
	mstruct = nr;
	return true;
}

AbsFunction::AbsFunction() : Function("abs", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
bool AbsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0]; 
	mstruct.eval(eo);
	if(mstruct.isNumber()) {
		Number nr = mstruct.number(); 
		if(!nr.abs() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {
			return false;
		} else {
			mstruct = nr; 
			return true;
		}
	} else if(mstruct.representsNegative()) {
		mstruct.negate();
		return true;
	} else if(mstruct.representsNonNegative()) {
		return true;
	}
	return false;
}
GcdFunction::GcdFunction() : Function("gcd", 2) {
	setArgumentDefinition(1, new IntegerArgument());
	setArgumentDefinition(2, new IntegerArgument());
}
bool GcdFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION_2(gcd)
}
SignumFunction::SignumFunction() : Function("sgn", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool SignumFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(signum)
}
CeilFunction::CeilFunction() : Function("ceil", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
bool CeilFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(ceil)
}
FloorFunction::FloorFunction() : Function("floor", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
bool FloorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(floor)
}
TruncFunction::TruncFunction() : Function("trunc", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
bool TruncFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(trunc)
}
RoundFunction::RoundFunction() : Function("round", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
bool RoundFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(round)
}
FracFunction::FracFunction() : Function("frac", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
bool FracFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(frac)
}
IntFunction::IntFunction() : Function("int", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
bool IntFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(trunc)
}
RemFunction::RemFunction() : Function("rem", 2) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(2)
}
bool RemFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION_2(rem)
}
ModFunction::ModFunction() : Function("mod", 2) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(2)
}
bool ModFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION_2(mod)
}

ImFunction::ImFunction() : Function("im", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool ImFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0].number().imaginaryPart();
	return true;
}
ReFunction::ReFunction() : Function("re", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool ReFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0].number().realPart();
	return true;
}
ArgFunction::ArgFunction() : Function("arg", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
bool ArgFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure m_re(CALCULATOR->f_re, &vargs[0], NULL);
	MathStructure m_im(CALCULATOR->f_re, &vargs[0], NULL);
	mstruct.set(CALCULATOR->f_atan, &m_im, &m_re, NULL);
	return true;
}

SqrtFunction::SqrtFunction() : Function("sqrt", 1, 1, SIGN_SQRT) {
}
bool SqrtFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct ^= MathStructure(1, 2);
	return true;
}
SquareFunction::SquareFunction() : Function("sq", 1) {
}
bool SquareFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct ^= 2;
	return true;
}

ExpFunction::ExpFunction() : Function("exp", 1) {
}
bool ExpFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = CALCULATOR->v_e;
	mstruct ^= vargs[0];
	return true;
}

LogFunction::LogFunction() : Function("ln", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false));
}
bool LogFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[0]; 
	
	if(eo.approximation == APPROXIMATION_TRY_EXACT) {
		EvaluationOptions eo2 = eo;
		eo2.approximation = APPROXIMATION_EXACT;
		CALCULATOR->beginTemporaryStopErrors();
		mstruct.eval(eo2);
		CALCULATOR->endTemporaryStopErrors();
	} else {
		mstruct.eval(eo);
	}
	bool b = false;
	if(mstruct.isVariable() && mstruct.variable() == CALCULATOR->v_e) {
		mstruct.set(1, 1);
		b = true;
	} else if(mstruct.isPower()) {
		if(mstruct[0].isVariable() && mstruct[0].variable() == CALCULATOR->v_e) {
			if(mstruct[1].representsReal()) {
				mstruct = mstruct[1];
				b = true;
			}
		} else if(mstruct[1].representsPositive() || (mstruct[1].representsNegative() && mstruct[0].representsPositive())) {
			MathStructure mstruct2;
			mstruct2.set(CALCULATOR->f_ln, &mstruct[0], NULL);
			mstruct2 *= mstruct[1];
			mstruct = mstruct2;
			b = true;
		}
	} else if(mstruct.isMultiplication()) {
		b = true;
		for(unsigned int i = 0; i < mstruct.size(); i++) {
			if(!mstruct[i].representsPositive()) {
				b = false;
				break;
			}
		}
		if(b) {
			MathStructure mstruct2;
			mstruct2.set(CALCULATOR->f_ln, &mstruct[0], NULL);
			for(unsigned int i = 1; i < mstruct.size(); i++) {
				mstruct2.add(MathStructure(CALCULATOR->f_ln, &mstruct[i], NULL), i > 1);
			}
			mstruct = mstruct2;
		}
	}
	if(b) {
		if(eo.approximation == APPROXIMATION_TRY_EXACT) {
			EvaluationOptions eo2 = eo;
			eo2.approximation = APPROXIMATION_EXACT;
			MathStructure mstruct2 = vargs[0];
			mstruct2.eval(eo2);
		}
		return true;
	}
	if(eo.approximation == APPROXIMATION_TRY_EXACT && !mstruct.isNumber()) {
		EvaluationOptions eo2 = eo;
		eo2.approximation = APPROXIMATION_APPROXIMATE;
		mstruct = vargs[0];
		mstruct.eval(eo2);
	}
	if(mstruct.isNumber()) {
		if(mstruct.number().isMinusOne()) {
			mstruct = CALCULATOR->v_i->get();
			mstruct *= CALCULATOR->v_pi;
			return true;
		} else if(mstruct.number().isI() || mstruct.number().isMinusI()) {
			mstruct = Number(1, 2);
			mstruct *= CALCULATOR->v_pi;
			mstruct *= CALCULATOR->v_i->get();
			return true;
		} else if(mstruct.number().isMinusInfinity()) {
			mstruct = CALCULATOR->v_pi;
			mstruct *= CALCULATOR->v_i->get();
			Number nr; nr.setPlusInfinity();
			mstruct += nr;
			return true;
		}
		Number nr(mstruct.number());
		if(nr.ln() && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {
			mstruct = nr;
			return true;
		}
	}
	return false;
	
}
LognFunction::LognFunction() : Function("log", 1, 2) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false));
	setArgumentDefinition(2, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false));
	setDefaultValue(2, "e");
}
bool LognFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	if(vargs[1].isVariable() && vargs[1].variable() == CALCULATOR->v_e) {
		mstruct = vargs[0]; 
		if(eo.approximation == APPROXIMATION_TRY_EXACT) {
			EvaluationOptions eo2 = eo;
			eo2.approximation = APPROXIMATION_EXACT;
			CALCULATOR->beginTemporaryStopErrors();
			mstruct.eval(eo2);
			CALCULATOR->endTemporaryStopErrors();
		} else {
			mstruct.eval(eo);
		}
		bool b = false;
		if(mstruct.isVariable() && mstruct.variable() == CALCULATOR->v_e) {
			mstruct.set(1, 1);
			b = true;
		} else if(mstruct.isPower()) {
			if(mstruct[0].isVariable() && mstruct[0].variable() == CALCULATOR->v_e) {
				if(mstruct[1].representsReal()) {
					mstruct = mstruct[1];
					b = true;
				}
			} else if(mstruct[1].representsPositive() || (mstruct[1].representsNegative() && mstruct[0].representsPositive())) {
				MathStructure mstruct2;
				mstruct2.set(CALCULATOR->f_ln, &mstruct[0], NULL);
				mstruct2 *= mstruct[1];
				mstruct = mstruct2;
				b = true;
			}
		} else if(mstruct.isMultiplication()) {
			b = true;
			for(unsigned int i = 0; i < mstruct.size(); i++) {
				if(!mstruct[i].representsPositive()) {
					b = false;
					break;
				}
			}
			if(b) {
				MathStructure mstruct2;
				mstruct2.set(CALCULATOR->f_ln, &mstruct[0], NULL);
				for(unsigned int i = 1; i < mstruct.size(); i++) {
					mstruct2.add(MathStructure(CALCULATOR->f_ln, &mstruct[i], NULL), i > 1);
				}
				mstruct = mstruct2;
			}
		}
		if(b) {
			if(eo.approximation == APPROXIMATION_TRY_EXACT) {
				EvaluationOptions eo2 = eo;
				eo2.approximation = APPROXIMATION_EXACT;
				MathStructure mstruct2 = vargs[0];
				mstruct2.eval(eo2);
			}
			return true;
		}
		if(eo.approximation == APPROXIMATION_TRY_EXACT && !mstruct.isNumber()) {
			EvaluationOptions eo2 = eo;
			eo2.approximation = APPROXIMATION_APPROXIMATE;
			mstruct = vargs[0];
			mstruct.eval(eo2);
		}
		if(mstruct.isNumber()) {
			if(mstruct.number().isMinusOne()) {
				mstruct = CALCULATOR->v_i->get();
				mstruct *= CALCULATOR->v_pi;
				return true;
			} else if(mstruct.number().isI() || mstruct.number().isMinusI()) {
				mstruct = Number(1, 2);
				mstruct *= CALCULATOR->v_pi;
				mstruct *= CALCULATOR->v_i->get();
				return true;
			} else if(mstruct.number().isMinusInfinity()) {
				mstruct = CALCULATOR->v_pi;
				mstruct *= CALCULATOR->v_i->get();
				Number nr; nr.setPlusInfinity();
				mstruct += nr;
				return true;
			}
			Number nr(mstruct.number());
			if(nr.ln() && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {
				mstruct = nr;
				return true;
			}
		}	
		return false;
	}
	mstruct = vargs[0];
	mstruct.eval(eo);
	MathStructure mstructv2 = vargs[1];
	mstructv2.eval(eo);
	if(mstruct.isPower()) {
		if(mstruct[1].representsPositive() || (mstruct[1].representsNegative() && mstruct[0].representsPositive())) {
			MathStructure mstruct2;
			mstruct2.set(CALCULATOR->f_logn, &mstruct[0], &mstructv2, NULL);
			mstruct2 *= mstruct[1];
			mstruct = mstruct2;
			return true;
		}
	} else if(mstruct.isMultiplication()) {
		bool b = true;
		for(unsigned int i = 0; i < mstruct.size(); i++) {
			if(!mstruct[i].representsPositive()) {
				b = false;
				break;
			}
		}
		if(b) {
			MathStructure mstruct2;
			mstruct2.set(CALCULATOR->f_logn, &mstruct[0], &mstructv2, NULL);
			for(unsigned int i = 1; i < mstruct.size(); i++) {
				mstruct2.add(MathStructure(CALCULATOR->f_logn, &mstruct[i], &mstructv2, NULL), i > 1);
			}
			mstruct = mstruct2;
			return true;
		}
	} else if(vargs[0].isNumber() && vargs[1].isNumber()) {
		Number nr(mstruct.number());
		if(nr.log(vargs[1].number()) && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {
			mstruct = nr;
			return true;
		}
	} 
	mstruct.set(CALCULATOR->f_ln, &vargs[0], NULL);
	mstruct /= MathStructure(CALCULATOR->f_ln, &vargs[1], NULL);
	return true;
}

SinFunction::SinFunction() : Function("sin", 1) {
	setArgumentDefinition(1, new AngleArgument());
}
bool SinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[0]; 
	if(CALCULATOR->u_rad) mstruct /= CALCULATOR->u_rad;
	if(eo.approximation == APPROXIMATION_TRY_EXACT) {
		EvaluationOptions eo2 = eo;
		eo2.approximation = APPROXIMATION_EXACT;
		CALCULATOR->beginTemporaryStopErrors();
		mstruct.eval(eo2);
		CALCULATOR->endTemporaryStopErrors();
	} else {
		mstruct.eval(eo);
	}
	bool b = false;
	if(mstruct.isVariable() && mstruct.variable() == CALCULATOR->v_pi) {
		mstruct.clear();
		b = true;
	} else if(mstruct.isFunction() && mstruct.size() == 1) {
		if(mstruct.function() == CALCULATOR->f_asin) {
			MathStructure mstruct_new(mstruct[0]);
			mstruct = mstruct_new;
			b = true;
		}
	} else if(mstruct.isMultiplication() && mstruct.size() == 2 && mstruct[0].isNumber() && mstruct[1].isVariable() && mstruct[1].variable() == CALCULATOR->v_pi) {
		if(mstruct[0].number().isInteger()) {
			mstruct.clear();
			b = true;
		} else if(!mstruct[0].number().isComplex() && !mstruct[0].number().isInfinite()) {
			if(mstruct[0].number().equals(Number(1, 2))) {
				mstruct = 1;
				b = true;
			} else if(mstruct[0].number().equals(Number(-1, 2))) {
				mstruct = -1;
				b = true;
			} else if(mstruct[0].number().equals(Number(1, 4))) {
				mstruct.set(2);
				mstruct ^= MathStructure(1, 2);
				mstruct /= 2;
				b = true;
			} else if(mstruct[0].number().equals(Number(-1, 4))) {
				mstruct.set(2);
				mstruct ^= MathStructure(1, 2);
				mstruct /= 2;
				mstruct.negate();
				b = true;
			} else if(mstruct[0].number().equals(Number(1, 3))) {
				mstruct.set(3);
				mstruct ^= MathStructure(1, 2);
				mstruct /= 2;
				b = true;
			} else if(mstruct[0].number().equals(Number(-1, 3))) {
				mstruct.set(3);
				mstruct ^= MathStructure(1, 2);
				mstruct /= 2;
				mstruct.negate();
				b = true;
			} else if(mstruct[0].number().equals(Number(1, 6))) {
				mstruct.set(1, 2);
				b = true;
			} else if(mstruct[0].number().equals(Number(-1, 6))) {
				mstruct.set(-1, 2);
				b = true;
			}
		}
	} else if(mstruct.isAddition()) {
		unsigned int i = 0;
		for(; i < mstruct.size(); i++) {
			if(mstruct[i] == CALCULATOR->v_pi || (mstruct[i].isMultiplication() && mstruct[i].size() == 2 && mstruct[i][1] == CALCULATOR->v_pi && mstruct[i][0].isNumber() && mstruct[i][0].number().isInteger())) {
				b = true;
				break;
			}
		}
		if(b) {
			MathStructure mstruct2;
			for(unsigned int i2 = 0; i2 < mstruct.size(); i2++) {
				if(i2 != i) {
					if(mstruct2.isZero()) {
						mstruct2 = mstruct[i2];
					} else {
						mstruct2.add(mstruct[i2], true);
					}
				}
			}
			mstruct.set(CALCULATOR->f_sin, &mstruct2, NULL);
		}
	}
	if(b) {
		if(eo.approximation == APPROXIMATION_TRY_EXACT) {
			EvaluationOptions eo2 = eo;
			eo2.approximation = APPROXIMATION_EXACT;
			MathStructure mstruct2 = vargs[0];
			if(CALCULATOR->u_rad) mstruct2 /= CALCULATOR->u_rad;
			mstruct2.eval(eo2);
		}
		return true;
	}
	if(eo.approximation == APPROXIMATION_TRY_EXACT && !mstruct.isNumber()) {
		EvaluationOptions eo2 = eo;
		eo2.approximation = APPROXIMATION_APPROXIMATE;
		mstruct = vargs[0];
		if(CALCULATOR->u_rad) mstruct /= CALCULATOR->u_rad;
		mstruct.eval(eo2);
	}
	if(mstruct.isNumber()) {
		Number nr(mstruct.number());
		if(nr.sin() && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {
			mstruct = nr;
			return true;
		}
	}
	if(mstruct.isNegate()) {
		MathStructure mstruct2(CALCULATOR->f_sin, &mstruct[0], NULL);
		mstruct = mstruct2;
		mstruct.negate();
		return true;
	}
	return false;
}
CosFunction::CosFunction() : Function("cos", 1) {
	setArgumentDefinition(1, new AngleArgument());
}
bool CosFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[0]; 
	if(CALCULATOR->u_rad) mstruct /= CALCULATOR->u_rad;
	if(eo.approximation == APPROXIMATION_TRY_EXACT) {
		EvaluationOptions eo2 = eo;
		eo2.approximation = APPROXIMATION_EXACT;
		CALCULATOR->beginTemporaryStopErrors();
		mstruct.eval(eo2);
		CALCULATOR->endTemporaryStopErrors();
	} else {
		mstruct.eval(eo);
	}
	bool b = false;
	if(mstruct.isVariable() && mstruct.variable() == CALCULATOR->v_pi) {
		mstruct = -1;
		b = true;
	} else if(mstruct.isFunction() && mstruct.size() == 1) {
		if(mstruct.function() == CALCULATOR->f_acos) {
			MathStructure mstruct_new(mstruct[0]);
			mstruct = mstruct_new;
			b = true;
		}
	} else if(mstruct.isMultiplication() && mstruct.size() == 2 && mstruct[0].isNumber() && mstruct[1].isVariable() && mstruct[1].variable() == CALCULATOR->v_pi) {
		if(mstruct[0].number().isInteger()) {
			if(mstruct[0].number().numeratorIsEven()) {
				mstruct = -1;
			} else {
				mstruct = 1;
			}
			b = true;
		} else if(!mstruct[0].number().isComplex() && !mstruct[0].number().isInfinite()) {
			Number nr(mstruct[0].number());
			nr.setNegative(false);
			if(mstruct[0].number().equals(Number(1, 2))) {
				mstruct.clear();
				b = true;
			} else if(mstruct[0].number().equals(Number(1, 4))) {
				mstruct.set(2);
				mstruct ^= MathStructure(1, 2);
				mstruct /= 2;
				b = true;
			} else if(mstruct[0].number().equals(Number(1, 6))) {
				mstruct.set(3);
				mstruct ^= MathStructure(1, 2);
				mstruct /= 2;
				b = true;
			} else if(mstruct[0].number().equals(Number(1, 3))) {
				mstruct.set(1, 2);
				b = true;
			}
		}
	} else if(mstruct.isAddition()) {
		unsigned int i = 0;
		for(; i < mstruct.size(); i++) {
			if(mstruct[i] == CALCULATOR->v_pi || (mstruct[i].isMultiplication() && mstruct[i].size() == 2 && mstruct[i][1] == CALCULATOR->v_pi && mstruct[i][0].isNumber() && mstruct[i][0].number().isInteger())) {
				b = true;
				break;
			}
		}
		if(b) {
			MathStructure mstruct2;
			for(unsigned int i2 = 0; i2 < mstruct.size(); i2++) {
				if(i2 != i) {
					if(mstruct2.isZero()) {
						mstruct2 = mstruct[i2];
					} else {
						mstruct2.add(mstruct[i2], true);
					}
				}
			}
			mstruct.set(CALCULATOR->f_cos, &mstruct2, NULL);
			mstruct.negate();
		}
	}
	if(b) {
		if(eo.approximation == APPROXIMATION_TRY_EXACT) {
			EvaluationOptions eo2 = eo;
			eo2.approximation = APPROXIMATION_EXACT;
			MathStructure mstruct2 = vargs[0];
			if(CALCULATOR->u_rad) mstruct2 /= CALCULATOR->u_rad;
			mstruct2.eval(eo2);
		}
		return true;
	}
	if(eo.approximation == APPROXIMATION_TRY_EXACT && !mstruct.isNumber()) {
		EvaluationOptions eo2 = eo;
		eo2.approximation = APPROXIMATION_APPROXIMATE;
		mstruct = vargs[0];
		if(CALCULATOR->u_rad) mstruct /= CALCULATOR->u_rad;
		mstruct.eval(eo2);
	}
	if(mstruct.isNumber()) {
		Number nr(mstruct.number());
		if(nr.cos() && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {
			mstruct = nr;
			return true;
		}
	}
	return false;
}
TanFunction::TanFunction() : Function("tan", 1) {
	setArgumentDefinition(1, new AngleArgument());
}
bool TanFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct.set(CALCULATOR->f_sin, &vargs[0], NULL);
	mstruct /= MathStructure(CALCULATOR->f_cos, &vargs[0], NULL);
	return true;
}
AsinFunction::AsinFunction() : Function("asin", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool AsinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	if(vargs[0].number().isZero()) {
		mstruct.clear();
	} else if(vargs[0].number().isOne()) {
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct.set(90, 1);
				break;
			}
			case GRADIANS: {
				mstruct.set(100, 1);
				break;
			}
			default: {
				mstruct.set(1, 2);
				mstruct *= CALCULATOR->v_pi;
			}
		}
	} else if(vargs[0].number().isMinusOne()) {
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct.set(-90, 1);
				break;
			}
			case GRADIANS: {
				mstruct.set(-100, 1);
				break;
			}
			default: {
				mstruct.set(-1, 2);
				mstruct *= CALCULATOR->v_pi;
			}
		}
	} else if(vargs[0].number().equals(Number(1, 2))) {
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct.set(30, 1);
				break;
			}
			case GRADIANS: {
				mstruct.set(100, 3);
				break;
			}
			default: {
				mstruct.set(1, 6);
				mstruct *= CALCULATOR->v_pi;
			}
		}
	} else {
		Number nr = vargs[0].number();
		if(!nr.asin() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) return false;
		mstruct = nr;
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct *= 180;
		    		mstruct /= CALCULATOR->v_pi;
				break;
			}
			case GRADIANS: {
				mstruct *= 200;
	    			mstruct /= CALCULATOR->v_pi;
				break;
			}
		}
	}
	return true;
	
}
AcosFunction::AcosFunction() : Function("acos", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool AcosFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	if(vargs[0].number().isZero()) {
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct.set(90, 1);
				break;
			}
			case GRADIANS: {
				mstruct.set(100, 1);
				break;
			}
			default: {
				mstruct.set(1, 2);
				mstruct *= CALCULATOR->v_pi;
			}
		}
	} else if(vargs[0].number().isOne()) {
		mstruct.clear();
	} else if(vargs[0].number().isMinusOne()) {
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct.set(180, 1);
				break;
			}
			case GRADIANS: {
				mstruct.set(200, 1);
				break;
			}
			default: {
				mstruct = CALCULATOR->v_pi;
			}
		}
	} else if(vargs[0].number().equals(Number(1, 2))) {
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct.set(60, 1);
				break;
			}
			case GRADIANS: {
				mstruct.set(200, 3);
				break;
			}
			default: {
				mstruct.set(1, 3);
				mstruct *= CALCULATOR->v_pi;
			}
		}
	} else {
		Number nr = vargs[0].number();
		if(!nr.acos() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) return false;
		mstruct = nr;
		switch(CALCULATOR->angleMode()) {
			case DEGREES: {
				mstruct *= 180;
		    		mstruct /= CALCULATOR->v_pi;
				break;
			}
			case GRADIANS: {
				mstruct *= 200;
	    			mstruct /= CALCULATOR->v_pi;
				break;
			}
		}
	}
	return true;
	
}
AtanFunction::AtanFunction() : Function("atan", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool AtanFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	if(vargs[0].number().isZero()) {
		mstruct.clear();
	} else if(vargs[0].number().isI()) {
		mstruct = vargs[0];
		Number nr; nr.setInfinity();
		mstruct *= nr;
	} else if(vargs[0].number().isMinusI()) {
		mstruct = vargs[0];
		Number nr; nr.setInfinity();
		mstruct *= nr;
	} else if(vargs[0].number().isPlusInfinity()) {
		mstruct.set(1, 2);
		mstruct *= CALCULATOR->v_pi;
	} else if(vargs[0].number().isMinusInfinity()) {
		mstruct.set(-1, 2);
		mstruct *= CALCULATOR->v_pi;
	} else {
		Number nr = vargs[0].number();
		if(!nr.atan() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) return false;
		mstruct = nr;
	}
	return true;
	
}
SinhFunction::SinhFunction() : Function("sinh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
bool SinhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(sinh)
}
CoshFunction::CoshFunction() : Function("cosh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));	
}
bool CoshFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(cosh)
}
TanhFunction::TanhFunction() : Function("tanh", 1) {}
bool TanhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct.set(CALCULATOR->f_sinh, &vargs[0], NULL);
	mstruct /= MathStructure(CALCULATOR->f_cosh, &vargs[0], NULL);
	return true;
}
AsinhFunction::AsinhFunction() : Function("asinh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
bool AsinhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure m_arg(vargs[0]);
	m_arg ^= 2;
	m_arg += 1;
	m_arg ^= Number(1, 2);
	m_arg += vargs[0];
	mstruct.set(CALCULATOR->f_ln, &m_arg, NULL);
	return true;
}
AcoshFunction::AcoshFunction() : Function("acosh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
bool AcoshFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure m_arg(vargs[0]);
	m_arg ^= 2;
	m_arg -= 1;
	m_arg ^= Number(1, 2);
	m_arg += vargs[0];
	mstruct.set(CALCULATOR->f_ln, &m_arg, NULL);
	return true;
}
AtanhFunction::AtanhFunction() : Function("atanh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
bool AtanhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	MathStructure m_arg = 1;
	m_arg += vargs[0];
	MathStructure m_den = 1;
	m_den -= vargs[0];
	m_arg /= m_den;
	mstruct.set(CALCULATOR->f_ln, &m_arg, NULL);
	mstruct *= Number(1, 2);
	return true;
	
}

RadiansToDefaultAngleUnitFunction::RadiansToDefaultAngleUnitFunction() : Function("radtodef", 1) {
}
bool RadiansToDefaultAngleUnitFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	switch(CALCULATOR->angleMode()) {
		case DEGREES: {
			mstruct *= 180;
	    		mstruct /= CALCULATOR->v_pi;
			break;
		}
		case GRADIANS: {
			mstruct *= 200;
	    		mstruct /= CALCULATOR->v_pi;
			break;
		}
	}
	return true;
}

TotalFunction::TotalFunction() : Function("total", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
bool TotalFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct.clear();
	for(unsigned int index = 0; index < vargs[0].size(); index++) {
		mstruct.add(vargs[0][index], true);
	}
	return true;
}
PercentileFunction::PercentileFunction() : Function("percentile", 2) {
	NumberArgument *arg = new NumberArgument();
	Number fr;
	arg->setMin(&fr);
	fr.set(99, 1);
	arg->setMax(&fr);
	arg->setIncludeEqualsMin(false);
	arg->setIncludeEqualsMax(false);
	setArgumentDefinition(1, arg);
	setArgumentDefinition(2, new VectorArgument(""));
}
bool PercentileFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure v(vargs[1]);
	MathStructure *mp;
	Number fr100(100);
	if(!v.sortVector()) {
		return false;
	} else {
		Number pfr(vargs[0].number());		
		pfr /= 100;
		pfr *= v.components() + 1;
/*		Number cfr(v->components());		
		if(pfr.isZero() || pfr.numerator()->isLessThan(pfr.denominator()) || pfr.isGreaterThan(&cfr)) {
			CALCULATOR->error(true, _("Not enough samples."), NULL);
		}*/
		if(pfr.isInteger()) {
			mp = v.getComponent(pfr.intValue());
			if(!mp) return false;
			mstruct = *mp;
		} else {
			Number ufr(pfr);
			ufr.ceil();
			Number lfr(pfr);
			lfr.floor();
			pfr -= lfr;
			mp = v.getComponent(ufr.intValue());
			if(!mp) return false;
			MathStructure gap(*mp);
			mp = v.getComponent(lfr.intValue());
			if(!mp) return false;
			gap -= *mp;
			gap *= pfr;
			mp = v.getComponent(lfr.intValue());
			if(!mp) return false;
			mstruct = *mp;
			mstruct += gap;
		}
	}
	return true;
}
MinFunction::MinFunction() : Function("min", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
bool MinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	ComparisonResult cmp;
	const MathStructure *min = NULL;
	vector<const MathStructure*> unsolveds;
	bool b = false;
	for(unsigned int index = 0; index < vargs[0].size(); index++) {
		if(min == NULL) {
			min = &vargs[0][index];
		} else {
			cmp = min->compare(vargs[0][index]);
			if(cmp == COMPARISON_RESULT_LESS) {
				min = &vargs[0][index];
				b = true;
			} else if(COMPARISON_NOT_FULLY_KNOWN(cmp)) {
				if(CALCULATOR->showArgumentErrors()) {
					CALCULATOR->error(true, _("Unsolvable comparison in %s()."), name().c_str(), NULL);
				}
				unsolveds.push_back(&vargs[0][index]);
			} else {
				b = true;
			}
		}
	}
	if(min) {
		if(unsolveds.size() > 0) {
			if(!b) return false;
			MathStructure margs; margs.clearVector();
			margs.addItem(*min);
			for(unsigned int i = 0; i < unsolveds.size(); i++) {
				margs.addItem(*unsolveds[i]);
			}
			mstruct.set(this, &margs, NULL);
			return true;
		} else {
			mstruct = *min;
			return true;
		}
	}
	return false;
}
MaxFunction::MaxFunction() : Function("max", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
bool MaxFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	ComparisonResult cmp;
	const MathStructure *max = NULL;
	vector<const MathStructure*> unsolveds;
	bool b = false;
	for(unsigned int index = 0; index < vargs[0].size(); index++) {
		if(max == NULL) {
			max = &vargs[0][index];
		} else {
			cmp = max->compare(vargs[0][index]);
			if(cmp == COMPARISON_RESULT_GREATER) {
				max = &vargs[0][index];
				b = true;
			} else if(COMPARISON_NOT_FULLY_KNOWN(cmp)) {
				if(CALCULATOR->showArgumentErrors()) {
					CALCULATOR->error(true, _("Unsolvable comparison in %s()."), name().c_str(), NULL);
				}
				unsolveds.push_back(&vargs[0][index]);
			} else {
				b = true;
			}
		}
	}
	if(max) {
		if(unsolveds.size() > 0) {
			if(!b) return false;
			MathStructure margs; margs.clearVector();
			margs.addItem(*max);
			for(unsigned int i = 0; i < unsolveds.size(); i++) {
				margs.addItem(*unsolveds[i]);
			}
			mstruct.set(this, &margs, NULL);
			return true;
		} else {
			mstruct = *max;
			return true;
		}
	}
	return false;
}
ModeFunction::ModeFunction() : Function("mode", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
bool ModeFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].size() <= 0) {
		return false;
	}
	int n = 0;
	bool b;
	vector<const MathStructure*> vargs_nodup;
	vector<int> is;
	const MathStructure *value = NULL;
	for(unsigned int index_c = 0; index_c < vargs[0].size(); index_c++) {
		b = true;
		for(unsigned int index = 0; index < vargs_nodup.size(); index++) {
			if(vargs_nodup[index]->equals(vargs[0][index_c])) {
				is[index]++;
				b = false;
				break;
			}
		}
		if(b) {
			vargs_nodup.push_back(&vargs[0][index_c]);
			is.push_back(1);
		}
	}
	for(unsigned int index = 0; index < is.size(); index++) {
		if(is[index] > n) {
			n = is[index];
			value = vargs_nodup[index];
		}
	}
	if(value) {
		mstruct = *value;
		return true;
	}
	return false;
}
RandFunction::RandFunction() : Function("rand", 0, 1) {
	setArgumentDefinition(1, new IntegerArgument());
	setDefaultValue(1, "-1"); 
}
bool RandFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].number().isNegative()) {
		Number nr;
		nr.setInternal(cln::random_F(cln::cl_float(1)));
		mstruct = nr;
	} else {
		Number nr;
		nr.setInternal(cln::random_I(cln::numerator(cln::rational(cln::realpart(vargs[0].number().internalNumber())))));
		mstruct = nr;
	}
	return true;
}

DaysFunction::DaysFunction() : Function("days", 2, 4) {
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
bool DaysFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int days = daysBetweenDates(vargs[0].symbol(), vargs[1].symbol(), vargs[2].number().intValue(), vargs[3].number().isZero());
	if(days < 0) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		return false;
	}
	mstruct.set(days, 1, 0);
	return true;			
}
YearFracFunction::YearFracFunction() : Function("yearfrac", 2, 4) {
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
bool YearFracFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	Number yfr = yearsBetweenDates(vargs[0].symbol(), vargs[1].symbol(), vargs[2].number().intValue(), vargs[3].number().isZero());
	if(yfr.isMinusOne()) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		return false;
	}
	mstruct.set(yfr);
	return true;
}
WeekFunction::WeekFunction() : Function("week", 0, 2) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new BooleanArgument());	
	setDefaultValue(1, "\"today\"");
}
bool WeekFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int w = week(vargs[0].symbol(), vargs[1].number().getBoolean());
	if(w < 0) {
		return false;
	}
	mstruct.set(w, 1);
	return true;
}
WeekdayFunction::WeekdayFunction() : Function("weekday", 0, 2) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new BooleanArgument());
	setDefaultValue(1, "\"today\"");
}
bool WeekdayFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int w = weekday(vargs[0].symbol());
	if(w < 0) {
		return false;
	}
	if(vargs[1].number().getBoolean()) {
		if(w == 7) w = 1;
		else w++;
	}
	mstruct.set(w, 1);
	return true;
}
YeardayFunction::YeardayFunction() : Function("yearday", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
bool YeardayFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int d = yearday(vargs[0].symbol());
	if(d < 0) {
		return false;
	}
	mstruct.set(d, 1);
	return true;
}
MonthFunction::MonthFunction() : Function("month", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
bool MonthFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int year, month, day;
	if(!s2date(vargs[0].symbol(), year, month, day)) {
		return false;
	}
	mstruct.set(month, 1);
	return true;
}
DayFunction::DayFunction() : Function("day", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
bool DayFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int year, month, day;
	if(!s2date(vargs[0].symbol(), year, month, day)) {
		return false;
	}
	mstruct.set(day, 1);
	return true;
}
YearFunction::YearFunction() : Function("year", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "\"today\"");
}
bool YearFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int year, month, day;
	if(!s2date(vargs[0].symbol(), year, month, day)) {
		return false;
	}
	mstruct.set(year, 1);
	return true;
}
TimeFunction::TimeFunction() : Function("time", 0) {
}
bool TimeFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int hour, min, sec;
	now(hour, min, sec);
	Number tnr(sec, 1);
	tnr /= 60;
	tnr += min;
	tnr /= 60;
	tnr += hour;
	mstruct = tnr;
	return true;
}

BinFunction::BinFunction() : Function("bin", 1) {
	setArgumentDefinition(1, new TextArgument());
}
bool BinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = Number(vargs[0].symbol(), 2);
	return true;
}
OctFunction::OctFunction() : Function("oct", 1) {
	setArgumentDefinition(1, new TextArgument());
}
bool OctFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = Number(vargs[0].symbol(), 8);
	return true;
}
HexFunction::HexFunction() : Function("hex", 1) {
	setArgumentDefinition(1, new TextArgument());
}
bool HexFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = Number(vargs[0].symbol(), 16);
	return true;
}
BaseFunction::BaseFunction() : Function("base", 2) {
	setArgumentDefinition(1, new TextArgument());
	IntegerArgument *arg = new IntegerArgument();
	Number integ(1, 1);
	arg->setMin(&integ);
	integ.set(36, 1);
	arg->setMax(&integ);
	setArgumentDefinition(2, arg);
}
bool BaseFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = Number(vargs[0].symbol(), vargs[1].number().intValue());
	return true;
}
RomanFunction::RomanFunction() : Function("roman", 1) {
	setArgumentDefinition(1, new TextArgument());
}
bool RomanFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = Number(vargs[0].symbol(), BASE_ROMAN_NUMERALS);
	return true;
}

AsciiFunction::AsciiFunction() : Function("code", 1) {
	TextArgument *arg = new TextArgument();
	arg->setCustomCondition("len(\\x) = 1");
	setArgumentDefinition(1, arg);
}
bool AsciiFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	unsigned char c = (unsigned char) vargs[0].symbol()[0];
	mstruct.set(c, 1);
	return true;
}
CharFunction::CharFunction() : Function("char", 1) {
	IntegerArgument *arg = new IntegerArgument();
	Number fr(32, 0);
	arg->setMin(&fr);
	fr.set(0x7f, 1);
	arg->setMax(&fr);
	setArgumentDefinition(1, arg);
}
bool CharFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	string str;
	str += vargs[0].number().intValue();
	mstruct = str;
	return true;
}

ConcatenateFunction::ConcatenateFunction() : Function("concatenate", 1) {
	VectorArgument *arg = new VectorArgument();
	arg->addArgument(new TextArgument());
	setArgumentDefinition(1, arg);	
}
bool ConcatenateFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	string str;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		str += vargs[i].symbol();
	}
	mstruct = str;
	return true;
}
LengthFunction::LengthFunction() : Function("len", 1) {
	setArgumentDefinition(1, new TextArgument());
}
bool LengthFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].symbol().length();
	return true;
}

ReplaceFunction::ReplaceFunction() : Function("replace", 3) {
}
bool ReplaceFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct.replace(vargs[1], vargs[2]);
	return true;
}

ErrorFunction::ErrorFunction() : Function("error", 1) {
	setArgumentDefinition(1, new TextArgument());
}
bool ErrorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	CALCULATOR->error(true, vargs[0].symbol().c_str(), NULL);
	return true;
}
WarningFunction::WarningFunction() : Function("warning", 1) {
	setArgumentDefinition(1, new TextArgument());
}
bool WarningFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	CALCULATOR->error(false, vargs[0].symbol().c_str(), NULL);
	return true;
}

ForFunction::ForFunction() : Function("for", 7) {
	setArgumentDefinition(2, new SymbolicArgument());	
	setArgumentDefinition(7, new SymbolicArgument());
}
bool ForFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[6];
	MathStructure mcounter = vargs[0];
	MathStructure mtest;
	MathStructure mcount;
	MathStructure mupdate;
	while(true) {
		mtest = vargs[3];
		mtest.replace(vargs[1], mcounter);
		mtest.eval(eo);
		if(!mtest.isNumber()) return false;
		if(!mtest.number().getBoolean()) {
			break;
		}
		
		mupdate = vargs[4];
		mupdate.replace(vargs[1], mcounter);
		mupdate.replace(vargs[5], mstruct);
		mstruct = mupdate;
		
		mcount = vargs[2];
		mcount.replace(vargs[1], mcounter);
		mcounter = mcount;
	}
	return true;

}
SumFunction::SumFunction() : Function("sum", 3, 4, SIGN_CAPITAL_SIGMA) {
	setArgumentDefinition(2, new IntegerArgument());
	setArgumentDefinition(3, new IntegerArgument());	
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "x");
	setCondition("\\z >= \\y");
}
bool SumFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct.clear();
	Number i_nr(vargs[1].number());
	MathStructure mstruct_calc;
	bool started = false, s2 = false;
	while(i_nr.isLessThanOrEqualTo(vargs[2].number())) {	
		mstruct_calc.set(vargs[0]);
		mstruct_calc.replace(vargs[3], i_nr);
		if(started) {
			mstruct.add(mstruct_calc, s2);
			s2 = true;
		} else {
			mstruct = mstruct_calc;
			started = true;
		}
		i_nr += 1;
	}
	return true;
	
}
ProductFunction::ProductFunction() : Function("product", 3, 4, SIGN_CAPITAL_PI) {
	setArgumentDefinition(2, new IntegerArgument());
	setArgumentDefinition(3, new IntegerArgument());	
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "x");
	setCondition("\\z >= \\y");
}
bool ProductFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct.clear();
	Number i_nr(vargs[1].number());
	MathStructure mstruct_calc;
	bool started = false, s2 = false;
	while(i_nr.isLessThanOrEqualTo(vargs[2].number())) {	
		mstruct_calc.set(vargs[0]);
		mstruct_calc.replace(vargs[3], i_nr);
		if(started) {
			mstruct.multiply(mstruct_calc, s2);
			s2 = true;
		} else {
			mstruct = mstruct_calc;
			started = true;
		}
		i_nr += 1;
	}
	return true;
	
}

ProcessFunction::ProcessFunction() : Function("process", 3, 5) {
	setArgumentDefinition(2, new SymbolicArgument());
	setArgumentDefinition(3, new VectorArgument());
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "\"\"");
	setArgumentDefinition(5, new SymbolicArgument());
	setDefaultValue(5, "\"\"");
}
bool ProcessFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[2];
	MathStructure mprocess;
	for(unsigned int index = 0; index < mstruct.size(); index++) {
		mprocess = vargs[0];
		mprocess.replace(vargs[1], mstruct[index]);
		if(!vargs[3].isEmptySymbol()) {
			mprocess.replace(vargs[3], (int) index);
		}
		if(!vargs[4].isEmptySymbol()) {
			mprocess.replace(vargs[4], vargs[2]);
		}
		mstruct[index] = mprocess;
	}
	return true;
	
}
ProcessMatrixFunction::ProcessMatrixFunction() : Function("processm", 3, 6) {
	setArgumentDefinition(2, new SymbolicArgument());
	setArgumentDefinition(3, new MatrixArgument());
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "\"\"");
	setArgumentDefinition(5, new SymbolicArgument());
	setDefaultValue(5, "\"\"");
	setArgumentDefinition(6, new SymbolicArgument());
	setDefaultValue(6, "\"\"");
}
bool ProcessMatrixFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[2];
	MathStructure mprocess;
	for(unsigned int rindex = 0; rindex < mstruct.size(); rindex++) {
		for(unsigned int cindex = 0; cindex < mstruct[rindex].size(); cindex++) {
			mprocess = vargs[0];
			mprocess.replace(vargs[1], mstruct[rindex][cindex]);
			if(!vargs[3].isEmptySymbol()) {
				mprocess.replace(vargs[3], (int) rindex);
			}
			if(!vargs[4].isEmptySymbol()) {
				mprocess.replace(vargs[4], (int) cindex);
			}
			if(!vargs[5].isEmptySymbol()) {
				mprocess.replace(vargs[5], vargs[2]);
			}
			mstruct[rindex][cindex] = mprocess;
		}
	}
	return true;
	
}
CustomSumFunction::CustomSumFunction() : Function("csum", 7, 9) {
	setArgumentDefinition(1, new IntegerArgument()); //begin
	setArgumentDefinition(2, new IntegerArgument()); //end
	//3. initial
	//4. function
	setArgumentDefinition(5, new SymbolicArgument()); //x var
	setArgumentDefinition(6, new SymbolicArgument()); //y var
	setArgumentDefinition(7, new VectorArgument());
	setArgumentDefinition(8, new SymbolicArgument()); //i var
	setDefaultValue(8, "\"\"");
	setArgumentDefinition(9, new SymbolicArgument()); //v var
	setDefaultValue(9, "\"\"");
}
bool CustomSumFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	int start = vargs[0].number().intValue();
	if(start < 1) start = 1;
	int end = vargs[1].number().intValue();
	int n = vargs[6].components();
	if(start > n) start = n;
	if(end < 1 || end > n) end = n;
	else if(end < start) end = start;	
	
	mstruct = vargs[2];
	MathStructure mprocess;

	for(unsigned int index = start - 1; index < (unsigned int) end; index++) {	
		mprocess = vargs[3];
		mprocess.replace(vargs[4], vargs[6][index]);
		mprocess.replace(vargs[5], mstruct);
		if(!vargs[7].isEmptySymbol()) {
			mprocess.replace(vargs[7], (int) index);
		}
		if(!vargs[8].isEmptySymbol()) {
			mprocess.replace(vargs[8], vargs[6]);
		}
		mstruct = mprocess;
	}
	return true;

}

FunctionFunction::FunctionFunction() : Function("function", 2) {
	setArgumentDefinition(1, new TextArgument());
	setArgumentDefinition(2, new VectorArgument());
}
bool FunctionFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	UserFunction f("", "Generated Function", vargs[0].symbol());
	MathStructure args = vargs[1];
	mstruct = f.Function::calculate(args, eo);	
	return true;
}
IFFunction::IFFunction() : Function("if", 3) {
	NON_COMPLEX_NUMBER_ARGUMENT(1)
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());
}
bool IFFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int result = vargs[0].number().getBoolean();
	if(result) {			
		mstruct = CALCULATOR->parse(vargs[1].symbol());
	} else if(result == 0) {			
		mstruct = CALCULATOR->parse(vargs[2].symbol());		
	} else {
		return false;
	}	
	return true;
}
LoadFunction::LoadFunction() : Function("load", 1, 3) {
	setArgumentDefinition(1, new FileArgument());
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(2, "1");
	setArgumentDefinition(3, new TextArgument());
	setDefaultValue(3, ",");	
}
bool LoadFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	string delim = vargs[2].symbol();
	if(delim == "tab") {
		delim = "\t";
	}
	if(!CALCULATOR->importCSV(mstruct, vargs[0].symbol().c_str(), vargs[1].number().intValue(), delim)) {
		CALCULATOR->error(true, "Failed to load %s.", vargs[0].symbol().c_str(), NULL);
		return false;
	}
	return true;
}
TitleFunction::TitleFunction() : Function("title", 1) {
	setArgumentDefinition(1, new ExpressionItemArgument());
}
bool TitleFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	ExpressionItem *item = CALCULATOR->getExpressionItem(vargs[0].symbol());
	if(!item) {
		CALCULATOR->error(true, _("Object %s does not exist."), vargs[0].symbol().c_str(), NULL);
		return false;
	} else {
		mstruct = item->title();
	}
	return true;
}
SaveFunction::SaveFunction() : Function("save", 2, 4) {
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());	
	setArgumentDefinition(4, new TextArgument());		
	setDefaultValue(3, "\"Temporary\"");
	setDefaultValue(4, "\"\"");	
}
bool SaveFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	CALCULATOR->addVariable(new KnownVariable(vargs[2].symbol(), vargs[1].symbol(), vargs[0], vargs[3].symbol()));
	return true;
}

DeriveFunction::DeriveFunction() : Function("diff", 1, 3) {
	setArgumentDefinition(2, new SymbolicArgument());
	setDefaultValue(2, "x");
	setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(3, "1");		
}
bool DeriveFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int i = vargs[2].number().intValue();
	mstruct = vargs[0];
	bool b = false;
	while(i) {
		if(!b && !mstruct.differentiate(vargs[1])) {
			return false;
		}
		b = true;
		i--;
	}
	return true;
}
