/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/


#include "Manager.h"

Manager::Manager(Calculator *calc_) {
	calc = calc_;
	refcount = 1;
	clear();
}
Manager::Manager(Calculator *calc_, long double value_) {
	calc = calc_;
	refcount = 1;
	set(value_);
}
Manager::Manager(Calculator *calc_, string var_) {
	calc = calc_;
	refcount = 1;
	set(var_);
}
Manager::Manager(Calculator *calc_, Function *f, ...) {
	calc = calc_;
	refcount = 1;
	Manager *mngr;
	va_list ap;
	va_start(ap, f); 
	for(int i = 0; true; i++) {
		mngr = va_arg(ap, Manager*);
		if(mngr == NULL) break;
		mngrs.push_back(new Manager(mngr));
	}
	va_end(ap);	
	o_function = f;
	c_type = FUNCTION_MANAGER;
}
Manager::Manager(Calculator *calc_, Unit *u, long double value_) {
	calc = calc_;
	refcount = 1;
	set(u, value_);
}
Manager::Manager(const Manager *mngr) {
	refcount = 1;
	set(mngr);
}
Manager::~Manager() {
	clear();
}
void Manager::set(const Manager *mngr) {
	clear();
	if(mngr != NULL) {
		calc = mngr->calc;
		d_value = mngr->d_value;
		o_unit = mngr->o_unit;
		s_var = mngr->s_var;
		o_function = mngr->o_function;
		for(int i = 0; i < mngr->mngrs.size(); i++) {
			mngrs.push_back(new Manager(mngr->mngrs[i]));
		}
		c_type = mngr->type();
	}
}
void Manager::set(long double value_) {
	clear();
	if(value_ == 0) return;
	d_value = value_;
	c_type = VALUE_MANAGER;
}
void Manager::set(string var_) {
	clear();
	if(var_.empty()) return;
	s_var = var_;
	c_type = STRING_MANAGER;
}
void Manager::set(Function *f, ...) {
	clear();
	Manager *mngr;
	va_list ap;
	va_start(ap, f); 
	for(int i = 0; true; i++) {
		mngr = va_arg(ap, Manager*);
		if(mngr == NULL) break;
		mngrs.push_back(new Manager(mngr));
	}
	va_end(ap);	
	o_function = f;
	c_type = FUNCTION_MANAGER;
}
void Manager::addFunctionArg(Manager *mngr) {
	if(c_type == FUNCTION_MANAGER) {
		mngrs.push_back(new Manager(mngr));
	}
}
void Manager::set(Unit *u, long double value_) {
	clear();
	if(value_ == 0) return;
	if(value_ == 1) {
		o_unit = u;
		c_type = UNIT_MANAGER;
	} else {
		mngrs.push_back(new Manager(calc, value_));
		mngrs.push_back(new Manager(calc, u));
		c_type = MULTIPLICATION_MANAGER;
	}
}
void Manager::plusclean() {
	if(c_type != ADDITION_MANAGER) return;
	int i, i2;
	for(i = 0; i < ((int) mngrs.size()) - 1; i++) {
		if(mngrs[i]->type() == NULL_MANAGER) {
			mngrs[i]->unref();
			mngrs.erase(mngrs.begin() + i);				
			i--;
		} else {
			for(i2 = i + 1; i2 < mngrs.size(); i2++) {
				if(mngrs[i2]->type() == NULL_MANAGER) {
					mngrs[i2]->unref();
					mngrs.erase(mngrs.begin() + i2);				
					i2--;
				} else if(mngrs[i]->add(mngrs[i2], PLUS_CH, false)) {
					mngrs[i2]->unref();
					mngrs.erase(mngrs.begin() + i2);
					i = -1;
					break;
				}
			}
		}
	}
	if(mngrs.size() == 1) {
		moveto(mngrs[0]);
	} else if(mngrs.size() < 1) {
		clear();
	}
}
void Manager::multiclean() {
	if(c_type != MULTIPLICATION_MANAGER) return;
	for(int i = 0; i < ((int) mngrs.size()) - 1; i++) {
		for(int i2 = i + 1; i2 < mngrs.size(); i2++) {
			if(mngrs[i]->add(mngrs[i2], MULTIPLICATION_CH, false)) {
				mngrs[i2]->unref();
				mngrs.erase(mngrs.begin() + i2);
				i = -1;
				break;
			}
		}
	}
	if(mngrs.size() == 1) {
		moveto(mngrs[0]);
	} else if(mngrs.size() < 1) {	
		clear();
	}	
}
void Manager::powerclean() {
	if(c_type != POWER_MANAGER) return;
	if(mngrs[0]->add(mngrs[1], POWER_CH, false)) {
		mngrs[1]->unref();
		mngrs.erase(mngrs.begin() + 1);
	}
	if(mngrs.size() == 1) {
		moveto(mngrs[0]);
	} else if(mngrs.size() < 1) {
		clear();
	}
}
bool Manager::reverseadd(Manager *mngr, char sign, bool translate_) {
	Manager *mngr2 = new Manager(mngr);
	if(!mngr2->add(this, sign, translate_)) {
		mngr2->unref();
		return false;
	}
	moveto(mngr2);
	mngr2->unref();
	return true;
}
void Manager::transform(Manager *mngr, char type_, char sign, bool reverse_) {
	Manager *mngr2 = new Manager(this);
	clear();
	Manager *mngr3 = new Manager(mngr);
	if(reverse_ || sign == POWER_CH) {
		mngrs.push_back(mngr2);
		mngrs.push_back(mngr3);
	} else {
		mngrs.push_back(mngr3);
		mngrs.push_back(mngr2);
	}
	c_type = type_;
}


