/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "BuiltinFunctions.h"

MatrixFunction::MatrixFunction() : Function("Matrices", "matrix", 2, "Construct Matrix", "", false, -1) {}
void MatrixFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[0]->fraction()->isPositive() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger() || !vargs[1]->fraction()->isPositive()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs.size());	
		mngr->set(mngr2);
		delete mngr2;
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
RowsFunction::RowsFunction() : Function("Matrices", "rows", 1, "Rows") {}
void RowsFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->rows(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ColumnsFunction::ColumnsFunction() : Function("Matrices", "columns", 1, "Columns") {}
void ColumnsFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]->matrix()->columns(), 1);
	} else {
		mngr->set(1, 1);
	}
}
ElementFunction::ElementFunction() : Function("Matrices", "element", 3, "Element") {}
void ElementFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[0]->fraction()->isPositive() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger() || !vargs[1]->fraction()->isPositive()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs.size());	
		mngr->set(mngr2);
		delete mngr2;
		CALCULATOR->error(true, _("Row and column in matrix must be positive integers."), NULL);
		return;
	}
	if(vargs[2]->isMatrix()) {
		mngr->set(vargs[2]->matrix()->get(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt()));
	} else {
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
	}
}
TransposeFunction::TransposeFunction() : Function("Matrices", "transpose", 1, "Transpose") {}
void TransposeFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]);
		mngr->matrix()->transpose();
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
IdentityFunction::IdentityFunction() : Function("Matrices", "identity", 1, "Identity") {}
void IdentityFunction::calculate2(Manager *mngr) {
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
DeterminantFunction::DeterminantFunction() : Function("Matrices", "det", 1, "Determinant") {}
void DeterminantFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isMatrix()) {
		Manager *det = vargs[0]->matrix()->determinant();
		mngr->set(det);
		delete det;	
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
CofactorFunction::CofactorFunction() : Function("Matrices", "cofactor", 3, "Cofactor") {}
void CofactorFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isFraction() || !vargs[0]->fraction()->isInteger() || !vargs[0]->fraction()->isPositive() || !vargs[1]->isFraction() || !vargs[1]->fraction()->isInteger() || !vargs[1]->fraction()->isPositive()) {
		Manager *mngr2 = createFunctionManagerFromVArgs(vargs.size());	
		mngr->set(mngr2);
		delete mngr2;
		CALCULATOR->error(true, _("Row and column in matrix must be positive integers."), NULL);
		return;
	}
	if(vargs[2]->isMatrix()) {
		Manager *mngr2 = vargs[2]->matrix()->cofactor(vargs[0]->fraction()->numerator()->getInt(), vargs[1]->fraction()->numerator()->getInt());
		mngr->set(mngr2);
		delete mngr2;
	} else {
		mngr->set(this, vargs[0], vargs[1], vargs[2], NULL);
	}
}
AdjointFunction::AdjointFunction() : Function("Matrices", "adj", 1, "Adjoint") {}
void AdjointFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]);
		mngr->matrix()->adjoint();	
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
InverseFunction::InverseFunction() : Function("Matrices", "inverse", 1, "Inverse") {}
void InverseFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isMatrix()) {
		mngr->set(vargs[0]);
		mngr->matrix()->inverse();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
IFFunction::IFFunction() : Function("Logical", "if", 3, "If...Then...Else") {
}
Manager *IFFunction::calculate(const string &argv) {
	Manager *mngr = NULL;
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		unsigned int i = svargs[0].find_first_of("<=>", 0);
		bool result = false;
		int com = 0;
		if(i == string::npos) {
			CALCULATOR->error(false, _("Condition contains no comparison, interpreting as \"%s > 0\"."), svargs[0].c_str(), NULL);
			svargs[0] += " > 0";
			i = svargs[0].find_first_of("<=>", 0);
		} 
		string str1 = svargs[0].substr(0, i);
		string str2 = svargs[0].substr(i + 1, svargs[0].length() - i + 1);			
		remove_blank_ends(str2);
		char sign1 = svargs[0][i], sign2 = 0;
		if(str2[0] != sign1 && (str2[0] == '>' || str2[0] == '=' || str2[0] == '<')) {
			sign2 = str2[0];
			str2.erase(0);
		}
		Manager *mngr1 = CALCULATOR->calculate(str1);
		Manager *mngr2 = CALCULATOR->calculate(str2);			
		mngr1->add(mngr2, SUBTRACT);
		if(mngr1->isNumber()) {
			if(sign1 == '=') {
				if(sign2 == 0) 
					result = mngr1->value() == 0L;
				else if(sign2 == '>') 
					result = mngr1->value() >= 0L;
				else if(sign2 == '<') 
					result = mngr1->value() <= 0L; 
			} else if(sign1 == '>') {
				if(sign2 == 0) 
					result = mngr1->value() > 0L;
				else if(sign2 == '=') 
					result = mngr1->value() >= 0L;
				else if(sign2 == '<') 
					result = mngr1->value() != 0L; 
			} else if(sign1 == '<') {
				if(sign2 == 0) 
					result = mngr1->value() < 0L;
				else if(sign2 == '>') 
					result = mngr1->value() != 0L;
				else if(sign2 == '=') 
					result = mngr1->value() <= 0L; 
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
		mngr = createFunctionManagerFromSVArgs(itmp);
	}
	clearSVArgs();
	return mngr;
}
GCDFunction::GCDFunction() : Function("Arithmetics", "gcd", 2, "Greatest Common Divisor") {
}
void GCDFunction::calculate2(Manager *mngr) {
	if(!vargs[0]->isFraction()) mngr->set(this, vargs[0], vargs[1], NULL);
	if(!vargs[1]->isFraction()) mngr->set(this, vargs[0], vargs[1], NULL);	
	mngr->set(vargs[0]);
	mngr->fraction()->gcd(vargs[1]->fraction());
}
DaysFunction::DaysFunction() : Function("Date & Time", "daysto", 2, "Days to date") {
}
Manager *DaysFunction::calculate(const string &argv) {
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		int days = daysBetweenDates(svargs[0], svargs[1], 1);
		if(days < 0) {
			CALCULATOR->error(false, _("Error in date format for function %s()."), name().c_str(), NULL);
		} else {
			Manager *mngr = new Manager(days, 1, 0);
			return mngr;
		}	
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
DaysBetweenDatesFunction::DaysBetweenDatesFunction() : Function("Date & Time", "days_between_dates", 2, "Days between two dates", "", false, 3) {
}
Manager *DaysBetweenDatesFunction::calculate(const string &argv) {
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		int basis = s2i(svargs[2]);
		int days = daysBetweenDates(svargs[0], svargs[1], basis);
		if(days < 0) {
			CALCULATOR->error(false, _("Error in date format for function %s()."), name().c_str(), NULL);
		} else {
			Manager *mngr = new Manager(days, 1, 0);
			return mngr;
		}		
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
YearsBetweenDatesFunction::YearsBetweenDatesFunction() : Function("Date & Time", "years_between_dates", 2, "Years between two dates", "", false, 3) {
}
Manager *YearsBetweenDatesFunction::calculate(const string &argv) {
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		int basis = s2i(svargs[2]);
		Fraction *fr = yearsBetweenDates(svargs[0], svargs[1], basis);
		if(!fr) {
			CALCULATOR->error(false, _("Error in date format for function %s()."), name().c_str(), NULL);
		} else {
			Manager *mngr = new Manager(fr);
			return mngr;
		}		
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
DifferentiateFunction::DifferentiateFunction() : Function("Experimental", "diff", 2, "Differentiate") {
}
Manager *DifferentiateFunction::calculate(const string &argv) {
//	CALCULATOR->error(true, _("%s() is an experimental unfinished function!"), name().c_str(), NULL);
	Manager *mngr = NULL;
	int itmp = stringArgs(argv);
	if(testArgCount(itmp)) {
		mngr = CALCULATOR->calculate(svargs[0]);
		mngr->differentiate(svargs[1]);
	} else {
		mngr = createFunctionManagerFromSVArgs(itmp);
	}
	clearSVArgs();
	return mngr;	
}
FactorialFunction::FactorialFunction() : Function("Arithmetics", "factorial", 1, "Factorial") {

}
void FactorialFunction::calculate2(Manager *mngr) {
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
void AbsFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->setNegative(false);		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
CeilFunction::CeilFunction() : Function("Arithmetics", "ceil", 1, "Round upwards") {

}
void CeilFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->ceil();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
FloorFunction::FloorFunction() : Function("Arithmetics", "floor", 1, "Round downwards") {

}
void FloorFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->floor();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
TruncFunction::TruncFunction() : Function("Arithmetics", "trunc", 1, "Round towards zero") {

}
void TruncFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->trunc();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
RoundFunction::RoundFunction() : Function("Arithmetics", "round", 1, "Round") {

}
void RoundFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->round();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
FracFunction::FracFunction() : Function("Arithmetics", "frac", 1, "Extract fractional part") {

}
void FracFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->mod();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
IntFunction::IntFunction() : Function("Arithmetics", "int", 1, "Extract integer part") {

}
void IntFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->trunc();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
RemFunction::RemFunction() : Function("Arithmetics", "rem", 2, "Reminder (rem)") {

}
void RemFunction::calculate2(Manager *mngr) {
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

}
void ModFunction::calculate2(Manager *mngr) {
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
void SinFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->sin();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
CosFunction::CosFunction() : Function("Trigonometry", "cos", 1, "Cosine") {}
void CosFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->cos();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
TanFunction::TanFunction() : Function("Trigonometry", "tan", 1, "Tangent") {}
void TanFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->tan();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
SinhFunction::SinhFunction() : Function("Trigonometry", "sinh", 1, "Hyperbolic sine") {}
void SinhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->sinh();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
CoshFunction::CoshFunction() : Function("Trigonometry", "cosh", 1, "Hyperbolic cosine") {}
void CoshFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->cosh();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
TanhFunction::TanhFunction() : Function("Trigonometry", "tanh", 1, "Hyperbolic tangent") {}
void TanhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->tanh();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
AsinFunction::AsinFunction() : Function("Trigonometry", "asin", 1, "Arcsine") {}
void AsinFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->asin();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
AcosFunction::AcosFunction() : Function("Trigonometry", "acos", 1, "Arccosine") {}
void AcosFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->acos();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
AtanFunction::AtanFunction() : Function("Trigonometry", "atan", 1, "Arctangent") {}
void AtanFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->atan();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
AsinhFunction::AsinhFunction() : Function("Trigonometry", "asinh", 1, "Hyperbolic arcsine") {}
void AsinhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->asinh();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
AcoshFunction::AcoshFunction() : Function("Trigonometry", "acosh", 1, "Hyperbolic arccosine") {}
void AcoshFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->acosh();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
AtanhFunction::AtanhFunction() : Function("Trigonometry", "atanh", 1, "Hyperbolic arctangent") {}
void AtanhFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		CALCULATOR->setAngleValue(mngr);
		mngr->fraction()->atanh();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
LogFunction::LogFunction() : Function("Exponents and Logarithms", "ln", 1, "Natural Logarithm") {}
void LogFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isZero()) {
		CALCULATOR->error(true, _("It is not possible to calculate the logarithm of zero."), NULL);
		mngr->set(this, vargs[0], NULL);
	} else if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->log();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
Log10Function::Log10Function() : Function("Exponents and Logarithms", "log", 1, "Base-10 Logarithm") {}
void Log10Function::calculate2(Manager *mngr) {
	if(vargs[0]->isZero()) {
		CALCULATOR->error(true, _("It is not possible to calculate the logarithm of zero."), NULL);
		mngr->set(this, vargs[0], NULL);
	} else if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->log10();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
Log2Function::Log2Function() : Function("Exponents and Logarithms", "log2", 1, "Base-2 Logarithm") {}
void Log2Function::calculate2(Manager *mngr) {
	if(vargs[0]->isZero()) {
		CALCULATOR->error(true, _("It is not possible to calculate the logarithm of zero."), NULL);
		mngr->set(this, vargs[0], NULL);
	} else if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->log2();		
	} else {
		mngr->set(this, vargs[0], NULL);
	}
}
ExpFunction::ExpFunction() : Function("Exponents and Logarithms", "exp", 1, "e raised to the power X") {}
void ExpFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->exp();		
	} else {
		mngr->set(E_VALUE);
		mngr->add(vargs[0], RAISE);	
	}
}
Exp10Function::Exp10Function() : Function("Exponents and Logarithms", "exp10", 1, "10 raised the to power X") {}
void Exp10Function::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->exp10();		
	} else {
		mngr->set(10);
		mngr->add(vargs[0], RAISE);	
	}
}
Exp2Function::Exp2Function() : Function("Exponents and Logarithms", "exp2", 1, "2 raised the to power X") {}
void Exp2Function::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->exp2();		
	} else {
		mngr->set(2);
		mngr->add(vargs[0], RAISE);	
	}
}
SqrtFunction::SqrtFunction() : Function("Exponents and Logarithms", "sqrt", 1, "Square Root") {

}
void SqrtFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		if(vargs[0]->fraction()->isNegative()) {
			CALCULATOR->error(true, _("Trying to calculate the square root of a negative number (%s)."), d2s(vargs[0]->value()).c_str(), NULL);
			mngr->set(this, vargs[0], NULL);
			return;
		}	
		mngr->set(vargs[0]);
		mngr->fraction()->sqrt();		
	} else {
		mngr->set(vargs[0]);
		Manager *mngr2 = new Manager(1, 2);
		mngr->add(mngr2, RAISE);
		mngr2->unref();		
	}
}
CbrtFunction::CbrtFunction() : Function("Exponents and Logarithms", "cbrt", 1, "Cube Root") {}
void CbrtFunction::calculate2(Manager *mngr) {
	if(vargs[0]->isFraction()) {
		mngr->set(vargs[0]);
		mngr->fraction()->cbrt();		
	} else {
		mngr->set(vargs[0]);
		Manager *mngr2 = new Manager(1, 3);		
		mngr->add(mngr2, RAISE);
		mngr2->unref();	
	}
}
HypotFunction::HypotFunction() : Function("Geometry", "hypot", 2, "Hypotenuse") {

}
void HypotFunction::calculate2(Manager *mngr) {
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
SumFunction::SumFunction() : Function("Statistics", "sum", -1, "Sum") {}
void SumFunction::calculate2(Manager *mngr) {
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], ADD);
	}
}
MeanFunction::MeanFunction() : Function("Statistics", "mean", -1, "Mean") {}
void MeanFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		mngr->add(vargs[i], ADD);
	}
	mngr->addInteger(vargs.size(), DIVIDE);	
}
MedianFunction::MedianFunction() : Function("Statistics", "median", -1, "Median") {}
void MedianFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	list<long double> largs;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		largs.push_back(vargs[i]->value());
	}
	largs.sort();
	list<long double>::iterator it = largs.begin();
	for(unsigned int i = 0; i < largs.size() / 2 - 1; i++) {
	    ++it;
	}	
	if(vargs.size() % 2 == 0) {
		mngr->set(*it);
		++it;
		mngr->addFloat(*it, ADD);
		mngr->addFloat(2, DIVIDE);
	} else {
		++it;
		mngr->set(*it);
	}
}
MinFunction::MinFunction() : Function("Statistics", "min", -1, "Min") {}
void MinFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	long double value = vargs[0]->value();
	unsigned int ui = 0;
	for(unsigned int i = 1; i < vargs.size(); i++) {
		if(vargs[i]->value() < value) {
			value = vargs[i]->value();
			ui = i;
		}
	}
	mngr->set(vargs[ui]);
}
MaxFunction::MaxFunction() : Function("Statistics", "max", -1, "Max") {}
void MaxFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0)
		return;
	long double value = vargs[0]->value();
	unsigned int ui = 0;
	for(unsigned int i = 1; i < vargs.size(); i++) {
		if(vargs[i]->value() > value) {
			value = vargs[i]->value();
			ui = i;
		}
	}
	mngr->set(vargs[ui]);
}
ModeFunction::ModeFunction() : Function("Statistics", "mode", -1, "Mode") {}
void ModeFunction::calculate2(Manager *mngr) {
	if(vargs.size() <= 0) {
		return;
	}
	int n = 0;
	bool b;
	vector<Manager*> vargs_nodup;
	vector<int> is;
	Manager *value;
	for(unsigned int i = 0; i < vargs.size(); i++) {
		b = true;
		for(unsigned int i2 = 0; i2 < vargs_nodup.size(); i2++) {
			if(vargs_nodup[i2]->equals(vargs[i])) {
				is[i2]++;
				b = false;
				break;
			}
		}
		if(b) {
			vargs_nodup.push_back(vargs[i]);
			is.push_back(1);
		}
	}
	for(unsigned int i = 0; i < is.size(); i++) {
		if(is[i] > n) {
			n = is[i];
			value = vargs_nodup[i];
		}
	}
	mngr->set(value);
}
NumberFunction::NumberFunction() : Function("Statistics", "number", -1, "Number") {}
void NumberFunction::calculate2(Manager *mngr) {
	mngr->set(vargs.size(), 1);
}
StdDevFunction::StdDevFunction() : Function("Statistics", "stddev", -1, "Standard Deviation") {}
void StdDevFunction::calculate2(Manager *mngr) {
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
StdDevSFunction::StdDevSFunction() : Function("Statistics", "stddevs", -1, "Standard Deviation (random sampling)") {}
void StdDevSFunction::calculate2(Manager *mngr) {
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
Manager *RandomFunction::calculate(const string &eq) {
	Manager *mngr = new Manager();
	mngr->set(drand48());
	return mngr;
}

BASEFunction::BASEFunction() : Function("General", "BASE", 2, "Number Base") {

}
Manager *BASEFunction::calculate(const string &eq) {
	int itmp = stringArgs(eq);
	if(testArgCount(itmp)) {
		Manager *mngr = CALCULATOR->calculate(svargs[1]);	
		int base = (int) mngr->value();
		long double value = 0;
		if(base < 2 || base > 36) {
			CALCULATOR->error(false, _("Base must be between 2 and 36 (was %s) for function %s()."), i2s(base).c_str(), name().c_str(), NULL);
			Manager *mngr2 = new Manager(base);
			Manager *mngr3 = new Manager(svargs[0]);			
			mngr->set(this, mngr3, mngr2, NULL);
			mngr2->unref();
			mngr3->unref();
		} else {
			value = (long double) strtol(svargs[0].c_str(), NULL, base);
			clearSVArgs();
			mngr->set(value);
		}
		return mngr;
	}
	Manager *mngr = createFunctionManagerFromSVArgs(itmp);
	clearSVArgs();
	return mngr;			
}
BINFunction::BINFunction() : Function("General", "BIN", 1, "Binary") {

}
Manager *BINFunction::calculate(const string &eq) {
	Manager *mngr = new Manager();
	string str = eq;
	remove_blanks(str);
	mngr->set((long double) strtol(str.c_str(), NULL, 2));
	return mngr;
}
OCTFunction::OCTFunction() : Function("General", "OCT", 1, "Octal") {

}
Manager *OCTFunction::calculate(const string &eq) {
	Manager *mngr = new Manager();
	mngr->set((long double) strtol(eq.c_str(), NULL, 8));
	return mngr;
}
HEXFunction::HEXFunction() : Function("General", "HEX", 1, "Hexadecimal") {

}
Manager *HEXFunction::calculate(const string &eq) {
	string stmp;
	if(eq.length() >= 2 && eq[0] == '0' && (eq[1] == 'x' || eq[1] == 'X'))
		stmp = eq;
	else {
		stmp = "0x";
		stmp += eq;
	}
	Manager *mngr = new Manager();
	mngr->set(strtold(stmp.c_str(), NULL));
	return mngr;
}

