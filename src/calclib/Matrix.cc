/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Matrix.h"
#include "Calculator.h"
#include "Manager.h"
#include "Integer.h"
#include "Fraction.h"
#include "util.h"

/**
* Contains a matrix.
*/

Matrix::Matrix() {
	b_exact = true;
	b_vector = false;
	elements.resize(1);
	elements[0].push_back(new Manager());
}
Matrix::Matrix(int nr_of_rows, int nr_of_columns) {
	b_exact = true;
	elements.resize(1);
	elements[0].push_back(new Manager());
	setColumns(nr_of_columns);
	setRows(nr_of_rows);
}
Matrix::Matrix(const Matrix *matrix) {
	elements.resize(1);
	elements[0].push_back(new Manager());
	set(matrix);
}
Matrix::~Matrix() {
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			if(elements[index_r][index_c]) {
				delete elements[index_r][index_c];
			}
		}
	} 
}
void Matrix::set(const Matrix *matrix) {
	if(!matrix->isVector()) b_vector = false;
	b_exact = matrix->isPrecise();
	setColumns(matrix->columns());
	setRows(matrix->rows());	
	for(int index_r = 1; index_r <= matrix->rows(); index_r++) {
		for(int index_c = 1; index_c <= matrix->columns(); index_c++) {
			set(matrix->get(index_r, index_c), index_r, index_c);
		}
	} 
}
void Matrix::setToIdentityMatrix(int rows_columns) {
	b_vector = false;
	if(rows_columns < 1) rows_columns = 1;
	setColumns(rows_columns);
	setRows(rows_columns);	
	Manager mngr;
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			if(index_c == index_r) {
				mngr.set(1, 1);
			} else {
				mngr.clear();
			}
			set(&mngr, index_r, index_c);
		}
	} 	
}
Matrix *Matrix::getIdentityMatrix() const {
	Matrix *matrix = new Matrix();
	matrix->setToIdentityMatrix(columns());
	return matrix;
}	
void Matrix::transpose() {
	b_vector = false;
	Matrix mtrx_save(this);
	setColumns(mtrx_save.rows());
	setRows(mtrx_save.columns());	
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			set(mtrx_save.get(index_c, index_r), index_r, index_c);
		}
	}	
}
bool Matrix::inverse() {
	Manager *mngr = determinant();
	if(!mngr) {
		CALCULATOR->error(true, _("Could not calculate determinant in inverse function."), NULL);
		return false;
	}
	if(mngr->isNull()) {
		delete mngr;
		return false;
	}
	b_vector = false;
	mngr->addInteger(-1, OPERATION_RAISE);
	adjoint();
	multiply(mngr);
	delete mngr;	
	return true;
}
bool Matrix::adjoint() {
	if(columns() != rows()) {
		CALCULATOR->error(true, _("The adjoint can only be calculated for matrices with an equal number of rows and columns."), NULL);
		return false;
	}
	b_vector = false;
	Matrix mtrx(this);
	Manager *mngr;
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			mngr = mtrx.cofactor(index_r, index_c);
			set(mngr, index_r, index_c);
			delete mngr;
		}
	}
	transpose();
	return true;
}
bool Matrix::rank(bool ascending) {
	Manager *mngr;
	vector<int> ranked_r;
	vector<int> ranked_c;	
	vector<Manager*> ranked_mngr;	
	vector<bool> ranked_equals_prev;	
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			mngr = get(index_r, index_c);
			bool b = false;
			for(int i = 0; i < ranked_r.size(); i++) {
				int cmp = mngr->compare(ranked_mngr[i]);
				if(cmp < -1) {
					CALCULATOR->error(true, _("Unsolvable comparison at element %s:%s when trying to rank matrix/vector."), i2s(index_r).c_str(), i2s(index_c).c_str(), NULL);
					return false;
				}
				if((cmp > 0 && ascending) || cmp == 0 || (cmp < 0 && !ascending)) {
					if(cmp == 0) {
						ranked_c.insert(ranked_c.begin() + i + 1, index_c);
						ranked_r.insert(ranked_r.begin() + i + 1, index_r);						
						ranked_mngr.insert(ranked_mngr.begin() + i + 1, mngr);
						ranked_equals_prev.insert(ranked_equals_prev.begin() + i + 1, true);
					} else {
						ranked_c.insert(ranked_c.begin() + i, index_c);
						ranked_r.insert(ranked_r.begin() + i, index_r);							
						ranked_mngr.insert(ranked_mngr.begin() + i, mngr);
						ranked_equals_prev.insert(ranked_equals_prev.begin() + i, false);
					}
					b = true;
					break;
				}
			}
			if(!b) {
				ranked_r.push_back(index_r);
				ranked_c.push_back(index_c);
				ranked_mngr.push_back(mngr);
				ranked_equals_prev.push_back(false);
			}
		}
	}	
	int n_rep = 0;
	for(int i = ranked_r.size() - 1; i >= 0; i--) {
		if(ranked_equals_prev[i]) {
			n_rep++;
		} else {
			if(n_rep) {
				Manager v(i + 1 + n_rep, 1);
				v.addInteger(i + 1, OPERATION_ADD);
				v.addInteger(2, OPERATION_DIVIDE);
				for(; n_rep >= 0; n_rep--) {
					get(ranked_r[i + n_rep], ranked_c[i + n_rep])->set(&v);
				}
			} else {
				get(ranked_r[i], ranked_c[i])->set(i + 1, 1);
			}
			n_rep = 0;
		}
	}
	return true;
}
bool Matrix::sort(bool ascending) {
	Manager *mngr;
	vector<Manager*> ranked_mngr;	
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			mngr = new Manager(get(index_r, index_c));
			bool b = false;
			for(int i = 0; i < ranked_mngr.size(); i++) {
				int cmp = mngr->compare(ranked_mngr[i]);
				if(cmp < -1) {
					CALCULATOR->error(true, _("Unsolvable comparison at element %s:%s when trying to sort matrix/vector."), i2s(index_r).c_str(), i2s(index_c).c_str(), NULL);
					return false;
				}
				if((cmp > 0 && ascending) || cmp == 0 || (cmp < 0 && !ascending)) {
					ranked_mngr.insert(ranked_mngr.begin() + i, mngr);
					b = true;
					break;
				}
			}
			if(!b) {
				ranked_mngr.push_back(mngr);
			}
		}
	}	
	for(int i = ranked_mngr.size() - 1; i >= 0; i--) {
		set(ranked_mngr[i], i / columns() + 1, i % columns() + 1);
		delete ranked_mngr[i];
	}
	return true;
}
Manager *Matrix::cofactor(int row, int column) const {
	if(row < 1) row = 1;
	if(column < 1) column = 1;
	Matrix mtrx(rows() - 1, columns() - 1);
	for(int index_r = 1; index_r <= rows(); index_r++) {
		if(index_r != row) {
			for(int index_c = 1; index_c <= columns(); index_c++) {
				if(index_c > column) {
					if(index_r > row) {
						mtrx.set(get(index_r, index_c), index_r - 1, index_c - 1);
					} else {
						mtrx.set(get(index_r, index_c), index_r, index_c - 1);
					}
				} else if(index_c < column) {
					if(index_r > row) {
						mtrx.set(get(index_r, index_c), index_r - 1, index_c);
					} else {
						mtrx.set(get(index_r, index_c), index_r, index_c);
					}
				}
			}
		}
	}	
	Manager *mngr = mtrx.determinant();	
	if((row + column) % 2 == 1) {
		mngr->addInteger(-1, OPERATION_MULTIPLY);
	}
	return mngr;
}
bool Matrix::isSymmetric() const {
	Matrix tr_mtrx(this);
	tr_mtrx.transpose();
	return equals(&tr_mtrx);
}
bool Matrix::isOrthogonal() const {
	Matrix tr_mtrx(this);
	tr_mtrx.transpose();
	tr_mtrx.multiply(this);
	Matrix *identity = getIdentityMatrix();
	bool b = identity->equals(&tr_mtrx);
	delete identity;
	return b;
}
Manager *Matrix::determinant(Manager *mngr) const {
	if(columns() != rows()) {
		CALCULATOR->error(true, _("The determinant can only be calculated for matrices with an equal number of rows and columns."), NULL);
		return NULL;
	}
	if(!mngr) {
		mngr = new Manager();
	} else {
		mngr->clear();
	}
	if(rows() == 1) {
		mngr->set(get(1, 1));
	} else if(rows() == 2) {
		Manager tmp;
		mngr->set(get(1, 1));
		mngr->add(get(2, 2), OPERATION_MULTIPLY);
		tmp.set(get(2, 1));
		tmp.add(get(1, 2), OPERATION_MULTIPLY);
		mngr->add(&tmp, OPERATION_SUBTRACT);
	} else {
		Manager tmp;
		Matrix mtrx(elements.size() - 1, elements[0].size() - 1);
		for(int index_c = 0; index_c < elements[0].size(); index_c++) {
			for(int index_r2 = 1; index_r2 < elements.size(); index_r2++) {
				for(int index_c2 = 0; index_c2 < elements[index_r2].size(); index_c2++) {
					if(index_c2 > index_c) {
						mtrx.set(elements[index_r2][index_c2], index_r2, index_c2);
					} else if(index_c2 < index_c) {
						mtrx.set(elements[index_r2][index_c2], index_r2, index_c2 + 1);
					}
				}
			}
			mtrx.determinant(&tmp);	
			if(index_c % 2 == 1) {
				tmp.addInteger(-1, OPERATION_MULTIPLY);
			}
			
			tmp.add(elements[0][index_c], OPERATION_MULTIPLY);
			mngr->add(&tmp, OPERATION_ADD);
		}
	}
	return mngr;
}
int Matrix::rows() const {
	return elements.size();
}
int Matrix::columns() const {
	return elements[0].size();
}
void Matrix::setRows(int nr_of_rows) {
	if(nr_of_rows < 1) nr_of_rows = 1;	
	if(nr_of_rows > 1) b_vector = false;
	for(int index_r = nr_of_rows; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			delete elements[index_r][index_c];
		}		
	}
	elements.resize(nr_of_rows);
	setColumns(columns());
}
void Matrix::setColumns(int nr_of_columns) {
	if(nr_of_columns < 1) nr_of_columns = 1;
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = nr_of_columns; index_c < elements[index_r].size(); index_c++) {
			delete elements[index_r][index_c];
		}
		elements[index_r].resize(nr_of_columns);	
		for(int index_c = 0; index_c < nr_of_columns; index_c++) {
			if(!elements[index_r][index_c]) {
				elements[index_r][index_c] = new Manager();
			}
		}		
	}
}
void Matrix::addRow() {
	setRows(rows() + 1);
}
void Matrix::addColumn() {
	setColumns(columns() + 1);
}
void Matrix::set(const Manager *mngr, int row, int column) {
	row--;
	column--;
	if(row < elements.size() && row >= 0 && column < elements[row].size() && column >= 0) {
		elements[row][column]->set(mngr);
		if(!elements[row][column]->isPrecise()) b_exact = false;
	}
}
Manager *Matrix::get(int row, int column) {
	row--;
	column--;
	if(row < elements.size() && row >= 0 && column < elements[row].size() && column >= 0) {
		return elements[row][column];
	}
	return NULL;
}	
const Manager *Matrix::get(int row, int column) const {
	row--;
	column--;
	if(row < elements.size() && row >= 0 && column < elements[row].size() && column >= 0) {
		return elements[row][column];
	}
	return NULL;
}	
bool Matrix::add(MathOperation op, const Matrix *matrix) {
	if(!matrix) return false;
	switch(op) {
		case OPERATION_SUBTRACT: {
			return subtract(matrix);
		}
		case OPERATION_ADD: {
			return add(matrix);
		} 
		case OPERATION_MULTIPLY: {
			return multiply(matrix);
		}
		case OPERATION_DIVIDE: {
			return divide(matrix);
		}		
		case OPERATION_RAISE: {
			return raise(matrix);
		}
		case OPERATION_EXP10: {
			return exp10(matrix);
		}
	}
	return false;	
}
bool Matrix::add(const Matrix *matrix) {
	if(columns() != matrix->columns() || rows() != matrix->rows()) {
		return false;
	}
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->add(matrix->get(index_r + 1, index_c + 1), OPERATION_ADD);
			if(!elements[index_r][index_c]->isPrecise()) b_exact = false;
		}
	}
	return true;
}
bool Matrix::subtract(const Matrix *matrix) {
	if(columns() != matrix->columns() || rows() != matrix->rows()) {
		return false;
	}
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->add(matrix->get(index_r + 1, index_c + 1), OPERATION_SUBTRACT);
			if(!elements[index_r][index_c]->isPrecise()) b_exact = false;
		}
	}
	return true;
}
bool Matrix::multiply(const Matrix *matrix) {
	if(columns() == 1 && matrix->columns() == 1 && rows() == matrix->rows()) {
		Manager mngr;
		for(int index_r = 1; index_r <= rows(); index_r++) {
			Manager mngr2(get(index_r, 1));
			mngr2.add(matrix->get(index_r, 1), OPERATION_MULTIPLY);
			mngr.add(&mngr2, OPERATION_ADD);
		}
		setRows(1);
		setColumns(1);
		set(&mngr, 1, 1);
		return true;
	} else if(rows() == 1 && matrix->rows() == 1 && columns() == matrix->columns()) {
		Manager mngr;
		for(int index_c = 1; index_c <= columns(); index_c++) {
			Manager mngr2(get(1, index_c));	
			mngr2.add(matrix->get(1, index_c), OPERATION_MULTIPLY);
			mngr.add(&mngr2, OPERATION_ADD);
		}
		setRows(1);		
		setColumns(1);
		set(&mngr, 1, 1);	
		return true;
	} else {
		if(columns() != matrix->rows()) {
			return false;
		}	
		Matrix product(rows(), matrix->columns());
		Manager mngr;
		for(int index_r = 1; index_r <= product.rows(); index_r++) {
			for(int index_c = 1; index_c <= product.columns(); index_c++) {
				for(int index = 1; index <= columns(); index++) {
					mngr.set(get(index_r, index));
					mngr.add(matrix->get(index, index_c), OPERATION_MULTIPLY);
					product.get(index_r, index_c)->add(&mngr, OPERATION_ADD);
					if(!product.get(index_r, index_c)->isPrecise()) {
						product.setPrecise(false);
					}	
				}			
			}		
		}
		set(&product);
		return true;
	}
	return false;
}
bool Matrix::divide(const Matrix *matrix) {
	return false;
}
bool Matrix::raise(const Matrix *matrix) {
	return false;
}
bool Matrix::exp10(const Matrix *matrix) {
	return false;
}
bool Matrix::add(MathOperation op, const Manager *mngr) {
	if(!mngr) return false;
	switch(op) {
		case OPERATION_SUBTRACT: {
			return subtract(mngr);
		}
		case OPERATION_ADD: {
			return add(mngr);
		} 
		case OPERATION_MULTIPLY: {
			return multiply(mngr);
		}
		case OPERATION_DIVIDE: {
			return divide(mngr);
		}		
		case OPERATION_RAISE: {
			return raise(mngr);
		}
		case OPERATION_EXP10: {
			return exp10(mngr);
		}
	}
	return false;	
}
bool Matrix::add(const Manager *mngr) {
	if(mngr->isMatrix()) {
		return add(mngr->matrix());
	}
	return false;
}
bool Matrix::subtract(const Manager *mngr) {
	if(mngr->isMatrix()) {
		return subtract(mngr->matrix());
	}
	return false;
}
bool Matrix::multiply(const Manager *mngr) {
	if(mngr->isMatrix()) {
		return multiply(mngr->matrix());
	}
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->add(mngr, OPERATION_MULTIPLY);
			if(!elements[index_r][index_c]->isPrecise()) b_exact = false;
		}
	}
	return true;
}
bool Matrix::divide(const Manager *mngr) {
	if(mngr->isMatrix()) {
		return divide(mngr->matrix());
	}
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->add(mngr, OPERATION_DIVIDE);
			if(!elements[index_r][index_c]->isPrecise()) b_exact = false;
		}
	}
	return true;
}
bool Matrix::raise(const Manager *mngr) {
	if(mngr->isOne()) {
		return true;
	}
	if(mngr->isMatrix()) {
		return raise(mngr->matrix());
	} else if(mngr->isFraction() && mngr->fraction()->isMinusOne()) {
		return inverse();
	} else if(mngr->isFraction() && mngr->fraction()->isInteger()) {
		if(mngr->fraction()->isNegative()) {
			return false;
		} else {
			Integer integer(mngr->fraction()->numerator());
			Matrix mtrx(this);
			integer.add(-1);
			for(; integer.isPositive(); integer.add(-1)) {
				multiply(&mtrx);
			}
			return true;
		}
	}
	return false;
}
bool Matrix::exp10(const Manager *mngr) {
	if(mngr->isMatrix()) {
		return exp10(mngr->matrix());
	}
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->add(mngr, OPERATION_EXP10);
			if(!elements[index_r][index_c]->isPrecise()) b_exact = false;
		}
	}	
	return true;
}
bool Matrix::equals(const Matrix *matrix) const {
	if(rows() != matrix->rows() || columns() != matrix->columns()) return false;
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			if(!get(index_r, index_c)->equals(matrix->get(index_r, index_c))) return false;
		}
	}	
	return true;	
}
bool Matrix::isPrecise() const {
	return b_exact;
}
void Matrix::setPrecise(bool is_precise) {
	b_exact = is_precise;
}
Vector *Matrix::getRange(int start, int end) {
	int n = columns() * rows();
	if(start < 1) start = 1;
	else if(start > n) start = n;
	if(end < 1 || end > n) end = n;
	else if(end < start) end = start;	
	Vector *vctr = new Vector(end + 1 - start);
	int i;
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			i = (index_r - 1) * columns() + index_c;
			if(i >= start && i <= end) {
				vctr->set(get(index_r, index_c), i + 1 - start);
			}
		}			
	}
	return vctr;
}
Vector *Matrix::toVector() {
	Vector *vctr = new Vector(columns() * rows());
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			vctr->set(get(index_r, index_c), (index_r - 1) * columns() + index_c);
		}			
	}
	return vctr;
}
Vector *Matrix::rowToVector(int row) {
	if(row < 1 || row > rows()) {
		return NULL;
	}
	Vector *vctr = new Vector(columns());
	for(int index_c = 1; index_c <= columns(); index_c++) {
		vctr->set(get(row, index_c), index_c);
	}
	return vctr;
}
Vector *Matrix::columnToVector(int column) {
	if(column < 1 || column > columns()) {
		return NULL;
	}
	Vector *vctr = new Vector(rows());
	for(int index_r = 1; index_r <= rows(); index_r++) {
		vctr->set(get(index_r, column), index_r);
	}
	return vctr;
}
bool Matrix::isVector() const {
	return b_vector;
}
void Matrix::recalculateFunctions() {
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->recalculateFunctions();
		}
	} 	
}
string Matrix::print(NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Integer *l_exp, bool in_composite, bool in_power) const {
	string str = "matrix(";
	str += i2s(rows());
	str += CALCULATOR->getComma();
	str += " ";
	str += i2s(columns());
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			str += CALCULATOR->getComma();
			str += " ";
			str += elements[index_r][index_c]->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power);
		}
	}	
	str += ")";
	return str;
}


Vector::Vector() : Matrix() {
	b_vector = true;
}
Vector::Vector(int components) : Matrix(1, components) {
	b_vector = true;
}
Vector::Vector(const Matrix *vector) : Matrix(vector) {
	b_vector = true;
}
void Vector::set(const Vector *vector) {
	Matrix::set(vector);
}
void Vector::set(const Manager *mngr, int component) {
	Matrix::set(mngr, 1, component);
}
Manager *Vector::get(int component) {
	return Matrix::get(1, component);
}
const Manager *Vector::get(int component) const {
	return Matrix::get(1, component);
}
int Vector::components() const {
	return Matrix::columns();
}
void Vector::addComponent() {
	Matrix::addColumn();
}
string Vector::print(NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Integer *l_exp, bool in_composite, bool in_power) const {
	if(!isVector()) {
		return Matrix::print(nrformat, displayflags, min_decimals, max_decimals, prefix, in_exact, usable, toplevel, plural, l_exp, in_composite, in_power);
	}
	string str = "vector(";
	for(int index = 1; index <= components(); index++) {
		if(index != 1) {
			str += CALCULATOR->getComma();
			str += " ";
		}
		str += get(index)->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power);
	}	
	str += ")";
	return str;
}
