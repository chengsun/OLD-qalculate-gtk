/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef INTEGER_H
#define INTEGER_H

class Integer;

/**
* An arbitrary precision integer.
*/

#include "includes.h"
#include "config.h"

#ifdef HAVE_LIBCLN
#define WANT_OBFUSCATING_OPERATORS
#include <cln/cln.h>
using namespace cln;
#else
#ifdef HAVE_LIBGMP
#include <gmp.h>
#endif
#endif

class Integer {

  public:

#ifdef HAVE_LIBCLN
	cl_I integ;
#else
#ifdef HAVE_LIBGMP
	mpz_t integ;	
#else
  	bool b_neg;
	vector<long int> bits;  
#endif
#endif  
	/**
	* Constructs an integer and initializes it to zero.
	*/
	Integer();  
	/**
	* Constructs an integer with an initial value.
	*
	* @param value Initial value.
	*/	
	Integer(long int value);
	/**
	* Constructs a copy of an integer.
	*
	* @param integer Integer to copy.
	*/		
	Integer(const Integer *integer);
	~Integer();
	
	/**
	* @return True if the integer is less than zero.
	* @see #isPositive
	* @see #isZero	
	*/	
	bool isNegative() const;
	/**
	* @return True if the integer is greater than zero.
	* @see #isNegative
	* @see #isZero
	*/		
	bool isPositive() const;	
	/**
	* @return True if the integer is even.
	*/		
	bool isEven() const;
	
	/**
	* Sets the sign of the integer.
	*
	* Negates the value if @p negative not equals the current sign. 
	*
	* @param negative If the integer shall be negative or not.
	*/		
	void setNegative(bool negative);
	
	/**
	* Compares the integer with another integer.
	*
	* @return 0 if @p integer is equal, 1 if it is greater, -1 if it is less.
	* @param integer Integer to compare with.
	*/			
	int compare(const Integer *integer) const; 
	/**
	* Compares the integer with an fixed precision integer.
	*
	* @return 0 if @p value is equal, 1 if it is greater, -1 if it is less.
	* @param value Integer to compare with.
	*/	
	int compare(long int value) const; 	
	bool isGreaterThan(const Integer *integer) const; 	
	bool isLessThan(const Integer *integer) const; 		
	bool isGreaterThan(long int value) const; 	
	bool isLessThan(long int value) const; 
	bool equals(const Integer *integer) const;				
	bool equals(long int value) const;
	void clear();
	void set(long int value);
	void set(const Integer *integer);	
	void add(long int value);
	void add(const Integer *integer);	
	void subtract(long int value);	
	void subtract(const Integer *integer);		
	void negate();
	void multiply(long int value);	
	void multiply(const Integer *integer);		
	bool divide(long int value, Integer **remainder = NULL);
	bool divide(const Integer *integer, Integer **remainder = NULL);	
	bool div10(Integer **remainder = NULL, long int exp = 1);
	bool mod10(Integer **remainder = NULL, long int exp = 1) const;
	void exp10(const Integer *integer = NULL);
	void exp10(long int value);
	void pow(const Integer *integer);
	void pow(long int value);
	bool gcd(const Integer *integer, Integer **divisor) const;
	bool isZero() const;
	bool isOne() const;
	bool isMinusOne() const;
	long int getLongInt() const;
	int getInt() const;	
	string print(int base = 10, bool display_sign = true) const;

};

#endif
