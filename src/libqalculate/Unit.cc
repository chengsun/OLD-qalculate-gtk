/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "support.h"

#include "Unit.h"
#include "util.h"
#include "Calculator.h"
#include "MathStructure.h"
#include "Prefix.h"

Unit::Unit(string cat_, string name_, string plural_, string singular_, string title_, bool is_local, bool is_builtin, bool is_active) : ExpressionItem(cat_, "", title_, "", is_local, is_builtin, is_active) {
	remove_blank_ends(plural_);
	remove_blank_ends(singular_);
	if(!name_.empty()) {
		names.resize(1);
		names[0].name = name_;
		names[0].unicode = false;
		names[0].abbreviation = true;
		names[0].case_sensitive = true;
		names[0].suffix = false;
		names[0].avoid_input = false;
		names[0].reference = true;
		names[0].plural = false;
	}
	if(!singular_.empty()) {
		names.resize(names.size() + 1);
		names[names.size() - 1].name = singular_;
		names[names.size() - 1].unicode = false;
		names[names.size() - 1].abbreviation = false;
		names[names.size() - 1].case_sensitive = text_length_is_one(names[names.size() - 1].name);
		names[names.size() - 1].suffix = false;
		names[names.size() - 1].avoid_input = false;
		names[names.size() - 1].reference = false;
		names[names.size() - 1].plural = false;
	}
	if(!plural_.empty()) {
		names.resize(names.size() + 1);
		names[names.size() - 1].name = plural_;
		names[names.size() - 1].unicode = false;
		names[names.size() - 1].abbreviation = false;
		names[names.size() - 1].case_sensitive = text_length_is_one(names[names.size() - 1].name);
		names[names.size() - 1].suffix = false;
		names[names.size() - 1].avoid_input = false;
		names[names.size() - 1].reference = false;
		names[names.size() - 1].plural = true;
	}
	b_si = false;
}
Unit::Unit() {
	b_si = false;
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
		b_si = ((Unit*) item)->isSIUnit();
		ssystem = ((Unit*) item)->system();
	}
	ExpressionItem::set(item);
}
bool Unit::isSIUnit() const {
	return b_si;
}
void Unit::setAsSIUnit() {
	if(!b_si) {
		b_si = true;
		ssystem == "SI";
		setChanged(true);
	}
}
void Unit::setSystem(string s_system) {
	if(s_system != ssystem) {
		ssystem = s_system;
		if(ssystem == "SI" || ssystem == "si" || ssystem == "Si") {
			b_si = true;
		} else {
			b_si = false;
		}
		setChanged(true);
	}
}
const string &Unit::system() const {
	return ssystem;
}
bool Unit::isCurrency() const {
	return baseUnit() == CALCULATOR->u_euro;
}
bool Unit::isUsedByOtherUnits() const {
	return CALCULATOR->unitIsUsedByOtherUnits(this);
}
string Unit::print(bool plural_, bool short_, bool use_unicode) const {
	return preferredName(short_, use_unicode, plural_).name;
}
const string &Unit::plural(bool return_singular_if_no_plural, bool use_unicode) const {
	return preferredName(false, use_unicode, true).name;
}
const string &Unit::singular(bool return_short_if_no_singular, bool use_unicode) const {
	return preferredName(false, use_unicode, false).name;
}
const string &Unit::shortName(bool use_unicode) const {
	return preferredName(true, use_unicode, false).name;
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
int Unit::subtype() const {
	return SUBTYPE_BASE_UNIT;
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
	if(fbu->subtype() != SUBTYPE_ALIAS_UNIT) return false;
	while(1) {
		if(fbu == this) return false;
		if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
		if(fbu->subtype() != SUBTYPE_ALIAS_UNIT) return false;
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
		if(isCurrency()) {
			CALCULATOR->checkExchangeRatesDate();
		}
	} else if(u->subtype() == SUBTYPE_COMPOSITE_UNIT) {
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
		if(((Unit*) item)->subtype() == SUBTYPE_ALIAS_UNIT) {
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
	if(exp != 1) mexp /= exp;
	ParseOptions po;
	if(isApproximate() && precision() < 1) {
		po.read_precision = ALWAYS_READ_PRECISION;
	}
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
			mvalue = CALCULATOR->parse(stmp, po);
			CALCULATOR->delId(x_id, true);
			CALCULATOR->delId(y_id, true);
		} else {
			MathStructure mstruct = CALCULATOR->parse(value, po);
			if(!mexp.isOne()) mstruct ^= mexp;
			mvalue.divide(mstruct, true);
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
			mvalue = CALCULATOR->parse(stmp, po);
			CALCULATOR->delId(x_id, true);
			CALCULATOR->delId(y_id, true);			
		} else {
			MathStructure mstruct = CALCULATOR->parse(rvalue, po);
			if(!mexp.isOne()) mstruct ^= mexp;
			mvalue.multiply(mstruct, true);
		}
	}
	if(precision() > 0 && (mvalue.precision() < 1 || precision() < mvalue.precision())) mvalue.setPrecision(precision());
	if(isApproximate()) mvalue.setApproximate();
	return mvalue;
}
MathStructure &AliasUnit::firstBaseValue(MathStructure &mvalue, MathStructure &mexp) const {
	ParseOptions po;
	if(isApproximate() && precision() < 1) {
		po.read_precision = ALWAYS_READ_PRECISION;
	}
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
		mvalue = CALCULATOR->parse(stmp, po);
		CALCULATOR->delId(x_id, true);
		CALCULATOR->delId(y_id, true);
	} else {
		MathStructure mstruct = CALCULATOR->parse(value, po);
		if(!mexp.isOne()) mstruct ^= mexp;
		mstruct.multiply(mvalue, true);
		mvalue = mstruct;
	}
	if(precision() > 0 && (mvalue.precision() < 1 || precision() < mvalue.precision())) mvalue.setPrecision(precision());
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
int AliasUnit::subtype() const {
	return SUBTYPE_ALIAS_UNIT;
}
bool AliasUnit::isChildOf(Unit *u) const {
	if(u == this) return false;
	if(baseUnit() == u) return true;
	if(u->baseUnit() != baseUnit()) return false;
	Unit *u2 = (Unit*) this;
	while(1) {
		u2 = (Unit*) ((AliasUnit*) u2)->firstBaseUnit();
		if(u == u2) return true;
		if(u2->subtype() != SUBTYPE_ALIAS_UNIT) return false;
	}
	return false;
}
bool AliasUnit::isParentOf(Unit *u) const {
	if(u == this) return false;
	if(u->baseUnit() != baseUnit()) return false;
	while(1) {
		if(u->subtype() != SUBTYPE_ALIAS_UNIT) return false;
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
			if(fbu->subtype() != SUBTYPE_ALIAS_UNIT) return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();			
		}	
	} else if(isChildOf(u)) {
		Unit *fbu = (Unit*) this;
		if(fbu->subtype() != SUBTYPE_ALIAS_UNIT) return false;
		while(true) {
			if((const Unit*) fbu == u) return false;
			if(((AliasUnit*) fbu)->hasComplexExpression()) return true;
			if(fbu->subtype() != SUBTYPE_ALIAS_UNIT) return false;
			fbu = (Unit*) ((AliasUnit*) fbu)->firstBaseUnit();
		}			
	} else {
		return hasComplexRelationTo(baseUnit()) || u->hasComplexRelationTo(u->baseUnit());
	}
}

