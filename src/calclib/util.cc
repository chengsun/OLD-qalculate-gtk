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
//#include <langinfo.h>
#include "Fraction.h"

bool eqstr::operator()(const char *s1, const char *s2) const {
	return strcmp(s1, s2) == 0;
}

char buffer[20000];

bool s2date(string str, int &year, int &month, int &day) {
	struct tm time;
	bool b = s2date(str, &time);
	if(b) {
		year = time.tm_year + 1900;
		month = time.tm_mon + 1;
		day = time.tm_mday;	
		return true;
	}
	return false;
}
bool s2date(string str, struct tm *time) {
	if(strptime(str.c_str(), "%x", time) || strptime(str.c_str(), "%Ex", time) || strptime(str.c_str(), "%Y-%m-%d", time) || strptime(str.c_str(), "%m/%d/%Y", time) || strptime(str.c_str(), "%m/%d/%y", time)) {
		return true;
	}
	//char *date_format = nl_langinfo(D_FMT);
	return false;
}
bool isLeapYear(int year) {
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}
int daysPerYear(int year, int basis) {
	switch(basis) {
		case 0: {
			return 360;
		}
		case 1: {
			if(isLeapYear(year)) {
				return 366;
			} else {
				return 365;
			}
		}
		case 2: {
			return 360;
		}		
		case 3: {
			return 365;
		} 
		case 4: {
			return 360;
		}
	}
	return -1;
}

int daysPerMonth(int month, int year) {
	switch(month) {
		case 1: {} case 3: {} case 5: {} case 7: {} case 8: {} case 10: {} case 12: {
			return 31;
		}
		case 2:	{
			if(isLeapYear(year)) return 29;
			else return 28;
		}				
		default: {
			return 30;
		}
	}
}	

