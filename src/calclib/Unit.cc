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
#include "MathStructure.h"
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
Unit* Unit::baseUnit() const {
	return (Unit*) this;
}
MathStructure &Unit::baseValue(MathStructure &mvalue, MathStructure &mexp) const {
	return mvalue;
}
MathStructure &Unit::convertToBase(MathStructure &mvalue, MathStructure &mexp) const {
	return mvalue;
}
MathStructure &Unit::baseValue(MathStructure &mvalue) const {
	return mvalue;
}
MathStructure &Unit::convertToBase(MathStructure &mvalue) const {
	return mvalue;
}
MathStructure Unit::baseValue() const {
	return MathStructure(1, 1);
}
MathStructure Unit::convertToBase() const {
	return MathStructure(1, 1);
}
int Unit::baseExp(int exp_) const {
	return exp_;
}
int Unit::type() const {
	return TYPE_UNIT;
}
int Unit::unitType() const {
	return BASE_UNIT;
}
bool Unit::isChildOf(Unit *u) const {
	return false;
}
bool Unit::isParentOf(Unit *u) const {
	return u != this && u->baseUnit() == this;
}
bool Unit::hasComplexRelationTo(Unit *u) const {
	if(u == this || u->baseUnit() != this) return false;
	Unit *fbu = u;
	if(fbu->unitType() != ALIAS_UNIT) return false;
	while(1) {
		if(fbu == this) return false;
		if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
		if(fbu->unitType() != ALIAS_UNIT) return false;
		fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();
	}
}
MathStructure Unit::convert(Unit *u, bool *converted) const {
	MathStructure mexp(1, 1);
	MathStructure mvalue(1, 1);
	return convert(u, mvalue, mexp, converted);
}
MathStructure &Unit::convert(Unit *u, MathStructure &mvalue, bool *converted) const {
	MathStructure mexp(1, 1);
	return convert(u, mvalue, mexp, converted);
}
MathStructure &Unit::convert(Unit *u, MathStructure &mvalue, MathStructure &mexp, bool *converted) const {
	bool b = false;
	if(u->baseUnit() == baseUnit()) {
		u->baseValue(mvalue, mexp);
		convertToBase(mvalue, mexp);
		b = true;
	} else if(u->unitType() == COMPOSITE_UNIT) {
		bool b2 = false;
		CompositeUnit *cu = (CompositeUnit*) u;
		for(unsigned int i = 0; i < cu->units.size(); i++) {
			convert(cu->units[i], mvalue, mexp, &b2);
			if(b2) b = true;
		}
	}
	//if(CALCULATOR->alwaysExact() && mvalue->isApproximate()) b = false;
	if(converted) *converted = b;
	return mvalue;
}

