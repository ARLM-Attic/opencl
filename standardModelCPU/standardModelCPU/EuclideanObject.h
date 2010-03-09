#ifndef _EUCLIDEAN_OBJECT_H_
#define _EUCLIDEAN_OBJECT_H_

#include <stdio.h>
#include <stdlib.h>

#include "Constraint.h"
#include "LinearAlgebra.h"
#include "definitions.h"

// deprecated
#include <vector>
using namespace std;

int** nchoosek(const int n, const int k, int* nck);
int** nchoosekMatrix(const int n, const int m, int *lines);
bool hasStandardOrientation(const float* const coefficients);
void getHyperplane(const Matrix points, constraint* const c, const int* basis, const int rank, const int* columns);

void printMatrix(const Matrix m, int lines=0, int cols=0);

void copyProjectedMatrix(const Matrix src, Matrix dst, const int* basis);
void halfSpaceConstraints(constraint* const halfSpace);
vector<constraint*> stSimplex(const Matrix points);

#endif