#ifndef _ST_SIMPLEX_H_
#define _ST_SIMPLEX_H_

#define N 3
#define K 2
#define SIMPLICES 1048576	//(1024*1024)
#define C_PER_SIMPLEX 16
#define CONSTRAINTS (SIMPLICES*C_PER_SIMPLEX)
#define TOLERANCE 1.0e-5f

#define LOCAL_WORKSIZE 128	// 64 registers per thread with 16k threads per block

typedef float constraint[N+1];
typedef float SMatrix[N+1][K+1];
//typedef float FMatrix[(N+1)*SIMPLICES][K+1];
//typedef float CMatrix[CONSTRAINTS][N+1];

#endif


//void getHyperplane(const SMatrix points,
//				   float* const c,
//				   __constant const int* const basis,
//				   const int rank) {
//	SMatrix m;
//
//	// Initializes the first matrix (first col out) -> first coefficient
//	for (int h = 0; h < rank; h++) {
//		for (int col = 0; col < rank; col++) {
//			if (col < rank-1)
//				m[col][h] = points[h].coordinates[col+1];
//			else
//				m[col][h] = 1;
//		}
//	}
//	c[0] = determinant(m, rank);
//
//	// updates the matrix -> more n-1 coefficients
//	for (int col = 0; col < num-1; col++) {
//		for (int h = 0; h < num; h++) {
//			m[col][h] = points[h].coordinates[col];
//		}
//		int multiplier = ((col+1)%2)?(-1):(1);
//		c->coefficients[col+1] = determinant(m, num) * multiplier;
//	}
//
//	// last one is b
//	for (int h = 0; h < num; h++)
//		m[num-1][h] = points[h].coordinates[num-1];
//	c->b = determinant(m, num) * -1;
//
//	free(m);
//
//	return c;
//}