AliasUnit_Composite::AliasUnit_Composite(Unit *alias, int exp_, Prefix *prefix_) : AliasUnit("", alias->name(), alias->plural(false), alias->singular(false), "", alias, "", exp_, "") {
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
		if(((Unit*) item)->subtype() == SUBTYPE_ALIAS_UNIT) {
			AliasUnit::set(item);
			prefixv = (Prefix*) ((AliasUnit_Composite*) item)->prefix();
		} else {
			Unit::set(item);
		}
	} else {
		ExpressionItem::set(item);
	}
}
string AliasUnit_Composite::print(bool plural_, bool short_, bool use_unicode) const {
	string str = "";
	if(prefixv) {
		str += prefixv->name(short_, use_unicode);
	}
	str += preferredName(short_, use_unicode, plural_).name;
	return str;
}
Prefix *AliasUnit_Composite::prefix() const {
	return prefixv;
}
int AliasUnit_Composite::prefixExponent() const {
	if(prefixv) return prefixv->exponent();
	return 0;
}
void AliasUnit_Composite::set(Unit *u, int exp_, Prefix *prefix_) {
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
		if(((Unit*) item)->subtype() == SUBTYPE_COMPOSITE_UNIT) {
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
void CompositeUnit::add(Unit *u, int exp_, Prefix *prefix) {
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
void CompositeUnit::setPrefix(unsigned int index, Prefix *prefix) {
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
string CompositeUnit::print(bool plural_, bool short_, bool use_unicode) const {
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
			if(plural_ && i == 0 && units[i]->firstBaseExp() > 0) {
				str += units[i]->print(true, short_, use_unicode);
			} else {
				str += units[i]->print(false, short_, use_unicode);
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
int CompositeUnit::subtype() const {
	return SUBTYPE_COMPOSITE_UNIT;
}
bool CompositeUnit::containsRelativeTo(Unit *u) const {
	if(u == this) return false;
	CompositeUnit *cu;
	for(unsigned int i = 0; i < units.size(); i++) {
		if(u == units[i] || u->baseUnit() == units[i]->baseUnit()) return true;
		if(units[i]->baseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
			cu = (CompositeUnit*) units[i]->baseUnit();
			if(cu->containsRelativeTo(u)) return true;
		}
	}
	if(u->subtype() == SUBTYPE_COMPOSITE_UNIT) {
		cu = (CompositeUnit*) u;
		for(unsigned int i = 0; i < cu->units.size(); i++) {	
			if(containsRelativeTo(cu->units[i]->baseUnit())) return true;
		}
		return false;
	}	
	return false;
}
MathStructure CompositeUnit::generateMathStructure() const {
	MathStructure mstruct;
	bool has_p = false;
	for(unsigned int i = 0; i < units.size(); i++) {
		if(units[i]->prefix()) {
			has_p = true;
			break;
		}
	}
	for(unsigned int i = 0; i < units.size(); i++) {
		MathStructure mstruct2;
		if(!has_p || units[i]->prefix()) {
			mstruct2.set(units[i]->firstBaseUnit(), units[i]->prefix());
		} else {				
			mstruct2.set(units[i]->firstBaseUnit(), CALCULATOR->null_prefix);
		}
		if(units[i]->firstBaseExp() != 1) mstruct2 ^= units[i]->firstBaseExp();
		if(i == 0) mstruct = mstruct2;
		else mstruct *= mstruct2;
	}
	return mstruct;
}
void CompositeUnit::setBaseExpression(string base_expression_) {
	clear();
	if(base_expression_.empty()) {
		setChanged(true);
		updateNames();
		return;
	}
	//fix!
	EvaluationOptions eo;
	eo.approximation = APPROXIMATION_EXACT;
	eo.sync_units = false;
	eo.keep_prefixes = true;
	ParseOptions po;
	po.variables_enabled = false;
	po.functions_enabled = false;
	po.unknowns_enabled = false;
	MathStructure mstruct(CALCULATOR->parse(base_expression_, po));
	mstruct.eval(eo);
	if(mstruct.isUnit()) {
		add(mstruct.unit(), 1, mstruct.prefix());
	} else if(mstruct.isPower() && mstruct[0].isUnit() && mstruct[1].isInteger()) {
		add(mstruct[0].unit(), mstruct[1].number().intValue(), mstruct[0].prefix());
	} else if(mstruct.isMultiplication()) {
		for(unsigned int i = 0; i < mstruct.size(); i++) {
			if(mstruct[i].isUnit()) {
				add(mstruct[i].unit(), 1, mstruct[i].prefix());
			} else if(mstruct[i].isPower() && mstruct[i][0].isUnit() && mstruct[i][1].isInteger()) {
				add(mstruct[i][0].unit(), mstruct[i][1].number().intValue(), mstruct[i][0].prefix());
			} else {
				CALCULATOR->error(false, _("Error in unitexpression."), NULL);
			}
		}
	} else {
		CALCULATOR->error(false, _("Error in unitexpression."), NULL);
	}
	setChanged(true);
	updateNames();
}
void CompositeUnit::updateNames() {
}
void CompositeUnit::clear() {
	for(unsigned int i = 0; i < units.size(); i++) {
		delete units[i];
	}
	units.clear();
}
