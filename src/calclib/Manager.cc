/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/


#include "Manager.h"
#include "Calculator.h"
#include "Unit.h"
#include "Function.h"
#include "Fraction.h"
#include "Integer.h"
#include "Matrix.h"
#include "Prefix.h"
#include "util.h"

void Manager::setType(int mngr_type) {
	c_type = mngr_type;
}
void Manager::setPrefix(Prefix *p) {
	o_prefix = p;
}
Prefix *Manager::prefix() const {
	return o_prefix;
}
bool Manager::clearPrefixes() {
	bool b = false;
	for(unsigned int i = 0; i < mngrs.size(); i++) {
		if(mngrs[i]->clearPrefixes()) b = true;
	}
	if(b) {
		clean();
	}
	if(o_prefix) {
		Fraction fr;
		o_prefix->value(1, &fr);
		o_prefix = NULL;
		Manager mngr(&fr);
		add(&mngr, OPERATION_MULTIPLY);
		b = true;
	}
	return b;
}
void Manager::init() {
	refcount = 1;
	b_exact = true;
	b_protect = false;
	fr = new Fraction();
	mtrx = NULL;
	o_prefix = NULL;
}
Manager::Manager() {
	init();
	clear();
}
Manager::Manager(long double value_) {
	init();
	set(value_);
}
Manager::Manager(long int numerator_, long int denominator_, long int fraction_exp_) {
	init();
	set(numerator_, denominator_, fraction_exp_);
}
Manager::Manager(string var_) {
	init();
	set(var_);
}
Manager::Manager(const Function *f, ...) {
	init();
	Manager *mngr;
	va_list ap;
	va_start(ap, f); 
	while(true) {
		mngr = va_arg(ap, Manager*);
		if(mngr == NULL) break;
		push_back(new Manager(mngr));
	}
	va_end(ap);	
	o_function = (Function*) f;
	c_type = FUNCTION_MANAGER;
}
Manager::Manager(const Unit *u, long int value_) {
	init();
	set(u, value_);
}
Manager::Manager(const Manager *mngr) {
	init();
	set(mngr);
}
Manager::Manager(const Fraction *fraction_) {
	init();
	set(fraction_);
}
Manager::Manager(const Matrix *matrix_) {
	init();
	set(matrix_);
}
Manager::Manager(const Vector *vector_) {
	init();
	set(vector_);
}
Manager::~Manager() {
	clear();
	delete fr;
}
void Manager::setNull() {
	clear();
}
void Manager::set(const Manager *mngr) {
	clear();
	if(mngr != NULL) {
		o_unit = mngr->unit();
		s_var = mngr->text();	
		o_function = mngr->function();
		o_prefix = mngr->prefix();
		comparison_type = mngr->comparisonType();
		if(mngr->matrix()) {
			if(mngr->matrix()->isVector()) {
				mtrx = new Vector(mngr->matrix());
			} else {
				mtrx = new Matrix(mngr->matrix());
			}
		}
		fr->set(mngr->fraction());
		for(unsigned int i = 0; i < mngr->countChilds(); i++) {
			mngrs.push_back(new Manager(mngr->getChild(i)));
		}	
		setPrecise(mngr->isPrecise());
		c_type = mngr->type();
	}
}
void Manager::set(const Fraction *fraction_) {
	clear();
	if(fraction_) {
		fr->set(fraction_);
		setPrecise(fr->isPrecise());
		c_type = FRACTION_MANAGER;
	}
}
void Manager::set(const Matrix *matrix_) {
	clear();
	if(!matrix_) return;
	if(matrix_->isVector()) {
		mtrx = new Vector(matrix_);	
	} else {
		mtrx = new Matrix(matrix_);	
	}
	setPrecise(mtrx->isPrecise());
	c_type = MATRIX_MANAGER;
}
void Manager::set(const Vector *vector_) {
	clear();
	if(!vector_) return;
	mtrx = new Vector(vector_);	
	setPrecise(mtrx->isPrecise());
	c_type = MATRIX_MANAGER;
}
void Manager::set(long double value_) {
	clear();
	fr->setFloat(value_);
	setPrecise(fr->isPrecise());	
	c_type = FRACTION_MANAGER;
}
void Manager::set(long int numerator_, long int denominator_, long int fraction_exp_) {
	clear();
	fr->set(numerator_, denominator_, fraction_exp_);
	setPrecise(fr->isPrecise());	
	c_type = FRACTION_MANAGER;	
}
void Manager::set(string var_) {
	clear();
//	if(var_.empty()) return;
	s_var = var_;
	c_type = STRING_MANAGER;
}
void Manager::set(const Function *f, ...) {
	clear();
	if(!f) return;
	Manager *mngr;
	va_list ap;
	va_start(ap, f); 
	while(true) {
		mngr = va_arg(ap, Manager*);
		if(mngr == NULL) break;
		push_back(new Manager(mngr));
	}
	va_end(ap);	
	o_function = (Function*) f;
	c_type = FUNCTION_MANAGER;
}
void Manager::addFunctionArg(const Manager *mngr) {
	if(c_type == FUNCTION_MANAGER) {
		push_back(new Manager(mngr));
	}
}
void Manager::push_back(Manager *mngr) {
	if(!mngr->isPrecise()) setPrecise(false);
	mngrs.push_back(mngr);	
}
void Manager::set(const Unit *u, long int exp10) {
	clear();
	if(exp10 == 0) {
		o_unit = (Unit*) u;
		c_type = UNIT_MANAGER;
	} else {
		push_back(new Manager(1, 1, exp10));
		push_back(new Manager(u));
		c_type = MULTIPLICATION_MANAGER;
	}
}
bool Manager::typeclean() {
	bool changed = false;
	switch(c_type) {
		case ALTERNATIVE_MANAGER: {
			int i, i2;
			for(i = 0; i + 1 < (int) mngrs.size(); i++) {	
				for(i2 = i + 1; i2 < (int) mngrs.size(); i2++) {	
					if(mngrs[i]->equals(mngrs[i2])) {
						mngrs[i2]->unref();
						mngrs.erase(mngrs.begin() + i2);				
						i2--;			
						changed = true;
					}	
				}
			}
			if(mngrs.size() == 1) {
				moveto(mngrs[0]);
				changed = true;
			}
			break;
		}
		case ADDITION_MANAGER: {
			int i, i2;
			for(i = 0; i + 1 < (int) mngrs.size(); i++) {
				if(mngrs[i]->isNull()) {
					mngrs[i]->unref();
						mngrs.erase(mngrs.begin() + i);				
					changed = true;
					i--;
				} else {
					for(i2 = i + 1; i2 < (int) mngrs.size(); i2++) {
						if(mngrs[i2]->isNull()) {
							mngrs[i2]->unref();
							mngrs.erase(mngrs.begin() + i2);				
							i2--;
							changed = true;
						} else if(mngrs[i]->add(mngrs[i2], OPERATION_ADD, false)) {
							mngrs[i2]->unref();
							mngrs.erase(mngrs.begin() + i2);
							i = -1;
							changed = true;
							break;
						}
					}
				}
			}
			if(mngrs.size() == 1) {
				moveto(mngrs[0]);
				changed = true;
			} else if(mngrs.size() < 1) {
				clear();
				changed = true;
			}
			break;
		}
		case MULTIPLICATION_MANAGER: {
			for(int i = 0; i + 1 < (int) mngrs.size(); i++) {
				for(int i2 = i + 1; i2 < (int) mngrs.size(); i2++) {
					if(mngrs[i]->add(mngrs[i2], OPERATION_MULTIPLY, false)) {
						mngrs[i2]->unref();
						mngrs.erase(mngrs.begin() + i2);
						changed = true;
						i = -1;
						break;
					}
				}
			}
			if(mngrs.size() == 1) {
				moveto(mngrs[0]);
				changed = true;
			} else if(mngrs.size() < 1) {	
				clear();
				changed = true;
			}
			break;	
		}
		case POWER_MANAGER: {
			if(mngrs[0]->add(mngrs[1], OPERATION_RAISE, false)) {
				mngrs[1]->unref();
				mngrs.erase(mngrs.begin() + 1);
				changed = true;
			}
			if(mngrs.size() == 1) {
				moveto(mngrs[0]);
				changed = true;
			} else if(mngrs.size() < 1) {
				clear();
				changed = true;
			}
			break;	
		}
		case AND_MANAGER: {}
		case OR_MANAGER: {
			MathOperation op;
			if(c_type == OR_MANAGER) op = OPERATION_OR;
			else op = OPERATION_AND;
			for(int i = 0; i + 1 < (int) mngrs.size(); i++) {
				for(int i2 = i + 1; i2 < (int) mngrs.size(); i2++) {
					if(mngrs[i]->add(mngrs[i2], op, false)) {
						mngrs[i2]->unref();
						mngrs.erase(mngrs.begin() + i2);
						changed = true;
						i = -1;
						break;
					}
				}
			}
			if(mngrs.size() == 1) {
				moveto(mngrs[0]);
				changed = true;
			} else if(mngrs.size() < 1) {	
				clear();
				changed = true;
			}
			break;	
		}
		case COMPARISON_MANAGER: {
			MathOperation op;
			switch(comparison_type) {
				case COMPARISON_EQUALS: {op = OPERATION_EQUALS; break;}
				case COMPARISON_NOT_EQUALS: {op = OPERATION_NOT_EQUALS; break;}
				case COMPARISON_LESS: {op = OPERATION_LESS; break;}
				case COMPARISON_GREATER: {op = OPERATION_GREATER; break;}
				case COMPARISON_EQUALS_LESS: {op = OPERATION_EQUALS_LESS; break;}
				case COMPARISON_EQUALS_GREATER: {op = OPERATION_EQUALS_GREATER; break;}
			}
			if(mngrs[0]->add(mngrs[1], op, false)) {
				mngrs[1]->unref();
				mngrs.erase(mngrs.begin() + 1);
				changed = true;
			}
			if(mngrs.size() == 1) {
				moveto(mngrs[0]);
				changed = true;
			} else if(mngrs.size() < 1) {
				clear();
				changed = true;
			}
			break;	
		}
		case NOT_MANAGER: {
			if(mngrs.size() && mngrs[0]->isFraction()) {
				mngrs[0]->fraction()->setNOT();
				moveto(mngrs[0]);
				changed = true;
			}
			break;	
		}
	}
	return changed;
}
bool Manager::reverseadd(const Manager *mngr, MathOperation op, bool translate_) {
	Manager *mngr2 = new Manager(mngr);
	if(!mngr2->add(this, op, translate_)) {
		mngr2->unref();
		return false;
	}
	moveto(mngr2);
	mngr2->unref();
	return true;
}
void Manager::transform(const Manager *mngr, char type_, MathOperation op, bool reverse_) {
	Manager *mngr2 = new Manager(this);
	clear();
	Manager *mngr3 = new Manager(mngr);
	if(reverse_ || op == OPERATION_RAISE) {
		push_back(mngr2);
		push_back(mngr3);
	} else {
		push_back(mngr2);
		push_back(mngr3);
	}
	c_type = type_;
}

