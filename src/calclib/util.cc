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
		i = str.find(pattern, i + sub.length());
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
long int s2li(const string& str) {
	long int li = 0;
	bool numbers_started = false, minus = false;		
	for(int index = 0; index < str.size(); index++) {
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
	for(int index = 0; index < str.size(); index++) {
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
	for(int index = 0; str[index] != '\0'; index++) {
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
	for(int index = 0; str[index] != '\0'; index++) {
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
		str.insert(0, 1, '0' + value % 10);
		value /= 10;						
		if(value == 0) break;
	}
	if(minus) str.insert(0, 1, '-');
	return str;
}
string li2s(long int value) {
	if(value == 0) return "0";
	string str = "";
	bool minus = value < 0;
	if(minus) value =- value;
	while(true) {
		str.insert(0, 1, '0' + value % 10);
		value /= 10;						
		if(value == 0) break;
	}
	if(minus) str.insert(0, 1, '-');
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
		str.insert(pos, 1, '0' + value % 10);
		value /= 10;						
		if(value == 0) break;
	}
	if(minus) str.insert(pos, 1, '-');
	return str;
}
long long int llpow(long long int base, long long int exp, bool &overflow) {
	if(exp < 0) {
		overflow = true;
		return LONG_LONG_MIN;
	}
	if(exp == 0) return 1;
	if(exp == 1) return base;
	long double d_test = powl(base, exp);
	if(d_test > LONG_LONG_MAX || d_test < LONG_LONG_MIN)  {
		overflow = true;
		return LONG_LONG_MAX;
	}	
	long long int value = base;				
	for(; exp > 1; exp--) {
		value *= base;
	}
	return value;
}

char op2ch(MathOperation op) {
	switch(op) {
		case ADD: return PLUS_CH;
		case SUBTRACT: return MINUS_CH;		
		case MULTIPLY: return MULTIPLICATION_CH;		
		case DIVIDE: return DIVISION_CH;		
		case RAISE: return POWER_CH;		
		case EXP10: return EXP_CH;		
	}
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

long double gcd_d(long double i1, long double i2) {
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
}
string &remove_trailing_zeros(string &str, int decimals_to_keep, bool expand, bool decrease) {
	int i2 = str.find_first_of(DOT_S);
	if(i2 != string::npos) {
		string str2 = "";
		int i4 = str.find_first_not_of(NUMBERS_S, i2 + 1);
		if(i4 != string::npos) {
			str2 = str.substr(i4, str.length() - i4);
			str = str.substr(0, i4);
		}
		int expands = 0;
		int decimals = str.length() - i2 - 1;
		int i3 = 1;
		while(str[str.length() - i3] == ZERO_CH) {
			i3++;
		}
		i3--;
		int dtk = decimals_to_keep;
		if(decimals_to_keep) {
			decimals_to_keep = decimals_to_keep - (decimals - i3);
			if(decimals_to_keep > 0) {
				expands = decimals_to_keep - i3;
				if(expands < 0)
					expands = 0;
				i3 -= decimals_to_keep;
				if(i3 < 0)
					i3 = 0;
			}
		}
		if(expand && expands) {
			while(expands > 0) {
				str += ZERO_STR;
				expands--;
			}
		} else {
			if(is_in(str[str.length() - i3], DOT_S, NULL))
				i3++;
			if(i3) {
				str = str.substr(0, str.length() - i3);
			}
		}
		if(decrease) {
			if(dtk > 0) {
				if(i2 + dtk + 1 < str.length()) {
					if(str.length() > i2 + dtk + 1 && str[i2 + dtk + 1] >= FIVE_CH) {
						i3 = dtk;
						while(i3 + i2 >= 0) {
							if(str[i2 + i3] == NINE_CH)  {
								str[i2 + i3] = ZERO_CH;
								if(i3 + i2 == 0) {
									str.insert(0, 1, ONE_CH);
									i2++;
									break;
								}
							} else {
								str[i2 + i3] = str[i2 + i3] + 1;
								break;
							}
							i3--;
							if(i3 == 0)
								i3--;
						}
					}
					str = str.substr(0, i2 + dtk + 1);
				}
			} else {
				if(str.length() > i2 + 1 && str[i2 + 1] >= FIVE_CH) {
					i3 = i2 - 1;
					while(i3 >= 0) {
						if(str[i3] == NINE_CH)  {
							str[i3] = ZERO_CH;
							if(i3 == 0) {
								str.insert(0, 1, ONE_CH);
								i2++;
								break;
							}
						} else {
							str[i3] = str[i3] + 1;
							break;
						}
						i3--;
					}
				}
				str = str.substr(0, i2);
			}
		}
		if(is_in(str[str.length() - 1], DOT_S, NULL))
			str = str.substr(0, str.length() - 1);
		str += str2;
	} else if(expand) {
		string str2 = "";
		int i4 = str.find_first_not_of(NUMBERS_S, 0);
		if(i4 != string::npos) {
			str2 = str.substr(i4, str.length() - i4);
			str = str.substr(0, i4);
		}
		if(decimals_to_keep > 0)
			str += DOT_STR;
		while(decimals_to_keep > 0) {
			str += ZERO_STR;
			decimals_to_keep--;
		}
		str += str2;
	}
	return str;
}

