/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef EQITEM_H
#define EQITEM_H

#include "includes.h"

class EqItem {
  protected:  
	MathOperation op;  
	Manager *mngr;
  public:  
	EqItem(MathOperation operation_);
	virtual ~EqItem(void);
	virtual Manager *calculate(void) = 0; 	  
	MathOperation operation(void);
};

class EqNumber : public EqItem {
  public:  
	EqNumber(Manager *value_, MathOperation operation_ = ADD);
	EqNumber(string str, MathOperation operation_ = ADD);
	Manager *calculate(void);
};

class EqContainer : public EqItem {
  private:
	vector<EqItem*> items;
	MathOperation rpn_operation;
  public:
	EqContainer(MathOperation operation_ = ADD);
	~EqContainer(void);
	EqContainer(string str, MathOperation operation_ = ADD);
	void add(string &str, MathOperation s = ADD);
	void add(EqItem *e);
	Manager *calculate(void);
};

#endif