void Manager::addAlternative(const Manager *mngr) {
	if(c_type != ALTERNATIVE_MANAGER) {
		Manager *mngr2 = new Manager(this);
		clear();
		push_back(mngr2);
		c_type = ALTERNATIVE_MANAGER;
	}
	push_back(new Manager(mngr));
}
bool Manager::add(const Manager *mngr, MathOperation op, bool translate_) {
	if(!mngr) return true;
//	printf("[%s] %c [%s] (%i)\n", print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str(), op2ch(op), mngr->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str(), translate_);
	if(!mngr->isPrecise()) setPrecise(false);
	if(mngr->type() == FRACTION_MANAGER && c_type == FRACTION_MANAGER) {
		Fraction fr_save(fr);
		int solutions = fr->add(op, mngr->fraction());
		if(solutions == 1) {
			if(!fr->isPrecise() || !mngr->isPrecise()) setPrecise(false);
			return true;
		} else if(solutions > 1) {
			if(!fr->isPrecise() || !mngr->isPrecise()) setPrecise(false);
			Manager mngr2;
			for(int i = 2; i <= solutions; i++) {
				mngr2.set(&fr_save);
				mngr2.fraction()->add(op, mngr->fraction(), i);
				if(!mngr2.fraction()->isPrecise()) mngr2.setPrecise(false);
				addAlternative(&mngr2);
			}
			return true;
		}
	}
	if(c_type == MATRIX_MANAGER) {
		if(mtrx->add(op, mngr)) {
			bool b = !mtrx->isPrecise() || !mngr->isPrecise();
			if(mtrx->rows() == 1 && mtrx->columns() == 1) {
				Manager mngr2(mtrx->get(1, 1));
				set(&mngr2);
			}
			if(b) setPrecise(false);
			return true;
		}
	} else if(mngr->type() == MATRIX_MANAGER) {
		return reverseadd(mngr, op, translate_);
	}
	if(c_type == ALTERNATIVE_MANAGER) {
		if(mngr->type() == ALTERNATIVE_MANAGER) {
			for(unsigned int i = 0; i < mngr->countChilds(); i++) {
				push_back(new Manager(mngr->getChild(i)));
			}
		} else {
			int is = mngrs.size();
			for(int i = 0; i < is; i++) {
				mngrs[i]->add(mngr, op);
				if(!mngrs[i]->isPrecise()) setPrecise(false);
				if(mngrs[i]->type() == ALTERNATIVE_MANAGER) {
					for(unsigned int i2 = 0; i2 < mngrs[i]->countChilds(); i2++) {
						mngrs[i]->getChild(i2)->ref();
						push_back(mngrs[i]->getChild(i2));
					}
					mngrs[i]->unref();
					mngrs.erase(mngrs.begin() + i);
					i--;
					is--;
				}
			}
		}
		typeclean();
		return true;
	} else if(mngr->type() == ALTERNATIVE_MANAGER) {
		return reverseadd(mngr, op, translate_);
	}	
	if(op == OPERATION_SUBTRACT) {
		op = OPERATION_ADD;
		Manager mngr2(mngr);
		mngr2.addInteger(-1, OPERATION_MULTIPLY);
		bool b = add(&mngr2, op);
		return b;
	}
	if(op == OPERATION_EXP10) {
		op = OPERATION_MULTIPLY;
		Manager mngr2(10, 1);
		mngr2.add(mngr, OPERATION_RAISE);
		bool b = add(&mngr2, op);
		return b;		
	}
	if(op == OPERATION_DIVIDE) {
		if(mngr->isNull()) {
//		 	CALCULATOR->error(true, _("Trying to divide \"%s\" with zero."), print().c_str(), NULL);
//			if(negative()) set(-INFINITY);
//			else set(INFINITY);
//			mngr->unref();
//			return false;
		} else if(mngr->isOne()) {
			return true;
		}
		op = OPERATION_MULTIPLY;
		Manager mngr2(mngr);
		Manager mngr3(-1, 1);
		mngr2.add(&mngr3, OPERATION_RAISE);
		bool b = add(&mngr2, op);
		return b;		
	}

	if(op == OPERATION_ADD) {
		if(mngr->isNull()) {
			return true;
		}
		if(isNull()) {
			set(mngr);
			return true;
		}		
		switch(c_type) {
			case ADDITION_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {
						bool b = false;
						for(unsigned int i = 0; i < mngr->mngrs.size(); i++) {
							if(add(mngr->mngrs[i], op, translate_)) {
								b = true;
							}
						}
						if(!b) {
							return false;
						}
						break;
					}
					default: {
						Manager *mngr2 = new Manager(mngr);
						push_back(mngr2);
						typeclean();
						break;
					}
				}
				break;
			}
			case MULTIPLICATION_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case MULTIPLICATION_MANAGER: {
						bool b = false;
						if(compatible(mngr)) {
							for(unsigned int i = 0; i < mngrs.size(); i++) {
								if(mngrs[i]->isNumber()) {
									for(unsigned int i2 = 0; i2 < mngr->mngrs.size(); i2++) {
										if(mngr->mngrs[i2]->isNumber()) {
											mngrs[i]->add(mngr->mngrs[i2], op);
											b = true;
											break;
										}
									}
									if(!b) {
										mngrs[i]->addInteger(1, op);
										b = true;
									}
									if(mngrs[i]->isNull()) {
										clear();
									} else if(mngrs[i]->isOne()) {
										mngrs[i]->unref();
										mngrs.erase(mngrs.begin() + i);
										if(mngrs.size() == 1) {
											moveto(mngrs[0]);
										}
									}
									break;
								}
							}
							if(!b) {
								for(unsigned int i2 = 0; i2 < mngr->mngrs.size(); i2++) {
									if(mngr->mngrs[i2]->isNumber()) {
										if(mngr->mngrs[i2]->value() == -1) {
											clear();
										} else {
											push_back(new Manager(1, 1));
											mngrs[mngrs.size() - 1]->add(mngr->mngrs[i2], op);
										}
										b = true;
										break;
									}
								}
							}
							if(!b) {
								push_back(new Manager(2, 1));
							}
							break;							
						}
						if(!translate_) {
							return false;
						}
						transform(mngr, ADDITION_MANAGER, op);
						break;
					}
					case UNIT_MANAGER: {
					}
					case POWER_MANAGER: {
					}
					case FUNCTION_MANAGER: {
					}
					case STRING_MANAGER: {
						if(compatible(mngr)) {
							bool b = false;
							for(unsigned int i = 0; i < mngrs.size(); i++) {
								if(mngrs[i]->isNumber()) {
									Manager *mngr2 = new Manager(1, 1);
									mngrs[i]->add(mngr2, op);
									mngr2->unref();
									if(mngrs[i]->value() == 0) {
										clear();
									} else if(mngrs[i]->isOne()) {
										mngrs[i]->unref();
										mngrs.erase(mngrs.begin());
										if(mngrs.size() == 1) {
											moveto(mngrs[0]);
										}
									}
									b = true;
									break;
								}
							}
							if(!b) {
								push_back(new Manager(2, 1));
							}
							break;
						}					
					}					
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, ADDITION_MANAGER, op);
						break;
					}
				}
				break;
			}
			case POWER_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case POWER_MANAGER: {
						if(equals(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							push_back(new Manager(2, 1));
							push_back(mngr2);
							c_type = MULTIPLICATION_MANAGER;
							break;
						}					
					}
					default: {
						if(!translate_) {			
							return false;
						}
						transform(mngr, ADDITION_MANAGER, op);
						break;
					}
				}
				break;
			}
			case FRACTION_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, ADDITION_MANAGER, op, true);
						break;
					}					
				}
				break;
			}
			case UNIT_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case UNIT_MANAGER: {
						if(equals(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							push_back(new Manager(2, 1));
							push_back(mngr2);
							c_type = MULTIPLICATION_MANAGER;
							break;
						}					
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, ADDITION_MANAGER, op);
						break;
					}
				}
				break;
			}
			case STRING_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case STRING_MANAGER: {
						if(s_var == mngr->s_var) {
							Manager *mngr2 = new Manager(this);
							clear();
							push_back(new Manager(2, 1));
							push_back(mngr2);
							c_type = MULTIPLICATION_MANAGER;
							break;
						}
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, ADDITION_MANAGER, op);
						break;
					}
				}
				break;
			}
			case FUNCTION_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case FUNCTION_MANAGER: {
						if(equals(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							push_back(new Manager(2, 1));
							push_back(mngr2);
							c_type = MULTIPLICATION_MANAGER;
							break;
						}
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, ADDITION_MANAGER, op);
						break;
					}
				}
				break;			
			}
			default: {
				if(!translate_) {
					return false;
				}
				transform(mngr, ADDITION_MANAGER, op);
				break;
			}			
		}
	} else if(op == OPERATION_MULTIPLY) {
		if(isNull() || mngr->isNull()) {
			clear(); 
			return true;
		} else if(mngr->isOne()) {
			return true;
		} else if(isOne()) {
			set(mngr);
			return true;
		}
		switch(c_type) {
			case ADDITION_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {
						Manager *mngr3 = new Manager(this);
						clear();
						for(unsigned int i = 0; i < mngr->mngrs.size(); i++) {
							Manager *mngr2 = new Manager(mngr3);
							mngr2->add(mngr->mngrs[i], op);
							add(mngr2, OPERATION_ADD);
							mngr2->unref();
						}
						mngr3->unref();
						break;
					}
					default: {
						for(unsigned int i = 0; i < mngrs.size(); i++) {
							mngrs[i]->add(mngr, op);
						}
						break;
					}
				}
				typeclean();
				break;
			}
			case MULTIPLICATION_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case MULTIPLICATION_MANAGER: {
						for(unsigned int i = 0; i < mngr->mngrs.size(); i++) {
							add(mngr->mngrs[i], op);
						}
						typeclean();
						break;
					}
					default: {
						Manager *mngr2 = new Manager(mngr);
						push_back(mngr2);
						typeclean();
						break;
					}
				}
				break;
			}
			case POWER_MANAGER: {
				switch(mngr->type()) {
					case MULTIPLICATION_MANAGER: {}
					case ADDITION_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case POWER_MANAGER: {
						if(mngr->mngrs[0]->equals(mngrs[0])) {
							mngrs[1]->add(mngr->mngrs[1], OPERATION_ADD);
							if(mngrs[1]->isNull()) {
								set(1, 1);
							} else if(mngrs[1]->isOne()) {
								moveto(mngrs[0]);
							}
						} else {
							if(!translate_) {
								return false;
							}
							transform(mngr, MULTIPLICATION_MANAGER, op);
						}
						break;
					}
					default: {
						if(mngr->equals(mngrs[0])) {
							mngrs[1]->addInteger(1, OPERATION_ADD);
							if(mngrs[1]->isNull()) {
								set(1, 1);
							} else if(mngrs[1]->isOne()) {
								moveto(mngrs[0]);
							}
						} else {
							if(!translate_) {
								return false;
							}
							transform(mngr, MULTIPLICATION_MANAGER, op);
						}
						break;
					}
				}
				break;
			}
			case FRACTION_MANAGER: {
				if(fr->isOne()) {set(mngr); return true;}
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {}
					case POWER_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, MULTIPLICATION_MANAGER, op, true);
						break;
					}
				}
				break;
			}
			case UNIT_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {}
					case POWER_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case UNIT_MANAGER: {
						if(o_unit == mngr->o_unit) {
							Manager *mngr2 = new Manager(this);
							clear();
							push_back(mngr2);
							push_back(new Manager(2, 1));
							c_type = POWER_MANAGER;
							break;
						}
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, MULTIPLICATION_MANAGER, op);
						break;
					}
				}
				break;
			}
			case STRING_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {}
					case POWER_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case STRING_MANAGER: {
						if(s_var == mngr->s_var) {
							Manager *mngr2 = new Manager(this);
							clear();
							push_back(mngr2);
							push_back(new Manager(2, 1));
							c_type = POWER_MANAGER;
							break;
						}
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, MULTIPLICATION_MANAGER, op);
						break;
					}
				}
				break;
			}
			case FUNCTION_MANAGER: {
				switch(mngr->type()) {
					case ADDITION_MANAGER: {}
					case MULTIPLICATION_MANAGER: {}
					case POWER_MANAGER: {
						if(!reverseadd(mngr, op, translate_)) {
							return false;
						}
						break;
					}
					case FUNCTION_MANAGER: {
						if(equals(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							push_back(mngr2);
							push_back(new Manager(2, 1));
							c_type = POWER_MANAGER;
							break;
						}
					}
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, MULTIPLICATION_MANAGER, op);
						break;
					}
				}
				break;
			}			
			default: {
				if(!translate_) {
					return false;
				}
				transform(mngr, MULTIPLICATION_MANAGER, op);
				break;
			}			
		}
	} else if(op == OPERATION_RAISE) {
		if(mngr->isNull()) {
			set(1, 1);
			return true;
		} else if(mngr->isOne()) {
			return true;
		} else if(isZero() && mngr->isFraction() && mngr->fraction()->isNegative()) {
			if(translate_) {CALCULATOR->error(true, _("Division by zero."), NULL);}
		}
		switch(c_type) {
			case MULTIPLICATION_MANAGER: {
				for(unsigned int i = 0; i < mngrs.size(); i++) {
					mngrs[i]->add(mngr, op);
				}
				break;
			}		
			case ADDITION_MANAGER: {
				switch(mngr->type()) {
					case FRACTION_MANAGER: {
						if(mngr->fraction()->isInteger() && !mngr->fraction()->isMinusOne()) {
							Integer *n = mngr->fraction()->getInteger();
							n->setNegative(false);
							Manager *mngr2 = new Manager(this);
							n->add(-1);
							for(; n->isPositive(); n->add(-1)) {
								add(mngr2, OPERATION_MULTIPLY);
							}
							if(mngr->fraction()->isNegative()) {
								mngr2->unref();
								mngr2 = new Manager(1, 1);
								mngr2->add(this, OPERATION_DIVIDE);
								moveto(mngr2);
								mngr2->unref();
							}
							delete n;
							break;
						}
						if(!translate_) {
							return false;
						}	
						transform(mngr, POWER_MANAGER, op);
						break;						
					}
					default: {
						if(!translate_) {
							return false;
						}	
						transform(mngr, POWER_MANAGER, op);
						break;
					}
				}
				break;
			}
			case POWER_MANAGER: {
				switch(mngr->type()) {
					default: {
						mngrs[1]->add(mngr, OPERATION_MULTIPLY);
						if(mngrs[1]->isNull()) {
							set(1, 1);
						} else if(mngrs[1]->isNumber() && mngrs[0]->isNumber()) {
							mngrs[0]->add(mngrs[1], OPERATION_RAISE);
							moveto(mngrs[0]);
						} else if(mngrs[1]->isOne()) {
							moveto(mngrs[0]);
						}
						break;
					}
				}
				break;
			}
			case FRACTION_MANAGER: {
				if(isNull()) {
					if(mngr->isNumber() && mngr->value() < 0) {
						if(!translate_) {
							return false;
						}
						if(mngr->value() == -1) {
							transform(mngr, POWER_MANAGER, op);
						} else {
							Manager *mngr2 = new Manager(-1, 1);
							transform(mngr2, POWER_MANAGER, op);
							mngr2->unref();
						}
					}
					break;
				}
				if(fr->isOne()) {
					return true;
				}
				switch(mngr->type()) {
					default: {
						if(!translate_) {
							return false;
						}
						transform(mngr, POWER_MANAGER, op);
						break;
					}
				}
				break;
			}						
			default: {
				if(!translate_) {
					return false;
				}
				transform(mngr, POWER_MANAGER, op);
				break;
			}
		}
	} else if(op == OPERATION_EQUALS || op == OPERATION_LESS || op == OPERATION_GREATER || op == OPERATION_NOT_EQUALS || op == OPERATION_EQUALS_LESS || op == OPERATION_EQUALS_GREATER) {
		int s = compare(mngr);
		if(s > -2) {
			clear();
			switch(op) {
				case OPERATION_EQUALS: {fr->setTrue(s == 0); break;}
				case OPERATION_LESS: {fr->setTrue(s > 0); break;}
				case OPERATION_GREATER: {fr->setTrue(s < 0); break;}
				case OPERATION_EQUALS_LESS: {fr->setTrue(s >= 0); break;}
				case OPERATION_EQUALS_GREATER: {fr->setTrue(s <= 0); break;}
				case OPERATION_NOT_EQUALS: {fr->setTrue(s != 0); break;}
				default: {}
			}
		} else {
		
			if(!translate_) {
				return false;
			}
			
			Manager *mngr2 = new Manager(this);	
			mngr2->add(mngr, OPERATION_SUBTRACT);
				
			clear();
			push_back(mngr2);
			mngr2 = new Manager();
			push_back(mngr2);
			switch(op) {
				case OPERATION_EQUALS: {comparison_type = COMPARISON_EQUALS; break;}
				case OPERATION_LESS: {comparison_type = COMPARISON_LESS; break;}
				case OPERATION_GREATER: {comparison_type = COMPARISON_GREATER; break;}
				case OPERATION_EQUALS_LESS: {comparison_type = COMPARISON_EQUALS_LESS; break;}
				case OPERATION_EQUALS_GREATER: {comparison_type = COMPARISON_EQUALS_GREATER; break;}
				case OPERATION_NOT_EQUALS: {comparison_type = COMPARISON_NOT_EQUALS; break;}
				default: {}
			}
			c_type = COMPARISON_MANAGER;
		}
	} else if(op == OPERATION_OR || OPERATION_AND) {
		int p; 
		if((p = isPositive()) >= 0) {
			if(p && op == OPERATION_OR) {
				set(1, 1);
			} else if(!p && op == OPERATION_AND) {
				clear();
			} else if(!mngr->isComparison() || !mngr->getChild(1)->isZero() || mngr->comparisonType() != COMPARISON_GREATER) {
				Manager *mngr2 = new Manager(mngr);
				clear();
				push_back(mngr2);
				mngr2 = new Manager();
				push_back(mngr2);
				c_type = COMPARISON_MANAGER;
				comparison_type = COMPARISON_GREATER;
			}
		} else if((p = mngr->isPositive()) >= 0) {
			if(p && op == OPERATION_OR) {
				set(1, 1);
			} else if(!p && op == OPERATION_AND) {
				clear();
			} else if(!isComparison() || !mngrs[1]->isZero() || comparison_type != COMPARISON_GREATER) {
				Manager *mngr2 = new Manager(this);
				clear();
				push_back(mngr2);
				mngr2 = new Manager();
				push_back(mngr2);
				c_type = COMPARISON_MANAGER;
				comparison_type = COMPARISON_GREATER;
			}		
		} else {
			bool b = false;
			if(isComparison() && mngrs[1]->isZero() && comparison_type == COMPARISON_GREATER) {
				moveto(mngrs[0]);
				b = true;
			} 
			if(mngr->isComparison() && mngr->getChild(1)->isZero() && mngr->comparisonType() == COMPARISON_GREATER) {
				b = true;
				mngr = mngr->getChild(0);
			}
			if(!b && !translate_) {
				return false;
			}
			if((c_type == OR_MANAGER && op == OPERATION_OR) || (c_type == AND_MANAGER && op == OPERATION_AND)) {
				push_back(new Manager(mngr));	
			} else if(op == OPERATION_OR) {
				if(mngr->type() == OR_MANAGER) {
					transform(mngr->getChild(0), OR_MANAGER, op);
					for(unsigned int i = 1; i < mngr->countChilds(); i++) {
						push_back(new Manager(mngr->getChild(i)));
					}
				} else {
					if(!translate_) {
						return false;
					}
					transform(mngr, OR_MANAGER, op);
				}
			} else {
				if(mngr->type() == AND_MANAGER) {
					transform(mngr->getChild(0), AND_MANAGER, op);
					for(unsigned int i = 1; i < mngr->countChilds(); i++) {
						push_back(new Manager(mngr->getChild(i)));
					}
				} else {
					if(!translate_) {
						return false;
					}
					transform(mngr, AND_MANAGER, op);
				}
			}
		}
	}
