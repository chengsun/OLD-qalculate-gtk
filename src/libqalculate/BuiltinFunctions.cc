/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "support.h"

#include "BuiltinFunctions.h"
#include "util.h"
#include "MathStructure.h"
#include "Number.h"
#include "Calculator.h"
#include "Variable.h"

#include <sstream>

#define FR_FUNCTION(FUNC)	Number nr(vargs[0].number()); if(!nr.FUNC() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) || (!eo.allow_complex && nr.isComplex() && !vargs[0].number().isComplex()) || (!eo.allow_infinite && nr.isInfinite() && !vargs[0].number().isInfinite())) {return 0;} else {mstruct = nr; return 1;}
#define FR_FUNCTION_2(FUNC)	Number nr(vargs[0].number()); if(!nr.FUNC(vargs[1].number()) || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) || (!eo.allow_complex && nr.isComplex() && !vargs[0].number().isComplex() && !vargs[1].number().isComplex()) || (!eo.allow_infinite && nr.isInfinite() && !vargs[0].number().isInfinite() && !vargs[1].number().isInfinite())) {return 0;} else {mstruct = nr; return 1;}

#define NON_COMPLEX_NUMBER_ARGUMENT(i)				NumberArgument *arg_non_complex##i = new NumberArgument(); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);
#define NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(i)			NumberArgument *arg_non_complex##i = new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);
#define NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(i)		NumberArgument *arg_non_complex##i = new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, true, false); arg_non_complex##i->setComplexAllowed(false); setArgumentDefinition(i, arg_non_complex##i);


VectorFunction::VectorFunction() : MathFunction("vector", -1) {
}
int VectorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs;
	mstruct.setType(STRUCT_VECTOR);
	return 1;
}
MatrixFunction::MatrixFunction() : MathFunction("matrix", 3) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(3, new VectorArgument());
}
int MatrixFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
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
	return 1 ;
}
RankFunction::RankFunction() : MathFunction("rank", 1, 2) {
	setArgumentDefinition(1, new VectorArgument(""));
	setArgumentDefinition(2, new BooleanArgument(""));
	setDefaultValue(2, "1");
}
int RankFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	return mstruct.rankVector(vargs[1].number().getBoolean());
}
SortFunction::SortFunction() : MathFunction("sort", 1, 2) {
	setArgumentDefinition(1, new VectorArgument(""));
	setArgumentDefinition(2, new BooleanArgument(""));
	setDefaultValue(2, "1");
}
int SortFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	return mstruct.sortVector(vargs[1].number().getBoolean());
}
MergeVectorsFunction::MergeVectorsFunction() : MathFunction("mergevectors", 1, -1) {
	setArgumentDefinition(1, new VectorArgument(""));
	setArgumentDefinition(2, new VectorArgument(""));
}
int MergeVectorsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
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
	return 1 ;
}
MatrixToVectorFunction::MatrixToVectorFunction() : MathFunction("matrix2vector", 1) {
	setArgumentDefinition(1, new MatrixArgument());
}
int MatrixToVectorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[0].matrixToVector(mstruct);
	return 1 ;
}
RowFunction::RowFunction() : MathFunction("row", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new MatrixArgument());	
}
int RowFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int row = vargs[0].number().intValue();
	if(row > (int) vargs[1].rows()) {
		CALCULATOR->error(true, _("Row %s does not exist in matrix."), vargs[0].print().c_str(), NULL);
		return 0;
	}
	vargs[1].rowToVector(row, mstruct);
	return 1 ;
}
ColumnFunction::ColumnFunction() : MathFunction("column", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new MatrixArgument());	
}
int ColumnFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int col = vargs[0].number().intValue();
	if(col > (int) vargs[1].columns()) {
		CALCULATOR->error(true, _("Column %s does not exist in matrix."), vargs[0].print().c_str(), NULL);
		return 0;
	}
	vargs[1].columnToVector(col, mstruct);
	return 1 ;
}
RowsFunction::RowsFunction() : MathFunction("rows", 1) {
	setArgumentDefinition(1, new MatrixArgument(""));
}
int RowsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].rows();
	return 1 ;
}
ColumnsFunction::ColumnsFunction() : MathFunction("columns", 1) {
	setArgumentDefinition(1, new MatrixArgument(""));
}
int ColumnsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].columns();
	return 1 ;
}
ElementsFunction::ElementsFunction() : MathFunction("elements", 1) {
	setArgumentDefinition(1, new MatrixArgument(""));
}
int ElementsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) (vargs[0].rows() * vargs[0].columns());
	return 1 ;
}
ElementFunction::ElementFunction() : MathFunction("element", 3) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(3, new MatrixArgument(""));
}
int ElementFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
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
ComponentsFunction::ComponentsFunction() : MathFunction("dimension", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
int ComponentsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].components();
	return 1 ;
}
ComponentFunction::ComponentFunction() : MathFunction("component", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new VectorArgument(""));
}
int ComponentFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int i = vargs[0].number().intValue();
	if(i > (int) vargs[1].components()) {
		CALCULATOR->error(true, _("Component %s does not exist in vector."), vargs[0].print().c_str(), NULL);
		return 0;
	}
	mstruct = *vargs[1].getComponent(i);
	return 1 ;
}
LimitsFunction::LimitsFunction() : MathFunction("limits", 3) {
	setArgumentDefinition(1, new IntegerArgument(""));
	setArgumentDefinition(2, new IntegerArgument(""));	
	setArgumentDefinition(3, new VectorArgument(""));	
}
int LimitsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[2].getRange(vargs[0].number().intValue(), vargs[1].number().intValue(), mstruct);
	return 1 ;
}
AreaFunction::AreaFunction() : MathFunction("area", 5) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(4, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(5, new MatrixArgument(""));	
}
int AreaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[4].getArea(vargs[0].number().intValue(), vargs[1].number().intValue(), vargs[2].number().intValue(), vargs[3].number().intValue(), mstruct);
	return 1 ;
}
TransposeFunction::TransposeFunction() : MathFunction("transpose", 1) {
	setArgumentDefinition(1, new MatrixArgument());
}
int TransposeFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	return mstruct.transposeMatrix();
}
IdentityFunction::IdentityFunction() : MathFunction("identity", 1) {
	ArgumentSet *arg = new ArgumentSet();
	arg->addArgument(new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	arg->addArgument(marg);
	setArgumentDefinition(1, arg);
}
int IdentityFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].isMatrix()) {
		if(vargs[0].rows() != vargs[0].columns()) {
			return 0;
		}
		mstruct.setToIdentityMatrix(vargs[0].size());
	} else {
		mstruct.setToIdentityMatrix((unsigned int) vargs[0].number().intValue());
	}
	return 1 ;
}
DeterminantFunction::DeterminantFunction() : MathFunction("det", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
int DeterminantFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[0].determinant(mstruct, eo);
	return !mstruct.isUndefined();
}
PermanentFunction::PermanentFunction() : MathFunction("permanent", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
int PermanentFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[0].permanent(mstruct, eo);
	return !mstruct.isUndefined();
}
CofactorFunction::CofactorFunction() : MathFunction("cofactor", 3) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));	
	setArgumentDefinition(3, new MatrixArgument());
}
int CofactorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	vargs[2].cofactor(vargs[0].number().intValue(), vargs[1].number().intValue(), mstruct, eo);
	return !mstruct.isUndefined();
}
AdjointFunction::AdjointFunction() : MathFunction("adj", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
int AdjointFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct.adjointMatrix(eo);
	return !mstruct.isUndefined();
}
InverseFunction::InverseFunction() : MathFunction("inverse", 1) {
	MatrixArgument *marg = new MatrixArgument();
	marg->setSymmetricDemanded(true);
	setArgumentDefinition(1, marg);
}
int InverseFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct.invertMatrix(eo);
	return !mstruct.isUndefined();;
}

