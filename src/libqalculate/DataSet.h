/*
    Qalculate    

    Copyright (C) 2004  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef DATA_SET_H
#define DATA_SET_H

#include <libqalculate/includes.h>
#include <libqalculate/Function.h>

typedef Sgi::vector<DataProperty*>::iterator DataObjectPropertyIter;

class DataObject {

  protected:

	vector<DataProperty*> properties;
	vector<string> s_properties;
	vector<string> s_nonlocalized_properties;
	vector<MathStructure*> m_properties;
	vector<int> a_properties;
	DataSet *parent;
	bool b_uchanged;
	
  public:
  
	DataObject(DataSet *parent_set);

	void eraseProperty(DataProperty *property);
	void setProperty(DataProperty *property, string s_value, int is_approximate = -1);
	void setNonlocalizedKeyProperty(DataProperty *property, string s_value);
	
	const MathStructure *getPropertyStruct(DataProperty *property);
	const string &getProperty(DataProperty *property, int *is_approximate = NULL);
	const string &getNonlocalizedKeyProperty(DataProperty *property);
	string getPropertyInputString(DataProperty *property);
	string getPropertyDisplayString(DataProperty *property);
	
	bool isUserModified() const;
	void setUserModified(bool user_modified = true);
	
	DataSet *parentSet() const;

};

typedef enum {
	PROPERTY_EXPRESSION,
	PROPERTY_NUMBER,
	PROPERTY_STRING
} PropertyType;

class DataProperty {

  protected:

	vector<string> names;
	vector<bool> name_is_ref;
	string sdescr, stitle, sunit;
	MathStructure *m_unit;
	bool b_approximate, b_brackets, b_key, b_case, b_hide;
	DataSet *parent;
	PropertyType ptype;
	bool b_uchanged;
	
  public:
  
	DataProperty(DataSet *parent_set, string s_name = "", string s_title = "", string s_description = "");
	DataProperty(const DataProperty &dp);
	
	void set(const DataProperty &dp);
	void setName(string s_name, bool is_ref = false);
	void setNameIsReference(unsigned int index = 1, bool is_ref = true);
	bool nameIsReference(unsigned int index = 1) const;
	void clearNames();
	void addName(string s_name, bool is_ref = false, unsigned int index = 0);
	bool hasName(const string &s_name);
	unsigned int countNames() const;
	const string &getName(unsigned int index = 1) const;
	const string &getReferenceName() const;
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
	
	bool isUserModified() const;
	void setUserModified(bool user_modified = true);
	
	DataSet *parentSet() const;
	
};

typedef vector<DataProperty*>::iterator DataPropertyIter;
typedef vector<DataObject*>::iterator DataObjectIter;

class DataSet : public MathFunction {

  protected:
  
	string sfile, scopyright;
	bool b_loaded;
	vector<DataProperty*> properties;
	vector<DataObject*> objects;
	
  public:
  
  	DataSet(string s_category = "", string s_name = "", string s_default_file = "", string s_title = "", string s_description = "", bool is_local = true);
	DataSet(const DataSet *o);
	
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
	
	bool loadObjects(const char *file_name = NULL, bool is_user_defs = true);
	int saveObjects(const char *file_name = NULL, bool save_global = false);
	bool objectsLoaded() const;
	void setObjectsLoaded(bool objects_loaded);
	
	void addProperty(DataProperty *dp);
	void delProperty(DataProperty *dp);
	void delProperty(DataPropertyIter *it);
	DataProperty *getPrimaryKeyProperty();
	DataProperty *getProperty(string property);
	DataProperty *getFirstProperty(DataPropertyIter *it);
	DataProperty *getNextProperty(DataPropertyIter *it);
	const string &getFirstPropertyName(DataPropertyIter *it);
	const string &getNextPropertyName(DataPropertyIter *it);
	
	void addObject(DataObject *o);
	void delObject(DataObject *o);
	void delObject(DataObjectIter *it);
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
  
  	DataSet *o_data;
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	DataPropertyArgument(DataSet *data_set, string name_ = "", bool does_test = true, bool does_error = true);
	DataPropertyArgument(const DataPropertyArgument *arg);
	~DataPropertyArgument();
	int type() const;
	Argument *copy() const;
	string print() const;
	DataSet *dataSet() const;
	void setDataSet(DataSet *data_set);
	
};

class DataObjectArgument : public Argument {

  protected:
  
  	DataSet *o_data;
  
	virtual bool subtest(MathStructure &value, const EvaluationOptions &eo) const;  
	virtual string subprintlong() const;

  public:
  
  	DataObjectArgument(DataSet *data_set, string name_ = "", bool does_test = true, bool does_error = true);
	DataObjectArgument(const DataObjectArgument *arg);
	~DataObjectArgument();
	int type() const;
	Argument *copy() const;
	string print() const;
	DataSet *dataSet() const;
	void setDataSet(DataSet *data_set);
	
};

#endif