//	printf("PRESORT [%s]\n", print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str());	
	sort();
//	printf("POSTSORT [%s]\n", print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_FRACTION).c_str());	
	CALCULATOR->checkFPExceptions();

	return true;
}
bool Manager::setNOT(bool translate_) {
	if(isFraction()) {
		fr->setNOT();
	} else {
		if(!translate_) {
			return false;
		}
		Manager *mngr = new Manager(this);
		clear();
		push_back(mngr);
		c_type = NOT_MANAGER;
	}
	return true;
}
void Manager::addUnit(const Unit *u, MathOperation op) {
	Manager *mngr = new Manager(u);
	mngr->finalize();
	add(mngr, op);
	mngr->unref();
}
void Manager::addInteger(long int value_, MathOperation op) {
	Manager *mngr = new Manager(value_, 1);
	add(mngr, op);
	mngr->unref();
}
int Manager::compare(const Manager *mngr) const {
	if(isFraction() && mngr->isFraction()) {
		return fraction()->compare(mngr->fraction());
	} else if(isFraction() != mngr->isFraction()) {
		return -2;
	} else if(isMultiplication() && mngrs.size() == 2 && mngr->isMultiplication() && mngr->countChilds() == 2 && mngrs[0]->isFraction() && mngr->getChild(0)->isFraction() && mngrs[1]->isUnit_exp() && mngr->getChild(1)->isUnit_exp() && mngrs[1]->equals(mngr->getChild(1))) {
		return mngrs[0]->fraction()->compare(mngr->getChild(0)->fraction());
	} else if(equals(mngr)) {
		return 0;
	} else {
		Manager mngr2(this);
		mngr2.add(mngr, OPERATION_SUBTRACT);
		mngr2.finalize();
		if(mngr2.isFraction()) {
			Fraction fr;
			return mngr2.fraction()->compare(&fr);
		} else if(mngr2.isUnit()) {
			return -1;
		} else if(mngr2.isMultiplication() && mngr2.countChilds() == 2 && mngr2.getChild(0)->isFraction() && mngr2.getChild(1)->isUnit_exp()) {
			Fraction fr;
			return mngr2.getChild(0)->fraction()->compare(&fr);
		}
	}
	return -2;
}
int Manager::sortCompare(const Manager *mngr, int sortflags) const {
	if(c_type != mngr->type()) {
		if(mngr->type() == ADDITION_MANAGER) return -mngr->sortCompare(this, sortflags);	
		if(mngr->type() == MULTIPLICATION_MANAGER && c_type != ADDITION_MANAGER) return -mngr->sortCompare(this, sortflags);		
		if(mngr->type() == POWER_MANAGER && c_type != ADDITION_MANAGER && c_type != MULTIPLICATION_MANAGER) return -mngr->sortCompare(this, sortflags);		
	}
	switch(c_type) {
		case FRACTION_MANAGER: {
			if(mngr->isNull()) {
				if(isNull()) return 0;
				return -1;
			}
			if(isNull()) {
				if(mngr->type() == NULL_MANAGER) return 0;
				if(mngr->type() == FUNCTION_MANAGER) return -1;					
				return 1;			
			}
			if(mngr->isFraction()) {
				return fr->compare(mngr->fraction());
			}
			if(mngr->type() == FUNCTION_MANAGER) return -1;
			return 1;
		} 
		case UNIT_MANAGER: {
			if(mngr->type() == UNIT_MANAGER) {
				if(o_unit->name() < mngr->o_unit->name()) return -1;
				if(o_unit->name() == mngr->o_unit->name()) return 0;
				return 1;
			}
			if(mngr->isNumber() || mngr->type() == FUNCTION_MANAGER) return -1;
			return 1;
		}
		case STRING_MANAGER: {
			if(mngr->type() == STRING_MANAGER) {
				if(s_var < mngr->s_var) return -1;
				else if(s_var == mngr->s_var) return 0;
				else return 1;
			}
			if(mngr->isNumber() || mngr->type() == UNIT_MANAGER || mngr->type() == FUNCTION_MANAGER) return -1;
			return 1;
		}
		case FUNCTION_MANAGER: {
			if(mngr->type() == FUNCTION_MANAGER) {
				if(o_function->name() < mngr->o_function->name()) return -1;
				if(o_function->name() == mngr->o_function->name()) {
					for(unsigned int i = 0; i < mngr->mngrs.size(); i++) {
						if(i >= mngrs.size()) {
							return -1;	
						}
						int i2 = mngr->mngrs[i]->sortCompare(mngrs[i], sortflags);
						if(i2 != 0) return i2;
					}
					return 0;
				} else return 1;
			}
			return 1;
		}
		case POWER_MANAGER: {
			if(mngr->type() == POWER_MANAGER) {
				if(mngrs[1]->hasNegativeSign() && !mngr->mngrs[1]->hasNegativeSign()) return 1;
				if(!mngrs[1]->hasNegativeSign() && mngr->mngrs[1]->hasNegativeSign()) return -1;				
				int i = mngrs[0]->sortCompare(mngr->mngrs[0], sortflags);
				if(i == 0) {
					return mngrs[1]->sortCompare(mngr->mngrs[1], sortflags);
				} else {
					return i;
				}
			} else if(mngr->type() != ADDITION_MANAGER) {
				if(!(sortflags & SORT_SCIENTIFIC) && mngrs[1]->hasNegativeSign()) {
					return 1;				
				}
				int i = mngrs[0]->sortCompare(mngr, sortflags);
				if(i == 0) {
					if(sortflags & SORT_SCIENTIFIC) {
						if(mngrs[1]->hasNegativeSign()) return 1;
					}
					return -1;
				}
				return i;
			}
			return 1;
		}
		case MULTIPLICATION_MANAGER: {
			if(mngrs.size() < 1) return 1;
			if(mngr->isNumber()) return -1;
			unsigned int start = 0;
			if(mngrs[0]->isNumber() && mngrs.size() > 1) start = 1;
			if(mngr->mngrs.size() < 1 || mngr->type() == POWER_MANAGER) return mngrs[start]->sortCompare(mngr, sortflags);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				if(mngr->mngrs.size() < 1) return -1;
				unsigned int mngr_start = 0;
				if(mngr->mngrs[0]->isNumber() && mngr->mngrs.size() > 1) mngr_start = 1;			
				for(unsigned int i = 0; ; i++) {
					if(i >= mngr->mngrs.size() - mngr_start && i >= mngrs.size() - start) return 0;
					if(i >= mngr->mngrs.size() - mngr_start) return 1;
					if(i >= mngrs.size() - start) return -1;
					int i2 = mngrs[i + start]->sortCompare(mngr->mngrs[i + mngr_start], sortflags);
					if(i2 != 0) return i2;
				}
			} 
			if(mngr->type() == ADDITION_MANAGER) return 1;
			return -1;
		} 
		case ADDITION_MANAGER: {		
			if(mngrs.size() < 1) return 1;
			if(mngr->isNumber()) return -1;
			if(mngr->type() == ADDITION_MANAGER) {
				if(mngr->mngrs.size() < 1) return -1;
				for(unsigned int i = 0; ; i++) {
					if(i >= mngr->mngrs.size() && i >= mngrs.size()) return 0;
					if(i >= mngr->mngrs.size()) return 1;
					if(i >= mngrs.size()) return -1;
					int i2 = mngrs[i]->sortCompare(mngr->mngrs[i], sortflags);
					if(i2 != 0) return i2;
				}
			} 
			return mngrs[0]->sortCompare(mngr, sortflags);
		}
		default: {
			return 1;
		}
	}
	return 1;
}

