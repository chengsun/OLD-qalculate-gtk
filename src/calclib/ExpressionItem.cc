/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "ExpressionItem.h"
#include "Calculator.h"
#include "util.h"

ExpressionItem::ExpressionItem(string cat_, string name_, string title_, string descr_, bool is_local, bool is_builtin, bool is_active) {
	b_local = is_local;
	b_builtin = is_builtin;
	remove_blank_ends(name_);
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	sname = name_;
	stitle = title_;
	scat = cat_;
	sdescr = descr_;
	b_changed = false;
	b_exact = true;
	b_active = is_active;
}
ExpressionItem::ExpressionItem() {
	b_changed = false;
	b_exact = true;
	b_active = true;
	b_local = true;
	b_builtin = false;
}
ExpressionItem::~ExpressionItem() {
}
void ExpressionItem::set(const ExpressionItem *item) {
	b_changed = item->hasChanged();
	b_exact = item->isPrecise();
	b_active = item->isActive();
	sname = item->name();
	stitle = item->title();
	scat = item->category();
	sdescr = item->description();
	b_local = item->isLocal();
	b_builtin = item->isBuiltin();
}
bool ExpressionItem::destroy() {
	CALCULATOR->expressionItemDeleted(this);
	delete this;
	return true;
}
string ExpressionItem::title(bool return_name_if_no_title) const {
	if(return_name_if_no_title && stitle.empty()) {
		return name();
	}
	return stitle;
}
void ExpressionItem::setTitle(string title_) {
	remove_blank_ends(title_);
	stitle = title_;
	b_changed = true;
}
string ExpressionItem::description() const {
	return sdescr;
}
void ExpressionItem::setDescription(string descr_) {
	remove_blank_ends(descr_);
	sdescr = descr_;
	b_changed = true;
}
void ExpressionItem::setName(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != sname) {
		sname = CALCULATOR->getName(name_, this, force);
		b_changed = true;
	}
	CALCULATOR->nameChanged(this);
}
string ExpressionItem::name() const {
	return sname;
}
string ExpressionItem::referenceName() const {
	return name();
}
string ExpressionItem::category() const {
	return scat;
}
void ExpressionItem::setCategory(string cat_) {
	remove_blank_ends(cat_);
	scat = cat_;
}
bool ExpressionItem::isLocal() const {
	return b_local;
}
bool ExpressionItem::setLocal(bool is_local, int will_be_active) {
	if(is_local != b_local) {
		if(!is_local) {
			bool was_active = b_active;
			b_active = false;
			ExpressionItem *item = copy();
			if(will_be_active < 0) {
				b_active = was_active;
			} else if(will_be_active) {
				b_active = was_active;
				setActive(true);
			} else {
				b_active = was_active;
				setActive(false);
			}
			CALCULATOR->addExpressionItem(item);
			if(was_active && will_be_active == 0) {
				item->setActive(true);
			}
		}
		b_local = is_local;
	} else if(will_be_active >= 0) {
		setActive(will_be_active);
	}
}
bool ExpressionItem::isBuiltin() const {
	return b_builtin;
}
bool ExpressionItem::hasChanged() const {
	return b_changed;
}
bool ExpressionItem::setChanged(bool has_changed) {
	b_changed = has_changed;
}
bool ExpressionItem::isPrecise() const {
	return b_exact;
}
void ExpressionItem::setPrecise(bool is_precise) {
	b_exact = is_precise;
}
bool ExpressionItem::isActive() const {
	return b_active;
}
void ExpressionItem::setActive(bool is_active, bool not_yet_added) {
	if(is_active != b_active) {
		if(!not_yet_added) {
			if(is_active) {
				CALCULATOR->expressionItemActivated(this);		
			} else {
				CALCULATOR->expressionItemDeactivated(this);
			}
		}
		b_active = is_active;		
	}
}

