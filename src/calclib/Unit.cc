/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Unit.h"
#include "util.h"

Unit::Unit(string cat_, string name_, string plural_, string short_name_, string title_, bool is_user_unit) {
	b_user = is_user_unit;
	remove_blank_ends(name_);
	remove_blank_ends(plural_);
	remove_blank_ends(short_name_);
	remove_blank_ends(cat_);
	setCategory(cat_);
	sname = name_;
	sshortname = short_name_;
	stitle = title_;
	splural = plural_;
	b_changed = false;
}
Unit::~Unit() {}
bool Unit::isUsedByOtherUnits() const {
	return CALCULATOR->unitIsUsedByOtherUnits(this);
}
void Unit::setTitle(string title_) {
	remove_blank_ends(title_);
	stitle = title_;
	b_changed = true;
}
string Unit::title(bool return_name_if_no_title) const {
	if(return_name_if_no_title && stitle.empty()) {
		return name();
	}
	return stitle;
}
void Unit::setCategory(string cat_) {
	remove_blank_ends(cat_);
	scategory = cat_;
	b_changed = true;
}
void Unit::setName(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != sname) {
		sname = CALCULATOR->getUnitName(name_, this, force);
		b_changed = true;
	}
	CALCULATOR->unitNameChanged(this);
}
void Unit::setPlural(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != splural) {
		splural = CALCULATOR->getUnitName(name_, this, force);
		b_changed = true;
	}
	CALCULATOR->unitPluralChanged(this);
}
void Unit::setShortName(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_.empty()) {
		sshortname = name_;
	} else if(name_ != sshortname) {
		sshortname = CALCULATOR->getUnitName(name_, this, force);
	}
	b_changed = true;
	CALCULATOR->unitShortNameChanged(this);

}
string Unit::category() const {
	return scategory;
}
string Unit::name() const {
	return sname;
}
string Unit::plural(bool return_name_if_no_plural) const {
	if(return_name_if_no_plural && splural.empty()) {
		return name();
	}
	return splural;
}
string Unit::baseName() const {
	return name();
}
string Unit::baseExpName() const {
	return name();
}
string Unit::shortName(bool return_name_if_no_short, bool plural_) const {
	if(return_name_if_no_short && sshortname.empty()) {
		if(plural_) {
			return plural(true);
		}
		return name();
	}
	return sshortname;
}
string Unit::shortBaseName() const {
	return shortName();
}
string Unit::shortBaseExpName() const {
	return shortName();
}
const Unit* Unit::baseUnit() const {
	return this;
}
Manager *Unit::baseValue(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);	
//	value_->add(1, RAISE);
	return value_;
}
Manager *Unit::convertToBase(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
//	value_->add(-1, RAISE);
	return value_;
}
long int Unit::baseExp(long int exp_) const {
	return exp_;
}
char Unit::type() const {
	return 'U';
}
bool Unit::isChildOf(const Unit *u) const {
	return false;
}
bool Unit::isParentOf(const Unit *u) const {
	return u != this && u->baseUnit() == this;
}
bool Unit::hasComplexRelationTo(const Unit *u) const {
	if(u == this || u->baseUnit() != this) return false;
	Unit *fbu = (Unit*) u;
	if(fbu->type() != 'A') return false;
	while(1) {
		if(fbu == this) return false;
		if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
		if(fbu->type() != 'A') return false;
		fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();
	}
}
Manager *Unit::convert(const Unit *u, Manager *value_, Manager *exp_, bool *converted) const {
	if(!value_) value_ = new Manager(1, 1);
	if(!exp_) exp_ = new Manager(1, 1);
	else exp_->ref();
	bool b = false;
	if(u->baseUnit() == baseUnit()) {
		u->baseValue(value_, exp_);		
		convertToBase(value_, exp_);
		b = true;
	} else if(u->type() == 'D') {
		CompositeUnit *cu = (CompositeUnit*) u;
		for(int i = 0; i < cu->units.size(); i++) {
			if(convert(cu->units[i], value_, exp_)) b = true;
		}
	}
	exp_->unref();
	if(converted) *converted = b;
	return value_;
}
bool Unit::isUserUnit() const {return b_user;}
bool Unit::hasChanged() const {return b_changed;}
void Unit::setUserUnit(bool is_user_unit) {b_user = is_user_unit;}
void Unit::setChanged(bool has_changed) {b_changed = has_changed;}