void Manager::sort(int sortflags) {
	if(c_type == POWER_MANAGER || c_type == COMPARISON_MANAGER || mngrs.size() < 2) return;
	vector<Manager*> sorted;
	bool b;
	for(unsigned int i = 0; i < mngrs.size(); i++) {
		b = false;
		if(c_type == MULTIPLICATION_MANAGER && mngrs[i]->isNumber()) {
			sorted.insert(sorted.begin(), mngrs[i]);
		} else if(c_type == MULTIPLICATION_MANAGER && mngrs[i]->isPower() && mngrs[i]->base()->isNumber()) {	
			for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
				if(sorted[i2]->isNumber()) {
				} else if(sorted[i2]->isPower() && sorted[i2]->base()->isNumber()) {
					if(mngrs[i]->sortCompare(sorted[i2], sortflags) < 0) {
						b = true;
					}
				} else {
					b = true;
				}
				if(b) {
					sorted.insert(sorted.begin() + i2, mngrs[i]);
					break;
				}
			}
			if(!b) sorted.push_back(mngrs[i]);
		} else {		
			for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
				if(c_type == ADDITION_MANAGER && !(sortflags & SORT_SCIENTIFIC) && mngrs[i]->hasNegativeSign() != sorted[i2]->hasNegativeSign()) {
					if(sorted[i2]->hasNegativeSign()) {
						sorted.insert(sorted.begin() + i2, mngrs[i]);
						b = true;
						break;
					}
				} else if(!(c_type == MULTIPLICATION_MANAGER && sorted[i2]->isNumber_exp()) && mngrs[i]->sortCompare(sorted[i2], sortflags) < 0) {
					sorted.insert(sorted.begin() + i2, mngrs[i]);
					b = true;
					break;
				}
			}
			if(!b) sorted.push_back(mngrs[i]);
		}
	}
	for(unsigned int i2 = 0; i2 < sorted.size(); i2++) {
		mngrs[i2] = sorted[i2];
	}
}
void Manager::moveto(Manager *term) {
	term->ref();
	clear();
	o_unit = term->unit();
	s_var = term->text();
	o_function = term->function();
	comparison_type = term->comparisonType();
	fr->set(term->fraction());
	if(term->matrix()) {
		if(term->matrix()->isVector()) {
			mtrx = new Vector(term->matrix());
		} else {
			mtrx = new Matrix(term->matrix());
		}
	}
	for(unsigned int i = 0; i < term->mngrs.size(); i++) {
		term->mngrs[i]->ref();
		mngrs.push_back(term->mngrs[i]);
	}
	setPrecise(term->isPrecise());
	c_type = term->type();
	term->unref();
}
bool Manager::equals(const Manager *mngr) const {
	if(c_type == mngr->type()) {
		if(isNull()) {
			return true;
		} else if(c_type == FRACTION_MANAGER) {
			return mngr->fraction()->equals(fr);
		} else if(c_type == MATRIX_MANAGER) {
			return mngr->matrix()->equals(mtrx);
		} else if(c_type == UNIT_MANAGER) {
			return o_unit == mngr->o_unit;
		} else if(c_type == STRING_MANAGER) {
			return s_var == mngr->s_var;
		} else if(c_type == FUNCTION_MANAGER) {
			if(o_function != mngr->function()) return false;
			if(mngrs.size() != mngr->mngrs.size()) return false;			
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equals(mngr->mngrs[i])) return false;
			}			
			return true;
		} else if(c_type == MULTIPLICATION_MANAGER || c_type == ADDITION_MANAGER || c_type == ALTERNATIVE_MANAGER || c_type == OR_MANAGER || c_type == AND_MANAGER) {
			if(mngrs.size() != mngr->mngrs.size()) return false;
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equals(mngr->mngrs[i])) return false;
			}
			return true;
		} else if(c_type == POWER_MANAGER) {
			return mngrs[0]->equals(mngr->mngrs[0]) && mngrs[1]->equals(mngr->mngrs[1]);
		} else if(c_type == COMPARISON_MANAGER) {
			if(comparison_type != mngr->comparisonType()) return false;
			if(mngrs.size() != mngr->mngrs.size()) return false;			
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equals(mngr->mngrs[i])) return false;
			}			
			return true;
		}
	}
	return false;
}
bool Manager::compatible(const Manager *mngr) {
	if(isNumber() || mngr->isNumber()) return true;
	if(c_type == mngr->type()) {
		if(c_type == UNIT_MANAGER) {
			if(o_unit == mngr->o_unit) return true;
		} else if(c_type == STRING_MANAGER) {
			if(s_var == mngr->s_var) return true;
		} else if(c_type == FUNCTION_MANAGER) {
			return equals(mngr);
		} else if(c_type == MATRIX_MANAGER) {
			return equals(mngr);
		} else if(c_type == MULTIPLICATION_MANAGER) {
			if(mngrs[0]->isNumber() && !mngr->mngrs[0]->isNumber()) {
				if(mngrs.size() != mngr->mngrs.size() + 1) return false;
				for(unsigned int i = 1; i < mngrs.size(); i++) {
					if(!mngrs[i]->compatible(mngr->mngrs[i - 1])) return false;
				}
			} else if(!mngrs[0]->isNumber() && mngr->mngrs[0]->isNumber()) {
				if(mngrs.size() + 1 != mngr->mngrs.size()) return false;
				for(unsigned int i = 1; i < mngr->mngrs.size(); i++) {
					if(!mngrs[i - 1]->compatible(mngr->mngrs[i])) return false;
				}
			} else {
				if(mngrs.size() != mngr->mngrs.size()) return false;
				for(unsigned int i = 0; i < mngrs.size(); i++) {
					if(!mngrs[i]->compatible(mngr->mngrs[i])) return false;
				}
			}
			return true;
		} else if(c_type == ADDITION_MANAGER) {
			if(mngrs.size() != mngr->mngrs.size()) return false;
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equals(mngr->mngrs[i])) return false;
			}
			return true;
		} else if(c_type == POWER_MANAGER) {
			return mngrs[0]->equals(mngr->mngrs[0]) && mngrs[1]->equals(mngr->mngrs[1]);
		}
	} else if(c_type == MULTIPLICATION_MANAGER) {
		if(!mngr->isNumber()) {
			if(mngrs.size() != 2) return false;
			if(!mngrs[0]->isNumber()) return false;			
			if(!mngrs[1]->compatible(mngr)) return false;
			return true;
		}
	}
	return false;
}
void Manager::addFloat(long double value_, MathOperation op) {
	Manager *mngr = new Manager(value_);
	add(mngr, op);
	mngr->unref();
}
void Manager::clear() {
	fr->clear();
	o_unit = NULL;
	s_var = "";
	o_function = NULL;
	o_prefix = NULL;
	if(mtrx) delete mtrx;
	mtrx = NULL;
	for(unsigned int i = 0; i < mngrs.size(); i++) {
		mngrs[i]->unref();
	}
	mngrs.clear();
	c_type = FRACTION_MANAGER;
}
long double Manager::value() const {
	if(c_type == FRACTION_MANAGER) return fr->value();
	return 0;
}
Fraction *Manager::fraction() const {
	return fr;
}
Matrix *Manager::matrix() const {
	return mtrx;
}
bool Manager::isPrecise() const {
	return b_exact;
}
void Manager::setPrecise(bool is_precise) {
	b_exact = is_precise;
	if(isFraction()) {
		fraction()->setPrecise(b_exact);
	} else if(isMatrix()) {
		matrix()->setPrecise(b_exact);
	}
}
const string &Manager::text() const {return s_var;}
Unit *Manager::unit() const {return o_unit;}
Manager *Manager::getChild(unsigned int index) const {
	if(index >= 0 && index < mngrs.size()) {
		return mngrs[index];
	}
	return NULL;
}
unsigned int Manager::countChilds() const {return mngrs.size();}
Manager *Manager::base() const {
	if(!isPower()) return NULL;
	return mngrs[0];
}
Manager *Manager::exponent() const {
	if(!isPower()) return NULL;
	return mngrs[1];
}
ComparisonType Manager::comparisonType() const {return comparison_type;}
Function *Manager::function() const {return o_function;}
bool Manager::isAlternatives() const {return c_type == ALTERNATIVE_MANAGER;}
bool Manager::isText() const {return c_type == STRING_MANAGER;}
bool Manager::isUnit() const {return c_type == UNIT_MANAGER;}
bool Manager::isUnit_exp() const {return c_type == UNIT_MANAGER || (c_type == POWER_MANAGER && mngrs[0]->type() == UNIT_MANAGER);}
bool Manager::isAddition() const {return c_type == ADDITION_MANAGER;}
bool Manager::isMultiplication() const {return c_type == MULTIPLICATION_MANAGER;}
bool Manager::isFunction() const {return c_type == FUNCTION_MANAGER;}
bool Manager::isPower() const {return c_type == POWER_MANAGER;}
bool Manager::isNumber_exp() const {return c_type == FRACTION_MANAGER || (c_type == POWER_MANAGER && mngrs[0]->type() == FRACTION_MANAGER);}
bool Manager::isNumber() const {return c_type == FRACTION_MANAGER;}
bool Manager::isFraction_exp() const {return c_type == FRACTION_MANAGER || (c_type == POWER_MANAGER && mngrs[0]->type() == FRACTION_MANAGER);}
bool Manager::isFraction() const {return c_type == FRACTION_MANAGER;}
bool Manager::isMatrix() const {return c_type == MATRIX_MANAGER;}
bool Manager::isAND() const {return c_type == AND_MANAGER;}
bool Manager::isOR() const {return c_type == OR_MANAGER;}
bool Manager::isNOT() const {return c_type == NOT_MANAGER;}
bool Manager::isComparison() const {return c_type == COMPARISON_MANAGER;}
bool Manager::isNull() const {
	return c_type == FRACTION_MANAGER && fr->isZero();
}
bool Manager::isZero() const {
	return isNull();
}
bool Manager::isOne() const {return c_type == FRACTION_MANAGER && fr->isOne();}
bool Manager::negative() const {
	if(c_type == FRACTION_MANAGER) return fr->isNegative();	
	else if(c_type == MULTIPLICATION_MANAGER || c_type == POWER_MANAGER) return mngrs[0]->negative();
	return false;
}
bool Manager::hasNegativeSign() const {
	if(c_type == FRACTION_MANAGER) return fr->isNegative();	
	else if(c_type == MULTIPLICATION_MANAGER || c_type == POWER_MANAGER) return mngrs[0]->negative();
	return false;
}
int Manager::signedness() const {
	if(c_type == FRACTION_MANAGER) {
		if(fr->isPositive()) {
			return 1;
		} else if(fr->isZero()) {
			return 0;
		} else {
			return -1;
		}
	} else if(isUnit_exp()) {
		return 1;
	} else if(c_type == MULTIPLICATION_MANAGER || c_type == ADDITION_MANAGER || c_type == ALTERNATIVE_MANAGER) {
		int s = 0, s2;
		for(unsigned int i = 0; i < mngrs.size(); i++) {
			s2 = mngrs[i]->signedness();
			if(s2 < -1) {
				return -2;
			} else if((s2 < 0 && s > 0) || (s2 > 0 && s < 0)) {
				return -2;
			} else if(s2 != 0) {
				s = s2;
			}
		}
		return s;
	}
	return -2;
}
int Manager::isPositive() const {
	int s = signedness();
	if(s > 0) {
		return 1;
	} else if(s > -2) {
		return 0;
	}
	return -1;
}
bool Manager::testDissolveCompositeUnit(const Unit *u) {
	if(c_type == UNIT_MANAGER) {
		if(o_unit->unitType() == COMPOSITE_UNIT) {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				Manager *mngr = ((CompositeUnit*) o_unit)->generateManager();
				moveto(mngr);
				mngr->unref();
				return true;
			}
		} else if(o_unit->unitType() == ALIAS_UNIT && o_unit->baseUnit()->unitType() == COMPOSITE_UNIT) {
			if(((CompositeUnit*) (o_unit->baseUnit()))->containsRelativeTo(u)) {
/*				Manager *mngr = o_unit->baseUnit()->convert(o_unit);
				Manager *mngr2 = ((CompositeUnit*) o_unit->baseUnit())->generateManager();
				moveto(mngr2);
				mngr2->unref();
				add(mngr, OPERATION_MULTIPLY);
				mngr->unref();*/
				convert(o_unit->baseUnit());
				convert(u);
				return true;
			}		
		}
	}
	return false; 
}
bool Manager::testCompositeUnit(const Unit *u) {
	if(c_type == UNIT_MANAGER) {
		if(o_unit->unitType() == COMPOSITE_UNIT) {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				return true;
			}
		} else if(o_unit->unitType() == ALIAS_UNIT && o_unit->baseUnit()->unitType() == COMPOSITE_UNIT) {
			if(((CompositeUnit*) (o_unit->baseUnit()))->containsRelativeTo(u)) {
				return true;
			}		
		}
	}
	return false; 
}
void Manager::clean() {
	for(unsigned int i = 0; i < mngrs.size(); i++) {
		mngrs[i]->clean();
		if(!mngrs[i]->isPrecise()) setPrecise(false);
	}
	typeclean();
}
void Manager::recalculateFunctions() {
	for(unsigned int i = 0; i < mngrs.size(); i++) {
		mngrs[i]->recalculateFunctions();
	}
	switch(c_type) {
		case FUNCTION_MANAGER: {
			clean();
			Manager *mngr_f = function()->calculate(mngrs);
			moveto(mngr_f);
			mngr_f->unref();
			break;
		}
		case MATRIX_MANAGER: {
			mtrx->recalculateFunctions();
		}				
	}
}
void Manager::finalize() {
	clearPrefixes();	
	dissolveAllCompositeUnits();		
	syncUnits();
	clean();
}
void gatherInformation(Manager *mngr, vector<Unit*> &base_units, vector<AliasUnit*> &alias_units) {
	switch(mngr->type()) {
		case UNIT_MANAGER: {
			switch(mngr->unit()->unitType()) {
				case BASE_UNIT: {
					for(unsigned int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == mngr->unit()) {
							return;
						}
					}
					base_units.push_back(mngr->unit());
					break;
				}
				case ALIAS_UNIT: {
					for(unsigned int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == mngr->unit()) {
							return;
						}
					}
					alias_units.push_back((AliasUnit*) (mngr->unit()));
					break;
				}
				case COMPOSITE_UNIT: {
					mngr = ((CompositeUnit*) (mngr->unit()))->generateManager();
					gatherInformation(mngr, base_units, alias_units);
					break;
				}				
			}
			break;
		}
		case MATRIX_MANAGER: {
			for(unsigned int i = 1; i <= mngr->matrix()->rows(); i++) {
				for(unsigned int i2 = 1; i2 <= mngr->matrix()->columns(); i2++) {
					gatherInformation(mngr->matrix()->get(i, i2), base_units, alias_units);
				}	
			}
			break;
		}
		default: {
			for(unsigned int i = 0; i < mngr->countChilds(); i++) {
				gatherInformation(mngr->getChild(i), base_units, alias_units);
			}
			break;
		}
	}

}
void Manager::syncUnits() {
	vector<Unit*> base_units;
	vector<AliasUnit*> alias_units;
	vector<CompositeUnit*> composite_units;	
	gatherInformation(this, base_units, alias_units);
	CompositeUnit *cu;
	bool b = false;
	for(int i = 0; i < (int) alias_units.size(); i++) {
		if(alias_units[i]->baseUnit()->unitType() == COMPOSITE_UNIT) {
			b = false;
			cu = (CompositeUnit*) alias_units[i]->baseUnit();
			for(unsigned int i2 = 0; i2 < base_units.size(); i2++) {
				if(cu->containsRelativeTo(base_units[i2])) {
					for(unsigned int i = 0; i < composite_units.size(); i++) {
						if(composite_units[i] == cu) {
							b = true;
							break;
						}
					}
					if(!b) composite_units.push_back(cu);					
					goto erase_alias_unit_1;
				}
			}
			for(unsigned int i2 = 0; i2 < alias_units.size(); i2++) {
				if(cu->containsRelativeTo(alias_units[i2])) {
					for(unsigned int i = 0; i < composite_units.size(); i++) {
						if(composite_units[i] == cu) {
							b = true;
							break;
						}
					}
					if(!b) composite_units.push_back(cu);				
					goto erase_alias_unit_1;
				}
			}					
		}
		goto dont_erase_alias_unit_1;
		erase_alias_unit_1:
		alias_units.erase(alias_units.begin() + i);
		for(int i2 = 0; i2 < (int) cu->units.size(); i2++) {
			b = false;
			switch(cu->units[i2]->firstBaseUnit()->unitType()) {
				case BASE_UNIT: {
					for(unsigned int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) base_units.push_back((Unit*) cu->units[i2]->firstBaseUnit());
					break;
				}
				case ALIAS_UNIT: {
					for(unsigned int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) alias_units.push_back((AliasUnit*) cu->units[i2]->firstBaseUnit());				
					break;
				}
				case COMPOSITE_UNIT: {
					Manager *mngr = ((CompositeUnit*) cu->units[i2]->firstBaseUnit())->generateManager();
					gatherInformation(mngr, base_units, alias_units);
					break;
				}
			}
		}
		i = -1;
		dont_erase_alias_unit_1:
		;
	}
	for(int i = 0; i < (int) alias_units.size(); i++) {
		for(int i2 = 0; i2 < (int) alias_units.size(); i2++) {
			if(i != i2 && alias_units[i]->baseUnit() == alias_units[i2]->baseUnit()) { 
				if(alias_units[i2]->isParentOf(alias_units[i])) {
					goto erase_alias_unit_2;
				}
				if(!alias_units[i]->isParentOf(alias_units[i2])) {
					b = false;
					for(unsigned int i3 = 0; i3 < base_units.size(); i3++) {
						if(base_units[i3] == alias_units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) base_units.push_back((Unit*) alias_units[i]->baseUnit());
					goto erase_alias_unit_2;
				}
			}
		} 
		goto dont_erase_alias_unit_2;
		erase_alias_unit_2:
		alias_units.erase(alias_units.begin() + i);
		i--;
		dont_erase_alias_unit_2:
		;
	}	
	for(int i = 0; i < (int) alias_units.size(); i++) {
		if(alias_units[i]->baseUnit()->unitType() == BASE_UNIT) {
			for(unsigned int i2 = 0; i2 < base_units.size(); i2++) {
				if(alias_units[i]->baseUnit() == base_units[i2]) {
					goto erase_alias_unit_3;
				}
			}
		} 
		goto dont_erase_alias_unit_3;
		erase_alias_unit_3:
		alias_units.erase(alias_units.begin() + i);
		i--;
		dont_erase_alias_unit_3:
		;
	}
	for(unsigned int i = 0; i < composite_units.size(); i++) {	
		convert(composite_units[i]);
	}	
	dissolveAllCompositeUnits();
	for(unsigned int i = 0; i < base_units.size(); i++) {	
		convert(base_units[i]);
	}
	for(unsigned int i = 0; i < alias_units.size(); i++) {	
		convert(alias_units[i]);
	}	
}

bool Manager::dissolveAllCompositeUnits() {
	switch(type()) {
		case UNIT_MANAGER: {
			if(o_unit->unitType() == COMPOSITE_UNIT) {
				Manager *mngr = ((CompositeUnit*) o_unit)->generateManager();			
				moveto(mngr);
				mngr->unref();
				return true;
			}
			break;
		}
		case MATRIX_MANAGER: {
			bool b = false;
			for(unsigned int i = 1; i <= mtrx->rows(); i++) {
				for(unsigned int i2 = 1; i2 <= mtrx->columns(); i2++) {
					if(mtrx->get(i, i2)->dissolveAllCompositeUnits()) b = true;
				}
			}
			return b;		
		}
		default: {
			bool b = false;
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(mngrs[i]->dissolveAllCompositeUnits()) b = true;
			}
			return b;
		}		
	}		
	return false;
}
bool Manager::convert(string unit_str) {
	Manager *mngr = CALCULATOR->calculate(unit_str);
	bool b = convert(mngr);
	mngr->unref();
	return b;
}
bool Manager::convert(const Manager *unit_mngr) {
	bool b = false;
	if(unit_mngr->type() == UNIT_MANAGER) {
		if(convert(unit_mngr->unit())) b = true;
	} else {
		for(unsigned int i = 0; i < unit_mngr->mngrs.size(); i++) {
			if(convert(unit_mngr->mngrs[i])) b = true;
		}
	}	
	return b;
}

