/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "EqItem.h"
#include "Calculator.h"
#include "Manager.h"
#include "util.h"
#include "Fraction.h"

EqItem::EqItem(MathOperation operation_) {
	op = operation_;
	mngr = NULL;
}
EqItem::~EqItem() {
	if(mngr) mngr->unref();
}
MathOperation EqItem::operation() {
	return op;
}



EqNumber::EqNumber(Manager *value_, MathOperation operation_) : EqItem(operation_) {
	mngr = value_;
	mngr->ref();
}
EqNumber::EqNumber(string str, MathOperation operation_) : EqItem(operation_) {
	string ssave = str;
	char s = PLUS_CH;
	gsub(RIGHT_BRACKET, "", str);
	for(int i = 0; i < (int) str.length() - 1; i++) {
		if(str[i] == PLUS_CH || str[i] == SPACE_CH) {
			str.erase(i, 1);
			i--;
		} else if(str[i] == MINUS_CH) {
			if(s == MINUS_CH)
				s = PLUS_CH;
			else
				s = MINUS_CH;
			str.erase(i, 1);
			i--;
		} else {
			break;
		}
	}	
	if(str[0] == ID_WRAP_LEFT_CH && str[str.length() - 1] == ID_WRAP_RIGHT_CH) {
		int id = s2i(str.substr(1, str.length() - 2));
		mngr = CALCULATOR->getId(id);
		if(mngr) {	
			if(s == MINUS_CH) mngr->addInteger(-1, MULTIPLY);
			mngr->ref();
			CALCULATOR->delId(id);
			return;
		}
	}
	mngr = new Manager();
	int itmp;
	if(str.empty() || ((itmp = str.find_first_not_of(" ")) == (int) string::npos)) {
		CALCULATOR->error(true, "Empty expression", NULL);
		return;
	}
	if((itmp = str.find_first_not_of(NUMBERS MINUS DOT, 0)) != (int) string::npos) {
		string stmp = str.substr(itmp, str.length() - itmp);
		str.erase(itmp, str.length() - itmp);

		if(itmp == 0) {
			CALCULATOR->error(true, _("\"%s\" is not a valid variable/function/unit."), ssave.c_str(), NULL);
			mngr->set(1, 1);
			return;
		} else {
			CALCULATOR->error(true, _("Trailing characters in expression \"%s\" was ignored."), ssave.c_str(), NULL);
		}
	}
	if(s == MINUS_CH) {
		str.insert(0, 1, MINUS_CH);
	}
	Fraction fr(str);
	mngr->set(&fr);
	return;
	
}
Manager *EqNumber::calculate() {
	return mngr;
}