Fraction *yearsBetweenDates(string date1, string date2, int basis, bool date_func) {
	if(basis < 0 || basis > 4) return NULL;
	if(basis == 1) {
		int day1, day2, month1, month2, year1, year2;
		if(!s2date(date1, year1, month1, day1)) {
  			return NULL;
		}
		if(!s2date(date2, year2, month2, day2)) {
  			return NULL;
		}		
		if(year1 > year2 || (year1 == year2 && month1 > month2) || (year1 == year2 && month1 == month2 && day1 > day2)) {
			int year3 = year1, month3 = month1, day3 = day1;
			year1 = year2; month1 = month2; day1 = day2;
			year2 = year3; month2 = month3; day2 = day3;		
		}		
		int days = 0;
		if(year1 == year2) {
			days = daysBetweenDates(year1, month1, day1, year2, month2, day2, basis, date_func);
			if(days < 0) return NULL;
			return new Fraction(days, daysPerYear(year1, basis));
		}
		for(int month = 12; month > month1; month--) {
			days += daysPerMonth(month, year1);
		}
		days += daysPerMonth(month1, year1) - day1 + 1;
/*		Fraction *fr = new Fraction(days, daysPerYear(year1, basis));
		year1++;
		if(year1 != year2) {
			Fraction yfr(year2 - year1);
			fr->add(&yfr);
		}
		days = 0;*/
		for(int month = 1; month < month2; month++) {
			days += daysPerMonth(month, year2);
		}
		days += day2 - 1;
		int days_of_years = 0;
		for(int year = year1; year <= year2; year++) {
			days_of_years += daysPerYear(year, basis);
			if(year != year1 && year != year2) {
				days += daysPerYear(year, basis);
			}
		}
		Fraction year_frac(days_of_years, year2 + 1 - year1);
/*		if(days > 0) {
			Fraction fr2(days, daysPerYear(year2, basis));
			fr->add(&fr2);
		}*/
		Fraction *fr = new Fraction(days);
		fr->divide(&year_frac);
		return fr;
	} else {
		int days = daysBetweenDates(date1, date2, basis, date_func);
		if(days < 0) return NULL;
		Fraction *fr = new Fraction(days, daysPerYear(0, basis));	
		return fr;
	}
	return NULL;
}
int daysBetweenDates(string date1, string date2, int basis, bool date_func) {
	int day1, day2, month1, month2, year1, year2;
	if(!s2date(date1, year1, month1, day1)) {
  		return -1;
	}
	if(!s2date(date2, year2, month2, day2)) {
  		return -1;
	}
	return daysBetweenDates(year1, month1, day1, year2, month2, day2, basis, date_func);	
}
int daysBetweenDates(int year1, int month1, int day1, int year2, int month2, int day2, int basis, bool date_func) {
	if(basis < 0 || basis > 4) return -1;
	bool isleap = false;
	int days, months, years;

	if(year1 > year2 || (year1 == year2 && month1 > month2) || (year1 == year2 && month1 == month2 && day1 > day2)) {
		int year3 = year1, month3 = month1, day3 = day1;
		year1 = year2; month1 = month2; day1 = day2;
		year2 = year3; month2 = month3; day2 = day3;		
	}

	years = year2  - year1;
	months = month2 - month1 + years * 12;
	days = day2 - day1;

	isleap = isLeapYear(year1);

	switch(basis) {
		case 0: {
			if(date_func) {
				if(month1 == 2 && ((day1 == 28 && !isleap) || (day1 == 29 && isleap)) && !(month2 == month1 && day1 == day2 && year1 == year2)) {
					if(isleap) return months * 30 + days - 1;
					else return months * 30 + days - 2;
				}
				if(day1 == 31 && day2 < 31) days++;
			} else {
				if(month1 == 2 && month2 != 2 && year1 == year2) {
					if(isleap) return months * 30 + days - 1;
					else return months * 30 + days - 2;
				}			
			}
			return months * 30 + days;
		}
		case 1: {}
		case 2: {}		
		case 3: {
			int month4 = month2;
			bool b;
			if(years > 0) {
				month4 = 12;
				b = true;
			} else {
				b = false;
			}
			for(; month1 < month4 || b; month1++) {
				if(month1 > month4 && b) {
					b = false;
					month1 = 1;
					month4 = month2;
					if(month1 == month2) break;
				}
				if(!b) {
					days += daysPerMonth(month1, year2);
				} else {
					days += daysPerMonth(month1, year1);
				}
			}
			if(years == 0) return days;
			if(basis == 1) {
				for(year1 += 1; year1 < year2; year1++) {
					if(isLeapYear(year1)) days += 366;
					else days += 365;
				} 
				return days;
			}
			if(basis == 2) return (years - 1) * 360 + days;		
			if(basis == 3) return (years - 1) * 365 + days;
		} 
		case 4: {
			if(date_func) {
				if(day2 == 31 && day1 < 31) days--;
				if(day1 == 31 && day2 < 31) days++;
			}
			return months * 30 + days;
		}
	}
	return -1;
	
}

