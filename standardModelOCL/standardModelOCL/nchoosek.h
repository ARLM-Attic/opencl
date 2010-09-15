#ifndef OCL_ST_MODEL_NCHOOSEK_H_
#define OCL_ST_MODEL_NCHOOSEK_H_

int** nchoosek(const int n, const int k, int& nck);
int** nchoosekMatrix(const int n, const int m, int& lines);
int* nchoosekVector(const int n, const int m, int& lines);

#endif
