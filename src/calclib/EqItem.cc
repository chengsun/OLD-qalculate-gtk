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
		} else if(is_in(OPERATORS, str[i])) {
			CALCULATOR->error(false, _("Misplaced '%s' ignored"), str.substr(0, 1).c_str(), NULL);
			str.erase(i, 1);
			i--;
		}
	}	
	if(str[0] == ID_WRAP_LEFT_CH && str[str.length() - 1] == ID_WRAP_RIGHT_CH) {
		int id = s2i(str.substr(1, str.length() - 2));
		mngr = CALCULATOR->getId(id);
		if(mngr) {	
			if(s == MINUS_CH) mngr->addInteger(-1, OPERATION_MULTIPLY);
			mngr->ref();
			CALCULATOR->delId(id);
			return;
		}
	}
	mngr = new Manager();
	int itmp;
	if(str.empty() || ((itmp = str.find_first_not_of(" ")) == (int) string::npos)) {
//		CALCULATOR->error(true, _("Empty expression"), NULL);
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
		str.insert(str.begin(), 1, MINUS_CH);
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
	for(unsigned int i = 0; i < items.size(); i++) {
		delete items[i];
	}
}
EqContainer::EqContainer(string str, MathOperation operation_) : EqItem(operation_) {
	mngr = new Manager();
	int i = 0, i2 = 0, i3 = 0;
	string str2, str3;
	MathOperation s = OPERATION_ADD;
	while(true) {
		//find first right parenthesis and then the last left parenthesis before
		i2 = str.find(RIGHT_PARENTHESIS_CH);
		if(i2 == (int) string::npos) {
			i = str.rfind(LEFT_PARENTHESIS_CH);	
			if(i == (int) string::npos) {
				//if no parenthesis break
				break;
			} else {
				//right parenthesis missing -- append
				str += RIGHT_PARENTHESIS_CH;
				i2 = str.length() - 1;
			}
		} else {
			if(i2 > 0) {
				i = str.rfind(LEFT_PARENTHESIS_CH, i2 - 1);
			} else {
				i = (int) string::npos;
			}
			if(i == (int) string::npos) {
				//left parenthesis missing -- prepend
				str.insert(str.begin(), 1, LEFT_PARENTHESIS_CH);
				i = 0;
				i2++;
			}
		}
		while(true) {
			//remove unnecessary double parenthesis and the found parenthesis
			if(i > 0 && i2 < (int) str.length() - 1 && str[i - 1] == LEFT_PARENTHESIS_CH && str[i2 + 1] == RIGHT_PARENTHESIS_CH) {
				str.erase(str.begin() + (i - 1));
				i--; i2--;
				str.erase(str.begin() + (i2 + 1));
			} else {
				break;
			}
		}
		if(i > 0 && is_in(NUMBERS DOT ID_WRAPS, str[i - 1])) {
			if(CALCULATOR->inRPNMode()) {
				str.insert(i2 + 1, MULTIPLICATION);	
				str.insert(i, SPACE);
				i++;
				i2++;					
			} else {		
				str.insert(i, MULTIPLICATION_2);
				i++;
				i2++;
			}
		}
		if(i2 < (int) str.length() - 1 && is_in(NUMBERS DOT ID_WRAPS, str[i2 + 1])) {
			if(CALCULATOR->inRPNMode()) {
				i3 = str.find(SPACE, i2 + 1);
				if(i3 == (int) string::npos) {
					str += MULTIPLICATION;
				} else {
					str.replace(i3, 1, MULTIPLICATION);
				}
				str.insert(i2 + 1, SPACE);
			} else {
				str.insert(i2 + 1, MULTIPLICATION_2);
			}
		}
		if(CALCULATOR->inRPNMode() && i > 0 && i2 + 1 == (int) str.length() && is_in(NUMBERS DOT OPERATORS ID_WRAPS, str[i - 1])) {
			str += MULTIPLICATION_CH;	
		}
		str2 = str.substr(i + 1, i2 - (i + 1));
		EqContainer eq_c(str2, OPERATION_ADD);
		Manager *mngr2 = eq_c.calculate();
		str2 = ID_WRAP_LEFT_CH;
		str2 += i2s(CALCULATOR->addId(mngr2));
		str2 += ID_WRAP_RIGHT_CH;
		str.replace(i, i2 - i + 1, str2);
	}
	if((i = str.find(AND, 1)) != (int) string::npos) {
		while(i != (int) string::npos) {
			s = OPERATION_AND;
			str2 = str.substr(0, i);
			EqContainer eq_c(str2, OPERATION_ADD);
			Manager *mngr2 = eq_c.calculate();
			if(mngr2->isFraction() && !mngr2->fraction()->getBoolean()) {
				mngr->clear();
				for(unsigned int ii = 0; ii < items.size(); ii++) {
					delete items[ii];
				}
				items.clear();
				return;
			}
			while((int) str.length() > i + 1 && str[i + 1] == AND_CH) {
				i++;
			}
			str = str.substr(i + 1, str.length() - (i + 1));
			add(new EqNumber(mngr2, s));
			i = str.find(AND, 1);
		}
		add(str);
		return;
	}
	if((i = str.find(OR, 1)) != (int) string::npos) {
		while(i != (int) string::npos) {
			s = OPERATION_OR;
			str2 = str.substr(0, i);
			EqContainer eq_c(str2, OPERATION_ADD);
			Manager *mngr2 = eq_c.calculate();
			if(mngr2->isFraction() && mngr2->fraction()->getBoolean()) {
				mngr->set(1, 1);
				for(unsigned int ii = 0; ii < items.size(); ii++) {
					delete items[ii];
				}
				items.clear();
				return;
			}
			while((int) str.length() > i + 1 && str[i + 1] == OR_CH) {
				i++;
			}
			str = str.substr(i + 1, str.length() - (i + 1));
			add(new EqNumber(mngr2, s));
			i = str.find(OR, 1);
		}
		add(str);
		return;
	}	
	if(str[0] == NOT_CH) {
		str.erase(str.begin());
		EqContainer eq_c(str, OPERATION_ADD);
		Manager *mngr2 = eq_c.calculate();
		mngr->set(mngr2);
		mngr->setNOT();
		return;
	}
	if((i = str.find_first_of(LESS GREATER EQUALS NOT, 0)) != (int) string::npos) {
		bool c = false;
		while(i != (int) string::npos && str[i] == NOT_CH && (int) str.length() > i + 1 && str[i + 1] == NOT_CH) {
			i++;
			if(i + 1 == (int) str.length()) {
				c = true;
			}
		}
		Manager *mngr1 = NULL;
		while(!c) {
			if(i == (int) string::npos) {
				str2 = str.substr(0, str.length());
			} else {
				str2 = str.substr(0, i);
			}
			EqContainer eq_c(str2, OPERATION_ADD);
			Manager *mngr2 = eq_c.calculate();			
			if(mngr1) {
				switch(i3) {
					case EQUALS_CH: {s = OPERATION_EQUALS; break;}
					case GREATER_CH: {s = OPERATION_GREATER; break;}
					case LESS_CH: {s = OPERATION_LESS; break;}
					case GREATER_CH * EQUALS_CH: {s = OPERATION_EQUALS_GREATER; break;}
					case LESS_CH * EQUALS_CH: {s = OPERATION_EQUALS_LESS; break;}
					case GREATER_CH * LESS_CH: {s = OPERATION_NOT_EQUALS; break;}
				}
				mngr1->add(mngr2, s);
				if(mngr1->isZero()) {
					mngr->clear();
					mngr1->unref();
					for(unsigned int ii = 0; ii < items.size(); ii++) {
						delete items[ii];
					}
					items.clear();
					return;
				}
				add(new EqNumber(mngr1, OPERATION_AND));
				mngr1->unref();
			}
			if(i == (int) string::npos) {
				return;
			}
			mngr1 = mngr2;
			mngr1->ref();
			if((int) str.length() > i + 1 && is_in(LESS GREATER NOT EQUALS, str[i + 1])) {
				if(str[i] == str[i + 1]) {
					i3 = str[i];
				} else {
					i3 = str[i] * str[i + 1];
					if(i3 == NOT_CH * EQUALS_CH) {
						i3 = GREATER_CH * LESS_CH;
					} else if(i3 == NOT_CH * LESS_CH) {
						i3 = GREATER_CH;
					} else if(i3 == NOT_CH * GREATER_CH) {
						i3 = LESS_CH;
					}
				}
				i++;
			} else {
				i3 = str[i];
			}
			str = str.substr(i + 1, str.length() - (i + 1));
			i = str.find_first_of(LESS GREATER NOT EQUALS, 0);
			while(i != (int) string::npos && str[i] == NOT_CH && (int) str.length() > i + 1 && str[i + 1] == NOT_CH) {
				i++;
				if(i + 1 == (int) str.length()) {
					i = (int) string::npos;
				}
			}
		}
	}		
	i = 0;
	i3 = 0;	
	if(CALCULATOR->inRPNMode()) {
		while(true) {
			i = str.find_first_of(OPERATORS SPACE, i3 + 1);
			if(i == (int) string::npos) {
				if(i3 != 0) {
					str2 = str.substr(i3 + 1, str.length() - i3 - 1);
				} else {
					str2 = str.substr(i3, str.length() - i3);
				}			
				if(str2.length() > 0) {
					CALCULATOR->setRPNMode(false);				
					add(str2, OPERATION_ADD);
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
			add(str2, OPERATION_ADD);
			CALCULATOR->setRPNMode(true);			
			if(str[i] != SPACE_CH) {
				switch(str[i]) {
					case PLUS_CH: {s = OPERATION_ADD; break;}
					case MINUS_CH: {s = OPERATION_SUBTRACT; break;}
					case MULTIPLICATION_CH: {s = OPERATION_MULTIPLY; break;}
					case DIVISION_CH: {s = OPERATION_DIVIDE; break;}
					case POWER_CH: {s = OPERATION_RAISE; break;}
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
	if((i = str.find_first_of(PLUS MINUS, 1)) != (int) string::npos) {
		bool b = false;
		while(i != (int) string::npos) {
			if(is_not_in(OPERATORS EXP, str[i - 1])) {
				if(str[i] == PLUS_CH) s = OPERATION_ADD;
				else s = OPERATION_SUBTRACT;
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
	if((i = str.find_first_of(MULTIPLICATION DIVISION, 1)) != (int) string::npos) {
		while(i != (int) string::npos) {
			if(str[i] == MULTIPLICATION_CH) s = OPERATION_MULTIPLY;
			else s = OPERATION_DIVIDE;			
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find_first_of(MULTIPLICATION DIVISION, 1);
		}
		add(str);
	} else if((i = str.find(MULTIPLICATION_2_CH, 1)) != (int) string::npos) {
		while(i != (int) string::npos) {
			s = OPERATION_MULTIPLY;
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find(MULTIPLICATION_2_CH, 1);
		}
		add(str);		
	} else if((i = str.find(POWER_CH, 1)) != (int) string::npos) {
		while(i != (int) string::npos) {
			s = OPERATION_RAISE;
			str2 = str.substr(0, i);
			str = str.substr(i + 1, str.length() - (i + 1));
			add(str2, s);
			i = str.find(POWER_CH, 1);
		}
		add(str);
	} else if((i = str.find(EXP_CH, 1)) != (int) string::npos) {
		while(i != (int) string::npos) {
			s = OPERATION_EXP10;
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
		if(str.find_first_not_of(OPERATORS EXP) != string::npos) {
			if((str.find_first_not_of(NUMBERS DOT ID_WRAPS, 1) != string::npos && str.find_first_not_of(NUMBERS DOT ID_WRAPS PLUS MINUS, 0) != 0) || (str.length() > 0 && str[0] == NOT_CH)) {
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
	MathOperation s = OPERATION_ADD;
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
