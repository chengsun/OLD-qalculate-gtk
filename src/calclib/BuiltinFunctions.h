/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef BUILTIN_FUNCTIONS_H
#define BUILTIN_FUNCTIONS_H

#include "Function.h"
#include "includes.h"

//CONSTANTS
class PiFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	PiFunction();
};
class EFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	EFunction();
};
class EulerFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	EulerFunction();
};
class CatalanFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	CatalanFunction();
};
class AperyFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AperyFunction();
};
class PythagorasFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	PythagorasFunction();
};
class GoldenFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	GoldenFunction();
};

#ifdef HAVE_LIBCLN
class ZetaFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ZetaFunction();
};
#endif
class ProcessFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ProcessFunction();
};
class CustomSumFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	CustomSumFunction();
};
class FunctionFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	FunctionFunction();
};
class MatrixFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	MatrixFunction();
};
class VectorFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	VectorFunction();
};
class MatrixToVectorFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	MatrixToVectorFunction();
};
class LimitsFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	LimitsFunction();
};
class RankFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	RankFunction();
};
class SortFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	SortFunction();
};
class RowsFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	RowsFunction();
};
class ColumnsFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ColumnsFunction();
};
class RowFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	RowFunction();
};
class ColumnFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ColumnFunction();
};
class ElementsFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ElementsFunction();
};
class ElementFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ElementFunction();
};
class ComponentsFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ComponentsFunction();
};
class ComponentFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ComponentFunction();
};
class TransposeFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	TransposeFunction();
};
class IdentityFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	IdentityFunction();
};
class DeterminantFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	DeterminantFunction();
};
class AdjointFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AdjointFunction();
};
class CofactorFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	CofactorFunction();
};
class InverseFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	InverseFunction();
};
class IFFunction : public Function {
  protected:
	Manager *calculate(const string &argv);  
	Manager *calculate(vector<Manager*> &vargs);  
  public:
	IFFunction();
};
class DifferentiateFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	DifferentiateFunction();
};
class GCDFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	GCDFunction();
};
class DaysFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	DaysFunction();
};
class YearFracFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	YearFracFunction();
};
class FactorialFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	FactorialFunction();
};
class AbsFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AbsFunction();
};
class CeilFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	CeilFunction();
};
class FloorFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	FloorFunction();
};
class TruncFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	TruncFunction();
};
class RoundFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	RoundFunction();
};
class FracFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	FracFunction();
};
class IntFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	IntFunction();
};
class RemFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	RemFunction();
};
class ModFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ModFunction();
};
class SinFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	SinFunction();
};
class CosFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	CosFunction();
};
class TanFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	TanFunction();
};
class SinhFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	SinhFunction();
};
class CoshFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	CoshFunction();
};
class TanhFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	TanhFunction();
};
class AsinFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AsinFunction();
};
class AcosFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AcosFunction();
};
class AtanFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AtanFunction();
};
class AsinhFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AsinhFunction();
};
class AcoshFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AcoshFunction();
};
class AtanhFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	AtanhFunction();
};
class LogFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	LogFunction();
};
class Log10Function : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	Log10Function();
};
class Log2Function : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	Log2Function();
};
class ExpFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ExpFunction();
};
class Exp10Function : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	Exp10Function();
};
class Exp2Function : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	Exp2Function();
};
class SqrtFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	SqrtFunction();
};
class CbrtFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	CbrtFunction();
};
class RootFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	RootFunction();
};
class PowFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	PowFunction();
};
class HypotFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	HypotFunction();
};
class SumFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	SumFunction();
};
class MeanFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	MeanFunction();
};
class MedianFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	MedianFunction();
};
class PercentileFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	PercentileFunction();
};
class MinFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	MinFunction();
};
class MaxFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	MaxFunction();
};
class ModeFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	ModeFunction();
};
class NumberFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	NumberFunction();
};
class StdDevFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	StdDevFunction();
};
class StdDevSFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	StdDevSFunction();
};
class RandomFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	RandomFunction();
};
class BASEFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	BASEFunction();
};
class BINFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	BINFunction();
};
class HEXFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	HEXFunction();
};
class OCTFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	OCTFunction();
};
class TitleFunction : public Function {
  protected:
	void calculate(Manager *mngr, vector<Manager*> &vargs);  
  public:
	TitleFunction();
};

#endif