bool Manager::convert(const Unit *u) {
	if(isNumber() || c_type == STRING_MANAGER) return false;
	bool b = false;	
	if(c_type == UNIT_MANAGER && o_unit == u) return false;
	if(u->unitType() == COMPOSITE_UNIT && !(c_type == UNIT_MANAGER && o_unit->baseUnit() == u)) {
		Manager *mngr = ((CompositeUnit*) u)->generateManager();
		b = convert(mngr);
		mngr->unref();
		return b;
	}
	if(c_type == MATRIX_MANAGER) {
		for(unsigned int i = 1; i <= mtrx->rows(); i++) {
			for(unsigned int i2 = 1; i2 <= mtrx->columns(); i2++) {
				if(mtrx->get(i, i2)->convert(u)) b = true;
			}
		}
		return b;		
	} else if(c_type == UNIT_MANAGER) {
		if(u == o_unit) return false;
		if(testDissolveCompositeUnit(u)) {
			convert(u);
			return true;
		}
		Manager *exp = new Manager(1, 1);
		Manager *mngr = u->convert(o_unit, NULL, exp, &b);
		if(b) {
			o_unit = (Unit*) u;
			if(!exp->isNumber() || exp->value() != 1.0L) {
				add(exp, OPERATION_RAISE);
			}
			add(mngr, OPERATION_MULTIPLY);
		}
		exp->unref();
		mngr->unref();
		return b;
	} else if(c_type == POWER_MANAGER) {
		bool b = false;
		b = mngrs[1]->convert(u);
		if(b) {
			typeclean();
			return convert(u);
		}
		if(mngrs[0]->type() == UNIT_MANAGER) {
			if(u == mngrs[0]->unit()) return false;
			if(mngrs[0]->testDissolveCompositeUnit(u)) {
				typeclean();
				convert(u);			
				return true;
			}
			Manager *mngr = u->convert(mngrs[0]->unit(), NULL, mngrs[1], &b);
			if(b) {
				mngrs[0]->set(u);
				add(mngr, OPERATION_MULTIPLY);
			}			
			mngr->unref();
		} else {
			b = mngrs[0]->convert(u);
		}
		return b;
	} else if(c_type == MULTIPLICATION_MANAGER) {
		bool c = false;
		for(int i = 0; i < (int) mngrs.size(); i++) {
			if(mngrs[i]->testDissolveCompositeUnit(u)) {
				typeclean();			
				b = true;
				convert(u);
				typeclean();
				i = -1;
				if(b) c = true;
			} else if(mngrs[i]->type() == UNIT_MANAGER && mngrs[i]->unit() != u) {
				Manager *mngr;
				if(mngrs[i]->unit()->hasComplexRelationTo(u)) {
					int i3 = 0;
					for(unsigned int i2 = 0; i2 < mngrs.size(); i2++) {
						if(mngrs[i2]->type() == UNIT_MANAGER || (mngrs[i2]->type() == POWER_MANAGER && mngrs[i2]->mngrs[0]->type() == UNIT_MANAGER)) {
							i3++;
						}
					}
					if(i3 > 1) return false;
				}
				mngr = new Manager(this);
				mngr->add(mngrs[i], OPERATION_DIVIDE);
				Manager *exp = new Manager(1, 1);				
				u->convert(mngrs[i]->unit(), mngr, exp, &b);
				if(b) {
					set(u);
					if(!exp->isNumber() || exp->value() != 1.0L) {
						add(exp, OPERATION_RAISE);
					}
					add(mngr, OPERATION_MULTIPLY);										
					c = true;
				}
				mngr->unref();
			} else if(mngrs[i]->type() == POWER_MANAGER && mngrs[i]->mngrs[0]->type() == UNIT_MANAGER && mngrs[i]->mngrs[0]->unit() != u) {
				Manager *mngr;
				if(mngrs[i]->mngrs[0]->unit()->hasComplexRelationTo(u)) {
					int i3 = 0;
					for(unsigned int i2 = 0; i2 < mngrs.size(); i2++) {
						if(mngrs[i2]->type() == UNIT_MANAGER || (mngrs[i2]->type() == POWER_MANAGER && mngrs[i2]->mngrs[0]->type() == UNIT_MANAGER)) {
							i3++;
						}
					}
					if(i3 > 1) return false;				
				}
				mngr = new Manager(this);
				mngr->add(mngrs[i], OPERATION_DIVIDE);
				u->convert(mngrs[i]->mngrs[0]->unit(), mngr, mngrs[i]->mngrs[1], &b);
				if(b) {
					Manager *mngr2 = mngrs[i]->mngrs[1];
					mngr2->ref();
					set(u);
					add(mngr2, OPERATION_RAISE);
					mngr2->unref();
					add(mngr, OPERATION_MULTIPLY);					
					c = true;
				}		
				mngr->unref();			
			}
			;
		}
		for(unsigned int i = 0; i < mngrs.size(); i++) {
			if(mngrs[i]->testDissolveCompositeUnit(u)) {
				 convert(u); c = true;
			} else if(mngrs[i]->convert(u)) b = true;
		}
		if(b) {
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(mngrs[i]->testDissolveCompositeUnit(u)) {
					b = true;
					convert(u);
					typeclean();
					c = true;
				} else if(mngrs[i]->type() == UNIT_MANAGER) {
					if(mngrs[i]->unit()->hasComplexRelationTo(u)) {
						return true;
					}				
					Manager *mngr = new Manager(this);
					Manager *exp = new Manager(1, 1);
					mngr->add(mngrs[i], OPERATION_DIVIDE);
					u->convert(mngrs[i]->unit(), mngr, exp, &b);
					if(b) {
						set(u);
						if(!exp->isNumber() || exp->value() != 1.0L) {
							add(exp, OPERATION_RAISE);
						}
						add(mngr, OPERATION_MULTIPLY);
						c = true;
					}
					mngr->unref();
				} else if(mngrs[i]->type() == POWER_MANAGER && mngrs[i]->mngrs[0]->type() == UNIT_MANAGER && mngrs[i]->mngrs[0]->unit() != u) {
					if(mngrs[i]->mngrs[0]->unit()->hasComplexRelationTo(u)) {
						return true;
					}
					Manager *mngr = new Manager(this);
					mngr->add(mngrs[i], OPERATION_DIVIDE);
					u->convert(mngrs[i]->mngrs[0]->unit(), mngr, mngrs[i]->mngrs[1], &b);
					if(b) {	
						Manager *mngr2 = mngrs[i]->mngrs[1];
						mngr2->ref();
						set(u);
						add(mngr2, OPERATION_RAISE);
						add(mngr, OPERATION_MULTIPLY);
						mngr->unref();
						c = true;
					}			
					mngr->unref();			
				}
			}		
			c = true;
		}
		return c;			
	} else {
		for(unsigned int i = 0; i < mngrs.size(); i++) {
			if(mngrs[i]->convert(u)) b = true;
		}
		if(b) {
			typeclean();
		}
		return b;		
	}
}
void Manager::unref() {
	if(!b_protect) {
		refcount--;
		if(refcount <= 0) delete this;
	}
}
void Manager::ref() {
	if(!b_protect) {
		refcount++;
	}
}
void Manager::protect(bool do_protect) {
	b_protect = do_protect;
}
bool Manager::isProtected() const {
	return b_protect;
}
int Manager::type() const {
	return c_type;
}

