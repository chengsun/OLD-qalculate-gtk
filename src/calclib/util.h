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

struct eqstr {
    bool operator()(const char *s1, const char *s2) const;	
};

string& gsub(const string &pattern, const string &sub, string &str);
string& gsub(const char *pattern, const char *sub, string &str);
string d2s(long double value, int precision = 100);
string i2s(int value);
int s2i(const string& str);
int s2i(const char *str);

long double rad2deg(long double &value);
long double deg2rad(long double &value);
long double rad2gra(long double &value);
long double gra2rad(long double &value);
long double deg2gra(long double &value);
long double gra2deg(long double &value);

string& wrap_p(string &str);
string& remove_blanks(string &str);
string& remove_blank_ends(string &str);

bool is_in(const char *str, char c);
bool is_not_in(const char *str, char c);
int sign_place(string *str, unsigned int start = 0);
string &remove_trailing_zeros(string &str, int decimals_to_keep = 0, bool expand = false, bool decrease = false);

#endif
