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

#define DECLARE_BUILTIN_FUNCTION(x)	class x : public Function { \
					  protected:\
						void calculate(Manager *mngr, vector<Manager*> &vargs);  \
					  public: \
						x(); \
						x(const x *function) {set(function);} \
						ExpressionItem *copy() const {return new x(this);} \
					};

#ifdef HAVE_LIBCLN
DECLARE_BUILTIN_FUNCTION(ZetaFunction);
#endif
DECLARE_BUILTIN_FUNCTION(ForFunction);
DECLARE_BUILTIN_FUNCTION(SumFunction);
DECLARE_BUILTIN_FUNCTION(ProductFunction);
DECLARE_BUILTIN_FUNCTION(ProcessFunction);
DECLARE_BUILTIN_FUNCTION(CustomSumFunction);
DECLARE_BUILTIN_FUNCTION(FunctionFunction);
DECLARE_BUILTIN_FUNCTION(MatrixFunction);
DECLARE_BUILTIN_FUNCTION(VectorFunction);
DECLARE_BUILTIN_FUNCTION(MatrixToVectorFunction);
DECLARE_BUILTIN_FUNCTION(LimitsFunction);
DECLARE_BUILTIN_FUNCTION(AreaFunction);
DECLARE_BUILTIN_FUNCTION(RangeFunction);
DECLARE_BUILTIN_FUNCTION(RankFunction);
DECLARE_BUILTIN_FUNCTION(SortFunction);
DECLARE_BUILTIN_FUNCTION(RowsFunction);
DECLARE_BUILTIN_FUNCTION(ColumnsFunction);
DECLARE_BUILTIN_FUNCTION(RowFunction);
DECLARE_BUILTIN_FUNCTION(ColumnFunction);
DECLARE_BUILTIN_FUNCTION(ElementsFunction);
DECLARE_BUILTIN_FUNCTION(ElementFunction);
DECLARE_BUILTIN_FUNCTION(ComponentFunction);
DECLARE_BUILTIN_FUNCTION(ComponentsFunction);
DECLARE_BUILTIN_FUNCTION(TransposeFunction);
DECLARE_BUILTIN_FUNCTION(IdentityFunction);
DECLARE_BUILTIN_FUNCTION(DeterminantFunction);
DECLARE_BUILTIN_FUNCTION(PermanentFunction);
DECLARE_BUILTIN_FUNCTION(AdjointFunction);
DECLARE_BUILTIN_FUNCTION(CofactorFunction);
DECLARE_BUILTIN_FUNCTION(InverseFunction);
DECLARE_BUILTIN_FUNCTION(GCDFunction);
DECLARE_BUILTIN_FUNCTION(DaysFunction);
DECLARE_BUILTIN_FUNCTION(YearFracFunction);
DECLARE_BUILTIN_FUNCTION(WeekFunction);
DECLARE_BUILTIN_FUNCTION(WeekdayFunction);
DECLARE_BUILTIN_FUNCTION(MonthFunction);
DECLARE_BUILTIN_FUNCTION(DayFunction);
DECLARE_BUILTIN_FUNCTION(YearFunction);
DECLARE_BUILTIN_FUNCTION(YeardayFunction);
DECLARE_BUILTIN_FUNCTION(TimeFunction);
DECLARE_BUILTIN_FUNCTION(FactorialFunction);
DECLARE_BUILTIN_FUNCTION(BinomialFunction);
DECLARE_BUILTIN_FUNCTION(AbsFunction);
DECLARE_BUILTIN_FUNCTION(CeilFunction);
DECLARE_BUILTIN_FUNCTION(FloorFunction);
DECLARE_BUILTIN_FUNCTION(TruncFunction);
DECLARE_BUILTIN_FUNCTION(RoundFunction);
DECLARE_BUILTIN_FUNCTION(IntFunction);
DECLARE_BUILTIN_FUNCTION(FracFunction);
DECLARE_BUILTIN_FUNCTION(ImaginaryPartFunction);
DECLARE_BUILTIN_FUNCTION(RealPartFunction);
DECLARE_BUILTIN_FUNCTION(ModFunction);
DECLARE_BUILTIN_FUNCTION(RemFunction);
DECLARE_BUILTIN_FUNCTION(LogFunction);
DECLARE_BUILTIN_FUNCTION(LognFunction);
DECLARE_BUILTIN_FUNCTION(TotalFunction);
DECLARE_BUILTIN_FUNCTION(PercentileFunction);
DECLARE_BUILTIN_FUNCTION(MinFunction);
DECLARE_BUILTIN_FUNCTION(MaxFunction);
DECLARE_BUILTIN_FUNCTION(ModeFunction);
DECLARE_BUILTIN_FUNCTION(RandomFunction);
DECLARE_BUILTIN_FUNCTION(BASEFunction);
DECLARE_BUILTIN_FUNCTION(BINFunction);
DECLARE_BUILTIN_FUNCTION(HEXFunction);
DECLARE_BUILTIN_FUNCTION(OCTFunction);
DECLARE_BUILTIN_FUNCTION(RomanFunction);
DECLARE_BUILTIN_FUNCTION(TitleFunction);
DECLARE_BUILTIN_FUNCTION(IFFunction);
DECLARE_BUILTIN_FUNCTION(ErrorFunction);
DECLARE_BUILTIN_FUNCTION(WarningFunction);
DECLARE_BUILTIN_FUNCTION(SaveFunction);
DECLARE_BUILTIN_FUNCTION(ConcatenateFunction);
DECLARE_BUILTIN_FUNCTION(LengthFunction);
DECLARE_BUILTIN_FUNCTION(AsciiFunction);
DECLARE_BUILTIN_FUNCTION(CharFunction);
DECLARE_BUILTIN_FUNCTION(LoadFunction);
DECLARE_BUILTIN_FUNCTION(ReplaceFunction);

DECLARE_BUILTIN_FUNCTION(DeriveFunction);
DECLARE_BUILTIN_FUNCTION(IntegrateFunction);
DECLARE_BUILTIN_FUNCTION(SolveFunction);
#ifdef HAVE_GIAC
DECLARE_BUILTIN_FUNCTION(GiacDeriveFunction);
DECLARE_BUILTIN_FUNCTION(GiacFunction);
DECLARE_BUILTIN_FUNCTION(GiacSolveFunction);
DECLARE_BUILTIN_FUNCTION(GiacIntegrateFunction);
#endif

#endif