void Manager::differentiate(string x_var) {
	switch(c_type) {
		case ADDITION_MANAGER: {
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				mngrs[i]->differentiate(x_var);
			}
			typeclean();
			break;
		}
		case FUNCTION_MANAGER: {
			Manager *mngr2 = new Manager(x_var);
			Manager *mngr = new Manager(CALCULATOR->getFunction("diff"), this, mngr2, NULL);
			mngr2->unref();
			moveto(mngr);
			break;
		}
		case STRING_MANAGER: {
			if(s_var == x_var) set(1, 1);
			else clear();
			break;
		}
		case POWER_MANAGER: {
			Manager *mngr = new Manager(mngrs[1]);
			Manager *mngr2 = new Manager(mngrs[0]);
			mngrs[1]->addInteger(-1, OPERATION_ADD);
			typeclean();
			add(mngr, OPERATION_MULTIPLY);
			mngr->unref();
			mngr2->differentiate(x_var);
			add(mngr2, OPERATION_MULTIPLY);
			mngr2->unref();
			break;
		}
		case MULTIPLICATION_MANAGER: {
			Manager *mngr = new Manager(mngrs[0]);
			Manager *mngr2 = new Manager(mngrs[1]);
			mngr->differentiate(x_var);
			mngr2->differentiate(x_var);			
			mngr->add(mngrs[1], OPERATION_MULTIPLY);					
			mngr2->add(mngrs[0], OPERATION_MULTIPLY);
			moveto(mngr);
			add(mngr2, OPERATION_ADD);
			mngr2->unref();			
			break;
		}		
		default: {
			clear();
			break;
		}	
	}
}