bool Manager::add(Manager *mngr, char sign, bool translate_) {
//	printf("[%s] %c [%s] (%i)\n", print().c_str(), sign, mngr->print().c_str(), translate_);
	if(mngr->type() == VALUE_MANAGER && (c_type == NULL_MANAGER || c_type == VALUE_MANAGER)) {
		add(mngr->d_value, sign);
		return true;
	}
	mngr->ref();
	if(sign == MINUS_CH) {
		sign = PLUS_CH;
		Manager *mngr2 = new Manager(mngr);
		mngr->unref();
		mngr2->add(-1, MULTIPLICATION_CH);
		mngr = mngr2;
	}
	if(sign == EXP_CH) {
		sign = MULTIPLICATION_CH;
		Manager *mngr2 = new Manager(calc, 10);
		mngr2->add(mngr, POWER_CH);
		mngr->unref();
		mngr = mngr2;
	}
	if(sign == DIVISION_CH) {
		if(mngr->c_type == 0) {
		 	calc->error(true, _("Trying to divide \"%s\" with zero."), print().c_str(), NULL);
			if(negative()) set(-INFINITY);
			else set(INFINITY);
			mngr->unref();
			return true;
		} else if(mngr->c_type == 'v' && mngr->d_value == 1) {
			mngr->unref();
			return true;
		}
		sign = MULTIPLICATION_CH;
		Manager *mngr2 = new Manager(mngr);
		mngr->unref();
		mngr2->add(-1, POWER_CH);
		mngr = mngr2;
	}

	if(sign == PLUS_CH) {
		if(mngr->c_type == 0) {
			mngr->unref(); 
			return true;
		}
		if(c_type == 0) {
			set(mngr);
			mngr->unref(); 
			return true;
		}		
		switch(c_type) {
			case PLUS_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {
						bool b = false;
						for(int i = 0; i < mngr->mngrs.size(); i++) {
							if(add(mngr->mngrs[i], sign, translate_)) b = true;
						}
						if(!b) {
							mngr->unref();
							return false;
						}
						break;
					}
					default: {
						Manager *mngr2 = new Manager(mngr);
						mngrs.push_back(mngr2);
						plusclean();
						break;
					}
				}
				break;
			}
			case MULTIPLICATION_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {
						if(!reverseadd(mngr, sign, translate_)) {
							mngr->unref(); 
							return false;
						}
						break;
					}
					case MULTIPLICATION_CH: {
						bool b = false;
						if(compatible(mngr)) {
							for(int i = 0; i < mngrs.size(); i++) {
								if(mngrs[i]->c_type == 'v') {
									for(int i2 = 0; i2 < mngr->mngrs.size(); i2++) {
										if(mngr->mngrs[i2]->c_type == 'v') {
											mngrs[i]->add(mngr->mngrs[i2], sign);
											b = true;
											break;
										}
									}
									if(!b) {
										mngrs[i]->add(1, sign);
										b = true;
									}
									if(mngrs[i]->c_type == 0) {
										clear();
									} else if(mngrs[i]->d_value == 1) {
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
								for(int i2 = 0; i2 < mngr->mngrs.size(); i2++) {
									if(mngr->mngrs[i2]->c_type == 'v') {
										if(mngr->mngrs[i2]->d_value == -1) {
											clear();
										} else {
											mngrs.push_back(new Manager(calc, 1));
											mngrs[mngrs.size() - 1]->add(mngr->mngrs[i2], sign);
										}
										b = true;
										break;
									}
								}
							}
							if(!b) {
								mngrs.push_back(new Manager(calc, 2));
							}
							break;							
						}
						if(!translate_) {
							mngr->unref(); 
							return false;
						}
						transform(mngr, PLUS_CH, sign);
						break;
					}
					case 'u': {
					}
					case POWER_CH: {
					}
					case FUNCTION_MANAGER: {
					}
					case 's': {
						if(compatible(mngr)) {
							bool b = false;
							for(int i = 0; i < mngrs.size(); i++) {
								if(mngrs[i]->c_type == 'v') {
									mngrs[i]->add(1, sign);
									if(mngrs[i]->c_type == 0) {
										clear();
									} else if(mngrs[i]->d_value == 1) {
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
								mngrs.push_back(new Manager(calc, 2));
							}
							break;
						}					
					}					
					case 'v': {
						if(!translate_) {
							mngr->unref(); 
							return false;
						}
						transform(mngr, PLUS_CH, sign);
						break;
					}
				}
				break;
			}
			case POWER_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case POWER_CH: {
						if(equal(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(new Manager(calc, 2));
							mngrs.push_back(mngr2);
							c_type = MULTIPLICATION_CH;
							break;
						}					
					}
					default: {
						if(!translate_) {
							mngr->unref(); 
							return false;
						}
						transform(mngr, PLUS_CH, sign);
						break;
					}
				}
				break;
			}
			case 0: {
				set(mngr);
				break;
			}
			case 'v': {
				switch(mngr->c_type) {
					case PLUS_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
					}
					case 'v': {
						d_value += mngr->d_value;
						if(d_value == 0) c_type = 0;
						break;
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, PLUS_CH, sign, true);
						break;
					}					
				}
				break;
			}
			case 'u': {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case 'u': {
						if(equal(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(new Manager(calc, 2));
							mngrs.push_back(mngr2);
							c_type = MULTIPLICATION_CH;
							break;
						}					
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, PLUS_CH, sign);
						break;
					}
				}
				break;
			}
			case 's': {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case 's': {
						if(s_var == mngr->s_var) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(new Manager(calc, 2));
							mngrs.push_back(mngr2);
							c_type = MULTIPLICATION_CH;
							break;
						}
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, PLUS_CH, sign);
						break;
					}
				}
				break;
			}
			case FUNCTION_MANAGER: {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case FUNCTION_MANAGER: {
						if(equal(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(new Manager(calc, 2));
							mngrs.push_back(mngr2);
							c_type = MULTIPLICATION_CH;
							break;
						}
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, PLUS_CH, sign);
						break;
					}
				}
				break;			
			}
		}
	} else if(sign == MULTIPLICATION_CH) {
		if(mngr->c_type == 0) {
			clear(); mngr->unref(); return true;
		} else if(mngr->c_type == 'v' && mngr->d_value == 1.0L) {
			mngr->unref();
			return true;
		}
		switch(c_type) {
			case PLUS_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {
						Manager *mngr3 = new Manager(this);
						clear();
						c_type = PLUS_CH;
						for(int i = 0; i < mngr->mngrs.size(); i++) {
							Manager *mngr2 = new Manager(mngr3);
							mngr2->add(mngr->mngrs[i], sign);
							mngrs.push_back(mngr2);
						}
						mngr3->unref();
						break;
					}
					default: {
						for(int i = 0; i < mngrs.size(); i++) {
							mngrs[i]->add(mngr, sign);
						}
						break;
					}
				}
				plusclean();
				break;
			}
			case MULTIPLICATION_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case MULTIPLICATION_CH: {
						for(int i = 0; i < mngr->mngrs.size(); i++) {
							add(mngr->mngrs[i], sign);
						}
						multiclean();
						break;
					}
					default: {
						Manager *mngr2 = new Manager(mngr);
						mngrs.push_back(mngr2);
						multiclean();
						break;
					}
				}
				break;
			}
			case POWER_CH: {
				switch(mngr->c_type) {
					case MULTIPLICATION_CH: {}
					case PLUS_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case POWER_CH: {
						if(mngr->mngrs[0]->equal(mngrs[0])) {
							mngrs[1]->add(mngr->mngrs[1], PLUS_CH);
							if(mngrs[1]->c_type == 0) {
								set(1);
							} else if(mngrs[1]->c_type == 'v' && mngrs[1]->d_value == 1) {
								moveto(mngrs[0]);
							}
						} else {
							if(!translate_) {mngr->unref(); return false;}
							transform(mngr, MULTIPLICATION_CH, sign);
						}
						break;
					}
					default: {
						if(mngr->equal(mngrs[0])) {
							mngrs[1]->add(1, PLUS_CH);
							if(mngrs[1]->c_type == 0) {
								set(1);
							} else if(mngrs[1]->c_type == 'v' && mngrs[1]->d_value == 1) {
								moveto(mngrs[0]);
							}
						} else {
							if(!translate_) {mngr->unref(); return false;}
							transform(mngr, MULTIPLICATION_CH, sign);
						}
						break;
					}
				}
				break;
			}
			case 0: {
				break;
			}
			case 'v': {
				if(d_value == 1.0L) {set(mngr); mngr->unref(); return true;}
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case 'v': {
						d_value *= mngr->d_value;
						break;
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, MULTIPLICATION_CH, sign, true);
						break;
					}
				}
				break;
			}
			case 'u': {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case 'u': {
						if(o_unit == mngr->o_unit) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(mngr2);
							mngrs.push_back(new Manager(calc, 2));
							c_type = POWER_CH;
							break;
						}
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, MULTIPLICATION_CH, sign);
						break;
					}
				}
				break;
			}
			case 's': {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case 's': {
						if(s_var == mngr->s_var) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(mngr2);
							mngrs.push_back(new Manager(calc, 2));
							c_type = POWER_CH;
							break;
						}
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, MULTIPLICATION_CH, sign);
						break;
					}
				}
				break;
			}
			case FUNCTION_MANAGER: {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
						break;
					}
					case FUNCTION_MANAGER: {
						if(equal(mngr)) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(mngr2);
							mngrs.push_back(new Manager(calc, 2));
							c_type = POWER_CH;
							break;
						}
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, MULTIPLICATION_CH, sign);
						break;
					}
				}
				break;
			}			
		}
	} else if(sign == POWER_CH) {
		if(mngr->c_type == 0) {
			if(c_type == 0) {
				calc->error(false, _("0^0 is undefined"), NULL);
				if(!translate_) {mngr->unref(); return false;}
				transform(mngr, POWER_CH, sign);
			} else {
				set(1);
				mngr->unref(); return true;
			}
		} else if(mngr->c_type == 'v' && mngr->d_value == 1) {mngr->unref(); return true;}
		switch(c_type) {
			case MULTIPLICATION_CH: {
				for(int i = 0; i < mngrs.size(); i++) {
					mngrs[i]->add(mngr, sign);
				}
				break;
			}		
			case PLUS_CH: {
				switch(mngr->c_type) {
					case 'v': {
						if(mngr->d_value != -1 && fmodl(mngr->d_value, 1) == 0) {
							long double d = mngr->d_value;
							if(d < 0) d = -d;
							Manager *mngr2 = new Manager(this);
							for(long double i = 1; i < d; i++) {
								add(mngr2, MULTIPLICATION_CH);
							}
							if(mngr->d_value < 0) {
								mngr2->unref();
								mngr2 = new Manager(calc, 1);
								mngr2->add(this, DIVISION_CH);
								moveto(mngr2);
								mngr2->unref();
							}
							break;
						}
					}
					default: {
						if(!translate_) {mngr->unref(); return false;}	
						transform(mngr, POWER_CH, sign);
						break;
					}
				}
				break;
			}
			case POWER_CH: {
				switch(mngr->c_type) {
					default: {
						mngrs[1]->add(mngr, MULTIPLICATION_CH);
						if(mngrs[1]->c_type == 0) {
							set(1);
						} else if(mngrs[1]->c_type == 'v' && mngrs[0]->c_type == 'v') {
							mngrs[0]->add(mngrs[1], POWER_CH);
							moveto(mngrs[0]);
						} else if(mngrs[1]->c_type == 'v' && mngrs[1]->d_value == 1) {
							moveto(mngrs[0]);
						}
						break;
					}
				}
				break;
			}
			case 0: {
				if(mngr->c_type == 'v' && mngr->d_value < 0) {
					if(!translate_) {mngr->unref(); return false;}
					if(mngr->d_value == -1) transform(mngr, POWER_CH, sign);
					else {
						Manager *mngr2 = new Manager(calc, -1);
						transform(mngr2, POWER_CH, sign);
						mngr2->unref();
					}
				}
				break;
			}
			case 'v': {
				if(d_value == 1) {mngr->unref(); return true;}
				switch(mngr->c_type) {
					case 'v': {
						d_value = powl(d_value, mngr->d_value);
						break;
					}				
					default: {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, POWER_CH, sign);
						break;
					}
				}
				break;
			}
			default: {
				if(!translate_) {mngr->unref(); return false;}
				transform(mngr, POWER_CH, sign);
				break;
			}
		}
	}
	sort();
	calc->checkFPExceptions();
	mngr->unref();
	return true;
}
void Manager::add(Unit *u, char sign) {
	Manager *mngr = new Manager(calc, u);
	mngr->finalize();
	add(mngr, sign);
	mngr->unref();
}
int Manager::compare(Manager *mngr) {
	if(c_type != mngr->type()) {
		if(mngr->type() == ADDITION_MANAGER) return -mngr->compare(this);	
		if(mngr->type() == MULTIPLICATION_MANAGER && c_type != ADDITION_MANAGER) return -mngr->compare(this);		
		if(mngr->type() == POWER_MANAGER && c_type != ADDITION_MANAGER && c_type != MULTIPLICATION_MANAGER) return -mngr->compare(this);		
	}
	switch(c_type) {
		case NULL_MANAGER: {
			if(mngr->type() == NULL_MANAGER) return 0;
			if(mngr->type() == FUNCTION_MANAGER) return -1;					
			return 1;
		} 
		case VALUE_MANAGER: {
			if(mngr->type() == NULL_MANAGER) return -1;
			if(mngr->type() == VALUE_MANAGER) {
				if(d_value == mngr->value()) return 0;
				if(d_value > 0 && d_value < mngr->value()) return 1;
				return -1;
			}
			if(mngr->type() == FUNCTION_MANAGER) return -1;
			return 1;
		} 
		case UNIT_MANAGER: {
			if(mngr->type() == UNIT_MANAGER) {
				if(o_unit->shortName() < mngr->o_unit->shortName()) return -1;
				if(o_unit->shortName() == mngr->o_unit->shortName()) return 0;
				return 1;
			}
			if(mngr->type() == VALUE_MANAGER || mngr->type() == NULL_MANAGER || mngr->type() == FUNCTION_MANAGER) return -1;
			return 1;
		}
		case STRING_MANAGER: {
			if(mngr->type() == STRING_MANAGER) {
				if(s_var < mngr->s_var) return -1;
				else if(s_var == mngr->s_var) return 0;
				else return 1;
			}
			if(mngr->type() == VALUE_MANAGER || mngr->type() == NULL_MANAGER || mngr->type() == UNIT_MANAGER || mngr->type() == FUNCTION_MANAGER) return -1;
			return 1;
		}
		case FUNCTION_MANAGER: {
			if(mngr->type() == FUNCTION_MANAGER) {
				if(o_function->name() < mngr->o_function->name()) return -1;
				if(o_function->name() == mngr->o_function->name()) {
					for(int i = 0; i < mngr->mngrs.size(); i++) {
						if(i >= mngrs.size()) {
							return -1;	
						}
						int i2 = mngr->mngrs[i]->compare(mngrs[i]);
						if(i2 != 0) return i2;
					}
					return 0;
				} else return 1;
			}
			return 1;
		}
		case POWER_MANAGER: {
			if(mngr->type() == POWER_MANAGER) {
				if(mngrs[1]->negative() && !mngr->mngrs[1]->negative()) return 1;
				if(!mngrs[1]->negative() && mngr->mngrs[1]->negative()) return -1;				
				int i = mngrs[0]->compare(mngr->mngrs[0]);
				if(i == 0) {
					return mngrs[1]->compare(mngr->mngrs[1]);
				} else {
					return i;
				}
			} else if(mngr->type() != ADDITION_MANAGER) {
				if(mngrs[1]->negative()) return 1;
				return mngrs[0]->compare(mngr);
			}
			return 1;
		}
		case MULTIPLICATION_MANAGER: {
			if(mngrs.size() < 1) return 1;
			if(mngr->type() == VALUE_MANAGER || mngr->type() == NULL_MANAGER) return -1;
			int start = 0;
			if(mngrs[0]->type() == VALUE_MANAGER && mngrs.size() > 1) start = 1;
			if(mngr->mngrs.size() < 1 || mngr->type() == POWER_MANAGER) return mngrs[start]->compare(mngr);
			if(mngr->type() == MULTIPLICATION_MANAGER) {
				if(mngr->mngrs.size() < 1) return -1;
				int mngr_start = 0;
				if(mngr->mngrs[0]->type() == VALUE_MANAGER && mngr->mngrs.size() > 1) mngr_start = 1;			
				for(int i = 0; ; i++) {
					if(i >= mngr->mngrs.size() - mngr_start && i >= mngrs.size() - start) return 0;
					if(i >= mngr->mngrs.size() - mngr_start) return 1;
					if(i >= mngrs.size() - start) return -1;
					int i2 = mngrs[i + start]->compare(mngr->mngrs[i + mngr_start]);
					if(i2 != 0) return i2;
				}
			} 
			if(mngr->type() == ADDITION_MANAGER) return 1;
			return -1;
		} 
		case ADDITION_MANAGER: {		
			if(mngrs.size() < 1) return 1;
			if(mngr->type() == VALUE_MANAGER || mngr->type() == NULL_MANAGER) return -1;
			if(mngr->type() == ADDITION_MANAGER) {
				if(mngr->mngrs.size() < 1) return -1;
				for(int i = 0; ; i++) {
					if(i >= mngr->mngrs.size() && i >= mngrs.size()) return 0;
					if(i >= mngr->mngrs.size()) return 1;
					if(i >= mngrs.size()) return -1;
					int i2 = mngrs[i]->compare(mngr->mngrs[i]);
					if(i2 != 0) return i2;
				}
			} 
			return mngrs[0]->compare(mngr);
		}
	}
	return 1;
}

