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

ExpressionItem::ExpressionItem(string cat_, string name_, string title_, string descr_, bool is_local, bool is_builtin, bool is_active, string unicode_name) {
	b_local = is_local;
	b_builtin = is_builtin;
	remove_blank_ends(name_);
	remove_blank_ends(cat_);
	remove_blank_ends(title_);
	sname = name_;
	uname = unicode_name;
	stitle = title_;
	scat = cat_;
	sdescr = descr_;
	b_changed = false;
	b_approx = false;
	b_active = is_active;
	b_registered = false;
	b_hidden = false;
	b_destroyed = false;
	i_ref = 0;
}
ExpressionItem::ExpressionItem() {
	b_changed = false;
	b_approx = false;
	b_active = true;
	b_local = true;
	b_builtin = false;
	b_registered = false;	
	b_hidden = false;
	b_destroyed = false;
	i_ref = 0;
}
ExpressionItem::~ExpressionItem() {
}
void ExpressionItem::set(const ExpressionItem *item) {
	b_changed = item->hasChanged();
	b_approx = item->isApproximate();
	b_active = item->isActive();
	sname = item->name(false);
	uname = item->unicodeName(false);
	stitle = item->title(false);
	scat = item->category();
	sdescr = item->description();
	b_local = item->isLocal();
	b_builtin = item->isBuiltin();
	b_hidden = item->isHidden();
}
bool ExpressionItem::destroy() {
	CALCULATOR->expressionItemDeleted(this);
	if(v_refs.size() > 0) {
		return false;
	} else if(i_ref > 0) {
		b_destroyed = true;
	} else {
		delete this;
	}
	return true;
}
bool ExpressionItem::isRegistered() const {
	return b_registered;
}
void ExpressionItem::setRegistered(bool is_registered) {
	b_registered = is_registered;
}
const string &ExpressionItem::title(bool return_name_if_no_title, bool use_unicode) const {
	if(return_name_if_no_title && stitle.empty()) {
		return name(use_unicode);
	}
	return stitle;
}
void ExpressionItem::setTitle(string title_) {
	remove_blank_ends(title_);
	if(stitle != title_) {
		stitle = title_;
		b_changed = true;
	}
}
const string &ExpressionItem::description() const {
	return sdescr;
}
void ExpressionItem::setDescription(string descr_) {
	remove_blank_ends(descr_);
	if(sdescr != descr_) {
		sdescr = descr_;
		b_changed = true;
	}
}
void ExpressionItem::setName(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != sname) {
		sname = CALCULATOR->getName(name_, this, force);
		b_changed = true;
	}
	CALCULATOR->nameChanged(this);
}
void ExpressionItem::setUnicodeName(string name_, bool force) {
	remove_blank_ends(name_);
	if(name_ != uname) {
		uname = CALCULATOR->getName(name_, this, force);
		b_changed = true;
	}
	CALCULATOR->nameChanged(this);
}
const string &ExpressionItem::name(bool use_unicode) const {
	if(use_unicode && !uname.empty()) return uname;
	return sname;
}
const string &ExpressionItem::unicodeName(bool return_name_if_no_unicode) const {
	if(return_name_if_no_unicode && uname.empty()) return sname;
	return uname;
}
const string &ExpressionItem::referenceName() const {
	return name();
}
const string &ExpressionItem::category() const {
	return scat;
}
void ExpressionItem::setCategory(string cat_) {
	remove_blank_ends(cat_);
	if(scat != cat_) {
		scat = cat_;
		b_changed = true;
	}
}
bool ExpressionItem::isLocal() const {
	return b_local;
}
bool ExpressionItem::setLocal(bool is_local, int will_be_active) {
	if(is_local != b_local) {
		if(!b_local) {
			bool was_active = b_active;
			b_active = false;	
			ExpressionItem *item = copy();
			b_local = is_local;	
			b_active = was_active;
			if(will_be_active) {
				setActive(true);
			} else if(will_be_active == 0) {
				setActive(false);
			}
			CALCULATOR->addExpressionItem(item);		
			if(was_active != item->isActive()) {
				item->setChanged(true);
			}
			if(was_active && will_be_active == 0) {
				item->setActive(true);
			}
		}
		b_local = is_local;
	} else if(will_be_active >= 0) {
		setActive(will_be_active);
	}
	return true;
}
bool ExpressionItem::isBuiltin() const {
	return b_builtin;
}
bool ExpressionItem::hasChanged() const {
	return b_changed;
}
void ExpressionItem::setChanged(bool has_changed) {
	b_changed = has_changed;
}
bool ExpressionItem::isApproximate() const {
	return b_approx;
}
void ExpressionItem::setApproximate(bool is_approx) {
	if(is_approx != b_approx) {
		b_approx = is_approx;
		b_changed = true;	
	}
}
bool ExpressionItem::isActive() const {
	return b_active;
}
void ExpressionItem::setActive(bool is_active) {
	if(is_active != b_active) {
		b_active = is_active;			
		if(b_registered) {
			if(is_active) {
				CALCULATOR->expressionItemActivated(this);		
			} else {
				CALCULATOR->expressionItemDeactivated(this);
			}
		}
		b_changed = true;
	}
}
bool ExpressionItem::isHidden() const {
	return b_hidden;
}
void ExpressionItem::setHidden(bool is_hidden) {
	if(is_hidden != b_hidden) {
		b_hidden = is_hidden;			
		b_changed = true;
	}
}

int ExpressionItem::refcount() const {
	return i_ref;
}
void ExpressionItem::ref() {
	i_ref++;
}
void ExpressionItem::unref() {
	i_ref--;
	if(b_destroyed && i_ref <= 0) {
		delete this;
	}
}
void ExpressionItem::ref(ExpressionItem *o) {
	i_ref++;
	v_refs.push_back(o);
}
void ExpressionItem::unref(ExpressionItem *o) {
	for(unsigned int i = 0; i < v_refs.size(); i++) {
		if(v_refs[i] == o) {
			i_ref--;
			v_refs.erase(v_refs.begin() + i);
			break;
		}
	}
}
ExpressionItem *ExpressionItem::getReferencer(unsigned int index) const {
	if(index > 0 && index <= v_refs.size()) {
		return v_refs[index - 1];
	}
	return NULL;
}
bool ExpressionItem::changeReference(ExpressionItem *o_from, ExpressionItem *o_to) {
	return false;
}