ZetaFunction::ZetaFunction() : MathFunction("zeta", 1, 1, SIGN_ZETA) {
	setArgumentDefinition(1, new IntegerArgument());
}
int ZetaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(zeta)
}
GammaFunction::GammaFunction() : MathFunction("gamma", 1, 1, SIGN_CAPITAL_GAMMA) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int GammaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].number().isRational()) {
		if(vargs[0].number().isInteger()) {
			mstruct.set(CALCULATOR->f_factorial, &vargs[0], NULL);
			mstruct[0] -= 1;
			return 1;
		} else if(vargs[0].number().denominatorIsTwo()) {
			Number nr(vargs[0].number());
			nr.floor();
			if(nr.isZero()) {
				MathStructure mtmp(CALCULATOR->v_pi);
				mstruct.set(CALCULATOR->f_sqrt, &mtmp, NULL);
				return 1;
			} else if(nr.isPositive()) {
				Number nr2(nr);
				nr2 *= 2;
				nr2 -= 1;
				nr2.doubleFactorial();
				Number nr3(2, 1);
				nr3 ^= nr;
				nr2 /= nr3;
				mstruct = nr2;
				MathStructure mtmp1(CALCULATOR->v_pi);
				MathStructure mtmp2(CALCULATOR->f_sqrt, &mtmp1, NULL);
				mstruct *= mtmp2;
				return 1;
			} else {
				nr.negate();
				Number nr2(nr);
				nr2 *= 2;
				nr2 -= 1;
				nr2.doubleFactorial();
				Number nr3(2, 1);
				nr3 ^= nr;
				if(nr.isOdd()) nr3.negate();
				nr3 /= nr2;
				mstruct = nr3;
				MathStructure mtmp1(CALCULATOR->v_pi);
				MathStructure mtmp2(CALCULATOR->f_sqrt, &mtmp1, NULL);
				mstruct *= mtmp2;
				return 1;
			}
		}
	}
	CALCULATOR->error(false, _("%s() does at the moment only support integers and fractions of two."), preferredDisplayName().name.c_str(), NULL); 
	return 0;
}
BetaFunction::BetaFunction() : MathFunction("beta", 2, 2, SIGN_CAPITAL_BETA) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, false));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, false));
}
int BetaFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0]; 
	mstruct.set(CALCULATOR->f_gamma, &vargs[0], NULL);
	MathStructure mstruct2(CALCULATOR->f_gamma, &vargs[1], NULL);
	mstruct *= mstruct2;
	mstruct2[0] += vargs[0];
	mstruct /= mstruct2;
	return 1 ;
}

FactorialFunction::FactorialFunction() : MathFunction("factorial", 1) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int FactorialFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(factorial)
}
DoubleFactorialFunction::DoubleFactorialFunction() : MathFunction("factorial2", 1) {
	IntegerArgument *arg = new IntegerArgument("", ARGUMENT_MIN_MAX_NONE, true, true);
	Number nr(-1, 1);
	arg->setMin(&nr);
	setArgumentDefinition(1, arg);
}
int DoubleFactorialFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(doubleFactorial)
}
MultiFactorialFunction::MultiFactorialFunction() : MathFunction("multifactorial", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE, true, true));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE, true, true));
}
int MultiFactorialFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION_2(multiFactorial)
}
BinomialFunction::BinomialFunction() : MathFunction("binomial", 2) {
	setArgumentDefinition(1, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE, true, true));
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE, true, true));
	setCondition("\\x>=\\y");
}
int BinomialFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	Number nr;
	if(!nr.binomial(vargs[0].number(), vargs[1].number())) return 0;
	mstruct = nr;
	return 1 ;
}

