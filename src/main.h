/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef MAIN_H
#define MAIN_H

enum {
	DECI_LEAST = 1,
	DECI_FIXED = 2
};

enum {
	MODE_NORMAL = 1,
	MODE_SCIENTIFIC = 2,
	MODE_SCIENTIFIC_PURE = 3,	
	MODE_DECIMALS = 4
};

enum {
	FRACTIONAL_MODE_DECIMAL = 1,
	FRACTIONAL_MODE_COMBINED = 2,	
	FRACTIONAL_MODE_FRACTION = 3,	
};

enum {
	BASE_DECI = 1,
	BASE_BIN = 2,		
	BASE_OCTAL = 3,
	BASE_HEX = 4
};

#include "Calculator.h"

#endif /* MAIN_H */
