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

class EqItem;
class EqNumber;
class EqContainer;

#include "Calculator.h"

class EqItem {
  protected:  
	char operation;  
	Calculator *calc;
	Manager *mngr;
  public:  
	EqItem(char operation_, Calculator *parent);
	virtual ~EqItem(void);
	virtual Manager *calculate(void) = 0; 	  
	char sign(void);
};

class EqNumber : public EqItem {
  private:	
	long double value;	
  public:  
	EqNumber(long double value_, Calculator *parent, char operation_ = PLUS_CH);
	EqNumber(string str, Calculator *parent, char operation_ = PLUS_CH);
	Manager *calculate(void);
};

class EqContainer : public EqItem {
  private:
	vector<EqItem*> items;
  public:
	EqContainer(char operation_ = PLUS_CH, Calculator *parent = NULL);
	~EqContainer(void);
	EqContainer(string str, Calculator *parent, char operation_ = PLUS_CH);
	void add(string *str, char s = PLUS_CH);
	void add(EqItem *e);
	Manager *calculate(void);
};

#endif