AbsFunction::AbsFunction() : MathFunction("abs", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
int AbsFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0]; 
	mstruct.eval(eo);
	if(mstruct.isNumber()) {
		Number nr = mstruct.number(); 
		if(!nr.abs() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate())) {
			return 0;
		} else {
			mstruct = nr; 
			return 1 ;
		}
	} else if(mstruct.representsNegative()) {
		mstruct.negate();
		return 1 ;
	} else if(mstruct.representsNonNegative()) {
		return 1 ;
	}
	return -1;
}
GcdFunction::GcdFunction() : MathFunction("gcd", 2) {
	setArgumentDefinition(1, new IntegerArgument());
	setArgumentDefinition(2, new IntegerArgument());
}
int GcdFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION_2(gcd)
}
SignumFunction::SignumFunction() : MathFunction("sgn", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int SignumFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(signum)
}
CeilFunction::CeilFunction() : MathFunction("ceil", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
int CeilFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(ceil)
}
FloorFunction::FloorFunction() : MathFunction("floor", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
int FloorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(floor)
}
TruncFunction::TruncFunction() : MathFunction("trunc", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
int TruncFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(trunc)
}
RoundFunction::RoundFunction() : MathFunction("round", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
int RoundFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(round)
}
FracFunction::FracFunction() : MathFunction("frac", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
int FracFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(frac)
}
IntFunction::IntFunction() : MathFunction("int", 1) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
}
int IntFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(trunc)
}
RemFunction::RemFunction() : MathFunction("rem", 2) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(2)
}
int RemFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION_2(rem)
}
ModFunction::ModFunction() : MathFunction("mod", 2) {
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR(1)
	NON_COMPLEX_NUMBER_ARGUMENT_NO_ERROR_NONZERO(2)
}
int ModFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION_2(mod)
}

ImFunction::ImFunction() : MathFunction("im", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int ImFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0].number().imaginaryPart();
	return 1 ;
}
ReFunction::ReFunction() : MathFunction("re", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int ReFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0].number().realPart();
	return 1 ;
}
ArgFunction::ArgFunction() : MathFunction("arg", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
int ArgFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].isNumber()) {
		if(vargs[0].number().isOne()) {
			mstruct.clear();
			return 1;
		} else if(vargs[0].number().isMinusOne()) {
			mstruct = CALCULATOR->v_pi;
			return 1;
		} else {
			Number nr(vargs[0].number().imaginaryPart());
			if(nr.isOne()) {
				nr = vargs[0].number().realPart();
				if(nr.isOne()) {
					mstruct = CALCULATOR->v_pi;
					mstruct /= 4;
					return 1;
				} else if(nr.isZero()) {
					mstruct = CALCULATOR->v_pi;
					mstruct /= 2;
					return 1;
				}
			} else if(nr.isMinusOne()) {
				mstruct = CALCULATOR->v_pi;
				mstruct /= -2;
				return 1;
			}
		}
	}
	MathStructure m_re(CALCULATOR->f_re, &vargs[0], NULL);
	MathStructure m_im(CALCULATOR->f_im, &vargs[0], NULL);
	m_im /= m_re;
	mstruct.set(CALCULATOR->f_atan, &m_im, NULL);
	return 1 ;
}

SqrtFunction::SqrtFunction() : MathFunction("sqrt", 1) {
}
int SqrtFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct ^= MathStructure(1, 2);
	return 1 ;
}
SquareFunction::SquareFunction() : MathFunction("sq", 1) {
}
int SquareFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	mstruct ^= 2;
	return 1 ;
}

ExpFunction::ExpFunction() : MathFunction("exp", 1) {
}
int ExpFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = CALCULATOR->v_e;
	mstruct ^= vargs[0];
	return 1 ;
}

LogFunction::LogFunction() : MathFunction("ln", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false));
}
int LogFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

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
		return 1 ;
	}
	if(eo.approximation == APPROXIMATION_TRY_EXACT && !mstruct.isNumber()) {
		EvaluationOptions eo2 = eo;
		eo2.approximation = APPROXIMATION_APPROXIMATE;
		mstruct = vargs[0];
		mstruct.eval(eo2);
	}
	if(mstruct.isNumber()) {
		if(eo.allow_complex && mstruct.number().isMinusOne()) {
			mstruct = CALCULATOR->v_i->get();
			mstruct *= CALCULATOR->v_pi;
			return 1 ;
		} else if(mstruct.number().isI() || mstruct.number().isMinusI()) {
			mstruct = Number(1, 2);
			mstruct *= CALCULATOR->v_pi;
			mstruct *= CALCULATOR->v_i->get();
			return 1 ;
		} else if(eo.allow_complex && eo.allow_infinite && mstruct.number().isMinusInfinity()) {
			mstruct = CALCULATOR->v_pi;
			mstruct *= CALCULATOR->v_i->get();
			Number nr; nr.setPlusInfinity();
			mstruct += nr;
			return 1 ;
		}
		Number nr(mstruct.number());
		if(nr.ln() && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) && !(!eo.allow_complex && nr.isComplex() && !mstruct.number().isComplex()) && !(!eo.allow_infinite && nr.isInfinite() && !mstruct.number().isInfinite())) {
			mstruct = nr;
			return 1 ;
		}
	}
	return -1;
	
}
LognFunction::LognFunction() : MathFunction("log", 1, 2) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false));
	setArgumentDefinition(2, new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO, false));
	setDefaultValue(2, "e");
}
int LognFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	if(vargs[1].isVariable() && vargs[1].variable() == CALCULATOR->v_e) {
		mstruct.set(CALCULATOR->f_ln, &vargs[0], NULL);
		return 1 ;
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
			return 1 ;
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
			return 1 ;
		}
	} else if(vargs[0].isNumber() && vargs[1].isNumber()) {
		Number nr(mstruct.number());
		if(nr.log(vargs[1].number()) && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) && !(!eo.allow_complex && nr.isComplex() && !mstruct.number().isComplex()) && !(!eo.allow_infinite && nr.isInfinite() && !mstruct.number().isInfinite())) {
			mstruct = nr;
			return 1 ;
		}
	} 
	mstruct.set(CALCULATOR->f_ln, &vargs[0], NULL);
	mstruct /= MathStructure(CALCULATOR->f_ln, &vargs[1], NULL);
	return 1 ;
}

