/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef UTIL_H
#define UTIL_H

#include <libqalculate/includes.h>

struct eqstr {
    bool operator()(const char *s1, const char *s2) const;	
};

string& gsub(const string &pattern, const string &sub, string &str);
string& gsub(const char *pattern, const char *sub, string &str);
string d2s(double value, int precision = 100);
string i2s(int value);
string i2s(long int value);
string i2s(unsigned int value);
string i2s(unsigned long int value);
string i2s(unsigned long long int value);
const char *b2yn(bool b, bool capital = true);
const char *b2tf(bool b, bool capital = true);
const char *b2oo(bool b, bool capital = true);
string p2s(void *o);
int s2i(const string& str);
int s2i(const char *str);
void *s2p(const string& str);
void *s2p(const char *str);

string date2s(int year, int month, int day);
int week(string str, bool start_sunday = false);
int weekday(string str);
int yearday(string str);
void now(int &hour, int &min, int &sec);
void today(int &year, int &month, int &day);
bool s2date(string str, int &year, int &month, int &day);
bool isLeapYear(int year);
int daysPerYear(int year, int basis = 0);
int daysPerMonth(int month, int year);
Number yearsBetweenDates(string date1, string date2, int basis, bool date_func = true);
int daysBetweenDates(string date1, string date2, int basis, bool date_func = true);
int daysBetweenDates(int year1, int month1, int day1, int year2, int month2, int day2, int basis, bool date_func = true);

size_t find_ending_bracket(const string &str, size_t start, int *missing = NULL);

char op2ch(MathOperation op);

string& wrap_p(string &str);
string& remove_blanks(string &str);
string& remove_duplicate_blanks(string &str);
string& remove_blank_ends(string &str);
string& remove_parenthesis(string &str);

bool is_in(const char *str, char c);
bool is_not_in(const char *str, char c);
bool is_in(const string &str, char c);
bool is_not_in(const string &str, char c);
int sign_place(string *str, size_t start = 0);
int gcd(int i1, int i2);

size_t unicode_length(const string &str);
size_t unicode_length(const char *str);
bool text_length_is_one(const string &str);
bool equalsIgnoreCase(const string &str1, const string &str2);
bool equalsIgnoreCase(const string &str1, const char *str2);

void parse_qalculate_version(string qalculate_version, int *qalculate_version_numbers);

string getLocalDir();

#endif
