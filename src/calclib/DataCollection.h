/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef DATA_COLLECTION_H
#define DATA_COLELCTION_H

#include "includes.h"
#include "Function.h"

typedef Sgi::vector<DataProperty*>::iterator DataObjectPropertyIter;

class DataObject {

  protected:

	vector<DataProperty*> properties;
	vector<string> s_properties;
	vector<string> s_nonlocalized_properties;
	vector<MathStructure*> m_properties;
	vector<int> a_properties;
	DataCollection *parent;
	
  public:
  
	DataObject(DataCollection *parent_collection);

	void setProperty(DataProperty *property, string s_value, int is_approximate = -1);
	void setNonlocalizedKeyProperty(DataProperty *property, string s_value);
	
	const MathStructure *getPropertyStruct(DataProperty *property);
	const string &getProperty(DataProperty *property, int *is_approximate = NULL);
	const string &getNonlocalizedKeyProperty(DataProperty *property);
	string getPropertyInputString(DataProperty *property);
	string getPropertyDisplayString(DataProperty *property);
	
	DataCollection *parentCollection() const;

};

typedef enum {
	PROPERTY_EXPRESSION,
	PROPERTY_NUMBER,
	PROPERTY_STRING
} PropertyType;

class DataProperty {

  protected:

	vector<string> names;
	string sdescr, stitle, sunit;
	MathStructure *m_unit;
	bool b_approximate, b_brackets, b_key, b_case, b_hide;
	DataCollection *parent;
	PropertyType ptype;
	
  public:
  
	DataProperty(DataCollection *parent_collection, string s_name = "", string s_title = "", string s_description = "");
	
	void setName(string s_name);
	void clearNames();
	void addName(string s_name, unsigned int index = 0);
	bool hasName(const string &s_name);
	unsigned int countNames() const;
	const string &getName(unsigned int index = 1) const;
	void setTitle(string s_title);
	const string &title(bool return_name_if_no_title = true) const;
	void setDescription(string s_description);
	const string &description() const;
	void setUnit(string s_unit);
	const string &getUnitString() const;
	const MathStructure *getUnitStruct();
	string getInputString(const string &valuestr);
	string getDisplayString(const string &valuestr);
	MathStructure *generateStruct(const string &valuestr, int is_approximate = -1);
	void setKey(bool is_key = true);
	bool isKey() const;
	void setHidden(bool is_hidden = true);
	bool isHidden() const;
	void setCaseSensitive(bool is_case_sensitive = true);
	bool isCaseSensitive() const;
	void setUsesBrackets(bool uses_brackets = true);
	bool usesBrackets() const;
	void setApproximate(bool is_approximate = true);
	bool isApproximate() const;
	void setPropertyType(PropertyType property_type);
	PropertyType propertyType() const;
	DataCollection *parentCollection() const;
	
};

typedef vector<DataProperty*>::iterator DataPropertyIter;
typedef vector<DataObject*>::iterator DataObjectIter;

class DataCollection : public Function {

  protected:
  
	string sfile, scopyright;
	bool b_loaded;
	vector<DataProperty*> properties;
	vector<DataObject*> objects;
	
  public:
  
  	DataCollection(string s_category = "", string s_name = "", string s_default_file = "", string s_title = "", string s_description = "");
	DataCollection(const DataCollection *o);
	
	ExpressionItem *copy() const;
	void set(const ExpressionItem *item);
	int subtype() const;
	
	void setCopyright(string s_copyright);
	const string &copyright() const;
	void setDefaultDataFile(string s_file);
	const string &defaultDataFile() const;
	
	void setDefaultProperty(string property);
	const string &defaultProperty() const;
	
	int calculate(MathStructure &mstruct, const MathStructure &vargs, const EvaluationOptions &eo);
	
	bool loadObjects(const char *file_name = NULL);
	bool objectsLoaded() const;
	
	void addProperty(DataProperty *dp);
	DataProperty *getPrimaryKeyProperty();
	DataProperty *getProperty(string property);
	DataProperty *getFirstProperty(DataPropertyIter *it);
	DataProperty *getNextProperty(DataPropertyIter *it);
	const string &getFirstPropertyName(DataPropertyIter *it);
	const string &getNextPropertyName(DataPropertyIter *it);
	
	void addObject(DataObject *o);
	DataObject *getObject(string object);
	DataObject *getObject(const MathStructure &object);
	DataObject *getFirstObject(DataObjectIter *it);
	DataObject *getNextObject(DataObjectIter *it);
	
	const MathStructure *getObjectProperyStruct(string property, string object);
	const string &getObjectProperty(string property, string object);
	string getObjectPropertyInputString(string property, string object);
	string getObjectPropertyDisplayString(string property, string object);
	
	string printProperties(string object);
	string printProperties(DataObject *o);
		
};

class DataPropertyArgument : public Argument {

  protected:
  
  	DataCollection *o_data;
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	DataPropertyArgument(DataCollection *data_collection, string name_ = "", bool does_test = true, bool does_error = true);
	DataPropertyArgument(const DataPropertyArgument *arg);
	~DataPropertyArgument();
	int type() const;
	Argument *copy() const;
	string print() const;
	DataCollection *dataCollection() const;
	void setDataCollection(DataCollection *data_collection);
	
};

class DataObjectArgument : public Argument {

  protected:
  
  	DataCollection *o_data;
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	DataObjectArgument(DataCollection *data_collection, string name_ = "", bool does_test = true, bool does_error = true);
	DataObjectArgument(const DataObjectArgument *arg);
	~DataObjectArgument();
	int type() const;
	Argument *copy() const;
	string print() const;
	DataCollection *dataCollection() const;
	void setDataCollection(DataCollection *data_collection);
	
};


#endif
