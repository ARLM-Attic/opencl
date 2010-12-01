#ifndef _ST_SIMPLEX_H_
#define _ST_SIMPLEX_H_

#define N_DIMENSIONS 3 // N points, N dimensions
#define K 2 // K-simplex, K+1 dimensions
#define _SIMPLICES 1048576	//(1024*1024)
#define C_PER_SIMPLEX 16
#define CONSTRAINTS (SIMPLICES*C_PER_SIMPLEX)
#define TOLERANCE 1.0e-5f

#define LOCAL_WORKSIZE 128	// 64 registers per thread with 16k registers per block

typedef float constraint[N_DIMENSIONS+1];
typedef float SMatrix[N_DIMENSIONS+1][K+1];
//typedef float FMatrix[(N_DIMENSIONS+1)*SIMPLICES][K+1];
//typedef float CMatrix[CONSTRAINTS][N_DIMENSIONS+1];

#endif
