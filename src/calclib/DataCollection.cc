/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "DataCollection.h"

#include "util.h"
#include "Calculator.h"
#include "MathStructure.h"
#include "Number.h"

DataObject::DataObject(DataCollection *parent_collection) {
	parent = parent_collection;
	b_approximate = false;
}

void DataObject::setProperty(DataProperty *property, string s_value) {
	for(unsigned int i = 0; i < properties.size(); i++) {
		if(properties[i] == property) {
			s_properties[i] = s_value;
			if(m_properties[i]) {
				delete m_properties[i];
				m_properties[i] = NULL;
			}
			return;
		}
	}
	properties.push_back(property);
	s_properties.push_back(s_value);
	m_properties.push_back(NULL);
}
const string &DataObject::getProperty(DataProperty *property) {
	for(unsigned int i = 0; i < properties.size(); i++) {
		if(properties[i] == property) {
			return s_properties[i];
		}
	}
	return empty_string;
}
string DataObject::getPropertyInputString(DataProperty *property) {
	for(unsigned int i = 0; i < properties.size(); i++) {
		if(properties[i] == property) {
			return property->getInputString(s_properties[i]);
		}
	}
	return empty_string;
}
string DataObject::getPropertyDisplayString(DataProperty *property) {
	for(unsigned int i = 0; i < properties.size(); i++) {
		if(properties[i] == property) {
			return property->getDisplayString(s_properties[i]);
		}
	}
	return empty_string;
}
const MathStructure *DataObject::getPropertyStruct(DataProperty *property) {
	for(unsigned int i = 0; i < properties.size(); i++) {
		if(properties[i] == property) {
			if(!m_properties[i]) {
				m_properties.push_back(property->generateStruct(s_properties[i], b_approximate));
			}
			return m_properties[i];
		}
	}
	return NULL;
}


DataProperty::DataProperty(DataCollection *parent_collection, string s_name, string s_title, string s_description) {
	if(!s_name.empty()) names.push_back(s_name);
	stitle = s_title;
	sdescr = s_description;
	parent = parent_collection;
	m_unit = NULL;
	ptype = PROPERTY_EXPRESSION;
	b_key = false; b_case = false; b_hide = false; b_brackets = false; b_approximate = false;
}
	
void DataProperty::setName(string s_name) {
	if(s_name.empty()) return;
	names.clear();
	names.push_back(s_name);
}
void DataProperty::clearNames() {
	names.clear();
}
void DataProperty::addName(string s_name) {
	if(s_name.empty()) return;
	names.push_back(s_name);
}
const string &DataProperty::getName(unsigned int index) const {
	if(index < 1 || index > names.size()) return empty_string;
	return names[index - 1];
}
const string &DataProperty::title(bool return_name_if_no_title) const {
	if(return_name_if_no_title && stitle.empty()) {
		return getName();
	}
	return stitle;
}
void DataProperty::setTitle(string s_title) {
	remove_blank_ends(s_title);
	stitle = s_title;
}
const string &DataProperty::description() const {
	return sdescr;
}
void DataProperty::setDescription(string s_description) {
	remove_blank_ends(s_description);
	sdescr = s_description;
}
void DataProperty::setUnit(string s_unit) {
	sunit = s_unit;
	if(m_unit) delete m_unit;
	m_unit = NULL;
}
const string &DataProperty::getUnitString() const {
	return sunit;
}
const MathStructure *DataProperty::getUnitStruct() {
	if(!m_unit && !sunit.empty()) {
		m_unit = new MathStructure(CALCULATOR->parse(sunit));
	}
	return m_unit;
}
string DataProperty::getInputString(const string &valuestr) {
	string str;
	if(b_brackets && valuestr[0] == '[' && valuestr[valuestr.length() - 1] == ']') {
		str = valuestr.substr(1, valuestr.length() - 2);
	} else {
		str = valuestr;
	}
	if(!sunit.empty()) {
		str += " ";
		str += sunit;
	}
	return str;
}
string DataProperty::getDisplayString(const string &valuestr) {
	string str = valuestr;
	if(!sunit.empty()) {
		str += " ";
		str += sunit;
	}
	return str;
}
MathStructure *DataProperty::generateStruct(const string &valuestr, int is_approximate) {
	MathStructure *mstruct;
	switch(ptype) {
		case PROPERTY_EXPRESSION: {
			ParseOptions po;
			if((b_approximate && is_approximate < 0) || is_approximate) po.read_precision = ALWAYS_READ_PRECISION;
			mstruct = new MathStructure(CALCULATOR->parse(valuestr, po));
			break;
		}
		case PROPERTY_NUMBER: {
			if((b_approximate && is_approximate < 0) || is_approximate) mstruct = new MathStructure(Number(valuestr, 10, ALWAYS_READ_PRECISION));
			else mstruct = new MathStructure(Number(valuestr));
			break;
		}
		case PROPERTY_STRING: {
			mstruct = new MathStructure(valuestr);
			break;
		}
	}
	if(getUnitStruct()) {
		mstruct->multiply(getUnitStruct());
	}
	return mstruct;
}
void DataProperty::setKey(bool is_key) {b_key = is_key;}
bool DataProperty::isKey() const {return b_key;}
void DataProperty::setHidden(bool is_hidden) {b_hide = is_hidden;}
bool DataProperty::isHidden() const {return b_hide;}
void DataProperty::setCaseSensitive(bool is_case_sensitive) {b_case = is_case_sensitive;}
bool DataProperty::isCaseSensitive() const {return b_case;}
void DataProperty::setUsesBrackets(bool uses_brackets) {b_brackets = uses_brackets;}
bool DataProperty::usesBrackets() const {return b_brackets;}
void DataProperty::setApproximate(bool is_approximate) {b_approximate = is_approximate;}
bool DataProperty::isApproximate() const {return b_approximate;}
void DataProperty::setPropertyType(PropertyType property_type) {ptype = property_type;}
PropertyType DataProperty::propertyType() const {return ptype;}


