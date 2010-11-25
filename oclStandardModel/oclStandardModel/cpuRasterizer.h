#ifndef _CPU_RASTERIZER_H_
#define _CPU_RASTERIZER_H_

#include "rasterizer.h"

#include "stSimplex.h"
#include <cmath>
#include <limits>
#include <cstdlib>

class CPU_Rasterizer : public Rasterizer {
	protected:
	public:
		void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols);
		void echelonTest(float* simplices, int* ranks, int* indCols, const int numSimplices);
		void stSimplexCPU(const float* const simplices,
			float* const constraints,
			const int* const proj,
			const int projRows,
			const int numSimplices,
			bool* const volume,
			const int volumeW, const int volumeH, const int volumeD);
		void makeStandardOriented(float* const coefficients);
		void triangleFaceOrientation(const SMatrix points, const int rank,
			const int* const columns, float* const coefficients);
		void getHyperplane(SMatrix points, float* const c, const int* base, const int rank, const int* columns);
		void halfSpaceConstraints(float* const coefficients);

		bool isZero(float value, float tolerance);
		float determinant(const SMatrix matrix, int size);
		void copyProjectedMatrix(SMatrix src,  SMatrix dst, const int* base);
};

#endif