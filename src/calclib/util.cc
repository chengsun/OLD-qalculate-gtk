/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "util.h"
#include <stdarg.h>

bool eqstr::operator()(const char *s1, const char *s2) const {
	return strcmp(s1, s2) == 0;
}

char buffer[20000];

int find_first_not_of(const string &str, int pos, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, pos); 
	for(int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);	
	for(int i = pos; i < str.length(); i++) {
		bool b = true;
		for(int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(int i3 = 0; i3 < strlen(strs[i2]); i3++) {
				if(str[i] == strs[i2][i3]) {
					b = false;
				}
			}
		}	
		if(b) {
			return i;		
		}
	}
	return string::npos;
}
int find_first_of(const string &str, int pos, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, pos); 
	for(int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);	
	for(int i = pos; i < str.length(); i++) {
		for(int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(int i3 = 0; i3 < strlen(strs[i2]); i3++) {
				if(str[i] == strs[i2][i3]) {
					return i;
				}
			}
		}		
	}
	return string::npos;
}
int find_last_not_of(const string &str, int pos, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, pos); 
	for(int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);	
	if(pos < 0) pos = str.length() - 1;		
	for(int i = pos; i >= 0; i--) {
		bool b = true;
		for(int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(int i3 = 0; i3 < strlen(strs[i2]); i3++) {
				if(str[i] == strs[i2][i3]) {
					b = false;
				}
			}
			if(b) return i;
		}	
	}
	return string::npos;
}
int find_last_of(const string &str, int pos, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, pos); 
	for(int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);
	if(pos < 0) pos = str.length() - 1;	
	for(int i = pos; i >= 0; i--) {
		for(int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(int i3 = 0; i3 < strlen(strs[i2]); i3++) {
				if(str[i] == strs[i2][i3]) {
					return i;
				}
			}
		}		
	}
	return string::npos;
}

string& gsub(const string &pattern, const string &sub, string &str) {
	unsigned int i = str.find(pattern);
	while(i != string::npos) {
		str.replace(i, pattern.length(), sub);
		i = str.find(sub, i + sub.length());
	}
	return str;
}
string& gsub(const char *pattern, const char *sub, string &str) {
	unsigned int i = str.find(pattern);
	while(i != string::npos) {
		str.replace(i, strlen(pattern), sub);
		i = str.find(pattern, i + strlen(sub));
	}
	return str;
}

string& remove_blanks(string &str) {
	int i = str.find_first_of(SPACE_S, 0);
	while(i > -1) {
		str.erase(i, 1);
		i = str.find_first_of(SPACE_S, i);
	}
	return str;
}

string& remove_blank_ends(string &str) {
	unsigned int i = str.find_first_not_of(SPACE_S);
	unsigned int i2 = str.find_last_not_of(SPACE_S);
	if(i != string::npos && i2 != string::npos)
		str = str.substr(i, i2 - i + 1);
	else
		str.resize(0);
	return str;
}

long double rad2deg(long double &value) {
	return value * 180 / PI_VALUE;
}
long double deg2rad(long double &value) {
	return value * PI_VALUE / 180;
}
long double rad2gra(long double &value) {
	return value * 200 / PI_VALUE;
}
long double gra2rad(long double &value) {
	return value * PI_VALUE / 200;
}
long double deg2gra(long double &value) {
	return value * 400 / 360;
}
long double gra2deg(long double &value) {
	return value * 360 / 400;
}

string d2s(long double value, int precision) {
	//	  qgcvt(value, precision, buffer);
	sprintf(buffer, "%.*LG", precision, value);
	string stmp = buffer;
	//	  gsub("e", "E", stmp);
	return stmp;
}

string i2s(int value) {
	//	  char buffer[10];
	sprintf(buffer, "%i", value);
	string stmp = buffer;
	return stmp;
}

int s2i(const string& str) {
	return strtol(str.c_str(), NULL, 10);
}
int s2i(const char *str) {
	return strtol(str, NULL, 10);
}
string& wrap_p(string &str) {
	str.insert(0, LEFT_BRACKET_STR);
	str.append(RIGHT_BRACKET_STR);
	return str;
}
bool is_in(char c, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, c); 
	for(int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);	
	for(int i = 0; true; i++) {
		if(!strs[i]) break;
		for(int i2 = 0; i2 < strlen(strs[i]); i2++) { 
			if(strs[i][i2] == c)
				return true;
		}
	}
	return false;
}
bool is_not_in(char c, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, c); 
	for(int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);	
	for(int i = 0; true; i++) {
		if(!strs[i]) break;
		for(int i2 = 0; i2 < strlen(strs[i]); i2++) { 
			if(strs[i][i2] == c)
				return false;
		}
	}
	return true;	
}
bool is_in(const char *str, char c) {
	for(int i = 0; i < strlen(str); i++) {
		if(str[i] == c)
			return true;
	}
	return false;
}
bool is_not_in(const char *str, char c) {
	for(int i = 0; i < strlen(str); i++) {
		if(str[i] == c)
			return false;
	}
	return true;
}

int sign_place(string *str, unsigned int start) {
	int i = str->find_first_of(OPERATORS_S, start);
	if(i != (int) string::npos)
		return i;
	else
		return -1;
}

long double gcd(long double i1, long double i2) {
	if(i1 == i2) return i2;
	long double i3;
	if(i2 > i1) {
		i3 = i2;
		i2 = i1;
		i1 = i3;
	}
	while((i3 =fmodl(i1, i2)) != 0.0L) {
		i1 = i2;
		i2 = i3;
	}
	return i2;
}

