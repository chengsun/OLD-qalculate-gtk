/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Error.h"

Error::Error(string message_, bool critical_ = false) {
	bcritical = critical_;
	smessage = message_;
}
string Error::message() {
	return smessage;
}
const char* Error::c_message() {
	return smessage.c_str();
}
bool Error::critical() {
	return bcritical;
}

