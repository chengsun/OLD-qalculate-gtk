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
	elements[0].resize(1);
}
Matrix::Matrix(int nr_of_rows, int nr_of_columns) {
	b_exact = true;
	elements.resize(1);
	elements[0].resize(1);
	setColumns(nr_of_columns);
	setRows(nr_of_rows);
}
Matrix::Matrix(const Matrix *matrix) {
}
Matrix::~Matrix() {
}
int Matrix::rows() const {
	return elements.size();
}
int Matrix::columns() const {
	return elements[0].size();
}
void Matrix::setRows(int nr_of_rows) {
	if(nr_of_rows < 1) nr_of_rows = 1;
	elements.resize(nr_of_rows);
	setColumns(columns());
}
void Matrix::setColumns(int nr_of_columns) {
	if(nr_of_columents < 1) nr_of_columns = 1;
	for(int index = 0; index < elements.size(); index++) {
		elements[index].resize(nr_of_columns);	
	}
}
void Matrix::addRow() {
	elements.resize(elements.size() + 1);
	elements[elements.size() - 1].resize(columns);
}
void Matrix::addColumn() {
	for(int index = 0; index < elements.size(); index++) {
		elements[index].resize(elements[index].size() + 1);	
	}
}
void Matrix::set(Manager *mngr, int row, int column) {
	row--;
	column--;
	if(row < elements.size() && row >= 0 && column < elements[row].size() && column >= 0) {
		elements[row][column].set(mngr);
	}
}
Manager *Matrix::get(int row, int column) {
	row--;
	column--;
	if(row < elements.size() && row >= 0 && column < elements[row].size() && column >= 0) {
		return &elements[row][column];
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
			return pow(matrix);
		}
		case EXP10: {
			return exp10(matrix);
		}
	}
	return true;	
}
bool Matrix::add(const Matrix *matrix) {
}
bool Matrix::subtract(const Matrix *matrix) {
}
bool Matrix::multiply(const Matrix *matrix) {
}
bool Matrix::divide(const Matrix *matrix) {
}
bool Matrix::raise(const Matrix *matrix) {
}
bool Matrix::exp10(const Matrix *matrix) {
}
bool Matrix::add(MathOperation op, Manager *mngr) {
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
			return pow(mngr);
		}
		case EXP10: {
			return exp10(mngr);
		}
	}
	return true;	
}
bool Matrix::add(Manager *mngr) {
	return false;
}
bool Matrix::subtract(Manager *mngr) {
	return false;
}
bool Matrix::multiply(Manager *mngr) {
	for(int index_c = 0; index_c < elements.size(); index_c++) {
		for(int index_r = 0; index_r < elements[index_c].size(); index_r++) {
			elements[index_c][index_r].add(mngr, MULTIPLY);
		}
	}
	return true;
}
bool Matrix::divide(Manager *mngr) {
	for(int index_c = 0; index_c < elements.size(); index_c++) {
		for(int index_r = 0; index_r < elements[index_c].size(); index_r++) {
			elements[index_c][index_r].add(mngr, DIVIDE);
		}
	}
	return true;
}
bool Matrix::raise(Manager *mngr) {
	return false;
}
bool Matrix::exp10(Manager *mngr) {
	for(int index_c = 0; index_c < elements.size(); index_c++) {
		for(int index_r = 0; index_r < elements[index_c].size(); index_r++) {
			elements[index_c][index_r].add(mngr, EXP10);
		}
	}	
	return true;
}
