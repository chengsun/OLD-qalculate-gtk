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
Manager::Manager(Calculator *calc_, Unit *u, long double value_) {
	calc = calc_;
	refcount = 1;
	set(u, value_);
}
Manager::Manager(Manager *mngr) {
	refcount = 1;
	set(mngr);
}
Manager::~Manager() {
	clear();
}
void Manager::set(Manager *mngr) {
	clear();
	if(mngr != NULL) {
		calc = mngr->calc;
		d_value = mngr->d_value;
		o_unit = mngr->o_unit;
		s_var = mngr->s_var;
		for(int i = 0; i < mngr->mngrs.size(); i++) {
			mngrs.push_back(new Manager(mngr->mngrs[i]));
		}
		c_type = mngr->c_type;
	}
}
void Manager::set(long double value_) {
	clear();
	if(value_ == 0) return;
	d_value = value_;
	c_type = 'v';
}
void Manager::set(string var_) {
	clear();
	if(var_.empty()) return;
	s_var = var_;
	c_type = 's';
}
void Manager::set(Unit *u, long double value_) {
	clear();
	if(value_ == 0) return;
	if(value_ == 1) {
		o_unit = u;
		c_type = 'u';
	} else {
		mngrs.push_back(new Manager(calc, value_));
		mngrs.push_back(new Manager(calc, u));
		c_type = MULTIPLICATION_CH;
	}
}
void Manager::plusclean() {
	for(int i = 0; i < mngrs.size() - 1; i++) {
		for(int i2 = i + 1; i2 < mngrs.size(); i2++) {
			if(mngrs[i]->add(mngrs[i2], PLUS_CH, false)) {
				mngrs[i2]->unref();
				mngrs.erase(mngrs.begin() + i2);
				i = -1;
				break;
			}
		}
	}
	if(mngrs.size() == 1) {
		moveto(mngrs[0]);
	}
}
void Manager::multiclean() {
	for(int i = 0; i < mngrs.size() - 1; i++) {
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
	}
}
void Manager::powerclean() {
	if(mngrs[0]->add(mngrs[1], POWER_CH, false)) {
		mngrs[1]->unref();
		mngrs.erase(mngrs.begin() + 1);
		//break;
	}
	if(mngrs.size() == 1) {
		moveto(mngrs[0]);
	}
}
bool Manager::reverseadd(Manager *mngr, char sign, bool translate_) {
	mngr = new Manager(mngr);
	if(!mngr->add(this, sign, translate_)) return false;
	set(mngr);
	mngr->unref();
	return true;
}
void Manager::transform(Manager *mngr, char type_, char sign, bool reverse_) {
	Manager *mngr2 = new Manager(this);
	clear();
	mngr = new Manager(mngr);
	if(reverse_ || sign == POWER_CH) {
		mngrs.push_back(mngr2);
		mngrs.push_back(mngr);
	} else {
		mngrs.push_back(mngr);
		mngrs.push_back(mngr2);
	}
	c_type = type_;
}
bool Manager::add(Manager *mngr, char sign, bool translate_) {
	printf("[%s] %c [%s] (%i)\n", print().c_str(), sign, mngr->print().c_str(), translate_);
	if(mngr->c_type == 'v' && (c_type == 0 || c_type == 'v')) {
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
		 	calc->error(false, 3, "Trying to divide \"", print().c_str(), "\" with zero");
		} else if(mngr->c_type == 'v' && mngr->d_value == 1) {
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
			mngr->unref(); return true;
		}
		switch(c_type) {
			case PLUS_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {
						for(int i = 0; i < mngr->mngrs.size(); i++) {
							add(mngr->mngrs[i], sign);
						}
						break;
					}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 'v': {}
					case 's': {}
					case 'u': {
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
						if(!reverseadd(mngr, sign, translate_)) {mngr->unref(); return false;}
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
										mngrs.erase(mngrs.begin());
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
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, PLUS_CH, sign);
						break;
					}
					case POWER_CH: {
					}
					case 's': {}
					case 'u': {
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
						if(!translate_) {mngr->unref(); return false;}
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
					case 'v': {}
					case 's': {}
					case 'u': {
						if(!translate_) {mngr->unref(); return false;}
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
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 's': {}
					case 'u': {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, PLUS_CH, sign, true);
						break;
					}
					case 'v': {
						d_value += mngr->d_value;
						if(d_value == 0) c_type = 0;
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
						//convert
						if(o_unit == mngr->o_unit) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(new Manager(calc, 2));
							mngrs.push_back(mngr2);
							c_type = MULTIPLICATION_CH;
							break;
						} else if(o_unit->baseUnit() == mngr->o_unit->baseUnit()) {
							if(o_unit->type() != 'A') {
								Manager *mngr2 = mngr->o_unit->baseValue();
								mngr2->add(o_unit, MULTIPLICATION_CH);
								add(mngr2, sign);
								mngr2->unref();
								break;
							}
							if(mngr->o_unit->type() != 'A') {
								Manager *mngr2 = o_unit->baseValue();
								mngr2->add(mngr->o_unit, MULTIPLICATION_CH);
								mngr2->add(mngr->o_unit, sign);
								moveto(mngr2);
								mngr2->unref();
								break;
							}
/*							long double rel = 1.0;
							Unit *u = o_unit;
							bool b = false;
							while(1) {
								Unit *u = ((AliasUnit*) u)->firstBaseUnit();
								if(u == mngr->o_unit) {
									rel = o_unit->baseValue();
									rel = mngr->o_unit->convertToBase(rel);
									set(mngr->o_unit, rel);
									add(mngr, sign);
									b = true;
									break;
								}
								if(u->type() != 'A') {break;}
							}
							if(!b) {
								u = mngr->o_unit;
								while(1) {
									Unit *u = ((AliasUnit*) u)->firstBaseUnit();
									if(u == o_unit) {
										rel = mngr->o_unit->baseValue();
										rel = o_unit->convertToBase(rel);
										Manager *mngr2 = new Manager(calc, o_unit, rel);
										add(mngr2, sign);
										mngr2->unref();
										b = true;
										break;
									}
									if(u->type() != 'A') {break;}
								}
							}
							if(!b) {
								rel = mngr->o_unit->baseValue();
								rel = o_unit->convertToBase(rel);
								Manager *mngr2 = new Manager(calc, o_unit, rel);
								add(mngr2, sign);
								mngr2->unref();
							}*/
							break;
						}
					}
					case POWER_CH: {}
					case 's': {}
					case 'v': {
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
					case POWER_CH: {}
					case 'u': {}
					case 'v': {
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
		} else if(mngr->c_type == 'v' && mngr->d_value == 1) {
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
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 'v': {}
					case 's': {}
					case 'u': {
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
					case POWER_CH: {}
					case 'v': {}
					case 's': {}
					case 'u': {
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
					case 'v': {}
					case 's': {}
					case 'u': {
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
				if(d_value == 1) {set(mngr); mngr->unref(); return true;}
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
					case 's': {}
					case 'u': {
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
						//convert
						if(o_unit == mngr->o_unit) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(mngr2);
							mngrs.push_back(new Manager(calc, 2));
							c_type = POWER_CH;
							break;
						}
					}
					case 's': {}
					case 'v': {
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
						//convert
						if(s_var == mngr->s_var) {
							Manager *mngr2 = new Manager(this);
							clear();
							mngrs.push_back(mngr2);
							mngrs.push_back(new Manager(calc, 2));
							c_type = POWER_CH;
							break;
						}
					}
					case 'u': {}
					case 'v': {
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
				calc->error(false, 1, "0^0 is undefined");
				if(!translate_) {mngr->unref(); return false;}
				transform(mngr, POWER_CH, sign);
			} else {
				set(1);
				mngr->unref(); return true;
			}
		} else if(mngr->c_type == 'v' && mngr->d_value == 1) {mngr->unref(); return true;}
		switch(c_type) {
			case PLUS_CH: {
				switch(mngr->c_type) {
					case 'v': {
						if(fmodl(mngr->d_value, 1) == 0) {
							long double d = mngr->d_value;
							if(d < 0) d = -d;
							Manager *mngr2 = new Manager(this);
							for(long double i = 1; i < d; i++) {
								add(mngr2, MULTIPLICATION_CH);
							}
							if(mngr2->d_value < 0) {
								mngr2->unref();
								mngr2 = new Manager(calc, 1);
								mngr2->add(this, DIVISION_CH);
								moveto(mngr2);
								mngr2->unref();
							}
							break;
						}
					}
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 's': {}
					case 'u': {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, POWER_CH, sign);
						break;
					}
				}
				break;
			}
			case MULTIPLICATION_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 'v': {}
					case 's': {}
					case 'u': {
						for(int i = 0; i < mngrs.size(); i++) {
							mngrs[i]->add(mngr, sign);
						}
						break;
					}
				}
				break;
			}
			case POWER_CH: {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 'v': {}
					case 's': {}
					case 'u': {
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
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 's': {}
					case 'u': {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, POWER_CH, sign);
						break;
					}
					case 'v': {
						d_value = powl(d_value, mngr->d_value);
						break;
					}
				}
				break;
			}
			case 'u': {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 'v': {}
					case 's': {}
					case 'u': {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, POWER_CH, sign);
						break;
					}
				}
				break;
			}
			case 's': {
				switch(mngr->c_type) {
					case PLUS_CH: {}
					case MULTIPLICATION_CH: {}
					case POWER_CH: {}
					case 'v': {}
					case 's': {}
					case 'u': {
						if(!translate_) {mngr->unref(); return false;}
						transform(mngr, POWER_CH, sign);
						break;
					}
				}
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
	add(mngr, sign);
	mngr->unref();
}
int Manager::compare(Manager *mngr) {
	if(mngr->c_type == 0) {
		if(c_type == 0) return 0;
		else return -1;
	} else if(mngr->c_type == 'v') {
		if(c_type == 0) return 1;
		else if(c_type == 'v') {
				if(d_value == mngr->d_value) return 0;
				else if(d_value > mngr->d_value) return -1;
				else return 1;
		} else return -1;
	} else if(mngr->c_type == 'u') {
		if(c_type == 'u') {
			if(mngr->o_unit->shortName() < o_unit->shortName()) return -1;
			else if(mngr->o_unit->shortName() == o_unit->shortName()) return 0;
			else return 1;
		} else if(c_type != 'v' && c_type != 0 && c_type != 's') return -1;
	} else if(mngr->c_type == 's') {
		if(c_type == 's') {
			if(mngr->s_var < s_var) return -1;
			else if(mngr->s_var == s_var) return 0;
			else return 1;
		} else if(c_type != 'v' && c_type != 0) return -1;
	} else if(mngr->c_type == POWER_CH) {
		if(c_type == POWER_CH) {
			if(mngrs[1]->negative()) {
				int i2 = mngrs[1]->compare(mngr->mngrs[1]);
				if(i2) return -1;
				if(i2 < 0) return 1;
				return 0;
			} else return mngrs[1]->compare(mngr->mngrs[1]);
		} else if(c_type != 'v' && c_type != 'u' && c_type != 0 && c_type != 's') return -1;
	} else if(mngr->c_type == MULTIPLICATION_CH) {
		if(c_type == MULTIPLICATION_CH) {
			for(int i = 0; ; i++) {
				if(i >= mngr->mngrs.size() && i >= mngrs.size()) return 0;
				if(i >= mngr->mngrs.size()) return -1;
				if(i >= mngrs.size()) return 1;
				int i2 = mngrs[i]->compare(mngr->mngrs[i]);
				if(i2 != 0) return i2;
			}
		} else if(c_type == PLUS_CH) return -1;
	} else if(mngr->c_type == PLUS_CH) {
		if(c_type == PLUS_CH) {
			int i1 = mngrs.size() - 1;
			int i2 = mngr->mngrs.size() - 1;
			while(1) {
				if(i1 < 0 && i2 < 0) return 0;
				if(i2 < 0) return -1;
				if(i1 < 0) return 1;
				int i3 = mngrs[i1]->compare(mngr->mngrs[i2]);
				if(i3 != 0) return i3;
				i1--; i2--;
			}
		}
	}
	return 1;
}
void Manager::sort() {
	if(c_type == 'v' || c_type == 'u' || c_type == 's' || c_type == POWER_CH) return;
	vector<Manager*> sorted;
	bool b = false;
	for(int i = 0; i < mngrs.size(); i++) {
		for(int i2 = 0; i2 < sorted.size(); i2++) {
			if((c_type == PLUS_CH && mngrs[i]->compare(sorted[i2]) < 0) || (c_type == MULTIPLICATION_CH && mngrs[i]->compare(sorted[i2]) > 0)) {
				sorted.insert(sorted.begin() + i2, mngrs[i]);
				b = true;
				break;
			}
		}
		if(!b) sorted.push_back(mngrs[i]);
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
	if(c_type == 0 || mngr->c_type == 0) return true;
	if(c_type == mngr->c_type) {
		if(c_type == 'v') {
			return true;
		} else if(c_type == 'u') {
			if(o_unit == mngr->o_unit) return true;
			else if(o_unit->baseUnit() == mngr->o_unit->baseUnit()) return true;
		} else if(c_type == 's') {
			if(s_var == mngr->s_var) return true;
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
		if(mngr->c_type != 'v') {
			if(mngrs.size() != 2) return false;
			if(!mngrs[1]->compatible(mngr)) return false;
			return true;
		}
	}
	return false;
}
void Manager::add(long double value_, char sign) {
	if(c_type == 0 && value_ != 0 && (sign != POWER_CH || value_ > 0)) {
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
	for(int i = 0; i < mngrs.size(); i++) {
		mngrs[i]->unref();
	}
	mngrs.clear();
	c_type = 0;
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
string Manager::print(NumberFormat nrformat, int unitflags, int precision, int decimals_to_keep, bool decimals_expand, bool decimals_decrease, bool *usable, bool toplevel, bool *plural) {
	string str, str2;
	if(toplevel && (unitflags & UNIT_FORMAT_TAGS)) {
	    str = "<b><big>";
	}
	else str = "";
	if(c_type == 'v') {
		long double new_value = d_value;
		switch(nrformat) {
			case NUMBER_FORMAT_NORMAL: {
				str2 = calc->value2str(d_value, precision);
				break;
			}
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
				str2 = calc->value2str_prefix(d_value, precision, unitflags & UNIT_FORMAT_SHORT, &new_value);
				if((unitflags & UNIT_FORMAT_SHORT) && (unitflags & UNIT_FORMAT_NONASCII)) gsub("micro", SIGN_MICRO, str2);
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
		}
		remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
		str += str2;
		if(unitflags & UNIT_FORMAT_TAGS) {
			int i = str.find("E");
			if(i != string::npos && i != str.length() - 1 && (is_in(PLUS_S MINUS_S NUMBERS_S, str[i + 1]))) {
				str.replace(i, 1, "<small>E</small>");
			}
		}
		if(plural) *plural = (new_value > 1 || new_value < -1);
	} else if(c_type == 'u') {
		if(toplevel) {
			str2 = "1";
			remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
			str += str2; str += " ";
		}
		if(unitflags & UNIT_FORMAT_SHORT) {
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
	} else if(c_type == 's') {
		str += s_var;
		if(plural) *plural = true;
	} else if(c_type == PLUS_CH) {
		for(int i = 0; i < mngrs.size(); i++) {
			if(i > 0) {
				str += " ";
				if(mngrs[i]->negative()) str += MINUS_STR;
				else str += PLUS_STR;
				str += " ";
			}
			if(mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u')) {
				str2 = "1";
				remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
				str += str2; str += " ";
			}
			if(mngrs[i]->negative() && i > 0) {
				str2 = mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false);
				str += str2.substr(1, str2.length() - 1);
			} else str += mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false);
		}
	} else if(c_type == MULTIPLICATION_CH) {
		bool b = false;
		bool plural_ = true;
		for(int i = 0; i < mngrs.size(); i++) {
			if(i == 0 && (mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u'))) {
				str2 = "1";
				remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
				str += str2; str += " ";
			}
			if(!(unitflags & UNIT_FORMAT_SCIENTIFIC) && i > 0 && mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[1]->negative()) {
				Manager *mngr = new Manager(mngrs[i]);
				mngr->add(-1, POWER_CH);
				if(!b) str += DIVISION_CH;
				if(!b && i < mngrs.size() - 1) str += LEFT_BRACKET_STR;
				str += mngr->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false, &plural_);
				if(b && i == mngrs.size() - 1) str += RIGHT_BRACKET_STR;
				b = true;
			} else if(mngrs[i]->c_type == PLUS_CH || ((mngrs[i]->c_type == POWER_CH && i == 0) || (i > 0 && mngrs[i - 1]->c_type == POWER_CH))) {
				str += LEFT_BRACKET_STR;
				str += mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false, &plural_);
				str += RIGHT_BRACKET_STR;
			} else {
				if(i > 0 && (mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u'))) str += " ";
				str += mngrs[i]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false, &plural_);
			}
			if(plural_ && (mngrs[i]->c_type == 'u' || (mngrs[i]->c_type == POWER_CH && mngrs[i]->mngrs[0]->c_type == 'u'))) {
				plural_ = false;
			}
		}
	} else if(c_type == POWER_CH) {
		if(toplevel && mngrs[0]->c_type == 'u') {
			str2 = "1";
			remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
			str += str2; str += " ";
		}
		if(mngrs[0]->mngrs.size() > 0) {
			str += LEFT_BRACKET_STR;
			str += mngrs[0]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false);
			str += RIGHT_BRACKET_STR;
		} else {
			str += mngrs[0]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false);
		}
		if(unitflags & UNIT_FORMAT_NONASCII && mngrs[1]->c_type == 'v' && mngrs[1]->d_value == 2) {
			str += SIGN_POWER_2;
		} else if(unitflags & UNIT_FORMAT_NONASCII && mngrs[1]->c_type == 'v' && mngrs[1]->d_value == 3) {
			str += SIGN_POWER_3;
		} else {
			if(unitflags & UNIT_FORMAT_TAGS) str += "<sup>";
			else str += POWER_STR;
			if(mngrs[1]->mngrs.size() > 0) {
				str += LEFT_BRACKET_STR;
				str += mngrs[1]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false);
				str += RIGHT_BRACKET_STR;
			} else {
				str += mngrs[1]->print(nrformat, unitflags, precision, decimals_to_keep, decimals_expand, decimals_decrease, usable, false);
			}
			if(unitflags & UNIT_FORMAT_TAGS) str += "</sup>";
		}
	} else {
		str2 = "0";
		remove_trailing_zeros(str2, decimals_to_keep, decimals_expand, decimals_decrease);
		str += str2;
	}
	if(toplevel && (unitflags & UNIT_FORMAT_TAGS)) str += "</big></b>";
	return str;
}
void Manager::finalize() {}
bool Manager::convert(Unit *u) {
	printf("CONVERT 1\n");
	if(c_type == 'v' || c_type == 's') return false;
	printf("CONVERT 2\n");	
	bool b = false;
	if(c_type == PLUS_CH) {
		printf("CONVERT PLUS 1\n");
		for(int i = 0; i < mngrs.size(); i++) {
			printf("CONVERT PLUS 2\n");
			if(mngrs[i]->convert(u)) b = true;
		}
		if(b) plusclean();
	} else if(c_type == 'u') {
		printf("CONVERT UNIT 1\n");
		if(u == o_unit) return false;
		printf("CONVERT UNIT 2\n");
		Manager *mngr = u->convert(o_unit, NULL, 1, &b);
		if(b) {
			o_unit = u;
			add(mngr, MULTIPLICATION_CH);
		}
		mngr->unref();
		return b;
	} else if(c_type == POWER_CH) {
		printf("CONVERT POWER 1\n");
		bool b = false;
		b = mngrs[1]->convert(u);
		if(b) {
			printf("CONVERT POWER 2\n");
			powerclean();
			return convert(u);
		}
		if(mngrs[0]->c_type == 'u') {
			printf("CONVERT POWER 3\n");
			if(u == o_unit) return false;
			Manager *mngr = u->convert(o_unit, NULL, mngrs[1]->value(), &b);
			if(b) {
				o_unit = u;
				add(mngr, MULTIPLICATION_CH);
			}			
			mngr->unref();
		} else {
			printf("CONVERT POWER 4\n");
			b = mngrs[0]->convert(u);
		}
		return b;
	} else if(c_type == MULTIPLICATION_CH) {
		printf("CONVERT 5\n");
	}
}
void Manager::unref() {
	refcount--;
	if(refcount <= 0) delete this;
}
void Manager::ref() {
	refcount++;
}
char Manager::type() {
	return c_type;
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

