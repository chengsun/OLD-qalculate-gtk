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

#include "includes.h"

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif


struct eqstr {
    bool operator()(const char *s1, const char *s2) const;	
};

string& gsub(const string &pattern, const string &sub, string &str);
string& gsub(const char *pattern, const char *sub, string &str);
string d2s(double value, int precision = 100);
string i2s(int value);
int s2i(const string& str);
int s2i(const char *str);
long int s2li(const string& str);
long int s2li(const char *str);
string li2s(long int value);
string &li2s(long int &value, string &str);

bool s2date(string str, int &year, int &month, int &day);
bool isLeapYear(int year);
int daysPerYear(int year, int basis = 0);
int daysPerMonth(int month, int year);
Number *yearsBetweenDates(string date1, string date2, int basis, bool date_func = true);
int daysBetweenDates(string date1, string date2, int basis, bool date_func = true);
int daysBetweenDates(int year1, int month1, int day1, int year2, int month2, int day2, int basis, bool date_func = true);

int find_ending_bracket(const string &str, int start, int *missing = NULL);

char op2ch(MathOperation op);

int find_first_not_of(const string &str, unsigned int pos, ...);
int find_first_of(const string &str, unsigned int pos, ...);
int find_last_not_of(const string &str, unsigned int pos, ...);
int find_last_of(const string &str, unsigned int pos, ...);

double rad2deg(double &value);
double deg2rad(double &value);
double rad2gra(double &value);
double gra2rad(double &value);
double deg2gra(double &value);
double gra2deg(double &value);

string& wrap_p(string &str);
string& remove_blanks(string &str);
string& remove_duplicate_blanks(string &str);
string& remove_blank_ends(string &str);
string& remove_parenthesis(string &str);

bool is_in(char c, ...);
bool is_not_in(char c, ...);
bool is_in(const char *str, char c);
bool is_not_in(const char *str, char c);
bool is_in(const string &str, char c);
bool is_not_in(const string &str, char c);
int sign_place(string *str, unsigned int start = 0);
long int gcd(long int i1, long int i2);

bool text_length_is_one(const string &str);

#endif