AliasUnit::AliasUnit(string cat_, string name_, string plural_, string short_name_, string title_, const Unit *alias, string relation, long int exp_, string reverse, bool is_user_unit) : Unit(cat_, name_, plural_, short_name_, title_, is_user_unit) {
	unit = (Unit*) alias;
	remove_blank_ends(relation);
	remove_blank_ends(reverse);
	value = relation;
	rvalue = reverse;
	exp = exp_;
	exp_mngr = new Manager(exp, 1);
	b_changed = false;
	b_exact = true;
}
AliasUnit::~AliasUnit() {
	exp_mngr->unref();
}
string AliasUnit::baseName() const {
	return unit->baseName();
}
string AliasUnit::baseExpName() const {
	if(baseExp() != 1) {
		string str = unit->baseName();
		str += POWER_STR;
		str += li2s(baseExp());
		return str;
	} else {
		return unit->baseName();
	}
}
string AliasUnit::shortBaseName() const {
	return unit->shortBaseName();
}
string AliasUnit::shortBaseExpName() const {
	if(baseExp() != 1) {
		string str = unit->shortBaseName();
		str += POWER_STR;
		str += li2s(baseExp());
		return str;
	} else {
		return unit->shortBaseName();
	}
}
const Unit* AliasUnit::baseUnit() const {
	return unit->baseUnit();
}
string AliasUnit::firstBaseName() const {
	return unit->name();
}
string AliasUnit::firstBaseExpName() const {
	if(exp != 1) {
		string str = unit->name();
		str += POWER_STR;
		str += li2s(exp);
		return str;
	} else
		return unit->name();
}
string AliasUnit::firstShortBaseName() const {
	return unit->shortName(true);
}
string AliasUnit::firstShortBaseExpName() const {
	if(exp != 1) {
		string str = unit->shortName(true);
		str += POWER_STR;
		str += li2s(exp);
		return str;
	} else
		return unit->shortName(true);
}
const Unit* AliasUnit::firstBaseUnit() const {
	return unit;
}
void AliasUnit::setBaseUnit(const Unit *alias) {
	unit = (Unit*) alias;
	b_changed = true;
}
string AliasUnit::expression() const {
	return value;
}
string AliasUnit::reverseExpression() const {
	return rvalue;
}
void AliasUnit::setExpression(string relation) {
	remove_blank_ends(relation);
	if(relation.empty())
		value = "1";
	else
		value = relation;
	b_changed = true;		
}
void AliasUnit::setReverseExpression(string reverse) {
	remove_blank_ends(reverse);
	rvalue = reverse;
	b_changed = true;
}
Manager *AliasUnit::baseValue(Manager *value_, Manager *exp_) const {
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	firstBaseValue(value_, exp_);
	exp_->add(exp_mngr, MULTIPLY);	
	unit->baseValue(value_, exp_);
	exp_->unref();
	return value_;
}
Manager *AliasUnit::convertToBase(Manager *value_, Manager *exp_) const {
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	exp_->add(exp_mngr, DIVIDE);	
	convertToFirstBase(value_, exp_);	
	unit->convertToBase(value_, exp_);
	exp_->unref();	
	return value_;
}
long int AliasUnit::baseExp(long int exp_) const {
	return unit->baseExp(exp_ * exp);
}
Manager *AliasUnit::convertToFirstBase(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	if(rvalue.empty()) {
		if(value.find(FUNCTION_VAR_X) != string::npos) {
			string stmp = value;
			string stmp2 = LEFT_BRACKET;
			stmp2 += value_->print();
			stmp2 += RIGHT_BRACKET_CH;
			gsub(FUNCTION_VAR_X, stmp2, stmp);
			stmp2 = LEFT_BRACKET;
			stmp2 += exp_->print();
			stmp2 += RIGHT_BRACKET_CH;
			gsub(FUNCTION_VAR_Y, stmp2, stmp);
			Manager *mngr = CALCULATOR->calculate(stmp);
//			value_->add(mngr, DIVIDE);
			value_->moveto(mngr);
			mngr->unref();
		} else {
			Manager *mngr = CALCULATOR->calculate(value);
			mngr->add(exp_, RAISE);
			value_->add(mngr, DIVIDE);
//			value_->moveto(mngr);
			mngr->unref();
		}
	} else {
		if(rvalue.find(FUNCTION_VAR_X) != string::npos) {
			string stmp = rvalue;
			string stmp2 = LEFT_BRACKET;
			stmp2 += value_->print();
			stmp2 += RIGHT_BRACKET_CH;
			gsub(FUNCTION_VAR_X, stmp2, stmp);
			stmp2 = LEFT_BRACKET;
			stmp2 += exp_->print();
			stmp2 += RIGHT_BRACKET_CH;
			gsub(FUNCTION_VAR_Y, stmp2, stmp);
			Manager *mngr = CALCULATOR->calculate(stmp);
//			value_->add(mngr, MULTIPLY);
			value_->moveto(mngr);
			mngr->unref();
		} else {
			Manager *mngr = CALCULATOR->calculate(rvalue);
			mngr->add(exp_, RAISE);
			value_->add(mngr, MULTIPLY);
//			mngr->add(value_, MULTIPLY);
//			value_->moveto(mngr);
			mngr->unref();
		}
	}
	exp_->unref();
	if(!isPrecise()) value_->setPrecise(false);
	return value_;
}
Manager *AliasUnit::firstBaseValue(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	if(value.find(FUNCTION_VAR_X) != string::npos) {
		string stmp = value;
		string stmp2 = LEFT_BRACKET;
		stmp2 += value_->print();
		stmp2 += RIGHT_BRACKET_CH;
		gsub(FUNCTION_VAR_X, stmp2, stmp);
		stmp2 = LEFT_BRACKET;
		stmp2 += exp_->print();
		stmp2 += RIGHT_BRACKET_CH;
		gsub(FUNCTION_VAR_Y, stmp2, stmp);
		Manager *mngr = CALCULATOR->calculate(stmp);
		value_->moveto(mngr);
		mngr->unref();
	} else {
		Manager *mngr = CALCULATOR->calculate(value);
		mngr->add(exp_, RAISE);
		mngr->add(value_, MULTIPLY);
		value_->moveto(mngr);
		mngr->unref();
	}
	exp_->unref();
	if(!isPrecise()) value_->setPrecise(false);	
	return value_;
}
void AliasUnit::setExponent(long int exp_) {
	exp = exp_;
	exp_mngr->set(exp, 1);
	b_changed = true;
}
long int AliasUnit::firstBaseExp() const {
	return exp;
}
char AliasUnit::type() const {
	return 'A';
}
bool AliasUnit::isChildOf(const Unit *u) const {
	if(u == this) return false;
	if(baseUnit() == u) return true;
	if(u->baseUnit() != baseUnit()) return false;
	Unit *u2 = (Unit*) this;
	while(1) {
		u2 = (Unit*) ((AliasUnit*) u2)->firstBaseUnit();
		if(u == u2) return true;
		if(u2->type() != 'A') return false;
	}
	return false;
}
bool AliasUnit::isParentOf(const Unit *u) const {
	if(u == this) return false;
	if(u->baseUnit() != baseUnit()) return false;
	while(1) {
		if(u->type() != 'A') return false;
		u = ((AliasUnit*) u)->firstBaseUnit();
		if(u == this) return true;
	}
	return false;
}
bool AliasUnit::hasComplexExpression() const {
	return value.find(FUNCTION_VAR_X) != string::npos;
}
bool AliasUnit::hasComplexRelationTo(const Unit *u) const {
	if(u == this || u->baseUnit() != baseUnit()) return false;
	if(isParentOf(u)) {
		Unit *fbu = (Unit*) u;
		while(1) {
			if(fbu == this) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->type() != 'A') return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();			
		}	
	} else if(isChildOf(u)) {
		Unit *fbu = (Unit*) this;
		if(fbu->type() != 'A') return false;
		while(1) {
			if(fbu == u) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->type() != 'A') return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();
		}			
	} else {
		return hasComplexRelationTo(baseUnit()) || u->hasComplexRelationTo(u->baseUnit());
	}
}
bool AliasUnit::isPrecise() const {
	return b_exact;
}
void AliasUnit::setPrecise(bool is_precise) {
	b_exact = is_precise;
}

