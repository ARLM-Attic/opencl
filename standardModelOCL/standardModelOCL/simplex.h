#ifndef OCL_ST_MODEL_SIMPLEX_H_
#define OCL_ST_MODEL_SIMPLEX_H_

#define N 3
#define K 2
#define SIMPLICES 1048576//64082 // 179*179*2 - //1048576		//(1024*1024)
#define C_PER_SIMPLEX 16
#define CONSTRAINTS (SIMPLICES*C_PER_SIMPLEX)
#define TOLERANCE 1.0e-5f

#define INF_FLOAT 99999999.0f

#define LOCAL_WORKSIZE 256	// 64 registers per thread with 16k registers per block

typedef float constraint[N+1];
typedef float smatrix[N+1][K+1];

#endif