SinFunction::SinFunction() : MathFunction("sin", 1) {
	setArgumentDefinition(1, new AngleArgument());
}
int SinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

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
		return 1 ;
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
		if(nr.sin() && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) && !(!eo.allow_complex && nr.isComplex() && !mstruct.number().isComplex()) && !(!eo.allow_infinite && nr.isInfinite() && !mstruct.number().isInfinite())) {
			mstruct = nr;
			return 1 ;
		}
	}
	if(mstruct.isNegate()) {
		MathStructure mstruct2(CALCULATOR->f_sin, &mstruct[0], NULL);
		mstruct = mstruct2;
		mstruct.negate();
		if(CALCULATOR->u_rad) mstruct[0] *= CALCULATOR->u_rad;
		return 1 ;
	}
	if(CALCULATOR->u_rad) mstruct *= CALCULATOR->u_rad;
	return -1;
}
CosFunction::CosFunction() : MathFunction("cos", 1) {
	setArgumentDefinition(1, new AngleArgument());
}
int CosFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

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
			if(mstruct[0].number().isEven()) {
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
		return 1 ;
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
		if(nr.cos() && !(eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) && !(!eo.allow_complex && nr.isComplex() && !mstruct.number().isComplex()) && !(!eo.allow_infinite && nr.isInfinite() && !mstruct.number().isInfinite())) {
			mstruct = nr;
			return 1 ;
		}
	}
	if(CALCULATOR->u_rad) mstruct *= CALCULATOR->u_rad;
	return -1;
}
TanFunction::TanFunction() : MathFunction("tan", 1) {
	setArgumentDefinition(1, new AngleArgument());
}
int TanFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct.set(CALCULATOR->f_sin, &vargs[0], NULL);
	mstruct /= MathStructure(CALCULATOR->f_cos, &vargs[0], NULL);
	return 1 ;
}
AsinFunction::AsinFunction() : MathFunction("asin", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int AsinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	if(vargs[0].number().isZero()) {
		mstruct.clear();
	} else if(vargs[0].number().isOne()) {
		switch(eo.angle_unit) {
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
		switch(eo.angle_unit) {
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
		switch(eo.angle_unit) {
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
		if(!nr.asin() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) || (!eo.allow_complex && nr.isComplex() && !mstruct.number().isComplex()) || (!eo.allow_infinite && nr.isInfinite() && !mstruct.number().isInfinite())) return 0;
		mstruct = nr;
		switch(eo.angle_unit) {
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
			default: {}
		}
	}
	return 1 ;
	
}
AcosFunction::AcosFunction() : MathFunction("acos", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int AcosFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	if(vargs[0].number().isZero()) {
		switch(eo.angle_unit) {
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
		switch(eo.angle_unit) {
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
		switch(eo.angle_unit) {
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
		if(!nr.acos() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) || (!eo.allow_complex && nr.isComplex() && !vargs[0].number().isComplex()) || (!eo.allow_infinite && nr.isInfinite() && !vargs[0].number().isInfinite())) return 0;
		mstruct = nr;
		switch(eo.angle_unit) {
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
			default: {}
		}
	}
	return 1 ;
	
}
AtanFunction::AtanFunction() : MathFunction("atan", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int AtanFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	if(vargs[0].number().isZero()) {
		mstruct.clear();
	} else if(eo.allow_infinite && vargs[0].number().isI()) {
		mstruct = vargs[0];
		Number nr; nr.setInfinity();
		mstruct *= nr;
	} else if(eo.allow_infinite && vargs[0].number().isMinusI()) {
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
		if(!nr.atan() || (eo.approximation == APPROXIMATION_EXACT && nr.isApproximate()) || (!eo.allow_complex && nr.isComplex() && !vargs[0].number().isComplex()) || (!eo.allow_infinite && nr.isInfinite() && !vargs[0].number().isInfinite())) return 0;
		mstruct = nr;
	}
	return 1 ;
	
}
SinhFunction::SinhFunction() : MathFunction("sinh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));
}
int SinhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(sinh)
}
CoshFunction::CoshFunction() : MathFunction("cosh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, true, false));	
}
int CoshFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	FR_FUNCTION(cosh)
}
TanhFunction::TanhFunction() : MathFunction("tanh", 1) {}
int TanhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct.set(CALCULATOR->f_sinh, &vargs[0], NULL);
	mstruct /= MathStructure(CALCULATOR->f_cosh, &vargs[0], NULL);
	return 1 ;
}
AsinhFunction::AsinhFunction() : MathFunction("asinh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
int AsinhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure m_arg(vargs[0]);
	m_arg ^= 2;
	m_arg += 1;
	m_arg ^= Number(1, 2);
	m_arg += vargs[0];
	mstruct.set(CALCULATOR->f_ln, &m_arg, NULL);
	return 1 ;
}
AcoshFunction::AcoshFunction() : MathFunction("acosh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
int AcoshFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure m_arg(vargs[0]);
	m_arg ^= 2;
	m_arg -= 1;
	m_arg ^= Number(1, 2);
	m_arg += vargs[0];
	mstruct.set(CALCULATOR->f_ln, &m_arg, NULL);
	return 1 ;
}
AtanhFunction::AtanhFunction() : MathFunction("atanh", 1) {
	setArgumentDefinition(1, new NumberArgument("", ARGUMENT_MIN_MAX_NONE, false, false));
}
int AtanhFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	
	MathStructure m_arg = 1;
	m_arg += vargs[0];
	MathStructure m_den = 1;
	m_den -= vargs[0];
	m_arg /= m_den;
	mstruct.set(CALCULATOR->f_ln, &m_arg, NULL);
	mstruct *= Number(1, 2);
	return 1 ;
	
}

RadiansToDefaultAngleUnitFunction::RadiansToDefaultAngleUnitFunction() : MathFunction("radtodef", 1) {
}
int RadiansToDefaultAngleUnitFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	switch(eo.angle_unit) {
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
		default: {}
	}
	return 1 ;
}

