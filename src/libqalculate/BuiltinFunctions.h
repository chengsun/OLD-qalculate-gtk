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

#include <libqalculate/Function.h>
#include <libqalculate/includes.h>

#define DECLARE_BUILTIN_FUNCTION(x)	class x : public MathFunction { \
					  public: \
						int calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo);  \
						x(); \
						x(const x *function) {set(function);} \
						ExpressionItem *copy() const {return new x(this);} \
					};					

#define DECLARE_BUILTIN_FUNCTION_R(x)	class x : public MathFunction { \
					  public: \
						int calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo);  \
						x(); \
						x(const x *function) {set(function);} \
						ExpressionItem *copy() const {return new x(this);} \
						bool representsPositive(const MathStructure &vargs);\
						bool representsNegative(const MathStructure &vargs);\
						bool representsNonNegative(const MathStructure &vargs);\
						bool representsNonPositive(const MathStructure &vargs);\
						bool representsInteger(const MathStructure &vargs);\
						bool representsNumber(const MathStructure &vargs);\
						bool representsRational(const MathStructure &vargs);\
						bool representsReal(const MathStructure &vargs);\
						bool representsComplex(const MathStructure &vargs);\
						bool representsNonZero(const MathStructure &vargs);\
						bool representsEven(const MathStructure &vargs);\
						bool representsOdd(const MathStructure &vargs);\
						bool representsUndefined(const MathStructure &vargs);\
					};
					
#define DECLARE_BUILTIN_FUNCTION_R2(x)	class x : public MathFunction { \
					  public: \
						int calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo);  \
						x(); \
						x(const x *function) {set(function);} \
						ExpressionItem *copy() const {return new x(this);} \
						bool representsNumber(const MathStructure &vargs);\
						bool representsReal(const MathStructure &vargs);\
					};
#define DECLARE_BUILTIN_FUNCTION_R1(x)	class x : public MathFunction { \
					  public: \
						int calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo);  \
						x(); \
						x(const x *function) {set(function);} \
						ExpressionItem *copy() const {return new x(this);} \
						bool representsNumber(const MathStructure &vargs);\
					};															


DECLARE_BUILTIN_FUNCTION(VectorFunction);
DECLARE_BUILTIN_FUNCTION(LimitsFunction);
DECLARE_BUILTIN_FUNCTION(RankFunction);
DECLARE_BUILTIN_FUNCTION(SortFunction);
DECLARE_BUILTIN_FUNCTION(ComponentFunction);
DECLARE_BUILTIN_FUNCTION(ComponentsFunction);

DECLARE_BUILTIN_FUNCTION(MatrixFunction);
DECLARE_BUILTIN_FUNCTION(MergeVectorsFunction);
DECLARE_BUILTIN_FUNCTION(MatrixToVectorFunction);
DECLARE_BUILTIN_FUNCTION(AreaFunction);
DECLARE_BUILTIN_FUNCTION(RowsFunction);
DECLARE_BUILTIN_FUNCTION(ColumnsFunction);
DECLARE_BUILTIN_FUNCTION(RowFunction);
DECLARE_BUILTIN_FUNCTION(ColumnFunction);
DECLARE_BUILTIN_FUNCTION(ElementsFunction);
DECLARE_BUILTIN_FUNCTION(ElementFunction);
DECLARE_BUILTIN_FUNCTION(TransposeFunction);
DECLARE_BUILTIN_FUNCTION(IdentityFunction);
DECLARE_BUILTIN_FUNCTION(DeterminantFunction);
DECLARE_BUILTIN_FUNCTION(PermanentFunction);
DECLARE_BUILTIN_FUNCTION(AdjointFunction);
DECLARE_BUILTIN_FUNCTION(CofactorFunction);
DECLARE_BUILTIN_FUNCTION(InverseFunction);

DECLARE_BUILTIN_FUNCTION_R(FactorialFunction);
DECLARE_BUILTIN_FUNCTION_R(DoubleFactorialFunction);
DECLARE_BUILTIN_FUNCTION_R(MultiFactorialFunction);
DECLARE_BUILTIN_FUNCTION(BinomialFunction);

DECLARE_BUILTIN_FUNCTION_R(AbsFunction);
DECLARE_BUILTIN_FUNCTION(GcdFunction);
DECLARE_BUILTIN_FUNCTION(SignumFunction);
DECLARE_BUILTIN_FUNCTION_R(RoundFunction);
DECLARE_BUILTIN_FUNCTION_R(FloorFunction);
DECLARE_BUILTIN_FUNCTION_R(CeilFunction);
DECLARE_BUILTIN_FUNCTION_R(TruncFunction);
DECLARE_BUILTIN_FUNCTION(IntFunction);
DECLARE_BUILTIN_FUNCTION(FracFunction);
DECLARE_BUILTIN_FUNCTION(RemFunction);
DECLARE_BUILTIN_FUNCTION(ModFunction);

