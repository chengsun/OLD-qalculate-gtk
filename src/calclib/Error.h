/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef ERROR_H
#define ERROR_H

class Error;

#include "includes.h"

class Error {
  protected:
	string smessage;
	bool bcritical;
  public:
	Error(string message_, bool critical_);
	string message(void);
	const char* c_message(void);	
	bool critical(void);
};

#endif
