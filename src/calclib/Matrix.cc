/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Matrix.h"

/**
* Contains a matrix.
*/

Matrix::Matrix() {
	b_exact = true;
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
	Matrix mtrx_save(this);
	setColumns(mtrx_save.rows());
	setRows(mtrx_save.columns());	
	for(int index_r = 1; index_r <= rows(); index_r++) {
		for(int index_c = 1; index_c <= columns(); index_c++) {
			set(mtrx_save.get(index_c, index_r), index_r, index_c);
		}
	}	
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
Manager *Matrix::determinant() const {
	if(columns() != rows()) {
		return NULL;
	}
	Manager *mngr = new Manager();
	if(rows() == 1) {
		mngr->set(get(1, 1));
	} else if(rows() == 2) {
		Manager tmp;
		mngr->set(get(1, 1));
		mngr->add(get(2, 2), MULTIPLY);
		tmp.set(get(2, 1));
		tmp.add(get(1, 2), MULTIPLY);
		mngr->add(&tmp, SUBTRACT);
	} else {
		Matrix mtrx(rows() - 1, columns() - 1);
		for(int index_c = 1; index_c <= columns(); index_c++) {
			for(int index_r = 2; index_r <= rows(); index_r++) {
				for(int index_c2 = 1; index_c2 <= columns(); index_c2++) {
					if(index_c2 > index_c) {
						mtrx.set(get(index_r, index_c2), index_r - 1, index_c2 - 1);
					} else if(index_c2 < index_c) {
						mtrx.set(get(index_r, index_c2), index_r - 1, index_c2);
					}
				}
			}	
			Manager *tmp = mtrx.determinant();
			tmp->add(get(1, index_c), MULTIPLY);
			if(index_c % 2 == 0) {
				mngr->add(tmp, SUBTRACT);
			} else {
				mngr->add(tmp, ADD);
			}
			delete tmp;
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
		case SUBTRACT: {
			return subtract(matrix);
		}
		case ADD: {
			return add(matrix);
		} 
		case MULTIPLY: {
			return multiply(matrix);
		}
		case DIVIDE: {
			return divide(matrix);
		}		
		case RAISE: {
			return raise(matrix);
		}
		case EXP10: {
			return exp10(matrix);
		}
	}
	return true;	
}
bool Matrix::add(const Matrix *matrix) {
	if(columns() != matrix->columns() || rows() != matrix->rows()) {
		return false;
	}
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->add(matrix->get(index_r + 1, index_c + 1), ADD);
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
			elements[index_r][index_c]->add(matrix->get(index_r + 1, index_c + 1), SUBTRACT);
			if(!elements[index_r][index_c]->isPrecise()) b_exact = false;
		}
	}
	return true;
}
bool Matrix::multiply(const Matrix *matrix) {
	if(columns() != matrix->rows()) {
		return false;
	}
	Matrix product(rows(), matrix->columns());
	Manager mngr;
	for(int index_r = 1; index_r <= product.rows(); index_r++) {
		for(int index_c = 1; index_c <= product.columns(); index_c++) {
			for(int index = 1; index <= columns(); index++) {
				mngr.set(get(index_r, index));
				mngr.add(matrix->get(index, index_c), MULTIPLY);
				product.get(index_r, index_c)->add(&mngr, ADD);
				if(product.get(index_r, index_c)->isPrecise()) {
					product.setPrecise(false);
				}	
			}			
		}		
	}
	set(&product);
	return true;
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
		case SUBTRACT: {
			return subtract(mngr);
		}
		case ADD: {
			return add(mngr);
		} 
		case MULTIPLY: {
			return multiply(mngr);
		}
		case DIVIDE: {
			return divide(mngr);
		}		
		case RAISE: {
			return raise(mngr);
		}
		case EXP10: {
			return exp10(mngr);
		}
	}
	return true;	
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
			elements[index_r][index_c]->add(mngr, MULTIPLY);
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
			elements[index_r][index_c]->add(mngr, DIVIDE);
			if(!elements[index_r][index_c]->isPrecise()) b_exact = false;
		}
	}
	return true;
}
bool Matrix::raise(const Manager *mngr) {
	if(mngr->isMatrix()) {
		return raise(mngr->matrix());
	}
	return false;
}
bool Matrix::exp10(const Manager *mngr) {
	if(mngr->isMatrix()) {
		return exp10(mngr->matrix());
	}
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			elements[index_r][index_c]->add(mngr, EXP10);
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

string Matrix::print(NumberFormat nrformat, int displayflags, int min_decimals, int max_decimals, Prefix *prefix, bool *in_exact, bool *usable, bool toplevel, bool *plural, Integer *l_exp, bool in_composite, bool in_power) const {
	string str = "[";
	for(int index_r = 0; index_r < elements.size(); index_r++) {
		if(index_r != 0) {
			str += "  \\  ";
		}
		for(int index_c = 0; index_c < elements[index_r].size(); index_c++) {
			if(index_c != 0) str += "  ";
			str += elements[index_r][index_c]->print(nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power);
		}
	}	
	str += "]";
	if(displayflags & DISPLAY_FORMAT_TAGS) {
		str += "<small><sub>";
		str += i2s(rows());
		str += "x";
		str += i2s(columns());
		str += "</sub></small>";
	}
	return str;
}