EqContainer::EqContainer(MathOperation operation_) : EqItem(operation_) {mngr = new Manager();}
EqContainer::~EqContainer() {
	for(int i = 0; i < items.size(); i++) {
		delete items[i];
	}
}
EqContainer::EqContainer(string str, MathOperation operation_) : EqItem(operation_) {
	mngr = new Manager();
	char buffer[100];
	EqContainer *eq_c;
	long double dtmp;
	int i = 0, i2 = 0, i3 = 0;
	string str2, str3;
	MathOperation s = ADD;
goto_place1:
	if((i = str.find(LEFT_BRACKET_CH)) != string::npos) {
		i2 = str.find(RIGHT_BRACKET_CH);
		if(i2 == string::npos) {
			str.append(1, RIGHT_BRACKET_CH);
			i2 = str.length() - 1;
		}
		while(1) {
			bool b = false;
			if(i == string::npos || i > i2) break;
			i3 = str.find(LEFT_BRACKET_CH, i + 1);
			while(i3 < i2 && i3 > -1) {
				i2 = str.find(RIGHT_BRACKET_CH, i2 + 1);
				if(i2 == string::npos) {
					str.append(1, RIGHT_BRACKET_CH);				
					i2 = str.length() - 1;				
					b = true;
				}
				i3 = str.find(LEFT_BRACKET_CH, i3 + 1);
			}
			if(i > 0 && is_in(NUMBERS DOT ID_WRAPS, str[i - 1])) {
				if(CALCULATOR->inRPNMode()) {
					str.insert(i2 + 1, 1, MULTIPLICATION_CH);	
					str.insert(i, 1, SPACE_CH);
					i++;
					i2++;					
				} else {		
					str.insert(i, 1, MULTIPLICATION_2_CH);
					i++;
					i2++;
				}
			}
			if(i2 < str.length() - 1 && is_in(NUMBERS DOT ID_WRAPS, str[i2 + 1])) {
				if(CALCULATOR->inRPNMode()) {
					i3 = str.find(SPACE, i2 + 1);
					if(i3 == string::npos) {
						str += MULTIPLICATION;
					} else {
						str.replace(i3, 1, MULTIPLICATION);
					}
					str.insert(i2 + 1, 1, SPACE_CH);
				} else {
					str.insert(i2 + 1, 1, MULTIPLICATION_2_CH);
				}
			}
			if(CALCULATOR->inRPNMode() && i > 0 && i2 + 1 == str.length() && is_in(NUMBERS DOT OPERATORS ID_WRAPS, str[i - 1])) {
				str += MULTIPLICATION_CH;	
			}
			str2 = str.substr(i + 1, i2 - (i + 1));
			EqContainer eq_c(str2, ADD);
			Manager *mngr2 = eq_c.calculate();
			str2 = ID_WRAP_LEFT_CH;
			str2 += i2s(CALCULATOR->addId(mngr2));
			str2 += ID_WRAP_RIGHT_CH;
			str.replace(i, i2 - i + 1, str2);
			i = str.find(LEFT_BRACKET_CH);
			i2 = str.find(RIGHT_BRACKET_CH);
			if(!b && i2 == string::npos) {
				str.append(1, RIGHT_BRACKET_CH);
				i2 = str.length() - 1;
			}			
		}
	}
	gsub(RIGHT_BRACKET, "", str);
	i = 0;
	i3 = 0;
	if(CALCULATOR->inRPNMode()) {
		while(true) {
			i = str.find_first_of(OPERATORS SPACE, i3 + 1);
			if(i == string::npos) {
				if(i3 != 0) {
					str2 = str.substr(i3 + 1, str.length() - i3 - 1);
				} else {
					str2 = str.substr(i3, str.length() - i3);
				}			
				if(str2.length() > 0) {
					CALCULATOR->setRPNMode(false);				
					add(str2, ADD);
					CALCULATOR->setRPNMode(true);								
				}
				if(items.size() > 1) {
					CALCULATOR->error(true, _("RPN syntax error."), NULL);
					while(items.size() > 1) {
						delete items[0];
						items.erase(items.begin());
					}
				}
				return;
			}
			if(i3 != 0) {
				str2 = str.substr(i3 + 1, i - i3 - 1);
			} else {
				str2 = str.substr(i3, i - i3);
			}
/*			i2 = str2.find(EXP_CH);
			if(i2 > 0 && i2 < str.length() - 1) {
			
			}*/
			CALCULATOR->setRPNMode(false);
			add(str2, ADD);
			CALCULATOR->setRPNMode(true);			
			if(str[i] != SPACE_CH) {
				switch(str[i]) {
					case PLUS_CH: {s = ADD; break;}
					case MINUS_CH: {s = SUBTRACT; break;}
					case MULTIPLICATION_CH: {s = MULTIPLY; break;}
					case DIVISION_CH: {s = DIVIDE; break;}
					case POWER_CH: {s = RAISE; break;}
				}
				if(items.size() < 2) {
					CALCULATOR->error(true, _("RPN syntax error."), NULL);
				} else {
					Manager *mngr2 = new Manager(items[items.size() - 2]->calculate());
					delete items[items.size() - 2];
					items.erase(items.begin() + (items.size() - 2));
					mngr2->add(items[items.size() - 1]->calculate(), s);
					delete items[items.size() - 1];
					items.erase(items.begin() + (items.size() - 1));				
					items.push_back(new EqNumber(mngr2));
					mngr2->unref();
				}
			}
			i3 = i;
		}
	}
	if((i = str.find_first_of(PLUS MINUS, 1)) != string::npos) {
		bool b = false;
		while(i != string::npos) {
			if(is_not_in(OPERATORS, str[i - 1])) {
				if(str[i] == PLUS_CH) s = ADD;
				else s = SUBTRACT;
				str2 = str.substr(0, i);
				str = str.substr(i + 1, str.length() - (i + 1));
				add(str2, s);
				i = str.find_first_of(PLUS MINUS, 1);
				b = true;
			} else {
				i = str.find_first_of(PLUS MINUS, i + 1);
			}
			
		}
		if(b) {
			add(str);
			return;
		}
	}
	if((i = str.find_first_of(MULTIPLICATION DIVISION, 1)) != string::npos) {
		while(i != string::npos) {
			if(str[i] == MULTIPLICATION_CH) s = MULTIPLY;
			else s = DIVIDE;			
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find_first_of(MULTIPLICATION DIVISION, 1);
		}
		add(str);
	} else if((i = str.find(MULTIPLICATION_2_CH, 1)) != string::npos) {
		while(i != string::npos) {
			s = MULTIPLY;
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find(MULTIPLICATION_2_CH, 1);
		}
		add(str);		
	} else if((i = str.find(POWER_CH, 1)) != string::npos) {
		while(i != string::npos) {
			s = RAISE;
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find(POWER_CH, 1);
		}
		add(str);
	} else if((i = str.find(EXP_CH, 1)) != string::npos) {
		while(i != string::npos) {
			s = EXP10;
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find(EXP_CH, 1);
		}
		add(str);
	} else {
		add(new EqNumber(str));
	}
}
void EqContainer::add(string &str, MathOperation s) {
	if(str.length() > 0) {
		string stmp = str;
		if(str.find_first_not_of(OPERATORS) != string::npos) {
			if(str.find_first_not_of(NUMBERS DOT ID_WRAPS, 1) != string::npos && str.find_first_not_of(NUMBERS DOT ID_WRAPS PLUS MINUS, 0) != 0) {
				add(new EqContainer(str, s));
			} else {
				add(new EqNumber(str, s));
			}
		}
	}
}
void EqContainer::add(EqItem *e) {
	items.push_back(e);
}
Manager *EqContainer::calculate() {
	MathOperation s = ADD;
	for(unsigned int i = 0; i < items.size(); i++) {
		if(i == 0) {
			mngr->set(items[i]->calculate());
		} else {
			mngr->add(items[i]->calculate(), s);
		}
		s = items[i]->operation();
	}
	return mngr;
}