AliasUnit_Composite::AliasUnit_Composite(const Unit *alias, long int exp_, const Prefix *prefix_) : AliasUnit("", alias->name(), alias->plural(false), alias->shortName(false), "", alias, "", exp_, "") {
	prefixv = (Prefix*) prefix_;
}
AliasUnit_Composite::~AliasUnit_Composite(void) {}
string AliasUnit_Composite::printShort(bool plural_) const {
	if(firstBaseUnit()->shortName(false).empty())
		return print(plural_);
	string str = "";
	if(prefixv) {
		str += prefixv->name(true);
	}
	str += firstBaseUnit()->shortName(true, plural_);
	return str;
}
string AliasUnit_Composite::print(bool plural_) const {
	string str = "";
	if(prefixv) {
		str += prefixv->name(false);
	}
	if(plural_)
		str += firstBaseUnit()->plural();
	else
		str += firstBaseUnit()->name();
	return str;
}
const Prefix *AliasUnit_Composite::prefix() const {
	return prefixv;
}
long int AliasUnit_Composite::prefixExponent() const {
	if(prefixv) return prefixv->exponent();
	return 0;
}
void AliasUnit_Composite::set(const Unit *u, long int exp_, const Prefix *prefix_) {
	setBaseUnit(u);
	setExponent(exp_);
	prefixv = (Prefix*) prefix_;
}
Manager *AliasUnit_Composite::firstBaseValue(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();	
	exp_->add(exp_mngr, DIVIDE);		
	Manager *mngr = new Manager(1, 1);
//	mngr->add(exp_, RAISE);
	value_->add(mngr, MULTIPLY);
	mngr->unref();
	exp_->unref();
	return value_;
}
Manager *AliasUnit_Composite::convertToFirstBase(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	exp_->add(exp_mngr, MULTIPLY);
	Manager *mngr = new Manager(1, 1);
//	mngr->add(exp_, RAISE);
	value_->add(mngr, DIVIDE);
	mngr->unref();
	exp_->unref();
	return value_;
}