TotalFunction::TotalFunction() : MathFunction("total", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
int TotalFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct.clear();
	for(unsigned int index = 0; index < vargs[0].size(); index++) {
		mstruct.add(vargs[0][index], true);
	}
	return 1 ;
}
PercentileFunction::PercentileFunction() : MathFunction("percentile", 2) {
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
int PercentileFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure v(vargs[1]);
	MathStructure *mp;
	Number fr100(100);
	if(!v.sortVector()) {
		return 0;
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
			if(!mp) return 0;
			mstruct = *mp;
		} else {
			Number ufr(pfr);
			ufr.ceil();
			Number lfr(pfr);
			lfr.floor();
			pfr -= lfr;
			mp = v.getComponent(ufr.intValue());
			if(!mp) return 0;
			MathStructure gap(*mp);
			mp = v.getComponent(lfr.intValue());
			if(!mp) return 0;
			gap -= *mp;
			gap *= pfr;
			mp = v.getComponent(lfr.intValue());
			if(!mp) return 0;
			mstruct = *mp;
			mstruct += gap;
		}
	}
	return 1 ;
}
MinFunction::MinFunction() : MathFunction("min", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
int MinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
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
			if(!b) return 0;
			MathStructure margs; margs.clearVector();
			margs.addItem(*min);
			for(unsigned int i = 0; i < unsolveds.size(); i++) {
				margs.addItem(*unsolveds[i]);
			}
			mstruct.set(this, &margs, NULL);
			return 1 ;
		} else {
			mstruct = *min;
			return 1 ;
		}
	}
	return 0;
}
MaxFunction::MaxFunction() : MathFunction("max", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
int MaxFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
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
			if(!b) return 0;
			MathStructure margs; margs.clearVector();
			margs.addItem(*max);
			for(unsigned int i = 0; i < unsolveds.size(); i++) {
				margs.addItem(*unsolveds[i]);
			}
			mstruct.set(this, &margs, NULL);
			return 1 ;
		} else {
			mstruct = *max;
			return 1 ;
		}
	}
	return 0;
}
ModeFunction::ModeFunction() : MathFunction("mode", 1) {
	setArgumentDefinition(1, new VectorArgument(""));
}
int ModeFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].size() <= 0) {
		return 0;
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
		return 1 ;
	}
	return 0;
}
RandFunction::RandFunction() : MathFunction("rand", 0, 1) {
	setArgumentDefinition(1, new IntegerArgument());
	setDefaultValue(1, "-1"); 
}
int RandFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	if(vargs[0].number().isZero() || vargs[0].number().isNegative()) {
		Number nr;
		nr.setInternal(cln::random_F(cln::cl_float(1)));
		mstruct = nr;
	} else {
		Number nr;
		nr.setInternal(cln::random_I(cln::numerator(cln::rational(cln::realpart(vargs[0].number().internalNumber())))) + 1);
		mstruct = nr;
	}
	return 1 ;
}

DaysFunction::DaysFunction() : MathFunction("days", 2, 4) {
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
int DaysFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int days = daysBetweenDates(vargs[0].symbol(), vargs[1].symbol(), vargs[2].number().intValue(), vargs[3].number().isZero());
	if(days < 0) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		return 0;
	}
	mstruct.set(days, 1, 0);
	return 1 ;			
}
YearFracFunction::YearFracFunction() : MathFunction("yearfrac", 2, 4) {
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
int YearFracFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	Number yfr = yearsBetweenDates(vargs[0].symbol(), vargs[1].symbol(), vargs[2].number().intValue(), vargs[3].number().isZero());
	if(yfr.isMinusOne()) {
		CALCULATOR->error(true, _("Error in date format for function %s()."), name().c_str(), NULL);
		return 0;
	}
	mstruct.set(yfr);
	return 1 ;
}
WeekFunction::WeekFunction() : MathFunction("week", 0, 2) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new BooleanArgument());	
	setDefaultValue(1, "today");
}
int WeekFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int w = week(vargs[0].symbol(), vargs[1].number().getBoolean());
	if(w < 0) {
		return 0;
	}
	mstruct.set(w, 1);
	return 1 ;
}
WeekdayFunction::WeekdayFunction() : MathFunction("weekday", 0, 2) {
	setArgumentDefinition(1, new DateArgument());
	setArgumentDefinition(2, new BooleanArgument());
	setDefaultValue(1, "today");
}
int WeekdayFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int w = weekday(vargs[0].symbol());
	if(w < 0) {
		return 0;
	}
	if(vargs[1].number().getBoolean()) {
		if(w == 7) w = 1;
		else w++;
	}
	mstruct.set(w, 1);
	return 1 ;
}
YeardayFunction::YeardayFunction() : MathFunction("yearday", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "today");
}
int YeardayFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int d = yearday(vargs[0].symbol());
	if(d < 0) {
		return 0;
	}
	mstruct.set(d, 1);
	return 1 ;
}
MonthFunction::MonthFunction() : MathFunction("month", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "today");
}
int MonthFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int year, month, day;
	if(!s2date(vargs[0].symbol(), year, month, day)) {
		return 0;
	}
	mstruct.set(month, 1);
	return 1 ;
}
DayFunction::DayFunction() : MathFunction("day", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "today");
}
int DayFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int year, month, day;
	if(!s2date(vargs[0].symbol(), year, month, day)) {
		return 0;
	}
	mstruct.set(day, 1);
	return 1 ;
}
YearFunction::YearFunction() : MathFunction("year", 0, 1) {
	setArgumentDefinition(1, new DateArgument());
	setDefaultValue(1, "today");
}
int YearFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int year, month, day;
	if(!s2date(vargs[0].symbol(), year, month, day)) {
		return 0;
	}
	mstruct.set(year, 1);
	return 1 ;
}
TimeFunction::TimeFunction() : MathFunction("time", 0) {
}
int TimeFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int hour, min, sec;
	now(hour, min, sec);
	Number tnr(sec, 1);
	tnr /= 60;
	tnr += min;
	tnr /= 60;
	tnr += hour;
	mstruct = tnr;
	return 1 ;
}

