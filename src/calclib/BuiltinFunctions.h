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
	IFFunction(Calculator *calc_);
};
class AbsFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AbsFunction(Calculator *calc_);
};
class CeilFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CeilFunction(Calculator *calc_);
};
class FloorFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	FloorFunction(Calculator *calc_);
};
class TruncFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	TruncFunction(Calculator *calc_);
};
class RoundFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	RoundFunction(Calculator *calc_);
};
class RemFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	RemFunction(Calculator *calc_);
};
class ModFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	ModFunction(Calculator *calc_);
};
class SinFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SinFunction(Calculator *calc_);
};
class CosFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CosFunction(Calculator *calc_);
};
class TanFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	TanFunction(Calculator *calc_);
};
class SinhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SinhFunction(Calculator *calc_);
};
class CoshFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CoshFunction(Calculator *calc_);
};
class TanhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	TanhFunction(Calculator *calc_);
};
class AsinFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AsinFunction(Calculator *calc_);
};
class AcosFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AcosFunction(Calculator *calc_);
};
class AtanFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AtanFunction(Calculator *calc_);
};
class AsinhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AsinhFunction(Calculator *calc_);
};
class AcoshFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AcoshFunction(Calculator *calc_);
};
class AtanhFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	AtanhFunction(Calculator *calc_);
};
class LogFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	LogFunction(Calculator *calc_);
};
class Log10Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Log10Function(Calculator *calc_);
};
class Log2Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Log2Function(Calculator *calc_);
};
class ExpFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	ExpFunction(Calculator *calc_);
};
class Exp10Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Exp10Function(Calculator *calc_);
};
class Exp2Function : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	Exp2Function(Calculator *calc_);
};
class SqrtFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SqrtFunction(Calculator *calc_);
};
class CbrtFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	CbrtFunction(Calculator *calc_);
};
class HypotFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	HypotFunction(Calculator *calc_);
};
class SumFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	SumFunction(Calculator *calc_);
};
class MeanFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MeanFunction(Calculator *calc_);
};
class MedianFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MedianFunction(Calculator *calc_);
};
class MinFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MinFunction(Calculator *calc_);
};
class MaxFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	MaxFunction(Calculator *calc_);
};
class ModeFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	ModeFunction(Calculator *calc_);
};
class NumberFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	NumberFunction(Calculator *calc_);
};
class StdDevFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	StdDevFunction(Calculator *calc_);
};
class StdDevSFunction : public Function {
  protected:
	void calculate2(Manager *mngr);  
  public:
	StdDevSFunction(Calculator *calc_);
};
class RandomFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	RandomFunction(Calculator *calc_);
};
class BASEFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	BASEFunction(Calculator *calc_);
};
class BINFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	BINFunction(Calculator *calc_);
};
class HEXFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	HEXFunction(Calculator *calc_);
};
class OCTFunction : public Function {
  protected:
	Manager *calculate(const string &eq);  
  public:
	OCTFunction(Calculator *calc_);
};

#endif
