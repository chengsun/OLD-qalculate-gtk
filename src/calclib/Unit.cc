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
#include "Calculator.h"
#include "Manager.h"
#include "Prefix.h"

Unit::Unit(string cat_, string name_, string plural_, string singular_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, name_, title_, "", is_local, is_builtin, is_active) {
	remove_blank_ends(plural_);
	remove_blank_ends(singular_);
	ssingular = singular_;
	splural = plural_;
}
Unit::Unit() {
	ssingular = "";
	splural = "";
}
Unit::Unit(const Unit *unit) {
	set(unit);
}
Unit::~Unit() {}
ExpressionItem *Unit::copy() const {
	return new Unit(this);
}
void Unit::set(const ExpressionItem *item) {
	if(item->type() == TYPE_UNIT) {
		splural = ((Unit*) item)->plural(false);
		ssingular = ((Unit*) item)->singular(false);
	}
	ExpressionItem::set(item);
}
bool Unit::isUsedByOtherUnits() const {
	return CALCULATOR->unitIsUsedByOtherUnits(this);
}
void Unit::setPlural(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != splural) {
		splural = CALCULATOR->getName(name_, this, force);
		setChanged(true);
	}
	CALCULATOR->unitPluralChanged(this);
}
void Unit::setSingular(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_.empty()) {
		ssingular = name_;
	} else if(name_ != ssingular) {
		ssingular = CALCULATOR->getName(name_, this, force);
	}
	setChanged(true);
	CALCULATOR->unitSingularChanged(this);

}
const string &Unit::singular(bool return_short_if_no_singular) const {
	if(return_short_if_no_singular && ssingular.empty()) {
		return shortName();
	}
	return ssingular;
}
const string &Unit::plural(bool return_singular_if_no_plural) const {
	if(return_singular_if_no_plural && splural.empty()) {
		return singular();
	}
	return splural;
}
const string &Unit::shortName() const {
	return name();
}
const Unit* Unit::baseUnit() const {
	return this;
}
Manager *Unit::baseValue(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);	
//	value_->add(1, OPERATION_RAISE);
	return value_;
}
Manager *Unit::convertToBase(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
//	value_->add(-1, OPERATION_RAISE);
	return value_;
}
long int Unit::baseExp(long int exp_) const {
	return exp_;
}
int Unit::type() const {
	return TYPE_UNIT;
}
int Unit::unitType() const {
	return BASE_UNIT;
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
	if(fbu->unitType() != ALIAS_UNIT) return false;
	while(1) {
		if(fbu == this) return false;
		if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
		if(fbu->unitType() != ALIAS_UNIT) return false;
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
	} else if(u->unitType() == COMPOSITE_UNIT) {
		CompositeUnit *cu = (CompositeUnit*) u;
		for(int i = 0; i < cu->units.size(); i++) {
			if(convert(cu->units[i], value_, exp_)) b = true;
		}
	}
	if(CALCULATOR->alwaysExact() && !value_->isPrecise()) b = false;
	exp_->unref();
	if(converted) *converted = b;
	return value_;
}