void Manager::sort() {
	if(c_type == POWER_MANAGER || mngrs.size() < 2) return;
	vector<Manager*> sorted;
	bool b;
	for(int i = 0; i < mngrs.size(); i++) {
		b = false;
		if(c_type == MULTIPLICATION_MANAGER && (mngrs[i]->type() == VALUE_MANAGER || mngrs[i]->type() == NULL_MANAGER)) {
			sorted.insert(sorted.begin(), mngrs[i]);	
		} else {		
			for(int i2 = 0; i2 < sorted.size(); i2++) {
				if(!(c_type == MULTIPLICATION_MANAGER && sorted[i2]->type() == VALUE_MANAGER) && mngrs[i]->compare(sorted[i2]) < 0) {
					sorted.insert(sorted.begin() + i2, mngrs[i]);
					b = true;
					break;
				}
			}
			if(!b) sorted.push_back(mngrs[i]);
		}
	}
	for(int i2 = 0; i2 < sorted.size(); i2++) {
		mngrs[i2] = sorted[i2];
	}
}
void Manager::moveto(Manager *term) {
	term->ref();
	clear();
	d_value = term->d_value;
	o_unit = term->o_unit;
	s_var = term->s_var;
	o_function = term->o_function;
	for(int i = 0; i < term->mngrs.size(); i++) {
		term->mngrs[i]->ref();
		mngrs.push_back(term->mngrs[i]);
	}
	c_type = term->c_type;
	term->unref();
}
bool Manager::equal(Manager *mngr) {
	if(c_type == mngr->c_type) {
		if(c_type == 0) {
			return true;
		} if(mngr->c_type == 'v') {
			return mngr->d_value == d_value;
		} else if(c_type == 'u') {
			return o_unit == mngr->o_unit;
		} else if(c_type == 's') {
			return s_var == mngr->s_var;
		} else if(c_type == FUNCTION_MANAGER) {
			if(o_function != mngr->o_function) return false;
			if(mngrs.size() != mngr->mngrs.size()) return false;			
			for(int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equal(mngr->mngrs[i])) return false;
			}			
			return true;
		} else if(c_type == MULTIPLICATION_CH) {
			if(mngrs.size() != mngr->mngrs.size()) return false;
			for(int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equal(mngr->mngrs[i])) return false;
			}
			return true;
		} else if(c_type == PLUS_CH) {
			if(mngrs.size() != mngr->mngrs.size()) return false;
			for(int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equal(mngr->mngrs[i])) return false;
			}
			return true;
		} else if(c_type == POWER_CH) {
			return mngrs[0]->equal(mngr->mngrs[0]) && mngrs[1]->equal(mngr->mngrs[1]);
		}
	}
	return false;
}
bool Manager::mergable(Manager *mngr, char type_) {
	switch(type_) {
		case PLUS_CH: {
			if(c_type == 'v' || c_type == 'u' || c_type == 's') return compatible(mngr);
			break;
		}
		case MULTIPLICATION_CH: {
			if(c_type == 'v' || c_type == 'u' || c_type == 's') return compatible(mngr);
			break;
		}
		case POWER_CH: {
			return true;
			break;
		}
	}
	return false;
}
bool Manager::compatible(Manager *mngr) {
	if(c_type == NULL_MANAGER || mngr->type() == NULL_MANAGER) return true;
	if(c_type == mngr->type()) {
		if(c_type == 'v') {
			return true;
		} else if(c_type == 'u') {
			if(o_unit == mngr->o_unit) return true;
		} else if(c_type == 's') {
			if(s_var == mngr->s_var) return true;
		} else if(c_type == FUNCTION_MANAGER) {
			return equal(mngr);
		} else if(c_type == MULTIPLICATION_CH) {
			if(mngrs[0]->c_type == 'v' && mngr->mngrs[0]->c_type != 'v') {
				if(mngrs.size() != mngr->mngrs.size() + 1) return false;
				for(int i = 1; i < mngrs.size(); i++) {
					if(!mngrs[i]->compatible(mngr->mngrs[i - 1])) return false;
				}
			} else if(mngrs[0]->c_type != 'v' && mngr->mngrs[0]->c_type == 'v') {
				if(mngrs.size() + 1 != mngr->mngrs.size()) return false;
				for(int i = 1; i < mngr->mngrs.size(); i++) {
					if(!mngrs[i - 1]->compatible(mngr->mngrs[i])) return false;
				}
			} else {
				if(mngrs.size() != mngr->mngrs.size()) return false;
				for(int i = 0; i < mngrs.size(); i++) {
					if(!mngrs[i]->compatible(mngr->mngrs[i])) return false;
				}
			}
			return true;
		} else if(c_type == PLUS_CH) {
			if(mngrs.size() != mngr->mngrs.size()) return false;
			for(int i = 0; i < mngrs.size(); i++) {
				if(!mngrs[i]->equal(mngr->mngrs[i])) return false;
			}
			return true;
		} else if(c_type == POWER_CH) {
			return mngrs[0]->equal(mngr->mngrs[0]) && mngrs[1]->equal(mngr->mngrs[1]);
		}
	} else if(c_type == MULTIPLICATION_CH) {
		if(mngr->type() != VALUE_MANAGER) {
			if(mngrs.size() != 2) return false;
			if(mngrs[0]->type() != VALUE_MANAGER) return false;			
			if(!mngrs[1]->compatible(mngr)) return false;
			return true;
		}
	}
	return false;
}
void Manager::add(long double value_, char sign) {
	if(value_ == 0.0L && sign == DIVISION_CH) {
	 	calc->error(true, _("Trying to divide \"%s\" with zero."), print().c_str());
		if(negative()) set(-INFINITY);
		else set(INFINITY);		
	} else if(c_type == 0 && value_ != 0 && (sign != POWER_CH || value_ > 0)) {
		switch(sign) {
			case PLUS_CH: {d_value = value_; c_type = 'v'; break;}
			case MINUS_CH: {d_value = -value_; c_type = 'v'; break;}
		}
		calc->checkFPExceptions();
	} else if(c_type == 'v') {
		switch(sign) {
			case PLUS_CH: {d_value += value_; break;}
			case MINUS_CH: {d_value -= value_; break;}
			case MULTIPLICATION_CH: {d_value *= value_; break;}
			case DIVISION_CH: {d_value /= value_; break;}
			case EXP_CH: {d_value *= exp10l(value_); break;}
			case POWER_CH: {d_value = powl(d_value, value_); break;}
		}
		if(d_value == 0) c_type = 0;
		calc->checkFPExceptions();
	} else {
		Manager *mngr = new Manager(calc, value_);
		add(mngr, sign);
		mngr->unref();
	}
}
void Manager::clear() {
	d_value = 0;
	o_unit = NULL;
	s_var = "";
	o_function = NULL;
	for(int i = 0; i < mngrs.size(); i++) {
		mngrs[i]->unref();
	}
	mngrs.clear();
	c_type = NULL_MANAGER;
}
long double Manager::value() {
	return d_value;
}
void Manager::value(long double value_) {
	set(value_);
}
Unit *Manager::unit() {
	return o_unit;
}
void Manager::unit(Unit *u, long double value_) {
	set(u, value_);
}
bool Manager::negative() {
	if(c_type == 0) return false;
	else if(c_type == 'v') return d_value < 0;
	else if(c_type == MULTIPLICATION_CH || c_type == POWER_CH) return mngrs[0]->negative();
	return false;
}
string Manager::print(NumberFormat nrformat, int unitflags, int precision, int decimals_to_keep, bool decimals_expand, bool decimals_decrease, bool *usable, long double prefix_, bool toplevel, bool *plural, long double *d_exp, bool in_composite, bool in_power) {
	string str, str2;
	if(toplevel && (unitflags & UNIT_FORMAT_TAGS)) {
	    str = "<b><big>";
	}
	else str = "";
	if(c_type == 'v') {
		long double new_value = d_value;
		switch(nrformat) {
			case NUMBER_FORMAT_DECIMALS: {
				str2 = calc->value2str_decimals(d_value, precision);
				break;
			}
			case NUMBER_FORMAT_EXP: {
				str2 = calc->value2str_exp(d_value, precision);
				break;
			}
			case NUMBER_FORMAT_EXP_PURE: {
				str2 = calc->value2str_exp_pure(d_value, precision);
				break;
			}
			case NUMBER_FORMAT_PREFIX: {
//				if(toplevel || d_exp) {
				if(d_exp) {
//					if(d_exp) {
					str2 = calc->value2str_prefix(d_value, *d_exp, precision, unitflags & UNIT_FORMAT_SHORT, &new_value, prefix_, !in_composite);
//					} else {
//						long double exp_value = 1.0L;
//						str2 = calc->value2str_prefix(d_value, exp_value, precision, unitflags & UNIT_FORMAT_SHORT, &new_value);
//					}
					if((unitflags & UNIT_FORMAT_SHORT) && (unitflags & UNIT_FORMAT_NONASCII)) gsub("micro", SIGN_MICRO, str2);
					break;
				}
			}
			case NUMBER_FORMAT_NORMAL: {
				str2 = calc->value2str(d_value, precision);
				break;
			}			
			case NUMBER_FORMAT_HEX: {
				str2 = calc->value2str_hex(d_value, precision);
				break;
			}
			case NUMBER_FORMAT_OCTAL: {
				str2 = calc->value2str_octal(d_value, precision);
				break;
			}
			case NUMBER_FORMAT_BIN: {
				str2 = calc->value2str_bin(d_value, precision);
				break;
			}			
		}	
		bool minus = false;
		if(str2.substr(0, strlen(MINUS_STR)) == MINUS_STR) {
			str2 = str2.substr(strlen(MINUS_STR), str2.length() - strlen(MINUS_STR));
			minus = true;
		}
		if(!in_composite) calc->remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
		if(minus) {
			if(unitflags & UNIT_FORMAT_NONASCII) {
				str2.insert(0, SIGN_MINUS);
			} else {
				str2.insert(0, MINUS_STR);
			}
		}
		str += str2;
		if(unitflags & UNIT_FORMAT_TAGS) {
			int i = str.find("E");
			if(i != string::npos && i != str.length() - 1 && (is_in(str[i + 1], PLUS_S, MINUS_S, NUMBERS_S, NULL))) {
				str.replace(i, 1, "<small>E</small>");
			}
		}
		if(plural) *plural = (new_value > 1 || new_value < -1);
	} else if(c_type == 'u') {
		if(!in_composite && toplevel) {
			str2 = "1";
			calc->remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
			str += str2; str += " ";
		}
		if(o_unit->type() == 'D') {
			Manager *mngr = ((CompositeUnit*) o_unit)->generateManager(false);
			str2 = mngr->print(NUMBER_FORMAT_PREFIX, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, NULL, NULL, true, in_power);		
//			gsub("1 ", "", str2);
//			if(str2.substr(0, 2) == "1 ") str2 = str2.substr(2, str2.length() - 2);
			str += str2;
			mngr->unref();
		} else {
			if(!(unitflags & UNIT_FORMAT_LONG)) {
				if(unitflags & UNIT_FORMAT_NONASCII) {
						if(o_unit->name() == "euro") str += SIGN_EURO;
						else if(o_unit->shortName() == "oC") str += SIGN_POWER_0 "C";
						else if(o_unit->shortName() == "oF") str += SIGN_POWER_0 "F";
						else if(o_unit->shortName() == "oR") str += SIGN_POWER_0 "R";
						else str += o_unit->shortName(plural && *plural);
				} else {
					str += o_unit->shortName(plural && *plural);
				}
			} else if(plural && *plural) str += o_unit->plural();
			else str += o_unit->name();
		}
	} else if(c_type == 's') {
		if(unitflags & UNIT_FORMAT_NONASCII) {
			if(s_var == "pi") str += SIGN_PI;
			else str += s_var;
		} else {
			str += s_var;
		}
		if(plural) *plural = true;
	} else if(c_type == FUNCTION_MANAGER) {
		str += o_function->name();
		str += LEFT_BRACKET_STR;
		for(int i = 0; i < mngrs.size(); i++) {
			if(i > 0) {
				str += COMMA_STR;
				str += SPACE_STR;
			}
			str += mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, NULL, d_exp, in_composite, in_power);
		}
		str += RIGHT_BRACKET_STR;		
	} else if(c_type == PLUS_CH) {
		for(int i = 0; i < mngrs.size(); i++) {
			if(i > 0) {
				str += " ";			
				if(unitflags & UNIT_FORMAT_NONASCII) {
					if(mngrs[i]->negative()) str += SIGN_MINUS;
					else str += SIGN_PLUS;
				} else {
					if(mngrs[i]->negative()) str += MINUS_STR;
					else str += PLUS_STR;
				}
				str += " ";				
			}
			d_exp = NULL;
/*			if(mngrs[i]->type() == VALUE_MANAGER) {
				long double exp_value = 1.0L;
				d_exp = &exp_value;
			}*/
			if(toplevel && !in_composite && (mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u'))) {
				str2 = "1";
				calc->remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
				str += str2; str += " ";
			}
			str2 = mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, NULL, d_exp, in_composite, in_power);
			if(i > 0 && unitflags & UNIT_FORMAT_NONASCII && str2.substr(0, strlen(SIGN_MINUS)) == SIGN_MINUS) {
				str2 = str2.substr(strlen(SIGN_MINUS), str2.length() - strlen(SIGN_MINUS));
			} else if(i > 0 && str2.substr(0, strlen(MINUS_STR)) == MINUS_STR) {
				str2 = str2.substr(strlen(MINUS_STR), str2.length() - strlen(MINUS_STR));
			}
			str += str2;
		}
	} else if(c_type == MULTIPLICATION_CH) {
		bool b = false, c = false;
		bool plural_ = true;
		int prefix = 0;
		bool had_unit = false, had_div_unit = false, is_unit = false;
		for(int i = 0; i < mngrs.size(); i++) {
			is_unit = false;
			if(mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u')) {
				is_unit = true;
			} else if(mngrs[i]->c_type == POWER_CH) {
				for(int i2 = 0; i2 < mngrs[i]->mngrs[0]->mngrs.size(); i2++) {
					if(mngrs[i]->mngrs[0]->mngrs[i2]->c_type == 'u') {
						is_unit = true;
						break;
					}
				}
			}		
			d_exp = NULL;
			if(prefix) prefix--;
			if(i == 0 && mngrs[i]->type() == VALUE_MANAGER && mngrs[i]->value() == -1.0L && mngrs.size() > 1 && mngrs[1]->type() != 'u' && (mngrs[1]->type() != UNIT_MANAGER || mngrs[1]->mngrs[0]->type() != UNIT_MANAGER)) {
				if(unitflags & UNIT_FORMAT_NONASCII) {
					str += SIGN_MINUS;
				} else {
					str += MINUS_STR;
				}
				i++;
			}
			if(mngrs[i]->type() == VALUE_MANAGER && mngrs.size() >= i + 2) {
				if(mngrs[i + 1]->type() == POWER_MANAGER) {
					if(mngrs[i + 1]->mngrs[1]->type() == VALUE_MANAGER && mngrs[i + 1]->mngrs[0]->type() == UNIT_MANAGER) {
						long double exp_value = mngrs[i + 1]->mngrs[1]->value();
						d_exp = &exp_value;										
						prefix = 2;
					}				
				} else if(mngrs[i + 1]->type() == UNIT_MANAGER && mngrs[i + 1]->o_unit->type() != 'D') {
					long double exp_value = 1.0L;
					d_exp = &exp_value;
					prefix = 2;						
				}
			}
//			if(!in_composite && i == 0 && (mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == 's' && mngrs[i]->s_var.length() > 1) || (mngrs[i]->c_type == POWER_CH && (mngrs[i]->mngrs[0]->c_type == 'u' || (mngrs[i]->mngrs[0]->c_type == 's' && mngrs[i]->mngrs[0]->s_var.length() > 1))))) {
			if(!in_composite && i == 0 && (mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u'))) {
				str2 = "1";
				calc->remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
				str += str2; str += " ";
			}
			if(!(unitflags & UNIT_FORMAT_SCIENTIFIC) && i > 0 && mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[1]->negative()) {
				Manager *mngr = new Manager(mngrs[i]);
				mngr->add(-1, POWER_CH);
				if(!b) {
					if(unitflags & UNIT_FORMAT_NONASCII) {
						str += " ";
						str += SIGN_DIVISION;
						str += " ";
					} else {
						str += DIVISION_STR;
					}
				}
				if(!b && (i < mngrs.size() - 1 || mngr->type() == ADDITION_MANAGER) && (i + 1 != mngrs.size() - 1 || mngr->type() != VALUE_MANAGER || mngr->type() == ADDITION_MANAGER)) {
					c = true;
					str += LEFT_BRACKET_STR;
				}
				if(b && is_unit)  {
					if(!prefix || is_in(str[str.length() - 1], NUMBERS_S, MINUS_S, NULL)) {
						str += " ";
						if(had_div_unit) {
							if(unitflags & UNIT_FORMAT_NONASCII) {
								str += SIGN_MULTIDOT;
							} else {
								str += MULTIPLICATION_STR;
							}
							str += " ";					
						}						
					}
				} else 	if(had_div_unit) {
					if(unitflags & UNIT_FORMAT_NONASCII) {
						str += SIGN_MULTIDOT;
					} else {
						str += MULTIPLICATION_STR;
					}
					str += " ";					
				}						
				if(is_unit) {
					had_div_unit = true;
				}								
				str += mngr->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, &plural_, d_exp, in_composite, in_power);
				if(c && i == mngrs.size() - 1) {
					str += RIGHT_BRACKET_STR;
				}
				b = true;
//			} else if(mngrs[i]->c_type == PLUS_CH || ((mngrs[i]->c_type == POWER_CH && i == 0) || (i > 0 && mngrs[i - 1]->c_type == POWER_CH))) {
			} else if(mngrs[i]->type() == ADDITION_MANAGER) {
				str += " ";
				if(unitflags & UNIT_FORMAT_NONASCII) {
					str += SIGN_MULTIDOT;
				} else {
					str += MULTIPLICATION_STR;
				}
				str += " ";								
				str += LEFT_BRACKET_STR;
				str += mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, &plural_, d_exp, in_composite, in_power);
				str += RIGHT_BRACKET_STR;
			} else {
				if(mngrs[i]->type() == POWER_MANAGER && (mngrs[i]->mngrs[0]->type() == VALUE_MANAGER || had_unit)) {
					str += " ";
					if(unitflags & UNIT_FORMAT_NONASCII) {
						str += SIGN_MULTIDOT;
					} else {
						str += MULTIPLICATION_STR;
					}
					str += " ";												
				} else if(i > 0 && (mngrs[i]->type() == FUNCTION_MANAGER || mngrs[i - 1]->type() == FUNCTION_MANAGER)) {
					str += " ";
					if(unitflags & UNIT_FORMAT_NONASCII) {
						str += SIGN_MULTIDOT;
					} else {
						str += MULTIPLICATION_STR;
					}
					str += " ";								
				} else if(i > 0 && is_unit)  {
					if(!prefix || is_in(str[str.length() - 1], NUMBERS_S, NULL)) {
						str += " ";
						if(had_unit) {
							if(unitflags & UNIT_FORMAT_NONASCII) {
								str += SIGN_MULTIDOT;
							} else {
								str += MULTIPLICATION_STR;
							}
							str += " ";					
						}
					}
				} else if(i > 0 && mngrs[i]->c_type == 's' && mngrs[i]->s_var.length() > 1 || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 's' && mngrs[i]->mngrs[0]->s_var.length() > 1)) {
					str += " ";
					if(i > 1) {
						if(unitflags & UNIT_FORMAT_NONASCII) {
							str += SIGN_MULTIDOT;
						} else {
							str += MULTIPLICATION_STR;
						}
						str += " ";					
					}
				} else if(had_unit && mngrs[i]->c_type == 'v') {
					str += " ";
					if(unitflags & UNIT_FORMAT_NONASCII) {
						str += SIGN_MULTIDOT;
					} else {
						str += MULTIPLICATION_STR;
					}
					str += " ";					
				}					
				str += mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, &plural_, d_exp, in_composite, in_power);
			}
			if(plural_ && (mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u'))) {
				plural_ = false;
			}
			if(is_unit) {
				had_unit = true;
			}
		}
	} else if(c_type == POWER_CH) {
		if(!in_composite && toplevel && mngrs[0]->c_type == 'u') {
			str2 = "1";
			calc->remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
			str += str2; str += " ";
		}
		if(mngrs[0]->mngrs.size() > 0 && !in_composite) {
			str += LEFT_BRACKET_STR;
			str += mngrs[0]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, NULL, NULL, in_composite, true);
			str += RIGHT_BRACKET_STR;
		} else {
			str += mngrs[0]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, NULL, NULL, in_composite, true);
		}
