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

Unit::Unit(Calculator *calc_, string cat_, string name_, string plural_, string short_name_, string title_) {
	calc = calc_;
	remove_blank_ends(name_);
	remove_blank_ends(plural_);
	remove_blank_ends(short_name_);
	remove_blank_ends(cat_);
	category(cat_);
	sname = name_;
	sshortname = short_name_;
	stitle = title_;
	splural = plural_;
}
Unit::~Unit() {}
bool Unit::isUsedByOtherUnits(void) {
	return calc->unitIsUsedByOtherUnits(this);
}
void Unit::title(string title_) {
	remove_blank_ends(title_);
	stitle = title_;
}
string Unit::title(void) {
	return stitle;
}
void Unit::category(string cat_) {
	remove_blank_ends(cat_);
	scategory = cat_;
}
void Unit::name(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != sname) {
		sname = calc->getUnitName(name_, this, force);
	}
	calc->unitNameChanged(this);
}
void Unit::plural(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != splural) {
		splural = calc->getUnitName(name_, this, force);
	}
	calc->unitPluralChanged(this);
}
void Unit::shortName(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_.empty())
		sshortname = name_;
	else if(name_ != sshortname) {
		sshortname = calc->getUnitName(name_, this, force);
	}
	calc->unitShortNameChanged(this);

}
string Unit::category() {
	return scategory;
}
string Unit::name() {
	return sname;
}
string Unit::plural() {
	if(hasPlural())
		return splural;
	else
		return sname;
}
bool Unit::hasPlural() {
	return !splural.empty();
}
bool Unit::hasShortName() {
	return !sshortname.empty();
}
string Unit::baseName() {
	return name();
}
string Unit::baseExpName() {
	return name();
}
string Unit::shortName(bool plural_) {
	if(hasShortName())
		return sshortname;
	if(plural_ && hasPlural())
		return splural;
	else
		return sname;
}
string Unit::shortBaseName() {
	return shortName();
}
string Unit::shortBaseExpName() {
	return shortName();
}
Unit* Unit::baseUnit() {
	return this;
}
Manager *Unit::baseValue(Manager *value_, Manager *exp_) {
	if(!value_) value_ = new Manager(calc, 1.0L);	
//	value_->add(1, POWER_CH);
	return value_;
}
Manager *Unit::convertToBase(Manager *value_, Manager *exp_) {
	if(!value_) value_ = new Manager(calc, 1.0L);
//	value_->add(-1, POWER_CH);
	return value_;
}
long double Unit::baseExp(long double exp_) {
	return exp_;
}
char Unit::type() const {
	return 'U';
}
bool Unit::isChildOf(Unit *u) {
	return false;
}
bool Unit::isParentOf(Unit *u) {
	return u != this && u->baseUnit() == this;
}
bool Unit::hasComplexRelationTo(Unit *u) {
	if(u == this || u->baseUnit() != this) return false;
	Unit *fbu = u;
	if(fbu->type() != 'A') return false;
	while(1) {
		if(fbu == this) return false;
		if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
		if(fbu->type() != 'A') return false;
		fbu = ((AliasUnit*) fbu)->firstBaseUnit();
	}
}
Manager *Unit::convert(Unit *u, Manager *value_, Manager *exp_, bool *converted) {
	if(!value_) value_ = new Manager(calc, 1.0L);
	if(!exp_) exp_ = new Manager(calc, 1.0L);
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

AliasUnit::AliasUnit(Calculator *calc_, string cat_, string name_, string plural_, string short_name_, string title_, Unit *alias, string relation, long double exp_, string reverse) : Unit(calc_, cat_, name_, plural_, short_name_, title_) {
	unit = alias;
	remove_blank_ends(relation);
	remove_blank_ends(reverse);
	value = relation;
	rvalue = reverse;
	d_exp = exp_;
}
AliasUnit::~AliasUnit() {}
string AliasUnit::baseName() {
	return unit->baseName();
}
string AliasUnit::baseExpName() {
	if(baseExp() != 1) {
		string str = unit->baseName();
		str += POWER_STR;
		str += d2s(baseExp(), PRECISION);
		return str;
	} else {
		return unit->baseName();
	}
}
string AliasUnit::shortBaseName() {
	return unit->shortBaseName();
}
string AliasUnit::shortBaseExpName() {
	if(baseExp() != 1) {
		string str = unit->shortBaseName();
		str += POWER_STR;
		str += d2s(baseExp(), PRECISION);
		return str;
	} else {
		return unit->shortBaseName();
	}
}
Unit* AliasUnit::baseUnit() {
	return unit->baseUnit();
}
string AliasUnit::firstBaseName() {
	return unit->name();
}
string AliasUnit::firstBaseExpName() {
	if(d_exp != 1) {
		string str = unit->name();
		str += POWER_STR;
		str += d2s(d_exp, PRECISION);
		return str;
	} else
		return unit->name();
}
string AliasUnit::firstShortBaseName() {
	return unit->shortName();
}
string AliasUnit::firstShortBaseExpName() {
	if(d_exp != 1) {
		string str = unit->shortName();
		str += POWER_STR;
		str += d2s(d_exp, PRECISION);
		return str;
	} else
		return unit->shortName();
}
Unit* AliasUnit::firstBaseUnit() {
	return unit;
}
void AliasUnit::baseUnit(Unit *alias) {
	unit = alias;
}
string AliasUnit::expression() {
	return value;
}
string AliasUnit::reverseExpression() {
	return rvalue;
}
void AliasUnit::expression(string relation) {
	remove_blank_ends(relation);
	if(relation.empty())
		value = "1";
	else
		value = relation;
}
void AliasUnit::reverseExpression(string reverse) {
	remove_blank_ends(reverse);
	rvalue = reverse;
}
Manager *AliasUnit::baseValue(Manager *value_, Manager *exp_) {
	if(!exp_) exp_ = new Manager(calc, 1.0L);		
	else exp_->ref();
	firstBaseValue(value_, exp_);
	exp_->add(d_exp, MULTIPLICATION_CH);	
	unit->baseValue(value_, exp_);
	exp_->unref();
	return value_;
}
Manager *AliasUnit::convertToBase(Manager *value_, Manager *exp_) {
	if(!exp_) exp_ = new Manager(calc, 1.0L);		
	else exp_->ref();
	exp_->add(d_exp, DIVISION_CH);	
	convertToFirstBase(value_, exp_);	
	unit->convertToBase(value_, exp_);
	exp_->unref();	
	return value_;
}
long double AliasUnit::baseExp(long double exp_) {
	return unit->baseExp(exp_ * d_exp);
}
Manager *AliasUnit::convertToFirstBase(Manager *value_, Manager *exp_) {
	if(!value_) value_ = new Manager(calc, 1.0L);
	if(!exp_) exp_ = new Manager(calc, 1.0L);		
	else exp_->ref();
	if(rvalue.empty()) {
		if(value.find(FUNCTION_VAR_X) != string::npos) {
			string stmp = value;
			string stmp2 = LEFT_BRACKET_STR;
			stmp2 += value_->print();
			stmp2 += RIGHT_BRACKET_STR;
			gsub(FUNCTION_VAR_X, stmp2, stmp);
			stmp2 = LEFT_BRACKET_STR;
			stmp2 += exp_->print();
			stmp2 += RIGHT_BRACKET_STR;
			gsub(FUNCTION_VAR_Y, stmp2, stmp);
			Manager *mngr = calc->calculate(stmp);
//			value_->add(mngr, DIVISION_CH);
			value_->moveto(mngr);
			mngr->unref();
		} else {
			Manager *mngr = calc->calculate(value);
			mngr->add(exp_, POWER_CH);
			value_->add(mngr, DIVISION_CH);
//			value_->moveto(mngr);
			mngr->unref();
		}
	} else {
		if(rvalue.find(FUNCTION_VAR_X) != string::npos) {
			string stmp = rvalue;
			string stmp2 = LEFT_BRACKET_STR;
			stmp2 += value_->print();
			stmp2 += RIGHT_BRACKET_STR;
			gsub(FUNCTION_VAR_X, stmp2, stmp);
			stmp2 = LEFT_BRACKET_STR;
			stmp2 += exp_->print();
			stmp2 += RIGHT_BRACKET_STR;
			gsub(FUNCTION_VAR_Y, stmp2, stmp);
			Manager *mngr = calc->calculate(stmp);
//			value_->add(mngr, MULTIPLICATION_CH);
			value_->moveto(mngr);
			mngr->unref();
		} else {
			Manager *mngr = calc->calculate(rvalue);
			mngr->add(exp_, POWER_CH);
			value_->add(mngr, MULTIPLICATION_CH);
//			mngr->add(value_, MULTIPLICATION_CH);
//			value_->moveto(mngr);
			mngr->unref();
		}
	}
	exp_->unref();
	return value_;
}
Manager *AliasUnit::firstBaseValue(Manager *value_, Manager *exp_) {
	if(!value_) value_ = new Manager(calc, 1.0L);
	if(!exp_) exp_ = new Manager(calc, 1.0L);		
	else exp_->ref();
	if(value.find(FUNCTION_VAR_X) != string::npos) {
		string stmp = value;
		string stmp2 = LEFT_BRACKET_STR;
		stmp2 += value_->print();
		stmp2 += RIGHT_BRACKET_STR;
		gsub(FUNCTION_VAR_X, stmp2, stmp);
		stmp2 = LEFT_BRACKET_STR;
		stmp2 += exp_->print();
		stmp2 += RIGHT_BRACKET_STR;
		gsub(FUNCTION_VAR_Y, stmp2, stmp);
		Manager *mngr = calc->calculate(stmp);
		value_->moveto(mngr);
		mngr->unref();
	} else {
		Manager *mngr = calc->calculate(value);
		mngr->add(exp_, POWER_CH);
		mngr->add(value_, MULTIPLICATION_CH);
		value_->moveto(mngr);
		mngr->unref();
	}
	exp_->unref();
	return value_;
}
void AliasUnit::exp(long double exp_) {
	d_exp = exp_;
}
long double AliasUnit::firstBaseExp() {
	return d_exp;
}
char AliasUnit::type() const {
	return 'A';
}
bool AliasUnit::isChildOf(Unit *u) {
	if(u == this) return false;
	if(baseUnit() == u) return true;
	if(u->baseUnit() != baseUnit()) return false;
	Unit *u2 = this;
	while(1) {
		u2 = ((AliasUnit*) u2)->firstBaseUnit();
		if(u == u2) return true;
		if(u2->type() != 'A') return false;
	}
	return false;
}
bool AliasUnit::isParentOf(Unit *u) {
	if(u == this) return false;
	if(u->baseUnit() != baseUnit()) return false;
	while(1) {
		if(u->type() != 'A') return false;
		u = ((AliasUnit*) u)->firstBaseUnit();
		if(u == this) return true;
	}
	return false;
}
bool AliasUnit::hasComplexExpression() {
	return value.find(FUNCTION_VAR_X) != string::npos;
}
bool AliasUnit::hasComplexRelationTo(Unit *u) {
	if(u == this || u->baseUnit() != baseUnit()) return false;
	if(isParentOf(u)) {
		Unit *fbu = u;
		while(1) {
			if(fbu == this) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->type() != 'A') return false;
			fbu = ((AliasUnit*) fbu)->firstBaseUnit();			
		}	
	} else if(isChildOf(u)) {
		Unit *fbu = this;
		if(fbu->type() != 'A') return false;
		while(1) {
			if(fbu == u) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->type() != 'A') return false;
			fbu = ((AliasUnit*) fbu)->firstBaseUnit();
		}			
	} else {
		return hasComplexRelationTo(baseUnit()) || u->hasComplexRelationTo(u->baseUnit());
	}
}