BinFunction::BinFunction() : MathFunction("bin", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int BinFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	//mstruct = Number(vargs[0].symbol(), 2);
	ParseOptions po = eo.parse_options;
	po.base = BASE_BINARY;
	mstruct = CALCULATOR->parse(vargs[0].symbol(), po);
	return 1 ;
}
OctFunction::OctFunction() : MathFunction("oct", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int OctFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	//mstruct = Number(vargs[0].symbol(), 8);
	ParseOptions po = eo.parse_options;
	po.base = BASE_OCTAL;
	mstruct = CALCULATOR->parse(vargs[0].symbol(), po);
	return 1 ;
}
HexFunction::HexFunction() : MathFunction("hex", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int HexFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	//mstruct = Number(vargs[0].symbol(), 16);
	ParseOptions po = eo.parse_options;
	po.base = BASE_HEXADECIMAL;
	mstruct = CALCULATOR->parse(vargs[0].symbol(), po);
	return 1;
}
BaseFunction::BaseFunction() : MathFunction("base", 2) {
	setArgumentDefinition(1, new TextArgument());
	IntegerArgument *arg = new IntegerArgument();
	Number integ(2, 1);
	arg->setMin(&integ);
	integ.set(36, 1);
	arg->setMax(&integ);
	setArgumentDefinition(2, arg);
}
int BaseFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	//mstruct = Number(vargs[0].symbol(), vargs[1].number().intValue());
	ParseOptions po = eo.parse_options;
	po.base = vargs[1].number().intValue();
	mstruct = CALCULATOR->parse(vargs[0].symbol(), po);
	return 1 ;
}
RomanFunction::RomanFunction() : MathFunction("roman", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int RomanFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	//mstruct = Number(vargs[0].symbol(), BASE_ROMAN_NUMERALS);
	ParseOptions po = eo.parse_options;
	po.base = BASE_ROMAN_NUMERALS;
	mstruct = CALCULATOR->parse(vargs[0].symbol(), po);
	return 1 ;
}

AsciiFunction::AsciiFunction() : MathFunction("code", 1) {
	TextArgument *arg = new TextArgument();
	arg->setCustomCondition("len(\\x) = 1");
	setArgumentDefinition(1, arg);
}
int AsciiFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	unsigned char c = (unsigned char) vargs[0].symbol()[0];
	mstruct.set(c, 1);
	return 1 ;
}
CharFunction::CharFunction() : MathFunction("char", 1) {
	IntegerArgument *arg = new IntegerArgument();
	Number fr(32, 0);
	arg->setMin(&fr);
	fr.set(0x7f, 1);
	arg->setMax(&fr);
	setArgumentDefinition(1, arg);
}
int CharFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	string str;
	str += vargs[0].number().intValue();
	mstruct = str;
	return 1 ;
}

ConcatenateFunction::ConcatenateFunction() : MathFunction("concatenate", 1, -1) {
	setArgumentDefinition(1, new TextArgument());
	setArgumentDefinition(2, new TextArgument());
}
int ConcatenateFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	string str;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		str += vargs[i].symbol();
	}
	mstruct = str;
	return 1 ;
}
LengthFunction::LengthFunction() : MathFunction("len", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int LengthFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = (int) vargs[0].symbol().length();
	return 1 ;
}

ReplaceFunction::ReplaceFunction() : MathFunction("replace", 3, 4) {
	setArgumentDefinition(4, new BooleanArgument());
	setDefaultValue(4, "0");
}
int ReplaceFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	if(vargs[3].number().getBoolean()) mstruct.eval(eo);
	mstruct.replace(vargs[1], vargs[2]);
	return 1 ;
}

ErrorFunction::ErrorFunction() : MathFunction("error", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int ErrorFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	CALCULATOR->error(true, vargs[0].symbol().c_str(), NULL);
	return 1 ;
}
WarningFunction::WarningFunction() : MathFunction("warning", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int WarningFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	CALCULATOR->error(false, vargs[0].symbol().c_str(), NULL);
	return 1 ;
}
MessageFunction::MessageFunction() : MathFunction("message", 1) {
	setArgumentDefinition(1, new TextArgument());
}
int MessageFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	CALCULATOR->message(MESSAGE_INFORMATION, vargs[0].symbol().c_str(), NULL);
	return 1 ;
}

ForFunction::ForFunction() : MathFunction("for", 7) {
	setArgumentDefinition(2, new SymbolicArgument());	
	setArgumentDefinition(7, new SymbolicArgument());
}
int ForFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[4];
	MathStructure mcounter = vargs[0];
	MathStructure mtest;
	MathStructure mcount;
	MathStructure mupdate;
	while(true) {
		mtest = vargs[3];
		mtest.replace(vargs[1], mcounter);
		mtest.eval(eo);
		if(!mtest.isNumber()) return 0;
		if(!mtest.number().getBoolean()) {
			break;
		}
		
		mupdate = vargs[5];
		mupdate.replace(vargs[1], mcounter);
		mupdate.replace(vargs[6], mstruct);
		mstruct = mupdate;
		
		mcount = vargs[3];
		mcount.replace(vargs[1], mcounter);
		mcounter = mcount;
	}
	return 1 ;

}
SumFunction::SumFunction() : MathFunction("sum", 3, 4) {
	setArgumentDefinition(2, new IntegerArgument());
	setArgumentDefinition(3, new IntegerArgument());	
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "x");
	setCondition("\\z >= \\y");
}
int SumFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

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
	return 1 ;
	
}
ProductFunction::ProductFunction() : MathFunction("product", 3, 4) {
	setArgumentDefinition(2, new IntegerArgument());
	setArgumentDefinition(3, new IntegerArgument());	
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "x");
	setCondition("\\z >= \\y");
}
int ProductFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

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
	return 1 ;
	
}