int find_first_not_of(const string &str, unsigned int pos, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, pos); 
	for(int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);	
	for(unsigned int i = pos; i < str.length(); i++) {
		bool b = true;
		for(unsigned int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(unsigned int i3 = 0; i3 < strlen(strs[i2]); i3++) {
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
int find_first_of(const string &str, unsigned int pos, ...) {
	char *strs[10];
	va_list ap;
	va_start(ap, pos); 
	for(unsigned int i = 0; true; i++) {
		strs[i] = va_arg(ap, char*);
		if(strs[i] == NULL) break;
	}
	va_end(ap);	
	for(unsigned int i = pos; i < str.length(); i++) {
		for(unsigned int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(unsigned int i3 = 0; i3 < strlen(strs[i2]); i3++) {
				if(str[i] == strs[i2][i3]) {
					return i;
				}
			}
		}		
	}
	return string::npos;
}
int find_last_not_of(const string &str, unsigned int pos, ...) {
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
		for(unsigned int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(unsigned int i3 = 0; i3 < strlen(strs[i2]); i3++) {
				if(str[i] == strs[i2][i3]) {
					b = false;
				}
			}
			if(b) return i;
		}	
	}
	return string::npos;
}
int find_last_of(const string &str, unsigned int pos, ...) {
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
		for(unsigned int i2 = 0; true; i2++) {
			if(!strs[i2]) break;
			for(unsigned int i3 = 0; i3 < strlen(strs[i2]); i3++) {
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
		i = str.find(pattern, i + sub.length());
	}
	return str;
}
string& gsub(const char *pattern, const char *sub, string &str) {
	unsigned int i = str.find(pattern);
	while(i != string::npos) {
		str.replace(i, strlen(pattern), string(sub));
		i = str.find(pattern, i + strlen(sub));
	}
	return str;
}

string& remove_blanks(string &str) {
	unsigned int i = str.find_first_of(SPACES, 0);
	while(i != string::npos) {
		str.erase(i, 1);
		i = str.find_first_of(SPACES, i);
	}
	return str;
}

string& remove_duplicate_blanks(string &str) {
	unsigned int i = str.find_first_of(SPACES, 0);
	while(i != string::npos) {
		if(i == 0 || is_in(SPACES, str[i - 1])) {
			str.erase(i, 1);
		} else {
			i++;
		}
		i = str.find_first_of(SPACES, i);
	}
	return str;
}

string& remove_blank_ends(string &str) {
	unsigned int i = str.find_first_not_of(SPACES);
	unsigned int i2 = str.find_last_not_of(SPACES);
	if(i != string::npos && i2 != string::npos)
		str = str.substr(i, i2 - i + 1);
	else
		str.resize(0);
	return str;
}
string& remove_parenthesis(string &str) {
	if(str[0] == LEFT_PARENTHESIS_CH && str[str.length() - 1] == RIGHT_PARENTHESIS_CH) {
		str = str.substr(1, str.length() - 2);
		return remove_parenthesis(str);
	}
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
long int s2li(const string& str) {
	long int li = 0;
	bool numbers_started = false, minus = false;		
	for(unsigned int index = 0; index < str.size(); index++) {
		if(str[index] >= '0' && str[index] <= '9') {
			li *= 10;
			li += str[index] - '0';
			numbers_started = true;
		} else if(!numbers_started && str[index] == '-') {
			minus = !minus;
		}
	}
	if(minus) li = -li;		
	return li;
}
long long int s2lli(const string& str) {
	long long int li = 0;
	bool numbers_started = false, minus = false;		
	for(unsigned int index = 0; index < str.size(); index++) {
		if(str[index] >= '0' && str[index] <= '9') {
			li *= 10;
			li += str[index] - '0';
			numbers_started = true;
		} else if(!numbers_started && str[index] == '-') {
			minus = !minus;
		}
	}
	if(minus) li = -li;		
	return li;
}
long int s2li(const char *str) {
	long int li = 0;
	bool numbers_started = false, minus = false;	
	for(unsigned int index = 0; str[index] != '\0'; index++) {
		if(str[index] >= '0' && str[index] <= '9') {
			li *= 10;
			li += str[index] - '0';
			numbers_started = true;
		} else if(!numbers_started && str[index] == '-') {
			minus = !minus;
		}
	}
	if(minus) li = -li;	
	return li;
}
long long int s2lli(const char *str) {
	long long int li = 0;
	bool numbers_started = false, minus = false;	
	for(unsigned int index = 0; str[index] != '\0'; index++) {
		if(str[index] >= '0' && str[index] <= '9') {
			li *= 10;
			li += str[index] - '0';
			numbers_started = true;
		} else if(!numbers_started && str[index] == '-') {
			minus = !minus;
		}
	}
	if(minus) li = -li;	
	return li;
}
int s2i(const string& str) {
	return strtol(str.c_str(), NULL, 10);
}
int s2i(const char *str) {
	return strtol(str, NULL, 10);
}

string lli2s(long long int value) {
	if(value == 0) return "0";
	string str = "";
	bool minus = value < 0;
	if(minus) value =- value;
	while(true) {
		str.insert(str.begin(), 1, '0' + value % 10);
		value /= 10;						
		if(value == 0) break;
	}
	if(minus) str.insert(str.begin(), 1, '-');
	return str;
}
string li2s(long int value) {
	if(value == 0) return "0";
	string str = "";
	bool minus = value < 0;
	if(minus) value =- value;
	while(true) {
		str.insert(str.begin(), 1, '0' + value % 10);
		value /= 10;						
		if(value == 0) break;
	}
	if(minus) str.insert(str.begin(), 1, '-');
	return str;
}
string &lli2s(long long int &value, string &str)  {
	if(value == 0) {
		str = "0";
		return str;
	}
	int pos = str.length();
	bool minus = value < 0;
	if(minus) value =- value;
	while(true) {
		str.insert(str.begin() + pos, 1, '0' + value % 10);
		value /= 10;						
		if(value == 0) break;
	}
	if(minus) str.insert(str.begin() + pos, 1, '-');
	return str;
}

int find_ending_bracket(const string &str, int start, int *missing) {
	int i_l = 1;
	while(true) {
		unsigned start = str.find_first_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, start);
		if(start == string::npos) {
			if(missing) *missing = i_l;
			return string::npos;
		}
		if(str[start] == LEFT_PARENTHESIS_CH) {
			i_l++;
		} else {
			i_l--;
			if(!i_l) {
				if(missing) *missing = i_l;
				return start;
			}
		}
		start++;
	}
}

char op2ch(MathOperation op) {
	switch(op) {
		case OPERATION_ADD: return PLUS_CH;
		case OPERATION_SUBTRACT: return MINUS_CH;		
		case OPERATION_MULTIPLY: return MULTIPLICATION_CH;		
		case OPERATION_DIVIDE: return DIVISION_CH;		
		case OPERATION_RAISE: return POWER_CH;		
		case OPERATION_EXP10: return EXP_CH;
		default: return ' ';		
	}
}

string& wrap_p(string &str) {
	str.insert(str.begin(), 1, LEFT_PARENTHESIS_CH);
	str += RIGHT_PARENTHESIS_CH;
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
		for(unsigned int i2 = 0; i2 < strlen(strs[i]); i2++) { 
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
		for(unsigned int i2 = 0; i2 < strlen(strs[i]); i2++) { 
			if(strs[i][i2] == c)
				return false;
		}
	}
	return true;	
}
bool is_in(const char *str, char c) {
	for(unsigned int i = 0; i < strlen(str); i++) {
		if(str[i] == c)
			return true;
	}
	return false;
}
bool is_not_in(const char *str, char c) {
	for(unsigned int i = 0; i < strlen(str); i++) {
		if(str[i] == c)
			return false;
	}
	return true;
}
bool is_in(const string &str, char c) {
	for(unsigned int i = 0; i < str.length(); i++) {
		if(str[i] == c)
			return true;
	}
	return false;
}
bool is_not_in(const string &str, char c) {
	for(unsigned int i = 0; i < str.length(); i++) {
		if(str[i] == c)
			return false;
	}
	return true;
}

int sign_place(string *str, unsigned int start) {
	unsigned int i = str->find_first_of(OPERATORS, start);
	if(i != string::npos)
		return i;
	else
		return -1;
}

long long int gcd(long long int i1, long long int i2) {
	if(i1 < 0) i1 = -i1;
	if(i2 < 0) i2 = -i2;
	if(i1 == i2) return i2;
	long long int i3;
	if(i2 > i1) {
		i3 = i2;
		i2 = i1;
		i1 = i3;
	}
	while((i3 = i1 % i2) != 0) {
		i1 = i2;
		i2 = i3;
	}
	return i2;
}

/*long double gcd_d(long double i1, long double i2) {
	if(i1 < 0) i1 = -i1;
	if(i2 < 0) i2 = -i2;
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
}*/

bool text_length_is_one(const string &str) {
	for(unsigned int i = 0; i < str.length() - 1; i++) {
		if(str[i] > 0) {
			return false;
		}
	}
	return true;
}