AliasUnit_Composite::AliasUnit_Composite(Calculator *calc_, Unit *alias, long double exp_, long double prefix_) : AliasUnit(calc_, "", alias->name(), alias->plural(), alias->shortName(), "", alias, "", exp_, "") {
	prefixv = prefix_;
}
AliasUnit_Composite::~AliasUnit_Composite(void) {}
string AliasUnit_Composite::printShort(bool plural_) {
	if(!firstBaseUnit()->hasShortName())
		return print(plural_);
	string str = "";
	if(prefixv != 1) {
		char c = calc->getSPrefix(prefixv);
		if(c > 0) {
			str = c;
		} else {
			const char *c_str = calc->getLPrefix(prefixv);
			if(c_str) str = c_str;
			else str += d2s(prefixv);			
		}
	}
	str += firstBaseUnit()->shortName();
	return str;
}
string AliasUnit_Composite::print(bool plural_) {
	string str = "";
	if(prefixv != 1) {
		const char *c_str = calc->getLPrefix(prefixv);
		if(c_str) {
			str = c_str;
		} else {
			char c = calc->getSPrefix(prefixv);
			if(c > 0) str = c;
			else str += d2s(prefixv);
		}
	}
	if(plural_)
		str += firstBaseUnit()->plural();
	else
		str += firstBaseUnit()->name();
	return str;
}
long double AliasUnit_Composite::prefixValue(void) {
	return prefixv;
}
void AliasUnit_Composite::set(Unit *u, long double exp_, long double prefix_) {
	baseUnit(u);
	exp(exp_);
	prefixv = prefix_;
}
Manager *AliasUnit_Composite::firstBaseValue(Manager *value_, Manager *exp_) {
	if(!value_) value_ = new Manager(calc, 1.0L);
	if(!exp_) exp_ = new Manager(calc, 1.0L);		
	else exp_->ref();	
	exp_->add(d_exp, DIVISION_CH);		
	Manager *mngr = new Manager(calc, 1.0L);
//	mngr->add(exp_, POWER_CH);
	value_->add(mngr, MULTIPLICATION_CH);
	mngr->unref();
	exp_->unref();
	return value_;
}
Manager *AliasUnit_Composite::convertToFirstBase(Manager *value_, Manager *exp_) {
	if(!value_) value_ = new Manager(calc, 1.0L);
	if(!exp_) exp_ = new Manager(calc, 1.0L);		
	else exp_->ref();
	exp_->add(d_exp, MULTIPLICATION_CH);
	Manager *mngr = new Manager(calc, 1.0L);
//	mngr->add(exp_, POWER_CH);
	value_->add(mngr, DIVISION_CH);
	mngr->unref();
	exp_->unref();
	return value_;
}