ProcessFunction::ProcessFunction() : MathFunction("process", 3, 5) {
	setArgumentDefinition(2, new SymbolicArgument());
	setArgumentDefinition(3, new VectorArgument());
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "\"\"");
	setArgumentDefinition(5, new SymbolicArgument());
	setDefaultValue(5, "\"\"");
}
int ProcessFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

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
	return 1 ;
	
}
ProcessMatrixFunction::ProcessMatrixFunction() : MathFunction("processm", 3, 6) {
	setArgumentDefinition(2, new SymbolicArgument());
	setArgumentDefinition(3, new MatrixArgument());
	setArgumentDefinition(4, new SymbolicArgument());
	setDefaultValue(4, "\"\"");
	setArgumentDefinition(5, new SymbolicArgument());
	setDefaultValue(5, "\"\"");
	setArgumentDefinition(6, new SymbolicArgument());
	setDefaultValue(6, "\"\"");
}
int ProcessMatrixFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

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
	return 1 ;
	
}
CustomSumFunction::CustomSumFunction() : MathFunction("csum", 7, 9) {
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
int CustomSumFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	int start = vargs[0].number().intValue();
	if(start < 1) start = 1;
	int end = vargs[1].number().intValue();
	int n = vargs[6].components();
	if(start > n) start = n;
	if(end < 1 || end > n) end = n;
	else if(end < start) end = start;	
	
	mstruct = vargs[2];
	MathStructure mexpr(vargs[3]);
	MathStructure mprocess;
	EvaluationOptions eo2 = eo;
	eo2.calculate_functions = false;
	//mexpr.eval(eo2);
	for(unsigned int index = start - 1; index < (unsigned int) end; index++) {	
		mprocess = mexpr;
		mprocess.replace(vargs[4], vargs[6][index]);
		mprocess.replace(vargs[5], mstruct);
		if(!vargs[7].isEmptySymbol()) {
			mprocess.replace(vargs[7], (int) index);
		}
		if(!vargs[8].isEmptySymbol()) {
			mprocess.replace(vargs[8], vargs[6]);
		}
		mprocess.eval(eo2);
		mstruct = mprocess;
	}
	return 1 ;

}

FunctionFunction::FunctionFunction() : MathFunction("function", 2) {
	setArgumentDefinition(1, new TextArgument());
	setArgumentDefinition(2, new VectorArgument());
}
int FunctionFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	UserFunction f("", "Generated MathFunction", vargs[0].symbol());
	MathStructure args = vargs[1];
	mstruct = f.MathFunction::calculate(args, eo);	
	return 1 ;
}
SelectFunction::SelectFunction() : MathFunction("select", 2, 4) {
	setArgumentDefinition(3, new SymbolicArgument());
	setDefaultValue(3, "x");
	setArgumentDefinition(4, new BooleanArgument());
	setDefaultValue(4, "0");
}
int SelectFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	MathStructure mtest;
	mstruct = vargs[0];
	mstruct.eval(eo);
	if(!mstruct.isVector()) {
		mtest = vargs[1];
		mtest.replace(vargs[2], mstruct);
		mtest.eval(eo);
		if(!mtest.isNumber() || mtest.number().getBoolean() < 0) {
			CALCULATOR->error(true, _("Comparison failed."), NULL);
			return -1;
		}
		if(mtest.number().getBoolean() == 0) {
			if(vargs[3].number().getBoolean() > 0) {
				CALCULATOR->error(true, _("No matching item found."), NULL);
				return -1;
			}
			mstruct.clearVector();
		}
		return 1;
	}
	for(int i = 0; i < (int) mstruct.size(); i++) {
		mtest = vargs[1];
		mtest.replace(vargs[2], mstruct[i]);
		mtest.eval(eo);
		if(!mtest.isNumber() || mtest.number().getBoolean() < 0) {
			CALCULATOR->error(true, _("Comparison failed."), NULL);
			return -1;
		}
		if(mtest.number().getBoolean() == 0) {
			if(vargs[3].number().getBoolean() == 0) {
				mstruct.delChild(i + 1);
				i--;
			}
		} else if(vargs[3].number().getBoolean() > 0) {
			MathStructure msave(mstruct[i]);
			mstruct = msave;
			return 1;
		}
	}
	if(vargs[3].number().getBoolean() > 0) {
		CALCULATOR->error(true, _("No matching item found."), NULL);
		return -1;
	}
	return 1;
}
IFFunction::IFFunction() : MathFunction("if", 3) {
	NON_COMPLEX_NUMBER_ARGUMENT(1)
}
int IFFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int result = vargs[0].number().getBoolean();
	if(result) {			
		mstruct = vargs[1];
	} else if(result == 0) {			
		mstruct = vargs[2];
	} else {
		return 0;
	}	
	return 1;
}
LoadFunction::LoadFunction() : MathFunction("load", 1, 3) {
	setArgumentDefinition(1, new FileArgument());
	setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(2, "1");
	setArgumentDefinition(3, new TextArgument());
	setDefaultValue(3, ",");	
}
int LoadFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	string delim = vargs[2].symbol();
	if(delim == "tab") {
		delim = "\t";
	}
	if(!CALCULATOR->importCSV(mstruct, vargs[0].symbol().c_str(), vargs[1].number().intValue(), delim)) {
		CALCULATOR->error(true, "Failed to load %s.", vargs[0].symbol().c_str(), NULL);
		return 0;
	}
	return 1 ;
}
ExportFunction::ExportFunction() : MathFunction("export", 2, 3) {
	setArgumentDefinition(1, new VectorArgument());
	setArgumentDefinition(2, new FileArgument());
	setArgumentDefinition(3, new TextArgument());
	setDefaultValue(3, ",");	
}
int ExportFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	string delim = vargs[2].symbol();
	if(delim == "tab") {
		delim = "\t";
	}
	if(!CALCULATOR->exportCSV(vargs[0], vargs[1].symbol().c_str(), delim)) {
		CALCULATOR->error(true, "Failed to export to %s.", vargs[1].symbol().c_str(), NULL);
		return 0;
	}
	return 1 ;
}
TitleFunction::TitleFunction() : MathFunction("title", 1) {
	setArgumentDefinition(1, new ExpressionItemArgument());
}
int TitleFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	ExpressionItem *item = CALCULATOR->getExpressionItem(vargs[0].symbol());
	if(!item) {
		CALCULATOR->error(true, _("Object %s does not exist."), vargs[0].symbol().c_str(), NULL);
		return 0;
	} else {
		mstruct = item->title();
	}
	return 1 ;
}
SaveFunction::SaveFunction() : MathFunction("save", 2, 4) {
	setArgumentDefinition(2, new TextArgument());
	setArgumentDefinition(3, new TextArgument());	
	setArgumentDefinition(4, new TextArgument());		
	setDefaultValue(3, "Temporary");
	setDefaultValue(4, "");	
}
int SaveFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	CALCULATOR->addVariable(new KnownVariable(vargs[2].symbol(), vargs[1].symbol(), vargs[0], vargs[3].symbol()));
	CALCULATOR->saveFunctionCalled();
	return 1 ;
}

