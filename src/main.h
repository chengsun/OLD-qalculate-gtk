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
	MODE_DECIMALS = 3,
	MODE_PREFIXES = 4
};

enum {
	BASE_DECI = 1,
	BASE_OCTAL = 2,
	BASE_HEX = 3
};

#include "Calculator.h"

#endif /* MAIN_H */
