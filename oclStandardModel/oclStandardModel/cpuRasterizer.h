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
		//
		// Change SMatrix 'matrix' to column echelon form.
		//	Returns:
		//		- rank of the matrix
		//		- new order of the columns
		//
		void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols);
		//
		//	Compute the Standard Model constraints
		//
		void stSimplex();
		//
		//	Receives a constraint and change it's coefficients
		//	signal if it's not standard oriented
		//
		void makeStandardOriented(float* const coefficients);
		//
		//	Make the normals of all faces of a triangle 
		//	point to the same side
		//
		void triangleFaceOrientation(const SMatrix points, const int rank,
									const int* const columns, float* const coefficients);
		//
		//	Compute the hyperplane induced by the "points" matrix.
		//	- Return it's coefficients in "c" array.
		//
		//	   (in this implementation, the hyperplane will be a plane and the points
		//	    matrix will represent a triangle)
		//
		void getHyperplane(SMatrix points, float* const c, const int* base, const int rank, const int* columns);
		//
		//	Compute the independent term of the half space given by "coefficients"
		//
		void halfSpaceConstraints(float* const coefficients);

		// Misc
		bool isZero(float value, float tolerance);
		float determinant(const SMatrix matrix, int size);
		void copyProjectedMatrix(SMatrix src,  SMatrix dst, const int* base);
		//
		//	Not used. Change all the input simplices to echelon form
		//
		void echelonTest(int* ranks, int* indCols);
};

#endif