CompositeUnit::CompositeUnit(string cat_, string name_, string title_, string base_expression_, bool is_user_unit) : Unit(cat_, name_, "", "", title_, is_user_unit) {
	setBaseExpression(base_expression_);
	b_changed = false;
}
CompositeUnit::~CompositeUnit(void) {
	for(int i = 0; i < units.size(); i++)
		delete units[i];
}
void CompositeUnit::add(const Unit *u, long int exp_, const Prefix *prefix) {
	bool b = false;
	for(int i = 0; i < (int) units.size(); i++) {
		if(exp_ > units[i]->firstBaseExp()) {
			units.insert(units.begin() + i, new AliasUnit_Composite(u, exp_, prefix));
			b = true;
			break;
		}
	}
	if(!b) {
		units.push_back(new AliasUnit_Composite(u, exp_, prefix));
	}
}
Unit *CompositeUnit::get(int index, long int *exp_, Prefix **prefix) const {
	if(index >= 0 && index < units.size()) {
		if(exp_) *exp_ = units[index]->firstBaseExp();
		if(prefix) *prefix = (Prefix*) units[index]->prefix();
		return units[index];
	}
	return NULL;
}
void CompositeUnit::del(Unit *u) {
	for(int i = 0; i < units.size(); i++) {
		if(units[i]->firstBaseUnit() == u) {
			delete units[i];
			units.erase(units.begin() + i);
		}
	}
}
string CompositeUnit::print(bool plural_, bool short_) const {
	string str = "";
	bool b = false, b2 = false;
	for(int i = 0; i < units.size(); i++) {
		if(units[i]->firstBaseExp() != 0) {
			if(!b && units[i]->firstBaseExp() < 0 && i > 0) {
				str += DIVISION_STR;
				b = true;
				if(i < units.size() - 1) {
					b2 = true;
					str += LEFT_BRACKET_STR;
				}				
			} else {
//				if(i > 0) str += MULTIPLICATION_STR;
				if(i > 0) str += " ";
			}
			if(short_) {
				if(plural_ && i == 0 && units[i]->firstBaseExp() > 0) {
					str += units[i]->printShort(true);
				} else {
					str += units[i]->printShort(false);
				}
			} else {
				if(plural_ && i == 0 && units[i]->firstBaseExp() > 0) {
					str += units[i]->print(true);
				} else {
					str += units[i]->print(false);
				}
			}
			if(b) {
				if(units[i]->firstBaseExp() != -1) {
					str += POWER_STR;
					str += li2s(-units[i]->firstBaseExp());
				}
			} else {
				if(units[i]->firstBaseExp() != 1) {
					str += POWER_STR;
					str += li2s(units[i]->firstBaseExp());
				}
			}
		}
	}
	if(b2) str += RIGHT_BRACKET_STR;
	return str;
}
string CompositeUnit::name(void) const {
	return print(false, false);
}
string CompositeUnit::plural(bool return_name_if_no_plural) const {
	return print(true, false);
}
string CompositeUnit::shortName(bool return_name_if_no_short, bool plural_) const {
	return print(plural_, true);
}
char CompositeUnit::type() const {
	return 'D';
}
bool CompositeUnit::containsRelativeTo(const Unit *u) const {
	if(u == this) return false;
	CompositeUnit *cu;
	for(int i = 0; i < units.size(); i++) {
		if(u == units[i] || u->baseUnit() == units[i]->baseUnit()) return true;
		if(units[i]->baseUnit()->type() == 'D') {
			cu = (CompositeUnit*) units[i]->baseUnit();
			if(cu->containsRelativeTo(u)) return true;
		}
	}
	if(u->type() == 'D') {
		cu = (CompositeUnit*) u;
		for(int i = 0; i < cu->units.size(); i++) {	
			if(containsRelativeTo(cu->units[i]->baseUnit())) return true;
		}
		return false;
	}	
	return false;
}
Manager *CompositeUnit::generateManager(bool cleaned) const {
	if(cleaned) {
		return CALCULATOR->calculate(print(false, true));
	} else {
		Manager *mngr = new Manager();
		if(units.size() > 0) mngr->c_type = MULTIPLICATION_MANAGER;
		for(int i = 0; i < units.size(); i++) {
			if(units[i]->firstBaseExp() != 1) {
				Manager *mngr2 = new Manager();
				mngr2->c_type = POWER_MANAGER;
				if(units[i]->prefix()) {
					Manager *mngr3 = new Manager();
					mngr3->c_type = MULTIPLICATION_MANAGER;
					mngr3->mngrs.push_back(new Manager(1, 1, units[i]->prefix()->exponent()));
					mngr3->mngrs.push_back(new Manager(units[i]->firstBaseUnit()));
					mngr2->mngrs.push_back(mngr3);
				} else {				
					mngr2->mngrs.push_back(new Manager(units[i]->firstBaseUnit()));
				}
				mngr2->mngrs.push_back(new Manager(units[i]->firstBaseExp(), 1));
				mngr->mngrs.push_back(mngr2);				
			} else {
				if(units[i]->prefix()) {
					mngr->mngrs.push_back(new Manager(1, 1, units[i]->prefix()->exponent()));
				}
				mngr->mngrs.push_back(new Manager(units[i]->firstBaseUnit()));
			}
		}
		return mngr;
	}
}
string CompositeUnit::internalName() const {
	return sname;
}
void CompositeUnit::setBaseExpression(string base_expression_) {
	units.clear();
	if(base_expression_.empty()) {
		b_changed = true;
		return;
	}
	bool b_var = CALCULATOR->variablesEnabled();
	CALCULATOR->setVariablesEnabled(false);
	bool b_var_u = CALCULATOR->unknownVariablesEnabled();
	CALCULATOR->setUnknownVariablesEnabled(false);	
	bool b_func = CALCULATOR->functionsEnabled();
	CALCULATOR->setFunctionsEnabled(false);	
	bool b_unit = CALCULATOR->unitsEnabled();
	CALCULATOR->setUnitsEnabled(true);		
	CALCULATOR->setFunctionsAndVariables(base_expression_);
	int div_place = base_expression_.find(DIVISION_CH);
	bool div = false;
	Prefix *prefix = NULL;
	long int exp = 1;
	int i = 0, i2 = 0, id;
	Manager *mngr;
	while(true) {
		i = base_expression_.find(ID_WRAP_LEFT_CH, i2);
		if(i == string::npos) {
			if(i2 == 0) {
				CALCULATOR->error(false, _("Error in unitexpression: \"%s\"."), base_expression_.c_str(), NULL);
			} else if(base_expression_.length() > i2 + 2) {
				CALCULATOR->error(false, _("Error in unitexpression: \"%s\"."), base_expression_.substr(i2 + 2, base_expression_.length() - i2 - 2).c_str(), NULL);
			}
			break;
		}
		if(i > i2 + 3) {
			if(!(i - i2 - 3 == 1 && is_in(OPERATORS, base_expression_[i2 + 2])))
				CALCULATOR->error(false, _("Error in unitexpression: \"%s\"."), base_expression_.substr(i2 + 2, i - i2 - 3).c_str(), NULL);
		} else if(i2 == 0 && i > 1) {
			if(!(i == 2 && is_in(OPERATORS, base_expression_[0])))
				CALCULATOR->error(false, _("Error in unitexpression: \"%s\"."), base_expression_.substr(0, i - 1).c_str(), NULL);
		}
		i2 = base_expression_.find(ID_WRAP_RIGHT_CH, i);		
		if(i2 == string::npos) {
			break;
		}
		id = s2i(base_expression_.substr(i + 1, i2 - i - 1));
		if(!div && div_place != string::npos && i > div_place) {
			div = true;
		}		
		mngr = CALCULATOR->getId(id);
		if(mngr) {
			prefix = NULL;
			exp = 1;
			if(mngr->type() == MULTIPLICATION_MANAGER && mngr->mngrs.size() == 2 && mngr->mngrs[0]->isFraction() && mngr->mngrs[1]->type() == UNIT_MANAGER) {
				prefix = CALCULATOR->getExactPrefix(mngr->mngrs[0]->fraction());
				mngr = mngr->mngrs[1];
			} 
			if(mngr->type() == UNIT_MANAGER) {
				if(base_expression_.length() > i2 + 3 && base_expression_[i2 + 2] == POWER_CH) {
					if(is_in(NUMBERS, base_expression_[i2 + 3])) {
						exp = s2li(base_expression_.substr(i2 + 3, 1));
						i2 += 2;
					} else if(base_expression_.length() > i2 + 4 && is_in(MINUS PLUS, base_expression_[i2 + 3]) && is_in(NUMBERS, base_expression_[i2 + 4])) {
						exp = s2li(base_expression_.substr(i2 + 3, 2));
						i2 += 3;
					}
				}
				if(div) {
					exp = -exp;
				}
				add(mngr->o_unit, exp, prefix);
			} else {
				CALCULATOR->error(false, _("Error in unitexpression: \"%s\"."), mngr->print().c_str(), NULL);
			}
			CALCULATOR->delId(id);
		}
	}
	CALCULATOR->setVariablesEnabled(b_var);
	CALCULATOR->setUnknownVariablesEnabled(b_var_u);	
	CALCULATOR->setFunctionsEnabled(b_func);	
	CALCULATOR->setUnitsEnabled(b_unit);			
	b_changed = true;
}

