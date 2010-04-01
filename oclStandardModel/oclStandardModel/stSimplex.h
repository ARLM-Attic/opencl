#ifndef _ST_SIMPLEX_H_
#define _ST_SIMPLEX_H_

#define N 3
#define K 3
#define SIMPLICES (1024*1024)
#define C_PER_SIMPLEX 16
#define CONSTRAINTS (SIMPLICES*C_PER_SIMPLEX)
#define TOLERANCE 1.0e-5f

#define LOCAL_WORKSIZE 256

typedef float Constraint[N+1];
typedef float SMatrix[N+1][K+1];
typedef float FMatrix[(N+1)*SIMPLICES][K+1];
typedef float CMatrix[CONSTRAINTS][N+1];

#endif
