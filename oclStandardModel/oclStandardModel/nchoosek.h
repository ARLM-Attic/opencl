#ifndef __NCHOOSEK_H
#define __NCHOOSEK_H

//
// Returns the set of all permutations of 0..n-1 with k elements
// this is represented as a matrix with 'n choose k' lines and k columns
//
int** nchoosek(const int n, const int k, int* nck);
//
// This function returns a matrix with n+1 columns (the last element is the number of non-zero elements in the row)
//  and sum(i=1, i<=m, nchoosek(n, i)) rows.
// Each row contains 0s and 1s corresponds to a possible projection.
// Returns a matrix with n+2 columns and 'lines' lines, which represents all the possible projections
//  of 1 to m coordinates.
//
int** nchoosekMatrix(const int n, const int m, int *lines);
//
// Returns a vector containing the nchoosekMatrix
// This vector size is lines*(n+2)
//
int* nchoosekVector(const int n, const int m, int *lines);

#endif