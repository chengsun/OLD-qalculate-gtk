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
#include <time.h>

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
string d2s(long double value, int precision = 100);
string i2s(int value);
int s2i(const string& str);
int s2i(const char *str);
long int s2li(const string& str);
long int s2li(const char *str);
long long int s2lli(const string& str);
long long int s2lli(const char *str);
string lli2s(long long int value);
string li2s(long int value);
string &lli2s(long long int &value, string &str);
string ld2s(long double value);

bool s2date(string str, int &year, int &month, int &day);
bool s2date(string str, struct tm *time);
bool isLeapYear(int year);
int daysPerYear(int year, int basis = 0);
Fraction *yearsBetweenDates(string date1, string date2, int basis);
int daysBetweenDates(string date1, string date2, int basis = 0);
int daysBetweenDates(int year1, int month1, int day1, int year2, int month2, int day2, int basis);

int find_ending_bracket(const string &str, int start, int *missing = NULL);

char op2ch(MathOperation op);

long long int llpow(long long int base, long long int exp, bool &overflow);

int find_first_not_of(const string &str, int pos, ...);
int find_first_of(const string &str, int pos, ...);
int find_last_not_of(const string &str, int pos, ...);
int find_last_of(const string &str, int pos, ...);

long double rad2deg(long double &value);
long double deg2rad(long double &value);
long double rad2gra(long double &value);
long double gra2rad(long double &value);
long double deg2gra(long double &value);
long double gra2deg(long double &value);

string& wrap_p(string &str);
string& remove_blanks(string &str);
string& remove_duplicate_blanks(string &str);
string& remove_blank_ends(string &str);
string& remove_brackets(string &str);

bool is_in(char c, ...);
bool is_not_in(char c, ...);
bool is_in(const char *str, char c);
bool is_not_in(const char *str, char c);
int sign_place(string *str, unsigned int start = 0);
long long int gcd(long long int i1, long long int i2);
long double gcd_d(long double i1, long double i2);

#endif
