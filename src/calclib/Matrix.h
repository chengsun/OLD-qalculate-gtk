/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef MATRIX_H
#define MATRIX_H

class Matrix;

#include "Calculator.h"
#include "Manager.h"

/**
* Contains a matrix.
*/

class Matrix {
  protected:
	vector<vector<Manager> > elements;
	bool b_exact;
  public:
  
	/**
	* Constructs an matrix one initial element.
	*/	  
	Matrix();
	
	/**
	* Constructs a matrix with specified number of rows and columns.
	*
	* @param nr_of_rows Number of initial rows (> 0).
	* @param nr_of_columns Number of initial columns (> 0).	
	*/	
	Matrix(int nr_of_rows, int nr_of_columns = 1);	
	
	/**
	* Constructs a copy of a matrix.
	*/		
	Matrix(const Matrix *matrix);
	
	~Matrix();

	/**
	* Returns the number of rows in the matrix.
	*
	* @see #columns
	* @see #setRows
	*/		
	int rows() const;
	/**
	* Returns the number of columns in the matrix.
	*
	* @see #rows
	* @see #setColumns	
	*/		
	int columns() const;
	
	/**
	* Sets the number of rows in the matrix.
	*
	* setRows will delete elements that do not fit with the new dimensions.
	*
	* @param nr_of_rows New number of rows (> 0).
	* @see #setColumns
	*/
	void setRows(int nr_of_rows);
	/**
	* Sets the number of columns in the matrix.
	*
	* setColumns will delete elements that do not fit with the new dimensions.
	*
	* @param nr_of_columns New number of columns (> 0).
	* @see #setColumns
	*/
	void setColumns(int nr_of_columns);

	/**
	* Adds a new row to the matrix.
	*
	* @see #addColumn
	* @see #setRows
	*/
	void addRow();
	/**
	* Adds a new row to the matrix.
	*
	* @see #addRow
	* @see #setColumns
	*/	
	void addColumn();

	/**
	* Sets the value of an element in the given row and column.
	*
	* @param mngr The new value of the element.
	* @param row Specifies the row in the matrix where the element resides.
	* @param column Specifies the column in the matrix where the element resides.
	*/
	void set(Manager *mngr, int row, int column);
	
	/**
	* Returns the value of an element in the given row and column.
	*
	* @param row Specifies the row in the matrix where the element resides.
	* @param column Specifies the column in the matrix where the element resides.	
	* @return Element at row,column.
	*/	
	Manager *get(int row, int column);	
	
	/**
	* Add a matrix using the specified mathematical operation.
	*
	* @param op The operation to use.
	* @param matrix The matrix to add.
	* @return If the operation was successful or not.
	*/
	bool add(MathOperation op, const Matrix *matrix);

	/**
	* Adds a matrix.
	*
	* @param matrix Term to add.
	* @return If the operation was successful or not.
	*/
	bool add(const Matrix *matrix);
	/**
	* Subtracts a matrix.
	*
	* @param matrix Term to subtract.
	* @return If the operation was successful or not.
	*/
	bool subtract(const Matrix *matrix);
	/**
	* Multiplies with a matrix.
	*
	* @param matrix Factor to multiply with.
	* @return If the operation was successful or not.
	*/
	bool multiply(const Matrix *matrix);
	/**
	* Divides with a matrix.
	*
	* @param matrix The denominator.
	* @return If the operation was successful or not.
	*/
	bool divide(const Matrix *matrix);
	/**
	* Raise with matrix as exponent.
	*
	* @param matrix The exponent.
	* @return If the operation was successful or not.
	*/
	bool raise(const Matrix *matrix);
	/**
	* Multiplies with 10 raised to matrix.
	*
	* @param matrix The exponent.
	* @return If the operation was successful or not.
	*/
	bool exp10(const Matrix *matrix);

	/**
	* Add a manager using the specified mathematical operation.
	*
	* @param op The operation to use.
	* @param mngr The manager to add.
	* @return If the operation was successful or not.
	*/
	bool add(MathOperation op, Manager *mngr);

	/**
	* Adds a manager.
	*
	* @param mngr Term to add.
	* @return If the operation was successful or not.
	*/
	bool add(Manager *mngr);
	/**
	* Subtracts a manager.
	*
	* @param mngr Term to subtract.
	* @return If the operation was successful or not.
	*/
	bool subtract(Manager *mngr);
	/**
	* Multiplies with a manager.
	*
	* @param mngr Factor to multiply with.
	* @return If the operation was successful or not.
	*/
	bool multiply(Manager *mngr);
	/**
	* Divides with a manager.
	*
	* @param mngr The denominator.
	* @return If the operation was successful or not.
	*/
	bool divide(Manager *mngr);
	/**
	* Raise with manager as exponent.
	*
	* @param mngr The exponent.
	* @return If the operation was successful or not.
	*/
	bool raise(Manager *mngr);
	/**
	* Multiplies with 10 raised to manager.
	*
	* @param mngr The exponent.
	* @return If the operation was successful or not.
	*/
	bool exp10(Manager *mngr);
	
	bool isPrecies() const;
	void setPrecise(bool is_precies);

};

#endif
