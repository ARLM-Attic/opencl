#ifndef __TESTER_H
#define __TESTER_H

#include "stSimplex.h"
#include <cmath>
#include "float.h"

void echelonTest(float* simplices, int* ranks, int* indCols);
void hyperplaneTest(float* points, float* constraints, const int* basis, const int rank, const int* columns);

#endif