//		if(unitflags & UNIT_FORMAT_NONASCII && mngrs[1]->c_type == 'v' && mngrs[1]->d_value == 2) {
//			str += SIGN_POWER_2;
//		} else if(unitflags & UNIT_FORMAT_NONASCII && mngrs[1]->c_type == 'v' && mngrs[1]->d_value == 3) {
//			str += SIGN_POWER_3;
//		} else {
/*			int i2 = 0;
			if(unitflags & UNIT_FORMAT_TAGS) {
				int i = 0;
				while((i = str.find("<big>", i)) != string::npos) {
					i2++;
					i += 5;
					i = str.find("</big>", i);	
					if(i == string::npos) break;
					i2--;
					i += 6;
				}
			}*/
			if(unitflags & UNIT_FORMAT_TAGS) {
				if(!in_power) {
					str += "</big>";
					str += "<sup>";
				} else {
					str += POWER_STR;
				}
			}
			else str += POWER_STR;
			if(mngrs[1]->mngrs.size() > 0) {
				str += LEFT_BRACKET_STR;
				str += mngrs[1]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, NULL, NULL, in_composite, true);
				str += RIGHT_BRACKET_STR;
			} else {
				str += mngrs[1]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, prefix_, false, NULL, NULL, in_composite, true);
			}
			if(unitflags & UNIT_FORMAT_TAGS) {
				if(!in_power) {
					str += "</sup>";
					str += "<big>";
				}
			}