DataCollection::DataCollection(string s_category, string s_name, string s_default_file, string s_title, string s_description) : Function(s_name, 1, 2, s_category, s_title, s_description) {
	sfile = s_default_file;
	setArgumentDefinition(1, new TextArgument(_("Object")));
	setArgumentDefinition(2, new DataPropertyArgument(this, _("Property")));
	setDefaultValue(2, "info");
}
DataCollection::DataCollection(const DataCollection *o) {
	set(o);
}
	
ExpressionItem *DataCollection::copy() const {return new DataCollection(this);}
void DataCollection::set(const ExpressionItem *item) {
	if(item->type() == TYPE_FUNCTION && item->subtype() == SUBTYPE_DATA_COLLECTION) {
	}
	Function::set(item);
}
int DataCollection::subtype() const {
	return SUBTYPE_DATA_COLLECTION;
}
	
int DataCollection::calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo) {
	return 1;
}

void DataCollection::setCopyright(string s_copyright) {
	scopyright = s_copyright;
}
const string &DataCollection::copyright() const {
	return scopyright;
}
	
bool DataCollection::loadObjects(const char *filename) {
	return true;
}
bool DataCollection::objectsLoaded() const {
	return objects.size() > 0;
}
	
DataProperty *DataCollection::getProperty(string property) const {
	return NULL;
}
DataProperty *DataCollection::getFirstProperty(DataPropertyIter *it) const {
	return NULL;
}
DataProperty *DataCollection::getNextProperty(DataPropertyIter *it) const {
	return NULL;
}
const string &DataCollection::getFirstPropertyName(DataPropertyIter *it) const {
	return empty_string;
}
const string &DataCollection::getNextPropertyName(DataPropertyIter *it) const {
	return empty_string;
}
	
DataObject *DataCollection::getObject(string object) const {
	return NULL;
}
DataObject *DataCollection::getFirstObject(DataObjectIter *it) const {
	return NULL;
}
DataObject *DataCollection::getNextObject(DataObjectIter *it) const {
	return NULL;
}
	
const MathStructure *DataCollection::getObjectProperyStruct(string property, string object) {
	return NULL;
}
const string &DataCollection::getObjectProperty(string property, string object) const {
	return empty_string;
}
string DataCollection::getObjectPropertyInputString(string property, string object) const {
	return empty_string;
}
string DataCollection::getObjectPropertyDisplayString(string property, string object) const {
	return empty_string;
}
	
string DataCollection::printProperties(string object) const {
	return empty_string;
}

DataPropertyArgument::DataPropertyArgument(DataCollection *data_collection, string name_, bool does_test, bool does_error) : Argument(name_, does_test, does_error) {
	b_text = true;
	o_data = data_collection;
}
DataPropertyArgument::DataPropertyArgument(const DataPropertyArgument *arg) {set(arg); b_text = true; o_data = arg->dataCollection();}
DataPropertyArgument::~DataPropertyArgument() {}
bool DataPropertyArgument::subtest(MathStructure &value, const EvaluationOptions &eo) const {
	if(!value.isSymbolic()) {
		value.eval(eo);
	}
	return value.isSymbolic() && o_data && o_data->getProperty(value.symbol());
}
int DataPropertyArgument::type() const {return ARGUMENT_TYPE_DATA_PROPERTY;}
Argument *DataPropertyArgument::copy() const {return new DataPropertyArgument(this);}
string DataPropertyArgument::print() const {return _("data property");}
string DataPropertyArgument::subprintlong() const {
	string str = _("name of a data property");
	str += " (";
	DataPropertyIter it;
	DataProperty *o = NULL;
	if(o_data) {
		o = o_data->getFirstProperty(&it);
	}
	if(!o) {
		str += _("no properties available");
	} else {
		while(true) {
			str += o->getName();
			o = o_data->getNextProperty(&it);
			if(!o) break;
			str += ", ";
		}
	}
	str += ")";
	return str;
}
DataCollection *DataPropertyArgument::dataCollection() const {return o_data;}
void DataPropertyArgument::setDataCollection(DataCollection *data_collection) {o_data = data_collection;}