CompositeUnit::CompositeUnit(Calculator *calc_, string cat_, string name_, string title_) : Unit(calc_, cat_, name_, "", "", title_) {
	units.clear();
}
CompositeUnit::~CompositeUnit(void) {
	for(int i = 0; i < units.size(); i++)
		delete units[i];
}
void CompositeUnit::add(Unit *u, long double exp_, long double prefix) {
	bsorted = false;
	units.push_back(new AliasUnit_Composite(calc, u, exp_, prefix));
	sort();
}
Unit *CompositeUnit::get(int index, long double *exp_, long double *prefix) {
	sort();
	if(index >= 0 && index < sorted.size()) {
		if(exp_) *exp_ = units[sorted[index]]->firstBaseExp();
		if(prefix) *prefix = units[sorted[index]]->prefixValue();
		return units[sorted[index]];
	}
	return NULL;
}
void CompositeUnit::del(Unit *u) {
	for(int i = 0; i < units.size(); i++) {
		if(units[i]->firstBaseUnit() == u) {
			bsorted = false;
			delete units[i];
			units.erase(units.begin() + i);
		}
	}
	sort();
}
void CompositeUnit::sort() {
	if(bsorted)
		return;
	bool btmp = false;
	sorted.clear();
	int i2;
	for(int i = 0; i < (int) units.size(); i++) {
		btmp = true;
		for(i2 = 0; i2 < i; i2++) {
			if(units[sorted[i2]]->firstBaseExp() < units[i]->firstBaseExp()) {
				sorted.push_back(sorted[i2]);
				sorted[i2] = i;
				btmp = false;
				break;
			}
		}
		if(btmp) {
			sorted.push_back(i);
		}
	}
	bsorted = true;
}
string CompositeUnit::print(bool plural_, bool short_) {
	string str = "";
	sort();
	bool b = false;
	for(int i = 0; i < sorted.size(); i++) {
		if(units[sorted[i]]->firstBaseExp() != 0) {
/*			if(!b && units[sorted[i]]->firstBaseExp() < 0) {
				str += UNIT_DIVISION_CH;
				b = true;
			}*/
			if(units[sorted[i]]->firstBaseExp() < 0) {
				str += DIVISION_CH;
				b = true;
			} else {
				if(i > 0) str += MULTIPLICATION_CH;
				b = false;
			}
			if(short_) {
				if(plural_ && i == 0 && units[sorted[i]]->firstBaseExp() > 0)
					str += units[sorted[i]]->printShort(true);
				else
					str += units[sorted[i]]->printShort(false);
			} else {
				if(plural_ && i == 0 && units[sorted[i]]->firstBaseExp() > 0)
					str += units[sorted[i]]->print(true);
				else
					str += units[sorted[i]]->print(false);
			}
			if(b) {
				if(units[sorted[i]]->firstBaseExp() < -1) {
					str += POWER_STR;
					str += d2s(-units[sorted[i]]->firstBaseExp());
				}
			} else {
				if(units[sorted[i]]->firstBaseExp() > 1) {
					str += POWER_STR;
					str += d2s(units[sorted[i]]->firstBaseExp());
				}
			}
		}
	}
	return str;
}
string CompositeUnit::name(void) {
	return print(false, false);
}
string CompositeUnit::plural(void) {
	return print(true, false);
}
string CompositeUnit::shortName(bool plural_) {
	return print(plural_, true);
}
bool CompositeUnit::hasShortName(void) {
	return true;
}
bool CompositeUnit::hasPlural(void) {
	return true;
}
char CompositeUnit::type() const {
	return 'D';
}
bool CompositeUnit::containsRelativeTo(Unit *u) {
	for(int i = 0; i < units.size(); i++) {
		if(u == units[i] || u->baseUnit() == units[i]->baseUnit()) return true;
	}
	return false;
}
Manager *CompositeUnit::generateManager() {
	return calc->calculate(print(false, true));
}
string CompositeUnit::internalName() {
	return sname;
}