string Manager::print(NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, bool *in_exact, bool *usable, Prefix *set_prefix, bool toplevel, bool *plural, Integer *l_exp, bool in_composite, bool in_power, bool draw_minus, bool print_equals, bool in_multiplication, bool wrap, bool wrap_all, bool *has_parenthesis, bool in_div, bool no_add_one, Integer *l_exp2, Prefix **prefix1, Prefix **prefix2, string string_fr) const {

	string str = "";
	if(has_parenthesis) *has_parenthesis = false;
	if(in_exact && !isPrecise()) {
		*in_exact = true;
	}
	if(usable) *usable = true;
	switch(type()) {
		case STRING_MANAGER: {
			if(displayflags & DISPLAY_FORMAT_TAGS) {
				str += "<i>";
			}
			if(displayflags & DISPLAY_FORMAT_NONASCII) {
				if(text() == "pi") str += SIGN_PI;
				else if(text() == "euler") str += SIGN_GAMMA;
				else if(text() == "apery") str += SIGN_ZETA "(3)";
				else if(text() == "pythagoras") str += SIGN_SQRT "2";
				//else if(text() == "catalan") str += "G";
				else if(text() == "golden") str += SIGN_PHI;
				else str += text();
			} else {
				str += text();
			}
			if(displayflags & DISPLAY_FORMAT_TAGS) {
				str += "</i>";
			}
			if(plural) *plural = true;
			break;
		}
		case FUNCTION_MANAGER: {

			string func_str = function()->name();
			
			if(displayflags & DISPLAY_FORMAT_NONASCII) {
				if(func_str == "zeta") func_str = SIGN_ZETA;
				else if(func_str == "sqrt") func_str = SIGN_SQRT;
			}			

			str += func_str;
			
			str += LEFT_PARENTHESIS;
			for(unsigned int index = 0; index < mngrs.size(); index++) {
				if(l_exp) delete l_exp;
				l_exp = NULL;			
				str += mngrs[index]->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, l_exp, in_composite, in_power, true, print_equals);
				if(index > 0) {
					str += CALCULATOR->getComma();
					str += " ";
				}				
			}
			str += RIGHT_PARENTHESIS;
			
			if(plural) *plural = true;
			break;
		}			
		case UNIT_MANAGER: {
			if(set_prefix) {
				Manager *mngr_d = new Manager();
				mngr_d->setType(MULTIPLICATION_MANAGER);
				mngr_d->push_back(new Manager(1, 1));
				((Manager*) this)->ref();
				mngr_d->push_back((Manager*) this);
				str = mngr_d->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, NULL, true, in_power, in_composite, print_equals, in_multiplication, wrap, wrap_all, has_parenthesis, in_div, no_add_one, l_exp2, prefix1, prefix2);		
				mngr_d->unref();
				return str;
			} else {
				if(!no_add_one) {
					str += "1";
					if(min_decimals > 0) {
						str += CALCULATOR->getDecimalPoint();
						for(int i = 0; i < min_decimals; i++) {
							str += '0';
						}
					}
					str += " ";
				}
				if(prefix()) {
					str += prefix()->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);	
				}
				if(displayflags & DISPLAY_FORMAT_SHORT_UNITS) {
					if(displayflags & DISPLAY_FORMAT_NONASCII) {
						if(unit()->name() == "EUR") str += SIGN_EURO;
						else if(unit()->name() == "oC") str += SIGN_POWER_0 "C";
						else if(unit()->name() == "oF") str += SIGN_POWER_0 "F";
						else if(unit()->name() == "oR") str += SIGN_POWER_0 "R";
						else if(unit()->name() == "deg") str += SIGN_POWER_0;
						else str += unit()->name();
					} else {
						str += unit()->name();
					}
				} else if(plural && *plural) str += unit()->plural();
				else str += unit()->singular();
				if(displayflags & DISPLAY_FORMAT_ALLOW_NOT_USABLE) {
					gsub("_", " ", str);
					if(usable) *usable = false;
				}
			}
			break;
		}
		case FRACTION_MANAGER: {
			bool minus, exp_minus;
			string whole_, numerator_, denominator_, exponent_, prefix_;
			fraction()->getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags, min_decimals, max_decimals, set_prefix, in_exact, usable, false, plural, l_exp, in_composite || no_add_one, in_power, l_exp2, prefix1, prefix2);
			if(!whole_.empty()) {
				if(minus && (toplevel || draw_minus)) {
					if(displayflags & DISPLAY_FORMAT_NONASCII) {
						whole_.insert(0, SIGN_MINUS);
					} else {
						whole_.insert(whole_.begin(), 1, MINUS_CH);
					}
					if(!toplevel && wrap) wrap_all = true;
				}
				str += whole_;
				if(!exponent_.empty()) {
					str += EXP;
					if(exp_minus) {
						if(displayflags & DISPLAY_FORMAT_NONASCII) {
							str += SIGN_MINUS;
						} else {
							str += MINUS;
						}
					}
					str += exponent_;
				}
			}
			if(!numerator_.empty()) {
				if(whole_.empty() && minus && (toplevel || draw_minus)) {
					if(displayflags & DISPLAY_FORMAT_NONASCII) {
						str += SIGN_MINUS;				
					} else {
						str += MINUS;
					}
				}
				if(!exponent_.empty() && !exp_minus) {
					numerator_ += EXP;
					numerator_ += exponent_;
				}
				str += numerator_;
				if(displayflags & DISPLAY_FORMAT_NONASCII) {
					str += " ";
					str += SIGN_DIVISION;
					str += " ";
				} else {
					str += DIVISION;
				}
				if(!exponent_.empty() && exp_minus) {
					denominator_ += EXP;
					denominator_ += exponent_;
				}
				str += denominator_;
				if(!toplevel && wrap) wrap_all = true;
			}
			break;
		}	
		case MATRIX_MANAGER: {
			str += mtrx->print(nrformat, displayflags, min_decimals, max_decimals, set_prefix, in_exact, usable, false, plural, l_exp, in_composite, in_power);
		}
		case NOT_MANAGER: {
			str += NOT;
			if(l_exp) delete l_exp;
			l_exp = NULL;			
			str += getChild(0)->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, l_exp, in_composite, in_power, true, print_equals);
			break;
		}
		case COMPARISON_MANAGER: {}
		case AND_MANAGER: {}				
		case OR_MANAGER: {
			string sign_str;
			if(type() == COMPARISON_MANAGER) {
				switch(comparisonType()) {
					case COMPARISON_EQUALS: {
						sign_str += "==";
						break;
					}
					case COMPARISON_NOT_EQUALS: {
						if(displayflags & DISPLAY_FORMAT_NONASCII) {
							sign_str += SIGN_NOT_EQUAL;
						} else {
							sign_str += NOT EQUALS;
						}
						break;
					}
					case COMPARISON_GREATER: {
						sign_str += GREATER;
						break;
					}
					case COMPARISON_LESS: {
						sign_str += LESS;
						break;
					}
					case COMPARISON_EQUALS_GREATER: {
						if(displayflags & DISPLAY_FORMAT_NONASCII) {
							sign_str += SIGN_GREATER_OR_EQUAL;
						} else {
							sign_str += GREATER EQUALS;
						}
						break;
					}
					case COMPARISON_EQUALS_LESS: {
						if(displayflags & DISPLAY_FORMAT_NONASCII) {
							sign_str += SIGN_LESS_OR_EQUAL;
						} else {
							sign_str += LESS EQUALS;
						}
						break;
					}
				}
			} else if(type() == AND_MANAGER) {
				sign_str += AND;
			} else if(type() == OR_MANAGER) {
				sign_str += OR;
			}
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(l_exp) delete l_exp;
				l_exp = NULL;	
				if(i > 0) {
					str += " ";
					str += sign_str;
					str += " ";
				}
				str += mngrs[i]->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, l_exp, in_composite, in_power, true, print_equals);
			}
			if(!toplevel && wrap) wrap_all = true;
			break;
		}			
		case ADDITION_MANAGER: {
			
			if(displayflags & DISPLAY_FORMAT_SCIENTIFIC) {
				((Manager*) this)->sort(SORT_SCIENTIFIC);
			} else {
				((Manager*) this)->sort(SORT_DEFAULT);
			}
			
			for(unsigned int i = 0; i < mngrs.size(); i++) {
				if(l_exp) delete l_exp;
				l_exp = NULL;	
				if(mngrs[i]->hasNegativeSign()) {
					if(i > 0) {
						str += " ";
					}
					if(displayflags & DISPLAY_FORMAT_NONASCII) {
						str += SIGN_MINUS;				
					} else {
						str += MINUS;
					}						
					if(i > 0) {
						str += " ";
					}
				} else if(i > 0) {
					str += " ";
					if(displayflags & DISPLAY_FORMAT_NONASCII) {
						str += SIGN_PLUS;
					} else {
						str += PLUS;
					}
					str += " ";
				}
				str += mngrs[i]->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, l_exp, in_composite, in_power, false, print_equals, false, mngrs[i]->isAddition());
			}
			
			((Manager*) this)->sort();
			
			if(!toplevel && wrap) wrap_all = true;
			
			break;
		}	
		case POWER_MANAGER: {
			if(!(displayflags & DISPLAY_FORMAT_SCIENTIFIC)) {
				if(exponent()->isFraction() && exponent()->fraction()->numerator()->equals(1) && exponent()->fraction()->denominator()->equals(2)) {
					Function *sqrt = CALCULATOR->getFunction("sqrt");
					if(sqrt) {
						Manager *m2 = new Manager(sqrt, base(), NULL);
						str = m2->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, toplevel, plural, l_exp, in_composite, in_power, draw_minus, print_equals, in_multiplication, wrap);
						m2->unref();
						return str;
					}
				} else if(!in_multiplication && exponent()->hasNegativeSign()) {
					Manager *m2 = new Manager();
					m2->setType(MULTIPLICATION_MANAGER);
					((Manager*) this)->ref();
					m2->push_back((Manager*) this);
					str = m2->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, toplevel, plural, l_exp, in_composite, in_power, draw_minus, print_equals, in_multiplication, wrap);
					m2->unref();
					return str;
				}
			}
			if(set_prefix && base()->isUnit() && exponent()->isFraction() && exponent()->fraction()->isInteger()) {
				Manager *mngr_d = new Manager();
				mngr_d->setType(MULTIPLICATION_MANAGER);
				mngr_d->push_back(new Manager(1, 1));
				((Manager*) this)->ref();
				mngr_d->push_back((Manager*) this);
				str = mngr_d->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, NULL, true, in_power, true, print_equals, in_multiplication, wrap, wrap_all, has_parenthesis, in_div, no_add_one, l_exp2, prefix1, prefix2);		
				mngr_d->unref();
				return str;
			}
			if(!no_add_one && base()->isUnit()) {
				str += "1 ";
			}
			bool wrap_base = false;
			if((base()->hasNegativeSign() || (base()->isFraction() && !base()->fraction()->isInteger() && ((displayflags & DISPLAY_FORMAT_FRACTION) || (displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY))))) {
				wrap_base = true;
			}
			str += base()->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, NULL, in_composite, in_power, true, print_equals, false, true, wrap_base, NULL, 0, true);			
			
			if(!in_power && displayflags & DISPLAY_FORMAT_TAGS) {
				str += "<sup>";
			} else {
				str += POWER;
			}
			if(!(displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY) && (displayflags & DISPLAY_FORMAT_FRACTION)) {
				displayflags = displayflags | DISPLAY_FORMAT_FRACTIONAL_ONLY;
			}			
			str += exponent()->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, NULL, in_composite, true, true, print_equals, false, true, false, NULL, 0, true);

			if(!in_power && displayflags & DISPLAY_FORMAT_TAGS) {
				str += "</sup>";
			}
			if(!toplevel && wrap) wrap_all = true;
			break;
		} 
		case MULTIPLICATION_MANAGER: {
		
			if(displayflags & DISPLAY_FORMAT_SCIENTIFIC) {
				((Manager*) this)->sort(SORT_SCIENTIFIC);
			} else {
				((Manager*) this)->sort(SORT_DEFAULT);
			}

			Manager *m_i, *m_i_prev = NULL;
			int div = 0;			
			Prefix *prefix_1 = NULL;
			Prefix *prefix_2 = NULL;
			if(l_exp) delete l_exp;
			l_exp = NULL;
			if(l_exp2) delete l_exp2;
			l_exp2 = NULL;
			bool no_prefix = false;
			for(unsigned int i = 0; i < countChilds(); i++) {
				if(getChild(i)->prefix()) {
					no_prefix = true;
					break;
				}
			}
			if(!in_div) {
				if(!(displayflags & DISPLAY_FORMAT_SCIENTIFIC)) {
					for(unsigned int i = 0; i < countChilds(); i++) {
						m_i = getChild(i);
						if(m_i->isPower() && m_i->exponent()->hasNegativeSign()) {
							div = true;
							break;
						}	
					}
				}
			} else {
				prefix_1 = *prefix1;
			}
			
			if(!div) {
				vector<int> do_space;
				bool first_is_minus = false;
				bool next_plural = false;
				vector<string> string_factors;
				vector<bool> f_has_parenthesis;
				m_i_prev = NULL;
				bool plural_ = true;
				bool prepend_one = false;
				bool tmp_par;
				string string_prefix;
				if(string_fr.empty()) {
					if(!no_prefix) {
						for(unsigned int i = 0; i < countChilds(); i++) {
							m_i = getChild(i);
							if(m_i->isUnit()) {
								l_exp = new Integer(1);
								break;
							} else if(m_i->isUnit_exp()) {
								if(m_i->exponent()->isFraction() && m_i->exponent()->fraction()->isInteger()) {
									l_exp = new Integer(m_i->exponent()->fraction()->numerator());
								}
								break;
							}
						}
					}
				}
				if(string_fr.empty() && !prefix_1 && set_prefix && !getChild(0)->isFraction() && l_exp) {
					m_i = new Manager(1, 1);
					string_fr = m_i->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, &plural_, l_exp, in_composite, in_power, draw_minus || toplevel, print_equals, true, true, false, &tmp_par, 0, false, NULL, &prefix_1);
					m_i->unref();
				}
				for(unsigned int i = 0; i < countChilds(); i++) {
					m_i = getChild(i);
					if(i == 0 && m_i->isFraction() && m_i->fraction()->isMinusOne() && countChilds() > 1 && !getChild(1)->isUnit_exp()) {
						first_is_minus = true;
						string_factors.push_back("");
						do_space.push_back(0);			
						f_has_parenthesis.push_back(false);
						i++;
						m_i = getChild(i);
					}	
					
					if(!no_add_one && i == 0 && string_fr.empty() && m_i->isUnit_exp()) {
						prepend_one = true;
						plural_ = false;
					} 
				
					if(next_plural) {
						plural_ = true;
						next_plural = false;
					}
					if(plural_ && m_i->isUnit() && i + 1 < countChilds() && getChild(i + 1)->isUnit()) {
						plural_ = false;
						next_plural = true;
					}
					if(m_i->isFraction()) {
						string_factors.push_back(m_i->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, &plural_, l_exp, false, in_power, draw_minus || toplevel || i, print_equals, true, true, false, &tmp_par, 0, no_add_one, NULL, &prefix_1));
						if(l_exp) delete l_exp;
						l_exp = NULL;
					} else {
						string_factors.push_back(m_i->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, NULL, false, &plural_, NULL, false, in_power, draw_minus || toplevel || i, print_equals, true, true, false, &tmp_par, 0, true));
					}
					f_has_parenthesis.push_back(tmp_par);
					
					if(!m_i_prev) {
						if(!string_fr.empty() || prepend_one) {
							if(f_has_parenthesis[i]) {
								do_space.push_back(0);
							} else {
								if(m_i->isText() && (text_length_is_one(m_i->text()) || (displayflags & DISPLAY_FORMAT_NONASCII && (m_i->text() == "pi" || m_i->text() == "euler" || m_i->text() == "golden")))) {
									do_space.push_back(0);
								} else if(m_i->isPower() && m_i->base()->isFraction()) {
									do_space.push_back(2);
								} else {
									do_space.push_back(1);
								}
							}
						} else {
							do_space.push_back(0);
						}
					} else {
						if(m_i_prev->countChilds() > 1 && !m_i_prev->isPower()) {
							do_space.push_back(2);
						} else if(f_has_parenthesis[i - 1]) {
							if(f_has_parenthesis[i]) {
								do_space.push_back(0);
							} else {
								do_space.push_back(2);
							}
						} else if(f_has_parenthesis[i]) {
							if(m_i_prev->isUnit_exp()) {
								do_space.push_back(2);
							} else {
								do_space.push_back(0);
							}
						} else if(m_i_prev->isFraction()) {
							if(m_i->isText() && (text_length_is_one(m_i->text()) || (displayflags & DISPLAY_FORMAT_NONASCII && (m_i->text() == "pi" || m_i->text() == "euler" || m_i->text() == "golden")))) {
								do_space.push_back(0);
							} else if(m_i->isPower() && m_i->base()->isFraction()) {
								do_space.push_back(2);
							} else {
								do_space.push_back(1);
							}
						} else if(m_i_prev->isPower() && m_i_prev->base()->isText()) {
							do_space.push_back(0);
						} else if(m_i->isUnit_exp() && m_i_prev->isUnit_exp()) {
							if(!(displayflags & DISPLAY_FORMAT_SHORT_UNITS)) {
								do_space.push_back(1);
							} else {
								do_space.push_back(2);
							}
						} else if(m_i_prev->isText() && (text_length_is_one(m_i_prev->text()) || (displayflags & DISPLAY_FORMAT_NONASCII && (m_i_prev->text() == "pi" || m_i_prev->text() == "euler" || m_i_prev->text() == "golden")))) {
							if(m_i->isUnit_exp()) {
								do_space.push_back(1);
							} else {
								do_space.push_back(0);
							}
						} else {
							do_space.push_back(2);
						}
					}
					
					if(do_space[i] == 2) {
						if(!toplevel && wrap) wrap_all = true;
					}
					m_i_prev = m_i;
				}
				if(prefix_1) {
					string_prefix = prefix_1->name(displayflags & DISPLAY_FORMAT_SHORT_UNITS);
					if((displayflags & DISPLAY_FORMAT_SHORT_UNITS) && (displayflags & DISPLAY_FORMAT_NONASCII)) {
						if(string_prefix == "micro") string_prefix = SIGN_MICRO;
					}
				}
				for(int i = 0; i < (int) countChilds(); i++) {
					if(i == 0 && first_is_minus) {
						if(toplevel || draw_minus) {
							if(displayflags & DISPLAY_FORMAT_NONASCII) {
								str += SIGN_MINUS;
							} else {
								str += MINUS;
							}
						}	
					} else {
						if(prepend_one) {
							str += "1";
							prepend_one = false;
						} else if(!string_fr.empty()) {
							str += string_fr;
							string_fr = "";
						}
						if(do_space[i] == 2) {
							str += " ";
							if(displayflags & DISPLAY_FORMAT_NONASCII) {
								str += SIGN_MULTIDOT;
							} else {
								str += MULTIPLICATION;
							}
							str += " ";
						} else if(do_space[i]) {
							str += " ";
						}
						if(!string_prefix.empty() && getChild(i)->isUnit_exp()) {
							str += string_prefix;
							string_prefix = "";
						}		
						str += string_factors[i];
					}
				}
			} else {
				bool first_is_minus = false;
				vector<Manager*> num_mngrs;
				vector<Manager*> den_mngrs;
				bool first_is_copy = false;
				string string_num_fr = "";
				string string_den_fr = "";
				bool do_num_frac = false, do_den_frac = false;
				for(unsigned int i = 0; i < countChilds(); i++) {
					m_i = getChild(i);
					if(i == 0 && m_i->isFraction()) {
						if(m_i->fraction()->isNegative()) {
							first_is_minus = draw_minus || toplevel;
						}
						bool exp_minus = false;
						bool minus = false;
						string whole_, numerator_, denominator_, exponent_, prefix_;
						Manager *m_i2;
						if(!no_prefix) {
							for(unsigned int i2 = 0; i2 < countChilds(); i2++) {
								m_i2 = getChild(i2);
								if(m_i2->isUnit()) {
									if(!l_exp) l_exp = new Integer(1);
								} else if(m_i2->isUnit_exp()) {
									if(m_i2->exponent()->isFraction() && m_i2->exponent()->fraction()->isInteger()) {
										if(m_i2->exponent()->fraction()->isNegative()) {
											l_exp2 = new Integer(m_i2->exponent()->fraction()->numerator());
											l_exp2->setNegative(false);
											break;
										} else if(!l_exp) {
											l_exp = new Integer(m_i2->exponent()->fraction()->numerator());
										}
									}
								}
							}
						}
						int displayflags_d = displayflags;
						if((displayflags & DISPLAY_FORMAT_FRACTION) && !(displayflags_d & DISPLAY_FORMAT_FRACTIONAL_ONLY)) {
							displayflags_d = displayflags_d | DISPLAY_FORMAT_FRACTIONAL_ONLY;
						}	
						m_i->fraction()->getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags_d, min_decimals, max_decimals, set_prefix, in_exact, usable, false, NULL, l_exp, in_composite, in_power, l_exp2, &prefix_1, &prefix_2);
						if(l_exp) delete l_exp;
						l_exp = NULL;
						if(l_exp2) delete l_exp2;
						l_exp2 = NULL;
						if(!denominator_.empty() || (exp_minus && !exponent_.empty())) {
							if(denominator_.empty() && exp_minus) denominator_ = "1";
							if(exp_minus && !exponent_.empty()) {
								exponent_.insert(0, EXP);
								denominator_ += exponent_;
							}
							string_den_fr = denominator_;			
						}
						if(numerator_.empty()) {
							numerator_ = whole_;
						}
						if(numerator_ == "1") numerator_ = "";
						if(!exp_minus && !exponent_.empty()) {
							if(numerator_.empty()) numerator_ = "1";
							exponent_.insert(0, EXP);
							numerator_ += exponent_;
						}
						string_num_fr = numerator_;
						do_num_frac = !string_num_fr.empty() || prefix_1;
						do_den_frac = !string_den_fr.empty() || prefix_2;
					} else if(m_i->isPower() && m_i->exponent()->hasNegativeSign()) {
						m_i = new Manager(m_i);
						m_i->addInteger(-1, OPERATION_RAISE);
						den_mngrs.push_back(m_i);
					} else {
						num_mngrs.push_back(m_i);
					}
				}
				string num_string;
				string den_string;
				if(!do_num_frac && num_mngrs.size() == 1) {
					num_string = num_mngrs[0]->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, set_prefix, false, NULL, l_exp, false, in_power, false, print_equals, true, true, false, NULL, 1, in_composite);
				} else if(num_mngrs.size() > 0) {
					Manager *num_mngr = new Manager();
					num_mngr->setType(MULTIPLICATION_MANAGER);
					for(unsigned int i = 0; i < num_mngrs.size(); i++) {
						num_mngrs[i]->ref();
						num_mngr->push_back(num_mngrs[i]);
					}
					num_string = num_mngr->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, NULL, false, NULL, l_exp, in_composite, in_power, false, print_equals, true, true, false, NULL, 1, in_composite, NULL, &prefix_1, NULL, string_num_fr);
					num_mngr->unref();
				}
				if(first_is_copy) {
					num_mngrs[0]->unref();
				}
				if(!do_den_frac && den_mngrs.size() == 1) {
					den_string = den_mngrs[0]->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, NULL, false, NULL, l_exp, false, in_power, false, print_equals, true, true, false, NULL, 2, true);
					den_mngrs[0]->unref();
				} else if(den_mngrs.size() > 0) {
					Manager *den_mngr = new Manager();
					den_mngr->setType(MULTIPLICATION_MANAGER);
					for(unsigned int i = 0; i < den_mngrs.size(); i++) {
						den_mngr->push_back(den_mngrs[i]);
					}
					den_string = den_mngr->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, NULL, false, NULL, l_exp, in_composite, in_power, false, print_equals, true, true, false, NULL, 2, true, NULL, &prefix_2, NULL, string_den_fr);
					den_mngr->unref();
				}
				if(first_is_minus) {
					if(displayflags & DISPLAY_FORMAT_NONASCII) {
						str += SIGN_MINUS;
					} else {
						str += MINUS;
					}
				}
				if(!num_string.empty()) {
					str += num_string;
				} else {
					str += "1";
				}
				str += " ";
				if(displayflags & DISPLAY_FORMAT_NONASCII) {
					str += SIGN_DIVISION;
				} else {
					str += DIVISION;
				}
				str += " ";
				if(!den_string.empty()) {
					str += den_string;
				} else {
					str += "1";
				}
			}
			
			((Manager*) this)->sort();
			if(!toplevel && wrap) wrap_all = true;
			break;
		}			
		case ALTERNATIVE_MANAGER: {

			string alt_str;
			for(unsigned int i = 0; i < countChilds(); i++) {

				bool bie = false;
				if(i > 0) {
					str += " ";
					str += _("or");
					str += " ";
				}
				alt_str = getChild(i)->print(nrformat, displayflags, min_decimals, max_decimals, &bie, usable, set_prefix, false, NULL, l_exp, in_composite, in_power, true, print_equals);
				if(print_equals) {
					if(!bie) {
						str += EQUALS;
					} else {
						if(displayflags & DISPLAY_FORMAT_NONASCII) {
							str += SIGN_ALMOST_EQUAL;
						} else {
							str += EQUALS;
							str += _("approx.");
						}
					}
				}
				str += alt_str;
				if(bie && in_exact) {
					*in_exact = true;
				}	
			}
			
			if(!toplevel && wrap) wrap_all = true;
			
			toplevel = false;
		}		
	}
	if(wrap_all && !str.empty()) {
		str.insert(0, LEFT_PARENTHESIS);
		if(has_parenthesis) *has_parenthesis = true;
		str += RIGHT_PARENTHESIS;
	}
	if(toplevel && print_equals && !str.empty()) {
		if((in_exact && *in_exact) || !isPrecise()) {
			if(displayflags & DISPLAY_FORMAT_NONASCII) {
				str.insert(0, SIGN_ALMOST_EQUAL " ");
			} else {
				str.insert(0, EQUALS " ");
				str.insert(0, _("approx."));
			}
		} else {
			str.insert(0, EQUALS " ");
		}
	}
	return str;
}