AliasUnit::AliasUnit(string cat_, string name_, string plural_, string short_name_, string title_, const Unit *alias, string relation, long int exp_, string reverse, bool is_local, bool is_builtin, bool is_active) : Unit(cat_, name_, plural_, short_name_, title_, is_local, is_builtin, is_active) {
	unit = (Unit*) alias;
	remove_blank_ends(relation);
	remove_blank_ends(reverse);
	value = relation;
	rvalue = reverse;
	exp = exp_;
	exp_mngr = new Manager(exp, 1);
}
AliasUnit::AliasUnit() {
	unit = NULL;
	value = "";
	rvalue = "";
	exp = 1;
	exp_mngr = new Manager(exp, 1);	
}
AliasUnit::AliasUnit(const AliasUnit *unit) {
	exp_mngr = new Manager(exp, 1);
	set(unit);
}
AliasUnit::~AliasUnit() {
	exp_mngr->unref();
}
ExpressionItem *AliasUnit::copy() const {
	return new AliasUnit(this);
}
void AliasUnit::set(const ExpressionItem *item) {
	if(item->type() == TYPE_UNIT) {
		Unit::set(item);
		if(((Unit*) item)->unitType() == ALIAS_UNIT) {
			AliasUnit *u = (AliasUnit*) item;
			unit = (Unit*) u->firstBaseUnit();
			exp = u->firstBaseExp();
			value = u->expression();
			rvalue = u->reverseExpression();
			exp_mngr->set(exp, 1);
		}
	} else {
		ExpressionItem::set(item);
	}
}
const Unit* AliasUnit::baseUnit() const {
	return unit->baseUnit();
}
const Unit* AliasUnit::firstBaseUnit() const {
	return unit;
}
void AliasUnit::setBaseUnit(const Unit *alias) {
	unit = (Unit*) alias;
	setChanged(true);
}
string AliasUnit::expression() const {
	return value;
}
string AliasUnit::reverseExpression() const {
	return rvalue;
}
void AliasUnit::setExpression(string relation) {
	remove_blank_ends(relation);
	if(relation.empty()) {
		value = "1";
	} else {
		value = relation;
	}
	setChanged(true);
}
void AliasUnit::setReverseExpression(string reverse) {
	remove_blank_ends(reverse);
	rvalue = reverse;
	setChanged(true);
}
Manager *AliasUnit::baseValue(Manager *value_, Manager *exp_) const {
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	firstBaseValue(value_, exp_);
	exp_->add(exp_mngr, OPERATION_MULTIPLY);	
	unit->baseValue(value_, exp_);
	exp_->unref();
	return value_;
}
Manager *AliasUnit::convertToBase(Manager *value_, Manager *exp_) const {
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	exp_->add(exp_mngr, OPERATION_DIVIDE);	
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
	bool was_rpn = CALCULATOR->inRPNMode();
	CALCULATOR->setRPNMode(false);
	if(rvalue.empty()) {
		if(value.find("\\x") != string::npos) {
			string stmp = value;
			string stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int x_id = CALCULATOR->addId(value_, true);
			stmp2 += i2s(x_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\x", stmp2, stmp);
			stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int y_id = CALCULATOR->addId(exp_, true);
			stmp2 += i2s(y_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\y", stmp2, stmp);
			Manager *mngr = CALCULATOR->calculate(stmp);
			CALCULATOR->delId(x_id, true);
			CALCULATOR->delId(y_id, true);
			value_->moveto(mngr);
			mngr->unref();
		} else {
			Manager *mngr = CALCULATOR->calculate(value);
			mngr->add(exp_, OPERATION_RAISE);
			value_->add(mngr, OPERATION_DIVIDE);
			mngr->unref();
		}
	} else {
		if(rvalue.find("\\x") != string::npos) {
			string stmp = rvalue;
			string stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int x_id = CALCULATOR->addId(value_, true);
			stmp2 += i2s(x_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\x", stmp2, stmp);
			stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int y_id = CALCULATOR->addId(exp_, true);
			stmp2 += i2s(y_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\y", stmp2, stmp);
			Manager *mngr = CALCULATOR->calculate(stmp);
			CALCULATOR->delId(x_id, true);
			CALCULATOR->delId(y_id, true);			
			value_->moveto(mngr);
			mngr->unref();
		} else {
			Manager *mngr = CALCULATOR->calculate(rvalue);
			mngr->add(exp_, OPERATION_RAISE);
			value_->add(mngr, OPERATION_MULTIPLY);
			mngr->unref();
		}
	}
	CALCULATOR->setRPNMode(was_rpn);		
	exp_->unref();
	if(!isPrecise()) value_->setPrecise(false);
	return value_;
}
Manager *AliasUnit::firstBaseValue(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	bool was_rpn = CALCULATOR->inRPNMode();
	CALCULATOR->setRPNMode(false);
	if(value.find("\\x") != string::npos) {
		string stmp = value;
		string stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
		int x_id = CALCULATOR->addId(value_, true);
		stmp2 += i2s(x_id);
		stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
		gsub("\\x", stmp2, stmp);
		stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
		int y_id = CALCULATOR->addId(exp_, true);
		stmp2 += i2s(y_id);
		stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
		gsub("\\y", stmp2, stmp);
		Manager *mngr = CALCULATOR->calculate(stmp);
		CALCULATOR->delId(x_id, true);
		CALCULATOR->delId(y_id, true);
		value_->moveto(mngr);
		mngr->unref();
	} else {
		Manager *mngr = CALCULATOR->calculate(value);
		mngr->add(exp_, OPERATION_RAISE);
		mngr->add(value_, OPERATION_MULTIPLY);
		value_->moveto(mngr);
		mngr->unref();
	}
	CALCULATOR->setRPNMode(was_rpn);	
	exp_->unref();
	if(!isPrecise()) value_->setPrecise(false);	
	return value_;
}
void AliasUnit::setExponent(long int exp_) {
	exp = exp_;
	exp_mngr->set(exp, 1);
	setChanged(true);
}
long int AliasUnit::firstBaseExp() const {
	return exp;
}
int AliasUnit::unitType() const {
	return ALIAS_UNIT;
}
bool AliasUnit::isChildOf(const Unit *u) const {
	if(u == this) return false;
	if(baseUnit() == u) return true;
	if(u->baseUnit() != baseUnit()) return false;
	Unit *u2 = (Unit*) this;
	while(1) {
		u2 = (Unit*) ((AliasUnit*) u2)->firstBaseUnit();
		if(u == u2) return true;
		if(u2->unitType() != ALIAS_UNIT) return false;
	}
	return false;
}
bool AliasUnit::isParentOf(const Unit *u) const {
	if(u == this) return false;
	if(u->baseUnit() != baseUnit()) return false;
	while(1) {
		if(u->unitType() != ALIAS_UNIT) return false;
		u = ((AliasUnit*) u)->firstBaseUnit();
		if(u == this) return true;
	}
	return false;
}
bool AliasUnit::hasComplexExpression() const {
	return value.find("\\x") != string::npos;
}
bool AliasUnit::hasComplexRelationTo(const Unit *u) const {
	if(u == this || u->baseUnit() != baseUnit()) return false;
	if(isParentOf(u)) {
		Unit *fbu = (Unit*) u;
		while(1) {
			if(fbu == this) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->unitType() != ALIAS_UNIT) return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();			
		}	
	} else if(isChildOf(u)) {
		Unit *fbu = (Unit*) this;
		if(fbu->unitType() != ALIAS_UNIT) return false;
		while(1) {
			if(fbu == u) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->unitType() != ALIAS_UNIT) return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();
		}			
	} else {
		return hasComplexRelationTo(baseUnit()) || u->hasComplexRelationTo(u->baseUnit());
	}
}

AliasUnit_Composite::AliasUnit_Composite(const Unit *alias, long int exp_, const Prefix *prefix_) : AliasUnit("", alias->name(), alias->plural(false), alias->singular(false), "", alias, "", exp_, "") {
	prefixv = (Prefix*) prefix_;
}
AliasUnit_Composite::AliasUnit_Composite(const AliasUnit_Composite *unit) {
	set(unit);
}
AliasUnit_Composite::~AliasUnit_Composite() {}
ExpressionItem *AliasUnit_Composite::copy() const {
	return new AliasUnit_Composite(this);
}
void AliasUnit_Composite::set(const ExpressionItem *item) {
	if(item->type() == TYPE_UNIT) {
		if(((Unit*) item)->unitType() == ALIAS_UNIT) {
			AliasUnit::set(item);
			prefixv = (Prefix*) ((AliasUnit_Composite*) item)->prefix();
		} else {
			Unit::set(item);
		}
	} else {
		ExpressionItem::set(item);
	}
}
string AliasUnit_Composite::printShort(bool plural_) const {
	string str = "";
	if(prefixv) {
		str += prefixv->name(true);
	}
	str += firstBaseUnit()->shortName();
	return str;
}
string AliasUnit_Composite::print(bool plural_) const {
	string str = "";
	if(prefixv) {
		str += prefixv->name(false);
	}
	if(plural_) {
		str += firstBaseUnit()->plural();
	} else {
		str += firstBaseUnit()->singular();
	}
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
	exp_->add(exp_mngr, OPERATION_DIVIDE);		
	Manager *mngr = new Manager(1, 1);
//	mngr->add(exp_, RAISE);
	value_->add(mngr, OPERATION_MULTIPLY);
	mngr->unref();
	exp_->unref();
	return value_;
}
Manager *AliasUnit_Composite::convertToFirstBase(Manager *value_, Manager *exp_) const {
	if(!value_) value_ = new Manager(1, 1);
	if(!exp_) exp_ = new Manager(1, 1);		
	else exp_->ref();
	exp_->add(exp_mngr, OPERATION_MULTIPLY);
	Manager *mngr = new Manager(1, 1);
//	mngr->add(exp_, RAISE);
	value_->add(mngr, OPERATION_DIVIDE);
	mngr->unref();
	exp_->unref();
	return value_;
}

CompositeUnit::CompositeUnit(string cat_, string name_, string title_, string base_expression_, bool is_local, bool is_builtin, bool is_active) : Unit(cat_, name_, "", "", title_, is_local, is_builtin, is_active) {
	setBaseExpression(base_expression_);
	setChanged(false);
}
CompositeUnit::CompositeUnit(const CompositeUnit *unit) {
	set(unit);
}
CompositeUnit::~CompositeUnit() {
	for(int i = 0; i < units.size(); i++) {
		delete units[i];
	}
}
ExpressionItem *CompositeUnit::copy() const {
	return new CompositeUnit(this);
}
void CompositeUnit::set(const ExpressionItem *item) {
	if(item->type() == TYPE_UNIT) {
		Unit::set(item);
		if(((Unit*) item)->unitType() == COMPOSITE_UNIT) {
			CompositeUnit *u = (CompositeUnit*) item;
			for(int i = 0; i < u->units.size(); i++) {
				units.push_back(new AliasUnit_Composite(u->units[i]));
			}
		}
		updateNames();
	} else {
		ExpressionItem::set(item);
	}
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
	updateNames();
}
Unit *CompositeUnit::get(int index, long int *exp_, Prefix **prefix) const {
	if(index >= 0 && index < units.size()) {
		if(exp_) *exp_ = units[index]->firstBaseExp();
		if(prefix) *prefix = (Prefix*) units[index]->prefix();
		return units[index];
	}
	return NULL;
}
int CompositeUnit::countUnits() const {
	units.size();
}
void CompositeUnit::del(Unit *u) {
	for(int i = 0; i < units.size(); i++) {
		if(units[i]->firstBaseUnit() == u) {
			delete units[i];
			units.erase(units.begin() + i);
		}
	}
	updateNames();
}
string CompositeUnit::print(bool plural_, bool short_) const {
	string str = "";
	bool b = false, b2 = false;
	for(int i = 0; i < units.size(); i++) {
		if(units[i]->firstBaseExp() != 0) {
			if(!b && units[i]->firstBaseExp() < 0 && i > 0) {
				str += "/";
				b = true;
				if(i < units.size() - 1) {
					b2 = true;
					str += "(";
				}				
			} else {
//				if(i > 0) str += "*";
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
					str += "^";
					str += li2s(-units[i]->firstBaseExp());
				}
			} else {
				if(units[i]->firstBaseExp() != 1) {
					str += "^";
					str += li2s(units[i]->firstBaseExp());
				}
			}
		}
	}
	if(b2) str += ")";
	return str;
}
const string &CompositeUnit::plural(bool return_name_if_no_plural) const {
	return splural;
}
const string &CompositeUnit::singular(bool return_short_if_no_singular) const {
	return ssingular;
}
const string &CompositeUnit::shortName() const {
	return sshort;
}
int CompositeUnit::unitType() const {
	return COMPOSITE_UNIT;
}
bool CompositeUnit::containsRelativeTo(const Unit *u) const {
	if(u == this) return false;
	CompositeUnit *cu;
	for(int i = 0; i < units.size(); i++) {
		if(u == units[i] || u->baseUnit() == units[i]->baseUnit()) return true;
		if(units[i]->baseUnit()->unitType() == COMPOSITE_UNIT) {
			cu = (CompositeUnit*) units[i]->baseUnit();
			if(cu->containsRelativeTo(u)) return true;
		}
	}
	if(u->unitType() == COMPOSITE_UNIT) {
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
		if(units.size() > 0) mngr->setType(MULTIPLICATION_MANAGER);
		for(int i = 0; i < units.size(); i++) {
			if(units[i]->firstBaseExp() != 1) {
				Manager *mngr2 = new Manager();
				mngr2->setType(POWER_MANAGER);
				if(units[i]->prefix()) {
					Manager *mngr3 = new Manager();
					mngr3->setType(MULTIPLICATION_MANAGER);
					mngr3->push_back(new Manager(1, 1, units[i]->prefix()->exponent()));
					mngr3->push_back(new Manager(units[i]->firstBaseUnit()));
					mngr2->push_back(mngr3);
				} else {				
					mngr2->push_back(new Manager(units[i]->firstBaseUnit()));
				}
				mngr2->push_back(new Manager(units[i]->firstBaseExp(), 1));
				mngr->push_back(mngr2);				
			} else {
				if(units[i]->prefix()) {
					mngr->push_back(new Manager(1, 1, units[i]->prefix()->exponent()));
				}
				mngr->push_back(new Manager(units[i]->firstBaseUnit()));
			}
		}
		return mngr;
	}
}
void CompositeUnit::setBaseExpression(string base_expression_) {
	units.clear();
	if(base_expression_.empty()) {
		setChanged(true);
		updateNames();
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
			if(mngr->isMultiplication() && mngr->countChilds() == 2 && mngr->getChild(0)->isFraction() && mngr->getChild(1)->isUnit()) {
				prefix = CALCULATOR->getExactPrefix(mngr->getChild(0)->fraction());
				mngr = mngr->getChild(1);
			} 
			if(mngr->isUnit()) {
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
				add(mngr->unit(), exp, prefix);
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
	setChanged(true);
	updateNames();
}
void CompositeUnit::updateNames() {
	sshort = print(false, true);
	splural = print(true, false);
	ssingular = print(false, false);
}

