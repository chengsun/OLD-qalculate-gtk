/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "EqItem.h"

EqItem::EqItem(char operation_, Calculator *parent) {
	operation = operation_;
	calc = parent;
	mngr = NULL;
}
EqItem::~EqItem() {
	if(mngr) mngr->unref();
}
char EqItem::sign() {
	return operation;
}



EqNumber::EqNumber(long double value_, Calculator *parent, char operation_) : EqItem(operation_, parent) {
	mngr = new Manager(calc, value_);
}
EqNumber::EqNumber(string str, Calculator *parent, char operation_) : EqItem(operation_, parent) {
	string ssave = str;
	char s = PLUS_CH;
	for(int i = 0; i < (int) str.length() - 1; i++) {
		if(is_in(str[i], PLUS_S, SPACE_S, NULL)) {
			str.erase(i, 1);
			i--;
		} else if(is_in(str[i], MINUS_S, NULL)) {
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
		mngr = calc->getId(id);
		if(mngr) {
			if(s == MINUS_CH) mngr->add(-1, MULTIPLICATION_CH);
			mngr->ref();
			calc->delId(id);
			return;
		}
	}
	mngr = new Manager(calc);
	if(operation_ == MINUS_CH || operation_ == PLUS_CH) value = 0;
	else value = 1;
	long double dtmp = 1;
	bool btmp = false;
	int itmp;
	//	    if(str.substr(0, 3) == "NAN" || str.substr(0, 3) == "INF") {
	//		calc->error(true, "Math error", NULL);
	//	    	return;
	//	    }
	if(str.empty() || ((itmp = str.find_first_not_of(" ")) == (int) string::npos)) {
		//		  calc->error(true, "Empty expression", NULL);
		mngr->set(value);
		return;
	}
	if(str.substr(0, 3) == NAN_STR) {
		mngr->set(NAN);
		//			calc->error(true, "Math error", NULL);
		str[0] = ZERO_CH;
		str[1] = ZERO_CH;
		str[2] = ZERO_CH;
		return;
	} else if(str.substr(0, 3) == INF_STR) {
		if(s == MINUS_CH)
			mngr->set(-INFINITY);
		else
			mngr->set(INFINITY);
		str[0] = ZERO_CH;
		str[1] = ZERO_CH;
		str[2] = ZERO_CH;
	} else {
		if(s == MINUS_CH)
			str.insert(0, 1, MINUS_CH);
		value = strtold(str.c_str(), NULL);
	}
	if((itmp = find_first_not_of(str, 0, NUMBERS_S, MINUS_S, DOT_S, NULL)) != (int) string::npos) {
		string stmp = str.substr(itmp, str.length() - itmp);
		str.erase(itmp, str.length() - itmp);

		if(itmp == 0) {
			value = 1;
			calc->error(true, _("\"%s\" is not a valid variable/function/unit."), ssave.c_str(), NULL);
			mngr->set(value);
			return;
		} else {
			calc->error(true, _("Trailing characters in expression \"%s\" was ignored (unknown variable/function/unit)."), ssave.c_str(), NULL);
		}
	}
	//if(s == '-') str.insert(0, "-");
	//if(s == '-') value = -value;
	if(btmp)
		value = value * dtmp;
	mngr->set(value);		
}
Manager *EqNumber::calculate() {
	return mngr;
}



EqContainer::EqContainer(char operation_, Calculator *parent) : EqItem(operation_, parent) {mngr = new Manager(calc);}
EqContainer::~EqContainer() {
	for(int i = 0; i < items.size(); i++) {
		delete items[i];
	}
}
EqContainer::EqContainer(string str, Calculator *parent, char operation_) : EqItem(operation_, parent) {
	mngr = new Manager(calc);
	char buffer[100];
	EqContainer *eq_c;
	long double dtmp;
	int i = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0;
	string str2, str3;
	char s = PLUS_CH;
goto_place1:
	if((i = str.find_first_of(LEFT_BRACKET_S)) != (int) string::npos && (i2 = str.find_first_of(RIGHT_BRACKET_S)) > 0 && i2 != (int) string::npos && i2 > i) {
		while(1) {
			if(i < 0 || i2 <= 0 || i2 < i)
				break;
			i3 = str.find_first_of(LEFT_BRACKET_S, i + 1);
			while(i3 < i2 && i3 > -1) {
				i2 = str.find_first_of(RIGHT_BRACKET_S, i2 + 1);
				if(i2 == (int) string::npos) {
					str.erase(i, 1);
					goto goto_place1;
				}
				i3 = str.find_first_of(LEFT_BRACKET_S, i3 + 1);
			}
			if(i > 0 && is_in(str[i - 1], NUMBERS_S, DOT_S, ID_WRAP_S, NULL)) {
				str.insert(i, MULTIPLICATION_STR);
				i++;
				i2++;
			}
			if(i2 < (int) str.length() - 1 && is_in(str[i2 + 1], NUMBERS_S, DOT_S, ID_WRAP_S, NULL)) {
				str.insert(i2 + 1, MULTIPLICATION_STR);
			}
			str2 = str.substr(i + 1, i2 - (i + 1));
			eq_c = new EqContainer(str2, calc, PLUS_CH);
			Manager *mngr2 = eq_c->calculate();
			str2 = ID_WRAP_LEFT_CH;
			str2 += i2s(calc->addId(mngr2));
			str2 += ID_WRAP_RIGHT_CH;
			delete eq_c;
			str.replace(i, i2 - i + 1, str2);
			i = str.find_first_of(LEFT_BRACKET_S);
			i2 = str.find_first_of(RIGHT_BRACKET_S);
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
	} else if((i = find_first_of(str, 1, PLUS_S, MINUS_S, NULL)) > 0 && i != (int) string::npos) {
		bool b = false;
		while(i > -1) {
			if(is_not_in(str[i - 1], OPERATORS_S, NULL)) {
				s = str[i];
				str2 = str.substr(0, i);
				str = str.substr(i + 1, str.length() - (i + 1));
				add(str2, s);
				i = find_first_of(str, 1, PLUS_S, MINUS_S, NULL);
				b = true;
			} else {
				i = find_first_of(str, i + 1, PLUS_S, MINUS_S, NULL);
			}
			
		}
		if(b) {
			add(str);
			return;
		}
	}
	if((i = find_first_of(str, 1, MULTIPLICATION_S, DIVISION_S, NULL)) > 0 && i != (int) string::npos) {
		while(i > -1) {
			s = str[i];
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = find_first_of(str, 1, MULTIPLICATION_S, DIVISION_S, NULL);
		}
		add(str);
	} else if((i = str.find_first_of(POWER_S, 1)) > 0 && i != (int) string::npos) {
		while(i > -1) {
			s = str[i];
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find_first_of(POWER_S, 1);
		}
		add(str);
	} else if((i = str.find_first_of(EXP_S, 1)) > 0 && i != (int) string::npos) {
		while(i > -1) {
			s = str[i];
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find_first_of(EXP_S, 1);
		}
		add(str);
	} else {
		add(new EqNumber(str, calc));
	}
}
void EqContainer::add(string &str, char s) {
	if(str.length() > 0) {
		string stmp = str;
		if(str.find_first_not_of(OPERATORS_S) != string::npos) {
			if(find_first_not_of(str, 1, NUMBERS_S, DOT_S, ID_WRAP_S, NULL) != string::npos && find_first_not_of(str, 0, NUMBERS_S, DOT_S, ID_WRAP_S, PLUS_S, MINUS_S, NULL) != 0) {
				add(new EqContainer(str, calc, s));
			} else {
				add(new EqNumber(str, calc, s));
			}
		}
	}
}
void EqContainer::add(EqItem *e) {
	items.push_back(e);
}
Manager *EqContainer::calculate() {
	long double value = 0, vtmp = 0;
	char s = PLUS_CH;
	for(unsigned int i = 0; i < items.size(); i++) {
		mngr->add(items[i]->calculate(), s);
		s = items[i]->sign();
	}
	return mngr;
}
