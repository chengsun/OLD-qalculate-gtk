/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#define BIT_SIZE		1000000000
#define BIT_EXP10		9

#include "Integer.h"
#include <unistd.h>

Integer::Integer() {
#ifdef HAVE_LIBGMP
	mpz_init(integ);
#endif
	clear();
}
Integer::Integer(long int value) {
#ifdef HAVE_LIBGMP
	mpz_init(integ);
#endif
	set(value);
}
Integer::Integer(const Integer *integer) {
#ifdef HAVE_LIBGMP
	mpz_init(integ);
#endif
	set(integer);
}
Integer::~Integer() {
#ifdef HAVE_LIBGMP
	mpz_clear(integ);
#endif
}
bool Integer::isEven() const {
#ifdef HAVE_LIBGMP
	return mpz_even_p(integ);
#else
	return isZero() || bits[0] % 2 == 0;
#endif
}
bool Integer::isNegative() const {
#ifdef HAVE_LIBGMP
	return mpz_cmp_si(integ, 0) < 0;
#else
	return b_neg;
#endif
}
bool Integer::isPositive() const {
#ifdef HAVE_LIBGMP
	return mpz_cmp_si(integ, 0) > 0;
#else
	return !isNegative() && !isZero();
#endif
}
void Integer::setNegative(bool negative) {
#ifdef HAVE_LIBGMP
	if(isNegative() != negative) mpz_neg(integ, integ);
#else
	b_neg = negative;
#endif
}
void Integer::clear() {
#ifdef HAVE_LIBGMP
	mpz_set_si(integ, 0);
#else
	b_neg = false;
	bits.clear();
#endif
}
void Integer::set(long int value) {
#ifdef HAVE_LIBGMP
	mpz_set_si(integ, value);
#else
	bits.clear();
	b_neg = false;
	if(value == 0) return;
	if(value < 0) {
		value = -value;
		b_neg = true;
	} else {
		b_neg = false;
	}
	if(value < BIT_SIZE) {
		bits.push_back(value);
	} else {
		long int overflow = value / BIT_SIZE, overflow2;
		value = value % BIT_SIZE;
		bits.push_back(value);
		while(overflow) {
			overflow2 = overflow;
			overflow = overflow2 / BIT_SIZE;
			overflow2 = overflow2 % BIT_SIZE;
			bits.push_back(overflow2);
		}
	}
#endif
}
void Integer::set(const Integer *integer) {
#ifdef HAVE_LIBGMP
	mpz_set(integ, integer->integ);
#else
	if(integer == this) return;
	bits.clear();
	b_neg = false;	
	if(!integer) return;
	b_neg = integer->isNegative();
	for(int index = 0; index < integer->bits.size(); index++) {
		bits.push_back(integer->bits[index]);
	}
#endif
}
int Integer::compare(const Integer *integer) const {
#ifdef HAVE_LIBGMP
	int i = mpz_cmp(integ, integer->integ);
	if(i > 0) return -1;
	else if(i < 0) return 1;
	return 0;
#else
	if(integer->isNegative() != isNegative()) {
		if(integer->isNegative()) return -1;
		return 1;
	} else if(isNegative()) {
		if(integer->bits.size() > bits.size()) return -1;
		if(integer->bits.size() < bits.size()) return 1;	
		for(int index = bits.size() - 1; index >= 0; index--) {
			if(integer->bits[index] > bits[index]) return -1;
			if(integer->bits[index] < bits[index]) return 1;		
		}	
	} else {
		if(integer->bits.size() > bits.size()) return 1;
		if(integer->bits.size() < bits.size()) return -1;	
		for(int index = bits.size() - 1; index >= 0; index--) {
			if(integer->bits[index] > bits[index]) return 1;
			if(integer->bits[index] < bits[index]) return -1;		
		}
	}
	return 0;
#endif
}
int Integer::compare(long int value) const {
#ifdef HAVE_LIBGMP
	int i = mpz_cmp_si(integ, value);
	if(i > 0) return -1;
	else if(i < 0) return 1;
	return 0;
#else
	if(isZero()) {
		if(value < 0) return -1;
		if(value == 0) return 0;
		return 1;
	} 
	if(value <= 0 && !isNegative()) return -1;
	if(value >= 0 && isNegative()) return 1;	
	if(bits.size() == 1) {
		bit_1_value_compare:
		if(isNegative()) {
			if(-value > bits[0]) return -1;
			if(-value == bits[0]) return 0;
			return 1;	
		} else {
			if(value < bits[0]) return -1;
			if(value == bits[0]) return 0;
			return 1;	
		}
	}
	if(bits.size() == 2 && value >= BIT_SIZE) {	
		long int value2 = value / BIT_SIZE;
		value = value % BIT_SIZE;
		if(isNegative()) {
			if(-value2 > bits[1]) return -1;
			if(-value2 == bits[1]) goto bit_1_value_compare;
			return 1;	
		} else {
			if(value2 < bits[1]) return -1;
			if(value2 == bits[1]) goto bit_1_value_compare;
			return 1;	
		}		
	}
	return -1;
#endif
}
bool Integer::isGreaterThan(const Integer *integer) const {
	return compare(integer) == -1;
}
bool Integer::isLessThan(const Integer *integer) const {
	return compare(integer) == 1;
}
bool Integer::isGreaterThan(long int value) const {
	return compare(value) == -1;
}
bool Integer::isLessThan(long int value) const {
	return compare(value) == 1;
}
bool Integer::equals(const Integer *integer) const {
#ifdef HAVE_LIBGMP
	return mpz_cmp(integ, integer->integ) == 0;
#else
	if(integer->isNegative() != isNegative()) return false;
	if(integer->bits.size() != bits.size()) return false;
	for(int index = bits.size() - 1; index >= 0; index--) {
		if(integer->bits[index] != bits[index]) return false;
	}
	return true;
#endif
}
bool Integer::equals(long int value) const {
#ifdef HAVE_LIBGMP
	return mpz_cmp_si(integ, value) == 0;
#else
	if(value == 0) {
		return isZero();
	}
	if(bits.size() == 1) {
		return (bits[0] == value && isPositive()) || (bits[0] == -value && isNegative());
	} else if(bits.size() == 2) {		
		long int value2 = value / BIT_SIZE;
		if(value == 0) return false;
		if(isNegative()) {
			if(-value2 != bits[1]) return false;
			value = value % BIT_SIZE;
			return -value == bits[0];
		} else {
			if(value2 != bits[1]) return false;		
			return value == bits[0];
		}
	}
	return false;
#endif
}
void Integer::add(long int value) {
#ifdef HAVE_LIBGMP
	if(value < 0) mpz_sub_ui(integ, integ, (unsigned long int) -value);
	else mpz_add_ui(integ, integ, (unsigned long int) value);
#else
	if(value == 0) return;
	if(bits.empty()) {
		set(value);
		return;
	}
	if(b_neg) {
		value = -value;
	}
	long int overflow, overflow2;
	if(value > 0) {
		int index = 0;
		overflow = value;
		while(overflow > 0) {
			overflow2 = overflow;
			overflow = overflow2 / BIT_SIZE;
			overflow2 = overflow2 % BIT_SIZE;
			if(index < bits.size()) {
				bits[index] += overflow2;
				if(bits[index] >= BIT_SIZE) {
					overflow += bits[index] / BIT_SIZE;
					bits[index] = bits[index] % BIT_SIZE;
				}				
			} else {
				bits.push_back(overflow2);
			}
			index++;
		}
	} else if(value < 0) {
		int index = 0;
		overflow = value;
		bool negated = false;
		while(overflow < 0) {
			overflow2 = overflow;
			overflow = overflow2 / BIT_SIZE;
			overflow2 = overflow2 % BIT_SIZE;
			if(index < bits.size()) {
				bits[index] += overflow2;
				if(bits[index] < 0) {
					overflow -= bits[index] / BIT_SIZE + 1;
					bits[index] = BIT_SIZE + bits[index] % BIT_SIZE;
				}		
			} else {
				if(negated) {
					bits.push_back(-overflow2);
				} else {
					negate();
					negated = true;
					if(index < bits.size()) {
						bits[index] -= overflow2;
					} else {
						bits.push_back(-overflow2);
					}
				}
				if(bits[index] >= BIT_SIZE) {
					overflow -= bits[index] / BIT_SIZE;
					bits[index] = bits[index] % BIT_SIZE;
				}				
			}
			index++;
		}		
	}
	for(int index = bits.size() - 1; index >= 0; index--) {
		if(bits[index] == 0) {
			bits.erase(bits.begin() + index);
		} else {
			break;
		}
	}
	if(bits.empty()) b_neg = false;
#endif
}
void Integer::add(const Integer *integer) {
#ifdef HAVE_LIBGMP
	mpz_add(integ, integ, integer->integ);
#else
	if(integer->isZero()) return;
	if(bits.empty()) {
		set(integer);
		return;
	}
	bool b_del = false;
	if(integer == this) {
		integer = new Integer(this);
		b_del = true;
	}		
	long int overflow, overflow2;
	if(integer->isNegative() == b_neg) {
		for(int index = 0; index < integer->bits.size(); index++) {
			if(index < bits.size()) {
				bits[index] += integer->bits[index];
				int index2 = index;
				while(bits[index2] >= BIT_SIZE) {
					if(index2 + 1 < bits.size()) {
						bits[index2 + 1] += bits[index2] / BIT_SIZE;
					} else {
						bits.push_back(bits[index2] / BIT_SIZE);
					}
					bits[index2] = bits[index2] % BIT_SIZE;
					index2++;
				}				
			} else {
				bits.push_back(integer->bits[index]);
			}			
		}
	} else {
		bool negated = false;
		for(int index = 0; index < integer->bits.size(); index++) {
			if(index < bits.size()) {
				bits[index] -= integer->bits[index];	
				for(int i = index; i < bits.size(); i++) {
					if(bits[i] < 0) {
						if(bits.size() == 1) {
							bits[i] = -bits[i];
							b_neg = !b_neg;
							negated = true;
							break;
						} else if(i + 1 == bits.size()) {
							long int li = -bits[i];
							bits.erase(bits.begin() + i);
							negate();															
							negated = true;
							if(i < bits.size()) {
								bits[i] += li;
								int index2 = li;
								while(bits[index2] >= BIT_SIZE) {
									if(index2 + 1 < bits.size()) {
										bits[index2 + 1] += bits[index2] / BIT_SIZE;
									} else {
										bits.push_back(bits[index2] / BIT_SIZE);
									}
									bits[index2] = bits[index2] % BIT_SIZE;
									index2++;
								}						
							} else {
								bits.push_back(li);
							}							
							break;	
						} else {
							bits[i] = BIT_SIZE + bits[i];											
							bits[i + 1]--;
						}
					} else {
						break;
					}
				}
			} else {
				if(negated) {	
					bits.push_back(integer->bits[index]);
				} else {				
					negate();
					negated = true;
					if(index < bits.size()) {
						bits[index] += integer->bits[index];
						int index2 = index;
						while(bits[index2] >= BIT_SIZE) {
							if(index2 + 1 < bits.size()) {
								bits[index2 + 1] += bits[index2] / BIT_SIZE;
							} else {
								bits.push_back(bits[index2] / BIT_SIZE);
							}
							bits[index2] = bits[index2] % BIT_SIZE;
							index2++;
						}						
					} else {
						bits.push_back(integer->bits[index]);
					}					
				}
			}			
		}	
	}
	for(int index = bits.size() - 1; index >= 0; index--) {
		if(bits[index] == 0) {
			bits.erase(bits.begin() + index);
		} else {
			break;
		}
	}
	if(bits.empty()) b_neg = false;	
	if(b_del) delete integer;
#endif
}
void Integer::negate() {
#ifdef HAVE_LIBGMP
#else
	b_neg = !b_neg;
	for(int i = 0; i < bits.size(); i++) {
		if(bits[i] != 0) {
			bits[i] = BIT_SIZE - bits[i];
			if(i + 1 != bits.size()) {
				bits[i + 1] -= 1;
			} else {
				bits.push_back(-1);
				break;
			}
		}
	}
#endif
}
void Integer::subtract(long int value) {
	add(-value);
}
void Integer::subtract(const Integer *integer) {
#ifdef HAVE_LIBGMP
	mpz_sub(integ, integ, integer->integ);
#else
	Integer integer2(integer);
	integer2.b_neg = !integer2.b_neg;
	add(&integer2);
#endif
}
void Integer::multiply(long int value) {
#ifdef HAVE_LIBGMP
	mpz_mul_si(integ, integ, value);
#else
	if(value == 0) {
		bits.clear();
		b_neg = false;
		return;
	}
	if(value < 0) {
		value = -value;
		b_neg = !b_neg;
	}
	if(value == 1) return;
	long long int multi, overflow, overflow2;
	vector<long int> bits_save(bits);
	bits.clear();
	for(int index = 0; index < bits_save.size(); index++) {
		multi = (long long int) bits_save[index] * (long long int) value;
		int index2 = index;
		overflow = multi;
		while(overflow > 0) {
			overflow2 = overflow;
			overflow = overflow2 / BIT_SIZE;
			overflow2 = overflow2 % BIT_SIZE;
			if(index2 < bits.size()) {
				bits[index2] += overflow2;
				if(bits[index2] >= BIT_SIZE) {
					overflow += bits[index2] / BIT_SIZE;
					bits[index2] = bits[index2] % BIT_SIZE;
				}				
			} else {
				for(int i = bits.size(); i < index2; i++) {
					bits.push_back(0);
				}
				bits.push_back(overflow2);				
			}
			index2++;
		}
	}
	for(int index = bits.size() - 1; index >= 0; index--) {
		if(bits[index] == 0) {
			bits.erase(bits.begin() + index);
		} else {
			break;
		}
	}
	if(bits.empty()) b_neg = false;		
#endif
}
void Integer::multiply(const Integer *integer) {
#ifdef HAVE_LIBGMP
	mpz_mul(integ, integ, integer->integ);
#else
	if(integer->isZero()) {
		bits.clear();
		b_neg = false;
		return;
	}
	if(integer->isNegative()) {
		b_neg = !b_neg;
	}
	if(integer->isOne() || integer->isMinusOne()) return;
	bool b_del = false;
	if(integer == this) {
		integer = new Integer(this);
		b_del = true;
	}	
	long long int multi, overflow, overflow2;
	vector<long int> bits_save(bits);
	bits.clear();
	for(int index = 0; index < bits_save.size(); index++) {
		for(int index2 = 0; index2 < integer->bits.size(); index2++) {	
			multi = (long long int) bits_save[index] * (long long int) integer->bits[index2];
			int index3 = index + index2;
			overflow = multi;
			while(overflow > 0) {
				overflow2 = overflow;
				overflow = overflow2 / BIT_SIZE;
				overflow2 = overflow2 % BIT_SIZE;
				if(index3 < bits.size()) {
					bits[index3] += overflow2;
					if(bits[index3] >= BIT_SIZE) {
						overflow += bits[index3] / BIT_SIZE;
						bits[index3] = bits[index3] % BIT_SIZE;
					}				
				} else {
					for(int i = bits.size(); i < index3; i++) {
						bits.push_back(0);
					}
					bits.push_back(overflow2);				
				}
				index3++;
			}
		}
	}
	for(int index = bits.size() - 1; index >= 0; index--) {
		if(bits[index] == 0) {
			bits.erase(bits.begin() + index);
		} else {
			break;
		}
	}
	if(bits.empty()) b_neg = false;		
	if(b_del) delete integer;
#endif
}
bool Integer::divide(long int value, Integer **reminder) {
#ifdef HAVE_LIBGMP
	mpz_t rem;
	mpz_init(rem);
	mpz_tdiv_qr_ui(integ, rem, integ, value);
	if(reminder) *reminder = new Integer();
	if(mpz_cmp_si(rem, 0) != 0) {
		if(reminder) mpz_set((*reminder)->integ, rem);
		mpz_clear(rem);	
		return false;
	}
	mpz_clear(rem);
	return true;
#else
	Integer integer(value);
	return divide(&integer, reminder);
#endif
}
bool Integer::divide(const Integer *integer, Integer **reminder) {
#ifdef HAVE_LIBGMP
	mpz_t rem;
	mpz_init(rem);
	mpz_tdiv_qr(integ, rem, integ, integer->integ);
	if(reminder) *reminder = new Integer();
	if(mpz_cmp_si(rem, 0) != 0) {
		if(reminder) mpz_set((*reminder)->integ, rem);
		mpz_clear(rem);	
		return false;
	}
	mpz_clear(rem);
	return true;
#else
//	printf("A %s : %s\n", print().c_str(), integer->print().c_str());
	if(integer->isZero()) {
		//division by zero error
		if(reminder) *reminder = new Integer();
		return true;
	}	
	if(isZero()) {
		if(reminder) *reminder = new Integer();
		return true;	
	}
	if(integer->isOne()) {
		if(reminder) *reminder = new Integer();
		return true;	
	}	
	if(integer->isMinusOne()) {
		if(reminder) *reminder = new Integer();
		setNegative(!isNegative());
		return true;	
	}	
	bool neg = isNegative() != integer->isNegative();		
	Integer *rem = new Integer(this);
	rem->setNegative(false);
	if(reminder) *reminder = rem;
	Integer integer_noneg(integer);
	integer_noneg.setNegative(false);	
	int i_com = rem->compare(&integer_noneg);
	if(i_com == 0) {		
		set(1);
		setNegative(neg);
		if(!reminder) delete rem;
		return true;
	} else if(i_com == 1) {
		rem->set(this);
		clear();		
		if(!reminder) delete rem;
		return false;
	}
	Integer divided, tmp, remdiv;
	long int q1;
	long long int lli;
	long int step;
	divided.b_neg = neg;
//		printf("E %i %s\n", rem->bits.size(), rem->print().c_str());			
	while(i_com < 0) {
//		printf("B\n");
		remdiv.clear();
		bool b2 = false, b = false, c = false, d = false;
		for(int i = 0; i < integer->bits.size(); i++) {
			remdiv.bits.insert(remdiv.bits.begin(), rem->bits[rem->bits.size() - 1]);
			rem->bits.pop_back();
		}
		q1 = remdiv.bits[remdiv.bits.size() - 1] / integer->bits[integer->bits.size() - 1];
		i_com = remdiv.compare(&integer_noneg);
		if(i_com == 1) {		
			remdiv.bits.insert(remdiv.bits.begin(), rem->bits[rem->bits.size() - 1]);
			rem->bits.pop_back();			
			lli = remdiv.bits[remdiv.bits.size() - 1];
			lli *= BIT_SIZE;
			lli += remdiv.bits[remdiv.bits.size() - 2];
			q1 = lli / (long long int) integer->bits[integer->bits.size() - 1];
			b2 = true;
		}
		if(q1 < 10) step = 10;
		if(q1 < 100) step = 100;		
		if(q1 < 1000) step = 1000;		
		if(q1 < 10000) step = 10000;		
		if(q1 < 100000) step = 100000;		
		if(q1 < 1000000) step = 1000000;		
		if(q1 < 10000000) step = 10000000;		
		if(q1 < 100000000) step = 100000000;		
		else step = 1000000000;		
		while(true) {
//			printf("C %li %s %s\n", q1, integer->print().c_str(), remdiv.print().c_str());
			tmp.set(&integer_noneg);
			tmp.multiply(q1);
			i_com = remdiv.compare(&tmp);
			if(i_com > 0) {
				if(d && step > 1) step /= 10;
				q1 -= step;
				if(q1 <= 0) q1 = 1;
				b = true;
				d = false;
				c = true;	
			} else if(b && (d || step > 1) && i_com != 0) {
				if(c) step /= 10;
				q1 += step;
				c = false;
				d = true;
			} else {
				break;
			}
		}
		remdiv.subtract(&tmp);
//		printf("F %i %s\n", remdiv.bits.size(), remdiv.print().c_str());		
		for(int i = 0; i < remdiv.bits.size(); i++) {
			rem->bits.push_back(remdiv.bits[i]);
		}
		for(int index = rem->bits.size() - 1; index >= 0; index--) {
			if(rem->bits[index] == 0) {
				rem->bits.erase(rem->bits.begin() + index);
			} else {
				break;
			}
		}	
		divided.bits.insert(divided.bits.begin(), q1);
//		printf("D %i %s : %s\n", rem->bits.size(), rem->print().c_str(), integer_noneg.print().c_str());		
		i_com = rem->compare(&integer_noneg);
		if(i_com == 0) {
			rem->clear();
			divided.bits.insert(divided.bits.begin(), 1);			
		}
	}
//		printf("D\n");
	set(&divided);
	bool return_value = rem->isZero();
	if(!reminder) delete rem;
	for(int index = bits.size() - 1; index >= 0; index--) {
		if(bits[index] == 0) {
			bits.erase(bits.begin() + index);
		} else {
			break;
		}
	}
	if(bits.empty()) b_neg = false;		
	return return_value;
#endif
}
bool Integer::div10(Integer **reminder, long int exp) {
#ifdef HAVE_LIBGMP
	Integer int10(10);	
	int10.pow(exp);
	return divide(&int10, reminder);
#else
	if(isZero() || exp == 0) {
		if(reminder) *reminder = new Integer();
		return true;
	}
	if(exp < 0) {
		if(reminder) *reminder = new Integer();
		Integer div(1);
		for(int i = 0; i > exp; i--) {	
			div.multiply(10);
		}
		multiply(&div);
		return true;
	}
	bool return_value = true;	
	if(exp == 1) {
		long int q = 0, q2 = 0;
		for(int i = bits.size() - 1; i >= 0; i--) {		
			q = bits[i] % 10;
			bits[i] /= 10;
			bits[i] += q2 * (BIT_SIZE / 10);
			q2 = q;
		}	
		if(reminder) *reminder = new Integer(q);
		return_value = q == 0;	
	} else {
		if(exp / BIT_EXP10 > bits.size()) {	
			if(reminder) *reminder = new Integer(this);
			clear();
			return false;
		}	
		Integer *rem = new Integer();
		mod10(&rem, exp);
		return_value = rem->isZero();
		if(reminder) {
			*reminder = rem;
		} else {
			delete rem;
		}	
		long int div = 1;
		for(int i = 0; i < exp % BIT_EXP10; i++) {
			div *= 10;
		}
		for(int i = 0; i < exp / BIT_EXP10; i++) {
			bits.erase(bits.begin());
		}
		long int q = 0, q2 = 0;
		for(int i = bits.size() - 1; i >= 0; i--) {		
			q = bits[i] % div;
			bits[i] /= div;
			bits[i] += q2 * (BIT_SIZE / div);
			q2 = q;
		}		
	}	
	for(int index = bits.size() - 1; index >= 0; index--) {
		if(bits[index] == 0) {
			bits.erase(bits.begin() + index);
		} else {
			break;
		}
	}
	if(bits.empty()) b_neg = false;		
	return return_value;
#endif
}
bool Integer::mod10(Integer **reminder, long int exp) const {
#ifdef HAVE_LIBGMP
	Integer int10(10);	
	int10.pow(exp);
	mpz_t rem;
	mpz_init(rem);
	mpz_tdiv_r(rem, integ, int10.integ);
	if(reminder) *reminder = new Integer();
	if(mpz_cmp_si(rem, 0) != 0) {
		if(reminder) mpz_set((*reminder)->integ, rem);
		mpz_clear(rem);	
		return false;
	}
	mpz_clear(rem);
	return true;	
#else
	if(isZero()) {
		if(reminder) *reminder = new Integer();
		return true;	
	}
	if(exp == 1) {
		if(bits[0] % 10 == 0) {
			if(reminder) *reminder = new Integer();
			return true;
		} else {
			if(reminder) *reminder = new Integer(bits[0] % 10);
			return false;
		}
	}
	if(exp <= 0) {
		if(reminder) *reminder = new Integer();
		return true;
	}
	if(exp / BIT_EXP10 > bits.size()) {
		if(reminder) *reminder = new Integer(this);
		return false;
	}
	long int div = 1;
	int i2 = -1;
	for(int i = 0; i < exp; i++) {
		if(i % BIT_EXP10 == 0) {
			div = 10;
			i2++;
		} else {
			div *= 10;
		}
		if(bits[i2] % div != 0) {
			if(reminder) {
				*reminder = new Integer();
				(*reminder)->b_neg = b_neg;
				for(int i3 = 0; i3 < exp / BIT_EXP10; i3++) {
					(*reminder)->bits.push_back(bits[i3]);
				}
				div = 1;
				for(int i = 0; i < exp % BIT_EXP10; i++) {
					div *= 10;
				}
				(*reminder)->bits.push_back(bits[exp / BIT_EXP10] % div);
			}
			return false;
		}
	}
	if(reminder) *reminder = new Integer();
	return true;
#endif
}
void Integer::exp10(const Integer *integer) {
	Integer exp;
	if(integer) exp.set(integer);
	else exp.set(this);
	Integer exp10(10);
	exp10.pow(&exp);
	if(integer) multiply(&exp10);
	else set(&exp10);
}
void Integer::exp10(long int value) {
	Integer exp10(10);
	exp10.pow(value);
	multiply(&exp10);
}
void Integer::pow(const Integer *exp) {
	if(exp->isZero()) {
		set(1);
	} else if(!exp->isOne()) {
		Integer this_saved(this);
		Integer expdiv(exp);
		bool even = expdiv.divide(2);
		pow(&expdiv);
		multiply(this);
		if(!even) {
			multiply(&this_saved);
		}
	}
}
void Integer::pow(long int exp) {
	if(exp == 0) {
		set(1);
	} else if(exp != 1) {
		Integer this_saved(this);	
		bool even = (exp % 2 == 0);
		exp /= 2;
		pow(exp);
		multiply(this);
		if(!even) {
			multiply(&this_saved);
		}
	}
}
void Integer::ln() {
	add(-1);
	Integer tmp1(this), tmp2, tmp_this(this);
	for(int i = 2; i < 5; i++) {
		tmp1.multiply(&tmp_this);
		tmp2.set(&tmp1);	
		tmp2.divide(i);	
		if(i % 2 == 0) tmp2.setNegative(!tmp2.isNegative());
		add(&tmp2);
	}
}
bool Integer::gcd(const Integer *integer, Integer **divisor) const {
#ifdef HAVE_LIBGMP
	mpz_t div;
	mpz_init(div);
	mpz_gcd(div, integ, integer->integ);
	if(divisor) {
		*divisor = new Integer();
		mpz_set((*divisor)->integ, div);	
	}
	mpz_clear(div);		
	if(mpz_cmp_ui(div, 1) == 0) {
		return false;
	}
	return true;
#else
	if(isZero() || integer->isZero()) {
		if(divisor) *divisor = new Integer();
		return false;
	}
	int comp = compare(integer);
	if(comp == 1) {
		return integer->gcd(this, divisor);
	}
	if(comp == 0) {
		if(divisor) *divisor = new Integer(this);
		return true;
	}	
	Integer int1(this), int2(integer);
	Integer *int3;
	while(!int1.divide(&int2, &int3)) {	
		int1.set(&int2);
		int2.set(int3);	
		delete int3;		
	}
	if(divisor) *divisor = new Integer(&int2);
	return !int2.isZero() && !int2.isOne() && !int2.isMinusOne();	
#endif
}
bool Integer::isZero() const {
#ifdef HAVE_LIBGMP
	return mpz_cmp_si(integ, 0) == 0;
#else
	return bits.empty();
#endif
}
bool Integer::isOne() const {
#ifdef HAVE_LIBGMP
	return mpz_cmp_ui(integ, 1) == 0;
#else
	return !b_neg && bits.size() == 1 && bits[0] == 1;
#endif
}
bool Integer::isMinusOne() const {
#ifdef HAVE_LIBGMP
	return mpz_cmp_si(integ, -1) == 0;
#else
	return b_neg && bits.size() == 1 && bits[0] == 1;
#endif
}
long int Integer::getLongInt() const {
#ifdef HAVE_LIBGMP
	return mpz_get_si(integ);
#else
	if(bits.size() == 0) {
		return 0;
	} else if(bits.size() > 1 && bits[1] / BIT_SIZE < LONG_MAX / BIT_SIZE) {
		if(isNegative()) {
			return -(bits[0] + bits[1] * BIT_SIZE);
		} else {
			return bits[0] + bits[1] * BIT_SIZE;
		}	
	} else {
		if(isNegative()) {
			return -bits[0];
		} else {
			return bits[0];
		}
	}
	return 0;
#endif
}
int Integer::getInt() const {
	long int li = getLongInt();
	if(li > INT_MAX) return INT_MAX;
	if(li < INT_MIN) return INT_MIN;	
	return (int) li;
}
string Integer::print(int base, bool display_sign) const {
#ifdef HAVE_LIBGMP
	char *c_str = mpz_get_str(NULL, base, integ);
	string mpz_str = c_str;
	if(!display_sign && isNegative()) {
		mpz_str.erase(0, 1);
	}
	string str = "";
	if(base == 16) {
		str += "0x";
	} else if(base == 8) {
		str += "0";
	} 
	str += mpz_str;
	if(base == 2) {
		for(int i = str.length() - 4; (isNegative() && i > 1) || (!isNegative() && i > 0); i -= 4) {
			str.insert(i, 1, ' ');
		} 
	}
	free(c_str);
	return str;
#else
	string str = "";
	long int value;
	char c;	
	if(b_neg && display_sign) str = '-';
	if(base == 10) {
		if(isZero()) return "0";
		bool keep_zeros = false;
		for(int index = bits.size() - 1; index >= 0; index--) {
			value = bits[index];
			c = '0' + value / 100000000;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}
			value = value % 100000000;
			c = '0' + value / 10000000;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}
			value = value % 10000000;		
			c = '0' + value / 1000000;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}		
			value = value % 1000000;		
			c = '0' + value / 100000;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}		
			value = value % 100000;
			c = '0' + value / 10000;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}		
			value = value % 10000;		
			c = '0' + value / 1000;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}		
			value = value % 1000;				
			c = '0' + value / 100;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}		
			value = value % 100;
			c = '0' + value / 10;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}		
			value = value % 10;		
			c = '0' + value;
			if(keep_zeros || c != '0') {keep_zeros = true; str += c;}		
		}
	} else {
		if(base == 16) {
			str += "0x";
		} else if(base == 8) {
			str += "0";
		}
		if(isZero()) {
			str += "0";
			return str;
		}
		int str_begin = str.length();
		Integer copy(this);
		Integer *rem;
		while(true) {
			copy.divide(base, &rem);
			if(rem->isZero()) value = 0;
			else value = rem->bits[0];
			if(value < 10) c = '0' + value;
			else c = 'a' + (value - 10);
			str.insert(str_begin, 1, c);
			delete rem;
			if(copy.isZero()) break;
		}
		if(base == 2) {
			for(int i = str.length() - 5; (isNegative() && i > 1) || (!isNegative() && i > 0); i -= 4) {
				str.insert(i, 1, ' ');
			} 
		}		
	}
	return str;
#endif
}

