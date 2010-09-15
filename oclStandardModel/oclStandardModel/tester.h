#ifndef __TESTER_H
#define __TESTER_H

#include "stSimplex.h"
#include <cmath>
#include <limits>

void printMatrix(const SMatrix m, int lines=0, int cols=0);
void echelonTest(float* simplices, int* ranks, int* indCols);
void stSimplexCPU(const float* const simplices,
				  float* const constraints,
				  const int* const proj,
				  const int projRows,
				  const int numSimplices);
void makeStandardOriented(float* const coefficients);
void triangleFaceOrientation(const SMatrix points, const int rank,
							 const int* const columns, float* const coefficients);
//void hyperplaneTest(float* points, float* constraints, const int* basis, const int rank, const int* columns);
//void fullTest(float* simplices, int* ranks, int* indCols, float* constraints, const int* basis);

#endif