//		}
	} else {
		str2 = "0";
		calc->remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
		str += str2;
	}
	if(toplevel && (unitflags & UNIT_FORMAT_TAGS)) str += "</big></b>";
	return str;
}
bool Manager::testDissolveCompositeUnit(Unit *u) {
	if(c_type == UNIT_MANAGER) {
		if(o_unit->type() == 'D') {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				Manager *mngr = ((CompositeUnit*) o_unit)->generateManager();
				moveto(mngr);
				mngr->unref();
				return true;
			}
		} else if(o_unit->type() == 'A' && o_unit->baseUnit()->type() == 'D') {
			if(((CompositeUnit*) (o_unit->baseUnit()))->containsRelativeTo(u)) {
/*				Manager *mngr = o_unit->baseUnit()->convert(o_unit);
				Manager *mngr2 = ((CompositeUnit*) o_unit->baseUnit())->generateManager();
				moveto(mngr2);
				mngr2->unref();
				add(mngr, MULTIPLICATION_CH);
				mngr->unref();*/
				convert(o_unit->baseUnit());
				convert(u);
				return true;
			}		
		}
	}
	return false; 
}
bool Manager::testCompositeUnit(Unit *u) {
	if(c_type == UNIT_MANAGER) {
		if(o_unit->type() == 'D') {
			if(((CompositeUnit*) o_unit)->containsRelativeTo(u)) {
				return true;
			}
		} else if(o_unit->type() == 'A' && o_unit->baseUnit()->type() == 'D') {
			if(((CompositeUnit*) (o_unit->baseUnit()))->containsRelativeTo(u)) {
				return true;
			}		
		}
	}
	return false; 
}
void Manager::clean() {
	for(int i = 0; i < mngrs.size(); i++) {
		mngrs[i]->clean();
	}
	switch(c_type) {
		case MULTIPLICATION_MANAGER: {
			multiclean();
			break;
		}
		case POWER_MANAGER: {
			powerclean();
			break;
		}
		case ADDITION_MANAGER: {
			plusclean();
			break;
		}		
	}
}
void Manager::finalize() {
	dissolveAllCompositeUnits();		
	syncUnits();
	clean();
}
void gatherInformation(Manager *mngr, vector<Unit*> &base_units, vector<AliasUnit*> &alias_units) {
	switch(mngr->type()) {
		case UNIT_MANAGER: {
			switch(mngr->o_unit->type()) {
				case 'U': {
					for(int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == mngr->o_unit) {
							return;
						}
					}
					base_units.push_back(mngr->o_unit);
					break;
				}
				case 'A': {
					for(int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == mngr->o_unit) {
							return;
						}
					}
					alias_units.push_back((AliasUnit*) (mngr->o_unit));				
					break;
				}
				case 'D': {
					mngr = ((CompositeUnit*) (mngr->o_unit))->generateManager();
					gatherInformation(mngr, base_units, alias_units);
					break;
				}				
			}
			break;
		}
		case POWER_MANAGER: {}
		case ADDITION_MANAGER: {}
		case FUNCTION_MANAGER: {}
		case MULTIPLICATION_MANAGER: {
			for(int i = 0; i < mngr->mngrs.size(); i++) {
				gatherInformation(mngr->mngrs[i], base_units, alias_units);
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
	for(int i = 0; i < alias_units.size(); i++) {
		if(alias_units[i]->baseUnit()->type() == 'D') {
			b = false;
			cu = (CompositeUnit*) alias_units[i]->baseUnit();
			for(int i2 = 0; i2 < base_units.size(); i2++) {
				if(cu->containsRelativeTo(base_units[i2])) {
					for(int i = 0; i < composite_units.size(); i++) {
						if(composite_units[i] == cu) {
							b = true;
						}
					}
					if(!b) composite_units.push_back(cu);					
					goto erase_alias_unit_1;
				}
			}
			for(int i2 = 0; i2 < alias_units.size(); i2++) {
				if(cu->containsRelativeTo(alias_units[i2])) {
					for(int i = 0; i < composite_units.size(); i++) {
						if(composite_units[i] == cu) {
							b = true;
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
		for(int i2 = 0; i2 < cu->units.size(); i2++) {
			b = false;
			switch(cu->units[i2]->firstBaseUnit()->type()) {
				case 'U': {
					for(int i = 0; i < base_units.size(); i++) {
						if(base_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) base_units.push_back(cu->units[i2]->firstBaseUnit());
					break;
				}
				case 'A': {
					for(int i = 0; i < alias_units.size(); i++) {
						if(alias_units[i] == cu->units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) alias_units.push_back((AliasUnit*) cu->units[i2]->firstBaseUnit());				
					break;
				}
				case 'D': {
					Manager *mngr = ((CompositeUnit*) cu->units[i2]->firstBaseUnit())->generateManager();
					gatherInformation(mngr, base_units, alias_units);
					break;
				}
			}
		}
		i = -1;
		dont_erase_alias_unit_1:
		true;
	}
	for(int i = 0; i < alias_units.size(); i++) {
		for(int i2 = 0; i2 < alias_units.size(); i2++) {
			if(i != i2 && alias_units[i]->baseUnit() == alias_units[i2]->baseUnit()) { 
				if(alias_units[i2]->isParentOf(alias_units[i])) {
					goto erase_alias_unit_2;
				}
				if(!alias_units[i]->isParentOf(alias_units[i2])) {
					b = false;
					for(int i3 = 0; i < base_units.size(); i3++) {
						if(base_units[i3] == alias_units[i2]->firstBaseUnit()) {
							b = true;
							break;
						}
					}
					if(!b) base_units.push_back(alias_units[i]->baseUnit());
					goto erase_alias_unit_2;
				}
			}
		} 
		goto dont_erase_alias_unit_2;
		erase_alias_unit_2:
		alias_units.erase(alias_units.begin() + i);
		i--;
		dont_erase_alias_unit_2:
		true;
	}	
	for(int i = 0; i < alias_units.size(); i++) {
		if(alias_units[i]->baseUnit()->type() == 'U') {
			for(int i2 = 0; i2 < base_units.size(); i2++) {
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
		true;
	}
	for(int i = 0; i < composite_units.size(); i++) {	
		convert(composite_units[i]);
	}	
	dissolveAllCompositeUnits();
	for(int i = 0; i < base_units.size(); i++) {	
		convert(base_units[i]);
	}
	for(int i = 0; i < alias_units.size(); i++) {	
		convert(alias_units[i]);
	}	
}

bool Manager::dissolveAllCompositeUnits() {
	switch(type()) {
		case UNIT_MANAGER: {
			if(o_unit->type() == 'D') {
				Manager *mngr = ((CompositeUnit*) o_unit)->generateManager();			
				moveto(mngr);
				mngr->unref();
				return true;
			}
			break;
		}
		case POWER_MANAGER: {}
		case MULTIPLICATION_MANAGER: {} 
		case ADDITION_MANAGER: {
			bool b = false;
			for(int i = 0; i < mngrs.size(); i++) {
				if(mngrs[i]->dissolveAllCompositeUnits()) b = true;
			}
			return b;
		}
	}		
	return false;
}
bool Manager::convert(string unit_str) {
	Manager *mngr = calc->calculate(unit_str);
	bool b = convert(mngr);
	mngr->unref();
	return b;
}
bool Manager::convert(Manager *unit_mngr) {
	unit_mngr->ref();
	bool b = false;
	switch(unit_mngr->type()) {
		case UNIT_MANAGER: {
			if(convert(unit_mngr->o_unit)) b = true;
			break;
		}
		case POWER_MANAGER: {}
		case MULTIPLICATION_MANAGER: {} 
		case ADDITION_MANAGER: {
			for(int i = 0; i < unit_mngr->mngrs.size(); i++) {
				if(convert(unit_mngr->mngrs[i])) b = true;
			}
			break;
		}
	}	
	unit_mngr->unref();
	return b;
}

bool Manager::convert(Unit *u) {
	if(c_type == VALUE_MANAGER || c_type == STRING_MANAGER) return false;
	bool b = false;	
	if(c_type == UNIT_MANAGER && o_unit == u) return false;
	if(u->type() == 'D' && !(c_type == UNIT_MANAGER && o_unit->baseUnit() == u)) {
		Manager *mngr = ((CompositeUnit*) u)->generateManager();
		b = convert(mngr);
		mngr->unref();
		return b;
	}
	if(c_type == ADDITION_MANAGER) {
		for(int i = 0; i < mngrs.size(); i++) {
			if(mngrs[i]->convert(u)) b = true;
		}
		plusclean();
		return b;
	} else if(c_type == FUNCTION_MANAGER) {
		for(int i = 0; i < mngrs.size(); i++) {
			if(mngrs[i]->convert(u)) b = true;
		}
		return b;
	} else if(c_type == UNIT_MANAGER) {
		if(u == o_unit) return false;
		if(testDissolveCompositeUnit(u)) {
			convert(u);
			return true;
		}
		Manager *exp = new Manager(calc, 1.0L);
		Manager *mngr = u->convert(o_unit, NULL, exp, &b);
		if(b) {
			o_unit = u;
			if(exp->type() != VALUE_MANAGER || exp->value() != 1.0L) {
				add(exp, POWER_CH);
			}
			add(mngr, MULTIPLICATION_CH);
		}
		exp->unref();
		mngr->unref();
		return b;
	} else if(c_type == POWER_MANAGER) {
		bool b = false;
		b = mngrs[1]->convert(u);
		if(b) {
			powerclean();
			return convert(u);
		}
		if(mngrs[0]->c_type == UNIT_MANAGER) {
			if(u == mngrs[0]->o_unit) return false;
			if(mngrs[0]->testDissolveCompositeUnit(u)) {
				powerclean();
				convert(u);			
				return true;
			}
			Manager *mngr = u->convert(mngrs[0]->o_unit, NULL, mngrs[1], &b);
			if(b) {
				mngrs[0]->o_unit = u;
				add(mngr, MULTIPLICATION_CH);
			}			
			mngr->unref();
		} else {
			b = mngrs[0]->convert(u);
		}
		return b;
	} else if(c_type == MULTIPLICATION_MANAGER) {
		bool c = false;
		for(int i = 0; i < mngrs.size(); i++) {
			if(mngrs[i]->testDissolveCompositeUnit(u)) {
				multiclean();			
				b = true;
				convert(u);
				multiclean();
				i = -1;
				if(b) c = true;
			} else if(mngrs[i]->type() == UNIT_MANAGER && mngrs[i]->o_unit != u) {
				Manager *mngr;
				if(mngrs[i]->o_unit->hasComplexRelationTo(u)) {
					int i3 = 0;
					for(int i2 = 0; i2 < mngrs.size(); i2++) {
						if(mngrs[i2]->type() == UNIT_MANAGER || (mngrs[i2]->type() == POWER_MANAGER && mngrs[i2]->mngrs[0]->c_type == UNIT_MANAGER)) {
							i3++;
						}
					}
					if(i3 > 1) return false;
				}
				mngr = new Manager(this);
				mngr->add(mngrs[i], DIVISION_CH);
				Manager *exp = new Manager(calc, 1.0L);				
				u->convert(mngrs[i]->o_unit, mngr, exp, &b);
				if(b) {
					set(u);
					if(exp->type() != VALUE_MANAGER || exp->value() != 1.0L) {
						add(exp, POWER_CH);
					}
					add(mngr, MULTIPLICATION_CH);										
					c = true;
				}
				mngr->unref();
			} else if(mngrs[i]->type() == POWER_MANAGER && mngrs[i]->mngrs[0]->c_type == UNIT_MANAGER && mngrs[i]->mngrs[0]->o_unit != u) {
				Manager *mngr;
				if(mngrs[i]->mngrs[0]->o_unit->hasComplexRelationTo(u)) {
					int i3 = 0;
					for(int i2 = 0; i2 < mngrs.size(); i2++) {
						if(mngrs[i2]->type() == UNIT_MANAGER || (mngrs[i2]->type() == POWER_MANAGER && mngrs[i2]->mngrs[0]->c_type == UNIT_MANAGER)) {
							i3++;
						}
					}
					if(i3 > 1) return false;				
				}
				mngr = new Manager(this);
				mngr->add(mngrs[i], DIVISION_CH);
				u->convert(mngrs[i]->mngrs[0]->o_unit, mngr, mngrs[i]->mngrs[1], &b);
				if(b) {
					Manager *mngr2 = mngrs[i]->mngrs[1];
					mngr2->ref();
					set(u);
					add(mngr2, POWER_CH);
					mngr2->unref();
					add(mngr, MULTIPLICATION_CH);					
					c = true;
				}		
				mngr->unref();			
			}
			true;
		}
//		return c;
//		if(c) return c;
		for(int i = 0; i < mngrs.size(); i++) {
			if(mngrs[i]->testDissolveCompositeUnit(u)) {
				 convert(u); c = true;
			} else if(mngrs[i]->convert(u)) b = true;
		}
		if(b) {
			for(int i = 0; i < mngrs.size(); i++) {
				if(mngrs[i]->testDissolveCompositeUnit(u)) {
					b = true;
					convert(u);
					multiclean();
					c = true;
				} else if(mngrs[i]->type() == UNIT_MANAGER) {
					if(mngrs[i]->o_unit->hasComplexRelationTo(u)) {
						return true;
					}				
					Manager *mngr = new Manager(this);
					Manager *exp = new Manager(calc, 1.0L);
					mngr->add(mngrs[i], DIVISION_CH);
					u->convert(mngrs[i]->o_unit, mngr, exp, &b);
					if(b) {
						set(u);
						if(exp->type() != VALUE_MANAGER || exp->value() != 1.0L) {
							add(exp, POWER_CH);
						}
						add(mngr, MULTIPLICATION_CH);
						c = true;
					}
					mngr->unref();
				} else if(mngrs[i]->type() == POWER_MANAGER && mngrs[i]->mngrs[0]->c_type == UNIT_MANAGER && mngrs[i]->mngrs[0]->o_unit != u) {
					if(mngrs[i]->mngrs[0]->o_unit->hasComplexRelationTo(u)) {
						return true;
					}
					Manager *mngr = new Manager(this);
					mngr->add(mngrs[i], DIVISION_CH);
					u->convert(mngrs[i]->mngrs[0]->o_unit, mngr, mngrs[i]->mngrs[1], &b);
					if(b) {	
						Manager *mngr2 = mngrs[i]->mngrs[1];
						mngr2->ref();
						set(u);
						add(mngr2, POWER_CH);
						add(mngr, MULTIPLICATION_CH);
						mngr->unref();
						c = true;
					}			
					mngr->unref();			
				}
			}		
			c = true;
		}
		return c;			
	}
}
void Manager::unref() {
	refcount--;
	if(refcount <= 0) delete this;
}
void Manager::ref() {
	refcount++;
}
char Manager::type() const {
	return c_type;
}

void Manager::differentiate(string x_var) {
	switch(c_type) {
		case ADDITION_MANAGER: {
			for(int i = 0; i < mngrs.size(); i++) {
				mngrs[i]->differentiate(x_var);
			}
			plusclean();
			break;
		}
		case VALUE_MANAGER: {
			clear();
			break;
		}
		case FUNCTION_MANAGER: {
			Manager *mngr = new Manager(calc, calc->getFunction("differentiate"), this);
			moveto(mngr);
			break;
		}
		case STRING_MANAGER: {
			if(s_var == x_var) set(1.0L);
			else clear();
			break;
		}
		case POWER_MANAGER: {
			Manager *mngr = new Manager(mngrs[1]);
			Manager *mngr2 = new Manager(mngrs[0]);
			mngrs[1]->add(-1, PLUS_CH);
			powerclean();
			add(mngr, MULTIPLICATION_CH);
			mngr->unref();
			mngr2->differentiate(x_var);
			add(mngr2, MULTIPLICATION_CH);
			mngr2->unref();
			break;
		}
		case MULTIPLICATION_MANAGER: {
			Manager *mngr = new Manager(mngrs[0]);
			Manager *mngr2 = new Manager(mngrs[1]);
			mngr->differentiate(x_var);
			mngr2->differentiate(x_var);			
			mngr->add(mngrs[1], MULTIPLICATION_CH);					
			mngr2->add(mngrs[0], MULTIPLICATION_CH);
			moveto(mngr);
			add(mngr2, PLUS_CH);
			mngr2->unref();			
			break;
		}		
	}
}

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

