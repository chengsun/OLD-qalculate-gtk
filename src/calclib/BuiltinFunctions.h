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

class IFFunction : public Function {
  protected:
	Manager *calculate(const string &argv);  
  public:
	IFFunction();
};
class DifferentiateFunction : public Function {
  protected:
	Manager *calculate(const string &argv);    
  public:
	DifferentiateFunction();
};
class GCDFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	GCDFunction();
};
class DaysFunction : public Function {
  protected:
	Manager *calculate(const string &argv);    
  public:
	DaysFunction();
};
class DaysBetweenDatesFunction : public Function {
  protected:
	Manager *calculate(const string &argv);    
  public:
	DaysBetweenDatesFunction();
};
class YearsBetweenDatesFunction : public Function {
  protected:
	Manager *calculate(const string &argv);    
  public:
	YearsBetweenDatesFunction();
};
class AbsFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AbsFunction();
};
class CeilFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CeilFunction();
};
class FloorFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	FloorFunction();
};
class TruncFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	TruncFunction();
};
class RoundFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	RoundFunction();
};
class FracFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	FracFunction();
};
class IntFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	IntFunction();
};
class RemFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	RemFunction();
};
class ModFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	ModFunction();
};
class SinFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SinFunction();
};
class CosFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CosFunction();
};
class TanFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	TanFunction();
};
class SinhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SinhFunction();
};
class CoshFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CoshFunction();
};
class TanhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	TanhFunction();
};
class AsinFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AsinFunction();
};
class AcosFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AcosFunction();
};
class AtanFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AtanFunction();
};
class AsinhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AsinhFunction();
};
class AcoshFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AcoshFunction();
};
class AtanhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AtanhFunction();
};
class LogFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	LogFunction();
};
class Log10Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Log10Function();
};
class Log2Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Log2Function();
};
class ExpFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	ExpFunction();
};
class Exp10Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Exp10Function();
};
class Exp2Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Exp2Function();
};
class SqrtFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SqrtFunction();
};
class CbrtFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CbrtFunction();
};
class HypotFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	HypotFunction();
};
class SumFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SumFunction();
};
class MeanFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MeanFunction();
};
class MedianFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MedianFunction();
};
class MinFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MinFunction();
};
class MaxFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MaxFunction();
};
class ModeFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	ModeFunction();
};
class NumberFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	NumberFunction();
};
class StdDevFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	StdDevFunction();
};
class StdDevSFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	StdDevSFunction();
};
class RandomFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	RandomFunction();
};
class BASEFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	BASEFunction();
};
class BINFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	BINFunction();
};
class HEXFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	HEXFunction();
};
class OCTFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	OCTFunction();
};

#endif
