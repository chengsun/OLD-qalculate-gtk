/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "EqItem.h"

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



EqNumber::EqNumber(long double value_, MathOperation operation_) : EqItem(operation_) {
	mngr = new Manager(value_);
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
	if(operation_ == SUBTRACT || operation_ == ADD) value = 0;
	else value = 1;
	int itmp;
	//	    if(str.substr(0, 3) == "NAN" || str.substr(0, 3) == "INF") {
	//		CALCULATOR->error(true, "Math error", NULL);
	//	    	return;
	//	    }
	if(str.empty() || ((itmp = str.find_first_not_of(" ")) == (int) string::npos)) {
		//		  CALCULATOR->error(true, "Empty expression", NULL);
		mngr->set(value);
		return;
	}
	if(str.substr(0, 3) == SNAN) {
		mngr->set(NAN);
		//			CALCULATOR->error(true, "Math error", NULL);
		str[0] = ZERO_CH;
		str[1] = ZERO_CH;
		str[2] = ZERO_CH;
		return;
	} else if(str.substr(0, 3) == SINF) {
		if(s == MINUS_CH)
			mngr->set(-INFINITY);
		else
			mngr->set(INFINITY);
		str[0] = ZERO_CH;
		str[1] = ZERO_CH;
		str[2] = ZERO_CH;
	} else {
		if(s == MINUS_CH) {
			str.insert(0, 1, MINUS_CH);
		}
		Fraction fr(str);
		mngr->set(&fr);
		return;
	}
	if((itmp = str.find_first_not_of(NUMBERS MINUS DOT, 0)) != (int) string::npos) {
		string stmp = str.substr(itmp, str.length() - itmp);
		str.erase(itmp, str.length() - itmp);

		if(itmp == 0) {
			value = 1;
			CALCULATOR->error(true, _("\"%s\" is not a valid variable/function/unit."), ssave.c_str(), NULL);
			mngr->set(value);
			return;
		} else {
			CALCULATOR->error(true, _("Trailing characters in expression \"%s\" was ignored (unknown variable/function/unit)."), ssave.c_str(), NULL);
		}
	}
	//if(s == '-') str.insert(0, "-");
	//if(s == '-') value = -value;
	mngr->set(value);		
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
	int i = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0;
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
				str.insert(i, 1, MULTIPLICATION_2_CH);
				i++;
				i2++;
			}
			if(i2 < (int) str.length() - 1 && is_in(NUMBERS DOT ID_WRAPS, str[i2 + 1])) {
				str.insert(i2 + 1, 1, MULTIPLICATION_2_CH);
			}
			str2 = str.substr(i + 1, i2 - (i + 1));
			eq_c = new EqContainer(str2, ADD);
			Manager *mngr2 = eq_c->calculate();
			str2 = ID_WRAP_LEFT_CH;
			str2 += i2s(CALCULATOR->addId(mngr2));
			str2 += ID_WRAP_RIGHT_CH;
			delete eq_c;
			str.replace(i, i2 - i + 1, str2);
			i = str.find(LEFT_BRACKET_CH);
			i2 = str.find(RIGHT_BRACKET_CH);
			if(!b && i2 == string::npos) {
				str.append(1, RIGHT_BRACKET_CH);
				i2 = str.length() - 1;
			}			
		}
	}
	i = 0;
	i3 = 0;
	i4 = 0;
	i5 = 0;
	if(i4 > 0) {
		if(!str.empty())
			add(str);
		return;
	} else if((i = str.find_first_of(PLUS MINUS, 1)) > 0 && i != (int) string::npos) {
		bool b = false;
		while(i > -1) {
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
	if((i = str.find_first_of(MULTIPLICATION DIVISION, 1)) > 0 && i != (int) string::npos) {
		while(i > -1) {
			if(str[i] == MULTIPLICATION_CH) s = MULTIPLY;
			else s = DIVIDE;			
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find_first_of(MULTIPLICATION DIVISION, 1);
		}
		add(str);
	} else if((i = str.find(MULTIPLICATION_2_CH, 1)) > 0 && i != (int) string::npos) {
		while(i > -1) {
			s = MULTIPLY;
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find(MULTIPLICATION_2_CH, 1);
		}
		add(str);		
	} else if((i = str.find(POWER_CH, 1)) > 0 && i != (int) string::npos) {
		while(i > -1) {
			s = RAISE;
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find(POWER_CH, 1);
		}
		add(str);
	} else if((i = str.find(EXP_CH, 1)) > 0 && i != (int) string::npos) {
		while(i > -1) {
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
	long double value = 0, vtmp = 0;
	MathOperation s = ADD;
	for(unsigned int i = 0; i < items.size(); i++) {
		mngr->add(items[i]->calculate(), s);
		s = items[i]->operation();
	}
	return mngr;
}
