#ifndef __LINEAR_ALGEBRA_H
#define __LINEAR_ALGEBRA_H

#include <cmath>
#include <cstdlib>

#include "definitions.h"

float determinant(const Matrix matrix, int size, const float tolerance);
float determinant(const Matrix matrix, int size);
void columnEchelonForm(Matrix matrix, int* rank, int* independentCols);
void columnEchelonForm(Matrix matrix, int* rank, int* independentCols, const float tolerance);
int pivot(Matrix m, const int row, const float tolerance);

#endif