DeriveFunction::DeriveFunction() : MathFunction("diff", 1, 3) {
	setArgumentDefinition(2, new SymbolicArgument());
	setDefaultValue(2, "x");
	setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
	setDefaultValue(3, "1");		
}
int DeriveFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	int i = vargs[2].number().intValue();
	mstruct = vargs[0];
	bool b = false;
	while(i) {
		if(!mstruct.differentiate(vargs[1], eo) && !b) {
			return 0;
		}
		b = true;
		i--;
	}
	return 1 ;
}
IntegrateFunction::IntegrateFunction() : MathFunction("integrate", 1, 2) {
	setArgumentDefinition(2, new SymbolicArgument());
	setDefaultValue(2, "x");
}
int IntegrateFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	mstruct = vargs[0];
	if(!mstruct.integrate(vargs[1], eo)) {
		mstruct = vargs[0];
		mstruct.eval(eo);
		if(mstruct == vargs[0]) {
			return 0;
		}
		MathStructure mstruct2(mstruct);
		if(!mstruct.integrate(vargs[1], eo)) {
			mstruct = mstruct2;
			return -1;
		}
	}
	return 1;
}
SolveFunction::SolveFunction() : MathFunction("solve", 1, 2) {
	setArgumentDefinition(2, new SymbolicArgument());
	setDefaultValue(2, "x");
}
int SolveFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct = vargs[0];
	EvaluationOptions eo2 = eo;
	eo2.assume_denominators_nonzero = false;
	eo2.isolate_var = &vargs[1];
	mstruct.eval(eo2);

	if(mstruct.isComparison()) {
		if(mstruct[0] != vargs[1]) {
			CALCULATOR->error(true, _("Unable to isolate %s."), vargs[1].print().c_str(), NULL);
		} else {
			if(mstruct.comparisonType() == COMPARISON_EQUALS) {
				MathStructure msave(mstruct[1]);
				mstruct = msave;	
			}
			return 1 ;
		}
	} else {
		CALCULATOR->error(true, _("No comparison to solve. The reason might be:\n\n1. The entered expression to solve is not correct (ex. \"x + 5 = 3\" is correct)\n\n2. The expression evaluates FALSE. There is no valid solution with the current assumptions (ex. \"x = -5\" with x assumed positive).\n\n3. The expression evaluates TRUE (ex. \"2x = 2x\")"), NULL);
	}
	return -1;
	
}
SolveMultipleFunction::SolveMultipleFunction() : MathFunction("multisolve", 2) {
	setArgumentDefinition(1, new VectorArgument());
	VectorArgument *arg = new VectorArgument();
	arg->addArgument(new SymbolicArgument());
	arg->setReoccuringArguments(true);
	setArgumentDefinition(2, arg);
	setCondition("dimension(\\x)=dimension(\\y)");
}
int SolveMultipleFunction::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {

	mstruct.clearVector();
	
	if(vargs[1].size() < 1) return 1;
	
	bool eleft[vargs[0].size()];
	for(unsigned int i = 0; i < vargs[0].size(); i++) eleft[i] = true;
	vector<unsigned int> eorder;
	bool b = false;
	for(unsigned int i = 0; i < vargs[1].size(); i++) {
		b = false;
		for(unsigned int i2 = 0; i2 < vargs[0].size(); i2++) {
			if(eleft[i2] && vargs[0][i2].contains(vargs[1][i], true)) {
				eorder.push_back(i2);
				eleft[i2] = false;
				b = true;
				break;
			}
		}
		if(!b) {
			eorder.clear();
			for(unsigned int i2 = 0; i2 < vargs[0].size(); i2++) {
				eorder.push_back(i2);
			}
			break;
		}
	}
	
	for(unsigned int i = 0; i < eorder.size(); i++) {
		MathStructure msolve(vargs[0][eorder[i]]);
		EvaluationOptions eo2 = eo;
		eo2.assume_denominators_nonzero = false;
		eo2.isolate_var = &vargs[1][i];
		for(unsigned int i2 = 0; i2 < i; i2++) {
			msolve.replace(vargs[1][i2], mstruct[i2]);
		}
		msolve.eval(eo2);

		if(msolve.isComparison()) {
			if(msolve[0] != vargs[1][i]) {
				if(!b) {
					CALCULATOR->error(true, _("Unable to isolate %s.\n\nYou might need to place the equations and variables in an appropriate order so that so that each equation at least contains the corresponding variable (if automatic reordering failed)."), vargs[1][i].print().c_str(), NULL);
				} else {
					CALCULATOR->error(true, _("Unable to isolate %s."), vargs[1][i].print().c_str(), NULL);
				}
				return 0;
			} else {
				if(msolve.comparisonType() == COMPARISON_EQUALS) {
					mstruct.addItem(msolve[1]);
				} else {
					CALCULATOR->error(true, _("Only equals comparison is allowed in the equations in %s()."), preferredName().name.c_str(), NULL);
					return 0;
				}
			}
		} else {
			CALCULATOR->error(true, _("No comparison to solve. The reason might be:\n\n1. The entered expression to solve is not correct (ex. \"x + 5 = 3\" is correct)\n\n2. The expression evaluates FALSE. There is no valid solution with the current assumptions (ex. \"x = -5\" with x assumed positive).\n\n3. The expression evaluates TRUE (ex. \"2x = 2x\")"), NULL);
			return 0;
		}
		for(unsigned int i2 = 0; i2 < i; i2++) {
			for(unsigned int i3 = 0; i3 <= i; i3++) {
				if(i2 != i3) {
					mstruct[i2].replace(vargs[1][i3], mstruct[i3]);
				}
			}
		}
	}
	
	return 1;
	
}
