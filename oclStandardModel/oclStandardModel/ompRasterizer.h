#ifndef _OMP_RASTERIZER_H_
#define _OMP_RASTERIZER_H_

#include "rasterizer.h"

#include "stSimplex.h"
#include <cmath>
#include <limits>
#include <cstdlib>
#include <omp.h>

/* For more information about each function, check cpuRasterizer.h */

class OMP_Rasterizer : public Rasterizer {
protected:
public:
	OMP_Rasterizer();

	void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols);
	void echelonTest(int* ranks, int* indCols);
	void stSimplex();
	void makeStandardOriented(float* const coefficients);
	void triangleFaceOrientation(const SMatrix points, const int rank,
		const int* const columns, float* const coefficients);
	void getHyperplane(SMatrix points, float* const c, const int* base, const int rank, const int* columns);
	void halfSpaceConstraints(float* const coefficients);

	bool isZero(float value, float tolerance);
	float determinant(const SMatrix matrix, int size);
	void copyProjectedMatrix(SMatrix src,  SMatrix dst, const int* base);

	void fillVolume();
};

#endif