Vector *Manager::generateVector(string x_var, const Manager *min, const Manager *max, int steps, Vector **x_vector) {
	if(steps < 1) {
		steps = 1;
	}
	Manager x_value(min);
	Manager x_mngr(x_var);
	Manager y_value;
	Vector *y_vector = new Vector();
	if(x_vector) {
		*x_vector = new Vector();
	}
	bool b = false;
	Manager step(max);
	step.add(min, OPERATION_SUBTRACT);
	step.addInteger(steps, OPERATION_DIVIDE);
	for(int i = 0; i <= steps; i++) {
		if(x_vector) {
			if(b) (*x_vector)->addComponent();
			(*x_vector)->set(&x_value, (*x_vector)->components());
		}
		y_value.set(this);
		y_value.replace(&x_mngr, &x_value);
		y_value.recalculateFunctions();
		y_value.finalize();
		if(b) y_vector->addComponent();
		y_vector->set(&y_value, y_vector->components());
		x_value.add(&step, OPERATION_ADD);
		b = true;
	}
	return y_vector;
}
Vector *Manager::generateVector(string x_var, Vector *x_vector) {
	Manager y_value;
	Manager x_mngr(x_var);
	Vector *y_vector = new Vector();
	for(unsigned int i = 1; i <= x_vector->components(); i++) {
		y_value.set(this);
		y_value.replace(&x_mngr, x_vector->get(i));
		y_value.recalculateFunctions();
		y_value.finalize();
		if(i > 1) y_vector->addComponent();
		y_vector->set(&y_value, y_vector->components());
	}
	return y_vector;
}

void Manager::replace(Manager *replace_this, Manager *replace_with) {
	if(this->equals(replace_this)) {
		set(replace_with);
		return;
	}
	for(unsigned int i = 0; i < mngrs.size(); i++) {
		mngrs[i]->replace(replace_this, replace_with);
	}
	clean();
}
void Manager::replace_no_copy(Manager *replace_this, Manager *replace_with) {
	for(unsigned int i = 0; i < mngrs.size(); i++) {
		if(mngrs[i]->equals(replace_this)) {
			mngrs[i]->unref();
			mngrs[i] = replace_with;
		}
		mngrs[i]->replace_no_copy(replace_this, replace_with);
	}
	clean();
}

/*void Manager::differentiate(string x_var) {
	Manager *a_mngr = new Manager(this);
	Manager *replace_this = new Manager(x_var);
	Manager *replace_with = new Manager(x_var);
	Manager *h_mngr = new Manager("limit");
	replace_with->add(h_mngr, OPERATION_ADD);
	replace(replace_this, replace_with);
	add(a_mngr, OPERATION_SUBTRACT);
	add(h_mngr, OPERATION_DIVIDE);
	Manager *null_mngr = new Manager(0, 1, 0);
	replace(h_mngr, null_mngr);
	a_mngr->unref();
	replace_this->unref();
	replace_with->unref();
	h_mngr->unref();
	null_mngr->unref();
}*/

/*

PLUS
---------------------
= 0 => SAME

PLUS	+	PLUS	= PLUS		if TERM + TERM != PLUS => MERGE
PLUS	+	MULTI	= PLUS
PLUS	+	POWER	= PLUS
PLUS	+	0	= SAME
PLUS	+	var	= PLUS
PLUS	+	U	= PLUS

MULTI	+	PLUS	= see above
MULTI	+	MULTI	= PLUS || MULTI ax + bx = (a+b)x
MULTI	+	POWER	= PLUS || MULTI a(x^y) +x^y  = (a+1)(x^y)
MULTI	+	0	= SAME
MULTI	+	var	= PLUS [ || MULTI ax + a = a(x + 1) ]
MULTI	+	U	= PLUS || MULTI au + u = (a + 1)u

POWER	+	PLUS	= see above
POWER	+	MULTI	= see above
POWER	+	POWER	= PLUS || MULTI x^y + x^y = 2x^y
POWER	+	0	= SAME
POWER	+	var	= PLUS
POWER	+	U	= PLUS

0	+	PLUS	= y
0	+	MULTI	= y
0	+	POWER	= y
0	+	0	= 0
0	+	var	= y
0	+	U	= y

var	+	PLUS	= see above
var	+	MULTI	= see above
var	+	POWER	= see above
var	+	0	= SAME
var	+	var	= var (a+b)
var	+	U	= PLUS

U	+	PLUS	= see above
U	+	MULTI	= see above
U	+	POWER	= see above
U	+	0	= SAME
U	+	var	= see above
U	+	U	= MULTI (2u)

MULTI
---------------------
= 0  => 0
= 1  => SAME
= -1 => -SAME

PLUS	*	PLUS	= PLUS (x + y) * (w + z) = xw + xz + yw + yz
PLUS	*	MULTI	= PLUS (x + y) * wz = xwz + ywz
PLUS	*	POWER	= PLUS (x + y) * w^z = x(w^z) + y(w^z)
PLUS	*	0	= 0
PLUS	*	var	= PLUS (x + y)*a = ax + ay
PLUS	*	U	= PLUS (x + y)*u = ux + uy

MULTI	*	PLUS	= see above	if TERM * TERM != MULTI => MERGE
MULTI	*	MULTI	= MULTI xy * wz = xywz
MULTI	*	POWER	= MULTI xy * w^z = xy(w^z)
MULTI	*	0	= 0
MULTI	*	var	= MULTI axy * b = (a + b)xy
MULTI	*	U	= MULTI

POWER	*	PLUS	= see above
POWER	*	MULTI	= see above
POWER	*	POWER	= MULTI x^y * w^z = (x^y)(x^z) || POWER x^y * x^z = x^(y + z)
POWER	*	0	= 0
POWER	*	var	= MULTI x^y * a = ax^y || POWER a^y * a = a^(y + 1)
POWER	*	U	= MULTI x^y * u = ux^y || POWER u^y * u = u^(y + 1)

0	*	PLUS	= 0
0	*	MULTI	= 0
0	*	POWER	= 0
0	*	0	= 0
0	*	var	= 0
0	*	U	= 0

var	*	PLUS	= see above
var	*	MULTI	= see above
var	*	POWER	= see above
var	*	0	= 0
var	*	var	= var a* b = (a*b)
var	*	U	= MULTI a * u = au

U	*	PLUS	= see above
U	*	MULTI	= see above
U	*	POWER	= see above
U	*	0	= 0
U	*	var	= see above
U	*	U	= POWER u * u = u^2

POWER
---------------------
= 0  => 1
= 1  => SAME
= -1 => 1 / SAME


PLUS	^	PLUS	= POWER
PLUS	^	MULTI	= POWER
PLUS	^	POWER	= POWER
PLUS	^	0	= 1
PLUS	^	var	= PLUS	(x + y)(x + y)(x + y)...
PLUS	^	U	= POWER

MULTI	^	PLUS	= MULTI (x^z)(y^z)
MULTI	^	MULTI	= MULTI
MULTI	^	POWER	= MULTI
MULTI	^	0	= 1
MULTI	^	var	= MULTI
MULTI	^	U	= MULTI

POWER	^	PLUS	= POWER x^(y * z)
POWER	^	MULTI	= POWER
POWER	^	POWER	= POWER
POWER	^	0	= 1
POWER	^	var	= POWER
POWER	^	U	= POWER

0	^	PLUS	= 0
0	^	MULTI	= 0
0	^	POWER	= 0
0	^	0	= ?
0	^	var	= 0
0	^	U	= 0

var	^	PLUS	= POWER a^(x)
var	^	MULTI	= POWER
var	^	POWER	= POWER
var	^	0	= 1
var	^	var	= var a^b
var	^	U	= POWER

U	^	PLUS	= POWER u^(x)
U	^	MULTI	= POWER
U	^	POWER	= POWER
U	^	0	= 1
U	^	var	= POWER
U	^	U	= POWER

*/