AliasUnit::AliasUnit(string cat_, string name_, string plural_, string short_name_, string title_, Unit *alias, string relation, int exp_, string reverse, bool is_local, bool is_builtin, bool is_active) : Unit(cat_, name_, plural_, short_name_, title_, is_local, is_builtin, is_active) {
	unit = (Unit*) alias;
	remove_blank_ends(relation);
	remove_blank_ends(reverse);
	value = relation;
	rvalue = reverse;
	exp = exp_;
}
AliasUnit::AliasUnit() {
	unit = NULL;
	value = "";
	rvalue = "";
	exp = 1;
}
AliasUnit::AliasUnit(const AliasUnit *unit) {
	set(unit);
}
AliasUnit::~AliasUnit() {}
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
		}
	} else {
		ExpressionItem::set(item);
	}
}
Unit* AliasUnit::baseUnit() const {
	return unit->baseUnit();
}
Unit* AliasUnit::firstBaseUnit() const {
	return unit;
}
void AliasUnit::setBaseUnit(Unit *alias) {
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
MathStructure &AliasUnit::baseValue(MathStructure &mvalue, MathStructure &mexp) const {
	firstBaseValue(mvalue, mexp);
	if(exp != 1) {
		mexp.multiply(exp);	
	}
	return unit->baseValue(mvalue, mexp);
}
MathStructure &AliasUnit::convertToBase(MathStructure &mvalue, MathStructure &mexp) const {
	Unit *u = (Unit*) baseUnit();
	AliasUnit *u2;
	while(true) {
		u2 = (AliasUnit*) this;
		while(true) {
			if(u2->firstBaseUnit() == u) {
				break;
			} else {
				u2 = (AliasUnit*) u2->firstBaseUnit();
			}
		}
		u = u2;
		u2->convertToFirstBase(mvalue, mexp);
		if(u == this) break;
	}	
	return mvalue;
}
MathStructure &AliasUnit::baseValue(MathStructure &mvalue) const {
	MathStructure mexp(1, 1);
	return baseValue(mvalue, mexp);
}
MathStructure &AliasUnit::convertToBase(MathStructure &mvalue) const {
	MathStructure mexp(1, 1);
	return convertToBase(mvalue, mexp);
}
MathStructure AliasUnit::baseValue() const {
	MathStructure mexp(1, 1);
	MathStructure mvalue(1, 1);
	return baseValue(mvalue, mexp);
}
MathStructure AliasUnit::convertToBase() const {
	MathStructure mexp(1, 1);
	MathStructure mvalue(1, 1);
	return convertToBase(mvalue, mexp);
}

int AliasUnit::baseExp(int exp_) const {
	return unit->baseExp(exp_ * exp);
}
MathStructure &AliasUnit::convertToFirstBase(MathStructure &mvalue, MathStructure &mexp) const {
	bool was_rpn = CALCULATOR->inRPNMode();
	CALCULATOR->setRPNMode(false);
	mexp /= exp;
	if(rvalue.empty()) {
		if(value.find("\\x") != string::npos) {
			string stmp = value;
			string stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int x_id = CALCULATOR->addId(mvalue, true);
			stmp2 += i2s(x_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\x", stmp2, stmp);
			stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int y_id = CALCULATOR->addId(mexp, true);
			stmp2 += i2s(y_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\y", stmp2, stmp);
			mvalue = CALCULATOR->parse(stmp);
			CALCULATOR->delId(x_id, true);
			CALCULATOR->delId(y_id, true);
		} else {
			MathStructure mstruct = CALCULATOR->parse(value);
			mstruct ^= mexp;
			mvalue /= mstruct;
		}
	} else {
		if(rvalue.find("\\x") != string::npos) {
			string stmp = rvalue;
			string stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int x_id = CALCULATOR->addId(mvalue, true);
			stmp2 += i2s(x_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\x", stmp2, stmp);
			stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
			int y_id = CALCULATOR->addId(mexp, true);
			stmp2 += i2s(y_id);
			stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
			gsub("\\y", stmp2, stmp);
			mvalue = CALCULATOR->parse(stmp);
			CALCULATOR->delId(x_id, true);
			CALCULATOR->delId(y_id, true);			
		} else {
			MathStructure mstruct = CALCULATOR->parse(rvalue);
			mstruct ^= mexp;
			mvalue *= mstruct;
		}
	}
	CALCULATOR->setRPNMode(was_rpn);		
	if(isApproximate()) mvalue.setApproximate();
	return mvalue;
}
MathStructure &AliasUnit::firstBaseValue(MathStructure &mvalue, MathStructure &mexp) const {
	bool was_rpn = CALCULATOR->inRPNMode();
	CALCULATOR->setRPNMode(false);
	if(value.find("\\x") != string::npos) {
		string stmp = value;
		string stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
		int x_id = CALCULATOR->addId(mvalue, true);
		stmp2 += i2s(x_id);
		stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
		gsub("\\x", stmp2, stmp);
		stmp2 = LEFT_PARENTHESIS ID_WRAP_LEFT;
		int y_id = CALCULATOR->addId(mexp, true);
		stmp2 += i2s(y_id);
		stmp2 += ID_WRAP_RIGHT RIGHT_PARENTHESIS;
		gsub("\\y", stmp2, stmp);
		mvalue = CALCULATOR->parse(stmp);
		CALCULATOR->delId(x_id, true);
		CALCULATOR->delId(y_id, true);
	} else {
		MathStructure mstruct = CALCULATOR->parse(value);
		mstruct ^= mexp;
		mstruct *= mvalue;
		mvalue = mstruct;
	}
	CALCULATOR->setRPNMode(was_rpn);	
	if(isApproximate()) mvalue.setApproximate();	
	return mvalue;
}
void AliasUnit::setExponent(int exp_) {
	exp = exp_;
	setChanged(true);
}
int AliasUnit::firstBaseExp() const {
	return exp;
}
int AliasUnit::unitType() const {
	return ALIAS_UNIT;
}
bool AliasUnit::isChildOf(Unit *u) const {
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
bool AliasUnit::isParentOf(Unit *u) const {
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
bool AliasUnit::hasComplexRelationTo(Unit *u) const {
	if(u == this || u->baseUnit() != baseUnit()) return false;
	if(isParentOf(u)) {
		Unit *fbu = u;
		while(true) {
			if((const Unit*) fbu == this) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->unitType() != ALIAS_UNIT) return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();			
		}	
	} else if(isChildOf(u)) {
		Unit *fbu = (Unit*) this;
		if(fbu->unitType() != ALIAS_UNIT) return false;
		while(true) {
			if((const Unit*) fbu == u) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->unitType() != ALIAS_UNIT) return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();
		}			
	} else {
		return hasComplexRelationTo(baseUnit()) || u->hasComplexRelationTo(u->baseUnit());
	}
}

AliasUnit_Composite::AliasUnit_Composite(Unit *alias, int exp_, const Prefix *prefix_) : AliasUnit("", alias->name(), alias->plural(false), alias->singular(false), "", alias, "", exp_, "") {
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
int AliasUnit_Composite::prefixExponent() const {
	if(prefixv) return prefixv->exponent();
	return 0;
}
void AliasUnit_Composite::set(Unit *u, int exp_, const Prefix *prefix_) {
	setBaseUnit(u);
	setExponent(exp_);
	prefixv = (Prefix*) prefix_;
}
MathStructure &AliasUnit_Composite::firstBaseValue(MathStructure &mvalue, MathStructure &mexp) const {
	//mexp.divide(exp);		
	//MathStructure mstruct(1, 1);
//	mstruct.raise(mexp);
	//mvalue.multiply(mstruct);
	return mvalue;
}
MathStructure &AliasUnit_Composite::convertToFirstBase(MathStructure &mvalue, MathStructure &mexp) const {
//	mexp.multiply(exp);
	//MathStructure mstruct(1, 1);
//	mstruct.add(exp, RAISE);
	//mvalue.add(mstruct, OPERATION_DIVIDE);
	return mvalue;
}

CompositeUnit::CompositeUnit(string cat_, string name_, string title_, string base_expression_, bool is_local, bool is_builtin, bool is_active) : Unit(cat_, name_, "", "", title_, is_local, is_builtin, is_active) {
	setBaseExpression(base_expression_);
	setChanged(false);
}
CompositeUnit::CompositeUnit(const CompositeUnit *unit) {
	set(unit);
}
CompositeUnit::~CompositeUnit() {
	clear();
}
ExpressionItem *CompositeUnit::copy() const {
	return new CompositeUnit(this);
}
void CompositeUnit::set(const ExpressionItem *item) {
	if(item->type() == TYPE_UNIT) {
		Unit::set(item);
		if(((Unit*) item)->unitType() == COMPOSITE_UNIT) {
			CompositeUnit *u = (CompositeUnit*) item;
			for(unsigned int i = 0; i < u->units.size(); i++) {
				units.push_back(new AliasUnit_Composite(u->units[i]));
			}
		}
		updateNames();
	} else {
		ExpressionItem::set(item);
	}
}
void CompositeUnit::add(Unit *u, int exp_, const Prefix *prefix) {
	bool b = false;
	for(unsigned int i = 0; i < units.size(); i++) {
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
Unit *CompositeUnit::get(unsigned int index, int *exp_, Prefix **prefix) const {
	if(index >= 0 && index < units.size()) {
		if(exp_) *exp_ = units[index]->firstBaseExp();
		if(prefix) *prefix = (Prefix*) units[index]->prefix();
		return (Unit*) units[index]->firstBaseUnit();
	}
	return NULL;
}
void CompositeUnit::setExponent(unsigned int index, int exp_) {
	if(index >= 0 && index < units.size()) {
		units[index]->setExponent(exp_);
	}
}
void CompositeUnit::setPrefix(unsigned int index, const Prefix *prefix) {
	if(index >= 0 && index < units.size()) {
		units[index]->set(units[index]->firstBaseUnit(), units[index]->firstBaseExp(), prefix);
	}
}
unsigned int CompositeUnit::countUnits() const {
	return units.size();
}
void CompositeUnit::del(Unit *u) {
	for(unsigned int i = 0; i < units.size(); i++) {
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
	for(unsigned int i = 0; i < units.size(); i++) {
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
					str += i2s(-units[i]->firstBaseExp());
				}
			} else {
				if(units[i]->firstBaseExp() != 1) {
					str += "^";
					str += i2s(units[i]->firstBaseExp());
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
bool CompositeUnit::containsRelativeTo(Unit *u) const {
	if(u == this) return false;
	CompositeUnit *cu;
	for(unsigned int i = 0; i < units.size(); i++) {
		if(u == units[i] || u->baseUnit() == units[i]->baseUnit()) return true;
		if(units[i]->baseUnit()->unitType() == COMPOSITE_UNIT) {
			cu = (CompositeUnit*) units[i]->baseUnit();
			if(cu->containsRelativeTo(u)) return true;
		}
	}
	if(u->unitType() == COMPOSITE_UNIT) {
		cu = (CompositeUnit*) u;
		for(unsigned int i = 0; i < cu->units.size(); i++) {	
			if(containsRelativeTo(cu->units[i]->baseUnit())) return true;
		}
		return false;
	}	
	return false;
}
MathStructure CompositeUnit::generateMathStructure(bool cleaned) const {
	if(cleaned) {
		return CALCULATOR->parse(print(false, true));
	} else {
		MathStructure mstruct;
		for(unsigned int i = 0; i < units.size(); i++) {
			if(units[i]->firstBaseExp() != 1) {
				MathStructure mstruct2;
				if(units[i]->prefix()) {
					mstruct2.set(1, 1, units[i]->prefix()->exponent());
					mstruct2 *= units[i]->firstBaseUnit();
				} else {				
					mstruct2.set(units[i]->firstBaseUnit());
				}
				mstruct2 ^= units[i]->firstBaseExp();
				if(i == 0) mstruct = mstruct2;
				else mstruct *= mstruct2;
			} else {
				if(units[i]->prefix()) {
					if(i == 0) mstruct.set(1, 1, units[i]->prefix()->exponent());
					else mstruct *= MathStructure(1, 1, units[i]->prefix()->exponent());
				}
				if(i == 0 && !units[i]->prefix()) mstruct.set(units[i]->firstBaseUnit());
				else mstruct *= units[i]->firstBaseUnit();
			}
		}
		return mstruct;
	}
}
void CompositeUnit::setBaseExpression(string base_expression_) {
	clear();
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
	//fix!
	CALCULATOR->parse(base_expression_);
	int div_place = base_expression_.find(DIVISION_CH);
	bool div = false;
	Prefix *prefix = NULL;
	int exp = 1;
	int i = 0, i2 = 0, id;
	const MathStructure *mstruct;
	while(true) {
		i = base_expression_.find(ID_WRAP_LEFT_CH, i2);
		if(i == (int) string::npos) {
			if(i2 == 0) {
				CALCULATOR->error(false, _("Error in unitexpression: \"%s\"."), base_expression_.c_str(), NULL);
			} else if((int) base_expression_.length() > i2 + 2) {
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
		if(i2 == (int) string::npos) {
			break;
		}
		id = s2i(base_expression_.substr(i + 1, i2 - i - 1));
		if(!div && div_place != (int) string::npos && i > div_place) {
			div = true;
		}		
		mstruct = CALCULATOR->getId(id);
		if(mstruct) {
			prefix = NULL;
			exp = 1;
			if(mstruct->isMultiplication() && mstruct->countChilds() == 2 && mstruct->getChild(0)->isNumber() && mstruct->getChild(1)->isUnit()) {
				prefix = CALCULATOR->getExactPrefix(mstruct->getChild(0)->number());
				mstruct = mstruct->getChild(1);
			} 
			if(mstruct->isUnit()) {
				if((int) base_expression_.length() > i2 + 3 && base_expression_[i2 + 2] == POWER_CH) {
					if(is_in(NUMBERS, base_expression_[i2 + 3])) {
						exp = s2i(base_expression_.substr(i2 + 3, 1));
						i2 += 2;
					} else if((int) base_expression_.length() > i2 + 4 && is_in(MINUS PLUS, base_expression_[i2 + 3]) && is_in(NUMBERS, base_expression_[i2 + 4])) {
						exp = s2i(base_expression_.substr(i2 + 3, 2));
						i2 += 3;
					}
				}
				if(div) {
					exp = -exp;
				}
				add(mstruct->unit(), exp, prefix);
			} else {
				CALCULATOR->error(false, _("Error in unitexpression: \"%s\"."), mstruct->print().c_str(), NULL);
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
void CompositeUnit::clear() {
	for(unsigned int i = 0; i < units.size(); i++) {
		delete units[i];
	}
	units.clear();
}