DECLARE_BUILTIN_FUNCTION_R(ReFunction);
DECLARE_BUILTIN_FUNCTION_R(ImFunction);
DECLARE_BUILTIN_FUNCTION(ArgFunction);

DECLARE_BUILTIN_FUNCTION(SqrtFunction);
DECLARE_BUILTIN_FUNCTION(SquareFunction);

DECLARE_BUILTIN_FUNCTION(ExpFunction);

DECLARE_BUILTIN_FUNCTION_R(LogFunction);
DECLARE_BUILTIN_FUNCTION(LognFunction);

DECLARE_BUILTIN_FUNCTION_R2(SinFunction);
DECLARE_BUILTIN_FUNCTION_R2(CosFunction);
DECLARE_BUILTIN_FUNCTION(TanFunction);
DECLARE_BUILTIN_FUNCTION_R1(AsinFunction);
DECLARE_BUILTIN_FUNCTION_R1(AcosFunction);
DECLARE_BUILTIN_FUNCTION_R2(AtanFunction);
DECLARE_BUILTIN_FUNCTION_R2(SinhFunction);
DECLARE_BUILTIN_FUNCTION_R2(CoshFunction);
DECLARE_BUILTIN_FUNCTION_R2(TanhFunction);
DECLARE_BUILTIN_FUNCTION_R2(AsinhFunction);
DECLARE_BUILTIN_FUNCTION_R1(AcoshFunction);
DECLARE_BUILTIN_FUNCTION(AtanhFunction);
DECLARE_BUILTIN_FUNCTION(RadiansToDefaultAngleUnitFunction);

DECLARE_BUILTIN_FUNCTION(ZetaFunction);
DECLARE_BUILTIN_FUNCTION(GammaFunction);
DECLARE_BUILTIN_FUNCTION(BetaFunction);

DECLARE_BUILTIN_FUNCTION(TotalFunction);
DECLARE_BUILTIN_FUNCTION(PercentileFunction);
DECLARE_BUILTIN_FUNCTION(MinFunction);
DECLARE_BUILTIN_FUNCTION(MaxFunction);
DECLARE_BUILTIN_FUNCTION(ModeFunction);
DECLARE_BUILTIN_FUNCTION(RandFunction);

DECLARE_BUILTIN_FUNCTION(DaysFunction);
DECLARE_BUILTIN_FUNCTION(YearFracFunction);
DECLARE_BUILTIN_FUNCTION(WeekFunction);
DECLARE_BUILTIN_FUNCTION(WeekdayFunction);
DECLARE_BUILTIN_FUNCTION(MonthFunction);
DECLARE_BUILTIN_FUNCTION(DayFunction);
DECLARE_BUILTIN_FUNCTION(YearFunction);
DECLARE_BUILTIN_FUNCTION(YeardayFunction);
DECLARE_BUILTIN_FUNCTION(TimeFunction);

DECLARE_BUILTIN_FUNCTION(BinFunction);
DECLARE_BUILTIN_FUNCTION(OctFunction);
DECLARE_BUILTIN_FUNCTION(HexFunction);
DECLARE_BUILTIN_FUNCTION(BaseFunction);
DECLARE_BUILTIN_FUNCTION(RomanFunction);

DECLARE_BUILTIN_FUNCTION(AsciiFunction);
DECLARE_BUILTIN_FUNCTION(CharFunction);

DECLARE_BUILTIN_FUNCTION(LengthFunction);
DECLARE_BUILTIN_FUNCTION(ConcatenateFunction);

DECLARE_BUILTIN_FUNCTION(ReplaceFunction);

DECLARE_BUILTIN_FUNCTION(ForFunction);
DECLARE_BUILTIN_FUNCTION(SumFunction);
DECLARE_BUILTIN_FUNCTION(ProductFunction);
DECLARE_BUILTIN_FUNCTION(ProcessFunction);
DECLARE_BUILTIN_FUNCTION(ProcessMatrixFunction);
DECLARE_BUILTIN_FUNCTION(CustomSumFunction);
DECLARE_BUILTIN_FUNCTION(FunctionFunction);
DECLARE_BUILTIN_FUNCTION(SelectFunction);
DECLARE_BUILTIN_FUNCTION(TitleFunction);
DECLARE_BUILTIN_FUNCTION(IFFunction);
DECLARE_BUILTIN_FUNCTION(ErrorFunction);
DECLARE_BUILTIN_FUNCTION(WarningFunction);
DECLARE_BUILTIN_FUNCTION(MessageFunction);
DECLARE_BUILTIN_FUNCTION(SaveFunction);
DECLARE_BUILTIN_FUNCTION(LoadFunction);
DECLARE_BUILTIN_FUNCTION(ExportFunction);

DECLARE_BUILTIN_FUNCTION(DeriveFunction);
DECLARE_BUILTIN_FUNCTION(IntegrateFunction);
DECLARE_BUILTIN_FUNCTION(SolveFunction);
DECLARE_BUILTIN_FUNCTION(SolveMultipleFunction);

#endif
